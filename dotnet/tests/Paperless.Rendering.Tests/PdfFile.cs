using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.Rendering.Tests;

/// <summary>
/// Reads back a PDF Paperless wrote, far enough to assert things about it.
/// </summary>
/// <remarks>
/// <para>
/// Deliberately a reader of <em>our own</em> output rather than a general PDF parser, for
/// the same reason the fidelity harness's readers are deliberately not one: what is being
/// checked is the file we produce, and a general parser would be a large dependency to
/// verify something already verified upstream. It knows the two shapes this writer emits —
/// a direct object and a stream object — and nothing else.
/// </para>
/// <para>
/// It is also the reason the tests can check font subsetting at all. The embedded font
/// programs are written uncompressed, so they come straight back out of the file and can be
/// taken apart as sfnt tables.
/// </para>
/// </remarks>
internal sealed partial class PdfFile
{
    private readonly byte[] _bytes;
    private readonly string _text;

    private PdfFile(byte[] bytes)
    {
        _bytes = bytes;
        _text = Encoding.Latin1.GetString(bytes);
    }

    /// <summary>Reads a PDF from bytes.</summary>
    public static PdfFile Parse(byte[] bytes) => new(bytes);

    /// <summary>The whole file as Latin-1 text, for asserting on its syntax.</summary>
    public string Text => _text;

    /// <summary>
    /// Every object in the file, in order, split into its dictionary and its payload.
    /// </summary>
    /// <remarks>
    /// Object by object rather than by scanning for the <c>stream</c> keyword, which is what
    /// the fidelity harness's readers do. They can: they only need the content streams and a
    /// stream that does not inflate is not one. A test that asks about a font program or an
    /// image needs to know which object it came from, so this walks the bodies properly.
    /// </remarks>
    private List<(int Id, string Dictionary, byte[]? Data, bool Deflated)> Objects()
    {
        List<(int, string, byte[]?, bool)> objects = [];

        foreach (Match match in ObjectStart().Matches(_text))
        {
            int id = int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
            int start = match.Index + match.Length;
            int end = _text.IndexOf("\nendobj", start, StringComparison.Ordinal);
            if (end < 0) continue;

            int stream = _text.IndexOf("\nstream\n", start, StringComparison.Ordinal);
            if (stream < 0 || stream > end)
            {
                objects.Add((id, _text[start..end].Trim(), null, false));
                continue;
            }

            string dictionary = _text[start..stream].Trim();
            int payload = stream + "\nstream\n".Length;
            int finish = _text.LastIndexOf("\nendstream", end, StringComparison.Ordinal);
            if (finish < payload) continue;

            byte[] raw = _bytes[payload..finish];
            bool deflated = dictionary.Contains("/FlateDecode", StringComparison.Ordinal);

            if (!deflated)
            {
                objects.Add((id, dictionary, raw, false));
                continue;
            }

            using MemoryStream compressed = new(raw);
            using ZLibStream inflater = new(compressed, CompressionMode.Decompress);
            using MemoryStream plain = new();
            inflater.CopyTo(plain);
            objects.Add((id, dictionary, plain.ToArray(), true));
        }

        return objects;
    }

    /// <summary>The bodies of the direct (non-stream) objects, by object number.</summary>
    public Dictionary<int, string> Dictionaries()
        => Objects().Where(o => o.Data is null).ToDictionary(o => o.Id, o => o.Dictionary);

    /// <summary>Every stream in the file, with its dictionary and payload.</summary>
    public List<(string Dictionary, byte[] Data, bool Deflated)> Streams()
        => [.. Objects().Where(o => o.Data is not null)
            .Select(o => (o.Dictionary, o.Data!, o.Deflated))];

    /// <summary>
    /// The page content streams, in file order.
    /// </summary>
    /// <remarks>
    /// A page's content is the one stream that states nothing about itself but its length:
    /// a font program carries <c>/Length1</c>, an image and a form carry a <c>/Subtype</c>,
    /// a <c>ToUnicode</c> CMap is not deflated at all, and a triangle-mesh shading — the one
    /// stream that is neither text nor a picture — names its <c>/ShadingType</c>.
    /// </remarks>
    public List<string> ContentStreams()
        => [.. Streams()
            .Where(s => s.Deflated
                        && !s.Dictionary.Contains("/Length1", StringComparison.Ordinal)
                        && !s.Dictionary.Contains("/Subtype", StringComparison.Ordinal)
                        && !s.Dictionary.Contains("/ShadingType", StringComparison.Ordinal))
            .Select(s => Encoding.Latin1.GetString(s.Data))];

    /// <summary>The embedded font programs, in file order.</summary>
    public List<byte[]> FontPrograms()
        // /Length1 marks a TrueType program and /Subtype/OpenType a CFF-flavoured one; a font
        // stream carries exactly one of the two, and looking only for the first misses every
        // .otf the machine has.
        => [.. Streams()
            .Where(s => s.Dictionary.Contains("/Length1", StringComparison.Ordinal)
                        || s.Dictionary.Contains("/Subtype/OpenType", StringComparison.Ordinal))
            .Select(s => s.Data)];

    /// <summary>Every <c>/MediaBox</c> in the file, in order.</summary>
    public List<(double Width, double Height)> PageSizes()
    {
        List<(double, double)> sizes = [];

        foreach (Match match in MediaBox().Matches(_text))
        {
            sizes.Add((
                double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture),
                double.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture)));
        }

        return sizes;
    }

    /// <summary>
    /// The <c>ToUnicode</c> mapping of one PDF font, as code to text.
    /// </summary>
    /// <param name="resource">The resource name, such as <c>F1</c>.</param>
    public Dictionary<byte, string> ToUnicode(string resource)
    {
        Dictionary<int, string> objects = Dictionaries();

        // "/Type/Font/" with the trailing slash, because "/Type/FontDescriptor" starts with the
        // same characters and is written first — a substring test picks the descriptor, which
        // names no ToUnicode, and the whole mapping comes back empty.
        int fontId = objects
            .Where(o => o.Value.Contains("/Type/Font/", StringComparison.Ordinal))
            .Select(o => o.Key)
            .ElementAtOrDefault(FontIndex(resource));

        if (fontId == 0) return [];

        Match reference = ToUnicodeReference().Match(objects[fontId]);
        if (!reference.Success) return [];

        int cmapId = int.Parse(reference.Groups[1].Value, CultureInfo.InvariantCulture);
        Match body = Regex.Match(
            _text,
            $@"(?<![0-9]){cmapId} 0 obj\n<<[^>]*>>\nstream\n(.*?)\nendstream",
            RegexOptions.Singleline);

        Dictionary<byte, string> map = [];
        if (!body.Success) return map;

        foreach (Match entry in BfChar().Matches(body.Groups[1].Value))
        {
            byte code = byte.Parse(entry.Groups[1].Value, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
            string hex = entry.Groups[2].Value;

            byte[] utf16 = new byte[hex.Length / 2];
            for (int i = 0; i < utf16.Length; i++)
            {
                utf16[i] = byte.Parse(
                    hex.AsSpan(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
            }

            map[code] = Encoding.BigEndianUnicode.GetString(utf16);
        }

        return map;
    }

    /// <summary>How many PDF fonts the file's resource dictionary names.</summary>
    public int FontCount()
    {
        Match resource = FontResource().Match(_text);
        return resource.Success ? FontEntry().Count(resource.Value) : 0;
    }

    private static int FontIndex(string resource)
        => int.Parse(resource.AsSpan(1), CultureInfo.InvariantCulture) - 1;

    [GeneratedRegex(@"(?<![0-9])(\d+) 0 obj\n")]
    private static partial Regex ObjectStart();

    [GeneratedRegex(@"/MediaBox\[0 0 ([-0-9.]+) ([-0-9.]+)\]")]
    private static partial Regex MediaBox();

    [GeneratedRegex(@"/ToUnicode (\d+) 0 R")]
    private static partial Regex ToUnicodeReference();

    [GeneratedRegex(@"<([0-9A-F]{2})> <([0-9A-F]+)>")]
    private static partial Regex BfChar();

    [GeneratedRegex(@"/Font<<[^>]*>>")]
    private static partial Regex FontResource();

    [GeneratedRegex(@"/F\d+ \d+ 0 R")]
    private static partial Regex FontEntry();
}
