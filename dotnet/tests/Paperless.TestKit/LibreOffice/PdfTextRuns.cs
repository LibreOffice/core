using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>
/// One stretch of text as a PDF positions it: the pen, the size, and how many glyphs were shown.
/// </summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="X">The pen's distance from the page's left edge, in points.</param>
/// <param name="Y">
/// The baseline's distance from the page's <em>top</em>, in points — converted from PDF space, whose
/// origin is the bottom left, so that it can be compared with a layout engine's coordinates directly.
/// </param>
/// <param name="FontSize">The em size it was shown at, in points.</param>
/// <param name="FontResource">The PDF resource name of the font, such as <c>F2</c>.</param>
/// <param name="GlyphCount">How many glyph codes were shown.</param>
public readonly record struct PdfTextRun(
    int PageIndex,
    double X,
    double Y,
    double FontSize,
    string FontResource,
    int GlyphCount);

/// <summary>
/// Reads the text-positioning operators out of a PDF LibreOffice wrote.
/// </summary>
/// <remarks>
/// <para>
/// Why bother, when <c>pdftotext -bbox</c> already reports word boxes: because a box is not a baseline
/// and a word is not a run. A box's top sits above the baseline by the font's ascent, which the PDF never
/// states, so a vertical comparison against boxes can only ever be relative. And poppler groups words
/// into lines by vertical position, which puts a 22 pt word on an 11 pt line into a line of its own and
/// scrambles the reading order of exactly the mixed-formatting documents this exists to check.
/// </para>
/// <para>
/// The content stream has neither problem. LibreOffice emits one <c>BT … ET</c> block per text portion —
/// per line, split again at every formatting change — in the order it drew them, each opening with an
/// absolute <c>Td</c>. That is the same unit a layout engine emits glyph runs in, so the comparison is
/// one run against one run with no grouping in between.
/// </para>
/// <para>
/// Deliberately not a PDF parser. It reads the shape LibreOffice's own export writes — Flate-compressed
/// content streams, one per page in file order, simple fonts with one-byte glyph codes — and would need
/// real object parsing for anything else. That is the right trade for a test harness: a general reader
/// would be a large dependency to verify one that is already verified upstream.
/// </para>
/// </remarks>
public static class PdfTextRuns
{
    /// <summary>Reads every positioned run in a PDF, in the order it was drawn.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    public static List<PdfTextRun> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        byte[] bytes = File.ReadAllBytes(pdfPath);
        double pageHeight = PageHeight(bytes);

        List<PdfTextRun> runs = [];
        int page = 0;

        foreach (string content in ContentStreams(bytes))
        {
            runs.AddRange(RunsIn(content, page, pageHeight));
            page++;
        }

        return runs;
    }

    /// <summary>How many pages had a content stream with text on it.</summary>
    public static int PageCount(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);
        return ContentStreams(File.ReadAllBytes(pdfPath)).Count;
    }

    /// <summary>
    /// The page height, from the first <c>/MediaBox</c> in the file.
    /// </summary>
    /// <remarks>
    /// The first rather than each page's own: PDF y grows upwards, so converting a baseline to a
    /// top-down coordinate needs the height of the page it is on. A document whose pages differ in size
    /// would need the boxes matched to the pages, which no corpus document does.
    /// </remarks>
    private static double PageHeight(byte[] bytes)
    {
        string text = Encoding.Latin1.GetString(bytes);
        Match box = Regex.Match(
            text, @"/MediaBox\s*\[\s*[-0-9.]+\s+[-0-9.]+\s+[-0-9.]+\s+([-0-9.]+)\s*\]");

        return box.Success ? Number(box.Groups[1].Value) : 0.0;
    }

    /// <summary>
    /// The decompressed content streams that draw text, in file order.
    /// </summary>
    /// <remarks>
    /// File order is page order in LibreOffice's output, which is what lets a page index be a counter
    /// rather than something resolved through the page tree. A stream that does not decompress is skipped
    /// rather than reported: a PDF holds fonts and images in streams too, and neither is text.
    /// </remarks>
    private static List<string> ContentStreams(byte[] bytes)
    {
        List<string> streams = [];
        string text = Encoding.Latin1.GetString(bytes);

        foreach (Match marker in Regex.Matches(text, @"stream\r?\n"))
        {
            int start = marker.Index + marker.Length;
            int end = text.IndexOf("endstream", start, StringComparison.Ordinal);
            if (end < 0) continue;

            if (Inflate(bytes, start, end - start) is not { } content) continue;
            if (!content.Contains("BT", StringComparison.Ordinal)) continue;

            streams.Add(content);
        }

        return streams;
    }

    private static string? Inflate(byte[] bytes, int offset, int length)
    {
        if (length <= 0) return null;

        try
        {
            using MemoryStream compressed = new(bytes, offset, length, writable: false);
            using ZLibStream inflater = new(compressed, CompressionMode.Decompress);
            using MemoryStream plain = new();
            inflater.CopyTo(plain);
            return Encoding.Latin1.GetString(plain.ToArray());
        }
        catch (InvalidDataException)
        {
            // Not a Flate stream, or not a stream at all: a false positive from scanning for the keyword.
            return null;
        }
    }

    /// <summary>
    /// The runs in one page's content stream.
    /// </summary>
    /// <remarks>
    /// <c>BT</c> resets the text matrix, so the first <c>Td</c> in a block is an absolute position — which
    /// is why this can read a pen position without tracking a matrix. The glyph count comes from the hex
    /// strings a <c>TJ</c> array or a <c>Tj</c> holds, two digits per glyph, ignoring the kerning numbers
    /// interleaved between them.
    /// </remarks>
    private static IEnumerable<PdfTextRun> RunsIn(string content, int page, double pageHeight)
    {
        foreach (Match block in Regex.Matches(
                     content, @"BT\s*(.*?)\s*ET", RegexOptions.Singleline))
        {
            string body = block.Groups[1].Value;

            Match position = Regex.Match(body, @"([-0-9.]+)\s+([-0-9.]+)\s+Td");
            Match font = Regex.Match(body, @"/([A-Za-z0-9]+)\s+([-0-9.]+)\s+Tf");
            if (!position.Success || !font.Success) continue;

            int glyphs = 0;
            foreach (Match shown in Regex.Matches(body, "<([0-9A-Fa-f]*)>"))
            {
                glyphs += shown.Groups[1].Value.Length / 2;
            }

            yield return new PdfTextRun(
                page,
                Number(position.Groups[1].Value),
                pageHeight - Number(position.Groups[2].Value),
                Number(font.Groups[2].Value),
                font.Groups[1].Value,
                glyphs);
        }
    }

    private static double Number(string value)
        => double.Parse(value, CultureInfo.InvariantCulture);
}
