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

    public PptxTextStyles(XElement? layout, XElement? master, XElement? defaultTextStyle, bool isNotesPage)
    {
        _layout = layout;
        _master = master;
        _defaultTextStyle = defaultTextStyle;
        _isNotesPage = isNotesPage;
    }

    /// <summary>
    /// The text-body options for a shape on this slide.
    /// </summary>
    /// <param name="shape">The slide's shape.</param>
    /// <param name="resolveHyperlink">Resolves an <c>r:id</c> declared by the slide part.</param>
    public DrawingTextOptions For(XElement shape, Func<string, string?> resolveHyperlink)
    {
        ArgumentNullException.ThrowIfNull(shape);

        PptxPlaceholder? placeholder = PptxPlaceholder.Read(shape, _master);

        // Resolved once per shape rather than once per level: the match does not depend on the
        // level, and a deck with a hundred paragraphs in one placeholder would otherwise search
        // the master's shape tree a hundred times.
        XElement? direct = null;
        XElement? inherited = null;
        if (placeholder is { } key)
        {
            List<XElement> masterShapes = [.. PptxPlaceholder.ShapesOf(_master)];
            List<XElement> layoutShapes = [.. PptxPlaceholder.ShapesOf(_layout)];

            direct = key.Find([.. masterShapes, .. layoutShapes]);

            // The layout placeholder has a placeholder of its own on the master; that second hop
            // is what makes a three-level chain rather than a two-level one.
            if (direct is not null && !masterShapes.Contains(direct)
                && PptxPlaceholder.Read(direct, _master) is { } layoutKey)
                inherited = layoutKey.Find(masterShapes);
        }

        string? textStyle = placeholder?.TextStyle(_isNotesPage);

        return new DrawingTextOptions
        {
            ResolveHyperlink = resolveHyperlink,
            InheritedLevelProperties = level =>
                [.. Chain(direct, inherited, textStyle, level)],
        };
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
