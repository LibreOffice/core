namespace Paperless.WordProcessing.Layout;

/// <summary>
/// The line-box conventions that are Writer's own rather than every text engine's.
/// </summary>
/// <remarks>
/// LibreOffice formats a word processing document's body text through Writer's own layout and
/// everything drawn <em>on</em> it — a chart, a shape's caption, an embedded object — through
/// EditEngine, and the two do not agree about the line box. Naming the difference once, here, is
/// what stops it being re-decided at each of the six places that resolve a face's metrics.
/// </remarks>
internal static class WriterLineBox
{
    /// <summary>
    /// Writer charges a face's external leading to the ascent, so it sits above the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwFntObj::GetFontAscent</c> adds the external leading to the ascent it read from the device
    /// (<c>sw/source/core/txtnode/fntcache.cxx</c>:326-329) and <c>GetFontHeight</c> adds the same
    /// leading to ascent-plus-descent (<c>:370-371</c>), so the descent stays the face's own and
    /// ascent + descent closes on the line height.
    /// </para>
    /// <para>
    /// EditEngine does the opposite by default and is therefore left alone:
    /// <see cref="Paperless.Text.Fonts.LineMetrics.ScaledAscent"/> carries that half, the citations
    /// for both, and the measurements that separate them.
    /// </para>
    /// <para>
    /// Because the leading cancels between two lines of one paragraph — it leaves line <i>n</i>'s
    /// descent and arrives in line <i>n</i>+1's ascent — the only place this is observable is the
    /// first baseline on a page, measured against the top margin. Liberation Sans at 11 pt in a 72 pt
    /// top margin: LibreOffice 82.3008 pt, and 81.95 without the leading.
    /// </para>
    /// </remarks>
    internal const bool LeadingAboveText = true;
}
