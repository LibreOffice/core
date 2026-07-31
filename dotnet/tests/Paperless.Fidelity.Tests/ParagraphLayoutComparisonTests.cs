using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Units;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a paragraph's lines land where LibreOffice puts them.
/// </summary>
/// <remarks>
/// <para>
/// Line breaking says which words are on which line; this says where those lines <em>are</em>. The two
/// compound: a line spacing wrong by one per cent moves the last baseline on a page by a whole line, so
/// the page breaks somewhere else and every page after it differs for reasons unrelated to drawing.
/// </para>
/// <para>
/// LibreOffice renders a paragraph to PDF, the word boxes are grouped into lines, and the distance
/// between consecutive lines is compared with what the layouter computed — for each of the four spacing
/// rules, since they are four separate calculations that happen to agree at single spacing. The
/// horizontal tests do the same for indents and alignment, where the interesting cases are the hanging
/// indent a numbered list is built from and the last line of a justified paragraph, which is the one
/// line justification leaves alone.
/// </para>
/// </remarks>
public sealed class ParagraphLayoutComparisonTests : IDisposable
{
    /// <summary>
    /// How far a position may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A tenth of a point. LibreOffice lays out in hundredths of a millimetre and PDF records points,
    /// so the two disagree in the last digit however right both are; a tenth of a point is a fortieth of
    /// a millimetre and well below anything visible.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// The extra slack a horizontal position gets, in points.
    /// </summary>
    /// <remarks>
    /// A PDF word box's left edge is where the first glyph's ink starts, not where the pen was — the
    /// difference is that glyph's left side bearing, about a tenth of a point for Carlito's lower case.
    /// A quarter of a point covers it and is still tight enough to catch a centring computed from a
    /// line width that is wrong by half a point.
    /// </remarks>
    private const double BearingSlackPoints = 0.25;

    /// <summary>A paragraph long enough to wrap several times at the width used here.</summary>
    private const string Paragraph =
        "Extraction and rendering are separate paths through the library, because extraction is the "
        + "common case and must not pay for fonts, layout or a rasteriser it never uses, and the "
        + "separation is what keeps the common case cheap.";

    private const double PageWidthCm = 21.0;
    private const double MarginCm = 2.0;
    private const double SizePoints = 12;

    private static Length TextWidth => Length.FromMillimetres((PageWidthCm - (2 * MarginCm)) * 10);

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-paragraphs").FullName;

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

    // ------------------------------------------------------------------------ line spacing

    [Theory]
    [InlineData("fo:line-height=\"100%\"", 1.0)]
    [InlineData("fo:line-height=\"150%\"", 1.5)]
    [InlineData("fo:line-height=\"200%\"", 2.0)]
    [InlineData("fo:line-height=\"115%\"", 1.15)]
    public void ProportionalSpacingPutsBaselinesWhereWriterDoes(string attribute, double proportion)
    {
        // The mode nearly every document uses, and the one where the arithmetic is a multiplication
        // rather than a comparison — so an error scales with the proportion and is invisible at 100%.
        AssertPitch(attribute, LineSpacingRule.Multiple(proportion));
    }

    [Theory]
    [InlineData("style:line-height-at-least=\"0.5cm\"")]
    [InlineData("style:line-height-at-least=\"1cm\"")]
    public void AtLeastSpacingTakesWhicheverIsLarger(string attribute)
    {
        // Two cases in one rule: half a centimetre is less than a 12 pt line needs, so the natural
        // height wins; one centimetre is more, so the declared value does. A reader that always took
        // the declared value would pass the second and fail the first.
        Length declared = attribute.Contains("0.5cm", StringComparison.Ordinal)
            ? Length.FromMillimetres(5)
            : Length.FromMillimetres(10);

        AssertPitch(attribute, LineSpacingRule.AtLeast(declared));
    }

    [Theory]
    [InlineData("fo:line-height=\"0.7cm\"")]
    [InlineData("fo:line-height=\"1.2cm\"")]
    public void ExactSpacingIsHonouredEvenWhenItIsTooSmall(string attribute)
    {
        // ODF spells exact spacing as a length in the same attribute a percentage goes in, so the two
        // are told apart by their units rather than by their names. The 0.7 cm case is shorter than the
        // text wants and still has to be honoured — the whole point of the mode.
        Length declared = attribute.Contains("0.7cm", StringComparison.Ordinal)
            ? Length.FromMillimetres(7)
            : Length.FromMillimetres(12);

        AssertPitch(attribute, LineSpacingRule.Exactly(declared));
    }

    [Theory]
    [InlineData("style:line-spacing=\"0.2cm\"")]
    [InlineData("style:line-spacing=\"0.5cm\"")]
    public void LeadingIsAddedToTheNaturalHeight(string attribute)
    {
        Length declared = attribute.Contains("0.2cm", StringComparison.Ordinal)
            ? Length.FromMillimetres(2)
            : Length.FromMillimetres(5);

        AssertPitch(attribute, LineSpacingRule.PlusLeading(declared));
    }

    /// <summary>
    /// Renders a paragraph with a spacing attribute and compares the pitch with the layouter's.
    /// </summary>
    private void AssertPitch(string spacingAttribute, LineSpacingRule rule)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);

        LaidOutParagraph laid = layouter.Layout(
            Paragraph,
            ParagraphFormat.Default with { LineSpacing = rule },
            Length.FromPoints(SizePoints),
            TextWidth,
            language: "en-GB");

        laid.Lines.Count.ShouldBeGreaterThan(2, "the paragraph has to wrap for a pitch to exist");

        List<double> tops = RenderedLineTops(
            $"fo:text-align=\"start\" fo:text-indent=\"0cm\" {spacingAttribute}");
        tops.Count.ShouldBe(
            laid.Lines.Count,
            $"line count differs for {spacingAttribute}");

        // The pitch, not the absolute position: where the first baseline sits also depends on the
        // paragraph's space-before and on how the renderer aligns the first line in the text area,
        // while the pitch is the spacing rule alone.
        for (int i = 1; i < tops.Count; i++)
        {
            double rendered = tops[i] - tops[i - 1];
            double computed = (laid.Lines[i].Top - laid.Lines[i - 1].Top).Points;

            Math.Abs(rendered - computed).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{spacingAttribute}: pitch between lines {i} and {i + 1} is {rendered:F3} pt "
                + $"rendered and {computed:F3} pt computed");
        }
    }

    // -------------------------------------------------------------------- horizontal placement

    [Theory]
    [InlineData("fo:margin-left=\"0cm\" fo:text-indent=\"0cm\"", 0, 0)]
    [InlineData("fo:margin-left=\"2cm\" fo:text-indent=\"0cm\"", 20, 0)]
    [InlineData("fo:margin-left=\"2cm\" fo:text-indent=\"1cm\"", 20, 10)]
    [InlineData("fo:margin-left=\"2cm\" fo:text-indent=\"-1cm\"", 20, -10)]
    public void IndentsPutLinesWhereWriterDoes(
        string attributes, double startIndentMm, double firstLineIndentMm)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);

        ParagraphFormat format = ParagraphFormat.Default with
        {
            StartIndent = Length.FromMillimetres(startIndentMm),
            FirstLineIndent = Length.FromMillimetres(firstLineIndentMm),
        };

        LaidOutParagraph laid = layouter.Layout(
            Paragraph, format, Length.FromPoints(SizePoints), TextWidth, language: "en-GB");

        List<double> lefts = RenderedLineLefts($"fo:text-align=\"start\" {attributes}");
        lefts.Count.ShouldBe(laid.Lines.Count, $"line count differs for {attributes}");

        // The negative first-line indent is the case that matters: a hanging indent is how every
        // numbered list puts its number to the left of its text, and a reader that clamps it at zero
        // makes every list in every document start in the wrong place.
        double marginLeft = Length.FromMillimetres(MarginCm * 10).Points;
        for (int i = 0; i < lefts.Count; i++)
        {
            double computed = marginLeft + laid.Lines[i].Left.Points;
            Math.Abs(lefts[i] - computed).ShouldBeLessThanOrEqualTo(
                TolerancePoints + BearingSlackPoints,
                $"{attributes}: line {i + 1} starts at {lefts[i]:F3} pt rendered "
                + $"and {computed:F3} pt computed");
        }
    }

    [Theory]
    [InlineData("start", TextAlignment.Start)]
    [InlineData("end", TextAlignment.End)]
    [InlineData("center", TextAlignment.Centre)]
    public void AlignmentPutsLinesWhereWriterDoes(string odfAlignment, TextAlignment alignment)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);

        LaidOutParagraph laid = layouter.Layout(
            Paragraph,
            ParagraphFormat.Default with { Alignment = alignment },
            Length.FromPoints(SizePoints),
            TextWidth,
            language: "en-GB");

        List<double> lefts = RenderedLineLefts(
            $"fo:text-indent=\"0cm\" fo:text-align=\"{odfAlignment}\"");
        lefts.Count.ShouldBe(laid.Lines.Count);

        // Centring and end-alignment are computed from the line's own width, so they are the assertion
        // that the measurement is right as well as the placement: a line measured a point too wide is
        // centred half a point too far left.
        double marginLeft = Length.FromMillimetres(MarginCm * 10).Points;
        for (int i = 0; i < lefts.Count; i++)
        {
            double computed = marginLeft + laid.Lines[i].Left.Points;
            Math.Abs(lefts[i] - computed).ShouldBeLessThanOrEqualTo(
                TolerancePoints + BearingSlackPoints,
                $"{odfAlignment}: line {i + 1} starts at {lefts[i]:F3} pt rendered "
                + $"and {computed:F3} pt computed");
        }
    }

    // --------------------------------------------------------------------------- unit checks

    [Fact]
    public void ProportionalSpacingsExtraHeightGoesAboveTheText()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);
        Length em = Length.FromPoints(SizePoints);

        LaidOutParagraph single = layouter.Layout(Paragraph, ParagraphFormat.Default, em, TextWidth);
        LaidOutParagraph doubled = layouter.Layout(
            Paragraph,
            ParagraphFormat.Default with { LineSpacing = LineSpacingRule.Multiple(2.0) },
            em,
            TextWidth);

        // The asymmetry, asserted directly. Doubling the spacing doubles the box and pushes the
        // baseline down inside it by exactly the height that was added — it does not leave the text at
        // the top with a gap underneath, which is what a naive implementation produces and what makes
        // a double-spaced document's first line sit too high.
        Math.Abs(doubled.Lines[0].Height.Emu - (single.Lines[0].Height.Emu * 2))
            .ShouldBeLessThanOrEqualTo(2);
        Math.Abs((doubled.Lines[0].Baseline - single.Lines[0].Baseline).Emu
                 - single.Lines[0].Height.Emu)
            .ShouldBeLessThanOrEqualTo(2);
    }

    [Fact]
    public void ExactSpacingNeverPutsTheBaselineAboveItsOwnBox()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);

        // A box far shorter than the ascent. The text has to be clipped from below rather than climb
        // into the paragraph above, so the baseline is clamped to the box — otherwise a form with tight
        // rows overwrites whatever precedes it.
        LaidOutParagraph laid = layouter.Layout(
            Paragraph,
            ParagraphFormat.Default with { LineSpacing = LineSpacingRule.Exactly(Length.FromPoints(4)) },
            Length.FromPoints(SizePoints),
            TextWidth);

        foreach (LineBox line in laid.Lines)
        {
            line.Baseline.ShouldBeGreaterThan(Length.Zero);
            line.Baseline.ShouldBeLessThanOrEqualTo(line.Height);
        }
    }

    [Fact]
    public void ContextualSpacingNeedsBothParagraphsToAskForIt()
    {
        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();
        ParagraphLayouter layouter = new(face);
        Length em = Length.FromPoints(SizePoints);

        ParagraphFormat listItem = ParagraphFormat.Default with
        {
            SpaceBefore = Length.FromPoints(10),
            HasContextualSpacing = true,
        };
        ParagraphFormat prose = listItem with { HasContextualSpacing = false };

        // Between two list items the space is suppressed, which is what keeps a bulleted list tight.
        layouter.Layout("Item", listItem, em, TextWidth, follows: listItem)
            .SpaceBefore.ShouldBe(Length.Zero);

        // But the list's first item, following prose, keeps its space — otherwise the list would run
        // straight into the paragraph that introduces it.
        layouter.Layout("Item", listItem, em, TextWidth, follows: prose)
            .SpaceBefore.ShouldBe(Length.FromPoints(10));

        // And the first paragraph in a document has nothing above it to collapse against.
        layouter.Layout("Item", listItem, em, TextWidth)
            .SpaceBefore.ShouldBe(Length.FromPoints(10));
    }

    // ------------------------------------------------------------------------- the reference

    /// <summary>
    /// The tops of the rendered lines, for a paragraph carrying extra properties.
    /// </summary>
    /// <remarks>
    /// The extra properties are spliced into <c>style:paragraph-properties</c> whole, and the template
    /// therefore states no alignment or indent of its own — a duplicated attribute makes the document
    /// malformed, and LibreOffice's response to malformed input is to convert nothing and exit zero, so
    /// it fails as "LibreOffice produced no output" rather than as a bad document.
    /// </remarks>
    private List<double> RenderedLineTops(string extraProperties)
        => [.. RenderedLines(extraProperties).Select(l => l.Top)];

    private List<double> RenderedLineLefts(string extraProperties)
        => [.. RenderedLines(extraProperties).Select(l => l.Left)];

    private List<(double Top, double Left)> RenderedLines(string extraProperties)
    {
        string key = extraProperties.GetHashCode(StringComparison.Ordinal)
            .ToString("x8", CultureInfo.InvariantCulture);

        string source = Path.Combine(_workDirectory, $"paragraph-{key}.fodt");
        File.WriteAllText(source, Document(extraProperties));

        string pdf = _libreOffice.ConvertToPdf(source, _workDirectory);
        return LinesOf(pdf);
    }

    /// <summary>
    /// The words grouped into lines by their vertical position.
    /// </summary>
    /// <remarks>
    /// A tolerance of a point, which is safe because the smallest pitch tested here is seven
    /// millimetres — and unsafe for anything tighter, which is why the exact-spacing cases stay well
    /// above it.
    /// </remarks>
    private static List<(double Top, double Left)> LinesOf(string pdfPath)
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

        List<(double Top, double Left)> lines = [];
        double currentTop = double.NaN;

        foreach (Match match in Regex.Matches(
                     output, "<word xMin=\"([0-9.]+)\" yMin=\"([0-9.]+)\""))
        {
            double left = double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
            double top = double.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture);

            if (double.IsNaN(currentTop) || Math.Abs(top - currentTop) > 1.0)
            {
                lines.Add((top, left));
                currentTop = top;
            }
        }

        return lines;
    }

    /// <summary>
    /// One paragraph on a page of known width, with the properties under test spliced in.
    /// </summary>
    /// <remarks>
    /// No hyphenation, and the page's own margins are the only thing between the text and the sheet, so
    /// the rendered left edge is the margin plus whatever the paragraph's own indents add.
    /// </remarks>
    private static string Document(string extraProperties)
    {
        string size = SizePoints.ToString(CultureInfo.InvariantCulture);
        string escaped = System.Security.SecurityElement.Escape(Paragraph);

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
                                       fo:hyphenate="false" {extraProperties}/>
           <style:text-properties style:font-name="Carlito" fo:font-family="Carlito"
                                  fo:font-size="{size}pt" fo:language="en" fo:country="GB"/>
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
