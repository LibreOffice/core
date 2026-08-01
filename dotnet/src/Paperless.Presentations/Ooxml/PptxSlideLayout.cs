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
    private readonly Dictionary<string, RasterImage?> _images = new(StringComparer.Ordinal);

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
            Background = Background(slide, theme),
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
    /// LibreOffice paints.
    /// </remarks>
    private Paint? Background(PptxSlide slide, SlideTheme theme)
    {
        foreach (XElement? part in (XElement?[])[slide.Root, slide.Layout, slide.Master])
        {
            XElement? background = Ppt.Child(Ppt.Child(part, "cSld"), "bg");
            if (background is null) continue;

            XElement? properties = Ppt.Child(background, "bgPr");
            if (properties is null) continue;

            // The whole sheet, so the fill's box is the slide and its placement is the identity —
            // a background has no a:xfrm and cannot be rotated.
            FillContext context =
                new(slide, theme.Colours, _file.SlideSize, AffineTransform.Identity);

            if (Fill(properties, inherited: null, context) is { } fill) return fill;
            if (Drawing.Child(properties, "noFill") is not null) return null;
        }

        return Paint.Solid(Colour.White);
    }

    /// <summary>
    /// What a fill needs beyond the element stating it: where the shape is, and what resolves
    /// its colours and its relationships.
    /// </summary>
    /// <param name="Slide">The slide, for the parts a relationship could be declared on.</param>
    /// <param name="Theme">The colour scheme, for a themed stop.</param>
    /// <param name="Size">The shape's own box, before placement.</param>
    /// <param name="Placement">The matrix taking that box onto the slide.</param>
    private readonly record struct FillContext(
        PptxSlide Slide, DrawingTheme? Theme, DocSize Size, AffineTransform Placement);

    /// <summary>
    /// Which part an element was read from, which is the part whose relationships its
    /// <c>r:embed</c> resolves against.
    /// </summary>
    /// <remarks>
    /// Derived from the element rather than passed down, because a fill can arrive from any of
    /// three parts and only the element knows which: a placeholder with no fill of its own takes
    /// the layout's or the master's, and each declares its own relationships. Resolving a
    /// master's <c>rId2</c> against the slide's relationships finds a different picture, or none.
    /// </remarks>
    private static string? PartOf(XElement element, PptxSlide slide)
    {
        XElement root = element.AncestorsAndSelf().Last();

        if (ReferenceEquals(root, slide.Layout)) return slide.LayoutPartName;
        if (ReferenceEquals(root, slide.Master)) return slide.MasterPartName;
        return slide.PartName;
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
                if (Shape(element, slide, theme, space) is { } placed) Add(placed, shapes);
            }
            else if (Ppt.Is(element, "grpSp") && depth < MaxGroupDepth)
            {
                Walk(element, slide, theme, GroupSpace(element, space), shapes, depth + 1);
            }
            else if (Ppt.Is(element, "graphicFrame"))
            {
                // A graphic frame is a table, a chart, a diagram or an embedded object. Only the
                // table is drawn; the rest have no geometry here yet and drawing their frame would
                // put an empty rectangle where the reference draws a picture.
                shapes.AddRange(Table(element, theme, space));
            }
            else if (Ppt.Is(element, "pic"))
            {
                // A picture is a shape with a picture in it: the same p:spPr states its transform,
                // its geometry, its fill and its line, and the p:blipFill beside them says what to
                // draw inside. A picture part that will not resolve leaves the frame — the outline
                // and any line it carries — which is what makes a missing image visible as a hole
                // rather than as nothing at all.
                if (Shape(element, slide, theme, space) is { } placed) Add(placed, shapes);
            }
        }
    }

    /// <summary>
    /// Adds a shape, and the arrowheads its line carries.
    /// </summary>
    /// <remarks>
    /// After the shape rather than before, because a marker is filled over the end of the shaft
    /// and the overlap between them is deliberate — a fifteenth of the marker's width, so that a
    /// notched arrowhead has no gap behind it. Drawing the markers first would put the shaft's
    /// own cap on top of that overlap.
    /// </remarks>
    private static void Add(PlacedShape shape, List<PlacedShape> shapes)
    {
        if (shape.Line is not { } stroke
            || (shape.HeadEnd.Type is null && shape.TailEnd.Type is null))
        {
            shapes.Add(shape);
            return;
        }

        (GraphicsPath shaft, List<PlacedShape> markers) = SlideLineEnds.Apply(
            shape.Outline, stroke, shape.HeadEnd, shape.TailEnd, shape.Name);

        shapes.Add(shape with { Outline = shaft });
        shapes.AddRange(markers);
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

    /// <summary>
    /// The shapes a <c>p:graphicFrame</c> holding an <c>a:tbl</c> draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A frame's transform is <c>p:xfrm</c> — PresentationML's own element, with DrawingML's
    /// <c>a:off</c> and <c>a:ext</c> inside it — rather than the <c>a:xfrm</c> a shape carries.
    /// Reading it with the drawing namespace finds nothing and puts every table at the slide's
    /// top-left corner at no size.
    /// </para>
    /// <para>
    /// A cell's text body is read by the same reader a shape's is, with three properties
    /// replaced: the insets become the cell's own <c>marL</c>…<c>marB</c> rather than the body's
    /// <c>a:bodyPr</c> insets, the anchor comes from <c>a:tcPr/@anchor</c>, and the line height
    /// comes from the <em>font's</em> metrics rather than from the em. That last one is measured
    /// rather than assumed, and is the trap in this feature — see the TODO.
    /// </para>
    /// </remarks>
    private List<PlacedShape> Table(XElement frame, SlideTheme theme, AffineTransform space)
    {
        XElement? graphic = Drawing.Child(Drawing.Child(frame, "graphic"), "graphicData");
        if (Drawing.Attribute(graphic, "uri") != DrawingTable.TableUri) return [];
        if (Drawing.Child(graphic, "tbl") is not { } table) return [];

        XElement? transform = Ppt.Child(frame, "xfrm");
        DocRect local = Bounds(transform);
        if (local.Width <= Length.Zero || local.Height <= Length.Zero) return [];

        AffineTransform placement = ShapeTransform.Place(
            local,
            ShapeTransform.Radians(Rotation(transform)),
            Drawing.Flag(transform, "flipH") ?? false,
            Drawing.Flag(transform, "flipV") ?? false,
            space);

        return SlideTable.Place(
            DrawingTableGeometry.Read(table, theme.Colours),
            local.Size,
            placement,
            cell => CellBody(cell, theme),
            _fonts,
            Name(frame));
    }

    private static SlideTextBody? CellBody(DrawingTableCellBox cell, SlideTheme theme)
    {
        if (cell.TextBody is not { } body || DrawingTextBody.IsEmpty(body)) return null;

        return PptxTextBody.Read(body, theme.Colours, theme.MinorLatin) with
        {
            Insets = cell.Margins,
            Anchor = cell.Anchor switch
            {
                "ctr" => TextAnchor.Middle,
                "b" => TextAnchor.Bottom,
                _ => TextAnchor.Top,
            },

            // Measured, and it is the opposite of what the current C++ says. A slide shape's line
            // height is the em (FontIndependentLineSpacing); a table cell's is the face's own
            // ascent. LibreOffice 24.2.7.2 draws deck-features.pptx's first cell — 18 pt Arial,
            // substituted by Liberation Sans, in a cell whose top edge its own PDF puts at
            // 170.079 pt — with a baseline 19.93 pt below that edge. Take off the 3.6 pt top
            // margin and the ascent is 16.33 pt, which is 0.907 em: the font's, not the em's,
            // which would have been 18.00.
            FontIndependentLineSpacing = false,
        };
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
        Dictionary<string, double>? adjustment = Adjustments(geometry);

        // a:custGeom states its own guides and paths, so it needs no preset name — and a shape
        // carrying one has no a:prstGeom at all.
        XElement? custom = Drawing.Child(properties, "custGeom")
                           ?? Drawing.Child(inherited, "custGeom");

        CustomShapeGeometry.Geometry? own = custom is null
            ? null
            : CustomShapeGeometry.Custom(custom, local.Size);

        GraphicsPath outline = ShapeTransform.Apply(
            placement,
            own?.Outline ?? SlidePresetGeometry.Outline(preset, local.Size, adjustment));

        FillContext fills = new(slide, theme.Colours, local.Size, placement);
        DocRect bounds = ShapeTransform.PlacedBounds(placement, local.Size);

        return new PlacedShape
        {
            Name = Name(shape),
            Outline = outline,
            Bounds = bounds,
            Fill = Fill(properties, inherited, fills),
            Picture = Picture(shape, slide, bounds),
            Line = Line(properties, inherited, theme.Colours),
            HeadEnd = LineEnd(properties, inherited, "headEnd"),
            TailEnd = LineEnd(properties, inherited, "tailEnd"),
            Text = Text(shape, local, own?.TextRectangle
                        ?? SlidePresetGeometry.TextRectangle(preset, local.Size, adjustment),
                        placement, theme),
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
        DocRect rectangle,
        AffineTransform placement,
        SlideTheme theme)
    {
        if (BodyOf(shape, theme) is not { } body) return null;

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
    /// Four of DrawingML's six kinds. <c>a:pattFill</c> is left unpainted — it resolves into a
    /// tiled bitmap the reader would have to synthesise rather than read — and a fill inherited
    /// from the theme's style matrix (<c>a:fillRef</c>) is a separate lookup that nothing
    /// measured needs yet. Both are in the TODO.
    /// </remarks>
    private Paint? Fill(XElement? properties, XElement? inherited, in FillContext context)
    {
        foreach (XElement? source in (XElement?[])[properties, inherited])
        {
            if (source is null) continue;
            if (Drawing.Child(source, "noFill") is not null) return null;
            if (SolidFill(source, context.Theme, placeholder: null) is { } fill) return fill;
            if (Gradient(Drawing.Child(source, "gradFill"), context) is { } gradient) return gradient;
            if (Bitmap(Drawing.Child(source, "blipFill"), source, context) is { } bitmap) return bitmap;
        }

        return null;
    }

    /// <summary>
    /// The picture a <c>p:pic</c> draws, or null for any other shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A picture's blip fill is a <c>p:blipFill</c> — PresentationML's own element, with
    /// DrawingML inside it — and not the <c>a:blipFill</c> a shape's <c>p:spPr</c> would carry.
    /// A reader looking for the drawing namespace here finds nothing and draws an empty frame,
    /// which looks exactly like a missing picture part.
    /// </para>
    /// <para>
    /// <c>a:srcRect</c> becomes a larger destination rectangle rather than a crop, because the
    /// drawing model has clipping and no crop and the two are the same thing — see
    /// <see cref="SlideImages.Uncropped"/>. The clip is the shape's outline, applied by
    /// <see cref="SlideDrawing"/>, which also handles the picture-inside-a-preset-shape case for
    /// free.
    /// </para>
    /// <para>
    /// The destination is the shape's placed rectangle, which is right for every picture that is
    /// not rotated. <c>DrawImage</c> takes a rectangle and not a matrix, so a rotated picture is
    /// drawn upright inside its rotated clip; recorded in the TODO rather than approximated.
    /// </para>
    /// </remarks>
    private PlacedPicture? Picture(XElement shape, PptxSlide slide, DocRect bounds)
    {
        if (!Ppt.Is(shape, "pic")) return null;
        if (DrawingFill.ReadBlip(Ppt.Child(shape, "blipFill")) is not { } blip) return null;
        if (Image(blip.EmbedId, PartOf(shape, slide)) is not { } image) return null;

        DocRect area = blip.FillRect.IsWhole
            ? bounds
            : SlideImages.Inset(
                bounds, blip.FillRect.Left, blip.FillRect.Top,
                blip.FillRect.Right, blip.FillRect.Bottom);

        DocRect? destination = SlideImages.Uncropped(
            area, blip.SourceRect.Left, blip.SourceRect.Top,
            blip.SourceRect.Right, blip.SourceRect.Bottom);

        return destination is { } placed
            ? new PlacedPicture(image, placed, Math.Clamp(blip.Opacity, 0, 1))
            : null;
    }

    /// <summary>
    /// An <c>a:gradFill</c>, resolved against the box it fills.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>A path gradient's stop 0 is at the centre, and a linear one's is at the start of its
    /// ramp.</b> That is not obvious from the file and is the mapping most easily got backwards:
    /// LibreOffice <em>reverses</em> the stop list for a path gradient
    /// (<c>fillproperties.cxx:544</c>) before handing it to a model whose first stop paints the
    /// outer edge, so the two reversals cancel and DrawingML's own order is already
    /// centre-outwards. ODF says the opposite and needs the swap; see
    /// <see cref="OpenDocument.OdpSlideLayout"/>.
    /// </para>
    /// <para>
    /// <c>a:path path="shape"</c> — a gradient following a custom outline — is drawn as a
    /// rectangular one, which is what LibreOffice does with it too: its comment says
    /// "XML_rect or XML_shape, but the latter is not implemented".
    /// </para>
    /// </remarks>
    private static Paint? Gradient(XElement? element, in FillContext context)
    {
        if (DrawingFill.ReadGradient(element) is not { Stops.Count: > 0 } gradient) return null;

        List<GradientStop> stops = [];
        foreach (DrawingGradientStop stop in gradient.Stops)
        {
            if (stop.Colour.Resolve(context.Theme, placeholder: null) is not { } colour) continue;
            stops.Add(new GradientStop(stop.Position, colour));
        }

        if (stops.Count == 0) return null;

        (DocRect box, AffineTransform space) = GradientSpace(context);

        if (gradient.Path is null)
        {
            double radians = (gradient.Angle ?? 0) * Math.PI / 180.0;
            return SlideGradients.Linear(box, Math.Cos(radians), Math.Sin(radians), stops)
                with { Transform = space };
        }

        // a:fillToRect states the inner rectangle the gradient converges on; its centre is what
        // LibreOffice keeps, as (MAX_PERCENT + l - r) / 2 (fillproperties.cxx:531-536).
        DocPoint centre = new(
            box.Left + (box.Width * ((1 + gradient.FillToRect.Left - gradient.FillToRect.Right) / 2)),
            box.Top + (box.Height * ((1 + gradient.FillToRect.Top - gradient.FillToRect.Bottom) / 2)));

        GradientKind kind = gradient.Path == "circle"
            ? GradientKind.Radial
            : GradientKind.Rectangular;

        return SlideGradients.Centred(kind, box, centre, stops) with { Transform = space };
    }

    /// <summary>
    /// An <c>a:blipFill</c> used as a shape's fill: a tiled or stretched bitmap.
    /// </summary>
    /// <remarks>
    /// A tile's size is the picture's <em>natural</em> size scaled by <c>a:tile/@sx</c> and
    /// <c>@sy</c>, which is why the reader has to know how large the picture is without decoding
    /// it — see <see cref="SlideImages.NaturalSize"/>. Measured on
    /// <c>paint-fills-pptx.pptx</c>: LibreOffice's own export of a one-centimetre checkerboard
    /// writes <c>sx="471698"</c> over an 8-pixel image, which is one centimetre only if those
    /// eight pixels are 8/96 of an inch.
    /// </remarks>
    private BitmapPaint? Bitmap(XElement? element, XElement source, in FillContext context)
    {
        if (DrawingFill.ReadBlip(element) is not { } blip) return null;
        if (Image(blip.EmbedId, PartOf(source, context.Slide)) is not { } image) return null;

        (DocRect box, _) = GradientSpace(context);

        if (!blip.Tile) return new BitmapPaint(image, box.Size, box.Origin, Stretch: true);

        DocSize natural = SlideImages.NaturalSize(image.EncodedBytes.Span) ?? box.Size;
        DocSize tile = new(natural.Width * blip.TileScaleX, natural.Height * blip.TileScaleY);
        if (tile.Width <= Length.Zero || tile.Height <= Length.Zero) return null;

        (int horizontal, int vertical) = TileAnchor(blip.TileAlign);
        DocPoint origin = SlideImages.TileOrigin(box, tile, horizontal, vertical);

        return new BitmapPaint(
            image,
            tile,
            new DocPoint(
                origin.X + Length.FromEmu(blip.TileOffsetX),
                origin.Y + Length.FromEmu(blip.TileOffsetY)),
            Stretch: false);
    }

    /// <summary>
    /// <c>a:tile/@algn</c> as a pair of −1/0/+1 edges.
    /// </summary>
    /// <remarks>
    /// The schema's default is <c>tl</c> and LibreOffice's own export writes <c>ctr</c>, which
    /// is the value that matters: a centred grid keeps the tiling symmetric about the shape when
    /// the shape's size is not a whole number of tiles, and anchoring it top-left instead moves
    /// every tile by up to half a tile.
    /// </remarks>
    private static (int Horizontal, int Vertical) TileAnchor(string align) => align switch
    {
        "tl" => (-1, -1),
        "t" => (0, -1),
        "tr" => (1, -1),
        "l" => (-1, 0),
        "ctr" => (0, 0),
        "r" => (1, 0),
        "bl" => (-1, 1),
        "b" => (0, 1),
        "br" => (1, 1),
        _ => (-1, -1),
    };

    /// <summary>
    /// The box a fill is computed in, and the matrix taking it to the slide.
    /// </summary>
    /// <remarks>
    /// The same split <see cref="Text"/> makes and for the same reason. An upright shape's fill
    /// is stated in slide coordinates outright, so a shading's numbers land in a backend's output
    /// directly comparable with a reference renderer's; a rotated or scaled one keeps its own box
    /// and travels with the matrix, which a <c>GradientPaint</c> carries and a backend applies as
    /// one more <c>cm</c> or one more local matrix.
    /// </remarks>
    private static (DocRect Box, AffineTransform Space) GradientSpace(in FillContext context)
        => IsUpright(context.Placement)
            ? (new DocRect(ShapeTransform.Apply(context.Placement, DocPoint.Origin), context.Size),
               AffineTransform.Identity)
            : (new DocRect(DocPoint.Origin, context.Size), context.Placement);

    /// <summary>
    /// The bytes of an image part named by an <c>r:embed</c>, still encoded.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Nothing here decodes.</b> <c>RasterImage.Encoded</c> carries the file's own bytes and
    /// whichever backend wants pixels decodes them, which is what keeps
    /// <c>Paperless.Presentations</c> free of a dependency on the rasteriser — and therefore
    /// keeps <c>paperless extract</c> free of a codec it never uses.
    /// </para>
    /// <para>
    /// Cached by part name because one picture serves every slide that shows it: a deck with a
    /// logo on its master would otherwise read and copy the same JPEG once per slide.
    /// </para>
    /// </remarks>
    private RasterImage? Image(string? embedId, string? partName)
    {
        if (embedId is null || partName is null) return null;
        if (_file.Relationship(partName, embedId) is not { IsExternal: false } relationship) return null;
        if (_images.TryGetValue(relationship.Target, out RasterImage? cached)) return cached;

        RasterImage? image = null;
        if (_file.Package.GetPart(relationship.Target) is { } part)
        {
            using Stream content = part.Open();
            using MemoryStream buffer = new();
            content.CopyTo(buffer);

            if (buffer.Length > 0) image = RasterImage.Encoded(buffer.ToArray(), part.MediaType);
        }

        _images[relationship.Target] = image;
        return image;
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

            // The width rounds into the drawing layer's own unit before anything is drawn with
            // it: a stated 38100 EMU — three points — comes out of the reference's PDF as a
            // 3.00467 pt pen, which is 106 hundredths of a millimetre. It matters beyond the pen
            // itself, because a dash pattern's lengths are multiples of it.
            Length width = Length.FromMm100((Emu(line, "w") + 180) / 360);
            LineCap cap = Cap(Drawing.Attribute(line, "cap"));

            return new Stroke(
                paint,
                width,
                cap,
                Join(line),
                DashPattern: SlideDashes.Pattern(
                    Drawing.Attribute(Drawing.Child(line, "prstDash"), "val"),
                    width,
                    capExtendsDash: cap != LineCap.Butt));
        }

        return null;
    }

    /// <summary>The marker one end of a line carries, from its own <c>a:ln</c> or its placeholder's.</summary>
    private static SlideLineEnd LineEnd(XElement? properties, XElement? inherited, string which)
    {
        foreach (XElement? source in (XElement?[])[properties, inherited])
        {
            XElement? line = Drawing.Child(source, "ln");
            if (line is null) continue;
            if (Drawing.Child(line, which) is not { } end) continue;

            string? type = Drawing.Attribute(end, "type");
            if (type is null or "none") return default;

            return new SlideLineEnd(
                type, Drawing.Attribute(end, "w"), Drawing.Attribute(end, "len"));
        }

        return default;
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

    /// <summary>
    /// The adjustment values a shape states, by name.
    /// </summary>
    /// <remarks>
    /// By name and not by position: a shape states only the handles its author moved, so
    /// <c>&lt;a:gd name="adj2" fmla="val 30000"/&gt;</c> alone must leave <c>adj1</c> at the
    /// preset's own default rather than becoming the first adjustment. Six of the presets take
    /// four handles and one takes eight.
    /// </remarks>
    private static Dictionary<string, double>? Adjustments(XElement? geometry)
    {
        Dictionary<string, double>? values = null;

        foreach (XElement guide in Drawing.Children(Drawing.Child(geometry, "avLst"), "gd"))
        {
            string? formula = Drawing.Attribute(guide, "fmla");
            string? name = Drawing.Attribute(guide, "name");

            if (name is null || formula is null
                || !formula.StartsWith("val ", StringComparison.Ordinal))
            {
                continue;
            }

            if (double.TryParse(
                    formula.AsSpan(4), NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double value))
            {
                (values ??= new Dictionary<string, double>(StringComparer.Ordinal))[name] = value;
            }
        }

        return values;
    }

    private static long Emu(XElement? element, string attribute)
        => long.TryParse(
            Drawing.Attribute(element, attribute), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : 0;
}
