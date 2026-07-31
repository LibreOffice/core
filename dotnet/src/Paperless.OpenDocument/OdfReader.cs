using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless.OpenDocument;

/// <summary>
/// The shared half of reading an ODF document: opening it, merging its styles, reading its
/// metadata, and handing the body to a family-specific walk.
/// </summary>
/// <remarks>
/// Everything up to <see cref="ReadBody"/> is identical for <c>odt</c>, <c>ods</c> and
/// <c>odp</c> — the container, the three style collections, <c>meta.xml</c>, and the flat-XML
/// variant. Only the shape of the body differs: a text flow, a set of sheets, or a set of
/// slides.
/// </remarks>
public abstract class OdfReader
{
    /// <summary>The family this reader produces documents for.</summary>
    protected abstract DocumentFamily Family { get; }

    /// <summary>
    /// Whether the master pages' headers and footers are read as content after the body.
    /// </summary>
    /// <remarks>
    /// True for word processing, where a header is document text a reader sees on every page.
    /// False for presentations, where a master slide's shapes are the deck's furniture and
    /// repeating them once per master would add text no slide actually shows; and for
    /// spreadsheets, where headers exist only in print settings.
    /// </remarks>
    protected virtual bool IncludesMasterPageContent => false;

    /// <summary>
    /// Reads a document, leaving the source's stream to the caller to dispose.
    /// </summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">
    /// The identified format, which is recorded on the result. Identification has already
    /// happened by this point; this reader does not sniff.
    /// </param>
    public OdfDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        OdfFile file = OdfFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];
            ContentDocument content = new() { Metadata = OdfMetadata.Read(file.Meta, Family) };

            XElement? body = file.Body;
            if (body is null)
            {
                // A document with no body is legal-ish and readable — its metadata and styles
                // are still there — so this is a diagnostic rather than an exception.
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2040",
                    "The document has no office:body, so it has no content to extract."));
            }
            else
            {
                OdfContentReader reader = new(file, diagnostics);
                ReadBody(body, reader, content);

                if (IncludesMasterPageContent)
                    ReadMasterPageHeadersAndFooters(file, reader, content);

                // Anything left hoisted belongs to no particular section — a note in a
                // construct the family reader did not attribute to one — so it lands at the
                // end rather than being dropped.
                foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
            }

            return new OdfDocument(format, Family, file, content, diagnostics);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Walks the family's body element, appending sections to the content document.
    /// </summary>
    /// <param name="body">
    /// The <c>office:text</c>, <c>office:spreadsheet</c> or <c>office:presentation</c>
    /// element.
    /// </param>
    /// <param name="reader">The shared content walker, already bound to the document.</param>
    /// <param name="content">The tree being built.</param>
    protected abstract void ReadBody(XElement body, OdfContentReader reader, ContentDocument content);

    /// <summary>
    /// Adds the header and footer content of every master page that has any.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Header and footer text is real document content that LibreOffice's own text export
    /// drops, so extracting it is a deliberate improvement rather than a difference to chase
    /// down.
    /// </para>
    /// <para>
    /// Every master carrying content is read, rather than only those a layout would use.
    /// Deciding which masters are reachable needs the page-break chain, and that needs
    /// layout; in practice LibreOffice writes only the masters a document actually uses, and a
    /// master with no header or footer produces nothing here. A template carrying unused
    /// masters is the case where this extracts more than a reader would show.
    /// </para>
    /// </remarks>
    protected static void ReadMasterPageHeadersAndFooters(
        OdfFile file, OdfContentReader reader, ContentDocument content)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(reader);
        ArgumentNullException.ThrowIfNull(content);

        int index = 0;
        foreach (Styles.OdfMasterPage master in file.Styles.MasterPages.Values.OrderBy(m => m.Name,
                                                                                      StringComparer.Ordinal))
        {
            // Left and first-page variants only appear when they differ from the shared one,
            // so reading all of them yields each distinct piece of text exactly once.
            AddIfPresent(master.Header, SectionKind.Header, master.Name);
            AddIfPresent(master.LeftHeader, SectionKind.Header, master.Name);
            AddIfPresent(master.FirstHeader, SectionKind.Header, master.Name);
            AddIfPresent(master.Footer, SectionKind.Footer, master.Name);
            AddIfPresent(master.LeftFooter, SectionKind.Footer, master.Name);
            AddIfPresent(master.FirstFooter, SectionKind.Footer, master.Name);
        }

        void AddIfPresent(XElement? element, SectionKind kind, string masterName)
        {
            if (element is null || !element.HasElements) return;

            ContentSection section = new() { Kind = kind, Index = index, Name = masterName };
            reader.ReadBlocks(element, section);
            if (section.Children.Count == 0) return;

            content.Children.Add(section);
            index++;
        }
    }
}
