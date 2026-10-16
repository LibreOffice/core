using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A category axis that states more than one level draws a row per level, not one joined string.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="ChartPlot.Categories"/> holds the join — <c>AM 9/5/2026</c> — because that is what
/// <c>ExplicitCategoriesProvider::getSimpleCategories</c> hands to a legend entry and a data
/// label, and it stays what it was. The axis is the other consumer and it draws the levels apart:
/// level zero nearest the axis line, the levels below it, and a long tick at every run boundary
/// (<c>chart2/source/view/axes/VCartesianAxis.cxx:575-610</c> and <c>:1913-1955</c>).
/// </para>
/// <para>
/// The two assertions that separate a correct port from a plausible one are the run rule and the
/// depth. A run is broken by the next <em>stated</em> value and not by the next different one, so
/// a repeated string is drawn again rather than merged; and the band is as many lines deep as it
/// has levels, or the second row is drawn through the bottom of the chart.
/// </para>
/// </remarks>
public class ChartComplexCategoryAxisTests
{
    /// <summary>Half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    /// <summary>Four categories: two days, morning and afternoon, as the file states them.</summary>
    private static ChartPlot Levelled(IReadOnlyList<IReadOnlyList<string?>>? levels) => new()
    {
        Categories = ["AM Mon", "PM Mon", "AM Tue", "PM Tue"],
        CategoryLevels = levels,
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    private static readonly IReadOnlyList<IReadOnlyList<string?>> TwoLevels =
    [
        new string?[] { "AM", "PM", "AM", "PM" },
        new string?[] { "Mon", "Mon", "Tue", "Tue" },
    ];

    /// <summary>Every level is drawn, and the joined string is not.</summary>
    [Fact]
    public void EachLevelIsDrawnOnItsOwnRow()
    {
        ChartDrawing drawing = Place(Levelled(TwoLevels));

        drawing.Labels.Count(label => label.Text == "AM").ShouldBe(2);
        drawing.Labels.Count(label => label.Text == "PM").ShouldBe(2);
        drawing.Labels.Count(label => label.Text == "Mon").ShouldBe(2);
        drawing.Labels.Count(label => label.Text == "Tue").ShouldBe(2);
        drawing.Labels.ShouldNotContain(label => label.Text == "AM Mon");

        // Level zero is the row nearest the axis and level one sits under it.
        Length inner = drawing.Labels.First(label => label.Text == "AM").At.Y;
        Length outer = drawing.Labels.First(label => label.Text == "Mon").At.Y;
        outer.ShouldBeGreaterThan(inner);
    }

    /// <summary>
    /// A repeated value starts a new run; only an unstated one continues the run above it.
    /// </summary>
    /// <remarks>
    /// <c>lcl_DataSequenceToComplexCategoryVector</c>'s own comment
    /// (<c>chart2/source/tools/ExplicitCategoriesProvider.cxx:296-299</c>): "Empty value is
    /// interpreted as a continuation of the previous category. Note that having the same value as
    /// the previous one does not equate to a continuation." So the two <c>Mon</c>s above are two
    /// labels and the one below is one, centred across the pair.
    /// </remarks>
    [Fact]
    public void AnUnstatedEntryContinuesTheRunAndARepeatDoesNot()
    {
        IReadOnlyList<IReadOnlyList<string?>> merged =
        [
            new string?[] { "AM", "PM", "AM", "PM" },
            new string?[] { "Mon", null, "Tue", null },
        ];

        ChartDrawing drawing = Place(Levelled(merged));

        drawing.Labels.Count(label => label.Text == "Mon").ShouldBe(1);
        drawing.Labels.Count(label => label.Text == "Tue").ShouldBe(1);

        // The merged label is centred on its two slots, which puts it between the two above it.
        Length left = drawing.Labels.First(label => label.Text == "AM").At.X;
        Length right = drawing.Labels.First(label => label.Text == "PM").At.X;
        Length middle = drawing.Labels.First(label => label.Text == "Mon").At.X;

        middle.ShouldBeGreaterThan(left);
        middle.ShouldBeLessThan(right);
    }

    /// <summary>
    /// The band is one line per level, so the plot area gives up two lines rather than one.
    /// </summary>
    [Fact]
    public void TwoLevelsReserveTwiceTheRoomOfOne()
    {
        ChartDrawing levelled = Place(Levelled(TwoLevels));
        ChartDrawing plain = Place(Levelled(null));

        Length line = Length.FromPoints(10) * 1.15;
        double taken = (plain.PlotArea.Bottom - levelled.PlotArea.Bottom).Emu;
        taken.ShouldBe(line.Emu, tolerance: line.Emu / 20.0);
    }

    /// <summary>
    /// A long tick separates the runs, and it reaches the bottom of the whole band.
    /// </summary>
    /// <remarks>
    /// The three boundaries between four categories, at the level whose runs end there. The
    /// innermost level's own boundaries are skipped when the axis states no major tick mark
    /// (<c>VCartesianAxis.cxx:1953</c>), which is what every levelled axis in the corpus states —
    /// so with ticks off the separators are the outer level's, one per pair.
    /// </remarks>
    [Fact]
    public void TheRunBoundariesAreSeparatedByALongTick()
    {
        IReadOnlyList<IReadOnlyList<string?>> merged =
        [
            new string?[] { "AM", "PM", "AM", "PM" },
            new string?[] { "Mon", null, "Tue", null },
        ];

        ChartDrawing drawing = Place(Levelled(merged) with { CategoryTicks = ChartTickMark.None });

        List<ChartLine> separators = [.. drawing.Lines.Where(line =>
            line.From.X == line.To.X
            && line.From.Y >= drawing.PlotArea.Bottom
            && line.To.Y > drawing.PlotArea.Bottom)];

        separators.Count.ShouldBe(1);
        separators[0].To.Y.ShouldBeGreaterThan(
            drawing.Labels.First(label => label.Text == "Mon").At.Y);
    }
}
