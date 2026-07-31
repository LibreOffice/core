using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.PropertySets;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Reads legacy binary PowerPoint presentations: <c>ppt</c>, the <c>pot</c> template and the
/// <c>pps</c> slide show, as written by PowerPoint 97 through 2003 and by LibreOffice.
/// </summary>
/// <remarks>
/// <para>
/// The file is an OLE2 compound file whose <c>PowerPoint Document</c> stream holds a tree of
/// eight-byte-headed records. What makes it unlike the other legacy formats is that the tree
/// cannot simply be walked: saving appends changed objects and orphans their predecessors, so
/// the stream holds several versions of the same slide and nothing in the record tree says
/// which is current. Only the persist directory does — see
/// <see cref="PptPersistDirectory"/>, which is read first and is the reason every offset below
/// comes from it rather than from a scan.
/// </para>
/// <para>
/// Slide text lives in two different places. Ordinary shapes carry it in their Escher client
/// textbox; placeholders on a slide made from an autolayout often carry only an
/// <c>OutlineTextRefAtom</c> naming the <em>n</em>th text run of the slide's own entry in the
/// document's slide list, where the characters actually are. A reader that handles only the
/// first case loses every title and body of a PowerPoint-authored deck while reading a
/// LibreOffice-authored one perfectly, because the two writers choose differently.
/// </para>
/// </remarks>
public static class PptReader
{
    /// <summary>The stream holding the record tree.</summary>
    public const string DocumentStreamName = "PowerPoint Document";

    /// <summary>The stream naming the newest edit session.</summary>
    public const string CurrentUserStreamName = "Current User";

    /// <summary>
    /// Reads a presentation, leaving the source's stream for the caller to dispose.
    /// </summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    /// <exception cref="MalformedDocumentException">
    /// The file has no <c>PowerPoint Document</c> stream, so it is not a binary PowerPoint file.
    /// </exception>
    public static PptDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        CompoundFile file = CompoundFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];

            byte[] bytes = ReadStream(file, DocumentStreamName)
                ?? throw new MalformedDocumentException(
                    "The compound file has no 'PowerPoint Document' stream, so it is not a "
                    + "binary PowerPoint presentation.");

            DffRecordBuffer stream = new(bytes);
            uint currentUserEdit = ReadCurrentUserEdit(file);
            PptPersistDirectory persist = PptPersistDirectory.Read(stream, currentUserEdit, diagnostics);

            ContentDocument content = new()
            {
                Metadata = OlePropertySetReader.Read(file),
            };

            new PptContentBuilder(stream, persist, diagnostics).Build(content);
            return new PptDocument(format, file, content, diagnostics);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// The offset of the newest <c>UserEditAtom</c>, as the <c>Current User</c> stream states it.
    /// </summary>
    /// <remarks>
    /// Zero when the stream is missing, does not begin with a <c>CurrentUserAtom</c>, or is too
    /// short — all of which happen, and none of which is fatal, because the document stream can
    /// be searched for the last edit atom instead.
    /// </remarks>
    private static uint ReadCurrentUserEdit(CompoundFile file)
    {
        byte[]? bytes = ReadStream(file, CurrentUserStreamName);
        if (bytes is null) return 0;

        DffRecordBuffer buffer = new(bytes);
        if (!buffer.TryReadHeader(0, out DffRecordHeader header)) return 0;
        if (header.Type != PptRecordTypes.CurrentUserAtom) return 0;

        // Four bytes of a size that counts only the fixed part, four of a magic number that
        // distinguishes an encrypted file, then the offset itself.
        ReadOnlySpan<byte> content = buffer.Content(header);
        return content.Length >= 12 ? DffRecordBuffer.ReadUInt32(content[8..]) : 0;
    }

    /// <summary>The whole of one stream, or null when the compound file has no such stream.</summary>
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

/// <summary>A legacy binary PowerPoint presentation that has been read.</summary>
public sealed class PptDocument : IDocument
{
    private readonly CompoundFile _file;

    internal PptDocument(
        DocumentFormat format,
        CompoundFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.Presentation;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <summary>
    /// The underlying compound file, for callers needing streams the content tree does not
    /// expose — the picture store above all. Valid until this document is disposed.
    /// </summary>
    public CompoundFile File => _file;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
