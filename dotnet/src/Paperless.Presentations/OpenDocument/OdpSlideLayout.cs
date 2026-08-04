using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;
using Paperless.Vector;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Turns an ODF presentation's <c>draw:page</c> elements into placed shapes.
/// </summary>
/// <remarks>
/// <para>
/// The ODF counterpart of <c>PptxSlideLayout</c>, and simpler in the two places that matter.
/// A shape states its position in absolute units — <c>svg:x</c>, <c>svg:y</c>, <c>svg:width</c>,
/// <c>svg:height</c> — rather than in a parent's coordinate space, and a <c>draw:g</c> carries no
/// child space at all: LibreOffice writes its children's coordinates already resolved. So group
/// nesting costs nothing here, where in PresentationML it is a matrix per level.
/// </para>
/// <para>
/// What ODF makes harder is everything else. A fill is not on the shape but in a style, reached
/// through <c>draw:style-name</c> and possibly <c>presentation:style-name</c> and a parent chain
/// behind either; and rotation is a <c>draw:transform</c> whose angle runs the opposite way from
/// OOXML's. Both are handled below with the measurement that pinned them down.
/// </para>
/// </remarks>
internal sealed partial class OdpSlideLayout
{
    /// <summary>How deep a <c>draw:g</c> nest is followed before it is abandoned.</summary>
    private const int MaxGroupDepth = 32;

    private readonly OdfFile _file;
    private readonly SlideFonts _fonts;
    private readonly OdpFills _fills;

    public OdpSlideLayout(OdfFile file, SlideFonts fonts)
    {
        _file = file;
        _fonts = fonts;
        _fills = new OdpFills(file);
    }

    /// <summary>Lays out every <c>draw:page</c> in the document body.</summary>
    public List<LaidOutSlide> Layout()
    {
        List<LaidOutSlide> slides = [];
        if (_file.Body is not { } body) return slides;

        int index = 0;
        foreach (XElement page in body.Elements(XName.Get("page", OdfNamespaces.Draw)))
        {
            slides.Add(Page(page, index++));
        }

        return slides;
    }

    private LaidOutSlide Page(XElement page, int index)
    {
        OdfMasterPage? master = _file.Styles.FindMasterPage(
            Attribute(page, OdfNamespaces.Draw, "master-page-name"));

        List<PlacedShape> shapes = [];
        Walk(page, AffineTransform.Identity, shapes, depth: 0);

        DocSize size = SlideSize(master);

        return new LaidOutSlide
        {
            Index = index,
            Size = size,
            Name = Attribute(page, OdfNamespaces.Draw, "name"),
            IsHidden = IsHidden(page),
            Background = Background(page, master, size),
            Shapes = shapes,
        };
    }

    /// <summary>
    /// The slide's size, from the master page's <c>style:page-layout</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The master's, not the first page layout in the file: a presentation carries at least two,
    /// one for the slides and one for the notes pages, and the notes layout is A4 portrait in
    /// every deck LibreOffice writes. Taking the wrong one gives a 21 × 29.7 cm slide.
    /// </para>
    /// <para>
    /// A document that names no master page at all — which hand-written flat ODF routinely does —
    /// gets Impress's own default rather than a zero-sized page, which renders as a one-pixel
    /// image and makes every comparison against it meaningless. That default is
    /// <c>PAPER_SCREEN_16_9</c> turned on its side: 157.5 by 280 mm at
    /// <c>i18nutil/source/utility/paper.cxx:152</c>, swapped to landscape at
    /// <c>sd/source/core/drawdoc2.cxx:705-706</c>.
    /// </para>
    /// </remarks>
    private DocSize SlideSize(OdfMasterPage? master)
    {
        DocSize screen16By9 = new(Length.FromMillimetres(280), Length.FromMillimetres(157.5));

        OdfStyle? layout = _file.Styles.FindPageLayout(master?.PageLayoutName);
        if (layout is null) return screen16By9;

        Core.Units.Length width = Property(
            layout, OdfPropertyKind.PageLayout, OdfNamespaces.FoCompatible, "page-width")
            .AsLength() ?? Length.Zero;
        Core.Units.Length height = Property(
            layout, OdfPropertyKind.PageLayout, OdfNamespaces.FoCompatible, "page-height")
            .AsLength() ?? Length.Zero;

        return width <= Length.Zero || height <= Length.Zero
            ? screen16By9
            : new DocSize(width, height);
    }

    private OdfProperty Property(
        OdfStyle style, OdfPropertyKind kind, string ns, string name)
        => _file.Styles.ResolveProperty(style.Name, style.Family, kind, ns, name);

    /// <summary>
    /// Whether the slide is skipped during a show, and therefore not exported.
    /// </summary>
    /// <remarks>
    /// <strong>The flag is <c>presentation:visibility</c> on the page's drawing-page style, not an
    /// attribute on the page.</strong> It has to be resolved through the style chain exactly as the
    /// fill is. Reading it as a <c>presentation:class</c> attribute — which is a placeholder's kind
    /// and never a page's — makes every slide of every deck visible, and the only symptom is one
    /// page too many: <c>slides-features.odp</c> came out at three pages against LibreOffice's two.
    /// Extraction had this right all along (<c>OdfContentReader.IsDrawingPageHidden</c>) and layout
    /// did not, which is exactly the kind of divergence two readers of one format produce.
    /// Found twice independently: once by comparing the same deck through the ODF and binary paths,
    /// where the PPT side flagged a slide the ODF side did not, and once by the page count.
    /// </remarks>
    private bool IsHidden(XElement page)
        => _file.Styles.ResolveProperty(
            Attribute(page, OdfNamespaces.Draw, "style-name"),
            OdfStyleFamily.DrawingPage,
            OdfPropertyKind.DrawingPage,
            OdfNamespaces.Presentation,
            "visibility").Is("hidden");

    /// <summary>
    /// The slide's background: its own drawing-page style, then the master's.
    /// </summary>
    /// <remarks>
    /// A <c>draw:page</c> names a drawing-page style that usually inherits from the master's, so
    /// resolving through the parent chain finds the master's fill without a second lookup. The
    /// master's own style is consulted only when the page names none at all.
    /// </remarks>
    private Paint? Background(XElement page, OdfMasterPage? master, DocSize size)
    {
        DocRect sheet = new(DocPoint.Origin, size);

        foreach (string? name in (string?[])
                 [Attribute(page, OdfNamespaces.Draw, "style-name"), master?.DrawStyleName])
        {
            if (name is null) continue;

            OdfProperty fill = Page(name, "fill");
            if (!fill.HasValue || fill.Is("none")) continue;

            if (fill.Is("solid"))
            {
                if (Page(name, "fill-color").AsColour() is { } resolved) return Paint.Solid(resolved);
                continue;
            }

            // A page background reads through the drawing-page property set rather than the
            // graphic one, so the fill helpers cannot be reused wholesale; what they share is the
            // named definition the style points at, which is the part worth sharing.
            if (fill.Is("gradient"))
            {
                return Gradient(_fills.Gradient(Page(name, "fill-gradient-name").Value), sheet);
            }

            if (!fill.Is("bitmap")) return null;
            if (_fills.Image(Page(name, "fill-image-name").Value) is not { } image) return null;

            OdfProperty repeat = _file.Styles.ResolveProperty(
                name, OdfStyleFamily.DrawingPage, OdfPropertyKind.DrawingPage,
                OdfNamespaces.Style, "repeat");

            if (repeat.Is("stretch") || !repeat.HasValue)
                return new BitmapPaint(image, sheet.Size, sheet.Origin, Stretch: true);

            DocSize tile = SlideImages.NaturalSize(image.EncodedBytes.Span) ?? sheet.Size;
            return new BitmapPaint(
                image, tile, SlideImages.TileOrigin(sheet, tile, 0, 0), Stretch: false);
        }

        // Nothing anywhere in the chain states one, which is white: the page colour LibreOffice
        // paints as a full-sheet rectangle on every slide of every deck.
        return Paint.Solid(Colour.White);
    }

    private OdfProperty Page(string style, string name)
        => _file.Styles.ResolveProperty(
            style, OdfStyleFamily.DrawingPage, OdfPropertyKind.DrawingPage,
            OdfNamespaces.Draw, name);

    private void Walk(XElement parent, AffineTransform space, List<PlacedShape> shapes, int depth)
    {
        foreach (XElement element in parent.Elements())
        {
            if (element.Name.NamespaceName != OdfNamespaces.Draw) continue;

            switch (element.Name.LocalName)
            {
                case "g" when depth < MaxGroupDepth:
                    // A group states no coordinate space of its own in ODF, so descending is a
                    // plain recursion; only a draw:transform on the group changes anything, and
                    // LibreOffice writes none.
                    Walk(element, Space(element, space), shapes, depth + 1);
                    break;

                case "frame"
                    when element.Element(XName.Get("table", OdfNamespaces.Table)) is { } table:
                    shapes.AddRange(Table(element, table, space));
                    break;

                // A frame holding an embedded chart draws the chart rather than the frame. It is
                // matched before the general shape case for the same reason a table is: both
                // produce a run of shapes rather than one, and falling through would draw the
                // frame's own empty rectangle over them.
                case "frame" when Chart(element, space) is { Count: > 0 } chart:
                    shapes.AddRange(chart);
                    break;

                case "custom-shape":
                case "rect":
                case "ellipse":
                case "circle":
                case "frame":
                case "polygon":
                case "line":
                    if (Shape(element, space) is { } placed) shapes.Add(placed);
                    break;

                // draw:page-thumbnail is a live preview of another slide, not a picture, and
                // presentation:notes is a separate page rather than part of this one.
                default:
                    break;
            }
        }
    }

    private static AffineTransform Space(XElement group, AffineTransform space)
        => Transform(group) is { } own ? AffineTransform.Concat(own, space) : space;

    /// <summary>
    /// The shapes a <c>draw:frame</c> holding a <c>table:table</c> draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Straight through <see cref="SlideTable.Place"/>, which is the whole point: a table on a
    /// slide is one filled-and-texted shape per cell followed by one stroke per consolidated grid
    /// line whichever filter read it, because LibreOffice decomposes the same <c>SdrTableObj</c>
    /// either way (<c>svx/source/table/viewcontactoftableobj.cxx:202-204</c>). What ODF supplies
    /// is the grid model and a delegate for the cell text; nothing here lays a table out.
    /// </para>
    /// <para>
    /// A frame carrying a table also carries a <c>draw:image</c> — LibreOffice writes a rendered
    /// preview of the table beside it, <c>Pictures/TablePreview1.svm</c>, for applications that
    /// cannot draw one. Drawing that as well would put a second copy of the table on the slide,
    /// so the frame's own shape is not placed at all when it holds a table.
    /// </para>
    /// </remarks>
    private List<PlacedShape> Table(XElement frame, XElement table, AffineTransform space)
    {
        DocSize size = new(
            Measure(frame, OdfNamespaces.SvgCompatible, "width"),
            Measure(frame, OdfNamespaces.SvgCompatible, "height"));

        if (size.IsEmpty) return [];

        return SlideTable.Place(
            OdfTableGeometry.Read(_file, table),
            size,
            AffineTransform.Concat(Placement(frame), space),
            cell => CellBody(cell),
            _fonts,
            Attribute(frame, OdfNamespaces.Draw, "name"));
    }

    /// <summary>
    /// One cell's text body, with the cell's own margins, alignment and line-height rule.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The paragraphs are the cell element's own <c>text:p</c> children, and the cascade the shape
    /// one uses does not apply: a cell's text resolves through its <c>table-cell</c> style rather
    /// than through the frame's graphic style, which carries nothing about the table.
    /// </para>
    /// <para>
    /// <c>FontIndependentLineSpacing</c> is off for the same measured reason it is off on the
    /// OOXML side: LibreOffice 24.2.7.2 draws a table cell's first baseline at the face's own
    /// ascent, not at one em, whatever <c>tablecellcontext.cxx:61</c> sets. Leaving it on puts
    /// every cell's text 1.7 pt low in an 18 pt face.
    /// </para>
    /// </remarks>
    private SlideTextBody? CellBody(DrawingTableCellBox cell)
    {
        if (cell.TextBody is not { } element) return null;

        List<OdfStyleReference> cascade =
        [
            new(element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value,
                OdfStyleFamily.TableCell),
        ];

        SlideTextBody body = OdfTextBody.Read(
            _file, element.Descendants(XName.Get("p", OdfNamespaces.Text)), cascade);

        if (body.Paragraphs.Count == 0) return null;

        return body with
        {
            Insets = cell.Margins,
            Anchor = cell.Anchor switch
            {
                "middle" or "center" => TextAnchor.Middle,
                "bottom" => TextAnchor.Bottom,
                _ => TextAnchor.Top,
            },
            FontIndependentLineSpacing = false,
        };
    }

    private PlacedShape? Shape(XElement element, AffineTransform space)
    {
        DocSize size = new(
            Measure(element, OdfNamespaces.SvgCompatible, "width"),
            Measure(element, OdfNamespaces.SvgCompatible, "height"));

        if (size.IsEmpty) return null;

        AffineTransform placement = AffineTransform.Concat(Placement(element), space);

        XElement? geometry = element.Element(XName.Get("enhanced-geometry", OdfNamespaces.Draw));
        CustomShapeGeometry.Geometry outline = Geometry(element, geometry, size);

        GraphicsPath local = Mirrored(outline.Outline, geometry, size);
        IReadOnlyList<OdfStyleReference> cascade = StyleCascade(element);
        DocRect bounds = ShapeTransform.PlacedBounds(placement, size);

        // The fill is computed in slide coordinates for an upright shape and in the shape's own
        // for a rotated one, which is the same split the text takes and for the same reason: a
        // gradient carries its own transform, so a rotated shape's fill travels with the matrix
        // that places it rather than being baked into coordinates the rotation would undo.
        bool upright = placement.A == 1 && placement.B == 0 && placement.C == 0 && placement.D == 1;
        DocRect box = upright
            ? new DocRect(ShapeTransform.Apply(placement, DocPoint.Origin), size)
            : new DocRect(DocPoint.Origin, size);

        Paint? fill = Fill(cascade, box);
        if (!upright && fill is GradientPaint gradient) fill = gradient with { Transform = placement };

        return new PlacedShape
        {
            Name = Attribute(element, OdfNamespaces.Draw, "name"),
            Outline = ShapeTransform.Apply(placement, local),
            Bounds = bounds,
            Fill = fill,
            Picture = Picture(element, bounds),
            Line = Line(cascade),
            Text = Text(element, outline.TextRectangle, placement, cascade),
            Shadow = Shadow(cascade),
        };
    }

    /// <summary>
    /// The drop shadow a shape casts, from its graphic properties.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF states in five attributes exactly what the model holds, which is not a coincidence:
    /// they are what LibreOffice's own <c>SdrShadowAttribute</c> is written out as, so this is
    /// the format that describes the feature most directly. <c>draw:shadow</c> is the switch —
    /// nearly every shape a Microsoft file produces carries a colour and an offset with the
    /// switch off, so reading the offset without it draws a shadow on almost everything.
    /// </para>
    /// <para>
    /// <c>draw:shadow-opacity</c> is an opacity and not a transparency, and defaults to fully
    /// opaque; <c>draw:shadow-color</c> defaults to the grey a binary file's shadow takes when
    /// it states none.
    /// </para>
    /// </remarks>
    private SlideShadow? Shadow(IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty visible = Graphic(cascade, OdfNamespaces.Draw, "shadow");
        if (!visible.HasValue || !visible.Is("visible")) return null;

        Core.Units.Length x =
            Graphic(cascade, OdfNamespaces.Draw, "shadow-offset-x").AsLength() ?? Core.Units.Length.Zero;
        Core.Units.Length y =
            Graphic(cascade, OdfNamespaces.Draw, "shadow-offset-y").AsLength() ?? Core.Units.Length.Zero;

        Colour colour = Graphic(cascade, OdfNamespaces.Draw, "shadow-color").AsColour()
                        ?? new Colour(0x80, 0x80, 0x80);

        double opacity = Graphic(cascade, OdfNamespaces.Draw, "shadow-opacity").AsPercentage() ?? 1.0;

        return new SlideShadow(
            x,
            y,
            colour.WithAlpha(255),
            Math.Clamp(opacity, 0, 1),
            Graphic(cascade, OdfNamespaces.Draw, "shadow-blur").AsLength() ?? Core.Units.Length.Zero);
    }

    /// <summary>
    /// A shape's outline and text rectangle: its own <c>draw:enhanced-path</c> first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The file's own path beats the preset name, always.</strong> ODF is self-describing
    /// here in a way DrawingML is not — a <c>draw:custom-shape</c> carries the whole geometry
    /// program, not a name to look up — and that is what LibreOffice draws: the <c>draw:type</c> is
    /// consulted only for a handful of special cases in <c>CreateSubPath</c> and never for the
    /// path itself. Preferring the name would answer correctly for the dozen presets whose ODF and
    /// DrawingML spellings we happen to have mapped and would draw a bounding rectangle for the
    /// other hundred and seventy, including every one LibreOffice's own drawing toolbar produces.
    /// </para>
    /// <para>
    /// The name is still the fallback, and it has two jobs: a <c>draw:rect</c>, <c>draw:ellipse</c>
    /// or <c>draw:circle</c> carries no enhanced geometry at all, and a <c>draw:custom-shape</c>
    /// whose path is malformed is better drawn as its preset than as nothing.
    /// </para>
    /// </remarks>
    private static CustomShapeGeometry.Geometry Geometry(
        XElement element, XElement? geometry, DocSize size)
    {
        if (geometry is not null && OdfEnhancedGeometry.Read(geometry, size) is { } stated)
        {
            return stated;
        }

        string? preset = Preset(element, geometry);

        return new CustomShapeGeometry.Geometry(
            SlidePresetGeometry.Outline(preset, size),
            SlidePresetGeometry.TextRectangle(preset, size));
    }

    /// <summary>
    /// The matrix taking the shape's own box onto the slide.
    /// </summary>
    /// <remarks>
    /// Either a plain translation from <c>svg:x</c>/<c>svg:y</c>, or the <c>draw:transform</c>
    /// when the shape has one — which is what LibreOffice writes for anything rotated.
    /// </remarks>
    private static AffineTransform Placement(XElement element)
    {
        if (Transform(element) is { } transform) return transform;

        return AffineTransform.Translation(
            Measure(element, OdfNamespaces.SvgCompatible, "x").Emu,
            Measure(element, OdfNamespaces.SvgCompatible, "y").Emu);
    }

    /// <summary>
    /// Parses a <c>draw:transform</c> into a matrix.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>ODF's rotation runs the other way from OOXML's.</strong> The angle is in radians
    /// and counter-clockwise in a y-up reading, which in the y-down space everything here works in
    /// means the matrix is <c>[cos, sin; −sin, cos]</c> — the transpose of what a naive reading
    /// gives. Measured on <c>shape-geometry.odp</c>, which LibreOffice wrote by converting a deck
    /// whose rectangle is rotated 30° clockwise: it comes out as
    /// <c>rotate (-0.523598775598299) translate (3.515cm 10.33cm)</c>, and only this reading puts
    /// the shape's centre back at the 5.0795 cm, 12.6995 cm the OOXML original states.
    /// </para>
    /// <para>
    /// The operations compose right to left, as they do in SVG: the last one written is applied
    /// last. Only <c>rotate</c>, <c>translate</c>, <c>scale</c> and <c>skewX</c> are read;
    /// <c>matrix</c> is rare enough in real files that leaving it out is honest about coverage
    /// rather than a gap worth filling blind.
    /// </para>
    /// </remarks>
    private static AffineTransform? Transform(XElement element)
    {
        string? text = Attribute(element, OdfNamespaces.Draw, "transform");
        if (string.IsNullOrWhiteSpace(text)) return null;

        AffineTransform result = AffineTransform.Identity;
        bool any = false;

        foreach ((string name, string[] arguments) in Operations(text))
        {
            AffineTransform step = name switch
            {
                "translate" when arguments.Length >= 1 => AffineTransform.Translation(
                    Emu(arguments[0]), arguments.Length > 1 ? Emu(arguments[1]) : 0),
                "rotate" when arguments.Length >= 1 => Rotation(Number(arguments[0])),
                "scale" when arguments.Length >= 1 => AffineTransform.Scale(
                    Number(arguments[0]),
                    arguments.Length > 1 ? Number(arguments[1]) : Number(arguments[0])),
                "skewX" when arguments.Length >= 1 => new AffineTransform(
                    1, 0, Math.Tan(Number(arguments[0])), 1, 0, 0),
                _ => AffineTransform.Identity,
            };

            result = AffineTransform.Concat(result, step);
            any = true;
        }

        return any ? result : null;
    }

    /// <summary>
    /// A transform argument that is a length, in EMUs.
    /// </summary>
    /// <remarks>
    /// A bare number is a hundredth of a millimetre, which is ODF's unitless default and what
    /// <see cref="OdfValue.ParseLength"/> already assumes.
    /// </remarks>
    private static double Emu(string token)
        => OdfValue.ParseLength(token) is { } length ? length.Emu : 0;

    /// <summary>
    /// A transform argument that is a plain number: an angle in radians, or a scale factor.
    /// </summary>
    /// <remarks>
    /// Read as a number and never as a length, which is the whole reason the two are separate.
    /// <see cref="OdfValue.ParseLength"/> takes a unitless value for hundredths of a millimetre,
    /// so putting <c>rotate (-0.5236)</c> through it rounds the angle to −1 and then treats it as
    /// −360 radians — a rotation of about −106 degrees once wrapped, which lands the shape in a
    /// plausible-looking wrong place rather than an obviously wrong one.
    /// </remarks>
    private static double Number(string token)
        => double.TryParse(token, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : 0;

    /// <summary>ODF's rotation, expressed in a y-down space.</summary>
    private static AffineTransform Rotation(double radians)
    {
        double cos = Math.Cos(radians);
        double sin = Math.Sin(radians);
        return new AffineTransform(cos, -sin, sin, cos, 0, 0);
    }

    /// <summary>
    /// The operations in a <c>draw:transform</c>, with their arguments still as written.
    /// </summary>
    /// <remarks>
    /// Unparsed, because what an argument means depends on the operation: <c>translate</c> takes
    /// lengths and <c>rotate</c> takes a bare number of radians, and the two readings are not
    /// interchangeable.
    /// </remarks>
    private static IEnumerable<(string Name, string[] Arguments)> Operations(string text)
    {
        int at = 0;
        while (at < text.Length)
        {
            while (at < text.Length && (char.IsWhiteSpace(text[at]) || text[at] == ',')) at++;

            int nameStart = at;
            while (at < text.Length && char.IsLetter(text[at])) at++;
            if (at == nameStart) yield break;

            string name = text[nameStart..at];

            while (at < text.Length && text[at] != '(') at++;
            int open = at + 1;
            int close = text.IndexOf(')', open);
            if (close < 0) yield break;

            string[] arguments = text[open..close].Split(
                [' ', ',', '\t', '\n', '\r'], StringSplitOptions.RemoveEmptyEntries);

            at = close + 1;
            yield return (name, arguments);
        }
    }

    /// <summary>
    /// The preset a shape draws, mapped onto the DrawingML names the expander knows.
    /// </summary>
    /// <remarks>
    /// LibreOffice tags a geometry it imported from OOXML with an <c>ooxml-</c> prefix
    /// (<c>ooxml-rect</c>, <c>ooxml-rtTriangle</c>) and keeps its own names for everything else
    /// (<c>rectangle</c>, <c>right-triangle</c>). Stripping the prefix therefore recovers the
    /// DrawingML name exactly, and the handful of native names are mapped by hand.
    /// </remarks>
    private static string? Preset(XElement element, XElement? geometry)
    {
        string? type = Attribute(geometry, OdfNamespaces.Draw, "type");

        if (type is not null && type.StartsWith("ooxml-", StringComparison.Ordinal))
            return type["ooxml-".Length..];

        return type switch
        {
            "rectangle" or "round-rectangle" when element.Name.LocalName == "custom-shape" => type switch
            {
                "round-rectangle" => "roundRect",
                _ => "rect",
            },
            "ellipse" or "circle" => "ellipse",
            "right-triangle" => "rtTriangle",
            "isosceles-triangle" => "triangle",
            "diamond" => "diamond",
            _ => element.Name.LocalName switch
            {
                "ellipse" or "circle" => "ellipse",
                _ => "rect",
            },
        };
    }

    /// <summary>The outline with the enhanced geometry's own mirror flags applied.</summary>
    /// <remarks>
    /// ODF puts the flip on the geometry rather than on the transform, so it acts in the shape's
    /// own box — which is the same place OOXML's <c>flipH</c> acts, and before any rotation for
    /// the same reason.
    /// </remarks>
    private static GraphicsPath Mirrored(GraphicsPath path, XElement? geometry, DocSize size)
    {
        bool horizontal =
            OdfValue.ParseBoolean(Attribute(geometry, OdfNamespaces.Draw, "mirror-horizontal"))
            ?? false;
        bool vertical =
            OdfValue.ParseBoolean(Attribute(geometry, OdfNamespaces.Draw, "mirror-vertical"))
            ?? false;

        if (!horizontal && !vertical) return path;

        return ShapeTransform.Apply(
            ShapeTransform.Place(
                new DocRect(Core.Units.Length.Zero, Core.Units.Length.Zero, size.Width, size.Height),
                rotation: 0,
                horizontal,
                vertical,
                AffineTransform.Identity),
            path);
    }

    /// <summary>
    /// The style cascade a shape's graphic properties resolve through.
    /// </summary>
    /// <remarks>
    /// Outermost first, which is what <see cref="OdfStyles.ResolveProperty(IReadOnlyList{OdfStyleReference}, OdfPropertyKind, string, string)"/>
    /// expects: the presentation style a placeholder carries, then the shape's own graphic style.
    /// A shape has one or the other in practice, but a placeholder that has been given a local
    /// override has both, and then the graphic style is the one that must win.
    /// </remarks>
    private static List<OdfStyleReference> StyleCascade(XElement element) =>
    [
        new(Attribute(element, OdfNamespaces.Presentation, "style-name"), OdfStyleFamily.Presentation),
        new(Attribute(element, OdfNamespaces.Draw, "style-name"), OdfStyleFamily.Graphic),
    ];

    /// <summary>
    /// The paragraph style the shape applies to all of its text.
    /// </summary>
    /// <remarks>
    /// <c>draw:text-style-name</c>, and it is not a nicety: LibreOffice's ODF export writes the
    /// shape's real paragraph formatting here and leaves the <c>text:p</c> pointing at an almost
    /// empty automatic style. A reader that consults only the paragraph's own
    /// <c>text:style-name</c> therefore finds no alignment, no line-height rule and no
    /// <c>style:font-independent-line-spacing</c> — which on <c>shape-geometry.odp</c> put every
    /// baseline 1.7 pt high, the difference between one em and Liberation Sans's own ascent.
    /// </remarks>
    private static OdfStyleReference TextStyle(XElement element)
        => new(
            Attribute(element, OdfNamespaces.Draw, "text-style-name"), OdfStyleFamily.Paragraph);

    private OdfProperty Graphic(IReadOnlyList<OdfStyleReference> cascade, string ns, string name)
        => _file.Styles.ResolveProperty(cascade, OdfPropertyKind.Graphic, ns, name);

    /// <summary>
    /// The shape's fill.
    /// </summary>
    /// <remarks>
    /// Three of ODF's five kinds. <c>draw:fill="hatch"</c> is left unpainted — it resolves into
    /// stroked lines the reader would have to synthesise rather than read — and so is a bitmap
    /// fill with <c>style:repeat="no-repeat"</c>, which places the picture once at a reference
    /// point and has no form in a paint that either tiles or stretches. Both are in the TODO.
    /// </remarks>
    private Paint? Fill(IReadOnlyList<OdfStyleReference> cascade, DocRect box)
    {
        OdfProperty fill = Graphic(cascade, OdfNamespaces.Draw, "fill");
        if (!fill.HasValue) return null;

        if (fill.Is("solid"))
        {
            return Graphic(cascade, OdfNamespaces.Draw, "fill-color").AsColour() is { } colour
                ? Paint.Solid(colour)
                : null;
        }

        if (fill.Is("gradient"))
        {
            return Gradient(
                _fills.Gradient(
                    Graphic(cascade, OdfNamespaces.Draw, "fill-gradient-name").Value),
                box);
        }

        return fill.Is("bitmap") ? Bitmap(cascade, box) : null;
    }

    /// <summary>
    /// A <c>draw:gradient</c>, resolved against the box it fills.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>ODF's <c>draw:start-color</c> paints the outer edge of every centred gradient, not its
    /// centre.</b> <c>getRadialGradientAlpha</c> returns <c>1 - hypot(x, y)</c>
    /// (<c>basegfx/source/tools/gradienttools.cxx:641</c>), so the ramp is measured inwards and
    /// the <em>end</em> colour lands in the middle. A <c>#00c0c0</c>-to-<c>#101010</c> radial
    /// renders with a black centre, which is the only way to notice: the geometry is identical
    /// either way round and only the colours differ. The swap happens here rather than in a
    /// backend so that one convention holds everywhere below the readers.
    /// </para>
    /// <para>
    /// An <c>axial</c> gradient is a linear one measured from the middle outwards, so it becomes
    /// three stops on an ordinary ramp — exactly, not approximately; see
    /// <see cref="SlideGradients.Axial"/>.
    /// </para>
    /// </remarks>
    private static GradientPaint? Gradient(OdpGradient? definition, DocRect box)
    {
        if (definition is not { } gradient) return null;

        // ODF measures its angle anticlockwise from a gradient that runs straight down the
        // shape, which in a y-down space makes the direction (sin, cos): at zero the ramp goes
        // top to bottom, and at ninety degrees it goes left to right.
        double dx = Math.Sin(gradient.Angle);
        double dy = Math.Cos(gradient.Angle);

        switch (gradient.Style)
        {
            case "axial":
                return SlideGradients.Linear(
                    box, dx, dy,
                    SlideGradients.Axial(gradient.StartColour, gradient.EndColour, gradient.Border));

            case "radial":
            case "ellipsoid":
            case "square":
            case "rectangular":
            {
                GradientKind kind = gradient.Style switch
                {
                    "radial" => GradientKind.Radial,
                    "ellipsoid" => GradientKind.Elliptical,
                    _ => GradientKind.Rectangular,
                };

                IReadOnlyList<GradientStop> stops = SlideGradients.WithBorder(
                    [
                        new GradientStop(0, gradient.EndColour),
                        new GradientStop(1, gradient.StartColour),
                    ],
                    gradient.Border,
                    atEnd: true);

                DocPoint centre = new(
                    box.Left + (box.Width * gradient.CentreX),
                    box.Top + (box.Height * gradient.CentreY));

                return SlideGradients.Centred(kind, box, centre, stops);
            }

            default:
                return SlideGradients.Linear(
                    box, dx, dy,
                    SlideGradients.WithBorder(
                        [
                            new GradientStop(0, gradient.StartColour),
                            new GradientStop(1, gradient.EndColour),
                        ],
                        gradient.Border));
        }
    }

    /// <summary>
    /// A <c>draw:fill="bitmap"</c>, tiled or stretched.
    /// </summary>
    /// <remarks>
    /// <c>draw:fill-image-width</c> and <c>-height</c> state the tile outright, as a length or
    /// as a percentage of the shape, and their absence means the picture's own natural size —
    /// which is the one case here that needs the header read. The grid is anchored on
    /// <c>draw:fill-image-ref-point</c>, whose default is the centre and not a corner; the
    /// corpus relies on it, and anchoring top-left instead moves every tile by up to half a tile.
    /// </remarks>
    private BitmapPaint? Bitmap(IReadOnlyList<OdfStyleReference> cascade, DocRect box)
    {
        if (_fills.Image(Graphic(cascade, OdfNamespaces.Draw, "fill-image-name").Value)
            is not { } image)
        {
            return null;
        }

        OdfProperty repeat = Graphic(cascade, OdfNamespaces.Style, "repeat");
        if (repeat.Is("stretch")) return new BitmapPaint(image, box.Size, box.Origin, Stretch: true);
        if (repeat.HasValue && !repeat.Is("repeat")) return null;

        DocSize natural = SlideImages.NaturalSize(image.EncodedBytes.Span) ?? box.Size;
        DocSize tile = new(
            TileEdge(cascade, "fill-image-width", box.Width, natural.Width),
            TileEdge(cascade, "fill-image-height", box.Height, natural.Height));

        if (tile.Width <= Core.Units.Length.Zero || tile.Height <= Core.Units.Length.Zero) return null;

        (int horizontal, int vertical) = RefPoint(
            Graphic(cascade, OdfNamespaces.Draw, "fill-image-ref-point").Value ?? "center");

        DocPoint origin = SlideImages.TileOrigin(box, tile, horizontal, vertical);

        return new BitmapPaint(
            image,
            tile,
            new DocPoint(
                origin.X + (tile.Width * Offset(cascade, "fill-image-ref-point-x")),
                origin.Y + (tile.Height * Offset(cascade, "fill-image-ref-point-y"))),
            Stretch: false);
    }

    /// <summary>One edge of a tile: a length, a percentage of the shape, or the picture's own.</summary>
    private Core.Units.Length TileEdge(
        IReadOnlyList<OdfStyleReference> cascade,
        string name,
        Core.Units.Length shape,
        Core.Units.Length natural)
    {
        OdfProperty stated = Graphic(cascade, OdfNamespaces.Draw, name);
        if (!stated.HasValue) return natural;

        if (OdfValue.ParsePercentage(stated.Value) is { } fraction) return shape * fraction;

        // Zero means "the picture's own size" rather than a degenerate tile, which is how ODF
        // spells the default when it states the attribute anyway.
        Core.Units.Length length = stated.AsLength() ?? natural;
        return length <= Core.Units.Length.Zero ? natural : length;
    }

    private double Offset(IReadOnlyList<OdfStyleReference> cascade, string name)
        => OdpFills.Percentage(Graphic(cascade, OdfNamespaces.Draw, name).Value) ?? 0;

    /// <summary><c>draw:fill-image-ref-point</c> as a pair of −1/0/+1 edges.</summary>
    private static (int Horizontal, int Vertical) RefPoint(string value) => value switch
    {
        "top-left" => (-1, -1),
        "top" => (0, -1),
        "top-right" => (1, -1),
        "left" => (-1, 0),
        "right" => (1, 0),
        "bottom-left" => (-1, 1),
        "bottom" => (0, 1),
        "bottom-right" => (1, 1),
        _ => (0, 0),
    };

    /// <summary>
    /// The picture a <c>draw:frame</c> shows, or null for any other shape.
    /// </summary>
    /// <remarks>
    /// A frame holds one of several things — a text box, an object, a picture — and only the
    /// <c>draw:image</c> case draws one. The destination is the frame itself; a
    /// <c>fo:clip</c> crop is not applied, and says so in the TODO.
    /// </remarks>
    private PlacedPicture? Picture(XElement element, DocRect bounds)
    {
        if (element.Name.LocalName != "frame") return null;

        XElement? image = element.Element(XName.Get("image", OdfNamespaces.Draw));
        (RasterImage? raster, Lazy<VectorImage>? vector, bool isInline) = _fills.Drawable(image);

        return raster is null && vector is null
            ? null
            : new PlacedPicture(raster, bounds) { Vector = vector, IsInline = isInline };
    }

    /// <summary>
    /// The shape's outline.
    /// </summary>
    /// <remarks>
    /// <c>draw:stroke</c> distinguishes none from solid from dashed; a dash names a
    /// <c>draw:stroke-dash</c> element whose pattern is not read yet, so a dashed line is drawn
    /// solid rather than not at all. A zero <c>svg:stroke-width</c> is a hairline, which is what
    /// both backends already mean by a zero width.
    /// </remarks>
    private Stroke? Line(IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty stroke = Graphic(cascade, OdfNamespaces.Draw, "stroke");
        if (!stroke.HasValue || stroke.Is("none")) return null;

        Colour colour =
            Graphic(cascade, OdfNamespaces.SvgCompatible, "stroke-color").AsColour() ?? Colour.Black;
        Core.Units.Length width =
            Graphic(cascade, OdfNamespaces.SvgCompatible, "stroke-width").AsLength()
            ?? Core.Units.Length.Zero;

        return new Stroke(Paint.Solid(colour), width);
    }

    private PlacedText? Text(
        XElement element,
        DocRect rectangle,
        AffineTransform placement,
        IReadOnlyList<OdfStyleReference> cascade)
    {
        SlideTextBody body = OdfTextBody.Read(
            _file, Paragraphs(element), [.. cascade, TextStyle(element)]);
        if (body.Paragraphs.Count == 0) return null;

        bool upright = placement.A == 1 && placement.B == 0 && placement.C == 0 && placement.D == 1;

        DocRect area = upright
            ? new DocRect(ShapeTransform.Apply(placement, rectangle.Origin), rectangle.Size)
            : rectangle;

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts);
        if (runs.Count == 0) return null;

        return new PlacedText(runs, upright ? AffineTransform.Identity : placement);
    }

    /// <summary>
    /// The paragraphs of a shape, in document order, however deeply they are wrapped.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three shapes of the same thing, and a reader that knows only one loses whole shapes' text.
    /// A <c>draw:frame</c> wraps its text in a <c>draw:text-box</c>; a <c>draw:custom-shape</c>
    /// holds its <c>text:p</c> children itself; and an <em>outline</em> placeholder wraps every
    /// paragraph in a <c>text:list</c>/<c>text:list-item</c> pair, one level of nesting per
    /// outline level. Measured on <c>slides-features.odp</c>: taking only the direct children lost
    /// all three lines of slide one's outline while its title read perfectly, which is exactly the
    /// failure that looks like a placement bug and is not.
    /// </para>
    /// <para>
    /// <strong>An embedded document's paragraphs are not the frame's.</strong> A packaged file
    /// keeps an object in a directory of its own and this walk never sees it; a <em>flat</em> one
    /// inlines the whole <c>office:document</c> inside the <c>draw:object</c>, so a descendant
    /// search picks up every <c>text:p</c> in it. Measured on <c>chart-bar-deck.fodp</c>: the
    /// chart's title, its axis titles and all fifteen cells of its local table came out as the
    /// frame's own text — eighteen words stacked as paragraphs where LibreOffice draws a bar
    /// chart — while the same deck as <c>.odp</c> drew nothing. The chart is still not drawn;
    /// that is the recorded deviation, and drawing its markup instead was not a lesser one.
    /// </para>
    /// </remarks>
    private static IEnumerable<XElement> Paragraphs(XElement element)
    {
        XElement? box = element.Element(XName.Get("text-box", OdfNamespaces.Draw));
        XElement root = box ?? element;

        foreach (XElement paragraph in root.Descendants(XName.Get("p", OdfNamespaces.Text)))
        {
            if (!IsEmbedded(paragraph, root)) yield return paragraph;
        }
    }

    /// <summary>True when a paragraph sits inside a document embedded within the shape.</summary>
    private static bool IsEmbedded(XElement paragraph, XElement root)
    {
        for (XElement? at = paragraph.Parent; at is not null && at != root; at = at.Parent)
        {
            if (at.Name.NamespaceName == OdfNamespaces.Office
                && at.Name.LocalName is "document" or "document-content")
            {
                return true;
            }
        }
        return false;
    }

    private static string? Attribute(XElement? element, string ns, string name)
        => element?.Attribute(XName.Get(name, ns))?.Value;

    /// <summary>A length attribute, or zero when it is absent or unparseable.</summary>
    private static Length Measure(XElement element, string ns, string name)
        => OdfValue.ParseLength(Attribute(element, ns, name)) ?? Core.Units.Length.Zero;
}
