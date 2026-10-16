using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A section's <c>w:pgBorders</c> is drawn round the page, which until now nothing drew.
/// </summary>
/// <remarks>
/// <para>
/// It was not implemented at all — <c>pgBorders</c> appeared nowhere in the tree — and no gate
/// column could see it, because a border adds no words and no pages. Seven of the 272 corpus DOCX
/// declare one and every one of the seven passes the word gate;
/// <c>Case-Study-Heathrow-Airport.docx</c> is the witness, where this one rectangle is 11.55 of the
/// 14.70 <c>|ink|%</c> that put it at the head of the words track's per-page ink ranking against
/// 26.2.4.2.
/// </para>
/// <para>
/// <strong>The numbers below are measured off 26.2.4.2's own PDF of that document, not derived from
/// the specification.</strong> A4 595.304 × 841.89 with <c>w:sz="36"</c>, <c>w:space="15"</c>,
/// <c>w:offsetFrom="page"</c> and <c>w:shadow="1"</c>, its content stream holds four
/// <c>4.5 w … S</c> strokes at <c>0.2235 0.3961 0.2 RG</c> with centrelines at 17.25, 573.60, 21.69
/// and 17.25, and two black <c>re f*</c> rectangles for the shadow. The one thing that cannot be
/// guessed is that the shadow <em>shrinks the box</em> — the right and bottom edges come in by its
/// width instead of the shadow hanging off the paper — and that is what the assertions below are
/// for.
/// </para>
/// </remarks>
public sealed class PageBorderTests
{
    private const double Tolerance = 0.02;

    /// <summary>The four sides land where 26.2.4.2 puts them, to a fiftieth of a point.</summary>
    [Fact]
    public void TheFourSidesLandWhereLibreOfficeDrawsThem()
    {
        List<DrawnStroke> strokes = [.. Drawn(Shadow: true).StrokedPaths
            .Where(s => IsGreen(s.Stroke.Paint))];

        strokes.Count.ShouldBe(4);
        strokes.ShouldAllBe(s => s.Stroke.Width == Length.FromPoints(4.5));

        // Top and bottom: horizontal, spanning the box, at space + width/2 from the top and at the
        // same distance from the bottom *plus the shadow's own width*.
        Horizontal(strokes, 17.25);
        Horizontal(strokes, 841.89 - 15 - 4.5 - 2.25);

        // Left and right, the same rule mirrored.
        Vertical(strokes, 17.25);
        Vertical(strokes, 595.304 - 15 - 4.5 - 2.25);
    }

    /// <summary>
    /// The shadow is two black bars offset down and to the right by its own width.
    /// </summary>
    [Fact]
    public void TheShadowIsTwoBarsOffsetByItsOwnWidth()
    {
        List<DrawnFill> black = [.. Drawn(Shadow: true).FilledPaths
            .Where(f => IsBlack(f.Paint))];

        black.Count.ShouldBe(2);

        DrawnFill bottom = black.MaxBy(f => f.Bounds.Y.Points);
        DrawnFill right = black.MinBy(f => f.Bounds.Y.Points);

        bottom.Bounds.Height.Points.ShouldBe(4.5, Tolerance);
        bottom.Bounds.Y.Points.ShouldBe(841.89 - 15 - 4.5, Tolerance);
        bottom.Bounds.X.Points.ShouldBe(15 + 4.5, Tolerance);

        right.Bounds.Width.Points.ShouldBe(4.5, Tolerance);
        right.Bounds.X.Points.ShouldBe(595.304 - 15 - 4.5, Tolerance);
    }

    /// <summary>Without the shadow the box uses the whole inset, and nothing black is drawn.</summary>
    /// <remarks>
    /// The control for the assertion above: if the shrink were unconditional the right and bottom
    /// sides would sit 4.5 pt inside where Word puts them on every unshadowed border in the corpus,
    /// which is five of the seven.
    /// </remarks>
    [Fact]
    public void WithoutTheShadowTheBoxKeepsTheWholeInset()
    {
        DrawnPage page = Drawn(Shadow: false);

        page.FilledPaths.Count(f => IsBlack(f.Paint)).ShouldBe(0);

        List<DrawnStroke> strokes = [.. page.StrokedPaths
            .Where(s => IsGreen(s.Stroke.Paint))];

        strokes.Count.ShouldBe(4);
        Vertical(strokes, 595.304 - 15 - 2.25);
        Horizontal(strokes, 841.89 - 15 - 2.25);
    }

    /// <summary>A section declaring no border draws none, which is nearly every section.</summary>
    [Fact]
    public void ASectionWithNoBorderDrawsNone()
    {
        Drawn(Shadow: false, borders: false).StrokedPaths
            .Count(s => IsGreen(s.Stroke.Paint)).ShouldBe(0);
    }

    /// <summary>
    /// An <c>offsetFrom="text"</c> border stands its own space clear of the text — and the text
    /// does not move.
    /// </summary>
    /// <remarks>
    /// The box's outer edge is <c>margin − space − width</c> from the paper, not <c>margin − space</c>:
    /// 26.2.4.2 puts the left stroke's centreline at 54.75 on this fixture, which is 52.5 + 2.25.
    /// Measured on <c>pgbC-text-plain.docx</c> in <c>probes/words-pgborder-r68</c>, whose ODT, DOC
    /// and RTF conversions all give the same four figures.
    /// </remarks>
    [Fact]
    public void ATextOffsetBorderStandsOffTheTextRatherThanThePaper()
    {
        List<DrawnStroke> strokes = [.. Drawn(Shadow: false, offsetFrom: "text").StrokedPaths
            .Where(s => IsGreen(s.Stroke.Paint))];

        strokes.Count.ShouldBe(4);
        Vertical(strokes, 72 - 15 - 4.5 + 2.25);
        Horizontal(strokes, 72 - 15 - 4.5 + 2.25);
        Vertical(strokes, 595.304 - (72 - 15 - 4.5) - 2.25);
        Horizontal(strokes, 841.89 - (72 - 15 - 4.5) - 2.25);
    }

    /// <summary>
    /// An absent <c>w:offsetFrom</c> means <em>text</em>, which is the opposite of the obvious reading.
    /// </summary>
    /// <remarks>
    /// <c>PageBordersHandler</c>'s constructor initialises <c>m_eOffsetFrom</c> to
    /// <c>BorderOffsetFrom::Text</c> while its <c>default:</c> arm for a stated value falls to
    /// <c>page</c>, so "absent" and "unrecognised" mean opposite things
    /// (<c>sw/source/writerfilter/dmapper/PageBordersHandler.cxx</c>:34-80). Measured: a fixture with
    /// no <c>w:offsetFrom</c> renders on 26.2.4.2 at the same four positions as one stating
    /// <c>text</c>. All seven corpus documents state <c>page</c>, so no corpus figure can see this.
    /// </remarks>
    [Fact]
    public void AnAbsentOffsetFromMeansTheTextAndNotThePaper()
    {
        List<DrawnStroke> strokes = [.. Drawn(Shadow: false, offsetFrom: null).StrokedPaths
            .Where(s => IsGreen(s.Stroke.Paint))];

        strokes.Count.ShouldBe(4);
        Vertical(strokes, 72 - 15 - 4.5 + 2.25);
    }

    /// <summary>
    /// A side that draws nothing leaves its edge of the box at the text area, not at the paper.
    /// </summary>
    /// <remarks>
    /// Measured on a fixture bordered left and right only: 26.2.4.2 runs the two verticals from
    /// y = 72 to y = 769.89 — the top and bottom margins — rather than the length of the sheet.
    /// <c>091_Business_Case_Template_Complete_Guide</c> is the corpus document that asks it; it is
    /// one of the seven and the only one of them with two sides rather than four.
    /// </remarks>
    [Fact]
    public void ASideThatDrawsNothingLeavesTheBoxAtTheTextArea()
    {
        List<DrawnStroke> strokes = [.. Drawn(Shadow: false, sides: ["left", "right"]).StrokedPaths
            .Where(s => IsGreen(s.Stroke.Paint))];

        strokes.Count.ShouldBe(2);
        strokes.ShouldAllBe(s => s.Bounds.Width == Length.Zero);

        // The verticals span the text area's height, 72 pt of margin at each end.
        strokes.ShouldAllBe(s => Math.Abs(s.Bounds.Y.Points - 72) < Tolerance);
        strokes.ShouldAllBe(s => Math.Abs(s.Bounds.Bottom.Points - (841.89 - 72)) < Tolerance);
    }

    /// <summary>
    /// An art border draws nothing, which is the reference's behaviour rather than a gap in ours.
    /// </summary>
    /// <remarks>
    /// 165 of <c>ST_Border</c>'s values name a repeating device rather than a line, and
    /// <c>lcl_convertBorderStyleFromToken</c> answers <c>none</c> for every one of them. Measured on
    /// two fixtures, <c>apples</c> and <c>basicWhiteDashes</c>: 26.2.4.2 draws no stroke, embeds no
    /// image, and puts the first line of text exactly where an unbordered page puts it.
    /// </remarks>
    [Theory]
    [InlineData("apples")]
    [InlineData("basicWhiteDashes")]
    public void AnArtBorderDrawsNothing(string art)
    {
        DrawnPage page = Drawn(Shadow: false, style: art);

        page.StrokedPaths.Count(s => IsGreen(s.Stroke.Paint)).ShouldBe(0);
        page.FilledPaths.Count(f => IsBlack(f.Paint)).ShouldBe(0);
    }

    /// <summary>
    /// <c>w:display</c> changes nothing, because the reference does not read it.
    /// </summary>
    /// <remarks>
    /// <c>PageBordersHandler</c> turns it into <c>m_eBorderApply</c> and
    /// <c>SectionPropertyMap::ApplyBorderToPageStyles</c> then declares the parameter
    /// <c>BorderApply /*eBorderApply*/</c> and never touches it
    /// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:649-721). Measured on two four-page
    /// fixtures declaring <c>firstPage</c> and <c>notFirstPage</c>: 26.2.4.2 draws the border on all
    /// four pages of both. The DOC reader is a different matter — <c>pgbApplyTo</c> really is
    /// honoured there — which is why <c>PageBorderDisplay</c> still exists.
    /// </remarks>
    [Theory]
    [InlineData("firstPage")]
    [InlineData("notFirstPage")]
    public void TheDisplayAttributeDoesNotChangeWhichPagesCarryTheBorder(string display)
        => Drawn(Shadow: false, display: display).StrokedPaths
            .Count(s => IsGreen(s.Stroke.Paint)).ShouldBe(4);

    /// <summary>
    /// The DOC, ODT and RTF readers reach the same rectangle the DOCX reader does.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The point of the fixtures rather than of the assertion: <c>w:pgBorders</c> was implemented in
    /// the DOCX reader alone, and the corpus holds <b>no ODF and no RTF at all</b>, so no corpus
    /// figure could ever have shown the other three readers were blind to it. The three files are
    /// 26.2.4.2's own conversions of one bordered DOCX — <c>page-border-source.docx</c>, A4 with
    /// <c>w:sz="36"</c>, <c>w:space="15"</c>, <c>w:offsetFrom="page"</c> and <c>w:shadow="1"</c> —
    /// except the RTF, which is hand-written because LibreOffice's own RTF <em>writer</em> loses the
    /// page border and emits a paragraph box instead.
    /// </para>
    /// <para>
    /// ODF is the one that also moves the text. It states the box as the page style's own —
    /// <c>fo:margin</c> to the border and <c>fo:padding</c> from the border to the text — so a reader
    /// taking <c>fo:margin-left</c> for the text's margin starts every line 57 pt too far left on this
    /// fixture. The last assertion is that one.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("page-border-doc.doc")]
    [InlineData("page-border-odt.odt")]
    [InlineData("page-border-rtf.rtf")]
    public void EveryWordProcessingReaderCarriesThePageBorder(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            ((IPaginatedDocument)document).Layout()[0].Draw(sink);
        }

        DrawnPage page = sink.Pages[0];

        List<DrawnStroke> strokes = [.. page.StrokedPaths.Where(s => IsGreen(s.Stroke.Paint))];
        strokes.Count.ShouldBe(4);
        strokes.ShouldAllBe(s => s.Stroke.Width == Length.FromPoints(4.5));

        Horizontal(strokes, 17.25);
        Vertical(strokes, 17.25);
        Horizontal(strokes, 841.89 - 15 - 4.5 - 2.25);
        Vertical(strokes, 595.304 - 15 - 4.5 - 2.25);

        // The shadow, which every one of the three states differently: DOC from the right BRC's
        // fShadow, ODF from style:shadow's own offset, RTF from \brdrsh on the right side.
        page.FilledPaths.Count(f => IsBlack(f.Paint)).ShouldBe(2);

        // And the text is where an unbordered page would put it: 26.2.4.2 draws the first line of
        // all four conversions at x = 72.10, one inch in, border or no border.
        page.Runs.ShouldNotBeEmpty();
        page.Runs[0].Origin.X.Points.ShouldBe(72, 0.5);
    }

    private static readonly Colour Green = new(0x39, 0x65, 0x33);

    private static bool IsGreen(Paint paint) => paint is SolidPaint solid && solid.Colour == Green;

    private static bool IsBlack(Paint paint) => paint is SolidPaint solid && solid.Colour == Colour.Black;

    private static void Horizontal(List<DrawnStroke> strokes, double y)
        => strokes.ShouldContain(
            s => s.Bounds.Height == Length.Zero
                 && Math.Abs(s.Bounds.Y.Points - y) < Tolerance,
            $"a horizontal side at y = {y}");

    private static void Vertical(List<DrawnStroke> strokes, double x)
        => strokes.ShouldContain(
            s => s.Bounds.Width == Length.Zero
                 && Math.Abs(s.Bounds.X.Points - x) < Tolerance,
            $"a vertical side at x = {x}");

    private static DrawnPage Drawn(
        bool Shadow,
        bool borders = true,
        string? offsetFrom = "page",
        string style = "single",
        string? display = null,
        string[]? sides = null)
    {
        RecordingDrawingSink sink = new();

        byte[] package = Package(Shadow, borders, offsetFrom, style, display, sides);

        using (DocumentSource source = DocumentSource.FromBytes(package, "b.docx"))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages[0].Draw(sink);
        }

        return sink.Pages[0];
    }

    /// <summary>
    /// One A4 page with one word on it, and the section properties the reference was measured with.
    /// </summary>
    private static byte[] Package(
        bool shadow,
        bool borders,
        string? offsetFrom = "page",
        string style = "single",
        string? display = null,
        string[]? sides = null)
    {
        string side(string name) =>
            $"""<w:{name} w:val="{style}" w:sz="36" w:space="15" w:color="396533"{(shadow ? " w:shadow=\"1\"" : "")}/>""";

        string attributes =
            (offsetFrom is null ? "" : $" w:offsetFrom=\"{offsetFrom}\"")
            + (display is null ? "" : $" w:display=\"{display}\"");

        string stated = string.Concat((sides ?? ["top", "left", "bottom", "right"]).Select(side));

        string pgBorders = borders
            ? $"""
               <w:pgBorders{attributes}>
                 {stated}
               </w:pgBorders>
               """
            : "";

        string document =
            $"""
             <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
             <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
               <w:body>
                 <w:p><w:r><w:t>Bordered</w:t></w:r></w:p>
                 <w:sectPr>
                   <w:pgSz w:w="11906" w:h="16838"/>
                   <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                            w:header="708" w:footer="708" w:gutter="0"/>
                   {pgBorders}
                 </w:sectPr>
               </w:body>
             </w:document>
             """;

        using MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml",
                """
                <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
                <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
                  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
                  <Default Extension="xml" ContentType="application/xml"/>
                  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
                </Types>
                """);
            Write(archive, "_rels/.rels",
                """
                <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
                <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
                  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
                </Relationships>
                """);
            Write(archive, "word/document.xml", document);
        }

        return stream.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        entry.Write(Encoding.UTF8.GetBytes(content));
    }
}
