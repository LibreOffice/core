using Paperless.Core.Graphics;

namespace Paperless.Core.Charts;

/// <summary>
/// The table of numbers a chart draws under its plot area.
/// </summary>
/// <remarks>
/// <para>
/// <c>c:dTable</c>, whose four booleans are all the file states about it — the contents are the
/// chart's own categories and series, so there is nothing to read but which lines are drawn
/// (<c>oox/source/drawingml/chart/datatablecontext.cxx:41-70</c>,
/// <c>datatablemodel.hxx:28-46</c>). ODF has no counterpart at all.
/// </para>
/// <para>
/// <strong>Every one of the four defaults to false, unlike the data-label flags beside them.</strong>
/// <c>DataTableModel</c>'s constructor initialises all four to <c>false</c> outright rather than to
/// <c>!bMSO2007Doc</c>, and <c>DataTableContext</c> reads each as
/// <c>getBool(XML_val, false)</c> — so the rule that caught out the <c>c:show*</c> flags does not
/// apply here, and assuming it does draws a grid on every data table that states none.
/// </para>
/// </remarks>
/// <param name="HorizontalBorders">Whether a line is drawn between rows.</param>
/// <param name="VerticalBorders">Whether a line is drawn between columns.</param>
/// <param name="Outline">Whether the table is boxed in.</param>
/// <param name="Keys">Whether each row is preceded by its series' colour key.</param>
/// <param name="Line">The colour the borders are drawn in.</param>
public readonly record struct ChartDataTable(
    bool HorizontalBorders,
    bool VerticalBorders,
    bool Outline,
    bool Keys,
    Colour Line);
