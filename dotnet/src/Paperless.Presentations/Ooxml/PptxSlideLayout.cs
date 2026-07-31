using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Turns a PPTX slide's shape tree into placed shapes.
/// </summary>
/// <remarks>
/// <para>
/// The rendering counterpart of <see cref="PptxShapeReader"/>, and a second walk over the same
/// XML for the reason every family has one: extraction discards the geometry, fills and sizes
/// rendering needs, and making it carry them would charge every caller for a feature most never
/// use.
/// </para>
/// <para>
/// The two walks agree about what a shape <em>is</em> and differ about what they keep, so the
/// order here is the same document order — which is z-order — and groups are descended into the
/// same way. What is new is the coordinate space: a group states the space its children are
/// written in, so descending means composing a matrix rather than merely recursing.
/// </para>
/// </remarks>
internal sealed class PptxSlideLayout
{
    /// <summary>How deep a group nest is followed before it is abandoned.</summary>
    /// <remarks>The same bound extraction uses; a pathological nesting costs stack, not correctness.</remarks>
    private const int MaxGroupDepth = 32;

    private readonly PptxFile _file;
    private readonly SlideFonts _fonts;
    private readonly Dictionary<string, SlideTheme> _themes = new(StringComparer.Ordinal);

    public PptxSlideLayout(PptxFile file, SlideFonts fonts)
    {
        _file = file;
        _fonts = fonts;
    }

    /// <summary>Lays one slide out.</summary>
    public LaidOutSlide Layout(PptxSlide slide)
    {
        SlideTheme theme = ThemeFor(slide);
        List<PlacedShape> shapes = [];

        if (slide.ShapeTree is { } tree)
        {
            Walk(tree, slide, theme, AffineTransform.Identity, shapes, depth: 0);
        }

        return new LaidOutSlide
        {
            Index = slide.Index,
            Size = _file.SlideSize,
            Name = slide.Name,
            IsHidden = slide.IsHidden,
            Background = Background(slide, theme.Colours),
            Shapes = shapes,
        };
    }

    /// <summary>
    /// A theme as slide layout needs it: the colours, and the minor typeface a run falls back to.
    /// </summary>
    /// <remarks>
    /// The font scheme is read here rather than added to <see cref="DrawingTheme"/> because that
    /// type is shared with the other two families and answers exactly one question — what a
    /// scheme colour resolves to. A deck's fallback typeface is a presentation concern, and
    /// widening a shared type for one consumer is how a shared type stops being shareable.
    /// </remarks>
    /// <param name="Colours">The colour scheme and map, or null when the deck has no theme.</param>
    /// <param name="MinorLatin">
    /// <c>a:fontScheme/a:minorFont/a:latin</c>, which is what body text uses when its run names no
    /// typeface — DrawingML's <c>+mn-lt</c>.
    /// </param>
    private readonly record struct SlideTheme(DrawingTheme? Colours, string? MinorLatin);

    /// <summary>
    /// The theme in force for a slide: the master's theme part, seen through the master's colour map.
    /// </summary>
    /// <remarks>
    /// Both halves are needed and neither answers a question alone. The scheme says what
    /// <c>dk1</c> is; the map says whether <c>bg1</c> means <c>dk1</c> or <c>lt1</c>, which is how
    /// a dark master inverts every themed shape on it. LibreOffice applies the same pair at
    /// <c>oox/source/ppt/pptimport.cxx:155</c>. Cached per master part because one master serves
    /// every slide under it.
    /// </remarks>
    private SlideTheme ThemeFor(PptxSlide slide)
    {
        if (slide.MasterPartName is not { } master) return default;
        if (_themes.TryGetValue(master, out SlideTheme cached)) return cached;

        XElement? part = _file.Load(_file.TargetOfType(master, "theme"));
        DrawingTheme? colours = DrawingTheme.Read(part)
            ?.WithMap(DrawingColourMap.Read(Drawing.Child(slide.Master, "clrMap")));

        XElement? minor = Drawing.Child(
            Drawing.Child(Drawing.Child(Drawing.Child(part, "themeElements"), "fontScheme"),
                          "minorFont"),
            "latin");

        SlideTheme theme = new(colours, Drawing.Attribute(minor, "typeface"));
        _themes[master] = theme;
        return theme;
    }

    /// <summary>
    /// The slide's background, taken from the slide, then its layout, then its master.
    /// </summary>
    /// <remarks>
    /// A slide with no <c>p:bg</c> shows its layout's, and a layout with none shows its master's —
    /// which is why nearly every deck states a background exactly once, on the master, and every
    /// slide in it is that colour. A deck that states none anywhere is white, which is what
    /// LibreOffice paints. Only a solid fill is resolved; a gradient or a picture background is
    /// left unpainted rather than approximated, and says so in the TODO.
    /// </remarks>
    private static Paint? Background(PptxSlide slide, DrawingTheme? theme)
    {
        foreach (XElement? part in (XElement?[])[slide.Root, slide.Layout, slide.Master])
        {
            XElement? background = Ppt.Child(Ppt.Child(part, "cSld"), "bg");
            if (background is null) continue;

            XElement? properties = Ppt.Child(background, "bgPr");
            if (properties is null) continue;

            if (SolidFill(properties, theme, placeholder: null) is { } fill) return fill;
            if (Drawing.Child(properties, "noFill") is not null) return null;
        }

        return Paint.Solid(Colour.White);
    }

    private void Walk(
        XElement parent,
        PptxSlide slide,
        SlideTheme theme,
        AffineTransform space,
        List<PlacedShape> shapes,
        int depth)
    {
        foreach (XElement element in parent.Elements())
        {
            if (Ppt.Is(element, "sp") || Ppt.Is(element, "cxnSp"))
            {
                if (Shape(element, slide, theme, space) is { } placed) shapes.Add(placed);
            }
            else if (Ppt.Is(element, "grpSp") && depth < MaxGroupDepth)
            {
                Walk(element, slide, theme, GroupSpace(element, space), shapes, depth + 1);
            }
            else if (Ppt.Is(element, "pic"))
            {
                // A picture's pixels need a decoder nothing in the layout path has yet, so what is
                // placed is its frame: the outline and any line it carries, which is what makes a
                // missing image visible as a hole rather than as nothing at all.
                if (Shape(element, slide, theme, space) is { } placed) shapes.Add(placed);
            }
        }
    }

    /// <summary>The matrix taking a group's child coordinate space onto the slide.</summary>
    private static AffineTransform GroupSpace(XElement group, AffineTransform space)
    {
        XElement? transform = Drawing.Child(Ppt.Child(group, "grpSpPr"), "xfrm");
        DocRect bounds = Bounds(transform);

        DocPoint childOrigin = new(
            Length.FromEmu(Emu(Drawing.Child(transform, "chOff"), "x")),
            Length.FromEmu(Emu(Drawing.Child(transform, "chOff"), "y")));

        DocSize childExtent = new(
            Length.FromEmu(Emu(Drawing.Child(transform, "chExt"), "cx")),
            Length.FromEmu(Emu(Drawing.Child(transform, "chExt"), "cy")));

        return ShapeTransform.GroupSpace(
            bounds,
            childOrigin,
            childExtent,
            ShapeTransform.Radians(Rotation(transform)),
            Drawing.Flag(transform, "flipH") ?? false,
            Drawing.Flag(transform, "flipV") ?? false,
            space);
    }

    private PlacedShape? Shape(
        XElement shape, PptxSlide slide, SlideTheme theme, AffineTransform space)
    {
        XElement? properties = Ppt.Child(shape, "spPr");
        XElement? transform = Drawing.Child(properties, "xfrm");

        // A placeholder that states no transform of its own inherits the whole of it from the
        // layout, and failing that from the master — which is the normal case for a title, whose
        // slide-level shape carries only its text. Falling back to a zero rectangle instead puts
        // every such shape in the top-left corner at no size.
        XElement? inherited = transform is null ? PlaceholderProperties(shape, slide) : null;
        transform ??= Drawing.Child(inherited, "xfrm");
        if (transform is null && inherited is null && properties is null) return null;

        DocRect local = Bounds(transform);
        if (local.Width <= Length.Zero && local.Height <= Length.Zero) return null;

        AffineTransform placement = ShapeTransform.Place(
            local,
            ShapeTransform.Radians(Rotation(transform)),
            Drawing.Flag(transform, "flipH") ?? false,
            Drawing.Flag(transform, "flipV") ?? false,
            space);

        XElement? geometry = Drawing.Child(properties, "prstGeom")
                             ?? Drawing.Child(inherited, "prstGeom");
        string? preset = Drawing.Attribute(geometry, "prst");
        int? adjustment = Adjustment(geometry);

        GraphicsPath outline = ShapeTransform.Apply(
            placement, SlidePresetGeometry.Outline(preset, local.Size, adjustment));

        return new PlacedShape
        {
            Name = Name(shape),
            Outline = outline,
            Bounds = ShapeTransform.PlacedBounds(placement, local.Size),
            Fill = Fill(properties, inherited, theme.Colours),
            Line = Line(properties, inherited, theme.Colours),
            Text = Text(shape, local, preset, adjustment, placement, theme),
        };
    }

    /// <summary>
    /// The <c>p:spPr</c> of the layout or master placeholder a slide shape stands in for.
    /// </summary>
    /// <remarks>
    /// The layout's shapes are searched before the master's, which
    /// <see cref="PptxPlaceholder.Find"/> arranges by walking a concatenated list backwards —
    /// the same reversal LibreOffice relies on (<c>oox/source/ppt/pptshape.cxx:791</c>), and the
    /// reason searching the two trees in the obvious order lets the master win every tie.
    /// </remarks>
    private static XElement? PlaceholderProperties(XElement shape, PptxSlide slide)
    {
        if (PptxPlaceholder.Read(shape, slide.Master) is not { } placeholder) return null;

        XElement? match = placeholder.Find(
            [.. PptxPlaceholder.ShapesOf(slide.Master), .. PptxPlaceholder.ShapesOf(slide.Layout)]);

        return Ppt.Child(match, "spPr");
    }

    private static SlideTextBody? BodyOf(XElement shape, SlideTheme theme)
    {
        XElement? body = Ppt.Child(shape, "txBody");
        return body is null || DrawingTextBody.IsEmpty(body)
            ? null
            : PptxTextBody.Read(body, theme.Colours, theme.MinorLatin);
    }

    private PlacedText? Text(
        XElement shape,
        DocRect local,
        string? preset,
        int? adjustment,
        AffineTransform placement,
        SlideTheme theme)
    {
        if (BodyOf(shape, theme) is not { } body) return null;

        DocRect rectangle = SlidePresetGeometry.TextRectangle(preset, local.Size, adjustment);

        // An upright shape's text goes straight into slide coordinates, which keeps the pens a
        // backend writes directly comparable with a reference renderer's. A rotated or mirrored
        // one cannot: a glyph run carries an origin and advances, not a matrix, so its runs stay
        // in the shape's own space and travel with the transform that places them.
        bool upright = IsUpright(placement);
        DocRect area = upright
            ? new DocRect(
                ShapeTransform.Apply(placement, rectangle.Origin), rectangle.Size)
            : rectangle;

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts);
        if (runs.Count == 0) return null;

        return new PlacedText(runs, upright ? AffineTransform.Identity : placement);
    }

    /// <summary>True when a placement is a pure translation, so text needs no matrix.</summary>
    private static bool IsUpright(AffineTransform transform)
        => transform.A == 1 && transform.B == 0 && transform.C == 0 && transform.D == 1;

    /// <summary>
    /// A shape's fill: its own, then its placeholder's.
    /// </summary>
    /// <remarks>
    /// Only <c>a:solidFill</c> and <c>a:noFill</c> are resolved. A gradient, a picture or a
    /// pattern fill is left unpainted rather than approximated by one of its stops — the backends
    /// do not draw a gradient yet and inventing a colour here would make a wrong answer look like
    /// a right one. Recorded in the TODO, with what it would take.
    /// </remarks>
    private static Paint? Fill(XElement? properties, XElement? inherited, DrawingTheme? theme)
    {
        foreach (XElement? source in (XElement?[])[properties, inherited])
        {
            if (source is null) continue;
            if (Drawing.Child(source, "noFill") is not null) return null;
            if (SolidFill(source, theme, placeholder: null) is { } fill) return fill;
        }

        return null;
    }

    private static Paint? SolidFill(XElement parent, DrawingTheme? theme, Colour? placeholder)
    {
        XElement? solid = Drawing.Child(parent, "solidFill");
        if (solid is null) return null;

        foreach (XElement child in solid.Elements())
        {
            if (DrawingColour.Read(child) is not { } colour) continue;
            if (colour.Resolve(theme, placeholder) is { } resolved) return Paint.Solid(resolved);
        }

        return null;
    }

    /// <summary>
    /// A shape's outline: its <c>a:ln</c>, then its placeholder's.
    /// </summary>
    /// <remarks>
    /// <c>w</c> is in EMUs and its absence means a hairline rather than nothing —
    /// <c>lineproperties.cxx</c> leaves the width unset and the draw layer draws the thinnest line
    /// the device can. A line whose only child is <c>a:noFill</c> is not drawn at all, which is
    /// how <c>&lt;a:ln w="0"&gt;&lt;a:noFill/&gt;&lt;/a:ln&gt;</c> — what LibreOffice's own export
    /// writes for an unstroked shape — says "no outline" rather than "a hairline in black".
    /// </remarks>
    private static Stroke? Line(XElement? properties, XElement? inherited, DrawingTheme? theme)
    {
        foreach (XElement? source in (XElement?[])[properties, inherited])
        {
            XElement? line = Drawing.Child(source, "ln");
            if (line is null) continue;
            if (Drawing.Child(line, "noFill") is not null) return null;
            if (SolidFill(line, theme, placeholder: null) is not { } paint) continue;

            long width = Emu(line, "w");
            return new Stroke(
                paint,
                Length.FromEmu(width),
                Cap(Drawing.Attribute(line, "cap")),
                Join(line));
        }

        return null;
    }

    private static LineCap Cap(string? cap) => cap switch
    {
        "rnd" => LineCap.Round,
        "sq" => LineCap.Square,
        _ => LineCap.Butt,
    };

    private static LineJoin Join(XElement line)
    {
        if (Drawing.Child(line, "round") is not null) return LineJoin.Round;
        if (Drawing.Child(line, "bevel") is not null) return LineJoin.Bevel;
        return LineJoin.Miter;
    }

    private static string? Name(XElement shape)
    {
        foreach (XElement child in shape.Elements())
        {
            if (Ppt.Child(child, "cNvPr") is { } properties)
                return Ppt.Attribute(properties, "name");
        }
        return null;
    }

    /// <summary>The <c>a:off</c>/<c>a:ext</c> pair, in EMUs and needing no conversion.</summary>
    private static DocRect Bounds(XElement? transform) => new(
        Length.FromEmu(Emu(Drawing.Child(transform, "off"), "x")),
        Length.FromEmu(Emu(Drawing.Child(transform, "off"), "y")),
        Length.FromEmu(Emu(Drawing.Child(transform, "ext"), "cx")),
        Length.FromEmu(Emu(Drawing.Child(transform, "ext"), "cy")));

    private static int Rotation(XElement? transform)
        => int.TryParse(
            Drawing.Attribute(transform, "rot"), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out int rotation)
            ? rotation
            : 0;

    /// <summary>The first adjustment value, which is all the supported presets take.</summary>
    private static int? Adjustment(XElement? geometry)
    {
        foreach (XElement guide in Drawing.Children(Drawing.Child(geometry, "avLst"), "gd"))
        {
            string? formula = Drawing.Attribute(guide, "fmla");
            if (formula is null || !formula.StartsWith("val ", StringComparison.Ordinal)) continue;

            if (int.TryParse(
                    formula.AsSpan(4), NumberStyles.Integer, CultureInfo.InvariantCulture,
                    out int value))
                return value;
        }

        return null;
    }

    private static long Emu(XElement? element, string attribute)
        => long.TryParse(
            Drawing.Attribute(element, attribute), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : 0;
}
