using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.MsBinary.PropertySets;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Reads legacy binary Word documents: <c>doc</c> and the <c>dot</c> template, as written by Word 97
/// through Word 2003.
/// </summary>
/// <remarks>
/// <para>
/// A DOC file is an OLE2 compound file whose <c>WordDocument</c> stream begins with the FIB — a
/// versioned table of contents locating everything else. Almost nothing lives in that stream except
/// the text: styles, formatting indexes, the piece table and the note tables all live in a second
/// stream, and <em>which</em> second stream is a bit in the FIB. Reading <c>0Table</c> when the
/// document says <c>1Table</c> finds a stale copy left behind by an earlier save, which parses
/// cleanly and describes a different document.
/// </para>
/// <para>
/// Metadata comes from the OLE property sets rather than from anything Word-specific, so it is read
/// by the shared reader in <c>Paperless.MsBinary</c>
/// (<c>research/05-infrastructure.md</c> section G.3).
/// </para>
/// </remarks>
public static class DocReader
{
    /// <summary>The name of the stream holding the FIB and the document's text.</summary>
    public const string WordDocumentStreamName = "WordDocument";

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    /// <exception cref="MalformedDocumentException">
    /// The file is not a Word 97-2003 document: it has no <c>WordDocument</c> stream, or that stream
    /// does not begin with a FIB this reader understands.
    /// </exception>
    /// <exception cref="PasswordRequiredException">The document is encrypted.</exception>
    public static Ww8Document Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        CompoundFile file = CompoundFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];

            byte[] wordDocument = ReadStream(file, WordDocumentStreamName)
                ?? throw new MalformedDocumentException(
                    "The compound file has no WordDocument stream, so it is not a Word document.");

            // Parsing the FIB is also the format check: it rejects a stream that does not begin with
            // the WW8 signature, and one written by Word 95 or earlier — which shares the container
            // and the signature but numbers its sprms differently, so it would be misread rather
            // than merely read incompletely.
            Ww8Fib fib = Ww8Fib.Parse(wordDocument);

            if (fib.IsEncrypted)
            {
                throw new PasswordRequiredException(
                    "The document is encrypted.", passwordWasSupplied: false);
            }

            // The FIB names which of the two table streams is current. A document that has been
            // saved repeatedly usually contains both, and the stale one parses without complaint.
            string tableName = fib.UsesTable1Stream ? "1Table" : "0Table";
            byte[] table = ReadStream(file, tableName) ?? [];
            if (table.Length == 0)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2310",
                    $"The document names {tableName} as its table stream, but that stream is "
                    + "missing or empty; formatting and structure cannot be resolved."));
            }

            Ww8DocumentReader reader = new(wordDocument, table, fib, diagnostics);
            ContentDocument content = reader.Read(OlePropertySetReader.Read(file));

            return new Ww8Document(format, file, content, diagnostics, reader.Sections);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// The whole of one stream, or null when the compound file has no such stream.
    /// </summary>
    /// <remarks>
    /// Read whole rather than streamed because every WW8 structure is reached by an offset another
    /// structure states: the piece table points into the text stream, the FIB points into the table
    /// stream, and an FKP is found by a page number. None of that can be done over a forward-only
    /// reader.
    /// </remarks>
    private static byte[]? ReadStream(CompoundFile package, string name)
    {
        IPackagePart? part = package.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new(part.Length > 0 ? (int)Math.Min(part.Length, int.MaxValue) : 0);
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

/// <summary>A legacy binary Word document that has been read.</summary>
public sealed class Ww8Document : IWordProcessingDocument
{
    private readonly CompoundFile _file;

    internal Ww8Document(
        DocumentFormat format,
        CompoundFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<Model.WritingSection> sections)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
        Sections = sections.Count > 0 ? sections : [new Model.WritingSection()];
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.WordProcessing;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Model.WritingSection> Sections { get; }

    /// <summary>
    /// The underlying compound file, for callers that need a stream the content tree does not
    /// expose — embedded objects and pictures, above all. Valid until this document is disposed.
    /// </summary>
    public CompoundFile File => _file;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
