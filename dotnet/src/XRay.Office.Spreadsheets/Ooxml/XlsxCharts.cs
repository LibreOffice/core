using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Containers.Ooxml;
using XRay.Office.Core.Extraction;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The charts anchored on a worksheet, as content rather than as geometry.
/// </summary>
/// <remarks>
/// <para>
/// The same drawing part <see cref="XlsxDrawings"/> walks for the rendering path, walked again
/// for the one thing extraction wants out of it. Two walks rather than one because the two
/// paths want different things and run at different times: extraction must not pay for the
/// anchor arithmetic, and a caller who only ever calls <c>Content</c> never opens a chart part
/// it did not need. The parts are cached by the package either way.
/// </para>
/// <para>
/// <strong>Three relationships deep, and each hop is from a different part.</strong> The sheet
/// declares a <c>drawing</c> relationship; the drawing declares a <c>chart</c> relationship per
/// graphic frame; and <c>c:chart/@r:id</c> inside the frame is resolved against the
/// <em>drawing</em> part, not the sheet. Resolving it against the sheet finds nothing, or —
/// worse, in a workbook whose sheet happens to declare an <c>rId1</c> of its own — finds the
/// wrong part.
/// </para>
/// </remarks>
internal static class XlsxCharts
{
    private const string DrawingNamespace = OoxmlNamespaces.DrawingMLSpreadsheet;

    /// <summary>Reads the charts anchored on one sheet, in the drawing's own order.</summary>
    /// <param name="package">The workbook's package.</param>
    /// <param name="sheetPartName">The worksheet part the drawing hangs off.</param>
    /// <param name="ranges">
    /// Resolves a sequence's <c>c:f</c> against the workbook's cells, so that the table this
    /// yields holds the same points the picture is drawn from. See <see cref="ChartRangeResolver"/>
    /// and <see cref="XlsxChartRanges.Resolver"/>, which binds one chart's <c>c:plotVisOnly</c>.
    /// </param>
    public static IEnumerable<ContentSection> Read(
        IPackage package, string? sheetPartName, XlsxChartRanges? ranges = null)
    {
        ArgumentNullException.ThrowIfNull(package);
        if (sheetPartName is null || package is not OpcPackage opc) yield break;

        // Once per sheet rather than once per chart: it reads docProps/app.xml, and it decides
        // only the c:autoTitleDeleted default. See DrawingChartTitle.
        bool office2007 = OoxmlMetadata.IsOffice2007(opc);

        foreach (OpcXml.Relationship relationship in opc.GetRelationshipsByType(
                     OoxmlNamespaces.Relationships + "/drawing", sheetPartName))
        {
            if (relationship.IsExternal) continue;
            if (opc.GetPart(relationship.Target) is not { } part) continue;

            XElement? root;
            using (Stream content = part.Open()) root = OoxmlXml.TryLoad(content, out _);
            if (root is null) continue;

            Dictionary<string, OpcXml.Relationship> targets = new(StringComparer.Ordinal);
            foreach (OpcXml.Relationship declared in opc.GetRelationships(part.Name))
                targets[declared.Id] = declared;

            foreach (XElement frame in root.Descendants(XName.Get("graphicFrame", DrawingNamespace)))
            {
                XElement? data = frame
                    .Element(XName.Get("graphic", OoxmlNamespaces.DrawingML))
                    ?.Element(XName.Get("graphicData", OoxmlNamespaces.DrawingML));

                if (Drawing.Attribute(data, "uri") != DrawingChart.ChartUri) continue;

                string? id = data!
                    .Element(XName.Get("chart", OoxmlNamespaces.DrawingMLChart))
                    ?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

                if (id is null || !targets.TryGetValue(id, out OpcXml.Relationship chart)) continue;
                if (chart.IsExternal || opc.GetPart(chart.Target) is not { } chartPart) continue;

                XElement? chartSpace;
                using (Stream content = chartPart.Open())
                    chartSpace = OoxmlXml.TryLoad(content, out _);

                if (chartSpace is null) continue;

                // Bound per chart, not per workbook: c:plotVisOnly decides whether a hidden
                // row's cells are chart data, and it is stated on the chart part.
                ChartRangeResolver? resolver = ranges?.Resolver(
                    DrawingChart.PlotsVisibleCellsOnly(chartSpace, office2007));

                if (DrawingChart.Read(chartSpace, resolver, office2007) is { } section)
                    yield return section;
            }
        }
    }
}
