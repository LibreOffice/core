using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// The family a chart states reaches both halves of the layout: what the text is measured in and
/// what each label says it is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// Both halves matter and neither implies the other. A family that reaches only the labels names
/// a face nothing was measured in, so the plot area is reserved for one face and filled with
/// another; a family that reaches only the measurer lays the chart out correctly and then draws
/// it in the consumer's default. The two are separate assertions here because the code that
/// carries them is separate — <c>ChartText</c> binds the measurer at the entry point and
/// <c>InFamily</c> stamps the labels at the exit.
/// </para>
/// <para>
/// <strong>The unstated case is a control and passes under any reading</strong>, since null
/// reaching a measurer that ignores it is indistinguishable from null never being passed. It is
/// here so that the two cases that <em>can</em> fail are read against something.
/// </para>
/// </remarks>
public class ChartTextFamilyTests
{
    /// <summary>A measurer that records the family it was asked for and answers by character.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public List<string?> Families { get; } = [];

        public DocSize Measure(string text, Length size, string? family)
        {
            Families.Add(family);
            return new DocSize(size * (0.5 * text.Length), size * 1.15);
        }
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Bars() => new()
    {
        Title = "Revenue",
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
        Legend = ChartLegendPosition.Right,
    };

    [Fact]
    public void TheStatedFamilyIsWhatTheTextIsMeasuredIn()
    {
        Ruler ruler = new();
        ChartLayout.Place(Bars() with { TextFamily = "Liberation Mono" }, Frame, ruler);

        ruler.Families.Count.ShouldBeGreaterThan(0, "the layout measured something");
        ruler.Families.ShouldAllBe(family => family == "Liberation Mono");
    }

    [Fact]
    public void TheStatedFamilyIsCarriedOnEveryLabel()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Bars() with { TextFamily = "Liberation Mono" }, Frame, new Ruler());

        drawing.Labels.Count.ShouldBeGreaterThan(0, "the chart drew some text");
        drawing.Labels.ShouldAllBe(label => label.Family == "Liberation Mono");
    }

    /// <summary>
    /// A chart composed at its own size and stretched into its frame keeps its family.
    /// </summary>
    /// <remarks>
    /// The stretch rebuilds every label to move it and rescale its em, and a rebuild that dropped
    /// a member would be invisible to the two tests above — they take the unstretched path. This
    /// is the case an ODF chart takes, where the chart document states a size of its own.
    /// </remarks>
    [Fact]
    public void AStretchedChartKeepsTheFamilyThroughTheRebuild()
    {
        ChartPlot stretched = Bars() with
        {
            TextFamily = "Liberation Mono",
            Space = new DocSize(Length.FromPoints(600), Length.FromPoints(350)),
        };

        ChartDrawing drawing = ChartLayout.Place(stretched, Frame, new Ruler());

        drawing.Labels.Count.ShouldBeGreaterThan(0, "the chart drew some text");
        drawing.Labels.ShouldAllBe(label => label.Family == "Liberation Mono");
    }

    /// <summary>The control: a chart stating nothing asks for nothing and claims nothing.</summary>
    [Fact]
    public void AChartStatingNoFamilyLeavesTheChoiceToTheConsumer()
    {
        Ruler ruler = new();
        ChartDrawing drawing = ChartLayout.Place(Bars(), Frame, ruler);

        ruler.Families.ShouldAllBe(family => family == null);
        drawing.Labels.ShouldAllBe(label => label.Family == null);
    }
}
