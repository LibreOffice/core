using Paperless.Core.Documents;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing;

/// <summary>
/// A word-processing document that knows its page geometry.
/// </summary>
/// <remarks>
/// <para>
/// Separate from <see cref="IDocument"/> because page geometry is a layout concern and extraction must
/// not pay for it — but read eagerly rather than deferred, because all four formats state it in the
/// same part of the file they were already parsing. Reading it costs a handful of attribute lookups
/// where laying the document out costs fonts, shaping and a rasteriser.
/// </para>
/// <para>
/// It lives here rather than in <c>Paperless.Core</c> because a slide and a spreadsheet page are not
/// the same thing as a Writer section, and the abstraction that would cover all three has nothing in
/// it.
/// </para>
/// </remarks>
public interface IWordProcessingDocument : IDocument
{
    /// <summary>
    /// The document's sections, in document order, never empty.
    /// </summary>
    /// <remarks>
    /// A document with no section properties of its own still gets one section carrying the defaults,
    /// so a caller never has to decide what to do with an empty list. That is also what the formats
    /// mean: a DOCX with no <c>w:sectPr</c> is one section of default geometry, not no pages.
    /// </remarks>
    IReadOnlyList<WritingSection> Sections { get; }

    /// <summary>
    /// What the document records over its text rather than in it: tracked changes, bookmarks and
    /// fields.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <em>record</em>, not a change of behaviour. Extraction still says what the tracked changes
    /// leave, a field still reports its cached result, and a bookmark still contributes no text —
    /// this is the part all four readers used to resolve and throw away.
    /// </para>
    /// <para>
    /// Read during the extraction walk rather than deferred, because every one of these is stated
    /// in the same part of the file the reader was already parsing and only a marked document pays
    /// for it at all: a file with no bookmarks, no revisions and no fields builds nothing here.
    /// <see cref="WritingMarks.Empty"/> is what such a document reports.
    /// </para>
    /// </remarks>
    WritingMarks Marks { get; }
}
