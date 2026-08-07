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
    /// <param name="theme">
    /// The workbook's theme, for resolving an <c>a:schemeClr</c> in a chart part.
    /// </param>
    /// <param name="fonts">
    /// The theme's font scheme, for resolving a shape run's <c>+mn-lt</c> into a real family.
    /// </param>
    public static SheetDrawings Read(
        IPackage package,
        string? sheetPartName,
        DrawingTheme? theme = null,
        DrawingFontScheme? fonts = null)
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
                if (ReadAnchor(anchor, anchored, opc, images, theme, fonts) is { } drawing)
                    drawings.Add(drawing);
            }
        }

        return drawings.Count == 0 ? SheetDrawings.Empty : new SheetDrawings(drawings);
    }

    private static SheetDrawing? ReadAnchor(
        XElement anchor,
        SheetAnchorKind kind,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> images,
        DrawingTheme? theme,
        DrawingFontScheme? fonts)
    {
        XElement? picture = Child(anchor, DrawingNamespace, "pic");
        XElement? frame = Child(anchor, DrawingNamespace, "graphicFrame");

        // A shape, a connector or a group. Its *anchor* counts whatever it holds, because Calc's
        // print area is the bounding box of every object on the drawing layer and a shape is an
        // object like any other (`GroupShapeContext::createShapeContext` takes sp, cxnSp, grpSp,
        // graphicFrame and pic alike, `sc/source/filter/oox/drawingfragment.cxx:198`). Dropping
        // them meant a sheet whose only content was a shape had no printed block at all and
        // produced *no pages*: `paperless render` failed outright with "the page range selects
        // none of the 0 pages" on 20 of the 55 workbooks in `sc/qa` and `chart2/qa` that hold one,
        // against LibreOffice's one or two pages for each.
        XElement? shape = Child(anchor, DrawingNamespace, "sp")
                          ?? Child(anchor, DrawingNamespace, "cxnSp")
                          ?? Child(anchor, DrawingNamespace, "grpSp");

        if (picture is null && frame is null && shape is null) return null;

        SheetDrawing drawing = new()
        {
            Anchor = kind,
            From = Point(Child(anchor, DrawingNamespace, "from")),
            To = Point(Child(anchor, DrawingNamespace, "to")),
            Extent = Size(Child(anchor, DrawingNamespace, "ext")),
            Position = Position(Child(anchor, DrawingNamespace, "pos")),
        };

        // Each shape kind wraps its cNvPr in a differently named non-visual container, and they
        // are searched in turn rather than selected, because a group's is nvGrpSpPr and a
        // connector's is nvCxnSpPr and the wrong guess reads no name and no hidden flag.
        XElement? properties = FirstChild(
            picture ?? frame ?? shape,
            "nvPicPr", "nvGraphicFramePr", "nvSpPr", "nvCxnSpPr", "nvGrpSpPr") is { } container
                ? Child(container, DrawingNamespace, "cNvPr")
                : null;

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

            return drawing with { IsChart = true, Chart = Plot(data, package, images, theme) };
        }

        // A shape's text box, which is the only content on a sheet that no walk of the cells can
        // reach. `xdr:txBody` holds a DrawingML body — the same `a:bodyPr`/`a:p`/`a:r` a slide's
        // does — so the element is looked up in the spreadsheet drawing namespace and everything
        // inside it in the main one.
        if (shape is not null && Child(shape, DrawingNamespace, "txBody") is { } body
            && ShapeText(body, fonts) is { IsEmpty: false } shapeText)
        {
            drawing = drawing with { Text = shapeText };
        }

        // A shape carries no image and no chart, so it reaches the print area and stops there.
        if (picture is null) return drawing;

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
    /// <strong>The theme comes in from the workbook, and without it a themed chart draws
    /// nothing.</strong> A chart part may state <c>a:schemeClr</c> rather than an
    /// <c>a:srgbClr</c>, and resolving one needs <c>xl/theme/theme1.xml</c> —
    /// <c>XlsxFile.ThemeRoot</c>, which the cell decoration already used and the drawing path did
    /// not. Measured on <c>chart2/qa/extras/data/xlsx/bubble_chart_simple.xlsx</c>, whose three
    /// series state <c>a:schemeClr val="accent1|2|3"</c> and <c>a:ln/a:noFill</c>: with no theme
    /// every bubble resolved to no fill and no outline and the plot area came out with its axes
    /// and not one mark on it. Every chart LibreOffice's own export writes states
    /// <c>a:srgbClr</c>, which is why a corpus of round-tripped files never showed it.
    /// </para>
    /// </remarks>
    private static ChartPlot? Plot(
        XElement? data,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> parts,
        DrawingTheme? theme)
    {
        string? id = Attribute(
            Child(data, OoxmlNamespaces.DrawingMLChart, "chart"),
            XName.Get("id", RelationshipNamespace));

        if (id is null || !parts.TryGetValue(id, out OpcXml.Relationship chart)) return null;
        if (chart.IsExternal || package.GetPart(chart.Target) is not { } chartPart) return null;

        XElement? chartSpace;
        using (Stream content = chartPart.Open()) chartSpace = OoxmlXml.TryLoad(content, out _);

        return chartSpace is null
            ? null
            : DrawingChartPlot.Read(chartSpace, theme, OoxmlMetadata.IsOffice2007(package));
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

    /// <summary>Reads a shape's <c>xdr:txBody</c> into the text the painter draws.</summary>
    /// <remarks>
    /// <para>
    /// The insets default to DrawingML's own — <c>91440</c> EMUs left and right and <c>45720</c>
    /// top and bottom, which is a tenth and a twentieth of an inch
    /// (<c>oox/source/drawingml/textbodyproperties.cxx</c>) — because a body that states none is
    /// laid out with them and a text box relies on the left one to clear its own border.
    /// </para>
    /// <para>
    /// A run's size is <c>sz</c> in hundredths of a point and its face is
    /// <c>a:rPr/a:latin/@typeface</c>; a run stating neither inherits the paragraph's
    /// <c>a:defRPr</c> before falling back to the body default. Weight, slant and colour are still
    /// not read, because nothing downstream would use them.
    /// </para>
    /// <para>
    /// <strong>The typeface is resolved before it is stored, never after.</strong>
    /// <c>+mn-lt</c> means "the theme's minor Latin face" and is what most authoring tools write;
    /// handing that string to a font resolver asks for a family that exists nowhere and gets
    /// whatever fontconfig offers instead — which is how a Calibri text box came to be measured in
    /// Liberation Sans. <see cref="DrawingFontScheme.Resolve"/> follows the six indirect names and
    /// leaves every real one alone (<c>Theme::resolveFont</c>,
    /// <c>oox/source/drawingml/theme.cxx:71</c>).
    /// </para>
    /// </remarks>
    private static SheetShapeText ShapeText(XElement body, DrawingFontScheme? fonts)
    {
        XElement? properties = Child(body, MainNamespace, "bodyPr");

        List<SheetShapeParagraph> paragraphs = [];
        foreach (XElement paragraph in body.Elements(XName.Get("p", MainNamespace)))
        {
            XElement? paragraphProperties = Child(paragraph, MainNamespace, "pPr");
            XElement? defaults = Child(paragraphProperties, MainNamespace, "defRPr");
            Length inherited = Points(defaults) ?? SheetShapeText.DefaultSize;
            string? inheritedFamily = Family(defaults, fonts);

            List<SheetShapeRun> runs = [];
            foreach (XElement run in paragraph.Elements(XName.Get("r", MainNamespace)))
            {
                string text = Child(run, MainNamespace, "t")?.Value ?? string.Empty;
                if (text.Length == 0) continue;

                XElement? runProperties = Child(run, MainNamespace, "rPr");
                runs.Add(new SheetShapeRun(
                    text,
                    Points(runProperties) ?? inherited,
                    Family(runProperties, fonts) ?? inheritedFamily));
            }

            // A paragraph with nothing in it still occupies a line, and `a:endParaRPr` is what
            // says how tall: the properties the next character typed would take. LibreOffice's own
            // flat-ODS export of a blank paragraph carries them as an empty span, so they are kept
            // here as an empty run rather than dropped — without them the gap between two blocks
            // of a text box is reserved at the body default instead of at the body's own size.
            if (runs.Count == 0)
            {
                XElement? ending = Child(paragraph, MainNamespace, "endParaRPr");
                runs.Add(new SheetShapeRun(
                    string.Empty,
                    Points(ending) ?? inherited,
                    Family(ending, fonts) ?? inheritedFamily));
            }

            // `a:br` is a line break inside a paragraph. Splitting the paragraph at one gives the
            // same lines, since a break and a paragraph end both start a new line here.
            paragraphs.Add(new SheetShapeParagraph
            {
                Runs = runs,
                Alignment = Attribute(paragraphProperties, "algn") switch
                {
                    "ctr" => SheetShapeAlignment.Centre,
                    "r" => SheetShapeAlignment.Right,
                    _ => SheetShapeAlignment.Left,
                },
            });
        }

        return new SheetShapeText
        {
            Paragraphs = paragraphs,
            LeftInset = Inset(properties, "lIns", 91440),
            RightInset = Inset(properties, "rIns", 91440),
            TopInset = Inset(properties, "tIns", 45720),
            BottomInset = Inset(properties, "bIns", 45720),
            Wraps = !string.Equals(Attribute(properties, "wrap"), "none", StringComparison.Ordinal),
            Anchor = Attribute(properties, "anchor") switch
            {
                "ctr" => SheetShapeAnchor.Middle,
                "b" => SheetShapeAnchor.Bottom,
                _ => SheetShapeAnchor.Top,
            },

            // Both values that are not "overflow" clip, which is what `oox` does with the
            // attribute (textbodypropertiescontext.cxx:85-97) — an ellipsis is a clip that
            // marks itself.
            ClipsVerticalOverflow = Attribute(properties, "vertOverflow") is "clip" or "ellipsis",
        };
    }

    /// <summary>
    /// A run's Latin typeface with the theme followed, or null where it states none.
    /// </summary>
    /// <remarks>
    /// Only <c>a:latin</c>. <c>a:ea</c> and <c>a:cs</c> are the East Asian and complex-script
    /// members of the same triple, and choosing between the three is script itemisation, which
    /// this path does not do — picking one arbitrarily would set a Latin body in a CJK face on
    /// every file that states all three.
    /// </remarks>
    private static string? Family(XElement? properties, DrawingFontScheme? fonts)
    {
        if (Child(properties, MainNamespace, "latin")?.Attribute("typeface")?.Value is not
            { Length: > 0 } stated)
        {
            return null;
        }

        // Without a scheme an indirect name resolves to nothing rather than to itself, which is
        // the same choice `DrawingCharacterStyle` makes and for the same reason: falling through
        // to the default face is a substitution, and asking for "+mn-lt" is a wrong answer.
        return fonts is { } scheme ? scheme.Resolve(stated) : (stated[0] == '+' ? null : stated);
    }

    /// <summary>A run's <c>sz</c>, in hundredths of a point, or null where it states none.</summary>
    private static Length? Points(XElement? properties)
        => properties?.Attribute("sz")?.Value is { } text
           && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
           && value > 0
            ? Length.FromPoints(value / 100.0)
            : null;

    /// <summary>A body inset in EMUs, falling back to DrawingML's default.</summary>
    private static Length Inset(XElement? properties, string name, long fallback)
        => Length.FromEmu(
            properties?.Attribute(name)?.Value is { } text
            && long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out long value)
            && value >= 0
                ? value
                : fallback);

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

    /// <summary>The first of several alternative children, in the spreadsheet drawing namespace.</summary>
    private static XElement? FirstChild(XElement? parent, params string[] names)
    {
        if (parent is null) return null;

        foreach (string name in names)
            if (parent.Element(XName.Get(name, DrawingNamespace)) is { } found) return found;

        return null;
    }

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
