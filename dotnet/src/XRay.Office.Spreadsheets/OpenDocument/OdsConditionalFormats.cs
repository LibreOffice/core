using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// The blocks a sheet's conditional formats cover.
/// </summary>
/// <remarks>
/// <para>
/// Only the ranges, because only the ranges decide anything here: a cell whose pattern carries a
/// non-empty <c>ATTR_CONDITIONAL</c> is measured rather than computed when its row height is
/// recovered, and Calc tests the attribute rather than the condition
/// (<c>ScColumn::GetOptimalHeight</c>, <c>sc/source/core/data/column2.cxx:937-941</c>). See
/// <see cref="SheetLayout.ConditionalRanges"/>.
/// </para>
/// <para>
/// <strong>The spelling is the extension namespace's, and reading it alone is exact rather than
/// a compromise.</strong> ODF 1.2 states a conditional format as a <c>style:map</c> on the cell's
/// own style; LibreOffice writes <em>both</em>, from one model and in one pass —
/// <c>ScXMLExport::ExportConditionalFormat</c> walks the sheet's
/// <c>ScConditionalFormatList</c> into <c>calcext:conditional-formats</c>
/// (<c>sc/source/filter/xml/xmlexprt.cxx</c>:4779-4800) and
/// <c>ScXMLAutoStylePoolP::exportStyleContent</c> writes a <c>style:map</c> onto every cell style
/// the same formats reach (<c>sc/source/filter/xml/xmlstyle.cxx</c>:700-810). So the two are the
/// same set of cells by construction.
/// </para>
/// <para>
/// <strong>Measured over the 307 converted <c>.ods</c>: 53 documents carry a conditional
/// <c>style:map</c>, every one of the 53 also states <c>calcext:conditional-format</c>, and
/// <strong>0 of them hold a single mapped cell outside a <c>calcext:target-range-address</c></strong>
/// — so reading the specification's spelling would reach nothing.</strong>
/// <c>probes/ods-residue-r95/stylemap-cover.py</c>.
/// </para>
/// <para>
/// <em>The figure this replaces — "one document states a <c>style:map</c> and no <c>calcext:</c>",
/// <c>probes/ods-notes-r92/results.md</c> §2 — counted the wrong element.</em> The identical
/// element name is also how a <c>number:*-style</c> states its positive, negative and zero
/// sub-formats, and a conditional cell format is the one that sits on a
/// <c>style:style style:family="table-cell"</c> and carries <c>style:base-cell-address</c>. The
/// document that census named, <c>2025_Active_Civil_Airmen_Statistics_FINAL.ods</c>, states
/// <strong>23 number-format maps and no conditional format at all</strong>, which is also why the
/// rule would not have closed it. <c>probes/ods-residue-r95/stylemap-census.py</c>.
/// </para>
/// <para>
/// <c>calcext:target-range-address</c> is a space-separated list in the same OOO syntax
/// <c>table:print-ranges</c> uses, so it goes through the same two helpers.
/// </para>
/// </remarks>
internal static class OdsConditionalFormats
{
    /// <summary>Reads the ranges every conditional format of one sheet covers.</summary>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static IReadOnlyList<SheetRange> ReadRanges(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        XElement? formats =
            table.Element(XName.Get("conditional-formats", OdfNamespaces.CalcExt));
        if (formats is null) return [];

        List<SheetRange> ranges = [];

        foreach (XElement format in
                 formats.Elements(XName.Get("conditional-format", OdfNamespaces.CalcExt)))
        {
            string? stated =
                format.Attribute(XName.Get("target-range-address", OdfNamespaces.CalcExt))?.Value;
            if (string.IsNullOrEmpty(stated)) continue;

            foreach (string part in SheetAddress.SplitList(stated, ' '))
            {
                if (SheetAddress.TryParseRange(part, out SheetRange range)) ranges.Add(range);
            }
        }

        return ranges;
    }
}
