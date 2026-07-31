using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// An ODF text document, with the page geometry a Writer document has and a spreadsheet does not.
/// </summary>
/// <remarks>
/// <para>
/// A wrapper rather than a subclass, because <see cref="OdfDocument"/> serves all three families and
/// lives in <c>Paperless.OpenDocument</c> — below this library in the dependency order, so it cannot
/// know what a Writer section is. Wrapping keeps the layering intact and costs one delegation per
/// member.
/// </para>
/// <para>
/// The sections are read here rather than during the content walk because ODF states them nowhere near
/// the content: a paragraph reaches its page setup through its paragraph style's master page, so the
/// answer comes from the style tables and not from the body.
/// </para>
/// </remarks>
public sealed class OdtWordDocument : IWordProcessingDocument
{
    private readonly OdfDocument _inner;

    internal OdtWordDocument(OdfDocument inner)
    {
        ArgumentNullException.ThrowIfNull(inner);
        _inner = inner;
        Sections = ReadSections(inner.File.Styles);
    }

    /// <inheritdoc/>
    public DocumentFormat Format => _inner.Format;

    /// <inheritdoc/>
    public DocumentFamily Family => _inner.Family;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => _inner.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content => _inner.Content;

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => _inner.Diagnostics;

    /// <inheritdoc/>
    public IReadOnlyList<WritingSection> Sections { get; }

    /// <summary>The underlying ODF file: its styles, master pages and remaining parts.</summary>
    public OdfFile File => _inner.File;

    /// <inheritdoc/>
    public void Dispose() => _inner.Dispose();

    /// <summary>
    /// One section per master page the document defines.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the same thing as one section per page break, which is what the other three formats give —
    /// ODF has no section list, only masters and the styles that reach them. Deciding which master
    /// applies where needs the page-break chain, and that needs layout. So this reports the geometries
    /// the document defines, with the <c>Standard</c> master first because that is what a paragraph
    /// naming no master gets.
    /// </para>
    /// <para>
    /// A document with no masters at all still gets one section of default geometry, which matches what
    /// LibreOffice does with such a file rather than leaving a caller with nothing to lay out on.
    /// </para>
    /// </remarks>
    private static List<WritingSection> ReadSections(OdfStyles styles)
    {
        List<WritingSection> sections = [];

        foreach (OdfMasterPage master in styles.MasterPages.Values
                     .OrderBy(m => m.Name == StandardMasterName ? 0 : 1)
                     .ThenBy(m => m.Name, StringComparer.Ordinal))
        {
            sections.Add(OdfPageGeometry.Read(styles, master));
        }

        if (sections.Count == 0) sections.Add(OdfPageGeometry.Read(styles, master: null));
        return sections;
    }

    /// <summary>
    /// The master page a paragraph naming none is laid on.
    /// </summary>
    /// <remarks>
    /// Not localised in the file: ODF stores the internal name and keeps the translated one in
    /// <c>style:display-name</c>, so matching on this is safe in every language.
    /// </remarks>
    private const string StandardMasterName = "Standard";
}
