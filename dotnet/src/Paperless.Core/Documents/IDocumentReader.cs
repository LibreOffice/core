using Paperless.Core.Formats;

namespace Paperless.Core.Documents;

/// <summary>
/// Reads one or more formats into an <see cref="IDocument"/>.
/// </summary>
/// <remarks>
/// Each format library (<c>Paperless.WordProcessing</c>, <c>Paperless.Spreadsheets</c>,
/// <c>Paperless.Presentations</c>) exposes readers through this interface, and the
/// <c>Paperless</c> facade package composes them behind a single entry point.
/// </remarks>
public interface IDocumentReader
{
    /// <summary>The formats this reader handles.</summary>
    IReadOnlyCollection<DocumentFormat> SupportedFormats { get; }

    /// <summary>
    /// Reads a document.
    /// </summary>
    /// <param name="source">The bytes to read, and any options needed to read them.</param>
    /// <returns>The loaded document. The caller owns it and must dispose it.</returns>
    /// <exception cref="UnsupportedFormatException">
    /// The source is not one of <see cref="SupportedFormats"/>.
    /// </exception>
    /// <exception cref="MalformedDocumentException">
    /// The document is damaged beyond what lenient parsing can recover.
    /// </exception>
    /// <exception cref="PasswordRequiredException">
    /// The document is encrypted and no usable password was supplied.
    /// </exception>
    IDocument Read(DocumentSource source);
}

/// <summary>
/// The input to <see cref="IDocumentReader.Read"/>: where the bytes come from,
/// plus the options that affect how they are interpreted.
/// </summary>
public sealed class DocumentSource : IDisposable
{
    private readonly bool _ownsStream;

    private DocumentSource(Stream stream, bool ownsStream, string? fileName)
    {
        Stream = stream;
        _ownsStream = ownsStream;
        FileName = fileName;
    }

    /// <summary>
    /// A seekable stream over the document. Readers seek freely — office
    /// containers are random-access by nature — so a non-seekable input must be
    /// buffered first, which <see cref="FromStream"/> does for you.
    /// </summary>
    public Stream Stream { get; }

    /// <summary>
    /// The original file name, if known. Used only as a tie-breaking hint during
    /// format identification and to populate diagnostics.
    /// </summary>
    public string? FileName { get; }

    /// <summary>
    /// The format to read as. When left as <see cref="DocumentFormat.Unknown"/>,
    /// the reader sniffs the content, which is the recommended behaviour.
    /// </summary>
    public DocumentFormat Format { get; init; }

    /// <summary>Password for encrypted documents, if the caller has one.</summary>
    public string? Password { get; init; }

    /// <summary>
    /// The encoding to assume for byte-oriented formats (legacy binary formats and
    /// CSV) when the file does not say. When null, the reader guesses from the
    /// content and from any codepage recorded in the file.
    /// </summary>
    public System.Text.Encoding? FallbackEncoding { get; init; }

    /// <summary>Opens a file on disk.</summary>
    public static DocumentSource FromFile(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        FileStream stream = new(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        return new DocumentSource(stream, ownsStream: true, Path.GetFileName(path));
    }

    /// <summary>Reads from an in-memory buffer.</summary>
    public static DocumentSource FromBytes(ReadOnlyMemory<byte> bytes, string? fileName = null)
        => new(new MemoryStream(bytes.ToArray(), writable: false), ownsStream: true, fileName);

    /// <summary>
    /// Reads from a stream, buffering it into memory first if it is not seekable.
    /// </summary>
    /// <param name="stream">The stream to read.</param>
    /// <param name="fileName">An optional file name hint.</param>
    /// <param name="leaveOpen">
    /// When true, disposing this source does not dispose <paramref name="stream"/>.
    /// Ignored when the stream had to be buffered.
    /// </param>
    public static DocumentSource FromStream(Stream stream, string? fileName = null, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (stream.CanSeek) return new DocumentSource(stream, ownsStream: !leaveOpen, fileName);

        MemoryStream buffered = new();
        stream.CopyTo(buffered);
        buffered.Position = 0;
        return new DocumentSource(buffered, ownsStream: true, fileName);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_ownsStream) Stream.Dispose();
    }
}
