using System.Xml.Linq;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Resolves the per-level paragraph properties a slide's text inherits.
/// </summary>
/// <remarks>
/// <para>
/// This is the chain the TODO calls the most common cause of wrong-looking slides:
/// </para>
/// <code>
/// shape → layout placeholder → master placeholder → master p:txStyles → p:defaultTextStyle
/// </code>
/// <para>
/// resolved <em>per text level</em>, because <c>a:lstStyle</c> is a nine-entry array and a
/// paragraph at level 2 inherits from every source's level-2 entry, not from the first source
/// that has an <c>a:lstStyle</c> at all. Merging whole list styles instead of levels is the
/// mistake that gives a nested bullet its parent's indent and marker.
/// </para>
/// <para>
/// For <em>extraction</em> only the bullet actually shows up in the output, but the chain still
/// has to be walked correctly to find it: a PowerPoint-authored deck states the bullet once, on
/// the master, and never on the slide. A LibreOffice-authored deck states it the other way
/// round — the exporter writes no <c>p:txStyles</c> at all and instead leaves the master
/// placeholder's own demonstration paragraphs ("Second Outline Level", one per level) carrying
/// the per-level properties, which is why those paragraphs are part of the chain here and not
/// only the <c>a:lstStyle</c> the specification points at.
/// </para>
/// </remarks>
internal sealed class PptxTextStyles
{
    private readonly XElement? _layout;
    private readonly XElement? _master;
    private readonly XElement? _defaultTextStyle;
    private readonly bool _isNotesPage;
    private readonly DrawingTheme? _theme;

    public PptxTextStyles(
        XElement? layout,
        XElement? master,
        XElement? defaultTextStyle,
        bool isNotesPage,
        DrawingTheme? theme = null)
    {
        _layout = layout;
        _master = master;
        _defaultTextStyle = defaultTextStyle;
        _isNotesPage = isNotesPage;
        _theme = theme;
    }

    /// <summary>
    /// The text-body options for a shape on this slide.
    /// </summary>
    /// <param name="shape">The slide's shape.</param>
    /// <param name="resolveHyperlink">Resolves an <c>r:id</c> declared by the slide part.</param>
    public DrawingTextOptions For(XElement shape, Func<string, string?> resolveHyperlink)
    {
        ArgumentNullException.ThrowIfNull(shape);

        return new DrawingTextOptions
        {
            ResolveHyperlink = resolveHyperlink,
            InheritedLevelProperties = LevelPropertiesFor(shape),
            Theme = _theme,
            ShapeTextStyle = ShapeTextStyle(shape),
        };
    }

    /// <summary>
    /// The per-level property sources a shape's text inherits, most specific first.
    /// </summary>
    /// <remarks>
    /// Separate from <see cref="For"/> because rendering needs the same chain without the rest
    /// of the extraction options: a bullet, an indent and a run's size all come out of it, and
    /// a second implementation would let the two readers disagree about which level a nested
    /// paragraph inherits from.
    /// </remarks>
    /// <param name="shape">The shape whose text body is being read.</param>
    public Func<int, IReadOnlyList<XElement>> LevelPropertiesFor(XElement shape)
    {
        ArgumentNullException.ThrowIfNull(shape);

        PptxPlaceholder? placeholder = PptxPlaceholder.Read(shape, _master, _layout);

        // Resolved once per shape rather than once per level: the match does not depend on the
        // level, and a deck with a hundred paragraphs in one placeholder would otherwise search
        // the master's shape tree a hundred times.
        (XElement? direct, XElement? inherited) = Placeholders(placeholder);

        string? textStyle = placeholder?.TextStyle(_isNotesPage);

        return level => [.. Chain(direct, inherited, textStyle, level)];
    }

    /// <summary>
    /// The <c>a:bodyPr</c> a shape inherits from the placeholders behind it, nearest first.
    /// </summary>
    /// <remarks>
    /// Empty for a shape that is not a placeholder. See <see cref="PptxTextBody"/> for why the
    /// body properties inherit at all — a slide's <c>&lt;a:bodyPr/&gt;</c> is silence, not an
    /// instruction, and PowerPoint writes one on every placeholder it has not re-formatted.
    /// </remarks>
    /// <param name="shape">The shape whose text body is being read.</param>
    public XElement?[] BodyPropertiesFor(XElement shape)
    {
        ArgumentNullException.ThrowIfNull(shape);

        PptxPlaceholder? placeholder = PptxPlaceholder.Read(shape, _master, _layout);
        if (placeholder is null) return [];

        (XElement? direct, XElement? inherited) = Placeholders(placeholder);
        return
        [
            Drawing.Child(Ppt.Child(direct, "txBody"), "bodyPr"),
            Drawing.Child(Ppt.Child(inherited, "txBody"), "bodyPr"),
        ];
    }

    /// <summary>
    /// The layout placeholder a shape stands in for, and the master placeholder behind that one.
    /// </summary>
    /// <remarks>
    /// The layout placeholder has a placeholder of its own on the master; that second hop is what
    /// makes a three-level chain rather than a two-level one, and it is the reason a title whose
    /// layout states nothing still finds the master's rectangle, list style and prompt geometry.
    /// </remarks>
    public (XElement? Direct, XElement? Inherited) Placeholders(PptxPlaceholder? placeholder)
    {
        if (placeholder is not { } key) return (null, null);

        List<XElement> masterShapes = [.. PptxPlaceholder.ShapesOf(_master)];
        List<XElement> layoutShapes = [.. PptxPlaceholder.ShapesOf(_layout)];

        XElement? direct = key.Find([.. masterShapes, .. layoutShapes]);

        XElement? inherited = null;
        if (direct is not null && !masterShapes.Contains(direct)
            && PptxPlaceholder.Read(direct, _master) is { } layoutKey)
            inherited = layoutKey.Find(masterShapes);

        return (direct, inherited);
    }

    /// <summary>
    /// The character properties the shape's own <c>p:style</c> contributes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The shape's own, and only its own.</b> A placeholder inherits nearly everything from
    /// the layout placeholder it stands in for, so inheriting the style reference too looks
    /// obviously right — and is wrong. <c>Shape::applyShapeReference</c>
    /// (<c>oox/source/drawingml/shape.cxx</c>:565-587) copies the shape properties, the
    /// <em>resolved</em> line, fill and effect properties, the custom-shape geometry and the
    /// master text list style, and does not touch <c>maShapeStyleRefs</c> — so the
    /// <c>a:fontRef</c> is not among the things a placeholder acquires.
    /// </para>
    /// <para>
    /// Measured on <c>deck-text-style.pptx</c>, whose seventh shape is a slide placeholder with
    /// no style of its own whose layout placeholder states an <c>a:fontRef</c> of accent 5:
    /// LibreOffice draws it in plain black in its default face, not in accent 5 and not in the
    /// theme's minor font. Inheriting it would have coloured a placeholder on every deck whose
    /// layouts style their placeholders, which is most PowerPoint-authored decks.
    /// </para>
    /// </remarks>
    /// <remarks>
    /// <para>
    /// Public because <em>rendering needs it too</em>, and for a while did not have it. The
    /// extraction path takes it inside <see cref="For"/>'s options; the rendering path builds its
    /// text body from <see cref="LevelPropertiesFor"/> and <see cref="BodyPropertiesFor"/>, and
    /// neither of those carries a rung that has no element in the text body. So a shape whose own
    /// <c>a:fontRef</c> names a colour extracted with that colour and drew in whatever the master
    /// gave it — the "resolved for extraction, taken literally by rendering" shape, which this
    /// project has now shipped three times.
    /// </para>
    /// <para>
    /// The symptom on <c>Sector_Skills_Insights…pptx</c>: body placeholders stating
    /// <c>&lt;a:fontRef idx="minor"&gt;&lt;a:schemeClr val="lt1"/&gt;</c> over a master
    /// <c>bodyStyle</c> stating <c>tx1</c>, so eleven of its twenty-four pages drew black body
    /// text on a dark teal panel. LibreOffice's own flat-ODF export of it resolves those runs to
    /// <c>fo:color="#ffffff"</c>.
    /// </para>
    /// </remarks>
    /// <param name="shape">The slide's shape.</param>
    public DrawingCharacterStyle? ShapeTextStyleFor(XElement shape)
    {
        ArgumentNullException.ThrowIfNull(shape);
        return ShapeTextStyle(shape);
    }

    private DrawingCharacterStyle? ShapeTextStyle(XElement shape)
    {
        if (Ppt.Child(shape, "style") is not { } style) return null;

        DrawingCharacterStyle resolved = DrawingCharacterStyle.FromShapeStyle(style, _theme);
        return resolved.IsEmpty ? null : resolved;
    }

    private IEnumerable<XElement> Chain(
        XElement? direct, XElement? inherited, string? textStyle, int level)
    {
        foreach (XElement source in FromPlaceholder(direct, level)) yield return source;
        foreach (XElement source in FromPlaceholder(inherited, level)) yield return source;

        // A shape that is not a placeholder falls to the master's otherStyle, which is what the
        // "other" in its name means: everything that is not a title and not an outline.
        XElement? masterStyle = Ppt.Child(Ppt.Child(_master, "txStyles"), textStyle ?? "otherStyle");
        if (DrawingTextBody.LevelProperties(masterStyle, level) is { } fromMaster) yield return fromMaster;

        // A placeholder's chain ends at the master's own style for its kind. p:defaultTextStyle
        // is reached only by a shape that found none — PPTShape::createAndInsert picks the
        // title, body or notes style by placeholder subtype and consults
        // getDefaultTextStyle() strictly under "if (!aMasterTextListStyle)"
        // (oox/source/ppt/pptshape.cxx:257-291 and 492-497). Letting a title fall through to it
        // is not a harmless extra rung: a deck converted from .ppt states
        // <a:buChar char="•"/> at every level of p:defaultTextStyle, so every title on every
        // slide acquires a bullet the reference does not draw.
        if (textStyle is not null) yield break;

        if (DrawingTextBody.LevelProperties(_defaultTextStyle, level) is { } fromDefault)
            yield return fromDefault;
    }

    /// <summary>
    /// The level-<paramref name="level"/> properties a layout or master placeholder contributes.
    /// </summary>
    /// <remarks>
    /// Two sources, in this order. The placeholder's <c>a:lstStyle</c> is what the specification
    /// intends and what PowerPoint writes. Its <em>paragraphs</em> are what LibreOffice writes —
    /// <c>PPTShape::setTextMasterStyles</c> (<c>oox/source/ppt/pptshape.cxx:162-233</c>) pushes a
    /// master placeholder's paragraph properties into the presentation style for that outline
    /// level, so they are a genuine per-level source rather than sample text to ignore. Only the
    /// first paragraph at the level counts; the rest are repetitions of it.
    /// </remarks>
    private static IEnumerable<XElement> FromPlaceholder(XElement? placeholder, int level)
    {
        if (placeholder is null) yield break;

        XElement? body = Ppt.Child(placeholder, "txBody");
        if (body is null) yield break;

        if (DrawingTextBody.LevelProperties(Drawing.Child(body, "lstStyle"), level) is { } fromStyle)
            yield return fromStyle;

        foreach (XElement paragraph in Drawing.Children(body, "p"))
        {
            XElement? properties = Drawing.Child(paragraph, "pPr");
            if ((Drawing.Number(properties, "lvl") ?? 0) != level) continue;
            if (properties is not null) yield return properties;
            yield break;
        }
    }
}
