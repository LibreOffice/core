namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// The cells behind one chart data sequence, read out of the document that holds the chart.
/// </summary>
/// <param name="Text">
/// One displayed string per cell, in the range's own order. Null where the cell is empty or does
/// not exist, which is what an unlabelled category means.
/// </param>
/// <param name="Numbers">
/// One number per cell, in the same order and of the same length. Null where the cell holds no
/// number — text, an error, or nothing — which is the missing point every plotter has to skip
/// rather than plot as zero.
/// </param>
public sealed record ChartRangeValues(IReadOnlyList<string?> Text, IReadOnlyList<double?> Numbers);

/// <summary>
/// Resolves a chart data sequence's <c>c:f</c> against the live cells it names.
/// </summary>
/// <param name="formula">
/// The <c>c:f</c> text as the file states it, sheet qualifier and dollars included —
/// <c>'Literature Mapping'!$B$4:$B$16</c>.
/// </param>
/// <returns>
/// The cells' values, or null when the reference cannot be resolved.
/// <para>
/// <strong>Null and empty are different answers and a caller must keep them apart.</strong> Null
/// means the reference could not be resolved at all — a multi-area reference, a defined name, an
/// external workbook, a sheet this reader does not hold — and is the state
/// <c>ExcelChartConverter::createDataSequence</c> reaches by throwing, where the cached points
/// stand. An <em>empty</em> sequence means the reference resolved and named no readable cell,
/// which Calc really does produce: a range every cell of which is an Excel table's totals row is
/// skipped outright by <c>ScChart2DataSequence::BuildDataCache</c>
/// (<c>sc/source/ui/unoobj/chart2uno.cxx:2616-2632</c>), leaving a series with no points and a
/// value axis at its default scale. Falling back to the cache there would draw a plot the
/// reference leaves blank.
/// </para>
/// </returns>
/// <remarks>
/// <para>
/// <strong>This is the seam between the two data providers LibreOffice keeps, and it exists
/// because the split is real.</strong> The base converter,
/// <c>ChartConverter::createDataSequence</c>
/// (<c>oox/source/drawingml/chart/chartconverter.cxx:117-152</c>), reads the cached points and
/// ignores the formula — right for Impress and Writer, where the data lives in a second document
/// this reader must not open. Calc overrides it:
/// <c>ExcelChartConverter::createDataSequence</c>
/// (<c>sc/source/filter/oox/excelchartconverter.cxx:65-105</c>) parses the formula and resolves
/// it against the workbook, and falls back to the cache <em>only</em> when there is no formula at
/// all. The formula wins outright; the cache is the fallback, not the source.
/// </para>
/// <para>
/// <strong>The difference is not academic.</strong> A workbook whose chart plots a pivot table
/// writes a cache one point shorter than the range it declares — the grand-total row is stated in
/// <c>c:f</c> and absent from <c>c:numCache</c> — so a cache-only reader draws the chart without
/// its largest value and scales the axis to the rest. Measured on
/// <c>Keywords_Mapping_Graphs_and_Charts.xlsx</c>: eleven charts, twenty-two sequences, every one
/// of them exactly one cell short, and a value axis running to 8 where the reference runs to 40.
/// </para>
/// <para>
/// A <see langword="null"/> resolver — the default everywhere but the spreadsheet readers — leaves
/// the cache-only behaviour exactly as it was, which is what the presentation and word-processing
/// paths need and is why this is a parameter rather than a change of rule.
/// </para>
/// </remarks>
public delegate ChartRangeValues? ChartRangeResolver(string formula);
