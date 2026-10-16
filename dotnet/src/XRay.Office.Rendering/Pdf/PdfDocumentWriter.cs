using System.Globalization;
using System.IO.Compression;
using System.Text;

namespace XRay.Office.Rendering.Pdf;

/// <summary>
/// The object, cross-reference and trailer layer of a PDF file.
/// </summary>
/// <remarks>
/// <para>
/// Objects are reserved first and filled in later, because a PDF is a graph and almost
/// every dictionary names an object that does not exist yet: a page names its content
/// stream and its resources, and the resources name fonts that are only discovered while
/// the content is being written. Reserving an object number up front and writing its body
/// afterwards is what lets the whole document be produced in one pass over the pages.
/// </para>
/// <para>
/// <b>Objects go straight to the destination stream</b> the moment they are given a body, so a
/// document is never held in memory as a whole: what is resident is the offset table (eight bytes an
/// object) and whichever single object is being written. A classic cross-reference table can still
/// be written at the end because it only needs each object's offset, and a PDF's objects may appear in
/// any order — a reserved number that is filled in last (the page tree, the resource dictionary,
/// whose fonts are only known once every page has been drawn) simply lands late in the file. The
/// consequence is that an object can be given a body exactly once, and the destination is written
/// to only forwards, so it need not be seekable.
/// </para>
/// <para>
/// A classic <c>xref</c> table rather than a cross-reference stream, and no object
/// streams. Both of the compact forms would hide the objects from a reader that scans for
/// them, and the fidelity harness in <c>tests/XRay.Office.TestKit/LibreOffice/</c> is
/// exactly such a reader — it is pointed at our output as well as at LibreOffice's, so
/// writing what LibreOffice writes is what makes the two comparable.
/// </para>
/// </remarks>
internal sealed class PdfDocumentWriter
{
    /// <summary>
    /// The zlib compression level used for every compressed stream.
    /// </summary>
    /// <remarks>
    /// Fixed rather than chosen per stream so that the same page always produces the same
    /// bytes. Deterministic output is what makes a written PDF checksummable in a test.
    /// </remarks>
    private const CompressionLevel Compression = CompressionLevel.Optimal;

    private const long Unwritten = -1;

    private readonly Stream _destination;
    private readonly List<long> _offsets = [];
    private long _position;
    private bool _finished;

    /// <summary>Starts a file on a stream; the header is written at once.</summary>
    public PdfDocumentWriter(Stream destination)
    {
        ArgumentNullException.ThrowIfNull(destination);
        _destination = destination;

        // The binary comment on the second line is what tells a transfer that treats the file as
        // text to stop doing so; every producer writes one and its absence corrupts files in transit.
        Write([.. "%PDF-1.7\n"u8, .. "%âãÏÓ\n"u8]);
    }

    /// <summary>How many objects have been reserved.</summary>
    public int ObjectCount => _offsets.Count;

    /// <summary>Reserves an object number whose body is supplied later.</summary>
    public int Reserve()
    {
        _offsets.Add(Unwritten);
        return _offsets.Count;
    }

    /// <summary>Reserves an object number and gives it a body at once.</summary>
    public int Add(string body)
    {
        int id = Reserve();
        Set(id, body);
        return id;
    }

    /// <summary>Fills in a reserved object with a dictionary or other direct body.</summary>
    public void Set(int id, string body)
    {
        ArgumentNullException.ThrowIfNull(body);

        Begin(id);
        Write(Encoding.Latin1.GetBytes(body));
        Write("\nendobj\n"u8);
    }

    /// <summary>
    /// Fills in a reserved object with a stream.
    /// </summary>
    /// <param name="id">The reserved object number.</param>
    /// <param name="entries">
    /// Extra dictionary entries, already spelled — <c>/Subtype/Image</c> and the like.
    /// <c>/Length</c> and <c>/Filter</c> are added here.
    /// </param>
    /// <param name="data">The stream's payload, uncompressed.</param>
    /// <param name="compress">
    /// Whether to deflate it. Content streams are compressed; font programs are not, and
    /// that is not a size decision — see <see cref="PdfRenderer"/>.
    /// </param>
    public void SetStream(int id, string entries, ReadOnlySpan<byte> data, bool compress)
    {
        ArgumentNullException.ThrowIfNull(entries);

        byte[]? deflated = compress ? Deflate(data) : null;
        int length = deflated?.Length ?? data.Length;
        string filter = compress ? "/Filter/FlateDecode" : string.Empty;

        Begin(id);
        Write(Encoding.Latin1.GetBytes(
            string.Create(
                CultureInfo.InvariantCulture,
                $"<<{entries}/Length {length}{filter}>>\nstream\n")));
        Write(deflated is null ? data : deflated);
        Write("\nendstream\nendobj\n"u8);
    }

    /// <summary>
    /// Writes the cross-reference table and trailer, which ends the file.
    /// </summary>
    /// <param name="catalogue">The object number of the document catalogue.</param>
    /// <param name="info">The object number of the document information dictionary.</param>
    /// <param name="fileId">The two halves of the trailer's <c>/ID</c>, or null to omit it.</param>
    public void Finish(int catalogue, int info, byte[]? fileId = null)
    {
        if (_finished) throw new InvalidOperationException("The file has already been finished.");
        _finished = true;

        // A number that was reserved and never filled still has to point somewhere.
        for (int i = 0; i < _offsets.Count; i++)
        {
            if (_offsets[i] == Unwritten) Set(i + 1, "null");
        }

        long startxref = _position;
        StringBuilder xref = new();
        xref.Append(CultureInfo.InvariantCulture, $"xref\n0 {_offsets.Count + 1}\n");
        xref.Append("0000000000 65535 f \n");
        foreach (long offset in _offsets)
        {
            xref.Append(offset.ToString("D10", CultureInfo.InvariantCulture)).Append(" 00000 n \n");
        }

        string identifier = fileId is null
            ? string.Empty
            : $"/ID[{PdfSyntax.HexString(fileId)}{PdfSyntax.HexString(fileId)}]";

        xref.Append(CultureInfo.InvariantCulture,
            $"trailer\n<</Size {_offsets.Count + 1}/Root {catalogue} 0 R/Info {info} 0 R{identifier}>>\n");
        xref.Append(CultureInfo.InvariantCulture, $"startxref\n{startxref}\n%%EOF\n");

        Write(Encoding.Latin1.GetBytes(xref.ToString()));
        _destination.Flush();
    }

    private void Begin(int id)
    {
        if (_finished) throw new InvalidOperationException("The file has already been finished.");
        if (id < 1 || id > _offsets.Count) throw new ArgumentOutOfRangeException(nameof(id));
        if (_offsets[id - 1] != Unwritten)
            throw new InvalidOperationException($"Object {id} has already been written.");

        _offsets[id - 1] = _position;
        Write(Encoding.Latin1.GetBytes(string.Create(CultureInfo.InvariantCulture, $"{id} 0 obj\n")));
    }

    private void Write(ReadOnlySpan<byte> bytes)
    {
        _destination.Write(bytes);
        _position += bytes.Length;
    }

    private static byte[] Deflate(ReadOnlySpan<byte> data)
    {
        using MemoryStream buffer = new();
        using (ZLibStream deflater = new(buffer, Compression, leaveOpen: true))
        {
            deflater.Write(data);
        }

        return buffer.ToArray();
    }
}
