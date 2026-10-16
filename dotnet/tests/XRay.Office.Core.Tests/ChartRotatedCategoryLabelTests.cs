using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Where a turned category label hangs from its tick.
/// </summary>
/// <remarks>
/// <para>
/// A rotated label is the one axis mark whose anchor cannot be checked by eye on a chart whose
/// categories are all the same length: corner-anchoring and centre-anchoring then differ by one
/// constant, so the axis looks merely shifted rather than wrong. It is only when the labels differ
/// in length that the two arrangements separate — and then a short name lands inside a long
/// neighbour, which is the defect these tests exist to keep closed.
/// </para>
/// <para>
/// The quantity asserted is deliberately the one that can be read out of a reference PDF without
/// knowing anything about side bearings: <strong>how far the right-hand end of one label is from
/// the right-hand end of the next</strong>. Corner-anchored it is one category slot, whatever the
/// names are; centred it is a slot plus half the difference of two widths. Measured on
/// <c>057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx</c>, whose chart sheet turns twenty
/// names of 22 to 141 pt to 45°: 26.2.4.2 advances by 28.67, 28.92, 29.14 … 28.73 pt against a
/// slot of 28.9465, and we advanced by 11.47, 23.31, 21.45 … 53.51.
/// </para>
/// </remarks>
public class ChartRotatedCategoryLabelTests
{
    /// <summary>Half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    /// <summary>Names of very unequal length, which is what separates the two anchorings.</summary>
    private static readonly string[] Names =
    [
        "Cash",
        "Investments",
        "Accounts receivable",
        "Less accumulated depreciation",
        "Goodwill",
        "Accrued compensation",
        "Other",
        "Accumulated retained earnings",
    ];

    private static ChartPlot Plot(string[] categories) => new()
    {
        Categories = [.. categories],
        Series =
        [
            new ChartSeries(
                "Current",
                [.. Enumerable.Range(1, categories.Length).Select(at => at * 10.0)],
                Colour.FromRgb(0x99CCFF)),
        ],
    };

    /// <summary>The right-hand end of each label's turned box, in drawing order.</summary>
    /// <remarks>
    /// A rotated label is emitted anchored at the centre of its turned bounding box — that is all
    /// a glyph run can be positioned by — so the end has to be reconstructed from the box, whose
    /// width is <c>W·|cos| + H·|sin|</c>.
    /// </remarks>
    private static List<Length> RightEnds(ChartDrawing drawing, string[] names)
    {
        Ruler ruler = new();
        List<Length> ends = [];

        foreach (ChartLabel label in drawing.Labels)
        {
            if (!names.Contains(label.Text)) continue;

            DocSize box = ruler.Measure(label.Text, label.Size, null, false);
            Length turned = box.Width * Math.Abs(Math.Cos(label.Rotation))
                            + box.Height * Math.Abs(Math.Sin(label.Rotation));
            ends.Add(label.At.X + turned / 2.0);
        }

        return ends;
    }

    /// <summary>
    /// Every turned label's far end advances by exactly one category slot.
    /// </summary>
    /// <remarks>
    /// <c>lcl_correctRotation_Bottom</c>'s <c>if( !bRotateAroundCenter )</c> term
    /// (<c>chart2/source/view/main/LabelPositionHelper.cxx:249-255</c>), and
    /// <c>bRotateAroundCenter</c> is <c>m_bComplexCategories</c>
    /// (<c>chart2/source/view/axes/VCartesianAxis.cxx:147-148</c>) — false here.
    /// </remarks>
    [Fact]
    public void ATurnedCategoryLabelHangsFromItsTickByItsFarEnd()
    {
        ChartDrawing drawing = ChartLayout.Place(Plot(Names), Frame, new Ruler());

        List<Length> ends = RightEnds(drawing, Names);
        ends.Count.ShouldBe(Names.Length);

        double slot = (drawing.PlotArea.Width / Names.Length).Points;
        slot.ShouldBeGreaterThan(0.0);

        for (int at = 1; at < ends.Count; at++)
        {
            (ends[at] - ends[at - 1]).Points.ShouldBe(slot, 0.01);
        }
    }

    /// <summary>
    /// The labels are turned in the first place, and no two of them overlap.
    /// </summary>
    /// <remarks>
    /// Two labels at 45° are strips, so their separation is measured perpendicular to their own
    /// baselines — <c>q = (x + y)/√2</c> for text running up to the right. Corner-anchored, every
    /// corner is one slot along the axis and the same depth below it, so <c>q</c> advances by
    /// <c>slot/√2</c> and nothing can touch. Centred, <c>q</c> carries half the label's own width
    /// as well, and a short name after a long one lands in the same strip: on the corpus witness
    /// <c>Less accumulated depreciation</c> sat at 565.1 and <c>Goodwill</c> at 562.4.
    /// </remarks>
    [Fact]
    public void NoTwoTurnedLabelsShareAStrip()
    {
        ChartDrawing drawing = ChartLayout.Place(Plot(Names), Frame, new Ruler());
        Ruler ruler = new();

        List<(double Across, double Half)> strips = [];
        foreach (ChartLabel label in drawing.Labels)
        {
            if (!Names.Contains(label.Text)) continue;

            label.Rotation.ShouldNotBe(0.0);

            double sine = Math.Sin(label.Rotation);
            double cosine = Math.Cos(label.Rotation);
            DocSize box = ruler.Measure(label.Text, label.Size, null, false);

            // The label's centre projected onto the normal of its own baseline, and half its
            // height along that normal — which is what one strip occupies.
            strips.Add((
                (label.At.X.Points * sine) + (label.At.Y.Points * cosine),
                box.Height.Points / 2.0));
        }

        strips.Count.ShouldBe(Names.Length);
        strips.Sort();

        for (int at = 1; at < strips.Count; at++)
        {
            (strips[at].Across - strips[at - 1].Across)
                .ShouldBeGreaterThan(strips[at].Half + strips[at - 1].Half);
        }
    }

    /// <summary>
    /// An upright label is still centred on its tick.
    /// </summary>
    /// <remarks>
    /// The lean is <c>-sign(sin a)·W·cos(a)/2</c> and every branch of
    /// <c>lcl_correctRotation_Bottom</c> leaves the zero-angle case alone
    /// (<c>LabelPositionHelper.cxx:246-248</c>), so an axis whose labels fit must not move at all.
    /// That is nearly every chart in the corpus, which is why it is asserted rather than assumed.
    /// </remarks>
    [Fact]
    public void AnUprightCategoryLabelIsUnmoved()
    {
        string[] shorts = ["Q1", "Q2", "Q3", "Q4"];

        ChartDrawing drawing = ChartLayout.Place(Plot(shorts), Frame, new Ruler());
        DocRect area = drawing.PlotArea;

        for (int at = 0; at < shorts.Length; at++)
        {
            ChartLabel label = drawing.Labels.Single(one => one.Text == shorts[at]);

            label.Rotation.ShouldBe(0.0);
            label.Anchor.ShouldBe(ChartLabelAnchor.CentreTop);
            label.At.X.Points.ShouldBe(
                (area.Left + (area.Width * ((at + 0.5) / shorts.Length))).Points, 0.01);
        }
    }
}
