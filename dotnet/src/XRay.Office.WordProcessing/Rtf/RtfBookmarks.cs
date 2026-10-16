using System.Globalization;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Rtf;

/// <summary>
/// Names an RTF document's bookmarks the way <c>writerfilter</c> names them, which is not the way the
/// file states them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>RTF sends a bookmark half's name before its id, and the mapper is written for the opposite
/// order.</strong> <c>lcl_getBookmarkProperties</c>
/// (<c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx</c>:224-236) sets
/// <c>LN_CT_Bookmark_name</c> first — its own comment says <em>"If present, this should be sent
/// first"</em> — and <c>LN_CT_MarkupRangeBookmark_id</c> second, and
/// <c>DomainMapper::lcl_attribute</c> (<c>dmapper/DomainMapper.cxx</c>:340-347) routes the two to
/// <c>SetBookmarkName</c> and <c>StartOrEndBookmark</c> in that order. But
/// <c>DomainMapper_Impl::SetBookmarkName</c> (<c>DomainMapper_Impl.cxx</c>:9426-9447) is written for
/// OOXML, where a <c>w:bookmarkStart</c> states its <c>w:id</c> and <em>then</em> its <c>w:name</c>:
/// it looks the *previously opened* start up by <c>m_sCurrentBkmkId</c> and, when that start is still
/// open, writes the incoming name onto it, reaching the <c>m_sCurrentBkmkName</c> that OOXML wants
/// only when the map misses.
/// </para>
/// <para>
/// So every name after the first lands one bookmark early, and a paragraph that opens five bookmarks
/// hands out five names one place shifted. Reproduced here because it is what the reference reads,
/// and because a <c>REF</c> field expands from whatever bookmark ends up holding its name — see
/// <see cref="RtfReferenceFields"/>. A document whose bookmarks never overlap is unaffected: the
/// rotation only bites where a second start is opened before the first is closed.
/// </para>
/// <para>
/// The ids are <c>RTFDocumentImpl</c>'s own (<c>rtfdocumentimpl.cxx</c>:2735-2764): a start takes
/// <c>m_aBookmarks.size()</c> and records it under its name, and an end takes
/// <c>m_aBookmarks[name]</c>, which default-constructs <b>0</b> for a name no start ever used. Both
/// are reproduced exactly, because both decide which half pairs with which.
/// </para>
/// </remarks>
internal sealed class RtfBookmarkRotation
{
    /// <summary><c>RTFDocumentImpl::m_aBookmarks</c>: a name's id.</summary>
    private readonly Dictionary<string, int> _ids = new(StringComparer.Ordinal);

    /// <summary><c>DomainMapper_Impl::m_aBookmarkMap</c>: the name an open start is holding.</summary>
    private readonly Dictionary<int, string> _open = [];

    /// <summary>The names already given out, for the duplicate rule below.</summary>
    private readonly HashSet<string> _taken = new(StringComparer.Ordinal);

    /// <summary><c>MarkManager::m_aMarkBasenameMapUniqueOffset</c>: where to resume counting.</summary>
    private readonly Dictionary<string, int> _copies = new(StringComparer.Ordinal);

    /// <summary><c>m_sCurrentBkmkId</c>: the most recently opened start, or null once one closed.</summary>
    private int? _currentId;

    /// <summary><c>m_sCurrentBkmkName</c>: a name waiting for the start that follows it.</summary>
    private string _pendingName = string.Empty;

    /// <summary>What one <c>\bkmkstart</c> or <c>\bkmkend</c> does.</summary>
    /// <param name="Id">The id the two halves pair by, which is the key rather than the name.</param>
    /// <param name="Opens">True when this half opened a bookmark rather than closing one.</param>
    /// <param name="Name">
    /// The name the closed bookmark carries — which is generally not the name on this half. Empty
    /// while <paramref name="Opens"/> is true, because a start's name is not settled until it closes.
    /// </param>
    internal readonly record struct Half(int Id, bool Opens, string Name);

    /// <summary>Runs one bookmark half through the importer's two steps.</summary>
    /// <param name="name">The name the destination's text gives, which may be empty.</param>
    /// <param name="start">True for a <c>\bkmkstart</c>.</param>
    public Half Take(string name, bool start)
    {
        int id;
        if (start)
        {
            // `int nPos = m_aBookmarks.size(); m_aBookmarks[aStr] = nPos;` -- a repeated name is
            // overwritten with the current size rather than keeping its first id.
            id = _ids.Count;
            _ids[name] = id;
        }
        else if (!_ids.TryGetValue(name, out id))
        {
            // std::map::operator[] on a name no start used inserts a value-initialised 0, and the
            // insertion moves every later start's id along with it.
            id = 0;
            _ids[name] = 0;
        }

        // SetBookmarkName. An empty name is not sent at all -- lcl_getBookmarkProperties omits the
        // attribute -- so an unnamed half rotates nothing and only takes its turn at the ids.
        if (name.Length > 0)
        {
            if (_currentId is { } current && _open.ContainsKey(current)) _open[current] = name;
            else _pendingName = name;
        }

        // StartOrEndBookmark.
        if (_open.TryGetValue(id, out string? held))
        {
            _open.Remove(id);
            _currentId = null;
            return new Half(id, false, Unique(held));
        }

        _open[id] = _pendingName;
        _pendingName = string.Empty;
        _currentId = id;
        return new Half(id, true, string.Empty);
    }

    /// <summary>
    /// The name a mark ends up with once Writer has made it unique.
    /// </summary>
    /// <remarks>
    /// <c>MarkManager::getUniqueMarkName</c> (<c>sw/source/core/doc/docbm.cxx</c>:1826-1863) appends
    /// <c>STR_MARK_COPY</c> — <c>"%1 Copy "</c> — and the lowest free number from 1. The rotation
    /// hands the same name out twice whenever two bookmarks close in the order their starts opened,
    /// so this is reached by ordinary documents rather than by malformed ones.
    /// </remarks>
    private string Unique(string name)
    {
        if (_taken.Add(name)) return name;

        int count = _copies.TryGetValue(name, out int resume) ? resume : 1;
        while (!_taken.Add(name + " Copy " + count.ToString(CultureInfo.InvariantCulture))) count++;

        _copies[name] = count + 1;
        return name + " Copy " + count.ToString(CultureInfo.InvariantCulture);
    }
}

/// <summary>
/// What a <c>REF</c> field draws, which is read out of its bookmark rather than out of the file.
/// </summary>
/// <remarks>
/// <para>
/// A <c>REF</c> naming a bookmark becomes a <c>SwGetRefField</c> of subtype <c>Bookmark</c> and part
/// <c>TEXT</c> (<c>DomainMapper_Impl.cxx</c>:8541-8635), and Writer recomputes it on load from the
/// bookmark's own text — so the <c>{\fldrslt}</c> the producer cached is not what the reference
/// draws whenever the two disagree, which is exactly what the naming rotation above arranges.
/// </para>
/// <para>
/// <c>SwGetRefFieldType::FindAnchor</c> (<c>sw/source/core/fields/reffld.cxx</c>:1559-1591) gives the
/// range: the start's node and offset, and an end which is the end offset when both halves are in
/// one node, the node's length for a collapsed cross-reference bookmark, the start for any other
/// collapsed one, and <b>−1</b> when the two halves are in different nodes.
/// <c>SwGetRefField::UpdateField</c> (<c>:603-607</c>) reads that −1 as <em>to the end of the
/// paragraph</em>, which is where a mis-ended bookmark turns into a whole paragraph of drawn text.
/// A name no mark holds draws <c>STR_GETREFFLD_REFITEMNOTFOUND</c> (<c>:594-598</c>).
/// </para>
/// </remarks>
internal static class RtfReferenceFields
{
    /// <summary>
    /// The expansion of every <c>REF</c> in the document whose bookmark says something other than the
    /// cached result, keyed by the bookmark's name; null when nothing needs replacing.
    /// </summary>
    /// <remarks>
    /// Computed after the first read, because a <c>REF</c> may name a bookmark that has not been
    /// reached yet, and applied by a second one: a token stream has nothing to revisit, and rewriting
    /// a paragraph that has already closed would rebase every offset counted against it — its runs,
    /// its notes, its frames and its own marks. Only a document that states a <c>REF</c> pays for it.
    /// </remarks>
    public static Dictionary<string, string>? Expansions(WritingMarks marks)
    {
        if (marks.Fields.Count == 0 || marks.Bookmarks.Count == 0) return null;

        Dictionary<string, WritingBookmark> byName = new(StringComparer.Ordinal);
        foreach (WritingBookmark bookmark in marks.Bookmarks)
        {
            // findMark answers the one holding the name; a later duplicate was renamed on insertion.
            byName.TryAdd(bookmark.Name, bookmark);
        }

        Dictionary<string, string>? expansions = null;
        foreach (WritingField field in marks.Fields)
        {
            if (FieldInstructions.ReferenceBookmark(field.Instruction) is not { } name) continue;
            if (expansions is not null && expansions.ContainsKey(name)) continue;

            // A name we hold no bookmark for keeps the producer's cached result rather than drawing
            // the reference's error string: our own bookmark table is what the lookup missed, and
            // inventing an error message is a worse answer than the one the file cached.
            if (!byName.TryGetValue(name, out WritingBookmark? bookmark)) continue;

            string expansion = Expand(bookmark);
            if (string.Equals(expansion, field.Result, StringComparison.Ordinal)) continue;

            (expansions ??= new Dictionary<string, string>(StringComparer.Ordinal))[name] = expansion;
        }

        return expansions;
    }

    /// <summary>The text a bookmark hands a <c>REF</c> field.</summary>
    private static string Expand(WritingBookmark bookmark)
    {
        WritingRange range = bookmark.Range;
        string text = range.Start.Paragraph.Text;
        int start = Math.Clamp(range.Start.Offset, 0, text.Length);
        int end;

        if (range.IsEmpty)
        {
            // #i81002#: a collapsed *cross-reference* bookmark stands for its whole node. Writer
            // makes one of those for a heading it references, and names it this way.
            end = ReferenceFieldText.IsCrossReference(bookmark.Name) ? text.Length : start;
        }
        else if (ReferenceEquals(range.Start.Paragraph, range.End.Paragraph))
        {
            end = Math.Clamp(range.End.Offset, start, text.Length);
        }
        else
        {
            end = text.Length;
        }

        return ReferenceFieldText.Filter(text[start..end]);
    }
}
