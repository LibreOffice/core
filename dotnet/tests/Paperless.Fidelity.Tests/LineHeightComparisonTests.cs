using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Units;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks the derived line height against the one LibreOffice actually renders.
/// </summary>
/// <remarks>
/// <para>
/// This is the single highest-value number in the whole rendering path. Line height decides where
/// every baseline sits, and therefore how much text fits on a page, and therefore where the page
/// breaks — so an error of one per cent does not stay local: it moves a break somewhere down the
/// document and every page after that differs for reasons unrelated to drawing.
/// </para>
/// <para>
/// It is also the number a font gives several conflicting answers to, so it cannot be checked by
/// reading a field. The check here is end-to-end and independent of Paperless: LibreOffice renders
/// three single-spaced lines to PDF, the baselines are measured out of the PDF, and the distance
/// between them is compared with what <see cref="LineSpacing"/> derived from the same font file. If
/// the precedence between <c>hhea</c>, <c>usWin*</c> and the typographic metrics were wrong, this is
/// where it would show.
/// </para>
/// </remarks>
public sealed class LineHeightComparisonTests : IDisposable
{
    /// <summary>
    /// How far the measured pitch may differ from the derived one, in points.
    /// </summary>
    /// <remarks>
    /// A fortieth of a point. LibreOffice lays out in hundredths of a millimetre and the PDF records
    /// positions in points, so the two disagree in the last digit however right both are; anything
    /// larger than this is a real difference in the metric rather than in the units.
    /// </remarks>
    private const double TolerancePoints = 0.025;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory = Directory.CreateTempSubdirectory("paperless-metrics").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Theory]
    [InlineData("Carlito", "Carlito-Regular.ttf")]
    [InlineData("Caladea", "Caladea-Regular.ttf")]
    [InlineData("Liberation Serif", "LiberationSerif-Regular.ttf")]
    [InlineData("Liberation Sans", "LiberationSans-Regular.ttf")]
    [InlineData("Liberation Mono", "LiberationMono-Regular.ttf")]
    public void TheDerivedLineHeightIsTheOneLibreOfficeRenders(string familyName, string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont(fileName);
        Assert.SkipWhen(fontPath is null, $"{fileName} is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        LineMetrics metrics = LineSpacing.Resolve(face);

        const double sizePoints = 12;
        double derived = metrics.ScaledLineHeight(Length.FromPoints(sizePoints)).Points;

        double measured = MeasureLinePitch(familyName, sizePoints);

        measured.ShouldBe(
            derived,
            TolerancePoints,
            $"{familyName}: LibreOffice renders a pitch of {measured:F4}pt and "
            + $"{metrics.Source} derives {derived:F4}pt. A line height this far out moves a page "
            + "break somewhere down any real document.");
    }

    [Fact]
    public void TheMetricSourceIsReportedSoADifferenceCanBeExplained()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        LineMetrics metrics = LineSpacing.Resolve(face);

        // Carlito is the interesting case: its two metric sets sum to the same line height — 1950+550
        // and 1536+512+452 are both 2500 — so the total cannot tell them apart. The ascent can, and
        // the ascent is what places the baseline within the line, so the source has to be recorded
        // rather than inferred from the height.
        metrics.Source.ShouldBe(LineMetricSource.WindowsMetrics);
        metrics.Ascent.ShouldBe(1950);
        metrics.Descent.ShouldBe(550);
        metrics.LineHeight.ShouldBe(2500);
    }

    /// <summary>
    /// The distance between consecutive baselines LibreOffice renders, in points.
    /// </summary>
    /// <remarks>
    /// Measured from the PDF rather than from anything Paperless produced, so the comparison is
    /// against LibreOffice's own output. Three lines rather than two: two give one measurement and no
    /// way to notice that the first line is placed differently from the rest, which it is — the top of
    /// the first line sits at the margin while every later one is placed relative to its predecessor.
    /// </remarks>
    private double MeasureLinePitch(string familyName, double sizePoints)
    {
        string source = Path.Combine(_workDirectory, $"{familyName.Replace(' ', '-')}.fodt");
        File.WriteAllText(source, SingleSpacedLines(familyName, sizePoints));

        string pdf = _libreOffice.ConvertToPdf(source, _workDirectory);
        File.Exists(pdf).ShouldBeTrue();

        List<double> tops = WordTops(pdf);
        tops.Count.ShouldBeGreaterThanOrEqualTo(3, "the document has three lines");

        // Consecutive differences, which must all agree: a single-spaced paragraph has one pitch, and
        // two differing gaps would mean the paragraph spacing had leaked in and the measurement is
        // not of the font's line height at all.
        double first = tops[1] - tops[0];
        for (int i = 2; i < tops.Count; i++)
        {
            (tops[i] - tops[i - 1]).ShouldBe(first, 0.01, "every line's pitch should be the same");
        }
        return first;
    }

    /// <summary>The top of each word box in the PDF, in points, top-down.</summary>
    private static List<double> WordTops(string pdfPath)
    {
        System.Diagnostics.ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add("-bbox");
        start.ArgumentList.Add(pdfPath);
        start.ArgumentList.Add("-");

        using System.Diagnostics.Process process =
            System.Diagnostics.Process.Start(start)
            ?? throw new InvalidOperationException("pdftotext did not start");

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);

        Assert.SkipWhen(
            process.ExitCode != 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<double> tops = [];
        foreach (Match match in Regex.Matches(output, "yMin=\"([0-9.]+)\""))
        {
            tops.Add(double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture));
        }
        return tops;
    }

    /// <summary>
    /// Three single-spaced lines in one font, with every source of extra spacing turned off.
    /// </summary>
    /// <remarks>
    /// Paragraph spacing above and below is set to zero and the line height to exactly 100%, because
    /// either would be added to the font's own line height and the measurement would then be of the
    /// document's styling rather than of the font.
    /// </remarks>
    private static string SingleSpacedLines(string familyName, double sizePoints) =>
        $"""
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
            office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
         <office:styles>
          <style:style style:name="Measured" style:family="paragraph">
           <style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"
                                       fo:line-height="100%"/>
           <style:text-properties style:font-name="{familyName}" fo:font-family="{familyName}"
                                  fo:font-size="{sizePoints.ToString(CultureInfo.InvariantCulture)}pt"/>
          </style:style>
         </office:styles>
         <office:body>
          <office:text>
           <text:p text:style-name="Measured">Alpha</text:p>
           <text:p text:style-name="Measured">Bravo</text:p>
           <text:p text:style-name="Measured">Charlie</text:p>
          </office:text>
         </office:body>
        </office:document>
        """;

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts/truetype/liberation2",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }
}
