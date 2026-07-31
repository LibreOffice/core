using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Ooxml;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Ooxml;

/// <content>
/// What the walk records rather than resolves: tracked changes, bookmarks and a field's definition.
/// </content>
/// <remarks>
/// <para>
/// None of this changes what the reader extracts. A <c>w:ins</c>'s content is still read and a
/// <c>w:del</c>'s still skipped; what is added is the account of who did it, when, and — for a
/// deletion — which words, since those are in the file and deliberately not in the extracted text.
/// </para>
/// <para>
/// Offsets count the characters of the paragraph's own runs. That is neither the XML's own idea of a
/// position nor Word's, and it is the only one the four formats can agree on, since it is a property
/// of what was extracted rather than of how the file spelled it.
/// </para>
/// </remarks>
public sealed partial class DocxContentReader
{
    /// <summary>
    /// How far into the paragraph the walk has got, counting the text already emitted.
    /// </summary>
    /// <remarks>
    /// Computed from the paragraph rather than counted as text is emitted, because a mark is rare
    /// and a character is not: adding a counter to <c>Emit</c> would charge every document for
    /// something only a marked one uses. The half-built run has to be included, since text is
    /// buffered until its formatting changes.
    /// </remarks>
    private int CurrentOffset(ContentParagraph? paragraph)
    {
        if (paragraph is null) return 0;

        int offset = _pendingText.Length;
        foreach (ContentNode child in paragraph.Children)
        {
            if (child is ContentRun run) offset += run.Text.Length;
        }
        return offset;
    }

    /// <summary>The paragraph's text so far, the half-built run included.</summary>
    private string CurrentText(ContentParagraph? paragraph)
    {
        if (paragraph is null) return string.Empty;

        StringBuilder text = new();
        foreach (ContentNode child in paragraph.Children)
        {
            if (child is ContentRun run) text.Append(run.Text);
        }
        return text.Append(_pendingText).ToString();
    }

    /// <summary>The paragraph's text between two offsets, clamped to what exists.</summary>
    private string Slice(ContentParagraph? paragraph, int start, int end)
    {
        string text = CurrentText(paragraph);
        int from = Math.Clamp(start, 0, text.Length);
        int to = Math.Clamp(end, from, text.Length);
        return text[from..to];
    }

    // ------------------------------------------------------------------------- bookmarks

    /// <summary>
    /// Records the start of a bookmark, keyed by the id its end will name.
    /// </summary>
    /// <remarks>
    /// <c>w:id</c> and not <c>w:name</c>: DOCX pairs the two halves by the id, and a name may be
    /// repeated across a document that was assembled from several. LibreOffice's own importer keys
    /// its <c>m_aBookmarkMap</c> the same way (<c>DomainMapper_Impl::StartOrEndBookmark</c>), taking
    /// the name from the start element alone.
    /// </remarks>
    private void RecordBookmarkStart(XElement element, ContentParagraph? paragraph)
    {
        string? id = Word.Attribute(element, "id");
        string? name = Word.Attribute(element, "name");
        if (id is null || string.IsNullOrEmpty(name)) return;

        _marks.OpenBookmark(id, name, _marks.At(CurrentOffset(paragraph)));
    }

    private void RecordBookmarkEnd(XElement element, ContentParagraph? paragraph)
    {
        if (Word.Attribute(element, "id") is not { } id) return;
        _marks.CloseBookmark(id, _marks.At(CurrentOffset(paragraph)));
    }

    // --------------------------------------------------------------------- tracked changes

    /// <summary>
    /// Records an insertion around the content it covers, which is read either way.
    /// </summary>
    /// <param name="element">The <c>w:ins</c> or <c>w:moveTo</c>.</param>
    /// <param name="paragraph">The paragraph being read, or null at block level.</param>
    /// <param name="read">Reads the covered content, between the two positions.</param>
    private void RecordInsertion(XElement element, ContentParagraph? paragraph, Action read)
    {
        int startOffset = CurrentOffset(paragraph);
        WritingPosition? start = _marks.At(startOffset);

        read();

        int endOffset = CurrentOffset(paragraph);
        _marks.AddChange(
            WritingChangeKind.Insertion,
            Word.Attribute(element, "author"),
            ParseDate(Word.Attribute(element, "date")),
            Slice(paragraph, startOffset, endOffset),
            start,
            _marks.At(endOffset));
    }

    /// <summary>
    /// Records a deletion, and the words it removed, at the position they were removed from.
    /// </summary>
    /// <remarks>
    /// The range is empty because the text is not in the document: extraction skips a
    /// <c>w:delText</c> deliberately, so there is nothing between the two positions to cover. The
    /// text is carried on the record instead, which is the only place it survives.
    /// </remarks>
    private void RecordDeletion(XElement element, ContentParagraph? paragraph)
    {
        WritingPosition? at = _marks.At(CurrentOffset(paragraph));

        StringBuilder removed = new();
        foreach (XElement text in element.Descendants())
        {
            if (text.Name.NamespaceName != OoxmlNamespaces.WordprocessingML) continue;
            if (text.Name.LocalName is "delText" or "t") removed.Append(text.Value);
            else if (text.Name.LocalName == "tab") removed.Append('\t');
        }

        _marks.AddChange(
            WritingChangeKind.Deletion,
            Word.Attribute(element, "author"),
            ParseDate(Word.Attribute(element, "date")),
            removed.ToString(),
            at,
            at);
    }

    /// <summary>
    /// The instant a <c>w:date</c> names, or null when there is none.
    /// </summary>
    /// <remarks>
    /// The attribute is optional and its absence is ordinary rather than exceptional: LibreOffice
    /// omits it whenever the source stated no date — the corpus's <c>revisions.docx</c>, exported
    /// from an RTF whose <c>\revdttm</c> is zero, carries a <c>w:author</c> and no <c>w:date</c> at
    /// all. Round-tripped rather than assumed: a reader that required the date would find no changes
    /// in a file LibreOffice itself wrote.
    /// </remarks>
    private static DateTime? ParseDate(string? value)
        => DateTime.TryParse(
            value, CultureInfo.InvariantCulture,
            DateTimeStyles.AdjustToUniversal | DateTimeStyles.AllowWhiteSpaces,
            out DateTime parsed)
            ? parsed
            : null;

    // ---------------------------------------------------------------------------- fields

    /// <summary>
    /// Reads a <c>w:fldSimple</c>: the instruction is an attribute and the result is the content.
    /// </summary>
    /// <remarks>
    /// The same field as the <c>w:fldChar</c> form and a completely different spelling of it — one
    /// element with the code in <c>w:instr</c> rather than a flat run of markers around it. Word
    /// writes whichever it likes, so both have to be read or half the page numbers in a corpus are
    /// invisible.
    /// </remarks>
    private void ReadSimpleField(
        XElement element, ContentParagraph paragraph, string? paragraphStyleId, string? hyperlink)
    {
        int startOffset = CurrentOffset(paragraph);
        WritingPosition? start = _marks.At(startOffset);

        ReadRuns(element, paragraph, paragraphStyleId, hyperlink);

        int endOffset = CurrentOffset(paragraph);
        _marks.AddField(
            Word.Attribute(element, "instr"),
            Slice(paragraph, startOffset, endOffset),
            start,
            _marks.At(endOffset));
    }
}
