using System.Globalization;
using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// The pictures and charts anchored on a worksheet: <c>xl/drawings/drawing*.xml</c>.
/// </summary>
/// <remarks>
/// <para>
/// Ported from <c>sc/source/filter/oox/drawingfragment.cxx</c> and the anchor arithmetic in
/// <c>drawingbase.cxx</c>. The part is reached through the <em>worksheet's</em> own
/// <c>drawing</c> relationship, never by name: <c>drawing1.xml</c> belongs to whichever sheet
/// points at it, which is not the first sheet in any workbook that has had a sheet deleted.
/// </para>
/// <para>
/// <strong>Three anchors, and they mean three different things.</strong>
/// <c>xdr:twoCellAnchor</c> states both corners as cells, so the drawing moves <em>and</em>
/// resizes when the grid does; <c>xdr:oneCellAnchor</c> states the top left as a cell and the
/// size as a length, so it moves and keeps its size; <c>xdr:absoluteAnchor</c> states a position
/// on the sheet and ignores the grid entirely. All three write their offsets and extents in EMUs,
/// which is the one place SpreadsheetML uses the same unit Paperless does.
/// </para>
/// <para>
/// <strong><c>editAs</c> is not read.</strong> LibreOffice honours it — a
/// <c>twoCellAnchor editAs="oneCell"</c> is imported with a fixed size
/// (<c>drawingfragment.cxx:284-287</c>) — but the attribute says how the drawing behaves when the
/// sheet is <em>edited</em>, and the rectangle it occupies on a printed page is the same either
/// way. Reading it would change nothing that is drawn.
/// </para>
/// </remarks>
internal static class XlsxDrawings
{
    private const string DrawingNamespace =
        "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";

    private const string MainNamespace =
        "http://schemas.openxmlformats.org/drawingml/2006/main";

    private const string RelationshipNamespace =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    private const string ChartUri = "http://schemas.openxmlformats.org/drawingml/2006/chart";

    /// <summary>Reads the drawings anchored on one sheet.</summary>
    /// <param name="package">The workbook's package.</param>
    /// <param name="sheetPartName">The worksheet part the drawing hangs off.</param>
    public static SheetDrawings Read(IPackage package, string? sheetPartName)
    {
        ArgumentNullException.ThrowIfNull(package);
        if (sheetPartName is null || package is not OpcPackage opc) return SheetDrawings.Empty;

        List<SheetDrawing> drawings = [];

        foreach (OpcXml.Relationship relationship in opc.GetRelationshipsByType(
                     RelationshipNamespace + "/drawing", sheetPartName))
        {
            if (relationship.IsExternal) continue;
            if (opc.GetPart(relationship.Target) is not { } part) continue;

            XElement? root;
            using (Stream content = part.Open())
            {
                root = OoxmlXml.TryLoad(content, out _);
            }

            if (root is null) continue;

            Dictionary<string, OpcXml.Relationship> images = [];
            foreach (OpcXml.Relationship image in opc.GetRelationships(part.Name))
                images[image.Id] = image;

            foreach (XElement anchor in root.Elements())
            {
                if (anchor.Name.NamespaceName != DrawingNamespace) continue;

                SheetAnchorKind? kind = anchor.Name.LocalName switch
                {
                    "twoCellAnchor" => SheetAnchorKind.TwoCell,
                    "oneCellAnchor" => SheetAnchorKind.OneCell,
                    "absoluteAnchor" => SheetAnchorKind.Absolute,
                    _ => null,
                };

                if (kind is not { } anchored) continue;
                if (ReadAnchor(anchor, anchored, opc, images) is { } drawing) drawings.Add(drawing);
            }
        }

        return drawings.Count == 0 ? SheetDrawings.Empty : new SheetDrawings(drawings);
    }

    private static SheetDrawing? ReadAnchor(
        XElement anchor,
        SheetAnchorKind kind,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> images)
    {
        XElement? picture = Child(anchor, DrawingNamespace, "pic");
        XElement? frame = Child(anchor, DrawingNamespace, "graphicFrame");

        if (picture is null && frame is null) return null;

        SheetDrawing drawing = new()
        {
            Anchor = kind,
            From = Point(Child(anchor, DrawingNamespace, "from")),
            To = Point(Child(anchor, DrawingNamespace, "to")),
            Extent = Size(Child(anchor, DrawingNamespace, "ext")),
            Position = Position(Child(anchor, DrawingNamespace, "pos")),
        };

        XElement? properties = Child(
            Child(picture ?? frame, DrawingNamespace, picture is not null ? "nvPicPr" : "nvGraphicFramePr"),
            DrawingNamespace,
            "cNvPr");

        drawing = drawing with
        {
            Name = Attribute(properties, "name"),
            Description = Attribute(properties, "descr"),

            // hidden="1" on cNvPr, which is what Excel writes for a shape the user has hidden.
            IsHidden = Attribute(properties, "hidden") is "1" or "true",
        };

        if (frame is not null)
        {
            string? uri = Attribute(
                Child(Child(frame, DrawingNamespace, "graphic"), MainNamespace, "graphicData"),
                "uri");

            // Recorded and not painted: a chart's series and axes live in their own part and
            // reproducing one is a project of its own, but dropping it would make a sheet with a
            // chart indistinguishable from a sheet without.
            return drawing with { IsChart = uri == ChartUri };
        }

        return drawing with
        {
            Image = Load(
                package,
                images,
                Attribute(
                    Child(Child(picture, DrawingNamespace, "blipFill"), MainNamespace, "blip"),
                    XName.Get("embed", RelationshipNamespace))),
        };
    }

    /// <summary>
    /// Loads a picture's bytes, encoded.
    /// </summary>
    /// <remarks>
    /// <see cref="RasterImage.Encoded"/> and no decoding: the bytes are a PNG or a JPEG in the
    /// package and the only thing that can turn them into pixels is a codec, which lives in the
    /// rendering library. A reader that decoded would drag one into the extraction path.
    /// </remarks>
    private static RasterImage? Load(
        OpcPackage package, Dictionary<string, OpcXml.Relationship> images, string? id)
    {
        if (id is null || !images.TryGetValue(id, out OpcXml.Relationship relationship)) return null;
        if (relationship.IsExternal) return null;
        if (package.GetPart(relationship.Target) is not { } part) return null;

        using Stream content = part.Open();
        using MemoryStream buffer = new();
        content.CopyTo(buffer);

        return buffer.Length == 0
            ? null
            : RasterImage.Encoded(buffer.ToArray(), part.MediaType);
    }

    private static SheetCellPoint Point(XElement? element)
    {
        if (element is null) return default;

        return new SheetCellPoint(
            Integer(element, "col"),
            Length.FromEmu(Integer(element, "colOff")),
            Integer(element, "row"),
            Length.FromEmu(Integer(element, "rowOff")));
    }

    private static DocSize Size(XElement? element)
        => element is null
            ? new DocSize(Length.Zero, Length.Zero)
            : new DocSize(
                Length.FromEmu(Long(element, "cx")), Length.FromEmu(Long(element, "cy")));

    private static DocPoint Position(XElement? element)
        => element is null
            ? new DocPoint(Length.Zero, Length.Zero)
            : new DocPoint(
                Length.FromEmu(Long(element, "x")), Length.FromEmu(Long(element, "y")));

    private static XElement? Child(XElement? parent, string ns, string name)
        => parent?.Element(XName.Get(name, ns));

    private static string? Attribute(XElement? element, string name)
        => element?.Attribute(name)?.Value;

    private static string? Attribute(XElement? element, XName name)
        => element?.Attribute(name)?.Value;

    /// <summary>An <c>xdr:col</c>-style child element, whose value is the number.</summary>
    private static int Integer(XElement element, string name)
        => element.Element(XName.Get(name, DrawingNamespace)) is { } child
           && int.TryParse(
               child.Value.Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? Math.Max(value, 0)
            : 0;

    /// <summary>An attribute holding an EMU count, which may exceed an <c>int</c>.</summary>
    private static long Long(XElement element, string name)
        => element.Attribute(name)?.Value is { } text
           && long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out long value)
            ? Math.Max(value, 0)
            : 0;
}
