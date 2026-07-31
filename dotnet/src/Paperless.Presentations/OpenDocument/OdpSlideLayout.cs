using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;

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
internal sealed class OdpSlideLayout
{
    /// <summary>How deep a <c>draw:g</c> nest is followed before it is abandoned.</summary>
    private const int MaxGroupDepth = 32;

    private readonly OdfFile _file;
    private readonly SlideFonts _fonts;

    public OdpSlideLayout(OdfFile file, SlideFonts fonts)
    {
        _file = file;
        _fonts = fonts;
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

        return new LaidOutSlide
        {
            Index = index,
            Size = SlideSize(master),
            Name = Attribute(page, OdfNamespaces.Draw, "name"),
            IsHidden = IsHidden(page),
            Background = Background(page, master),
            Shapes = shapes,
        };
    }

    /// <summary>
    /// The slide's size, from the master page's <c>style:page-layout</c>.
    /// </summary>
    /// <remarks>
    /// The master's, not the first page layout in the file: a presentation carries at least two,
    /// one for the slides and one for the notes pages, and the notes layout is A4 portrait in
    /// every deck LibreOffice writes. Taking the wrong one gives a 21 × 29.7 cm slide.
    /// </remarks>
    private DocSize SlideSize(OdfMasterPage? master)
    {
        OdfStyle? layout = _file.Styles.FindPageLayout(master?.PageLayoutName);
        if (layout is null) return default;

        Core.Units.Length width = Property(
            layout, OdfPropertyKind.PageLayout, OdfNamespaces.FoCompatible, "page-width")
            .AsLength() ?? Length.Zero;
        Core.Units.Length height = Property(
            layout, OdfPropertyKind.PageLayout, OdfNamespaces.FoCompatible, "page-height")
            .AsLength() ?? Length.Zero;

        return new DocSize(width, height);
    }

    private OdfProperty Property(
        OdfStyle style, OdfPropertyKind kind, string ns, string name)
        => _file.Styles.ResolveProperty(style.Name, style.Family, kind, ns, name);

    private static bool IsHidden(XElement page)
        => Attribute(page, OdfNamespaces.Draw, "style-name") is not null
           && Attribute(page, OdfNamespaces.Presentation, "class") == "hidden";

    /// <summary>
    /// The slide's background: its own drawing-page style, then the master's.
    /// </summary>
    /// <remarks>
    /// A <c>draw:page</c> names a drawing-page style that usually inherits from the master's, so
    /// resolving through the parent chain finds the master's fill without a second lookup. The
    /// master's own style is consulted only when the page names none at all.
    /// </remarks>
    private Paint? Background(XElement page, OdfMasterPage? master)
    {
        foreach (string? name in (string?[])
                 [Attribute(page, OdfNamespaces.Draw, "style-name"), master?.DrawStyleName])
        {
            if (name is null) continue;

            OdfProperty fill = _file.Styles.ResolveProperty(
                name, OdfStyleFamily.DrawingPage, OdfPropertyKind.DrawingPage,
                OdfNamespaces.Draw, "fill");

            if (!fill.HasValue || fill.Is("none")) continue;
            if (!fill.Is("solid")) return null;

            OdfProperty colour = _file.Styles.ResolveProperty(
                name, OdfStyleFamily.DrawingPage, OdfPropertyKind.DrawingPage,
                OdfNamespaces.Draw, "fill-color");

            if (colour.AsColour() is { } resolved) return Paint.Solid(resolved);
        }

        // Nothing anywhere in the chain states one, which is white: the page colour LibreOffice
        // paints as a full-sheet rectangle on every slide of every deck.
        return Paint.Solid(Colour.White);
    }

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

    private PlacedShape? Shape(XElement element, AffineTransform space)
    {
        DocSize size = new(
            Measure(element, OdfNamespaces.SvgCompatible, "width"),
            Measure(element, OdfNamespaces.SvgCompatible, "height"));

        if (size.IsEmpty) return null;

        AffineTransform placement = AffineTransform.Concat(Placement(element, size), space);

        XElement? geometry = element.Element(XName.Get("enhanced-geometry", OdfNamespaces.Draw));
        string? preset = Preset(element, geometry);

        GraphicsPath local = Mirrored(
            SlidePresetGeometry.Outline(preset, size), geometry, size);

        IReadOnlyList<OdfStyleReference> cascade = StyleCascade(element);

        return new PlacedShape
        {
            Name = Attribute(element, OdfNamespaces.Draw, "name"),
            Outline = ShapeTransform.Apply(placement, local),
            Bounds = ShapeTransform.PlacedBounds(placement, size),
            Fill = Fill(cascade),
            Line = Line(cascade),
            Text = Text(element, size, preset, placement, cascade),
        };
    }

    /// <summary>
    /// The matrix taking the shape's own box onto the slide.
    /// </summary>
    /// <remarks>
    /// Either a plain translation from <c>svg:x</c>/<c>svg:y</c>, or the <c>draw:transform</c>
    /// when the shape has one — which is what LibreOffice writes for anything rotated.
    /// </remarks>
    private static AffineTransform Placement(XElement element, DocSize size)
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
    /// Only <c>draw:fill="solid"</c> is painted. A gradient, hatch or bitmap fill names a
    /// separate <c>draw:gradient</c> or <c>draw:hatch</c> element and is left unpainted rather
    /// than approximated, for the same reason the OOXML path leaves one: the backends do not draw
    /// a gradient yet, and inventing a colour would make a wrong answer look like a right one.
    /// </remarks>
    private Paint? Fill(IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty fill = Graphic(cascade, OdfNamespaces.Draw, "fill");
        if (!fill.HasValue || !fill.Is("solid")) return null;

        return Graphic(cascade, OdfNamespaces.Draw, "fill-color").AsColour() is { } colour
            ? Paint.Solid(colour)
            : null;
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
        DocSize size,
        string? preset,
        AffineTransform placement,
        IReadOnlyList<OdfStyleReference> cascade)
    {
        SlideTextBody body = OdfTextBody.Read(
            _file, Paragraphs(element), [.. cascade, TextStyle(element)]);
        if (body.Paragraphs.Count == 0) return null;

        DocRect rectangle = SlidePresetGeometry.TextRectangle(preset, size);
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
    /// Three shapes of the same thing, and a reader that knows only one loses whole shapes' text.
    /// A <c>draw:frame</c> wraps its text in a <c>draw:text-box</c>; a <c>draw:custom-shape</c>
    /// holds its <c>text:p</c> children itself; and an <em>outline</em> placeholder wraps every
    /// paragraph in a <c>text:list</c>/<c>text:list-item</c> pair, one level of nesting per
    /// outline level. Measured on <c>slides-features.odp</c>: taking only the direct children lost
    /// all six lines of slide one's outline while its title read perfectly, which is exactly the
    /// failure that looks like a placement bug and is not.
    /// </remarks>
    private static IEnumerable<XElement> Paragraphs(XElement element)
    {
        XElement? box = element.Element(XName.Get("text-box", OdfNamespaces.Draw));
        return (box ?? element).Descendants(XName.Get("p", OdfNamespaces.Text));
    }

    private static string? Attribute(XElement? element, string ns, string name)
        => element?.Attribute(XName.Get(name, ns))?.Value;

    /// <summary>A length attribute, or zero when it is absent or unparseable.</summary>
    private static Length Measure(XElement element, string ns, string name)
        => OdfValue.ParseLength(Attribute(element, ns, name)) ?? Core.Units.Length.Zero;
}
