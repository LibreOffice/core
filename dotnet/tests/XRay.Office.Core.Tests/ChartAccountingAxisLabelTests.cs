using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// An accounting format on a value axis, which is the one place the zero subformat's
/// <c>?</c> placeholders are load-bearing.
/// </summary>
/// <remarks>
/// <para>
/// Excel's accounting formats hold the dash of their zero row apart from the column's right edge
/// with two <c>?</c> — <c>_("$"* "-"??_)</c> — so that the dash lines up with the decimal point
/// of the rows above it. Written with ordinary blanks instead, those two placeholders are
/// narrower than a digit <em>and</em> are trailing whitespace, which a right-aligned axis label
/// drops altogether: the tick then reads <c>$-</c> hard against the axis where the reference
/// reads <c>$-</c> a digit and a half clear of it.
/// </para>
/// <para>
/// Measured on <c>Demick_JetBlue.pptx</c> page 5 against 26.2.4.2, which draws
/// <c>&#x20;$-&#x2007;&#x2007;&#x20;</c> with its dash at x = 144.14 and every other label on the
/// axis right-aligned at 162.83. Ours put the same dash at 155.85; with the figure spaces it is
/// at 143.67, which is 0.47 pt from the reference and the rest of that is the chart's own scale.
/// </para>
/// </remarks>
public class ChartAccountingAxisLabelTests
{
    /// <summary>Half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    /// <summary>The code <c>Demick_JetBlue.pptx</c>'s value axis states, verbatim.</summary>
    private const string Accounting =
        "_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)";

    private static ChartPlot Plot() => new()
    {
        Categories = ["Q1", "Q2"],
        Series = [new ChartSeries("Revenue", [0.0, 200000.0], Colour.FromRgb(0x99CCFF))],
        ValueFormat = NumberFormatCode.Parse(Accounting),
        ValueScale = new ChartScaleRequest(0.0, 200000.0, 100000.0),
    };

    /// <summary>The zero tick shows the dash the zero subformat states, and its two blanks.</summary>
    /// <remarks>
    /// The dash alone is not the assertion worth making: it survives even when the placeholders
    /// are dropped, and dropping them is what moves it. So the label is asserted whole.
    /// </remarks>
    [Fact]
    public void TheZeroTickOfAnAccountingAxisShowsItsDashAndItsPlaceholders()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Plot(),
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300)),
            new Ruler());

        drawing.Labels.Select(label => label.Text)
               .ShouldContain(" $-\u2007\u2007 ");
    }

    /// <summary>
    /// The placeholders are as wide as a digit, so the zero label reserves as much room as a
    /// three-digit one.
    /// </summary>
    /// <remarks>
    /// This is the half that a dash-only fix would leave broken. The axis right-aligns its labels
    /// on their full width, so what puts the dash where the reference puts it is the two blanks
    /// after it having a digit's width each — ordinary spaces are narrower and, being blanks, are
    /// dropped from the end of the line as well.
    /// </remarks>
    [Fact]
    public void TheBlankPlaceholdersAreDigitWidthRatherThanSpaces()
    {
        string zero = NumberFormatter.Format(NumberFormatCode.Parse(Accounting), 0.0);

        zero.ShouldBe(" $-\u2007\u2007 ");

        // Two digit-width blanks from the `??`, and one ordinary blank from the `_)` after them.
        // The ordinary one is the only part of the tail a right-aligned line may drop.
        zero.Count(character => character == NumberFormatter.BlankDigit).ShouldBe(2);
        zero.TrimEnd(NumberFormatter.BlankDigit, ' ').ShouldBe(" $-");
    }
}
