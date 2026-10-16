using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The parts of a worksheet's legacy VML drawing that every reader of one needs: the two
/// namespaces, the <c>x:ClientData/x:Anchor</c> cell anchor, and CSS visibility.
/// </summary>
/// <remarks>
/// <para>
/// A sheet can reach VML twice over — a comment's caption
/// (<see cref="XlsxNoteCaptions"/>) and a legacy picture or control
/// (<see cref="XlsxLegacyPictures"/>) sit in the same part, told apart only by
/// <c>x:ClientData/@ObjectType</c> — so the anchor arithmetic lives here rather than in
/// whichever of the two was written first.
/// </para>
/// <para>
/// <strong>A VML anchor's offsets are pixels, not EMUs.</strong>
/// <c>ShapeAnchor::importVmlAnchor</c> sets <c>CellAnchorType::Pixel</c>
/// (<c>sc/source/filter/oox/drawingbase.cxx:152-155</c>) where the DrawingML anchor's are EMUs,
/// and <c>calcCellAnchorEmu</c> scales them through <c>Unit::ScreenX</c>. Reading them as EMUs
/// puts every legacy object within a rounding error of its own cell's corner.
/// </para>
/// </remarks>
internal static class XlsxVml
{
    /// <summary>The <c>v:</c> namespace.</summary>
    public const string Namespace = OoxmlNamespaces.Vml;

    /// <summary>The <c>x:</c> namespace, VML's Excel extensions.</summary>
    public const string ExcelNamespace = "urn:schemas-microsoft-com:office:excel";

    /// <summary>The <c>o:</c> namespace, VML's Office extensions.</summary>
    public const string OfficeNamespace = OoxmlNamespaces.VmlOffice;

    /// <summary>How many of a VML anchor's pixels make an inch.</summary>
    /// <remarks>
    /// <c>UnitConverter</c>'s <c>Unit::ScreenX</c>, which is the reference device's resolution and
    /// is 96 on every platform Calc runs headless on. Measured rather than assumed: an anchor
    /// stating a row offset of 111 pixels comes back as 83.25 pt in the flat-ODF export, and
    /// 111 / 96 x 72 is 83.25 exactly.
    /// </remarks>
    public const double PixelsPerInch = 96;

    /// <summary>A screen pixel offset as a length.</summary>
    public static Length Pixels(int pixels) => Length.FromInches(pixels / PixelsPerInch);

    /// <summary>
    /// The eight comma-separated numbers of an <c>x:Anchor</c>, as two cell points.
    /// </summary>
    /// <remarks>
    /// The order is column, column offset, row, row offset, twice —
    /// <c>ShapeAnchor::importVmlAnchor</c> assigns them in exactly that sequence
    /// (<c>drawingbase.cxx:157-172</c>) — which is <em>not</em> the row-first order the
    /// <c>x:Row</c> and <c>x:Column</c> elements beside it are written in.
    /// </remarks>
    public static (SheetCellPoint From, SheetCellPoint To)? ParseAnchor(string? anchor)
    {
        if (anchor is null) return null;

        string[] parts = anchor.Split(',');
        if (parts.Length < 8) return null;

        Span<int> values = stackalloc int[8];
        for (int at = 0; at < 8; at++)
        {
            if (!int.TryParse(parts[at].Trim(), NumberStyles.Integer,
                              CultureInfo.InvariantCulture, out values[at]))
            {
                return null;
            }
        }

        if (values[0] < 0 || values[2] < 0 || values[4] < 0 || values[6] < 0) return null;

        return (new SheetCellPoint(values[0], Pixels(values[1]), values[2], Pixels(values[3])),
                new SheetCellPoint(values[4], Pixels(values[5]), values[6], Pixels(values[7])));
    }

    /// <summary>
    /// Whether a VML shape's style says it is shown.
    /// </summary>
    /// <remarks>
    /// A shape stating no <c>visibility</c> at all is shown, which is CSS's own default and what
    /// <c>ShapeTypeModel</c> initialises <c>mbVisible</c> to
    /// (<c>oox/source/vml/vmlshape.cxx</c>, <c>ShapeTypeModel::ShapeTypeModel</c>). A hidden one is
    /// given <c>Printable = false</c> as well as <c>Visible = false</c>
    /// (<c>vmlshape.cxx:897-901</c>), so it reaches no printed page and no print area.
    /// </remarks>
    public static bool IsVisible(string? style)
    {
        if (style is null) return true;

        foreach (string declaration in style.Split(';'))
        {
            int colon = declaration.IndexOf(':', StringComparison.Ordinal);
            if (colon < 0) continue;

            if (!declaration.AsSpan(0, colon).Trim().Equals("visibility", StringComparison.Ordinal))
                continue;

            return !declaration.AsSpan(colon + 1).Trim().Equals("hidden", StringComparison.Ordinal);
        }

        return true;
    }

    /// <summary>A shape's <c>x:ClientData</c>, or null when it has none.</summary>
    public static XElement? ClientData(XElement shape)
        => shape.Element(XName.Get("ClientData", ExcelNamespace));

    /// <summary>The <c>ObjectType</c> of a shape's client data, or null.</summary>
    public static string? ObjectType(XElement? client)
        => client?.Attribute("ObjectType")?.Value;
}
