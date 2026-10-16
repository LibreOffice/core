using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell's ink is cut at the edge of the columns the page prints, not only at the room its text
/// was given.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::AdjustAreaParamClipRect</c> (<c>sc/source/ui/view/output2.cxx:2928-2954</c>)
/// trims the output area to <c>[mnScrX, mnScrX + mnScrW]</c> and <em>sets</em> <c>mbLeftClip</c>
/// or <c>mbRightClip</c> when it has to. <c>LayoutStrings</c> reads <c>bHClip</c> from those flags
/// afterwards (<c>:2038-2039</c>) and <c>DrawEditStandard</c> ors them into its own <c>bClip</c>
/// (<c>:3239</c>), so the trim turns a clip <em>on</em> for a cell whose text fitted the room it
/// borrowed perfectly well. A merge wider than the page's columns is the commonest way in.
/// </para>
/// <para>
/// Every figure here is read off LibreOffice 26.2.4.2's own content stream for
/// <c>sheet-clip-block.fods</c>, whose header carries the full table. The block on page 1 runs
/// 56.693 to 481.890 pt.
/// </para>
/// <para>
/// The defect this pins was invisible to the word gate and to every metric this project had:
/// a clip removes ink and leaves the glyphs in the PDF's text layer, so both renderers extracted
/// the same words while only one of them cut the page. It showed as description text running 40 pt
/// past where the reference cut it mid-glyph on <c>fse_identification_form.xlsx</c> page 1, and as
/// seven <c>kein WLAN</c> rows overhanging a table border on
/// <c>Infotabelle_WLAN im Flugzeug.xlsx</c> page 2 — a document that passed the gate throughout.
/// </para>
/// </remarks>
public sealed class SheetBlockClipTests
{
    private static List<PlacedDrawingSink> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-clip-block.fods"));

        List<PlacedDrawingSink> pages = [];
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages)
        {
            PlacedDrawingSink sink = new();
            page.Draw(sink);
            pages.Add(sink);
        }

        return pages;
    }

    /// <summary>Every clip on a page as a left–right pair in points, left to right.</summary>
    private static IReadOnlyList<(double Left, double Right)> Clips(PlacedDrawingSink page)
        => [.. page.Clips
            .Select(clip => (Math.Round(clip.Left.Points, 2), Math.Round(clip.Right.Points, 2)))
            .OrderBy(pair => pair.Item1)
            .ThenBy(pair => pair.Item2)];

    private static void ShouldMatch(
        IReadOnlyList<(double Left, double Right)> actual,
        params (double Left, double Right)[] expected)
    {
        actual.Count.ShouldBe(expected.Length);
        for (int at = 0; at < expected.Length; at++)
        {
            actual[at].Left.ShouldBe(expected[at].Left, 0.1);
            actual[at].Right.ShouldBe(expected[at].Right, 0.1);
        }
    }

    [Fact]
    public void EveryCellThatOverrunsThePagesColumnsIsCutAtTheirEdge()
    {
        // Page 1, left to right, against 26.2.4.2's own four clip rectangles:
        //
        //   56.69..481.89  ZAAA, anchored in A with every neighbour free, so the area is widened
        //                  until its width is absorbed and then trimmed back to the block;
        //   226.77..311.76 ZDDD, anchored in C with D occupied — nothing to borrow, so the clip
        //                  is column C's own edge.  The control: a rule that clipped everything
        //                  to the block would widen this one and paint over the cell beside it;
        //   226.77..481.89 ZCCC, a centred C5:H5 merge whose align rectangle itself crosses the
        //                  block, though its text fits the merge easily;
        //   396.85..481.89 ZBBB, anchored in E, the last column the page prints, with F free —
        //                  cut at the block's 481.89 rather than at F's 566.93, and keeping
        //                  column E's own left edge, which is what shows the rectangle was
        //                  trimmed rather than replaced.
        ShouldMatch(
            Clips(Draw()[0]),
            (56.69, 481.89), (226.77, 311.76), (226.77, 481.89), (396.85, 481.89));
    }

    [Fact]
    public void AMergeReachingInFromTheLeftIsCutAtTheNearEdgeInstead()
    {
        // Page 2 draws the same C5:H5 merge from its true origin, off the left of the paper, and
        // trims the other side: 56.69..311.75 in the reference. Both directions come from one
        // fixture so that a fix which only ever trimmed the right would fail here.
        ShouldMatch(Clips(Draw()[1]), (56.69, 311.75));
    }

    [Fact]
    public void AStringThatFitsItsColumnIsNotClippedAtAll()
    {
        // Six runs on page 1 and four clips: ZEEE and STOP both fit their columns and Calc emits
        // no clip region for either. A clip per cell would put two operators around every run in
        // the file.
        Draw()[0].Runs.Count.ShouldBe(6);
    }

    [Fact]
    public void EveryCellClipAsksToKeepTheTextBehindIt()
    {
        // The half of the rule that is not about ink. LibreOffice's own PDF keeps every character
        // of a clipped cell, which is why the word gate never saw this defect — so a cell's clip
        // has to be the variant that cuts the ink and leaves the glyphs. Asking for the plain one
        // cost 124 words on `Data-Architecture-Tool-Fit-Assessment-Template.xlsx` alone.
        // `PdfInvisibleTextTests` pins what the two do differently in a file.
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-clip-block.fods"));

        ClipKindSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        sink.Keeping.ShouldBe(5, customMessage: "four clips on page 1 and one on page 2");
        sink.Hiding.ShouldBe(0);
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
