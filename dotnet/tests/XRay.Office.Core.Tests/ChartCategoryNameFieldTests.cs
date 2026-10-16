using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What a data label's <c>[CATEGORY NAME]</c> field draws when the category is a number.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The axis' number format, not the source cell's.</strong>
/// <c>VSeriesPlotter::getCategoryName</c>
/// (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx</c>:2213-2224) is
/// <c>m_pExplicitCategoriesProvider-&gt;getSimpleCategories()[n]</c>, and those strings are built
/// by <c>ExplicitCategoriesProvider::convertCategoryAnysToText</c>
/// (<c>chart2/source/tools/ExplicitCategoriesProvider.cxx</c>:186-227), which resolves one number
/// format <em>before</em> its loop — from <c>getAxisByDimension2(0, 0)</c> through
/// <c>AxisHelper::getExplicitNumberFormatKeyForAxis</c> — and writes every numeric category
/// through it. A category that is already a string is passed straight out.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2</strong> on one-attribute variants of
/// <c>055_Project_timeline_with_milestones</c>, whose thirteen milestone labels are a
/// <c>CELLRANGE</c> field over a <c>CATEGORYNAME</c> field, whose <c>c:dateAx</c> states
/// <c>[$-409]d\ mmm;@</c> and whose source cells are <c>m/d/yyyy</c>. Rewriting the axis'
/// <c>formatCode</c> to <c>yyyy</c> draws them <c>2023</c>, to <c>mmmm</c> draws them
/// <c>April</c>, to <c>0.00</c> draws them <c>45021.00</c>; rewriting the <em>cells'</em> format
/// to either <c>yyyy</c> or <c>0.00</c> leaves the whole rendering byte-identical to the control,
/// and so does setting <c>sourceLinked="1"</c> on the axis.
/// <c>probes/chart-cap-r113</c> §3, <c>catname-055.py</c>.
/// </para>
/// <para>
/// <strong>The gate consequence is stated rather than buried.</strong> On the one corpus document
/// this reaches, correcting it takes column 9 from 960 to 934 against a reference reading 1045 —
/// a shortfall of 111 where it was 85 — because the reference's extra 23 date-axis labels are the
/// <c>TODAY()</c> confound (<c>CLAUDE.md</c>'s sixth) and have the opposite sign. With that
/// confound frozen out, both of round 112's static variants read <strong>934 on both sides</strong>
/// where this tree read 960 before. The verdict does not move either way; the column does.
/// </para>
/// </remarks>
public class ChartCategoryNameFieldTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.55 * text.Length), size * 1.15);
    }

    /// <summary>2023-04-05, 2023-04-24 and 2023-05-01 as 1900-system serials.</summary>
    private static readonly double[] Milestones = [45021.0, 45040.0, 45047.0];

    /// <summary>
    /// The milestone chart's shape: three points on a date axis, each labelled with nothing but
    /// its own category name.
    /// </summary>
    /// <param name="axisFormat">The <c>c:dateAx</c>'s own format code.</param>
    /// <param name="cached">
    /// The category text as the reader cached it — for an OOXML chart in a workbook this is the
    /// range resolved against the live sheet, so it arrives already written through the
    /// <em>cell's</em> format.
    /// </param>
    private static ChartPlot Timeline(string axisFormat, string[] cached)
    {
        NumberFormatCode format = NumberFormatCode.Parse(axisFormat);

        return new ChartPlot
        {
            Kind = ChartPlotKind.Line,
            Direction = ChartBarDirection.Column,
            Categories = cached,
            CategoryFormat = NumberFormatCode.Parse("m/d/yyyy"),
            LabelSize = Length.FromPoints(9),

            // Both axes off, so that what `Drawn` returns is the data labels and nothing else —
            // a date axis' own tick labels go through the same format and would otherwise be
            // indistinguishable from the field under test.
            ValueAxisVisible = false,
            CategoryAxisVisible = false,
            DateAxis = new ChartDateAxis(
                45021.0,
                45047.0,
                ChartTimeUnit.Day,
                new ChartTimeInterval(10, ChartTimeUnit.Day),
                [45021.0, 45031.0, 45041.0],
                [.. Milestones.Select(v => (double?)v)],
                format),
            Series =
            [
                new ChartSeries("Milestones", [1.0, 2.0, 3.0], Colour.FromRgb(0x4472C4))
                {
                    PointLabels =
                    [
                        CategoryOnly(), CategoryOnly(), CategoryOnly(),
                    ],
                },
            ],
        };
    }

    private static ChartDataLabel CategoryOnly() => new()
    {
        Parts = [new ChartLabelPart(ChartLabelField.Category, "[CATEGORYNAME]")],
    };

    private static string[] Drawn(ChartPlot plot)
        => [.. ChartLayout
            .Place(plot, new DocRect(
                Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200)),
                new Ruler())
            .Labels
            .Select(label => label.Text)];

    /// <summary>
    /// The field is written through the date axis' format, not through the text the cell gave.
    /// </summary>
    /// <remarks>
    /// <strong>Fails at the base</strong>, which appended <c>plot.Categories[index]</c> verbatim
    /// and drew <c>4/5/2023</c> where 26.2.4.2 draws <c>5 Apr</c>.
    /// </remarks>
    [Fact]
    public void ACategoryNameFieldIsWrittenThroughTheAxisFormat()
    {
        string[] drawn = Drawn(
            Timeline("[$-409]d\\ mmm;@", ["4/5/2023", "4/24/2023", "5/1/2023"]));

        drawn.ShouldContain("5 Apr");
        drawn.ShouldContain("24 Apr");
        drawn.ShouldContain("1 May");
        drawn.ShouldNotContain("4/5/2023");
    }

    /// <summary>
    /// And it follows the axis when the axis changes — the arm that separates the axis' format
    /// from any other source of one.
    /// </summary>
    [Fact]
    public void ItFollowsTheAxisFormatWhenThatChanges()
    {
        Drawn(Timeline("yyyy", ["4/5/2023", "4/24/2023", "5/1/2023"]))
            .ShouldAllBe(text => text == "2023");

        Drawn(Timeline("0.00", ["4/5/2023", "4/24/2023", "5/1/2023"]))
            .ShouldContain("45021.00");
    }

    /// <summary>
    /// The text the cell gave decides nothing: the same axis over differently formatted cached
    /// text draws the same labels.
    /// </summary>
    /// <remarks>
    /// The counterpart of <c>catname-055.py</c>'s <c>cell_yyyy</c> and <c>cell_hash</c> arms,
    /// which leave 26.2.4.2's rendering byte-identical to the control.
    /// </remarks>
    [Fact]
    public void TheCachedCellTextDecidesNothing()
    {
        Drawn(Timeline("[$-409]d\\ mmm;@", ["45021", "45040", "45047"]))
            .ShouldBe(Drawn(Timeline("[$-409]d\\ mmm;@", ["4/5/2023", "4/24/2023", "5/1/2023"])));
    }

    /// <summary>
    /// A string category is passed straight out, which is the branch
    /// <c>convertCategoryAnysToText</c> takes for an <c>Any</c> that is not a double.
    /// </summary>
    [Fact]
    public void AStringCategoryIsDrawnAsItStands()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Bar,
            Direction = ChartBarDirection.Column,
            Categories = ["Kickoff", "Design", "Build"],
            LabelSize = Length.FromPoints(9),
            ValueAxisVisible = false,
            CategoryAxisVisible = false,
            Series =
            [
                new ChartSeries("Milestones", [1.0, 2.0, 3.0], Colour.FromRgb(0x4472C4))
                {
                    PointLabels = [CategoryOnly(), CategoryOnly(), CategoryOnly()],
                },
            ],
        };

        string[] drawn = Drawn(plot);

        drawn.ShouldContain("Kickoff");
        drawn.ShouldContain("Design");
        drawn.ShouldContain("Build");
    }
}
