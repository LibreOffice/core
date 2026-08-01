using System.Globalization;
using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Spreadsheets.Layout;
using Paperless.Vector;

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

            // Indexed once per drawing part and shared by both hops out of it: a picture's
            // r:embed and a chart's r:id are both resolved against the *drawing*, never against
            // the sheet, which is the mistake that finds nothing in most workbooks and the wrong
            // part in one whose sheet happens to declare an rId1 of its own.
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
            // Both elements are DrawingML's, not the spreadsheet drawing's: a graphic frame's
            // content is <a:graphic><a:graphicData uri="…"> in the *main* namespace. Looking the
            // outer one up in the spreadsheetDrawing namespace finds nothing, which read every
            // chart in every workbook as a frame of unknown kind — the flag never got set and no
            // chart could be drawn. It is invisible until something downstream needs the flag.
            XElement? data =
                Child(Child(frame, MainNamespace, "graphic"), MainNamespace, "graphicData");

            if (Attribute(data, "uri") != ChartUri) return drawing;

            return drawing with { IsChart = true, Chart = Plot(data, package, images) };
        }

        // `BlipReference.Choose` rather than `r:embed` read straight off the blip: since Office 2016
        // one `a:blip` may name an SVG in an `asvg:svgBlip` extension beside the raster, and the
        // vector is the one to draw. The raster is kept beside it, so a decode that comes back empty
        // still leaves the picture the file put there for exactly that.
        XElement? blip = Child(Child(picture, DrawingNamespace, "blipFill"), MainNamespace, "blip");
        BlipReference.Choice choice = BlipReference.Choose(blip);

        (RasterImage? raster, Lazy<VectorImage>? vector) = Load(package, images, choice.RelationshipId);

        if (choice.IsVector && choice.FallbackRelationshipId is { } fallback)
        {
            (RasterImage? spare, Lazy<VectorImage>? _) = Load(package, images, fallback);
            if (vector is null) return drawing with { Image = spare };

            raster = spare;
        }

        return drawing with { Image = raster, Vector = vector };
    }

    /// <summary>
    /// The chart a graphic frame points at, laid out later, or null when it cannot be drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read here rather than in a pass of its own because the anchor arithmetic in this file is
    /// what gives the chart a rectangle, and the rendering path walks the drawing part exactly
    /// once. <c>XlsxCharts</c> still walks it a second time for the content tree, which is
    /// deliberate: extraction must not pay for the anchors, and a caller that never asks for
    /// content never opens a chart part.
    /// </para>
    /// <para>
    /// <strong>No theme.</strong> A chart part may state <c>a:schemeClr</c>, which needs the
    /// workbook's <c>xl/theme/theme1.xml</c> to resolve; the sheet reader does not load one, so a
    /// themed fill reads as null and the bar draws as an outline. Every chart in the corpus states
    /// its fills as <c>a:srgbClr</c>, which is what LibreOffice's own export writes.
    /// </para>
    /// </remarks>
    private static ChartPlot? Plot(
        XElement? data,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> parts)
    {
        string? id = Attribute(
            Child(data, OoxmlNamespaces.DrawingMLChart, "chart"),
            XName.Get("id", RelationshipNamespace));

        if (id is null || !parts.TryGetValue(id, out OpcXml.Relationship chart)) return null;
        if (chart.IsExternal || package.GetPart(chart.Target) is not { } chartPart) return null;

        XElement? chartSpace;
        using (Stream content = chartPart.Open()) chartSpace = OoxmlXml.TryLoad(content, out _);

        return chartSpace is null ? null : DrawingChartPlot.Read(chartSpace);
    }

    /// <summary>
    /// Loads a picture's bytes, encoded, and says which kind of picture they are.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="RasterImage.Encoded"/> and no decoding: the bytes are a PNG or a JPEG in the
    /// package and the only thing that can turn them into pixels is a codec, which lives in the
    /// rendering library. A reader that decoded would drag one into the extraction path.
    /// </para>
    /// <para>
    /// A metafile is deferred the same way and for a sharper reason — it <em>can</em> be decoded from
    /// here, and doing it eagerly would put the font stack's start-up cost on a caller that only
    /// wanted cell values. <c>VectorImages.For</c> decides which of the two a part is, from the bytes:
    /// the part name and the declared content type are both a producer's choice and neither
    /// distinguishes an EMF from a WMF, let alone an EMF+ from an EMF.
    /// </para>
    /// </remarks>
    private static (RasterImage? Raster, Lazy<VectorImage>? Vector) Load(
        OpcPackage package, Dictionary<string, OpcXml.Relationship> images, string? id)
    {
        if (id is null || !images.TryGetValue(id, out OpcXml.Relationship relationship)) return default;
        if (relationship.IsExternal) return default;
        if (package.GetPart(relationship.Target) is not { } part) return default;

        using Stream content = part.Open();
        using MemoryStream buffer = new();
        content.CopyTo(buffer);

        if (buffer.Length == 0) return default;

        ReadOnlyMemory<byte> bytes = buffer.ToArray();

        return VectorImages.For(bytes.Span) is not null
            ? (null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)))
            : (RasterImage.Encoded(bytes, part.MediaType), null);
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
