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
}
