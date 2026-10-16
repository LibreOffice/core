using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A frame bigger than its page is cut down to the page, and a picture's or a chart's aspect is kept
/// while it is.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwFlyFreeFrame::CheckClip</c> (<c>sw/source/core/layout/flylay.cxx</c>:471-660), called from
/// <c>MakeAll</c> (:251) on every free fly. It first gives up the position and only squeezes if the
/// frame still does not fit, so the frames whose <em>size</em> it changes are exactly those bigger
/// than the sheet; :598-627 makes the squeeze proportional when the fly's lower is a
/// <c>SwGrfNode</c> or a <c>SwOLENode</c>, and :638-648 writes the result back into the frame format
/// for an OLE object — which is why the flat ODT can be used to read it.
/// </para>
/// <para>
/// <b>Every expectation here was read out of 26.2.4.2's own <c>.fodt</c> of
/// <c>023_Unit_Circle_Chart_Circular_Percentage</c></b>, one <c>wp:extent</c> changed at a time on a
/// 595.30 x 841.89 pt page (<c>probes/chart-fit-r97/framesize.py</c>). The document as authored
/// states a 682.10 x 493.50 pt chart and the reference resolves it to <b>595.30 x 430.70</b>.
/// </para>
/// <para>
/// The flat ODT shows a squeeze only for an OLE object, because the write-back at :638-648 is
/// OLE-only, so the wrap and node-kind rows were <em>rendered</em> instead
/// (<c>probes/chart-fit-r97/nodekind.py</c>): the same picture given the chart's extent is drawn
/// at all 682.10 pt with its authored <c>wrapNone</c> and at 595.30 x 430.70 with the wrap alone
/// changed to <c>wrapSquare</c>, and the chart with its wrap alone changed to <c>wrapNone</c> is
/// not squeezed either. The corpus states the rule once more without being asked:
/// <c>fleetfastfacts16nov2023.docx</c>'s anchored 606.90 x 231.60 pt picture is drawn 595.30 x
/// 227.20 on a 595.30 pt page.
/// </para>
/// <para>
/// <b>Nothing here covers an as-character object</b>, which is a <c>SwFlyInContentFrame</c> and so
/// not a <c>SwFlyFreeFrame</c> at all (<c>sw/source/core/inc/flyfrms.hxx</c>:212 against :150).
/// 26.2.4.2 draws <c>tibs_guidelines_2.docx</c>'s 686.20 pt <c>wp:inline</c> picture at 686.20 pt
/// on a 612 pt page. <see cref="FrameLayout.Place"/> is never called for one, so this needs no
/// arm — but a future reader moving the squeeze elsewhere would have to put one back.
/// </para>
/// </remarks>
public sealed class FrameSqueezedToPageTests
{
    /// <summary>The witness's own page: A4 portrait.</summary>
    private static readonly PageGeometry Page = new()
    {
        Size = new DocSize(Length.FromPoints(595.3), Length.FromPoints(841.89)),
        Margins = PageMargins.Uniform(Length.FromPoints(72)),
    };

    /// <summary>
    /// The witness: a chart wider than the sheet comes back at the page's width and proportionally
    /// shorter.
    /// </summary>
    [Theory]
    [InlineData(682.10, 493.50, 595.30, 430.70)]
    [InlineData(708.66, 157.48, 595.30, 132.29)]
    [InlineData(314.96, 905.51, 292.79, 841.89)]
    [InlineData(708.66, 905.51, 595.30, 760.65)]
    [InlineData(393.70, 236.22, 393.70, 236.22)]
    public void AChartBiggerThanThePageIsCutDownToItProportionally(
        double width, double height, double expectedWidth, double expectedHeight)
    {
        DocRect placed = Place(Frame(width, height) with { IsImage = true });

        // 0.05 pt is one twip, and the ratio is applied in whole truncated twips on the reference
        // side — `aFrameRect.Width() * aOldSize.Height() / aOldSize.Width()` over a
        // `tools::Long` — where this multiplies EMU and rounds. The worst of the five rows is
        // 0.04 pt.
        placed.Width.Points.ShouldBe(expectedWidth, 0.05);
        placed.Height.Points.ShouldBe(expectedHeight, 0.05);
    }

    /// <summary>
    /// And it starts from the page's own edge, because the move that precedes the squeeze has
    /// already put it there.
    /// </summary>
    [Fact]
    public void ItIsFlushWithTheEdgeItOverflowed()
    {
        DocRect placed = Place(Frame(682.10, 493.50) with { IsImage = true });

        placed.X.ShouldBe(Length.Zero);
        placed.Y.Points.ShouldBe(117.50, 0.02);
    }

    /// <summary>
    /// A frame holding neither a picture nor a chart has its two axes cut independently: :598
    /// reaches the proportional branch only for a <c>SwNoTextFrame</c> lower.
    /// </summary>
    /// <remarks>
    /// <b>This arm is read out of the C++ and is not measured.</b> The corpus states no witness for
    /// it — <c>probes/chart-fit-r97/census.py</c> finds three oversize text boxes in one document,
    /// <c>docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx</c>, and all three are
    /// exempt for other reasons (two <c>wp:inline</c>, one wrap-through). It is here because the
    /// bBot/bRig cuts at :571-585 run before the <c>IsNoTextFrame</c> test and leaving it out would
    /// need its own justification, not because a rendering was compared.
    /// </remarks>
    [Fact]
    public void ATextFrameKeepsTheAxisThatFits()
    {
        DocRect placed = Place(Frame(682.10, 493.50));

        placed.Width.Points.ShouldBe(595.30, 0.02);
        placed.Height.Points.ShouldBe(493.50, 0.02);
    }

    /// <summary>
    /// <c>DisableOffPagePositioning</c> and a wrap-through object together exempt it, and neither
    /// alone does.
    /// </summary>
    /// <remarks>
    /// <c>SwAnchoredObject::IsDraggingOffPageAllowed</c>
    /// (<c>sw/source/core/layout/anchoredobject.cxx</c>:790-801), the guard on
    /// <c>CheckClip</c>:493. <c>sw/source/writerfilter/filter/WriterFilter.cxx</c>:333 is the only
    /// setter of the setting in <c>sw/</c>, and it is the <em>OOXML</em> filter — RTF's own
    /// <c>RtfFilter::setTargetDocument</c> (<c>RtfFilter.cxx</c>:191-195) sets nothing — so only the
    /// DOCX reader turns it on.
    /// </remarks>
    [Theory]
    [InlineData(false, TextWrap.Both, 595.30)]
    [InlineData(false, TextWrap.Through, 595.30)]
    [InlineData(true, TextWrap.Both, 595.30)]
    [InlineData(true, TextWrap.Through, 682.10)]
    public void OnlyAWrapThroughObjectUnderTheSettingEscapes(
        bool disables, TextWrap wrap, double expectedWidth)
    {
        PageFrame frame = Frame(682.10, 493.50) with { IsImage = true, Wrap = wrap };

        FrameLayout.Place(
                frame, Page, Page.TextArea, Length.FromPoints(100),
                disablesOffPagePositioning: disables)
            .Width.Points.ShouldBe(expectedWidth, 0.02);
    }

    /// <summary>
    /// A drawing object is exempt whatever it holds, because it is not a fly.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CheckClip</c> is a <c>SwFlyFreeFrame</c> method and a shape is an
    /// <c>SwAnchoredDrawObject</c>. <strong>A shape carrying a text box is still a shape</strong>:
    /// <c>SwTextBoxHelper</c> pairs a draw format with a fly format, the fly holds the text and
    /// goes through <c>CheckClip</c>, and what is drawn — fill, outline, the rectangle a
    /// <see cref="PageFrame"/> models — stays the draw object. This is the opposite answer from
    /// <see cref="PaginationOptions.CapturesAnchoredObjectsOnPage"/>'s <c>bConsidered</c>, which
    /// treats a TextBox shape <em>as</em> a fly.
    /// </para>
    /// <para>
    /// Measured, because the other reading cut three corpus documents the reference does not cut:
    /// <c>ABCD-WB-08-00 Weight and Balance Report</c> and <c>ABCD-FE-01-00 Flight Envelope</c>
    /// each state a <c>wrapSquare</c> <c>wp:anchor</c>/<c>wps:txbx</c> banner 739.25 x 56.85 pt in
    /// a running head on a 595.30 pt page, and <c>ABCD-SDE-23-00 - Avionic System Description</c>
    /// one of 839.80 x 56.85 — and 26.2.4.2 draws 739.2 and 839.8 pt of them, off the sheet.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(FrameObjectKind.Shape, 682.10)]
    [InlineData(FrameObjectKind.TextBoxShape, 682.10)]
    [InlineData(FrameObjectKind.Fly, 595.30)]
    public void ADrawingObjectIsNotAFlyAndIsNotCut(FrameObjectKind kind, double expectedWidth)
    {
        PageFrame frame = Frame(682.10, 493.50) with { ObjectKind = kind };

        Place(frame).Width.Points.ShouldBe(expectedWidth, 0.02);
    }

    /// <summary>The witness's own anchor: 45.36 pt below a paragraph at the top of the body.</summary>
    private static PageFrame Frame(double width, double height) => new()
    {
        Size = new DocSize(Length.FromPoints(width), Length.FromPoints(height)),
        Anchor = FrameAnchor.Paragraph,
        VerticalOrigin = FrameVerticalOrigin.Paragraph,
        VerticalAlignment = FrameVerticalAlignment.Offset,
        VerticalOffset = Length.FromPoints(17.5),
        HorizontalOrigin = FrameHorizontalOrigin.Column,
        HorizontalAlignment = FrameHorizontalAlignment.Offset,
    };

    private static DocRect Place(PageFrame frame)
        => FrameLayout.Place(frame, Page, Page.TextArea, Length.FromPoints(100));
}
