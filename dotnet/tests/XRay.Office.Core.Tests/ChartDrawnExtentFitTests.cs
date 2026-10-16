using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// An embedded chart is fitted to its own drawn extent, not to its page.
/// </summary>
/// <remarks>
/// <para>
/// <c>ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters</c>
/// (<c>svx/source/sdr/contact/viewcontactofsdrole2obj.cxx</c>:88-116) takes the bounding range of
/// every primitive the chart's own draw page produced —
/// <c>ChartHelper::tryToGetChartContentAsPrimitive2DSequence</c>'s
/// <c>aRetval.getB2DRange(…)</c>, <c>svx/source/svdraw/charthelper.cxx</c>:96-100 — translates
/// its minimum to the origin, scales it by <c>1/width, 1/height</c> and multiplies by the OLE
/// object's own matrix. So a chart whose labels overflow its page is squeezed until the overflow
/// fits, <strong>by two different factors, one per axis</strong>.
/// </para>
/// <para>
/// <strong>Measured on <c>N2_E_Maestroni_Swarm_COP.pptx</c> page 7.</strong> 26.2.4.2 draws the
/// chart's own background rectangle at <c>119.083 … 719.660</c> by <c>92.58 … 516.983</c> in a
/// frame of <c>0 … 720</c> by <c>92.57 … 540.0</c>, which is 0.834135 across and
/// <strong>0.948534</strong> down; this tree now draws it at <c>1.847 … 720</c> by
/// <c>92.572 … 518.002</c>, which is <strong>0.95084</strong> down. The vertical factors agree to
/// 0.24% with no free parameter — the overflow there is that chart's rotated date labels, which
/// both stacks draw the same way. The horizontal ones do not, because that chart's category
/// labels are drawn on one line each by the reference and wrapped by us, which is a separate open
/// fault in the axis arrangement rather than in the fit.
/// </para>
/// </remarks>
public sealed class ChartDrawnExtentFitTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.FromPoints(10), Length.FromPoints(20),
            Length.FromPoints(400), Length.FromPoints(300));

    /// <summary>A chart whose labels all fit the room the layout gives them.</summary>
    private static ChartPlot Fitting() => new()
    {
        Background = Colour.FromRgb(0xFFFFFF),
        Categories = ["One", "Two", "Three"],
        Series = [new ChartSeries("Values", [4.0, 9.0, 2.0], Colour.FromRgb(0x0070C0))],
    };

    /// <summary>The same chart with one category name too long for its slot.</summary>
    private static ChartPlot Overflowing() => new()
    {
        Background = Colour.FromRgb(0xFFFFFF),
        Categories =
        [
            "A category name very much longer than the slot it is centred under",
            "Two",
            "Three",
        ],
        Series = [new ChartSeries("Values", [4.0, 9.0, 2.0], Colour.FromRgb(0x0070C0))],
    };

    /// <summary>The chart's own background, which is drawn at the page's own rectangle.</summary>
    private static ChartBox Page(ChartDrawing drawing)
        => drawing.Boxes.First(box => box.Fill == Colour.FromRgb(0xFFFFFF));

    /// <summary>
    /// A chart that overflows its page is drawn smaller than the frame that displays it.
    /// </summary>
    [Fact]
    public void AChartWhoseLabelsOverflowIsSqueezedInsideItsFrame()
    {
        ChartDrawing drawing = ChartLayout.Place(Overflowing(), Frame, new Ruler());

        DocRect page = Page(drawing).Bounds;

        page.Width.ShouldBeLessThan(Frame.Width);
        page.Left.ShouldBeGreaterThanOrEqualTo(Frame.Left);
        page.Right.ShouldBeLessThanOrEqualTo(Frame.Right);
    }

    /// <summary>
    /// The two factors are independent, which is what "scaled unequally" means.
    /// </summary>
    /// <remarks>
    /// The discriminator against fitting by a single factor, which is what a page-shaped
    /// normalisation would give. Here only the horizontal overflows, so the vertical factor is
    /// exactly one and the horizontal is not.
    /// </remarks>
    [Fact]
    public void TheTwoFactorsAreMeasuredSeparatelyPerAxis()
    {
        ChartDrawing drawing = ChartLayout.Place(Overflowing(), Frame, new Ruler());

        DocRect page = Page(drawing).Bounds;

        double across = (double)page.Width.Emu / Frame.Width.Emu;
        double down = (double)page.Height.Emu / Frame.Height.Emu;

        across.ShouldBeLessThan(0.99);
        down.ShouldBe(1.0, 0.001);
    }

    /// <summary>
    /// The em follows the vertical factor and the residual goes onto the label's stretch.
    /// </summary>
    /// <remarks>
    /// A glyph run carries one em and an anisotropic fit has two, so the type is scaled by the
    /// vertical factor alone and <see cref="ChartLabel.Stretch"/> carries <c>sx/sy</c> for the
    /// consumer to fold into its own transform. Dropping it draws every word of a squeezed chart
    /// that much too wide.
    /// </remarks>
    [Fact]
    public void ASqueezedChartCarriesTheResidualOnItsLabels()
    {
        ChartDrawing drawing = ChartLayout.Place(Overflowing(), Frame, new Ruler());

        DocRect page = Page(drawing).Bounds;
        double across = (double)page.Width.Emu / Frame.Width.Emu;
        double down = (double)page.Height.Emu / Frame.Height.Emu;

        drawing.Labels.ShouldNotBeEmpty();
        foreach (ChartLabel label in drawing.Labels)
            label.Stretch.ShouldBe(across / down, 0.002);
    }

    /// <summary>
    /// A chart whose composition stays inside its page is placed exactly as it was before.
    /// </summary>
    /// <remarks>
    /// The control, and the reason this is safe to apply to every chart rather than only to the
    /// ones a file marks: the drawn extent of an automatically laid out chart <em>is</em> its
    /// page, because the plot gives up whatever room its labels need, so the fit is the identity
    /// and no chart that was right moves.
    /// </remarks>
    [Fact]
    public void AChartThatFitsItsPageIsNotMovedAtAll()
    {
        ChartDrawing drawing = ChartLayout.Place(Fitting(), Frame, new Ruler());

        DocRect page = Page(drawing).Bounds;

        page.X.ShouldBe(Frame.X);
        page.Y.ShouldBe(Frame.Y);
        page.Width.ShouldBe(Frame.Width);
        page.Height.ShouldBe(Frame.Height);

        foreach (ChartLabel label in drawing.Labels)
            label.Stretch.ShouldBe(1.0, 1e-9);
    }

    /// <summary>A label that measures nothing does not drag the extent to the origin.</summary>
    /// <remarks>
    /// The bug the corpus found rather than the source: an empty label and an empty path both
    /// answered <c>DocRect.Empty</c>, and taking that as a rectangle is taking the point
    /// <c>(0, 0)</c>. On <c>048_Expense_trends_budget</c> — a chart whose content fits its page
    /// exactly — one value-axis label that formats to nothing fitted the whole picture at
    /// <c>sy = 0.6561</c>, which moved every bar of every one of its fourteen pages. A number
    /// format of <c>;;;</c> is the same statement in one attribute: it suppresses positives,
    /// negatives, zeroes and text alike, so every tick on the axis writes an empty string.
    /// </remarks>
    [Fact]
    public void ALabelThatFormatsToNothingIsNotAPointAtTheOrigin()
    {
        DocRect frame = new(
            Length.FromPoints(200), Length.FromPoints(200),
            Length.FromPoints(400), Length.FromPoints(300));

        ChartDrawing drawing = ChartLayout.Place(
            Fitting() with { ValueFormat = NumberFormatCode.Parse(";;;") }, frame, new Ruler());

        DocRect page = Page(drawing).Bounds;

        page.X.ShouldBe(frame.X);
        page.Y.ShouldBe(frame.Y);
        page.Width.ShouldBe(frame.Width);
        page.Height.ShouldBe(frame.Height);
    }

    /// <summary>
    /// A series mark contributes only the part of itself that falls inside the plot.
    /// </summary>
    /// <remarks>
    /// Every plotter clips its polygon to the scaled logic rectangle before it makes a shape of
    /// it — <c>Clipping::clipPolygonAtRectangle</c> at <c>AreaChart.cxx</c>:318, 336, 343, 359 and
    /// 445 among others — so what the reference's own range holds is the clipped geometry. Ours
    /// is not clipped yet, and on <c>171128IPAP.pptx</c> one line series runs 932 pt left of a
    /// 576 pt chart page: taking its own extent fitted that chart at <c>sx = 0.3546</c> where
    /// 26.2.4.2 does not fit it at all.
    /// </remarks>
    [Fact]
    public void ALineDrawnOutsideThePlotDoesNotDecideTheFit()
    {
        ChartPlot plot = new()
        {
            Background = Colour.FromRgb(0xFFFFFF),
            Kind = ChartPlotKind.Line,
            Categories = ["One", "Two", "Three"],
            ValueScale = new ChartScaleRequest(Minimum: 0.0, Maximum: 10.0),
            Series = [new ChartSeries("Runaway", [4.0, -900.0, 2.0], Colour.FromRgb(0x0070C0))],
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());
        DocRect page = Page(drawing).Bounds;

        page.Width.ShouldBe(Frame.Width);
        page.Height.ShouldBe(Frame.Height);
    }
}
