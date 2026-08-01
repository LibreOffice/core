using System.Globalization;
using Paperless.Core.Numbers;

namespace Paperless.Core.Charts;

/// <summary>Where a data label sits relative to the mark it names.</summary>
/// <remarks>
/// <c>com::sun::star::chart::DataLabelPlacement</c>, reached from OOXML's <c>c:dLblPos</c> and
/// ODF's <c>chart:label-position</c>. The default is per chart type rather than global —
/// <c>OUTSIDE</c> for a bar, <c>RIGHT</c> for a line or a scatter, <c>CENTER</c> for an area and
/// <c>AVOID_OVERLAP</c> for a pie
/// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:95-107</c>) — which is why
/// <see cref="ChartDataLabel.Placement"/> is nullable and resolved against the series' geometry.
/// </remarks>
public enum ChartLabelPlacement
{
    /// <summary>Beyond the end of the mark — <c>c:dLblPos val="outEnd"</c>.</summary>
    Outside = 0,

    /// <summary>Just inside the end — <c>inEnd</c>.</summary>
    Inside,

    /// <summary>In the middle of the mark — <c>ctr</c>.</summary>
    Centre,

    /// <summary>At the baseline end — <c>inBase</c>.</summary>
    NearOrigin,

    /// <summary>Above — <c>t</c>.</summary>
    Top,

    /// <summary>Below — <c>b</c>.</summary>
    Bottom,

    /// <summary>To the left — <c>l</c>.</summary>
    Left,

    /// <summary>To the right — <c>r</c>.</summary>
    Right,

    /// <summary>Wherever it fits — <c>bestFit</c>, which a pie defaults to.</summary>
    BestFit,
}

/// <summary>What one run of a custom data label stands for.</summary>
/// <remarks>
/// <c>DataPointCustomLabelFieldType</c>, reached from a <c>a:fld/@type</c> inside the label's
/// <c>c:rich</c>. A custom label is a little template rather than a string — <c>90.0 = </c>
/// followed by a <c>VALUE</c> field — and the field's own <c>a:t</c> holds a localised
/// placeholder such as <c>[WARTOŚĆ]</c>, so drawing the text verbatim draws the placeholder.
/// </remarks>
public enum ChartLabelField
{
    /// <summary>Text the file states outright.</summary>
    Literal = 0,

    /// <summary>The point's value.</summary>
    Value,

    /// <summary>The category's name.</summary>
    Category,

    /// <summary>The series' name.</summary>
    Series,

    /// <summary>The point's share of the series total.</summary>
    Percentage,

    /// <summary>Text from a cell the label points at, already cached in the run.</summary>
    CellRange,

    /// <summary>A line break.</summary>
    NewLine,
}

/// <summary>One run of a custom data label.</summary>
/// <param name="Field">What it stands for.</param>
/// <param name="Text">
/// Its literal text, which for a field is the placeholder and is used only when the field cannot
/// be resolved — a <see cref="ChartLabelField.CellRange"/>, whose cached string is all there is.
/// </param>
public readonly record struct ChartLabelPart(ChartLabelField Field, string Text);

/// <summary>
/// What one data label shows and how it is written.
/// </summary>
/// <remarks>
/// <para>
/// A port of the two halves LibreOffice keeps apart: <c>DataPointLabel</c>, which says which of
/// the four fields are shown, and the pair of number-format properties
/// (<c>NumberFormat</c> and <c>PercentageNumberFormat</c>) that say how the two numeric ones are
/// written. Both are read per series and overridden per point, which is exactly the
/// <c>c:dLbls</c>/<c>c:dLbl</c> nesting.
/// </para>
/// <para>
/// <strong>The percentage format wins over the value format, and that is not a tie-break —
/// it is which property the file's one <c>c:numFmt</c> lands in.</strong>
/// <c>ObjectFormatter::convertNumberFormat</c>
/// (<c>oox/source/drawingml/chart/objectformatter.cxx:1118-1148</c>) puts the stated code into
/// <c>PercentageNumberFormat</c> whenever the label shows a percentage and the format is not
/// source-linked, and substitutes <c>0%</c> for a code of <c>General</c> in that case. Measured on
/// <c>chart2/qa/extras/data/pptx/percentage-number-formats.pptx</c>, whose four pie points state
/// <c>General</c>, <c>0.00%</c>, <c>0%</c> and <c>0.00%</c> against values 8.2, 3.2, 1.4 and 1.2
/// summing to 14: LibreOffice draws <c>8.2; 59%</c>, <c>3.2; 22.86%</c>, <c>10%</c> and
/// <c>8.57%</c> — so the <c>General</c> on the first point produced <c>59%</c> and not
/// <c>0.585714…</c>, which is only explicable by the substitution.
/// </para>
/// </remarks>
public sealed record ChartDataLabel
{
    /// <summary>Whether the point's own value is shown — <c>c:showVal</c>.</summary>
    public bool ShowValue { get; init; }

    /// <summary>
    /// Whether the point's share of the series total is shown — <c>c:showPercent</c>.
    /// </summary>
    /// <remarks>
    /// Honoured by a pie and by nothing else: <c>bShowPercent</c> is ANDed with
    /// <c>meTypeCategory == TYPECATEGORY_PIE</c>
    /// (<c>oox/source/drawingml/chart/seriesconverter.cxx:141</c>), so a column chart whose
    /// <c>c:dLbls</c> asks for percentages gets values instead. Applying it everywhere puts a
    /// second number on every bar of several corpus decks.
    /// </remarks>
    public bool ShowPercent { get; init; }

    /// <summary>Whether the category name is shown — <c>c:showCatName</c>.</summary>
    public bool ShowCategory { get; init; }

    /// <summary>Whether the series name is shown — <c>c:showSerName</c>.</summary>
    public bool ShowSeries { get; init; }

    /// <summary>
    /// What joins the shown fields.
    /// </summary>
    /// <remarks>
    /// <c>c:separator</c>, defaulting to <c>"; "</c> — or to a newline when a percentage is shown
    /// without a value, which is what Office does and what
    /// <c>seriesconverter.cxx:168-172</c> reproduces.
    /// </remarks>
    public string Separator { get; init; } = "; ";

    /// <summary>Where the label goes, or null to take the series' geometry's default.</summary>
    public ChartLabelPlacement? Placement { get; init; }

    /// <summary>How the value is written, or null for the general format.</summary>
    public NumberFormatCode? ValueFormat { get; init; }

    /// <summary>How the percentage is written, or null for <c>0%</c>.</summary>
    public NumberFormatCode? PercentFormat { get; init; }

    /// <summary>
    /// The literal text a custom label states, which replaces all four fields.
    /// </summary>
    /// <remarks>
    /// <c>c:dLbl/c:tx/c:rich</c>, ODF's <c>chart:data-label</c> holding a <c>text:p</c>. A custom
    /// label may itself contain field runs that stand for the value or the category
    /// (<c>DataPointCustomLabelFieldType</c>), which are not resolved here: what is kept is the
    /// text the file states, which is what the reference draws for the plain case.
    /// </remarks>
    public string? Text { get; init; }

    /// <summary>
    /// The runs of a custom label whose text is a template rather than a string, or null.
    /// </summary>
    /// <remarks>
    /// Set instead of <see cref="Text"/> whenever the label's <c>c:rich</c> holds an
    /// <c>a:fld</c>. <c>VSeriesPlotter</c> sets the separator to the empty string for a custom
    /// label (<c>VSeriesPlotter.cxx:498</c>), because the template already states its own
    /// punctuation — which is how one file's labels read <c>…, 24.8%</c> and another's
    /// <c>…; 22%</c>.
    /// </remarks>
    public IReadOnlyList<ChartLabelPart>? Parts { get; init; }

    /// <summary>Whether this label draws anything at all.</summary>
    public bool Draws
        => Parts is { Count: > 0 }
           || Text is { Length: > 0 }
           || ShowValue || ShowPercent || ShowCategory || ShowSeries;

    /// <summary>The format a percentage takes when the file states none.</summary>
    /// <remarks>
    /// <c>DiagramHelper::getPercentNumberFormat</c> builds <c>0.00%</c> from the locale, and the
    /// OOXML importer substitutes a plain <c>0%</c> for a stated <c>General</c>
    /// (<c>objectformatter.cxx:1128</c>). The second is the one that fires on a real file, because
    /// a label showing a percentage almost always states a format for it.
    /// </remarks>
    private static readonly NumberFormatCode DefaultPercent = NumberFormatCode.Parse("0%");

    /// <summary>
    /// The text this label draws, or null when it draws nothing.
    /// </summary>
    /// <remarks>
    /// The four fields are assembled in <c>VSeriesPlotter::createDataLabel</c>'s order — category,
    /// series, value, percentage — and joined with the separator, skipping the empty ones
    /// (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx:563-647</c>). Getting the order wrong
    /// is invisible in a word count and obvious on the page.
    /// </remarks>
    /// <param name="category">The category's name, or null.</param>
    /// <param name="series">The series' name, or null.</param>
    /// <param name="value">The point's value.</param>
    /// <param name="total">
    /// The sum of the series' absolute values, for the percentage. Zero is treated as one, which
    /// is what LibreOffice does rather than dividing by zero.
    /// </param>
    public string? Compose(string? category, string? series, double value, double total)
    {
        if (Parts is { Count: > 0 } template) return Resolve(template, category, series, value, total);
        if (Text is { Length: > 0 } custom) return custom;

        List<string> parts = [];

        if (ShowCategory && category is { Length: > 0 }) parts.Add(category);
        if (ShowSeries && series is { Length: > 0 }) parts.Add(series);

        if (ShowValue)
        {
            parts.Add(ValueFormat is { } code && !code.IsGeneral
                ? NumberFormatter.Format(code, value)
                : NumberFormatter.General(value));
        }

        if (ShowPercent)
        {
            double sum = total == 0.0 || !double.IsFinite(total) ? 1.0 : total;
            double share = Math.Abs(value / sum);
            parts.Add(NumberFormatter.Format(PercentFormat ?? DefaultPercent, share));
        }

        return parts.Count == 0 ? null : string.Join(Separator, parts);
    }

    /// <summary>A custom label's template, with its fields filled in.</summary>
    private string Resolve(
        IReadOnlyList<ChartLabelPart> template,
        string? category,
        string? series,
        double value,
        double total)
    {
        System.Text.StringBuilder built = new();

        foreach (ChartLabelPart part in template)
        {
            switch (part.Field)
            {
                case ChartLabelField.Value:
                    built.Append(Write(value, ValueFormat));
                    break;

                case ChartLabelField.Category:
                    built.Append(category ?? string.Empty);
                    break;

                case ChartLabelField.Series:
                    built.Append(series ?? string.Empty);
                    break;

                case ChartLabelField.Percentage:
                {
                    double sum = total == 0.0 || !double.IsFinite(total) ? 1.0 : total;
                    built.Append(NumberFormatter.Format(
                        PercentFormat ?? DefaultPercent, Math.Abs(value / sum)));
                    break;
                }

                case ChartLabelField.NewLine:
                    built.Append('\n');
                    break;

                default:
                    built.Append(part.Text);
                    break;
            }
        }

        return built.ToString();
    }

    /// <summary>
    /// A tick or category label written through a format code, or in the general format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is the method the layering move existed for.</strong> The engine is
    /// <c>Paperless.Core.Numbers</c>, which used to sit in <c>Paperless.Spreadsheets</c> — above
    /// <c>Paperless.Core</c> — so a chart composed in Core could not reach it and every tick was
    /// written in its shortest round-trip form. That is right for a whole-number scale and wrong
    /// for every currency, percentage and date axis: <c>percentage-number-formats.pptx</c>'s value
    /// axis states <c>0.0%</c> and was drawn <c>0 0.005 0.01 …</c> against a reference reading
    /// <c>0.0% 0.5% 1.0% …</c>.
    /// </para>
    /// <para>
    /// The general case does <em>not</em> go through the format engine, because
    /// <c>General</c> is not a format code at all — <c>convertNumberFormat</c> asks the number
    /// formats supplier for its standard index instead of converting the string
    /// (<c>objectformatter.cxx:1132-1134</c>).
    /// </para>
    /// </remarks>
    /// <param name="value">The number.</param>
    /// <param name="format">The format code, or null for the general format.</param>
    public static string Write(double value, NumberFormatCode? format)
        => format is { } code && !code.IsGeneral
            ? NumberFormatter.Format(code, value)
            : NumberFormatter.General(value);

    /// <summary>
    /// A category label written through the category axis' format when it is a bare number.
    /// </summary>
    /// <remarks>
    /// A date axis states its categories as serial numbers in a <c>c:numCache</c> and its format
    /// as <c>c:dateAx/c:numFmt</c>, so the cache holds <c>41183</c> and the axis reads
    /// <c>Oct</c>. Measured on <c>chart2/qa/extras/data/pptx/bnc889755.pptx</c>, whose sixteen
    /// categories are month serials: without this the axis draws sixteen five-digit numbers, which
    /// is the whole of that deck's word-count error.
    /// </remarks>
    /// <param name="text">The cached label.</param>
    /// <param name="format">The category axis' format, or null.</param>
    public static string? WriteCategory(string? text, NumberFormatCode? format)
    {
        if (text is not { Length: > 0 }) return text;
        if (format is not { } code || code.IsGeneral) return text;

        return double.TryParse(
            text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? NumberFormatter.Format(code, value)
            : text;
    }
}
