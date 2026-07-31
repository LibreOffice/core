using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Reading <c>w:altChunk</c>: a whole other document embedded in the package by reference.
/// </summary>
/// <remarks>
/// <para>
/// The element is a placeholder at block level with an <c>r:id</c> and nothing else. What it
/// points at is a complete file — a DOCX, an RTF, an ODT, an HTML fragment, or plain text — that
/// a consumer is expected to import at that position. Word does the import when the file is
/// opened and then rewrites the document without the chunk, which is why a document containing
/// one has usually been produced by a generator rather than by Word.
/// </para>
/// <para>
/// So the reading is exactly "run the reader again on another part and splice the result in".
/// The chunk's own body sections become blocks of the host at the chunk's position, and its
/// notes and comments are hoisted alongside the host's — which is where they would have ended up
/// had the two documents been one. Its headers and footers are dropped, as Word drops them:
/// page furniture belongs to the section that owns the page, and the chunk does not own one.
/// </para>
/// </remarks>
public sealed partial class DocxContentReader
{
    /// <summary>
    /// How many chunks deep a read may go.
    /// </summary>
    /// <remarks>
    /// A chunk can embed a chunk, and nothing stops a package from embedding itself — the
    /// relationship is by part name and a self-reference is well-formed. Four levels is more than
    /// any real document and turns an infinite recursion into a diagnostic.
    /// </remarks>
    private const int MaximumChunkDepth = 4;

    /// <summary>
    /// The depth of the current chunk read.
    /// </summary>
    /// <remarks>
    /// Static and thread-static rather than an instance field, because the recursion does not run
    /// through this object: a nested chunk is read by a whole new reader built by
    /// <see cref="WordProcessingReader"/>, so nothing an instance holds reaches it.
    /// </remarks>
    [ThreadStatic]
    private static int _chunkDepth;

    /// <summary>Reads an embedded chunk into the host, or reports why it could not be.</summary>
    private void ReadAltChunk(XElement element, ContentNode target)
    {
        string? relationshipId = Word.RelationshipId(element);

        if (_file.Relationship(relationshipId) is not { IsExternal: false } relationship
            || _file.Package.GetPart(relationship.Target) is not { } part)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2120",
                "The document embeds a chunk (w:altChunk) whose part cannot be resolved"
                + (relationshipId is null ? ": it names no relationship." : $": '{relationshipId}'.")));
            return;
        }

        if (_chunkDepth >= MaximumChunkDepth)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2120",
                $"Embedded chunks (w:altChunk) nest more than {MaximumChunkDepth} deep; "
                + $"'{part.Name}' and anything below it have been skipped.",
                new DiagnosticLocation(part.Name)));
            return;
        }

        _chunkDepth++;
        try
        {
            Splice(part, target);
        }
        finally
        {
            _chunkDepth--;
        }
    }

    private void Splice(IPackagePart part, ContentNode target)
    {
        DocumentFormat format;
        using DocumentSource source = DocumentSource.FromStream(part.Open(), part.Name);

        try
        {
            format = SourceIdentification.Resolve(source);
        }
        catch (Exception exception) when (exception is IOException or MalformedDocumentException)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2120",
                $"An embedded chunk (w:altChunk) could not be read: {exception.Message}",
                new DiagnosticLocation(part.Name)));
            return;
        }

        // Sniffed rather than taken from the part's content type. The content type is what the
        // package *says* the chunk is, and an altChunk's is frequently wrong — a generator that
        // writes an RTF chunk and declares it as HTML is a common enough bug that Word ignores
        // the declaration too. Detection by content is the house rule for the same reason.
        WordProcessingReader reader = new();
        if (!reader.SupportedFormats.Contains(format))
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2120",
                $"The document embeds a chunk (w:altChunk) holding {Describe(format)}, which is "
                + "not a format Paperless reads; its content is not extracted.",
                new DiagnosticLocation(part.Name)));
            return;
        }

        IDocument chunk;
        try
        {
            chunk = reader.Read(source);
        }
        catch (Exception exception) when (exception is UnsupportedFormatException
                                             or MalformedDocumentException
                                             or IOException)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2120",
                $"An embedded chunk (w:altChunk) of {format} could not be read: "
                + $"{exception.Message}",
                new DiagnosticLocation(part.Name)));
            return;
        }

        using (chunk)
        {
            foreach (Diagnostic diagnostic in chunk.Diagnostics) _diagnostics.Add(diagnostic);

            foreach (ContentNode node in chunk.Content.Children)
            {
                if (node is ContentSection { Kind: SectionKind.Body } body)
                {
                    // The body's blocks, not the section itself: a section is the host's own
                    // division of the document, and nesting one inside a paragraph list — or
                    // inside a table cell, which is also a legal place for a chunk — would put a
                    // structure there that no consumer expects.
                    foreach (ContentNode block in body.Children) target.Children.Add(block);
                }
                else
                {
                    // Notes and comments, which the chunk hoisted out of its own body for the
                    // same reason the host hoists its own.
                    _hoisted.Add(node);
                }
            }
        }
    }

    /// <summary>How an unreadable chunk's format is named in a diagnostic.</summary>
    /// <remarks>
    /// <c>Unknown</c> is the interesting case and deserves different wording: HTML and plain text
    /// are the two content types <c>w:altChunk</c> was invented for, and neither is a format this
    /// library claims, so they sniff as nothing rather than as something unsupported.
    /// </remarks>
    private static string Describe(DocumentFormat format)
        => format == DocumentFormat.Unknown ? "an unrecognised format" : $"a {format} document";
}
