using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Units;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that Paperless breaks a paragraph's lines where LibreOffice breaks them.
/// </summary>
/// <remarks>
/// <para>
/// The end-to-end test of everything measurement-related at once: the font resolver has to pick the
/// same face, the metrics reader has to read the same advance widths, the line breaker has to offer
/// the same opportunities, and the filler has to choose among them the same way. Any one of the four
/// being wrong moves a break, and a moved break moves every line after it — so this is a single
/// assertion that covers four components and would fail loudly for any of them.
/// </para>
/// <para>
/// LibreOffice lays the paragraph out into a PDF at a known text width; the words' bounding boxes are
/// read back out of the PDF and grouped into lines by their vertical position, which gives the words
/// LibreOffice put on each line. Paperless fills the same text at the same width and the two line
/// divisions are compared.
/// </para>
/// <para>
/// Both sides shape with HarfBuzz, so both apply the font's kerning and its optional ligatures. That
/// is not a refinement — it is what makes the comparison possible at all. Measured without kerning,
/// the second paragraph here comes out 0.8 pt over a 481.9 pt text width and loses its last word to
/// the next line, and every line after that is then wrong too. The accumulated kerning on that one
/// line is 244 thousandths of an em, just under 3 pt, and Carlito shapes its "ti" as a ligature 15
/// design units narrower than the two glyphs apart.
/// </para>
/// <para>
/// Where they can still legitimately differ: Paperless does not split a run into script sub-runs the
/// way LibreOffice does, so a paragraph mixing Latin with a complex script may shape differently;
/// there is no bidi reordering; and there is no hyphenation, which needs a dictionary. The paragraphs
/// here are single-script and left-aligned, and the documents switch hyphenation off, so none of the
/// three is in play.
/// </para>
/// </remarks>
public sealed class LineBreakPositionTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-breaks").FullName;

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

    /// <summary>The page and margins the test documents use, in centimetres.</summary>
    private const double PageWidthCm = 21.0;
    private const double MarginCm = 2.0;

    /// <summary>The text width those give, which is what a line has to fit in.</summary>
    private static Length TextWidth => Length.FromMillimetres((PageWidthCm - (2 * MarginCm)) * 10);

    [Theory]
    [InlineData(
        "The quick brown fox jumps over the lazy dog, and then the dog gets up and jumps back "
        + "over the fox, which is not something the original sentence ever promised would happen.")]
    [InlineData(
        "Extraction and rendering are separate paths through the library, because extraction is "
        + "the common case and must not pay for fonts, layout or a rasteriser it never uses.")]
    [InlineData(
        "Short words in a row: a b c d e f g h i j k l m n o p q r s t u v w x y z and then some "
        + "much longer words afterwards to change where the line has to break.")]
    [InlineData(
        "Numbers like 1,234.56 and ranges like 100-199 and hyphenated-words all affect where a "
        + "line may be broken, so a paragraph containing them exercises more than plain prose.")]
    // Kerning-heavy on purpose: AV, AW, To, Ty, Yo, w. and y, are the pairs a font actually kerns,
    // and a paragraph made of them accumulates the error fast enough to move a break within one line.
    [InlineData(
        "AVAILABLE TYPEFACES: To Waverley, Yorkshire, Away. Take Two, Type Wave, Val Toy, "
        + "Wavy Tokyo, Yves Tavy, and AVAST Ye. Then Tywyn, Avon Way, Yew Tavern, Toy Wave.")]
    [InlineData(
        "Officially, the affiliated office staff filed affidavits affirming the offer; the "
        + "difficulty was that no official file suffix sufficed for the affiliate's fifth draft.")]
    public void ParagraphsBreakWhereLibreOfficeBreaksThem(string paragraph)
        => ParagraphsAgree(paragraph, sizePoints: 12, kerning: true);

    [Theory]
    [InlineData(
        "Extraction and rendering are separate paths through the library, because extraction is "
        + "the common case and must not pay for fonts, layout or a rasteriser it never uses.")]
    [InlineData(
        "AVAILABLE TYPEFACES: To Waverley, Yorkshire, Away. Take Two, Type Wave, Val Toy, "
        + "Wavy Tokyo, Yves Tavy, and AVAST Ye. Then Tywyn, Avon Way, Yew Tavern, Toy Wave.")]
    public void KerningTurnedOffOnBothSidesStillAgrees(string paragraph)
    {
        // The other half of the same claim. Writer's `style:letter-kerning="false"` and Paperless's
        // DisableKerning have to mean the same thing, or a document that switches kerning off would
        // reflow.
        ParagraphsAgree(paragraph, sizePoints: 12, kerning: false);

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");
        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();

        // And the premise, asserted rather than assumed: these two paragraphs really do break in
        // different places with kerning than without. Without this, both halves of the comparison
        // would keep passing if DisableKerning silently stopped doing anything.
        PaperlessLines(paragraph, face, sizePoints: 12, kerning: false)
            .ShouldNotBe(PaperlessLines(paragraph, face, sizePoints: 12, kerning: true));
    }

    [Theory]
    [InlineData(9.0)]
    [InlineData(14.0)]
    [InlineData(18.0)]
    public void TheAgreementHoldsAtOtherSizes(double sizePoints)
    {
        // Design units scale linearly but the rounding to EMUs does not, and a font's kerning is a
        // fixed number of design units however large the text is — so the same paragraph at three
        // sizes gives three different sets of breaks and three chances for the rounding to be wrong.
        ParagraphsAgree(
            "Extraction and rendering are separate paths through the library, because extraction is "
            + "the common case and must not pay for fonts, layout or a rasteriser it never uses.",
            sizePoints,
            kerning: true);
    }

    private void ParagraphsAgree(string paragraph, double sizePoints, bool kerning)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();

        List<string> reference = ReferenceLines(paragraph, sizePoints, kerning);
        List<string> actual = PaperlessLines(paragraph, face, sizePoints, kerning);

        actual.Count.ShouldBe(
            reference.Count,
            $"line count differs.\n  LibreOffice:\n    {string.Join("\n    ", reference)}"
            + $"\n  Paperless:\n    {string.Join("\n    ", actual)}");

        for (int i = 0; i < reference.Count; i++)
        {
            actual[i].ShouldBe(
                reference[i],
                $"line {i + 1} differs.\n  LibreOffice: {reference[i]}\n  Paperless:   {actual[i]}");
        }
    }

    [Fact]
    public void ALineTooNarrowForOneWordGivesItTheLineAnyway()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        LineFiller filler = new(new TextMeasurer(face));

        // A width narrower than a single word. The word overflows rather than being dropped, and the
        // paragraph still terminates — the alternative is an empty line followed by the same problem,
        // which is an infinite loop rather than a bad-looking page.
        List<TextLine> lines = filler.Fill(
            "unbreakable words here", Length.FromPoints(12), Length.FromPoints(4));

        lines.Count.ShouldBe(3);
        lines[^1].EndsParagraph.ShouldBeTrue();
        lines.Sum(l => l.Length).ShouldBe("unbreakable words here".Length);
    }

    [Fact]
    public void TrailingSpacesDoNotPushAWordToTheNextLine()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        TextMeasurer measurer = new(face);
        LineFiller filler = new(measurer);

        Length em = Length.FromPoints(12);

        // A width that fits "aaa bbb" exactly and nothing more. The space after "bbb" would overflow
        // it, and a filler that counted trailing spaces would move "bbb" to the next line — which is
        // why a paragraph of short words does not break after every one of them.
        Length exact = measurer.Measure("aaa bbb", em);
        List<TextLine> lines = filler.Fill("aaa bbb ccc", em, exact);

        lines[0].VisibleTextIn("aaa bbb ccc").ToString().ShouldBe("aaa bbb");
        lines[0].Width.ShouldBeLessThanOrEqualTo(exact);
        lines.Count.ShouldBe(2);
    }

    [Fact]
    public void MeasurementIsSummedInDesignUnitsRatherThanPerCharacter()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        TextMeasurer measurer = new(face);
        Length em = Length.FromPoints(12);

        // Rounding each character to a whole unit and adding them accumulates an error proportional to
        // the line's length, which does its damage exactly at the margin. Summing in design units and
        // scaling once keeps a long line's width the sum of its parts.
        string text = new('m', 200);
        Length whole = measurer.Measure(text, em);
        Length halves = measurer.Measure(text.AsSpan(0, 100), em)
                        + measurer.Measure(text.AsSpan(100), em);

        Math.Abs(whole.Emu - halves.Emu).ShouldBeLessThanOrEqualTo(1);
    }

    // ---------------------------------------------------------------------- the reference

    /// <summary>The words LibreOffice put on each line, one string per line.</summary>
    private List<string> ReferenceLines(string paragraph, double sizePoints, bool kerning)
    {
        string source = Path.Combine(
            _workDirectory,
            $"break-{paragraph.Length}-{sizePoints.ToString(CultureInfo.InvariantCulture)}"
            + $"-{(kerning ? "kerned" : "flat")}.fodt");
        File.WriteAllText(source, Document(paragraph, sizePoints, kerning));

        string pdf = _libreOffice.ConvertToPdf(source, _workDirectory);
        return GroupWordsIntoLines(pdf);
    }

    /// <summary>
    /// Reads the PDF's word boxes and groups them by vertical position.
    /// </summary>
    /// <remarks>
    /// Grouped by the top of the box rather than by any structure in the PDF, because a PDF has no
    /// lines — it has positioned text. Words whose tops are within a point of each other are on the
    /// same line, which is safe here because the line pitch is over fourteen points.
    /// </remarks>
    private static List<string> GroupWordsIntoLines(string pdfPath)
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

        List<string> lines = [];
        List<string> current = [];
        double currentTop = double.NaN;

        foreach (Match match in Regex.Matches(
                     output, "<word xMin=\"[0-9.]+\" yMin=\"([0-9.]+)\"[^>]*>([^<]*)</word>"))
        {
            double top = double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
            string word = System.Net.WebUtility.HtmlDecode(match.Groups[2].Value);

            if (double.IsNaN(currentTop) || Math.Abs(top - currentTop) <= 1.0)
            {
                currentTop = double.IsNaN(currentTop) ? top : currentTop;
                current.Add(word);
                continue;
            }

            lines.Add(string.Join(' ', current));
            current = [word];
            currentTop = top;
        }

        if (current.Count > 0) lines.Add(string.Join(' ', current));
        return lines;
    }

    /// <summary>The words Paperless puts on each line, in the same form.</summary>
    private static List<string> PaperlessLines(
        string paragraph, OpenTypeFace face, double sizePoints, bool kerning)
    {
        LineFiller filler = new(new TextMeasurer(face));
        List<TextLine> lines = filler.Fill(
            paragraph,
            Length.FromPoints(sizePoints),
            TextWidth,
            options: new ShapingOptions(Language: "en-GB", DisableKerning: !kerning));

        // Normalised to whitespace-separated words, because the comparison is about which words are on
        // which line rather than about how the spaces between them are represented.
        return
        [
            .. lines
                .Select(l => l.VisibleTextIn(paragraph).ToString())
                .Select(t => string.Join(' ', t.Split(' ', StringSplitOptions.RemoveEmptyEntries)))
                .Where(t => t.Length > 0),
        ];
    }

    /// <summary>
    /// One paragraph on a page of known width, with nothing that would change the text width.
    /// </summary>
    /// <remarks>
    /// No indents, no hyphenation and no justification. Each would change where a line breaks, and the
    /// point here is to compare the measurement rather than the styling — a justified paragraph breaks
    /// in the same places but at different word spacing, and hyphenation adds break opportunities that
    /// need a dictionary Paperless does not have.
    /// </remarks>
    private static string Document(string paragraph, double sizePoints, bool kerning)
    {
        string size = sizePoints.ToString(CultureInfo.InvariantCulture);
        string escaped = System.Security.SecurityElement.Escape(paragraph);
        string letterKerning = kerning ? "true" : "false";

        return $"""
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
            office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
         <office:automatic-styles>
          <style:page-layout style:name="Measured">
           <style:page-layout-properties fo:page-width="{PageWidthCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:page-height="29.7cm"
                                         fo:margin-left="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-right="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-top="2cm" fo:margin-bottom="2cm"/>
          </style:page-layout>
         </office:automatic-styles>
         <office:master-styles>
          <style:master-page style:name="Standard" style:page-layout-name="Measured"/>
         </office:master-styles>
         <office:styles>
          <style:style style:name="Measured" style:family="paragraph">
           <style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"
                                       fo:line-height="100%" fo:text-align="start"
                                       fo:text-indent="0cm" fo:hyphenate="false"/>
           <style:text-properties style:font-name="Carlito" fo:font-family="Carlito"
                                  fo:font-size="{size}pt" style:letter-kerning="{letterKerning}"
                                  fo:language="en" fo:country="GB"/>
          </style:style>
         </office:styles>
         <office:body>
          <office:text>
           <text:p text:style-name="Measured">{escaped}</text:p>
          </office:text>
         </office:body>
        </office:document>
        """;
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
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
