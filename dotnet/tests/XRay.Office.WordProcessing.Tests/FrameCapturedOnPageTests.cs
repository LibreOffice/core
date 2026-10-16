using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A content-anchored frame is pulled back inside the page it is on; a page-anchored one is not.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwAnchoredObjectPosition::ImplAdjustVertRelPos</c>
/// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:504-667), whose own comment is
/// <em>"adjust calculated vertical in order to keep object inside 'page' alignment layout frame"</em>:
/// the bottom is corrected first and the top afterwards, so a frame taller than the page ends flush
/// with the top and overflows the bottom. <c>SwToLayoutAnchoredObjectPosition</c> — a
/// <c>FLY_AT_PAGE</c> fly — never calls it (<c>tolayoutanchoredobjectposition.cxx</c>:100-131), which
/// is why the anchor decides and the vertical origin does not.
/// </para>
/// <para>
/// Measured on <c>words/done-013/doc/omrIMInterpretiveGuideLine.doc</c> against 26.2.4.2 with the
/// tarball's <c>LiberationSansNarrow</c> aside, so both stacks resolve the same four faces. Its
/// <c>COMMENTS AND QUESTIONS</c> block is a WW8 APO: <c>sprmPDyaAbs</c> 100.90 pt from an anchor
/// paragraph 666.45 pt down a 792 pt page, 36.00 pt tall, so its stated bottom is 803.35 — <b>11.35 pt
/// below the sheet</b>, and its second line was drawn at a baseline of −0.70 and lost from the text
/// layer. The reference draws the block with its bottom flush at 791.95, and with this the two agree
/// to <b>0.05 pt</b> on both of its lines, which is the tolerance page 1's other 30 lines already met.
/// The document's gate verdict goes <c>words</c> → <c>match</c>, 2370 alphanumerics against 2370.
/// </para>
/// <para>
/// <see cref="PaginationOptions.CapturesAnchoredObjectsOnPage"/> carries which formats do this and
/// why; the short of it is that <c>WriterFilter.cxx</c>:332 sets
/// <c>DoNotCaptureDrawObjsOnPage</c> for DOCX and RTF and no other importer does — and that the RTF
/// reader turns the capture back on anyway, because its shapes are Writer <em>flies</em> rather than
/// drawing objects and a fly is clipped onto its page by a route that flag does not reach. See
/// <see cref="RtfShapePlacementTests"/>.
/// </para>
/// <para>
/// <b>The horizontal half is the same rule and the same guard.</b>
/// <c>SwAnchoredObjectPosition::ImplAdjustHoriRelPos</c> (the same file, :674-721) corrects the right
/// edge first and the left afterwards, so a frame wider than the page ends flush with the <em>left</em>
/// — the mirror of the vertical order, because the second correction is the one that survives.
/// </para>
/// </remarks>
public sealed class FrameCapturedOnPageTests
{
    /// <summary>A 792 pt page with a one-inch margin, which is the witness's own geometry.</summary>
    private static readonly PageGeometry Page = new()
    {
        Size = new DocSize(Length.FromPoints(612), Length.FromPoints(792)),
        Margins = PageMargins.Uniform(Length.FromPoints(72)),
    };

    /// <summary>A frame past the page's bottom is pulled up until its bottom rests on it.</summary>
    [Fact]
    public void AFrameBelowThePageIsPulledUpToItsBottom()
    {
        Place(Frame(FrameAnchor.Paragraph, height: 36), anchorTop: 666.45)
            .Y.ShouldBe(Length.FromPoints(756));
    }

    /// <summary>A frame that fits is left exactly where its offset put it.</summary>
    [Fact]
    public void AFrameInsideThePageIsNotMoved()
    {
        Place(Frame(FrameAnchor.Paragraph, height: 36), anchorTop: 400)
            .Y.ShouldBe(Length.FromPoints(500.9));
    }

    /// <summary>An anchor character is the same case: it is the anchor type, not the origin.</summary>
    [Fact]
    public void ACharacterAnchorIsCapturedToo()
    {
        Place(Frame(FrameAnchor.Character, height: 36), anchorTop: 666.45)
            .Y.ShouldBe(Length.FromPoints(756));
    }

    /// <summary>A page-anchored frame is left hanging off the sheet.</summary>
    [Fact]
    public void APageAnchoredFrameIsNotCaptured()
    {
        Place(Frame(FrameAnchor.Page, height: 36), anchorTop: 666.45)
            .Y.ShouldBe(Length.FromPoints(767.35));
    }

    /// <summary>
    /// A frame taller than the page ends flush with the <em>top</em> and overflows the bottom.
    /// </summary>
    /// <remarks>
    /// The order of the two corrections, which is not arbitrary: the bottom is adjusted first and the
    /// top afterwards, so the second correction is what survives when both fire.
    /// </remarks>
    [Fact]
    public void AFrameTallerThanThePageIsFlushWithItsTop()
    {
        Place(Frame(FrameAnchor.Paragraph, height: 900), anchorTop: 100)
            .Y.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A negative offset that would take a frame above the sheet is pushed back down to it.
    /// </summary>
    [Fact]
    public void AFrameAboveThePageIsPushedDownToItsTop()
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            VerticalOffset = Length.FromPoints(-200),
        };

        Place(frame, anchorTop: 100).Y.ShouldBe(Length.Zero);
    }

    /// <summary>A frame past the page's right edge is pulled left until its right rests on it.</summary>
    [Fact]
    public void AFrameBeyondThePageIsPulledLeftToItsRightEdge()
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            HorizontalOffset = Length.FromPoints(200),
        };

        // The column starts at 72 pt, so 200 pt past it is 272 and the 486 pt frame would end at 758
        // on a 612 pt page. Its right edge comes back to 612 and its left to 126.
        Place(frame, anchorTop: 100).X.ShouldBe(Length.FromPoints(126));
    }

    /// <summary>A negative offset that would take a frame off the left edge is pushed back to it.</summary>
    [Fact]
    public void AFrameLeftOfThePageIsPushedRightToItsLeftEdge()
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            HorizontalOffset = Length.FromPoints(-200),
        };

        Place(frame, anchorTop: 100).X.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A frame wider than the page ends flush with its <em>left</em>, which is the order of the two
    /// corrections and the mirror of the vertical case.
    /// </summary>
    [Fact]
    public void AFrameWiderThanThePageIsFlushWithItsLeft()
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            Size = new DocSize(Length.FromPoints(700), Length.FromPoints(36)),
        };

        Place(frame, anchorTop: 100).X.ShouldBe(Length.Zero);
    }

    /// <summary>A frame that fits across the page is left where its offset put it.</summary>
    [Fact]
    public void AFrameInsideThePageIsNotMovedAcrossIt()
        => Place(Frame(FrameAnchor.Paragraph, height: 36), anchorTop: 100)
            .X.ShouldBe(Length.FromPoints(72));

    /// <summary>With capture off — a DOCX — nothing is moved in either direction.</summary>
    [Fact]
    public void AFormatThatDoesNotCaptureLeavesTheFrameWhereItIs()
    {
        FrameLayout.Place(
                Frame(FrameAnchor.Paragraph, height: 36),
                Page,
                Page.TextArea,
                Length.FromPoints(666.45),
                capturesOnPage: false)
            .Y.ShouldBe(Length.FromPoints(767.35));

        FrameLayout.Place(
                Frame(FrameAnchor.Paragraph, height: 36) with
                {
                    HorizontalOffset = Length.FromPoints(200),
                },
                Page,
                Page.TextArea,
                Length.FromPoints(100),
                capturesOnPage: false)
            .X.ShouldBe(Length.FromPoints(272));
    }

    /// <summary>
    /// Under a format that sets <c>DoNotCaptureDrawObjsOnPage</c>, what escapes is decided by the
    /// object's <em>kind</em> as well as by its wrap.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>bConsidered</c> is <c>bWrapThrough &amp;&amp; !bTextBox</c> for a fly and
    /// <c>bWrapThrough || !bTextBox</c> for a draw object (<c>anchoredobjectposition.cxx</c>:130-141,
    /// whose own comment states the asymmetry), so under DOCX's flag a picture and a shape carrying a
    /// text box are captured unless they wrap through and <b>a shape with no text box never is</b>.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 over 93 one-attribute fixtures — three object kinds × five wraps × six
    /// positions, <c>probes/words-close-r95/make-capture.py</c>. A 100 pt band stated 40 pt from the
    /// right edge of a 595.3 pt page is drawn at x <b>495.25</b> as a picture or a text box under
    /// <c>wrapSquare</c> and at <b>555.25</b> as a bare shape or under <c>wrapNone</c>; the same band
    /// 20 pt above the bottom edge is at y 801.75 and 822.00. 91 of the 93 carry a scoreable band and
    /// this tree reproduces all 91.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(FrameObjectKind.Fly, TextWrap.Both, 126)]
    [InlineData(FrameObjectKind.TextBoxShape, TextWrap.Both, 126)]
    [InlineData(FrameObjectKind.Shape, TextWrap.Both, 272)]
    [InlineData(FrameObjectKind.Fly, TextWrap.Through, 272)]
    [InlineData(FrameObjectKind.TextBoxShape, TextWrap.Through, 272)]
    [InlineData(FrameObjectKind.Shape, TextWrap.Through, 272)]
    public void ADocxShapeWithNoTextBoxIsNeverCaptured(
        FrameObjectKind kind, TextWrap wrap, int expected)
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            HorizontalOffset = Length.FromPoints(200),
            ObjectKind = kind,
            Wrap = wrap,
        };

        FrameLayout.Place(
                frame, Page, Page.TextArea, Length.FromPoints(100),
                capturesOnPage: false, capturesWrappedObjects: true)
            .X.ShouldBe(Length.FromPoints(expected));
    }

    /// <summary>
    /// The area narrows to the body only for an anchor that has one — not for a running head's.
    /// </summary>
    /// <remarks>
    /// <c>ImplAdjustVertRelPos</c>:568-573 walks <c>mpAnchorFrame-&gt;FindBodyFrame()</c> up to a page
    /// body frame whose upper is this page, and a header, a footer and a footnote anchor have none.
    /// Measured on the same fixtures: a <c>wrapSquare</c> shape carrying a text box, anchored at its
    /// own paragraph 36 pt down inside the running head of a page with 72 pt margins, is drawn by
    /// 26.2.4.2 at y <b>36.00</b> and not at the body's 72 — where the same object anchored in the
    /// body is drawn at 72.00. It is what keeps this off <c>b053-19</c>'s and
    /// <c>Case-Study-Heathrow-Airport</c>'s header pictures, which the wide rule of
    /// <c>probes/frame-area-r85</c> cost 8.25 and 1.09 of mean page ink.
    /// </remarks>
    [Theory]
    [InlineData(FrameAnchorPlace.Body, 72)]
    [InlineData(FrameAnchorPlace.Furniture, 20)]
    [InlineData(FrameAnchorPlace.Elsewhere, 20)]
    public void OnlyAnAnchorInTheBodyNarrowsTheCaptureToIt(FrameAnchorPlace place, int expected)
    {
        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            ObjectKind = FrameObjectKind.TextBoxShape,
            Wrap = TextWrap.Both,
            VerticalOrigin = FrameVerticalOrigin.Paragraph,
            VerticalOffset = Length.FromPoints(-80),
        };

        FrameLayout.Place(
                frame, Page, Page.TextArea, Length.FromPoints(100),
                capturesOnPage: false, capturesWrappedObjects: true, narrowsCaptureToBody: true,
                anchorPlace: place)
            .Y.ShouldBe(Length.FromPoints(expected));
    }

    /// <summary>
    /// A vertical origin of <c>PAGE_FRAME</c> or <c>PAGE_PRINT_AREA</c> keeps the sheet as its area.
    /// </summary>
    /// <remarks>
    /// The two relations <c>ImplAdjustVertRelPos</c>:564-565 excludes by name. The two margin
    /// <em>bands</em> are <c>PAGE_PRINT_AREA_TOP</c> and <c>_BOTTOM</c>, which are different
    /// enumerators and are narrowed — which is why the three genogram templates move and a frame
    /// stated against <c>page</c> or <c>margin</c> does not.
    /// </remarks>
    [Theory]
    [InlineData(FrameVerticalOrigin.Page, 20)]
    [InlineData(FrameVerticalOrigin.PageMargin, 20)]
    [InlineData(FrameVerticalOrigin.Paragraph, 72)]
    public void TheSheetAndTheMarginRelationsAreNotNarrowed(
        FrameVerticalOrigin origin, int expected)
    {
        Length offset = origin == FrameVerticalOrigin.PageMargin
            ? Length.FromPoints(-52)
            : Length.FromPoints(origin == FrameVerticalOrigin.Page ? 20 : -80);

        PageFrame frame = Frame(FrameAnchor.Paragraph, height: 36) with
        {
            ObjectKind = FrameObjectKind.TextBoxShape,
            Wrap = TextWrap.Both,
            VerticalOrigin = origin,
            VerticalOffset = offset,
        };

        FrameLayout.Place(
                frame, Page, Page.TextArea, Length.FromPoints(100),
                capturesOnPage: false, capturesWrappedObjects: true, narrowsCaptureToBody: true)
            .Y.ShouldBe(Length.FromPoints(expected));
    }

    /// <summary>The witness's own frame: 486 x 36 pt at a stated 100.90 pt below its anchor.</summary>
    private static PageFrame Frame(FrameAnchor anchor, double height) => new()
    {
        Size = new DocSize(Length.FromPoints(486), Length.FromPoints(height)),
        Anchor = anchor,
        VerticalOrigin = FrameVerticalOrigin.Paragraph,
        VerticalAlignment = FrameVerticalAlignment.Offset,
        VerticalOffset = Length.FromPoints(100.9),
        HorizontalOrigin = FrameHorizontalOrigin.Column,
        HorizontalAlignment = FrameHorizontalAlignment.Offset,
    };

    private static DocRect Place(PageFrame frame, double anchorTop)
        => FrameLayout.Place(frame, Page, Page.TextArea, Length.FromPoints(anchorTop));
}
