using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A shape anchored in a table cell reads a page-relative vertical origin as the page's
/// <em>printable area</em>.
/// </summary>
/// <remarks>
/// <para>
/// <c>ProcessEscherAlign</c> (<c>sw/source/filter/ww8/ww8graf.cxx</c>:2516-2527), under the comment
/// <em>"Microsoft is buggy and inconsistent in how they handle layoutInCell"</em>: for a shape
/// anchored at a character inside a table and laid out in its cell, an origin of
/// <c>PAGE_FRAME</c> becomes <c>PAGE_PRINT_AREA</c> — <em>"page is implemented as if it was margin"</em>
/// — and every vertical alignment but "at the stated offset" becomes "top".
/// </para>
/// <para>
/// It is worth a whole top margin. Measured on
/// <c>words/pagination-001/doc/absrc-pac-01-info-note-en.doc</c>, whose two masthead logos are
/// character-anchored in the first row of the banner table 180 twips below a page-relative origin:
/// the reference draws them from 45.00 to 90.80 pt from the page's top edge, which is the 36 pt top
/// margin plus the stated 9 pt, and following the field alone drew them from 9.00 to 54.75 — the
/// whole margin high, with every row of the table under them following.
/// </para>
/// </remarks>
public sealed class Ww8LayoutInCellTests
{
    /// <summary>Outside a table the field is followed exactly as stated.</summary>
    [Fact]
    public void APageRelativeShapeOutsideATableKeepsThePageOrigin()
    {
        Build(inTableCell: false).VerticalOrigin.ShouldBe(FrameVerticalOrigin.Page);
    }

    /// <summary>Inside one it is read as the printable area instead.</summary>
    [Fact]
    public void APageRelativeShapeInACellIsMeasuredFromThePrintableArea()
    {
        Build(inTableCell: true).VerticalOrigin.ShouldBe(FrameVerticalOrigin.PageMargin);
    }

    /// <summary>
    /// Word 97 never lays a shape out in the cell, so a file it wrote is left alone.
    /// </summary>
    /// <remarks>
    /// <c>IsObjectLayoutInTableCell</c> switches on the writing application rather than on the format
    /// version, and asserts that Word 97 states no group-shape booleans at all
    /// (<c>ww8graf.cxx</c>:2570-2578).
    /// </remarks>
    [Fact]
    public void AWord97FileIsLeftAlone()
    {
        Build(inTableCell: true, writtenByWord97: true)
            .VerticalOrigin.ShouldBe(FrameVerticalOrigin.Page);
    }

    /// <summary>The horizontal origin is not touched by any of it.</summary>
    /// <remarks>
    /// The two axes have separate rules and only the vertical one is remapped here — which is what
    /// the corpus document confirms: its logos are drawn at the same x by both renderers and 36 pt
    /// apart in y.
    /// </remarks>
    [Fact]
    public void TheHorizontalOriginIsUnchanged()
    {
        Build(inTableCell: true).HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Paragraph);
    }

    /// <summary>
    /// One <c>FSPA</c> for a 100 × 100 twip shape at a stated offset, page-relative vertically and
    /// paragraph-relative horizontally — the shape the corpus document writes.
    /// </summary>
    private static PageFrame Build(bool inTableCell, bool writtenByWord97 = false)
    {
        Ww8ShapeAnchor anchor = new(
            Position: 0,
            ShapeId: 1,
            Left: 0,
            Top: 180,
            Right: 100,
            Bottom: 280,
            IsHeaderAnchor: false,
            HorizontalOrigin: Ww8ShapeOrigin.Text,
            VerticalOrigin: Ww8ShapeOrigin.Page,
            Wrap: 2,
            WrapSide: 0,
            IsPageRelative: false,
            IsBelowText: false);

        return Ww8Frames.Build(
            anchor, shape: null, offset: 0, blocks: [],
            setInLine: false, inTableCell: inTableCell, writtenByWord97: writtenByWord97)
            .ShouldNotBeNull();
    }
}
