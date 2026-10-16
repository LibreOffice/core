using XRay.Office.MsBinary.Records;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// The identifiers of the hyperlinks a deck declares, which is what decides whether a text
/// range's <c>InteractiveInfo</c> becomes a field.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A <c>TxInteractiveInfoAtom</c> is not by itself a hyperlink.</strong>
/// <c>PPTTextObj</c>'s <c>PPT_PST_InteractiveInfo</c> case searches
/// <c>SdrPowerPointImport::m_aHyperList</c> for an entry whose index matches the
/// <c>InteractiveInfoAtom</c>'s <c>exHyperlinkId</c>, and everything that makes the run a
/// field — the <c>SvxURLField</c>, the underline, the scheme colour — happens inside that
/// loop (<c>filter/source/msfilter/svdfppt.cxx</c>:6907-6941). With no matching entry the
/// record is stepped over and the text is drawn exactly as it stands. This is the legacy
/// twin of DrawingML's <em>the hyperlink property map must be non-empty</em>, and it is not
/// a formality: <c>BUS-Chapter 05.ppt</c> states <strong>57</strong> live text ranges and
/// declares no hyperlink at all, and 26.2.4.2 draws every one of them in the body's own
/// colour, word-broken and unlinked.
/// </para>
/// <para>
/// <strong>The list is filled from the document summary and <em>indexed</em> from here.</strong>
/// <c>ImplSdPPTImport::Import</c> builds one entry per link of the
/// <c>_PID_HLINKS</c> blob in the user-defined property section
/// (<c>sd/source/filter/ppt/pptin.cxx</c>:353-518, six properties per link) and then gives the
/// <em>k</em>th entry the <em>k</em>th <c>ExHyperlinkAtom</c> of the <c>ExObjList</c>
/// (<c>:531-549</c>); when the blob yields no entry at all it builds one per
/// <c>ExHyperlink</c> instead (<c>:551-575</c>).
/// </para>
/// <para>
/// <strong>So the blob's yield <em>caps</em> the set of indices that can match, and it is not
/// the same as the count the blob declares.</strong> What stood here — "either way the set is
/// the set of <c>ExHyperlinkAtom</c> values", on a census that found the declared count equal
/// to the atom count on all 27 documents stating either — is withdrawn: the census counted
/// what the blob <em>declares</em> and the loop is fed a buffer the property reader has
/// truncated, on <strong>9 of the corpus's 51 <c>.ppt</c></strong>. See
/// <see cref="PptHyperlinkBlob"/> for the arithmetic, the ten-variant measurement at 26.2.4.2
/// that establishes it, and what an over-created field costs a slide.
/// </para>
/// <para>
/// <strong>It must be read off the <em>live</em> document container.</strong> A <c>.ppt</c>
/// stream keeps every superseded version of every object, and a scan of the stream finds
/// orphaned <c>ExObjList</c> records as readily as the current one — on
/// <c>080214-Intl-pol-frameworks…ppt</c> a stream scan finds a dead list declaring one link
/// where the live one declares two, which would have left the second URL unlinked. The
/// container this is given comes from the persist directory, so the question does not arise.
/// </para>
/// </remarks>
internal static class PptHyperlinks
{
    /// <summary>
    /// The <c>ExHyperlinkAtom</c> values the document's <c>ExObjList</c> declares.
    /// </summary>
    /// <remarks>
    /// Every value is kept, duplicates included, because the count is what bounds the entry
    /// list; membership is all a text range asks of it.
    /// </remarks>
    /// <param name="stream">The <c>PowerPoint Document</c> stream.</param>
    /// <param name="document">The live <c>Document</c> container.</param>
    /// <param name="summary">
    /// The whole <c>\u0005DocumentSummaryInformation</c> stream, which is what says how many of
    /// these identifiers the reference can actually hand out. Empty for a compound file with no
    /// such stream, and for a caller that has none to give: every identifier then resolves,
    /// which is what this did before the cap was read at all.
    /// </param>
    public static HashSet<uint> Read(
        DffRecordBuffer stream, DffRecordHeader document, ReadOnlySpan<byte> summary = default)
    {
        ArgumentNullException.ThrowIfNull(stream);

        HashSet<uint> ids = [];

        if (stream.FirstChild(document, PptRecordTypes.ExObjList) is not { } list) return ids;

        // A blob that yields nothing leaves the list empty, and the reference then falls back to
        // one entry per ExHyperlink record -- so zero and absent both mean "no cap".
        int cap = PptHyperlinkBlob.Count(summary) ?? 0;

        // Counted in RECORDS rather than in distinct identifiers: the reference's loop is over
        // the entries it built, one ExHyperlink each, and a deck that states the same identifier
        // on several records -- ws_prod-...-Aercap states 2 five times -- would otherwise be
        // allowed more records than it has entries.
        int taken = 0;

        foreach (DffRecordHeader link in stream.Children(list))
        {
            if (link.Type != PptRecordTypes.ExHyperlink) continue;
            if (cap > 0 && taken >= cap) break;

            taken++;

            foreach (DffRecordHeader atom in stream.Children(link))
            {
                if (atom.Type != PptRecordTypes.ExHyperlinkAtom) continue;

                ReadOnlySpan<byte> content = stream.Content(atom);
                if (content.Length >= 4) ids.Add(DffRecordBuffer.ReadUInt32(content));
                break;
            }
        }

        return ids;
    }
}
