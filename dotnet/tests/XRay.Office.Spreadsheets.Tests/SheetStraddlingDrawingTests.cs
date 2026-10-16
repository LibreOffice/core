using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A drawing wider than the page it starts on is printed on every page it reaches.
/// </summary>
/// <remarks>
/// <para>
/// A drawing does not belong to a page. <c>ScOutputData::PrePrintDrawingLayer</c>
/// (<c>sc/source/ui/view/output3.cxx:40-104</c>) sets a map-mode offset of minus the width of the
/// columns and the height of the rows before the page's first, and <c>PrintDrawingLayer</c>
/// (<c>:138</c>) then paints the <em>whole</em> drawing page through it. So a picture straddling a
/// break appears on both pages, cut, and a renderer that anchors it to the page holding its
/// top-left cell loses the second half.
/// </para>
/// <para>
/// <strong>What cuts it is the page's own cell block and not the paper</strong>, which this class
/// asserted the wrong way round for four rounds. The rectangle <c>PrePrintDrawingLayer</c> hands to
/// <c>BeginDrawLayers</c> reaches the paint window (<c>SdrPageWindow::PrepareRedraw</c>,
/// <c>svx/source/svdraw/sdrpagewindow.cxx:212-224</c>), the <c>DisplayInfo</c> (<c>:347</c>,
/// <c>:404</c>) and finally the device — <c>pOutDev-&gt;IntersectClipRegion(rRedrawArea)</c>,
/// <c>svx/source/sdr/contact/objectcontactofpageview.cxx:163-171</c>. On this fixture 26.2.4.2
/// writes <c>q 50.4 749.48 444.756 38.409 re W* n</c> round the box on each of the two pages: three
/// columns of 148.252 pt, the block, where the paper would have allowed 494.9.
/// </para>
/// <para>
/// Measured on <c>Air_Boss_Master_List.xlsx</c>, whose note box is anchored in column E and
/// straddles the column break: LibreOffice prints its left half on page 1 and its right half on
/// page 3, so page 3 carried 15 words against our none — 514 words against 527, and 529 now.
/// Checked against LibreOffice 24.2.7.2's own PDF for the fixture: page 1 ends the box at
/// "STRADDLINGT" and page 2 opens with the rest of it.
/// </para>
/// </remarks>
public sealed class SheetStraddlingDrawingTests
{
    private static IReadOnlyList<DrawnPage> Draw(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages;
    }

    private static PlacedDrawingSink Place(string name, int page)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        PlacedDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[page].Draw(sink);
        return sink;
    }

    private static string TextOf(DrawnPage page)
        => string.Join(" ", page.Runs.Select(run => run.Text));

    [Fact]
    public void AShapeStraddlingABreakIsDrawnOnBothPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-drawing-across-break.xlsx");
        pages.Count.ShouldBe(2, "six wide columns take two pages side by side");

        // Anchored in column B, so its head is on the first page and its tail — six inches to the
        // right of the anchor — lands past the break on the second.
        TextOf(pages[0]).ShouldContain("STRADDLINGHEAD");
        TextOf(pages[1]).ShouldContain("STRADDLINGTAIL");
    }

    /// <summary>
    /// The control: the cells still land on the page their own column band is on.
    /// </summary>
    /// <remarks>
    /// A rule that put every drawing on every page would satisfy the test above, and would also
    /// have to have broken the ordinary placement to do it. This pins the placement.
    /// </remarks>
    [Fact]
    public void TheCellsAreStillSplitBetweenTheTwoPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-drawing-across-break.xlsx");

        TextOf(pages[0]).ShouldContain("Alpha");
        TextOf(pages[0]).ShouldNotContain("Foxtrot");
        TextOf(pages[1]).ShouldContain("Foxtrot");
        TextOf(pages[1]).ShouldNotContain("Alpha");
    }

    /// <summary>
    /// The half that lands on the other page is cut off at the block's edge, not at the paper's.
    /// </summary>
    /// <remarks>
    /// The clip is what makes the two halves complementary. Without it the tail is painted from the
    /// left margin of the second page as well, over the cells the reference leaves clear — worth
    /// 89.20 of summed <c>|ink|%</c> across the 74 sheets renderings it moves, and worth nothing at
    /// all to the gate, whose page and alphanumeric counts are identical on every one of the 74.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(1)]
    public void TheStraddlingShapeIsCutAtTheBlockAndNotAtThePaper(int page)
    {
        PlacedDrawingSink sink = Place("sheet-drawing-across-break.xlsx", page);

        DocRect clip = sink.Clips.ShouldHaveSingleItem();
        clip.Left.Points.ShouldBe(50.4, 0.2);
        clip.Right.Points.ShouldBe(495.16, 0.2);

        // And it is doing work: the box itself reaches past that edge on the page that starts it
        // and begins before it on the page that finishes it.
        DocRect ink = sink.Ink;
        if (page == 0) ink.Right.Points.ShouldBeGreaterThan(clip.Right.Points);
        else ink.Left.Points.ShouldBeLessThan(clip.Left.Points);
    }

    /// <summary>
    /// A drawing that stays inside its page's block is drawn without a clip at all.
    /// </summary>
    /// <remarks>
    /// The clip is emitted only where it removes ink, for the same reason a picture's crop clip is:
    /// an unconditional one would put a <c>q</c>/<c>W n</c>/<c>Q</c> pair round every picture in the
    /// corpus and change every one of those renderings to no visible effect. This is the control
    /// that keeps it conditional.
    /// </remarks>
    [Fact]
    public void ADrawingInsideItsBlockIsNotClipped()
    {
        Place("sheet-chart-face-stated.xlsx", 0).Clips.ShouldBeEmpty();
    }

    /// <summary>
    /// The block clip cuts the shape's ink and leaves the shape's own glyphs in the text layer.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the half of the rule <see cref="PlacedDrawingSink"/> cannot see, because that sink
    /// does not override <see cref="IDrawingSink.ClipPathKeepingText"/> and so records both kinds
    /// alike. Two rounds wrote this clip independently and git merged both cleanly, one of them
    /// with the text-hiding <see cref="IDrawingSink.ClipPath"/>; nothing in the suite would have
    /// told them apart.
    /// </para>
    /// <para>
    /// <strong>Measured at 26.2.4.2 rather than chosen.</strong> On an authored probe
    /// (<c>probes/clip-seats-r97/make-shape-text-probe.py</c>) whose rectangle carries a
    /// right-aligned word landing 52 pt past the block's right edge and still on the paper, the
    /// reference cuts the rectangle's fill at the block and writes the word anyway —
    /// <c>568.545 759.089 Td … Tj</c> inside the page's own <c>50.4 58.28 466.214 729.609 re W* n</c>,
    /// and <c>pdftotext</c> reads it. Over the 947 banked reference renderings, 150 pages in 61
    /// documents carry drawing-layer text lying wholly outside the drawing-layer clip that governs
    /// it, 5865 glyphs in all. Hiding them instead costs <strong>3150 alphanumeric characters over
    /// 56 of the 74 sheets renderings this clip touches</strong>, and moves six of them out of the
    /// gate's own <c>max(2%, 15)</c> glyph band on balance — eight leave it and two enter, and the
    /// two that enter do so because we draw more glyphs there than the reference does.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheBlockClipKeepsTheShapesOwnGlyphs()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-drawing-across-break.xlsx"));

        ClipKindSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        sink.Keeping.ShouldBe(2, customMessage: "one clip on each of the two pages");
        sink.Hiding.ShouldBe(0, customMessage: "and neither of them removes the shape's words");
    }

    /// <summary>Counts the two kinds of clip apart, which no shared sink does.</summary>
    private sealed class ClipKindSink : IDrawingSink
    {
        public int Hiding { get; private set; }

        public int Keeping { get; private set; }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) => Hiding++;

        public void ClipPathKeepingText(GraphicsPath path, FillRule rule = FillRule.NonZero)
            => Keeping++;

        public void BeginPage(DocSize size) { }

        public void EndPage() { }

        public void Save() { }

        public void Restore() { }

        public void Transform(AffineTransform transform) { }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero) { }

        public void StrokePath(GraphicsPath path, Stroke stroke) { }

        public void DrawGlyphRun(GlyphRun run, Paint paint) { }

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0) { }

        public void BeginTransparencyGroup(double opacity) { }

        public void EndTransparencyGroup() { }
    }
}
