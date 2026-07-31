using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless.WordProcessing.Rtf;

/// <summary>Reads Rich Text Format documents.</summary>
/// <remarks>
/// RTF is the one word-processing format with no container: the whole document is one byte stream
/// of nested groups. So there is no package to open and no parts to resolve — the reader is the
/// tokeniser plus the state machine in <see cref="RtfDocumentReader"/>.
/// </remarks>
public static class RtfReader
{
    /// <summary>
    /// The largest RTF document Paperless will buffer.
    /// </summary>
    /// <remarks>
    /// RTF is read whole rather than streamed, because its state machine can be sent back to an
    /// earlier destination at any point and because the format is verbose enough that a large
    /// document is still modest in memory. The cap bounds what a hostile input can allocate.
    /// </remarks>
    public const long MaxDocumentBytes = 512L * 1024 * 1024;

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    public static RtfDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        byte[] data = ReadAll(source.Stream);
        if (!LooksLikeRtf(data))
        {
            throw new MalformedDocumentException(
                "The document does not begin with '{\\rtf', so it is not Rich Text Format.");
        }

        List<Diagnostic> diagnostics = [];
        RtfDocumentReader reader = new(data, diagnostics);
        ContentDocument content = reader.Read();

        return new RtfDocument(format, content, diagnostics, reader.Sections);
    }

    /// <summary>
    /// True when the bytes start with an RTF signature, allowing for leading whitespace and a
    /// byte-order mark.
    /// </summary>
    /// <remarks>
    /// Some producers write a UTF-8 BOM before the signature even though RTF is not UTF-8, and
    /// mail systems add leading blank lines. Neither makes the document unreadable, so neither is
    /// treated as fatal.
    /// </remarks>
    private static bool LooksLikeRtf(ReadOnlySpan<byte> data)
    {
        if (data.Length >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) data = data[3..];
        while (data.Length > 0 && data[0] is (byte)' ' or (byte)'\t' or (byte)'\r' or (byte)'\n')
            data = data[1..];

        return data.StartsWith("{\\rtf"u8);
    }

    private static byte[] ReadAll(Stream stream)
    {
        if (stream.CanSeek)
        {
            if (stream.Length - stream.Position > MaxDocumentBytes)
            {
                throw new MalformedDocumentException(
                    $"The document is larger than the {MaxDocumentBytes / (1024 * 1024)} MB "
                    + "Paperless will buffer for RTF.");
            }

            byte[] exact = new byte[stream.Length - stream.Position];
            stream.ReadExactly(exact);
            return exact;
        }

        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

/// <summary>An RTF document that has been read.</summary>
public sealed class RtfDocument : IWordProcessingDocument
{
    internal RtfDocument(
        DocumentFormat format,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<Model.WritingSection> sections)
    {
        Format = format;
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

    /// <inheritdoc/>
    /// <remarks>
    /// Nothing to release: RTF has no container, and the document was read from a buffer the
    /// caller owns.
    /// </remarks>
    public void Dispose() { }
}
