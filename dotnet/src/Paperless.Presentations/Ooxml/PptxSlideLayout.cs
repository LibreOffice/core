using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;
using Paperless.Vector;

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
internal sealed partial class PptxSlideLayout
{
    /// <summary>How deep a group nest is followed before it is abandoned.</summary>
    /// <remarks>The same bound extraction uses; a pathological nesting costs stack, not correctness.</remarks>
    private const int MaxGroupDepth = 32;

    private readonly PptxFile _file;
    private readonly SlideFonts _fonts;
    private readonly Dictionary<string, SlideTheme> _themes = new(StringComparer.Ordinal);
    private readonly Dictionary<string, PptxPicture> _images = new(StringComparer.Ordinal);

    /// <summary>
    /// The part each synthesised element tree came from, by the tree's own identity.
    /// </summary>
    /// <remarks>
    /// Keyed by reference, because such a tree is built rather than loaded and so is not any of
    /// the three parts <see cref="PartOf"/> otherwise recognises — and its relationships are its
    /// own. See the remarks there for what goes wrong without it. Two things arrive this way: a
    /// SmartArt diagram's baked shape tree, whose part is the drawing beside the data model; and
    /// a themed fill lifted out of <c>a:fmtScheme</c>, whose part is the theme.
    /// </remarks>
    private readonly Dictionary<XElement, string> _synthesised =
        new(ReferenceEqualityComparer.Instance);

    /// <summary>
    /// The per-level inheritance chain for the slide currently being laid out.
    /// </summary>
    /// <remarks>
    /// Held as state rather than threaded through every method because it is a property of the
    /// slide and is needed at the leaves — a table cell's body, a diagram node's body — where
    /// passing it down would mean widening a dozen signatures that have nothing else to do with
    /// text. Set once at the top of <see cref="Layout"/>.
    /// </remarks>
    private PptxTextStyles? _styles;

    /// <summary>
    /// What the slide currently being laid out resolves its automatic fields to.
    /// </summary>
    /// <remarks>Held beside <see cref="_styles"/>, and for the same reason.</remarks>
    private SlideFields _fields;

    /// <summary>
    /// The background of the slide currently being laid out.
    /// </summary>
    /// <remarks>
    /// Resolved before the shapes rather than after them because a shape can ask for it:
    /// <c>p:sp/@useBgFill</c> fills a shape with the slide's own background. Held beside
    /// <see cref="_styles"/> for the same reason those are.
    /// </remarks>
    private Paint? _background;

    public PptxSlideLayout(PptxFile file, SlideFonts fonts)
    {
        _file = file;
        _fonts = fonts;
    }

    /// <summary>Lays one slide out.</summary>
    public LaidOutSlide Layout(PptxSlide slide)
    {
        SlideTheme theme = ThemeFor(slide);
        _styles = new PptxTextStyles(
            slide.Layout, slide.Master, _file.DefaultTextStyle, isNotesPage: false,
            theme.Colours);
        _fields = new SlideFields(slide.Index + 1, _file.Slides.Count);
        _background = Background(slide, theme);

        List<PlacedShape> shapes = [];

        InheritedShapes(slide, theme, shapes);

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
            Background = _background,
            Shapes = shapes,
        };
    }

    /// <summary>
    /// Draws the shapes the slide inherits from its layout and its master, under its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An Impress master page is a PPTX master and a PPTX layout merged into one, and
    /// everything on it that is not a placeholder is drawn under every slide that uses that
    /// layout.</strong> The merge is literal:
    /// <c>oox/source/ppt/presentationfragmenthandler.cxx:246-296</c> makes one
    /// <c>SlidePersist</c> per <em>layout</em>, imports the master fragment into it, then imports
    /// the layout fragment into the same one, and calls <c>createXShapes</c> once over the pair.
    /// So a logo on the master and a strapline on the layout are the same kind of thing by the
    /// time anything is drawn, and both belong here.
    /// </para>
    /// <para>
    /// Placeholders are excluded because on the Impress side they are presentation objects rather
    /// than background objects: a master's "Click to edit Master title style" is a prompt shown in
    /// master view, and drawing it would put that sentence on every slide of every deck. A slide's
    /// own placeholder already resolves its rectangle and formatting through the same two parts,
    /// by matching type and index rather than by being drawn twice.
    /// </para>
    /// <para>
    /// <strong><c>showMasterSp</c> is real and is almost never the reason a strapline is
    /// invisible.</strong> On the slide it clears <c>IsBackgroundObjectsVisible</c> and hides both
    /// parts' shapes (<c>slidefragmenthandler.cxx:96-98</c>); on the layout it hides only what the
    /// master contributed, because the layout fragment is imported after the master's and
    /// <c>hideShapesAsMasterShapes</c> marks whatever is already there
    /// (<c>slidepersist.cxx:399-411</c>). But of the six decks in
    /// <c>sd/qa/unit/data/pptx/</c> whose master carries a non-placeholder shape with text, not
    /// one states the attribute. The apparent counter-example is
    /// <c>slide-sections.pptx</c>, whose master strapline LibreOffice draws on none of its seven
    /// pages while it draws its layout's on the seventh: the master's three text boxes are at
    /// y = 6 959 601 on a 6 858 000 slide and at x = −2 250 002 and x = −950 805, so they are
    /// simply parked off the page. They <em>are</em> drawn, into nothing. Reading that as a
    /// visibility rule and hunting for the flag that produces it costs an afternoon; the
    /// discriminator is the shape's position, and there is no rule to find.
    /// </para>
    /// </remarks>
    private void InheritedShapes(PptxSlide slide, SlideTheme theme, List<PlacedShape> shapes)
    {
        if (!Ppt.Flag(slide.Root, "showMasterSp", whenAbsent: true)) return;

        bool masterShown = Ppt.Flag(slide.Layout, "showMasterSp", whenAbsent: true);

        foreach (XElement? part in (XElement?[])[masterShown ? slide.Master : null, slide.Layout])
        {
            if (Ppt.Child(Ppt.Child(part, "cSld"), "spTree") is not { } tree) continue;

            Walk(tree, slide, theme, AffineTransform.Identity, shapes, depth: 0, background: true);
        }
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
    /// <param name="Styles">
    /// <c>a:fmtScheme</c>'s fill and line style lists, which a shape's <c>p:style</c> names by
    /// index. Null when the deck's theme declares no format scheme.
    /// </param>
    private readonly record struct SlideTheme(
        DrawingTheme? Colours, string? MinorLatin, DrawingStyleMatrix? Styles);

    /// <summary>
    /// The theme in force for a slide: the master's theme part, seen through the master's colour map.
    /// </summary>
    /// <remarks>
    /// Both halves are needed and neither answers a question alone. The scheme says what
    /// <c>dk1</c> is; the map says whether <c>bg1</c> means <c>dk1</c> or <c>lt1</c>, which is how
    /// a dark master inverts every themed shape on it. LibreOffice applies the same pair at
    /// <c>oox/source/ppt/pptimport.cxx:155</c>.
    /// </remarks>
    private SlideTheme ThemeFor(PptxSlide slide)
    {
        if (slide.MasterPartName is not { } master) return default;

        // Keyed by master *and* layout rather than by master alone. One master serves every
        // layout under it and a layout may still amend the colour map, so caching by master
        // hands the first layout's answer to all of them.
        string key = master + "\u0000" + slide.LayoutPartName;
        if (_themes.TryGetValue(key, out SlideTheme cached)) return cached;

        XElement? part = _file.Load(_file.TargetOfType(master, "theme"));

        // p:clrMap, not a:clrMap. The map is the one element of the pair that is PresentationML —
        // the scheme it reorders is DrawingML — and asking for it in the drawing namespace finds
        // nothing on every deck ever written, which reads as the identity map. That silently
        // inverts a dark master: `bg2` resolves to the theme's second *light* colour instead of
        // its second dark one, so the deck renders as dark text on pale paper where the reference
        // draws white text on a navy slide. Extraction has always read it correctly
        // (`PptxFile.ThemeOf`), which is why no text comparison ever saw this.
        //
        // The layout's override patches that map. A *slide* may state one too and this
        // deliberately does not apply it: measured against the binary, a slide's override does
        // not reach the background it inherits, because Impress resolves a master page's fill
        // once as it imports the layout and the slide only shows it. Modelling that faithfully
        // needs two maps, one for the inherited page and one for the slide's own shapes, and
        // exactly one slide in the 112-deck corpus states an override at all — on
        // NAS-Infrastructure-Roadmaps-v16.0.pptx, where it restates the master's map and
        // changes nothing. Twenty layout overrides across nine decks are the reach here.
        DrawingTheme? colours = DrawingTheme.Read(part)
            ?.WithMap(DrawingColourMap.ReadLayered(
                Ppt.Child(slide.Master, "clrMap"),
                Override(slide.Layout)));

        XElement? minor = Drawing.Child(
            Drawing.Child(Drawing.Child(Drawing.Child(part, "themeElements"), "fontScheme"),
                          "minorFont"),
            "latin");

        SlideTheme theme = new(
            colours, Drawing.Attribute(minor, "typeface"), DrawingStyleMatrix.Read(part));
        _themes[key] = theme;
        return theme;
    }

    /// <summary>
    /// A layout's <c>p:clrMapOvr/a:overrideClrMapping</c>, or null when it states none or
    /// states <c>a:masterClrMapping</c>.
    /// </summary>
    /// <remarks>
    /// The two children of <c>p:clrMapOvr</c> are alternatives and only one of them carries
    /// attributes: <c>a:masterClrMapping</c> is the empty element that says "inherit", so an
    /// absent override and an inheriting one are the same answer and both come back null.
    /// The override itself is DrawingML while the wrapper is PresentationML, which is the sort
    /// of split that makes a single-namespace search find nothing on every deck ever written.
    /// </remarks>
    private static XElement? Override(XElement? root)
        => Drawing.Child(Ppt.Child(root, "clrMapOvr"), "overrideClrMapping");

    /// <summary>
    /// The slide's theme part as XML, which a diagram's style references index into directly.
    /// </summary>
    /// <remarks>
    /// <see cref="SlideTheme"/> holds a resolved colour scheme and a typeface, which is all a
    /// shape needs; a diagram also needs <c>a:fmtScheme</c>, whose three fill styles and three
    /// line styles its quick style names by index and whose <c>phClr</c> its colour transform
    /// substitutes. That is a format matrix rather than a colour, so it is taken from the part
    /// rather than added to the cached theme — and the part itself is already cached by
    /// <see cref="PptxFile.Load"/>, so asking twice costs a dictionary lookup.
    /// </remarks>
    private XElement? ThemePart(PptxSlide slide)
        => slide.MasterPartName is { } master
            ? _file.Load(_file.TargetOfType(master, "theme"))
            : null;

    /// <summary>
    /// The slide's background, taken from the slide, then its layout, then its master.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A slide with no <c>p:bg</c> shows its layout's, and a layout with none shows its master's —
    /// which is why nearly every deck states a background exactly once, on the master, and every
    /// slide in it is that colour. A deck that states none anywhere is white, which is what
    /// LibreOffice paints.
    /// </para>
    /// <para>
    /// <c>p:bg</c> holds one of two children and only one of them is a fill. <c>p:bgPr</c> states
    /// it outright; <c>p:bgRef</c> names one of the theme's <c>a:bgFillStyleLst</c> entries by
    /// index and supplies the colour it is written in terms of, which is
    /// <see cref="DrawingStyleMatrix.Background"/>'s job. Reading only the first of the two makes
    /// a deck that uses the second come out white — and a white slide passes a page count and a
    /// word count alike, so nothing but a picture of the page can see it.
    /// </para>
    /// </remarks>
    private Paint? Background(PptxSlide slide, SlideTheme theme)
    {
        foreach (XElement? part in (XElement?[])[slide.Root, slide.Layout, slide.Master])
        {
            XElement? background = Ppt.Child(Ppt.Child(part, "cSld"), "bg");
            if (background is null) continue;

            XElement? properties = Ppt.Child(background, "bgPr") ?? Themed(background, slide, theme);
            if (properties is null) continue;

            // The whole sheet, so the fill's box is the slide and its placement is the identity —
            // a background has no a:xfrm and cannot be rotated.
            FillContext context =
                new(slide, theme.Colours, _file.SlideSize, AffineTransform.Identity);

            // A slide background is in no group, so there is no group fill for it to ask for.
            if (Fill(properties, [], context, groupFill: null) is { } fill) return fill;
            if (Drawing.Child(properties, "noFill") is not null) return null;
        }

        return Paint.Solid(Colour.White);
    }

    /// <summary>
    /// The fill a <c>p:bg</c>'s <c>p:bgRef</c> names in the theme, shaped like a <c>p:bgPr</c>.
    /// </summary>
    /// <remarks>
    /// The result is a clone of the theme's entry and so belongs to no loaded part, which matters
    /// for the one entry in three that is a <c>a:blipFill</c>: its <c>r:embed</c> is a
    /// relationship of the <em>theme</em>, and resolving it against the slide finds a different
    /// picture or none at all. Registering the wrapper in <see cref="_synthesised"/> is what tells
    /// <see cref="PartOf"/> so — the same mechanism a baked SmartArt tree uses, and for the same
    /// reason.
    /// </remarks>
    private XElement? Themed(XElement background, PptxSlide slide, SlideTheme theme)
    {
        if (theme.Styles is not { } styles) return null;
        if (styles.Background(Ppt.Child(background, "bgRef"), theme.Colours) is not { } fill)
            return null;

        if (slide.MasterPartName is { } master
            && _file.TargetOfType(master, "theme") is { } part)
        {
            _synthesised[fill] = part;
        }

        return fill;
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
    /// <para>
    /// Derived from the element rather than passed down, because a fill can arrive from any of
    /// four parts and only the element knows which: a placeholder with no fill of its own takes
    /// the layout's or the master's, each declares its own relationships, and a SmartArt
    /// diagram's baked shapes come from a fourth part with relationships of its own. Resolving a
    /// master's <c>rId2</c> against the slide's relationships finds a different picture, or none.
    /// </para>
    /// <para>
    /// The diagram case is the one that fails <em>quietly</em>, because the ids collide rather
    /// than run out: in <c>sd/qa/unit/data/pptx/smartart-picture-strip.pptx</c> the drawing
    /// part's <c>rId1</c> is <c>image1.png</c> and the slide's is <c>slideLayout1.xml</c>.
    /// </para>
    /// </remarks>
    private string? PartOf(XElement element, PptxSlide slide)
    {
        XElement root = element.AncestorsAndSelf().Last();

        if (_synthesised.TryGetValue(root, out string? synthesised)) return synthesised;
        if (ReferenceEquals(root, slide.Layout)) return slide.LayoutPartName;
        if (ReferenceEquals(root, slide.Master)) return slide.MasterPartName;
        return slide.PartName;
    }

    /// <summary>
    /// Draws a SmartArt diagram from the shape tree the authoring application baked into the
    /// package beside the layout definition that generated it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Why the baked tree and not the layout algorithms.</strong> Measured over every
    /// OOXML document in the LibreOffice tree carrying a <c>dgm:relIds</c> — 86 of them — 46
    /// have a drawing part with at least one <c>dsp:sp</c> in it, 15 have it emptied and 25 have
    /// no part. Every one of the 40 without a usable one is a LibreOffice import fixture, and the
    /// split is by authoring application: of the 62 written by Office 2010 or later
    /// (<c>&lt;AppVersion&gt;</c> 14, 15 or 16), 46 carry a usable baked drawing and the other 16
    /// all show it removed by hand. Office 2007 — <c>12.0000</c> — wrote none at all, 0 of 24,
    /// which is not a distribution either: the drawing vocabulary's namespace is dated 2008, so
    /// a 2007-era file predates the feature.
    /// Evaluating <c>layout1.xml</c> instead is the largest subsystem in LibreOffice's PPTX
    /// importer and would produce a diagram that differs from the reference; reading the baked
    /// tree reuses the slide layouter entire.
    /// </para>
    /// <para>
    /// <strong>The frame's transform is a group's, not a shape's.</strong> The baked shapes'
    /// <c>a:off</c> are in the diagram's own space, measured from the frame's top-left corner,
    /// so the frame maps a child coordinate space exactly as a <c>p:grpSp</c> does — with no
    /// <c>a:chOff</c> or <c>a:chExt</c>, which is the absent case
    /// <see cref="ShapeTransform.GroupSpace"/> already answers with a factor of one. LibreOffice
    /// states the same thing from the other end at
    /// <c>oox/source/drawingml/diagram/diagram.cxx:131</c>,
    /// <c>pParentShape-&gt;setChildSize(pParentShape-&gt;getSize())</c>: a child space the same
    /// size as the frame, so the mapping is the frame's offset and nothing else.
    /// </para>
    /// </remarks>
    private void Diagram(
        XElement frame,
        PptxSlide slide,
        SlideTheme theme,
        AffineTransform space,
        List<PlacedShape> shapes,
        int depth)
    {
        if (depth >= MaxGroupDepth) return;

        XElement? graphic = Drawing.Child(Drawing.Child(frame, "graphic"), "graphicData");
        if (Drawing.Attribute(graphic, "uri") != PptxDiagram.Uri) return;
        if (PartOf(frame, slide) is not { } part) return;

        XElement? transform = Ppt.Child(frame, "xfrm");
        DocRect local = Bounds(transform);
        if (local.Width <= Length.Zero || local.Height <= Length.Zero) return;

        // The baked drawing first, and the layout-atom evaluator only when there is none. The
        // baked tree is what the authoring application itself drew, so preferring it keeps a
        // modern file independent of the evaluator agreeing with PowerPoint; LibreOffice decides
        // the same way in one line — diagram.cxx:701, bCreate = getExtDrawings().empty().
        if ((PptxDiagram.Baked(_file, part, graphic!)
             ?? PptxDiagram.Evaluated(
                 _file,
                 part,
                 graphic!,
                 ThemePart(slide),
                 theme.Colours,
                 (int)local.Width.Emu,
                 (int)local.Height.Emu)) is not { } baked)
        {
            return;
        }

        _synthesised[baked.ShapeTree] = baked.PartName;

        AffineTransform inside = ShapeTransform.GroupSpace(
            local,
            childOrigin: default,
            childExtent: default,
            ShapeTransform.Radians(Rotation(transform)),
            Drawing.Flag(transform, "flipH") ?? false,
            Drawing.Flag(transform, "flipV") ?? false,
            space);

        Walk(baked.ShapeTree, slide, theme, inside, shapes, depth + 1);
    }

    /// <summary>
    /// Walks a shape tree in document order, which is z-order, composing group spaces as it goes.
    /// </summary>
    /// <remarks>
    /// <c>background</c> is true when the tree is a master's or a layout's rather than the
    /// slide's, which excludes its placeholders — see <see cref="InheritedShapes"/>.
    /// </remarks>
    private void Walk(
        XElement parent,
        PptxSlide slide,
        SlideTheme theme,
        AffineTransform space,
        List<PlacedShape> shapes,
        int depth,
        bool background = false,
        Paint? groupFill = null)
    {
        foreach (XElement element in parent.Elements())
        {
            if (IsHidden(element)) continue;

            if (Ppt.Is(element, "sp") || Ppt.Is(element, "cxnSp"))
            {
                if (background && PptxPlaceholder.Read(element, slide.Master, slide.Layout) is not null)
                    continue;
                if (Shape(element, slide, theme, space, groupFill) is { } placed) Add(placed, shapes);
            }
            else if (Ppt.Is(element, "grpSp") && depth < MaxGroupDepth)
            {
                Walk(
                    element, slide, theme, GroupSpace(element, space), shapes, depth + 1,
                    background, GroupFill(element, slide, theme, space, groupFill));
            }
            else if (Ppt.Is(element, "graphicFrame"))
            {
                // A graphic frame is a table, a chart, a diagram or an embedded object, and all
                // four now draw: a table from its own model, a chart from the plot the reader
                // built, a diagram from the shape tree the authoring application baked beside its
                // layout definition, and an embedded object from the picture of itself it carries.
                shapes.AddRange(Table(element, theme, space));
                shapes.AddRange(Chart(element, slide, theme, space));
                shapes.AddRange(Ole(element, slide, theme, space));
                Diagram(element, slide, theme, space, shapes, depth);
            }
            else if (Ppt.Is(element, "pic"))
            {
                // A picture is a shape with a picture in it: the same p:spPr states its transform,
                // its geometry, its fill and its line, and the p:blipFill beside them says what to
                // draw inside. A picture part that will not resolve leaves the frame — the outline
                // and any line it carries — which is what makes a missing image visible as a hole
                // rather than as nothing at all.
                if (Shape(element, slide, theme, space, groupFill) is { } placed) Add(placed, shapes);
            }
        }
    }

    /// <summary>
    /// True when a shape states <c>p:cNvPr/@hidden</c>, and so is not drawn at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One attribute on the non-visual properties of any shape kind, read by every shape context
    /// in the importer (<c>oox/source/ppt/pptshapecontext.cxx:61</c>,
    /// <c>drawingml/shapecontext.cxx:79</c>, and the two group contexts beside them) and turned
    /// into <c>Visible = false</c> and <c>Printable = false</c> at
    /// <c>oox/source/drawingml/shape.cxx:1436-1442</c>.
    /// </para>
    /// <para>
    /// It is not a rarity confined to authoring artefacts. A corporate master routinely parks a
    /// hidden prompt — "&lt;Presentation Title – Change on Master Slide&gt;" — behind its real
    /// title, so a renderer that ignores the attribute prints instructions to the author on every
    /// slide of the deck.
    /// </para>
    /// </remarks>
    private static bool IsHidden(XElement element)
    {
        foreach (XElement child in element.Elements())
        {
            if (Ppt.Child(child, "cNvPr") is { } properties)
                return Ppt.Flag(properties, "hidden", whenAbsent: false);
        }
        return false;
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

    /// <summary>
    /// The fill a <c>p:grpSp</c> offers the children that ask for it with <c>a:grpFill</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Never painted. A group has no geometry of its own, so its <c>p:grpSpPr</c> fill exists
    /// only to be inherited — which is why this is resolved on the way down the tree rather than
    /// turned into a shape. A group may itself state <c>a:grpFill</c>, so the fill of the group
    /// above it is passed in and the chain resolves as far up as it is written.
    /// </para>
    /// <para>
    /// Measured on <c>slides/batch-002/pptx/iaeg_work_group_leader_updates.pptx</c>, an
    /// organisation chart whose eight working-group boxes sit in a group filled
    /// <c>bg2 lumMod 90000</c>: two state that colour outright and six say <c>a:grpFill</c>, so
    /// the reference draws eight tan boxes and Paperless drew two. Eight of the slides corpus's
    /// 112 PPTX decks use the element on a slide.
    /// </para>
    /// </remarks>
    private Paint? GroupFill(
        XElement group,
        PptxSlide slide,
        SlideTheme theme,
        AffineTransform space,
        Paint? outerFill)
    {
        XElement? properties = Ppt.Child(group, "grpSpPr");
        if (properties is null) return null;

        DocRect bounds = Bounds(Drawing.Child(properties, "xfrm"));
        FillContext fills = new(slide, theme.Colours, bounds.Size, space);

        return Fill(properties, [], fills, outerFill);
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

        DrawingTableStyle? style = DrawingTableStyle.Read(
            _file.TableStyles,
            Drawing.Child(Drawing.Child(table, "tblPr"), "tableStyleId")?.Value);

        return SlideTable.Place(
            DrawingTableGeometry.Read(table, theme.Colours, style, theme.Styles),
            local.Size,
            placement,
            cell => CellBody(cell, theme),
            _fonts,
            Name(frame));
    }

    private static SlideTextBody? CellBody(DrawingTableCellBox cell, SlideTheme theme)
    {
        if (cell.TextBody is not { } body || DrawingTextBody.IsEmpty(body)) return null;

        // The table style's text properties enter as an inherited level style rather than being
        // stamped onto each run, because that is the rung they belong to: a run stating its own
        // colour or weight still wins, and this is what it wins over.
        IReadOnlyList<XElement> themed = CellTextStyle(cell);

        return PptxTextBody.Read(
            body, theme.Colours, theme.MinorLatin,
            themed.Count == 0 ? null : _ => themed) with
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

    /// <summary>
    /// A table style's text properties as a level style, which is how the text reader takes them.
    /// </summary>
    /// <remarks>
    /// One synthetic <c>a:lvl1pPr</c> carrying an <c>a:defRPr</c>, returned for every outline
    /// level: a table cell has no outline and its style applies to all of its text. Building an
    /// element rather than post-processing the runs keeps one precedence rule in one place — the
    /// same chain a shape's placeholder and the master's text styles travel down.
    /// </remarks>
    private static IReadOnlyList<XElement> CellTextStyle(DrawingTableCellBox cell)
    {
        if (cell.TextColour is null && cell.Bold is null && cell.Italic is null) return [];

        XElement run = new(Drawing.Name("defRPr"));

        if (cell.Bold is { } bold) run.SetAttributeValue("b", bold ? "1" : "0");
        if (cell.Italic is { } italic) run.SetAttributeValue("i", italic ? "1" : "0");

        if (cell.TextColour is { } colour)
        {
            run.Add(new XElement(
                Drawing.Name("solidFill"),
                new XElement(
                    Drawing.Name("srgbClr"),
                    new XAttribute("val", $"{colour.R:X2}{colour.G:X2}{colour.B:X2}"))));
        }

        return [new XElement(Drawing.Name("lvl1pPr"), run)];
    }

    private PlacedShape? Shape(
        XElement shape, PptxSlide slide, SlideTheme theme, AffineTransform space,
        Paint? groupFill = null)
    {
        XElement? properties = Ppt.Child(shape, "spPr");
        XElement? transform = Drawing.Child(properties, "xfrm");

        // A placeholder inherits the whole of its layout's — and failing that its master's —
        // p:spPr, and what it states for itself wins property by property. A transform is only the
        // commonest thing to inherit: a title whose slide-level shape carries only its text takes
        // its rectangle this way, and falling back to a zero one puts every such shape in the
        // top-left corner at no size.
        //
        // **Moving a placeholder does not cut it off from the rest of what it inherits**, and
        // reading the chain only when the slide states no a:xfrm says that it does. LibreOffice
        // applies the reference on p:nvSpPr — before p:spPr is parsed at all
        // (oox/source/ppt/pptshapecontext.cxx:157-162) — and merges fill, line and geometry from
        // it underneath the shape's own (shape.cxx:2816-2843). Measured on
        // slides/batch-011/pptx/171128IPAP.pptx, whose slide titles state their own a:xfrm and
        // take their C00000 plate from the layout: 32 of its 40 pages lost the red banner behind
        // white title text, which is text that extracts perfectly and cannot be read.
        XElement?[] inherited = PlaceholderProperties(shape, slide);
        transform ??= First(inherited, "xfrm");
        if (transform is null && inherited.Length == 0 && properties is null) return null;

        DocRect local = Bounds(transform);
        if (local.Width <= Length.Zero && local.Height <= Length.Zero) return null;

        double turn = ShapeTransform.Radians(Rotation(transform));
        bool flipHorizontal = Drawing.Flag(transform, "flipH") ?? false;
        bool flipVertical = Drawing.Flag(transform, "flipV") ?? false;

        AffineTransform placement =
            ShapeTransform.Place(local, turn, flipHorizontal, flipVertical, space);

        // The same placement without the mirror, which is what the shape's *text* travels with.
        // See Text() for why the two differ.
        AffineTransform upright = flipHorizontal || flipVertical
            ? ShapeTransform.Place(local, turn, flipHorizontal: false, flipVertical: false, space)
            : placement;

        // What a parent group's child coordinate space multiplies this shape's own units by;
        // (1, 1) outside one, and outside the very common group that states a child space equal
        // to its own extent.
        (double scaleX, double scaleY) = ShapeTransform.ScaleOf(upright);

        XElement? geometry = Drawing.Child(properties, "prstGeom") ?? First(inherited, "prstGeom");
        string? preset = Drawing.Attribute(geometry, "prst");
        Dictionary<string, double>? adjustment = Adjustments(geometry);

        // a:custGeom states its own guides and paths, so it needs no preset name — and a shape
        // carrying one has no a:prstGeom at all.
        XElement? custom = Drawing.Child(properties, "custGeom") ?? First(inherited, "custGeom");

        CustomShapeGeometry.Geometry? own = custom is null
            ? null
            : CustomShapeGeometry.Custom(custom, local.Size);

        GraphicsPath outline = ShapeTransform.Apply(
            placement,
            own?.Outline ?? SlidePresetGeometry.Outline(preset, local.Size, adjustment));

        FillContext fills = new(slide, theme.Colours, local.Size, placement);
        DocRect bounds = ShapeTransform.PlacedBounds(placement, local.Size);

        // What the shape's p:style takes from the theme's format matrix. A shape stating neither
        // a fill nor an outline is still painted when it names one — a flowchart box outlined in
        // accent 1, a master's rule — and taking the elements here rather than inside Fill and
        // Line keeps the two indices resolved once per shape.
        XElement? style = Ppt.Child(shape, "style");
        XElement? themedFill = theme.Styles?.Fill(style, theme.Colours);
        XElement? themedLine = theme.Styles?.Line(style, theme.Colours);

        return new PlacedShape
        {
            Name = Name(shape),
            Outline = outline,
            Bounds = bounds,
            Fill = ShapeFill(shape, properties, inherited, themedFill, fills, groupFill),
            Picture = Picture(shape, slide, bounds),
            Line = Line(properties, inherited, theme.Colours, themedLine),
            HeadEnd = LineEnd(properties, inherited, "headEnd"),
            TailEnd = LineEnd(properties, inherited, "tailEnd"),
            Text = Text(
                shape,
                // Into slide units, because the type inside is already in them. A group scales
                // its children's coordinates and not their font sizes — LibreOffice decomposes
                // the cumulative matrix and gives the shape the absolute size the scale produces
                // (shape.cxx:1129-1140), then lays the text out in that. Leaving the rectangle in
                // the child space measures 12 pt text against a box a thousandth of an inch wide,
                // so every word is too wide for its line. The scale has to come back off the
                // matrix that carries the runs, which is what Text is given below.
                ShapeTransform.Scaled(
                    Mirrored(
                        TextRectangle(shape, local, own, preset, adjustment),
                        local.Size, flipHorizontal, flipVertical),
                    scaleX,
                    scaleY),
                ShapeTransform.WithoutScale(upright, scaleX, scaleY),
                theme),
            Shadow = Shadow(properties, inherited, style, theme),
        };
    }

    /// <summary>
    /// The drop shadow a shape casts, from its own effect list or from the theme's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The order is the shape's own <c>a:effectLst</c>, then the one it inherits from its
    /// placeholder, then the theme's — the same chain <c>Shape::getActualEffectProperties</c>
    /// walks (<c>oox/source/drawingml/shape.cxx:2868-2888</c>) — and the <em>first one that
    /// states an outer shadow</em> wins rather than the first one that exists.
    /// </para>
    /// <para>
    /// That distinction was measured rather than reasoned, and reading the source alone would
    /// have got it backwards. <c>EffectProperties::assignUsed</c> replaces the whole effect list
    /// when the source states any effect at all, which says a shape writing
    /// <c>&lt;a:effectLst/&gt;</c> — or one holding only an <c>a:glow</c> — drops the theme's
    /// shadow. The running binary does not: LibreOffice 24.2.7.2's own flat-ODF export of
    /// <c>slide-drop-shadow.pptx</c> gives the themed 38%-black shadow to all three of the shape
    /// that states nothing, the shape with an empty list and the shape with a glow. The binary
    /// made the reference PDFs, so the binary wins.
    /// </para>
    /// </remarks>
    private static SlideShadow? Shadow(
        XElement? properties, XElement?[] inherited, XElement? style, SlideTheme theme)
    {
        XElement?[] sources =
        [
            Drawing.Child(properties, "effectLst"),
            First(inherited, "effectLst"),
            DrawingEffects.ThemeEffects(theme.Styles, style),
        ];

        foreach (XElement? effects in sources)
        {
            if (DrawingEffects.OuterShadow(effects, theme.Colours) is not { } shadow) continue;

            return new SlideShadow(
                shadow.OffsetX, shadow.OffsetY, shadow.Colour, shadow.Opacity, shadow.Blur);
        }

        return null;
    }

    /// <summary>
    /// A text rectangle reflected about its shape's centre, for each axis the shape mirrors.
    /// </summary>
    /// <remarks>
    /// The geometry a mirrored shape draws is the mirrored geometry, and the text area belongs
    /// to the geometry — so an asymmetric preset's text moves with it. What does <em>not</em>
    /// move is the writing: see <see cref="Text"/>.
    /// </remarks>
    private static DocRect Mirrored(DocRect rectangle, DocSize box, bool horizontal, bool vertical)
    {
        if (!horizontal && !vertical) return rectangle;

        return new DocRect(
            horizontal ? box.Width - rectangle.X - rectangle.Width : rectangle.X,
            vertical ? box.Height - rectangle.Y - rectangle.Height : rectangle.Y,
            rectangle.Width,
            rectangle.Height);
    }

    /// <summary>
    /// The rectangle a shape's text is laid out in, in the shape's own coordinates.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Normally the geometry's: a preset states where its text goes, and a <c>a:custGeom</c>
    /// states its own <c>a:rect</c>. A SmartArt shape states it a third way and overrides both —
    /// <c>dsp:txXfrm</c>, an offset and extent in the <em>diagram's</em> coordinates rather than
    /// the shape's, which is why the shape's own offset comes off it here.
    /// </para>
    /// <para>
    /// Taking it at face value is a deliberate departure from LibreOffice, which cannot.
    /// <c>Transform2DContext</c> (<c>oox/source/drawingml/transform2dcontext.cxx:299-391</c>)
    /// says so in its own comment — "We cannot change the text area rectangle directly, because
    /// currently we depend on the geometry definition of the preset. As workaround we change the
    /// indents to move and scale the text block" — and its <c>ConstructPresetTextRectangle</c>
    /// hand-implements the text rectangle for <em>fourteen</em> presets and returns false for the
    /// rest, dropping the <c>dsp:txXfrm</c> entirely. Nothing here depends on the preset: the text
    /// rectangle is a parameter of text layout, so the file's own answer is usable as stated.
    /// </para>
    /// <para>
    /// The size of the divergence is measured rather than assumed. Of the 469 baked shapes in
    /// LibreOffice's corpus 286 carry a <c>dsp:txXfrm</c>, and <strong>273 of those — 95% — use
    /// one of the fourteen</strong>, where LibreOffice's indents land the text block on exactly
    /// the stated rectangle and the two agree. The other 13 are where this puts a label where
    /// PowerPoint does and LibreOffice does not:
    /// <c>sd/qa/unit/data/pptx/tdf149551_SmartArt_Gear.pptx</c> is one, on a <c>gear9</c>, whose
    /// 20 pt "Three" fits on one line for LibreOffice and wraps here.
    /// </para>
    /// </remarks>
    private static DocRect TextRectangle(
        XElement shape,
        DocRect local,
        CustomShapeGeometry.Geometry? custom,
        string? preset,
        Dictionary<string, double>? adjustment)
    {
        if (Ppt.Child(shape, "txXfrm") is { } stated)
        {
            DocRect area = Bounds(stated);
            if (area.Width > Length.Zero && area.Height > Length.Zero)
            {
                return new DocRect(area.X - local.X, area.Y - local.Y, area.Width, area.Height);
            }
        }

        return custom?.TextRectangle
               ?? SlidePresetGeometry.TextRectangle(preset, local.Size, adjustment);
    }

    /// <summary>
    /// The <c>p:spPr</c> of every placeholder a slide shape stands in for, nearest first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The layout's shapes are searched before the master's, which
    /// <see cref="PptxPlaceholder.Find"/> arranges by walking a concatenated list backwards —
    /// the same reversal LibreOffice relies on (<c>oox/source/ppt/pptshape.cxx:791</c>), and the
    /// reason searching the two trees in the obvious order lets the master win every tie.
    /// </para>
    /// <para>
    /// <strong>Both rungs, not just the nearest.</strong> A layout written by PowerPoint states
    /// <c>&lt;p:spPr/&gt;</c> — empty — on every placeholder it inherits unchanged, so stopping
    /// at the layout finds no <c>a:xfrm</c> and the shape is dropped for having no size. That is
    /// how a deck loses every slide title while keeping its body text, whose <c>idx</c> happens
    /// to match the master's placeholder directly. LibreOffice reaches the same rectangle by
    /// importing the master fragment and the layout fragment into one <c>SlidePersist</c>
    /// (<c>presentationfragmenthandler.cxx:246-296</c>), which leaves both shapes in the list
    /// the slide's placeholder searches.
    /// </para>
    /// </remarks>
    private XElement?[] PlaceholderProperties(XElement shape, PptxSlide slide)
    {
        if (_styles is null) return [];
        if (PptxPlaceholder.Read(shape, slide.Master, slide.Layout) is not { } placeholder)
            return [];

        (XElement? direct, XElement? inherited) = _styles.Placeholders(placeholder);
        return [Ppt.Child(direct, "spPr"), Ppt.Child(inherited, "spPr")];
    }

    private SlideTextBody? BodyOf(XElement shape, SlideTheme theme)
    {
        XElement? body = Ppt.Child(shape, "txBody");
        return body is null || DrawingTextBody.IsEmpty(body)
            ? null
            : PptxTextBody.Read(
                body, theme.Colours, theme.MinorLatin, _styles?.LevelPropertiesFor(shape),
                _fields, _styles?.BodyPropertiesFor(shape), _styles?.ShapeTextStyleFor(shape));
    }

    /// <summary>
    /// The text a shape draws, laid out in its text rectangle.
    /// </summary>
    /// <remarks>
    /// <strong>The placement is the shape's without its mirror.</strong> A flipped shape draws
    /// mirrored geometry and upright writing: LibreOffice records <c>flipH</c> on a preset or
    /// custom shape as <c>MirroredX</c> on the custom-shape properties
    /// (<c>oox/source/drawingml/shape.cxx:2146-2151</c>), which reflects the geometry alone —
    /// only the shapes with no attribute for it, pictures and the like, get the negative scale
    /// at <c>shape.cxx:1128</c> that would reflect everything. Mirroring the runs instead draws
    /// legible-looking text that reads backwards, and extracts backwards too, which is how the
    /// defect survives a visual check.
    /// </remarks>
    /// <param name="shape">The shape whose body is being read.</param>
    /// <param name="rectangle">
    /// Its text rectangle, at slide scale but still at the shape's own origin — so a parent
    /// group's scale is already in the extent and is not in the matrix.
    /// </param>
    /// <param name="placement">
    /// The matrix placing the shape, with any mirror and any group scale already removed.
    /// </param>
    /// <param name="theme">The theme, for run colours and the fallback typeface.</param>
    private PlacedText? Text(
        XElement shape, DocRect rectangle, AffineTransform placement, SlideTheme theme)
    {
        if (BodyOf(shape, theme) is not { } body) return null;

        // The text area's own turn, outside the body's and inside the shape's — see
        // TextAreaTurn. Folding it into the placement is what puts it in that order.
        double areaTurn = TextAreaTurn(shape);
        if (areaTurn != 0)
        {
            placement = AffineTransform.Concat(About(rectangle, areaTurn), placement);
        }

        // A body that turns its own text can never be upright, whatever the shape does: the runs
        // have to travel with a matrix, because a glyph run carries an origin and advances rather
        // than one of its own.
        if (body.Rotation != 0) return Turned(body, rectangle, placement);

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

    /// <summary>
    /// Lays a body out in a text frame turned inside its shape, and gives it the matrix.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:bodyPr/@rot</c>, and what a SmartArt <c>autoTxRot</c> resolves to. The turn is about
    /// the text rectangle's own centre and the shape itself does not move, so the only thing the
    /// layout sees is a different rectangle: a quarter turn transposes it — the lines now run down
    /// the shape and break at its height — and a half turn leaves it as it was. Laying out in the
    /// untransposed rectangle and rotating afterwards breaks the lines at the wrong width, which
    /// is a different paragraph rather than the same one turned.
    /// </para>
    /// <para>
    /// Anything nearer a quarter turn than not transposes, which is exact for the four angles
    /// <c>autoTxRot</c> produces and is the only defensible rule for an angle between them.
    /// </para>
    /// </remarks>
    private PlacedText? Turned(SlideTextBody body, DocRect rectangle, AffineTransform placement)
    {
        double centreX = rectangle.X.Emu + (rectangle.Width.Emu / 2.0);
        double centreY = rectangle.Y.Emu + (rectangle.Height.Emu / 2.0);

        bool quarter = Math.Abs(Math.Sin(body.Rotation)) > Math.Abs(Math.Cos(body.Rotation));

        DocRect area = quarter
            ? new DocRect(
                Length.FromEmu((long)(centreX - (rectangle.Height.Emu / 2.0))),
                Length.FromEmu((long)(centreY - (rectangle.Width.Emu / 2.0))),
                rectangle.Height,
                rectangle.Width)
            : rectangle;

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, _fonts);
        if (runs.Count == 0) return null;

        return new PlacedText(
            runs, AffineTransform.Concat(About(rectangle, body.Rotation), placement));
    }

    /// <summary>
    /// How far a shape's <em>text area</em> is turned, clockwise, in radians.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A SmartArt shape's <c>dsp:txXfrm</c> states the rectangle its text occupies, and may state
    /// a <c>rot</c> on it as well. That angle is not a second copy of the shape's own: it is
    /// stated <em>against</em> it, and the two add — <c>Transform2DContext</c> puts it straight
    /// into the same field <c>a:bodyPr/@rot</c> feeds, adding rather than replacing
    /// (<c>oox/source/drawingml/transform2dcontext.cxx:53-58</c>), and reads the sum back as
    /// "the rotation beyond compensation of the shape rotation" at
    /// <c>transform2dcontext.cxx:341-344</c>. So a chevron laid on its side by
    /// <c>&lt;a:xfrm rot="5400000"/&gt;</c> whose text area says <c>rot="-5400000"</c> keeps its
    /// writing horizontal, which is the case this exists for.
    /// </para>
    /// <para>
    /// <strong>It turns the laid-out box rather than transposing it</strong>, which is where it
    /// differs from <see cref="Turned"/>. LibreOffice scales the text box to the text range's
    /// stated width and height <em>first</em> and rotates the result about its centre
    /// (<c>svx/source/sdr/contact/viewcontactofsdrobjcustomshape.cxx:168-191</c>), so the lines
    /// still break at the width the file states; only <c>TextPreRotateAngle</c>, the turn a
    /// diagram's <c>upr</c> and <c>grav</c> produce, is applied before the scale and so changes
    /// the shape of the box. Transposing here would break "Sensorimotor" at 32 pt instead of
    /// 75 and overflow every chevron in the diagram.
    /// </para>
    /// <para>
    /// Reach, measured over the slides corpus: 15 of its 112 decks bake a diagram drawing, 13 of
    /// those carry a <c>dsp:txXfrm</c> (171 shapes), and <strong>3 state a non-zero
    /// <c>rot</c> on one</strong> — 18 shapes, all of them a quarter turn against a shape
    /// rotated the opposite quarter.
    /// </para>
    /// </remarks>
    private static double TextAreaTurn(XElement shape)
        => ShapeTransform.Radians(Rotation(Ppt.Child(shape, "txXfrm")));

    /// <summary>A turn, clockwise in radians, about a rectangle's own centre.</summary>
    private static AffineTransform About(DocRect rectangle, double radians)
    {
        double centreX = rectangle.X.Emu + (rectangle.Width.Emu / 2.0);
        double centreY = rectangle.Y.Emu + (rectangle.Height.Emu / 2.0);

        return AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-centreX, -centreY),
                AffineTransform.Rotation(radians)),
            AffineTransform.Translation(centreX, centreY));
    }

    /// <summary>The named child of the first source in a chain to carry one.</summary>
    private static XElement? First(XElement?[] sources, string name)
    {
        foreach (XElement? source in sources)
        {
            if (Drawing.Child(source, name) is { } child) return child;
        }
        return null;
    }

    /// <summary>True when a placement is a pure translation, so text needs no matrix.</summary>
    private static bool IsUpright(AffineTransform transform)
        => transform.A == 1 && transform.B == 0 && transform.C == 0 && transform.D == 1;

    /// <summary>
    /// A shape's fill: its own, then its placeholder's, then its <c>p:style</c>'s.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Four of DrawingML's six kinds. <c>a:pattFill</c> is left unpainted — it resolves into a
    /// tiled bitmap the reader would have to synthesise rather than read — and is in the TODO.
    /// The theme's style matrix comes last in the chain because that is the merging order
    /// <c>Shape::getActualFillProperties</c> uses: the theme is the base and anything the shape
    /// states wins over it, so a box stating <c>a:noFill</c> under an <c>a:fillRef</c> is empty.
    /// </para>
    /// <para>
    /// The fifth kind, <c>a:grpFill</c>, is not a fill at all but a reference to the enclosing
    /// group's: the reference threads the parent group's fill properties down the tree and
    /// <c>Shape::getActualFillProperties</c> uses them when the shape's own fill type is
    /// <c>XML_grpFill</c>. It ends the search either way — a shape asking for a group fill does
    /// not then fall through to its placeholder or its theme — so a group with no fill leaves the
    /// shape unfilled.
    /// </para>
    /// </remarks>
    /// <summary>
    /// A shape's fill, and what <c>p:sp/@useBgFill</c> does to it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>useBgFill="1"</c> means "fill this shape with the slide's own background". It is not a
    /// colour but a reference, and the shape that carries it normally states no fill and names a
    /// themed one — PowerPoint's Designer writes a full-slide <c>p:sp useBgFill="1"</c> whose
    /// <c>a:fillRef idx="1"</c> points at <c>accent1</c>, so a reader that walks past the
    /// attribute paints the whole slide in the accent colour instead of leaving the background
    /// showing. Measured on <c>slides/batch-006/pptx/Course Selection 2025-26 Current Grade
    /// 09.pptx</c>: nine of its ten pages came out solid orange against a white reference, 247
    /// points of unaccounted ink on a ten-page deck.
    /// </para>
    /// <para>
    /// The precedence is <c>oox/source/ppt/pptshapegroupcontext.cxx:109-113</c>, which sets the
    /// shape's own fill type to <c>XML_noFill</c> <em>before</em> parsing its children: a fill the
    /// shape states for itself still wins, and the theme's <c>a:fillRef</c> — which is merged in
    /// underneath rather than into the shape — no longer reaches it. Then
    /// <c>fillproperties.cxx:439-443</c> sets <c>FillUseSlideBackground</c> on exactly the
    /// <c>noFill</c> branch, so an explicit <c>a:noFill</c> beside the attribute shows the
    /// background too rather than nothing.
    /// </para>
    /// <para>
    /// What is drawn here is the background <em>paint</em> rather than the page behind the shape.
    /// The two are the same thing for a solid background, which is every corpus instance; for a
    /// gradient they differ, because LibreOffice shows the page's gradient in page coordinates and
    /// a paint re-anchors to the shape.
    /// </para>
    /// </remarks>
    private Paint? ShapeFill(
        XElement shape,
        XElement? properties,
        XElement?[] inherited,
        XElement? themedFill,
        in FillContext context,
        Paint? groupFill)
    {
        if (!Ppt.Flag(shape, "useBgFill", whenAbsent: false))
        {
            return Fill(properties, [.. inherited, themedFill], context, groupFill);
        }

        return Fill(properties, [], context, groupFill) ?? _background;
    }

    private Paint? Fill(
        XElement? properties, XElement?[] inherited, in FillContext context, Paint? groupFill)
    {
        foreach (XElement? source in (XElement?[])[properties, .. inherited])
        {
            if (source is null) continue;
            if (Drawing.Child(source, "noFill") is not null) return null;
            if (Drawing.Child(source, "grpFill") is not null) return groupFill;
            if (SolidFill(source, context.Theme, placeholder: null) is { } fill) return fill;
            if (Gradient(Drawing.Child(source, "gradFill"), context) is { } gradient) return gradient;
            if (Bitmap(Drawing.Child(source, "blipFill"), source, context) is { } bitmap) return bitmap;
            if (DrawingHatch.Read(Drawing.Child(source, "pattFill"), context.Theme) is { } hatch)
            {
                return hatch;
            }
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

        XElement? fill = Ppt.Child(shape, "blipFill");
        if (DrawingFill.ReadBlip(fill) is not { } blip) return null;

        // `BlipReference.Choose` rather than `blip.EmbedId`, because one `a:blip` may carry an
        // `asvg:svgBlip` in an extension beside the raster in `r:embed` — the vector is what
        // PowerPoint draws and the raster is what it shows a consumer that cannot.
        BlipReference.Choice choice = BlipReference.Choose(Drawing.Child(fill, "blip"));
        string? part = PartOf(shape, slide);

        PptxPicture picture = Loaded(choice.RelationshipId ?? blip.EmbedId, part);

        if (choice.IsVector && choice.FallbackRelationshipId is { } fallback)
        {
            picture = picture.Vector is null
                ? Loaded(fallback, part)
                : picture with { Raster = Loaded(fallback, part).Raster };
        }

        if (picture.IsEmpty) return null;

        DocRect area = blip.FillRect.IsWhole
            ? bounds
            : SlideImages.Inset(
                bounds, blip.FillRect.Left, blip.FillRect.Top,
                blip.FillRect.Right, blip.FillRect.Bottom);

        DocRect? destination = SlideImages.Uncropped(
            area, blip.SourceRect.Left, blip.SourceRect.Top,
            blip.SourceRect.Right, blip.SourceRect.Bottom);

        return destination is { } placed
            ? new PlacedPicture(
                  picture.Raster is { } raster
                      ? Duotoned(raster, blip, ThemeFor(slide).Colours)
                      : null,
                  placed,
                  Math.Clamp(blip.Opacity, 0, 1))
              { Vector = picture.Vector }
            : null;
    }

    /// <summary>
    /// The picture with its <c>a:duotone</c> attached, resolved against the theme.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Attached rather than applied: mapping a JPEG's pixels onto a ramp needs a codec, and a
    /// reader has none. <see cref="RasterImage.Duotone"/> carries the pair to whichever
    /// backend decodes the picture.
    /// </para>
    /// <para>
    /// This is how a theme paints one grey texture in a deck's own colours, and it is the
    /// largest single figure on the slides track's ink measurement:
    /// <c>order-of-worship-ppt-revised-2018.pptx</c> takes its whole background from
    /// <c>a:bgFillStyleLst</c>'s third entry, a stretched blip under a duotone, and drew as a
    /// dark grey vignette against a pale reference — 766.96 of unaccounted ink over 28 pages,
    /// 27% of the whole track's figure. <c>HENTZEN_…AEROSPACE_INDUSTRY.pptx</c> is the same
    /// mechanism at 127.20, its dark red banner coming out grey. 17 of the 112 corpus decks
    /// state one.
    /// </para>
    /// </remarks>
    private static RasterImage Duotoned(
        RasterImage image, DrawingBlipFill blip, DrawingTheme? theme)
    {
        if (blip.Duotone is not { } pair) return image;
        if (pair.Dark.Resolve(theme, placeholder: null) is not { } dark) return image;
        if (pair.Light.Resolve(theme, placeholder: null) is not { } light) return image;

        return image with { Duotone = new DuotoneRecolour(dark, light) };
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

        image = Duotoned(image, blip, context.Theme);

        (DocRect box, _) = GradientSpace(context);

        double opacity = Math.Clamp(blip.Opacity, 0, 1);

        if (!blip.Tile)
        {
            return new BitmapPaint(image, box.Size, box.Origin, Stretch: true)
                { Opacity = opacity };
        }

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
            Stretch: false)
            { Opacity = opacity };
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
    private RasterImage? Image(string? embedId, string? partName) => Loaded(embedId, partName).Raster;

    /// <summary>
    /// The picture an <c>r:embed</c> names, raster or vector, cached by part name.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Which of the two it is comes from the bytes.</b> Nothing else can say: LibreOffice writes a
    /// genuine EMF into a part named <c>.wmf</c>, an EMF+ has no signature of its own anywhere, and
    /// <c>[Content_Types].xml</c> is a producer's claim rather than a fact. <c>VectorImages.For</c> is
    /// the registry that knows which formats have a decoder, so it is what decides.
    /// </para>
    /// <para>
    /// A part that is not a vector becomes a <see cref="RasterImage.Encoded"/> whether or not it sniffs
    /// as a raster this library knows, which is deliberate: a TIFF is a real picture some backend may
    /// grow a codec for, and declining it here would lose the bytes rather than the drawing.
    /// </para>
    /// </remarks>
    private PptxPicture Loaded(string? embedId, string? partName)
    {
        if (embedId is null || partName is null) return default;
        if (_file.Relationship(partName, embedId) is not { IsExternal: false } relationship) return default;
        if (_images.TryGetValue(relationship.Target, out PptxPicture cached)) return cached;

        PptxPicture picture = default;
        if (_file.Package.GetPart(relationship.Target) is { } part)
        {
            using Stream content = part.Open();
            using MemoryStream buffer = new();
            content.CopyTo(buffer);

            if (buffer.Length > 0)
            {
                ReadOnlyMemory<byte> bytes = buffer.ToArray();

                picture = VectorImages.For(bytes.Span) is not null
                    ? new PptxPicture(null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)))
                    : new PptxPicture(RasterImage.Encoded(bytes, part.MediaType), null);
            }
        }

        _images[relationship.Target] = picture;
        return picture;
    }

    /// <summary>One package part's picture: a raster, or a vector decoded when something draws it.</summary>
    /// <param name="Raster">The encoded bytes, when they are not a vector this library reads.</param>
    /// <param name="Vector">The deferred decode, when they are.</param>
    private readonly record struct PptxPicture(RasterImage? Raster, Lazy<VectorImage>? Vector)
    {
        /// <summary>True when the part held nothing drawable.</summary>
        public bool IsEmpty => Raster is null && Vector is null;
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
    /// A shape's outline: its <c>a:ln</c>, then its placeholder's, over its <c>p:style</c>'s.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A line whose only child is <c>a:noFill</c> is not drawn at all, which is how
    /// <c>&lt;a:ln w="0"&gt;&lt;a:noFill/&gt;&lt;/a:ln&gt;</c> — what LibreOffice's own export
    /// writes for an unstroked shape — says "no outline" rather than "a hairline in black". It
    /// beats the theme's style matrix too: an arrow suppressing its outline under an
    /// <c>a:lnRef</c> has none.
    /// </para>
    /// <para>
    /// Anything else the shape states is laid <em>over</em> the themed line rather than replacing
    /// it, because a shape routinely states one half of a line and means the theme's for the
    /// other — <c>&lt;a:ln w="57150"/&gt;</c> under an <c>a:lnRef idx="1"</c> is the theme's
    /// colour at four and a half points. See <see cref="DrawingStyleMatrix.Overlay"/>.
    /// </para>
    /// </remarks>
    private static Stroke? Line(
        XElement? properties, XElement?[] inherited, DrawingTheme? theme, XElement? themed)
    {
        foreach (XElement? source in (XElement?[])[properties, .. inherited])
        {
            XElement? line = Drawing.Child(source, "ln");
            if (line is null) continue;
            if (Drawing.Child(line, "noFill") is not null) return null;
            if (themed is not null) line = DrawingStyleMatrix.Overlay(themed, line);
            if (Pen(line, theme) is { } stroke) return stroke;
        }

        return themed is null ? null : Pen(themed, theme);
    }

    /// <summary>
    /// The stroke one resolved <c>a:ln</c> draws, or null when it names no colour.
    /// </summary>
    /// <remarks>
    /// <c>w</c> is in EMUs and its absence means a hairline rather than nothing —
    /// <c>lineproperties.cxx</c> leaves the width unset and the draw layer draws the thinnest line
    /// the device can.
    /// </remarks>
    private static Stroke? Pen(XElement line, DrawingTheme? theme)
    {
        if (SolidFill(line, theme, placeholder: null) is not { } paint) return null;

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

    /// <summary>The marker one end of a line carries, from its own <c>a:ln</c> or its placeholder's.</summary>
    private static SlideLineEnd LineEnd(XElement? properties, XElement?[] inherited, string which)
    {
        foreach (XElement? source in (XElement?[])[properties, .. inherited])
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
