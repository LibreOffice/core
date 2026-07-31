using System.Globalization;
using System.IO.Compression;
using System.Text;

namespace Paperless.Rendering.Pdf;

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
/// The body is buffered in memory and written out at the end. That is a deliberate limit
/// rather than an oversight — see the open question in this library's
/// <c>TODO.md</c> — and it is what makes the classic cross-reference table cheap: every
/// object's offset is known before the first byte reaches the stream.
/// </para>
/// <para>
/// A classic <c>xref</c> table rather than a cross-reference stream, and no object
/// streams. Both of the compact forms would hide the objects from a reader that scans for
/// them, and the fidelity harness in <c>tests/Paperless.TestKit/LibreOffice/</c> is
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

    private readonly List<byte[]?> _objects = [];

    /// <summary>Reserves an object number whose body is supplied later.</summary>
    public int Reserve()
    {
        _objects.Add(null);
        return _objects.Count;
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
        _objects[id - 1] = Encoding.Latin1.GetBytes(body);
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

        byte[] payload = compress ? Deflate(data) : data.ToArray();
        string filter = compress ? "/Filter/FlateDecode" : string.Empty;
        byte[] header = Encoding.Latin1.GetBytes(
            string.Create(
                CultureInfo.InvariantCulture,
                $"<<{entries}/Length {payload.Length}{filter}>>\nstream\n"));
        byte[] trailer = Encoding.Latin1.GetBytes("\nendstream");

        byte[] body = new byte[header.Length + payload.Length + trailer.Length];
        header.CopyTo(body, 0);
        payload.CopyTo(body, header.Length);
        trailer.CopyTo(body, header.Length + payload.Length);

        _objects[id - 1] = body;
    }

    /// <summary>Writes the whole file, with its header, body, cross-reference and trailer.</summary>
    /// <param name="destination">Where to write.</param>
    /// <param name="catalogue">The object number of the document catalogue.</param>
    /// <param name="info">The object number of the document information dictionary.</param>
    public void Write(Stream destination, int catalogue, int info)
    {
        ArgumentNullException.ThrowIfNull(destination);

        long position = 0;
        long WriteBytes(byte[] bytes)
        {
            destination.Write(bytes, 0, bytes.Length);
            long at = position;
            position += bytes.Length;
            return at;
        }

        // The binary comment on the second line is what tells a transfer that treats the file as
        // text to stop doing so; every producer writes one and its absence corrupts files in transit.
        WriteBytes([.. "%PDF-1.7\n"u8, .. "%âãÏÓ\n"u8]);

        long[] offsets = new long[_objects.Count];
        for (int i = 0; i < _objects.Count; i++)
        {
            byte[] body = _objects[i] ?? Encoding.Latin1.GetBytes("null");
            offsets[i] = WriteBytes(Encoding.Latin1.GetBytes(
                string.Create(CultureInfo.InvariantCulture, $"{i + 1} 0 obj\n")));
            WriteBytes(body);
            WriteBytes(Encoding.Latin1.GetBytes("\nendobj\n"));
        }

        long startxref = position;
        StringBuilder xref = new();
        xref.Append(CultureInfo.InvariantCulture, $"xref\n0 {_objects.Count + 1}\n");
        xref.Append("0000000000 65535 f \n");
        foreach (long offset in offsets)
        {
            xref.Append(offset.ToString("D10", CultureInfo.InvariantCulture)).Append(" 00000 n \n");
        }

        xref.Append(CultureInfo.InvariantCulture,
            $"trailer\n<</Size {_objects.Count + 1}/Root {catalogue} 0 R/Info {info} 0 R>>\n");
        xref.Append(CultureInfo.InvariantCulture, $"startxref\n{startxref}\n%%EOF\n");

        WriteBytes(Encoding.Latin1.GetBytes(xref.ToString()));
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
