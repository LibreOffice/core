using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Containers.Ooxml;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.Ooxml;

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
/// which is the one place SpreadsheetML uses the same unit X-Ray.Office does.
/// </para>
/// <para>
/// <strong><c>editAs</c> decides which of the three a two-cell anchor really is</strong>, and it
/// is read. <c>editAs="oneCell"</c> or <c>"absolute"</c> makes Calc keep the shape's stated
/// <c>a:ext</c> and ignore the second corner (<c>drawingfragment.cxx:284-295</c>). This paragraph
/// used to say the attribute changed nothing that is drawn; measured on <c>SIL_TDB648.xlsx</c>,
/// whose cover photograph is anchored that way across eighteen rows that shrink on load, it is a
/// <strong>4.7%</strong> error in the picture's height — 286.6 pt from the anchor against the
/// reference's 300.73, which is the stated extent to a fiftieth of a point.
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
    /// <param name="ranges">
    /// Resolves a chart sequence's <c>c:f</c> against the workbook's own cells. This is the whole
    /// difference between Calc's chart data provider and the base one — see
    /// <see cref="ChartRangeResolver"/> — and it is a parameter because only a spreadsheet reader
    /// has a workbook to resolve in. The workbook's resolver rather than one delegate, because
    /// each chart binds its own <c>c:plotVisOnly</c> — see
    /// <see cref="XlsxChartRanges.Resolver"/>.
    /// </param>
    /// <param name="styles">
    /// The theme's <c>a:fmtScheme</c>, which a shape's <c>xdr:style</c> indexes for its fill and
    /// its outline. 583 of the corpus's 644 worksheet shapes state one; see
    /// <see cref="XlsxShapeInk"/> for how much of the answer it really decides.
    /// </param>
    public static SheetDrawings Read(
        IPackage package,
        string? sheetPartName,
        DrawingTheme? theme = null,
        DrawingFontScheme? fonts = null,
        XlsxChartRanges? ranges = null,
        DrawingStyleMatrix? styles = null)
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
                if (ReadAnchor(anchor, anchored, opc, images, theme, fonts, ranges, styles)
                    is { } drawing)
                {
                    drawings.Add(drawing);
                }
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
        DrawingFontScheme? fonts,
        XlsxChartRanges? ranges,
        DrawingStyleMatrix? styles)
    {
        XElement? picture = Child(anchor, DrawingNamespace, "pic");
        XElement? frame = Child(anchor, DrawingNamespace, "graphicFrame");

        // A shape, a connector or a group. Its *anchor* counts whatever it holds, because Calc's
        // print area is the bounding box of every object on the drawing layer and a shape is an
        // object like any other (`GroupShapeContext::createShapeContext` takes sp, cxnSp, grpSp,
        // graphicFrame and pic alike, `sc/source/filter/oox/drawingfragment.cxx:198`). Dropping
        // them meant a sheet whose only content was a shape had no printed block at all and
        // produced *no pages*: `xray-office render` failed outright with "the page range selects
        // none of the 0 pages" on 20 of the 55 workbooks in `sc/qa` and `chart2/qa` that hold one,
        // against LibreOffice's one or two pages for each.
        XElement? shape = Child(anchor, DrawingNamespace, "sp")
                          ?? Child(anchor, DrawingNamespace, "cxnSp")
                          ?? Child(anchor, DrawingNamespace, "grpSp");

        if (picture is null && frame is null && shape is null) return null;

        XElement? anchored = picture ?? frame ?? shape;
        XElement? transform = Transform(anchored);
        DocSize extent = Size(Child(anchor, DrawingNamespace, "ext"));

        // `editAs` on a two-cell anchor, which decides whether the second corner is a *size* or a
        // hint. Calc reads it: a `twoCellAnchor editAs="oneCell"` or `"absolute"` takes the shape's
        // own `a:ext` and keeps it, because the anchor's second corner is only where the writing
        // application happened to leave it (`drawingfragment.cxx:284-295`). This file used to state
        // that reading the attribute "would change nothing that is drawn", and that was measurably
        // wrong: `SIL_TDB648.xlsx` anchors its cover photograph `editAs="oneCell"` across rows 1 to
        // 18, and those rows are recomputed shorter on load — so the anchor gives 286.6 pt where the
        // stated extent gives 300.75 and the reference draws 300.73. A 4.7% error in a picture's
        // height, on a page whose word count is exact.
        SheetAnchorKind resolved = kind;
        if (kind == SheetAnchorKind.TwoCell
            && Attribute(anchor, "editAs") is "oneCell" or "absolute"
            && Size(Child(transform, MainNamespace, "ext")) is
               { Width.Emu: > 0, Height.Emu: > 0 } stated)
        {
            resolved = SheetAnchorKind.OneCell;
            extent = stated;
        }

        SheetDrawing drawing = new()
        {
            Anchor = resolved,
            From = Point(Child(anchor, DrawingNamespace, "from")),
            To = Point(Child(anchor, DrawingNamespace, "to")),
            Extent = extent,
            Position = Position(Child(anchor, DrawingNamespace, "pos")),
            Parts = Parts(anchored, transform, package, images, theme, styles),
        };

        // Each shape kind wraps its cNvPr in a differently named non-visual container, and they
        // are searched in turn rather than selected, because a group's is nvGrpSpPr and a
        // connector's is nvCxnSpPr and the wrong guess reads no name and no hidden flag.
        XElement? properties = FirstChild(
            anchored,
            "nvPicPr", "nvGraphicFramePr", "nvSpPr", "nvCxnSpPr", "nvGrpSpPr") is { } container
                ? Child(container, DrawingNamespace, "cNvPr")
                : null;

        drawing = drawing with
        {
            Name = Attribute(properties, "name"),
            Description = Attribute(properties, "descr"),

            // hidden="1" on cNvPr, which is what Excel writes for a shape the user has hidden.
            IsHidden = Attribute(properties, "hidden") is "1" or "true",

            // Whether the anchor states the rectangle the shape occupies *after* its own quarter
            // turn, which Excel writes and Calc undoes: see `SheetDrawing.QuarterTurnedAnchor`
            // and `sc/source/filter/oox/drawingfragment.cxx`:299-330.
            QuarterTurnedAnchor = IsQuarterTurned(transform),
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

            string? uri = Attribute(data, "uri");

            // The extended ("chartex") vocabulary is a second chart payload with a second
            // namespace, and 26.2.4.2 draws it — see `DrawingChartex`. Reaching it at all needs
            // `OoxmlXml.ResolveAlternateContent` to have preferred the `cx1` choice, which it
            // does, and which is now worth doing for the chart rather than for the absence of
            // the fallback's advisory sentence.
            if (uri == DrawingChartex.ChartUri)
            {
                return drawing with
                {
                    IsChart = true,
                    Chart = ExtendedPlot(data, package, images, theme, styles, ranges),
                };
            }

            if (uri != ChartUri) return drawing;

            return drawing with
            {
                IsChart = true,
                Chart = Plot(data, package, images, theme, styles, ranges),
            };
        }

        // A shape's text box, which is the only content on a sheet that no walk of the cells can
        // reach. `xdr:txBody` holds a DrawingML body — the same `a:bodyPr`/`a:p`/`a:r` a slide's
        // does — so the element is looked up in the spreadsheet drawing namespace and everything
        // inside it in the main one.
        if (shape is not null && Child(shape, DrawingNamespace, "txBody") is { } body
            && ShapeText(body, fonts, StyleFace(shape, fonts)) is { IsEmpty: false } shapeText)
        {
            // The preset the text is laid out inside, which is not the same rectangle as the
            // anchor: see `SheetShapeText.Preset`.
            XElement? geometry = Child(
                Child(shape, DrawingNamespace, "spPr"), MainNamespace, "prstGeom");

            drawing = drawing with
            {
                Text = shapeText with
                {
                    Preset = Attribute(geometry, "prst"),
                    Adjustments = Adjustments(geometry),
                },
            };
        }

        // The shape's own fill and outline, which nothing read in any format until round 84.
        // Set on the *anchor* rather than on a part, except where `Parts` has already answered a
        // part for it: a group has one fill per leaf and a turned shape's fill has to be turned
        // with it, so both of those cases are painted from `SheetDrawingPart` instead and setting
        // it here as well would paint the ink twice. See `SheetShapeInk`.
        if (shape is not null && drawing.Parts.Count == 0)
        {
            XlsxShapeInk.Ink ink = XlsxShapeInk.Read(shape, theme, styles);
            if (ink.HasInk)
            {
                drawing = drawing with
                {
                    Fill = ink.Fill,
                    Gradient = ink.Gradient,
                    Stroke = ink.Stroke,
                    StrokeWidth = ink.StrokeWidth,
                    Preset = ink.Preset,
                    Adjustments = ink.Adjustments,
                    FlipHorizontal = ink.FlipHorizontal,
                    FlipVertical = ink.FlipVertical,
                };
            }
        }

        // A shape's own interior can be a bitmap rather than a colour: `a:blipFill` inside
        // `xdr:sp/xdr:spPr`, which is a *fill*, not an `xdr:pic`. It is drawn here rather than in
        // `XlsxShapeInk` because the picture below already does every part of the work — the blip
        // choice, `a:alphaModFix`, `a:srcRect` — and because `Ink` carries colours, so teaching it
        // a bitmap would mean a second image path. `PptxSlideLayout` and `DocxPictures` resolve the
        // same element; only the spreadsheet reader had the hole, and the ten corpus shapes were
        // drawn with no interior at all.
        //
        // **Only `a:stretch`.** A stretched bitmap fills the shape's rectangle, which is exactly
        // what the picture path draws, so the two are the same operation. `a:tile` is a repeat at
        // the blip's own size with its own offsets and is a different geometry; it is left
        // unfilled, as before, rather than drawn stretched — a tile painted stretched would be a
        // confident wrong answer where today there is an absent one. No corpus shape tiles: all
        // ten are `prstGeom` `rect`, `a:stretch`, unrotated, and nine of ten also state `a:srcRect`.
        XElement? shapeFill = picture is not null || shape is null
            ? null
            : Child(Child(shape, DrawingNamespace, "spPr"), MainNamespace, "blipFill");

        if (shapeFill is not null && Child(shapeFill, MainNamespace, "stretch") is null)
            shapeFill = null;

        // A shape carries no image and no chart, so it reaches the print area and stops there.
        if (picture is null && shapeFill is null) return drawing;

        // `BlipReference.Choose` rather than `r:embed` read straight off the blip: since Office 2016
        // one `a:blip` may name an SVG in an `asvg:svgBlip` extension beside the raster, and the
        // vector is the one to draw. The raster is kept beside it, so a decode that comes back empty
        // still leaves the picture the file put there for exactly that.
        XElement? blipFill = shapeFill ?? Child(picture, DrawingNamespace, "blipFill");
        XElement? blip = Child(blipFill, MainNamespace, "blip");
        BlipReference.Choice choice = BlipReference.Choose(blip);

        // `a:alphaModFix` is the whole of what makes a sheet picture a watermark, and it is read
        // through the shared blip reader rather than off the attribute here because that reader is
        // already the one place the element's three wrappers — `a:`, `p:` and `xdr:blipFill` — are
        // known to carry identical content. See `SheetDrawing.Opacity`.
        DrawingBlipFill? fill = DrawingFill.ReadBlip(blipFill);

        // `a:srcRect`: the fraction of each edge of the source the picture throws away, which is
        // what BIFF states as Escher properties 256-259 and this side did not read. LibreOffice
        // turns both into the same `text::GraphicCrop` against the graphic's original size and
        // leaves the anchor where it is (`oox/source/drawingml/fillproperties.cxx`:844-873), so the
        // surviving part fills the anchored box. `SheetPageGraphics` already does that arithmetic
        // for the BIFF path.
        drawing = drawing with
        {
            Opacity = fill?.Opacity ?? 1,
            Crop = fill is null || fill.SourceRect.IsWhole
                ? PictureCropFractions.None
                : new PictureCropFractions(
                    fill.SourceRect.Left, fill.SourceRect.Top,
                    fill.SourceRect.Right, fill.SourceRect.Bottom),
        };

        (RasterImage? raster, Lazy<VectorImage>? vector) =
            LoadImage(package, images, choice.RelationshipId);

        if (choice.IsVector && choice.FallbackRelationshipId is { } fallback)
        {
            (RasterImage? spare, Lazy<VectorImage>? _) = LoadImage(package, images, fallback);
            if (vector is null) return drawing with { Image = KnockedOut(fill, spare) };

            raster = spare;
        }

        return drawing with
        {
            Image = vector is null ? KnockedOut(fill, raster) : raster,
            Vector = vector,
        };
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
        DrawingTheme? theme,
        DrawingStyleMatrix? styles,
        XlsxChartRanges? ranges)
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
            // `automaticChartAreaLine`: the grey D9D9D9 default chart-area border is skipped only
            // under the Impress filter (objectformatter.cxx:838-848, tdf#150176), and this is Calc.
            // A hidden row or column is not chart data unless the chart says otherwise, and
            // that is a property of the chart part rather than of the workbook — so the
            // resolver is bound here, once the chart's own c:plotVisOnly is in hand. See
            // XlsxChartHiddenCells.
            : DrawingChartPlot.Read(
                chartSpace, theme, OoxmlMetadata.IsOffice2007(package), styles,
                ranges?.Resolver(DrawingChart.PlotsVisibleCellsOnly(
                    chartSpace, OoxmlMetadata.IsOffice2007(package))),
                automaticChartAreaLine: true);
    }

    /// <summary>The chartex chart a <c>cx:chart</c> graphic frame names, or null.</summary>
    /// <remarks>
    /// The same shape as <see cref="Plot"/> and a different namespace at every step: the
    /// relationship id is on <c>cx:chart</c> rather than on <c>c:chart</c>, and the part's root is
    /// <c>cx:chartSpace</c>. The resolver is bound unconditionally — chartex has no
    /// <c>c:plotVisOnly</c>, and `plotVisibleOnly`'s own default is what a chart written by Excel
    /// 2010 or later means.
    /// </remarks>
    private static ChartPlot? ExtendedPlot(
        XElement? data,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> parts,
        DrawingTheme? theme,
        DrawingStyleMatrix? styles,
        XlsxChartRanges? ranges)
    {
        string? id = Attribute(
            Child(data, OoxmlNamespaces.ExtendedChart, "chart"),
            XName.Get("id", RelationshipNamespace));

        if (id is null || !parts.TryGetValue(id, out OpcXml.Relationship chart)) return null;
        if (chart.IsExternal || package.GetPart(chart.Target) is not { } chartPart) return null;

        XElement? chartSpace;
        using (Stream content = chartPart.Open()) chartSpace = OoxmlXml.TryLoad(content, out _);

        return chartSpace is null
            ? null
            : DrawingChartex.Read(chartSpace, theme, styles, ranges?.Resolver(true));
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
    /// <summary>
    /// The picture with its <c>a:clrChange</c> attached — PowerPoint's <em>Set Transparent
    /// Color</em>, which SpreadsheetML states on the same <c>a:blip</c>.
    /// </summary>
    /// <remarks>
    /// Attached rather than applied, the same split <c>SheetDrawing.Opacity</c> makes: matching
    /// a colour needs decoded pixels and a reader has no codec. Withheld from a picture that
    /// resolved to a vector, because the reference applies the transform only to a
    /// <c>GraphicType::Bitmap</c> — see <see cref="ColourKnockout"/>.
    /// </remarks>
    private static RasterImage? KnockedOut(DrawingBlipFill? fill, RasterImage? raster)
        => raster is { } image
           && DrawingPictureEffects.Knockout(fill, theme: null, image.EncodedBytes.Span)
                  is { } knockout
            ? image with { Knockout = knockout }
            : raster;

    /// <summary>
    /// Loads one picture part by relationship id. Shared with
    /// <see cref="XlsxLegacyPictures"/>, whose VML shapes name their image the same way and must
    /// not decide separately what an EMF is.
    /// </summary>
    internal static (RasterImage? Raster, Lazy<VectorImage>? Vector) LoadImage(
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
    private static SheetShapeText ShapeText(
        XElement body, DrawingFontScheme? fonts, string? styleFace)
    {
        XElement? properties = Child(body, MainNamespace, "bodyPr");

        List<SheetShapeParagraph> paragraphs = [];
        foreach (XElement paragraph in body.Elements(XName.Get("p", MainNamespace)))
        {
            XElement? paragraphProperties = Child(paragraph, MainNamespace, "pPr");
            XElement? defaults = Child(paragraphProperties, MainNamespace, "defRPr");
            Length inherited = Points(defaults) ?? SheetShapeText.DefaultSize;
            string? inheritedFamily = Family(defaults, fonts) ?? styleFace;
            bool inheritedBold = Bold(defaults) ?? false;

            List<SheetShapeRun> runs = [];
            foreach (XElement run in paragraph.Elements(XName.Get("r", MainNamespace)))
            {
                string text = Child(run, MainNamespace, "t")?.Value ?? string.Empty;
                if (text.Length == 0) continue;

                XElement? runProperties = Child(run, MainNamespace, "rPr");
                runs.Add(new SheetShapeRun(
                    text,
                    Points(runProperties) ?? inherited,
                    Family(runProperties, fonts) ?? inheritedFamily,
                    Bold(runProperties) ?? inheritedBold));
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
                    Family(ending, fonts) ?? inheritedFamily,
                    Bold(ending) ?? inheritedBold));
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

    /// <summary>
    /// The face a shape's <c>xdr:style/a:fontRef</c> names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A shape's style reference is where most text boxes get their face, and every run in
    /// them states none.</strong> <c>a:fontRef idx="minor"</c> means "the theme's minor font", and
    /// <c>Shape::createAndInsert</c> applies it to the shape's text before any run property is
    /// (<c>oox/source/drawingml/shape.cxx</c>, the <c>maShapeStyleRefs</c> walk). Without it a box
    /// authored by Excel — which writes the reference and no <c>a:latin</c> anywhere — falls
    /// through to the drawing layer's own default and is drawn in Liberation Serif where the
    /// reference draws Carlito.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2: <c>Air_Boss_Master_List.xlsx</c>'s note box states
    /// <c>&lt;a:fontRef idx="minor"&gt;</c> against a Calibri theme and nothing else, and the
    /// reference draws it in Carlito-Bold.
    /// </para>
    /// </remarks>
    private static string? StyleFace(XElement shape, DrawingFontScheme? fonts)
        => fonts is { } scheme
           && Child(shape, DrawingNamespace, "style") is { } style
           && Child(style, MainNamespace, "fontRef") is { } reference
            ? scheme.ForReference(reference.Attribute("idx")?.Value, "latin")
            : null;

    /// <summary>
    /// The adjustment values a shape states for its preset, by guide name, or null for none.
    /// </summary>
    /// <remarks>
    /// <c>a:avLst/a:gd</c> states them as <c>fmla="val N"</c>, in the preset's own units — a
    /// hundred-thousandth of the shape for a fraction, a sixtieth of a degree for an angle — and
    /// the evaluator wants the bare number. Anything that is not a <c>val</c> is skipped rather
    /// than guessed at, which leaves the preset's own default in force.
    /// </remarks>
    private static Dictionary<string, double>? Adjustments(XElement? geometry)
    {
        if (Child(geometry, MainNamespace, "avLst") is not { } list) return null;

        Dictionary<string, double> values = [];
        foreach (XElement guide in list.Elements(XName.Get("gd", MainNamespace)))
        {
            if (guide.Attribute("name")?.Value is not { Length: > 0 } name) continue;
            if (guide.Attribute("fmla")?.Value is not { Length: > 4 } formula) continue;
            if (!formula.StartsWith("val ", StringComparison.Ordinal)) continue;

            if (double.TryParse(
                    formula.AsSpan(4).Trim(),
                    NumberStyles.Float,
                    CultureInfo.InvariantCulture,
                    out double value))
            {
                values[name] = value;
            }
        }

        return values.Count == 0 ? null : values;
    }

    /// <summary>A run's <c>b</c>, or null where it states none and inherits.</summary>
    /// <remarks>
    /// Three spellings, because DrawingML's boolean is an <c>xsd:boolean</c> and all of them
    /// appear in the corpus. Absent is not false: it means "take what the paragraph or the body
    /// said", which is why this answers null rather than defaulting here.
    /// </remarks>
    private static bool? Bold(XElement? properties)
        => properties?.Attribute("b")?.Value switch
        {
            "1" or "true" or "on" => true,
            "0" or "false" or "off" => false,
            _ => null,
        };

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

    /// <summary>The <c>a:xfrm</c> a shape or a group states, or null when it states none.</summary>
    private static XElement? Transform(XElement? shape)
        => Child(FirstChild(shape, "spPr", "grpSpPr"), MainNamespace, "xfrm");

    /// <summary>
    /// The leaf shapes inside a drawing, as fractions of its own rectangle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read only so that <see cref="SheetDrawingBounds"/> can tell a drawing's frame from the
    /// rectangle its contents actually cover. Nothing is read for the common case: a shape with no
    /// turned part covers its frame and no more, so the walk returns nothing and the frame stands.
    /// </para>
    /// <para>
    /// A group's <c>a:chOff</c>/<c>a:chExt</c> maps its children's coordinates onto its own
    /// <c>a:off</c>/<c>a:ext</c>, and the mapping composes down the nesting, so each part comes back
    /// positioned in the <em>anchored</em> shape's frame however deep it sat. Rotation does not
    /// compose the same way and is not accumulated: DrawingML turns a group's children about their
    /// own centres, and a group carrying its own <c>rot</c> is rare enough that taking the leaf's
    /// alone is the smaller error than pretending the two add.
    /// </para>
    /// </remarks>
    private static List<SheetDrawingPart> Parts(
        XElement? shape,
        XElement? transform,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> images,
        DrawingTheme? theme,
        DrawingStyleMatrix? styles)
    {
        if (shape is null || transform is null) return [];

        // Nothing to say about an untuned shape that is not a group: it covers its frame exactly,
        // and the walk below would return one part restating it. Checked first so that the common
        // drawing — one picture, no rotation — costs a single attribute lookup.
        bool isGroup = shape.Name.LocalName == "grpSp";
        if (!isGroup && Attribute(transform, "rot") is null) return [];

        XElement? offset = Child(transform, MainNamespace, "off");
        XElement? extent = Child(transform, MainNamespace, "ext");
        if (offset is null || extent is null) return [];

        double frameX = Long(offset, "x");
        double frameY = Long(offset, "y");
        double frameWidth = Long(extent, "cx");
        double frameHeight = Long(extent, "cy");
        if (frameWidth <= 0 || frameHeight <= 0) return [];

        List<SheetDrawingPart> parts = [];
        Collect(shape, frameX, frameY, frameWidth, frameHeight, 0, default);
        return parts.Count == 0 ? [] : parts;

        // (x, y, width, height) are the container's own rectangle, in the anchored shape's EMUs.
        // `inherited` is the enclosing group's own fill, which is what a leaf saying `a:grpFill`
        // takes; it is resolved on the way down for that reason.
        void Collect(
            XElement container, double x, double y, double width, double height, int depth,
            XlsxShapeInk.Ink inherited)
        {
            XElement? own = Transform(container);
            XElement? childOffset = Child(own, MainNamespace, "chOff");
            XElement? childExtent = Child(own, MainNamespace, "chExt");

            if (container.Name.LocalName != "grpSp" || own is null
                || childOffset is null || childExtent is null
                || Long(childExtent, "cx") <= 0 || Long(childExtent, "cy") <= 0
                || depth > 8)
            {
                if (own is null) return;

                parts.Add(Inked(
                    Painted(
                        container, package, images,
                        new SheetDrawingPart(
                            (x - frameX) / frameWidth,
                            (y - frameY) / frameHeight,
                            width / frameWidth,
                            height / frameHeight,
                            Degrees(own))),
                    container, theme, styles, inherited));
                return;
            }

            // A group states its children in a coordinate space of its own and this is the mapping
            // onto the space it occupies — `a:chOff`/`a:chExt` against `a:off`/`a:ext`. It composes
            // down the nesting, which is why the recursion passes a rectangle rather than a scale.
            double originX = Long(childOffset, "x");
            double originY = Long(childOffset, "y");
            double scaleX = width / Long(childExtent, "cx");
            double scaleY = height / Long(childExtent, "cy");

            // The group's own fill, for whichever of its children says `a:grpFill`. A group that
            // itself says so takes its parent's, which is why this is resolved from `inherited`
            // rather than from nothing.
            XlsxShapeInk.Ink groupFill = XlsxShapeInk.Read(container, theme, styles, inherited);

            foreach (XElement child in container.Elements())
            {
                if (child.Name.NamespaceName != DrawingNamespace) continue;
                if (child.Name.LocalName is not ("sp" or "pic" or "grpSp" or "cxnSp"
                    or "graphicFrame"))
                {
                    continue;
                }

                if (Transform(child) is not { } childTransform) continue;
                if (Child(childTransform, MainNamespace, "off") is not { } at) continue;
                if (Child(childTransform, MainNamespace, "ext") is not { } size) continue;

                Collect(
                    child,
                    x + ((Long(at, "x") - originX) * scaleX),
                    y + ((Long(at, "y") - originY) * scaleY),
                    Long(size, "cx") * scaleX,
                    Long(size, "cy") * scaleY,
                    depth + 1,
                    groupFill);
            }
        }
    }

    /// <summary>
    /// A part with whatever picture its leaf shape holds, or the part unchanged when it holds none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A picture inside an <c>xdr:grpSp</c> was read for its bounds and never drawn.</strong>
    /// <see cref="ReadAnchor"/> looks for an <c>xdr:pic</c> directly under the anchor; a group is
    /// none, so the anchor came back with no image, no vector, no chart and no text, and
    /// <see cref="Layout.SheetPageGraphics"/> skipped it — while
    /// <see cref="Layout.SheetDrawingArea"/> still counted it, so the group widened the printed
    /// block and put nothing in it. Calc makes no such distinction:
    /// <c>GroupShapeContext::createShapeContext</c> takes <c>sp</c>, <c>cxnSp</c>, <c>grpSp</c>,
    /// <c>graphicFrame</c> and <c>pic</c> alike (<c>sc/source/filter/oox/drawingfragment.cxx:198</c>)
    /// and every leaf ends up on the drawing layer.
    /// </para>
    /// <para>
    /// Measured on <c>SIL_TDB648.xlsx</c>: its eleven sheet drawings hold one group each, of
    /// fourteen turned, faded copies of the same <c>Honeywell</c> wordmark. <c>pdfimages -list</c>
    /// counts that image on 86 of the reference's 88 pages and on <b>none</b> of our 90.
    /// </para>
    /// <para>
    /// Only a picture is read here; a leaf's fill and outline are <see cref="Inked"/>'s, which the
    /// model grew a <see cref="SheetDrawingPart.Fill"/> for in round 84. A leaf's <em>text</em>
    /// body is still not read — <see cref="Layout.SheetDrawing.Text"/> carries one for the whole
    /// anchor and a group has many.
    /// </para>
    /// </remarks>
    private static SheetDrawingPart Painted(
        XElement leaf,
        OpcPackage package,
        Dictionary<string, OpcXml.Relationship> images,
        SheetDrawingPart part)
    {
        if (leaf.Name.LocalName != "pic") return part;

        XElement? blipFill = Child(leaf, DrawingNamespace, "blipFill");
        XElement? blip = Child(blipFill, MainNamespace, "blip");
        if (blip is null) return part;

        BlipReference.Choice choice = BlipReference.Choose(blip);
        DrawingBlipFill? fill = DrawingFill.ReadBlip(blipFill);

        (RasterImage? raster, Lazy<VectorImage>? vector) =
            LoadImage(package, images, choice.RelationshipId);

        // The `svgBlip` case: the vector is what to draw and the raster beside it is the fallback,
        // so an empty decode still leaves a picture. The same choice `ReadAnchor` makes.
        if (choice.IsVector && choice.FallbackRelationshipId is { } fallback)
        {
            (raster, _) = LoadImage(package, images, fallback);
        }

        if (raster is null && vector is null) return part;

        return part with
        {
            Image = vector is null ? KnockedOut(fill, raster) : raster,
            Vector = vector,
            Opacity = fill?.Opacity ?? 1,
            Crop = fill is null || fill.SourceRect.IsWhole
                ? PictureCropFractions.None
                : new PictureCropFractions(
                    fill.SourceRect.Left, fill.SourceRect.Top,
                    fill.SourceRect.Right, fill.SourceRect.Bottom),
        };
    }

    /// <summary>
    /// A part with its leaf shape's own fill and outline, or the part unchanged when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The ink of a grouped or turned shape belongs on the part rather than on the
    /// anchor.</strong> An <c>xdr:grpSp</c> anchor holds many shapes with many fills, and the
    /// anchor carries one — censused whole-corpus, <b>174 of the 644 worksheet <c>xdr:sp</c> sit
    /// inside a group</b>, in 6 documents. A turned top-level shape reaches this by the same road:
    /// <see cref="Parts"/> already answers a single part restating the frame whenever the
    /// transform states a <c>rot</c>, and only a part carries an angle, so painting the fill from
    /// the anchor would draw it upright inside the turned shape's bounding box.
    /// </para>
    /// <para>
    /// <c>a:grpFill</c> is what makes the group's own fill worth resolving on the way down rather
    /// than at the leaf: it is a reference to the enclosing group's fill, and <b>98 corpus shapes
    /// in 2 documents</b> state one.
    /// </para>
    /// </remarks>
    private static SheetDrawingPart Inked(
        SheetDrawingPart part,
        XElement leaf,
        DrawingTheme? theme,
        DrawingStyleMatrix? styles,
        XlsxShapeInk.Ink inherited)
    {
        if (leaf.Name.LocalName is not ("sp" or "cxnSp" or "grpSp")) return part;

        XlsxShapeInk.Ink ink = XlsxShapeInk.Read(leaf, theme, styles, inherited);
        if (!ink.HasInk) return part;

        return part with
        {
            Fill = ink.Fill,
            Gradient = ink.Gradient,
            Stroke = ink.Stroke,
            StrokeWidth = ink.StrokeWidth,
            Preset = ink.Preset,
            Adjustments = ink.Adjustments,
            FlipHorizontal = ink.FlipHorizontal,
            FlipVertical = ink.FlipVertical,
        };
    }

    /// <summary>
    /// Whether a shape's own rotation falls in the two quarter-turn ranges Excel rewrites an
    /// anchor for.
    /// </summary>
    /// <remarks>
    /// The test is <c>drawingfragment.cxx</c>:300-301 exactly: <c>[45°, 135°)</c> and
    /// <c>[225°, 315°)</c> of the raw <c>@rot</c>, in sixtieth-thousandths, which is a
    /// half-open partition of the circle into the two anchor conventions Excel uses. A negative
    /// or over-turn value is brought into one revolution first, because <c>@rot</c> is not
    /// normalised in real files.
    /// </remarks>
    private static bool IsQuarterTurned(XElement? transform)
    {
        const long Revolution = 360 * 60000L;

        if (transform?.Attribute("rot")?.Value is not { } text) return false;
        if (!long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out long raw))
            return false;

        long rotation = ((raw % Revolution) + Revolution) % Revolution;

        return (rotation >= 45 * 60000L && rotation < 135 * 60000L)
               || (rotation >= 225 * 60000L && rotation < 315 * 60000L);
    }

    /// <summary>How far a transform turns its shape clockwise, in degrees.</summary>
    /// <remarks><c>@rot</c> is in sixtieth-thousandths of a degree.</remarks>
    private static double Degrees(XElement transform)
        => transform.Attribute("rot")?.Value is { } text
           && long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out long value)
            ? value / 60000.0
            : 0;

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
