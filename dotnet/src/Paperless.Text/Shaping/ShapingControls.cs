namespace Paperless.Text.Shaping;

/// <summary>
/// The characters that must never reach a shaper, because a shaper draws a box for them.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice removes these before shaping rather than after: <c>ImplLayoutArgs::AddRun</c> splits every
/// run at each one and shapes the pieces (<c>vcl/source/text/ImplLayoutArgs.cxx</c>, the list is
/// <c>IsControlChar</c> at :111). So a control character contributes no glyph and no advance at all,
/// while still occupying a position in the text — which is what lets a caret sit either side of it and a
/// hint be attached at its index.
/// </para>
/// <para>
/// The distinction matters because none of these is in a text font's <c>cmap</c>, so a shaper handed one
/// returns <c>.notdef</c> with the face's own glyph-zero advance — 0.78 em in Liberation Serif and 0.75
/// in Liberation Sans, which is wider than most letters. That is ink on the page and room taken on the
/// line, for a character that means "something else stands here". Every word-processing reader emits
/// U+0001 for a picture, a floating frame, an embedded object and a comment mark, so the error is one
/// missing-glyph box per drawing in the document.
/// </para>
/// <para>
/// <strong>The tab is the one member of the C0 range that is kept.</strong> Its width is not a property
/// of the font — it advances to the next stop — so it is resolved by <c>TabRuler</c> against the
/// paragraph's stops, exactly as Writer resolves it with a tab portion rather than with a glyph.
/// Removing it here as LibreOffice does would be invisible on any line the ruler measures and would
/// silently zero it on one measured without a paragraph format, so it stays and the ruler keeps
/// deciding.
/// </para>
/// </remarks>
public static class ShapingControls
{
    /// <summary>
    /// True for a character a shaper must not be given.
    /// </summary>
    /// <remarks>
    /// LibreOffice's <c>IsControlChar</c> minus the tab: NUL, the rest of the C0 range, the directional
    /// marks and the embedding and override controls, the invisible operators and the deprecated format
    /// characters, the byte-order mark, and the two permanent non-characters.
    /// </remarks>
    public static bool IsRemovedBeforeShaping(char character) => character
        is '\u0000'
        or >= '\u0001' and <= '\u0008'
        or >= '\u000A' and <= '\u001F'
        or >= '\u200E' and <= '\u200F'
        or >= '\u2028' and <= '\u202E'
        or '\u2060'
        or >= '\u206A' and <= '\u206F'
        or '\uFEFF' or '\uFFFE' or '\uFFFF';

    /// <summary>
    /// Where the next character a shaper may be given is, at or after a position.
    /// </summary>
    /// <param name="text">The text being shaped.</param>
    /// <param name="from">Where to start looking.</param>
    /// <param name="limit">One past the last position to consider.</param>
    public static int NextShapable(ReadOnlySpan<char> text, int from, int limit)
    {
        int at = from;
        while (at < limit && IsRemovedBeforeShaping(text[at])) at++;
        return at;
    }

    /// <summary>
    /// Where the stretch of shapable characters beginning at a position ends.
    /// </summary>
    /// <param name="text">The text being shaped.</param>
    /// <param name="from">The stretch's first character, which must itself be shapable.</param>
    /// <param name="limit">One past the last position to consider.</param>
    public static int EndOfShapable(ReadOnlySpan<char> text, int from, int limit)
    {
        int at = from;
        while (at < limit && !IsRemovedBeforeShaping(text[at])) at++;
        return at;
    }
}
