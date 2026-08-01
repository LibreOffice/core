using Paperless.Core.Charts;
using Paperless.Core.Numbers;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// What a data label says, as distinct from where it goes.
/// </summary>
/// <remarks>
/// The composition is worth pinning separately from the layout because it is the half a word
/// count sees and the half a position comparison cannot: a label in the wrong place is still the
/// right words, and a label reading <c>0.585714285714286</c> where the reference reads <c>59%</c>
/// is the right place and the wrong words. Every expected string here was read out of
/// LibreOffice's own PDF for the file it names.
/// </remarks>
public class ChartDataLabelTests
{
    /// <summary>
    /// The four fields go category, series, value, percentage — <c>VSeriesPlotter.cxx:563-596</c>.
    /// </summary>
    /// <remarks>
    /// Invisible in a word count, which is why it is asserted here: all four orders produce the
    /// same four words.
    /// </remarks>
    [Fact]
    public void TheFieldsAreJoinedInTheOrderCategorySeriesValuePercentage()
    {
        ChartDataLabel label = new()
        {
            ShowCategory = true,
            ShowSeries = true,
            ShowValue = true,
            ShowPercent = true,
            Separator = "; ",
        };

        label.Compose("Q1", "North", 25.0, 100.0).ShouldBe("Q1; North; 25; 25%");
    }

    /// <summary>
    /// The <c>General</c> on a percentage label becomes <c>0%</c> and not the raw fraction.
    /// </summary>
    /// <remarks>
    /// <c>ObjectFormatter::convertNumberFormat</c> substitutes <c>0%</c> for a stated
    /// <c>General</c> whenever the label shows a percentage
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx:1128</c>). Measured against
    /// LibreOffice's PDF for <c>percentage-number-formats.pptx</c>: its first pie point states
    /// <c>General</c> against 8.2 of 14, and the reference draws <c>8.2; 59%</c>.
    /// </remarks>
    [Fact]
    public void AGeneralFormatOnAPercentageLabelIsWrittenAsWholePerCent()
    {
        ChartDataLabel label = new() { ShowValue = true, ShowPercent = true, Separator = "; " };

        label.Compose(null, null, 8.2, 14.0).ShouldBe("8.2; 59%");
    }

    /// <summary>A stated percentage format is honoured to its decimals.</summary>
    [Fact]
    public void AStatedPercentageFormatIsUsedForTheShare()
    {
        ChartDataLabel label = new()
        {
            ShowValue = true,
            ShowPercent = true,
            Separator = "; ",
            PercentFormat = NumberFormatCode.Parse("0.00%"),
        };

        label.Compose(null, null, 3.2, 14.0).ShouldBe("3.2; 22.86%");
    }

    /// <summary>A custom label's fields are resolved rather than drawn as placeholders.</summary>
    /// <remarks>
    /// The placeholder is localised — <c>[WARTOŚĆ]</c> in
    /// <c>CustomDataLabel_tdf115107.pptx</c> — so drawing the run's own text draws a Polish word
    /// on an English chart, which is the failure this guards.
    /// </remarks>
    [Fact]
    public void ACustomLabelResolvesItsFieldsAndKeepsItsLiterals()
    {
        ChartDataLabel label = new()
        {
            Parts =
            [
                new ChartLabelPart(ChartLabelField.Literal, "90.0 = "),
                new ChartLabelPart(ChartLabelField.Value, "[WARTOŚĆ]"),
            ],
        };

        label.Compose("B", "Series 1", 90.0, 300.0).ShouldBe("90.0 = 90");
    }

    /// <summary>A custom label ignores the four flags entirely.</summary>
    [Fact]
    public void ACustomLabelReplacesTheFieldsRatherThanJoiningThem()
    {
        ChartDataLabel label = new()
        {
            ShowValue = true,
            ShowCategory = true,
            Parts = [new ChartLabelPart(ChartLabelField.Category, "[CAT]")],
        };

        label.Compose("Bbb", "North", 12.0, 100.0).ShouldBe("Bbb");
    }

    /// <summary>An empty label draws nothing, which is what a deleted one is.</summary>
    [Fact]
    public void ALabelShowingNothingDrawsNothing()
    {
        new ChartDataLabel().Draws.ShouldBeFalse();
        new ChartDataLabel().Compose("Q1", "North", 1.0, 2.0).ShouldBeNull();
    }

    /// <summary>
    /// A tick is written through the axis' format when it has one and by the general rule when not.
    /// </summary>
    /// <remarks>
    /// The general case is the one the corpus exercises and the formatted case is the one the
    /// layering move existed for: <c>percentage-number-formats.pptx</c>'s value axis states
    /// <c>0.0%</c>, and its ticks read <c>0.0% 0.5% 1.0% …</c> in LibreOffice's PDF where the
    /// shortest round-trip form gives <c>0 0.005 0.01 …</c>.
    /// </remarks>
    [Theory]
    [InlineData(180.0, null, "180")]
    [InlineData(0.0, null, "0")]
    [InlineData(2.5, null, "2.5")]
    [InlineData(0.005, "0.0%", "0.5%")]
    [InlineData(1200000.0, "#,##0", "1,200,000")]
    public void ATickTakesTheAxisFormatWhereThereIsOne(double value, string? code, string expected)
    {
        NumberFormatCode? format = code is null ? null : NumberFormatCode.Parse(code);
        ChartDataLabel.Write(value, format).ShouldBe(expected);
    }

    /// <summary>
    /// A numeric category label goes through the category axis' format; a textual one does not.
    /// </summary>
    /// <remarks>
    /// A date axis caches its categories as serials — <c>bnc889755.pptx</c>'s sixteen are
    /// <c>41183</c> upwards — and the axis states the format that turns them into month names.
    /// A text axis' labels are already strings and must survive untouched, which is why the parse
    /// is a test rather than an assumption.
    /// </remarks>
    [Fact]
    public void ANumericCategoryIsFormattedAndATextualOneIsLeftAlone()
    {
        NumberFormatCode format = NumberFormatCode.Parse("MMM-YY");

        ChartDataLabel.WriteCategory("41183", format).ShouldBe("Oct-12");
        ChartDataLabel.WriteCategory("Q1", format).ShouldBe("Q1");
        ChartDataLabel.WriteCategory("41183", null).ShouldBe("41183");
    }
}
