namespace Paperless.WordProcessing.Rtf;

/// <summary>One entry from an RTF <c>{\stylesheet}</c> group.</summary>
/// <param name="Name">The style's name, as the trailing text of its definition.</param>
/// <param name="BasedOn">The style this one is based on, or null.</param>
/// <param name="OutlineLevel">
/// The zero-based outline level the style gives its paragraphs, or null for body text.
/// </param>
/// <param name="IsCharacterStyle">True for a <c>\cs</c> character style.</param>
public readonly record struct RtfStyle(
    string Name,
    int? BasedOn,
    int? OutlineLevel,
    bool IsCharacterStyle);

/// <summary>
/// The style names an RTF document declares.
/// </summary>
/// <remarks>
/// <para>
/// RTF's stylesheet is far less load-bearing than ODF's or DOCX's, and for a reason worth
/// knowing: RTF requires a writer to emit a paragraph's <em>effective</em> formatting inline
/// after the <c>\s</c> that names its style. Applying the style is the writer's job, not the
/// reader's, so there is no cascade to resolve here.
/// </para>
/// <para>
/// What the stylesheet is still needed for is the parts that are not repeated inline: the style's
/// name, which is what a caller recognises, and its outline level, which is what makes a
/// paragraph a heading when the body does not restate it.
/// </para>
/// </remarks>
public sealed class RtfStyles
{
    private readonly Dictionary<int, RtfStyle> _paragraphStyles = [];
    private readonly Dictionary<int, RtfStyle> _characterStyles = [];

    /// <summary>Records a style definition read from the stylesheet.</summary>
    internal void Add(int id, RtfStyle style)
    {
        if (style.IsCharacterStyle) _characterStyles[id] = style;
        else _paragraphStyles[id] = style;
    }

    /// <summary>The paragraph style with this <c>\s</c> id, or null.</summary>
    public RtfStyle? ParagraphStyle(int id)
        => _paragraphStyles.TryGetValue(id, out RtfStyle style) ? style : null;

    /// <summary>The character style with this <c>\cs</c> id, or null.</summary>
    public RtfStyle? CharacterStyle(int id)
        => _characterStyles.TryGetValue(id, out RtfStyle style) ? style : null;

    /// <summary>
    /// The outline level a paragraph style gives, following <c>\sbasedon</c> when the style
    /// itself does not say.
    /// </summary>
    /// <remarks>
    /// Cycle-guarded: a <c>\sbasedon</c> loop is malformed but appears in files written by
    /// converters, and this walks a chain over untrusted input.
    /// </remarks>
    public int? OutlineLevelOf(int id)
    {
        HashSet<int> visited = [];
        int? current = id;

        while (current is { } styleId && visited.Add(styleId))
        {
            if (_paragraphStyles.TryGetValue(styleId, out RtfStyle style))
            {
                if (style.OutlineLevel is { } level) return level;
                current = style.BasedOn;
            }
            else
            {
                return null;
            }
        }
        return null;
    }
}
