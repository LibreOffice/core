using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One filled rectangle as a PDF positions it.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="Left">Its left edge, in points from the page's left.</param>
/// <param name="Top">
/// Its top edge, in points from the page's <em>top</em> — converted from PDF space, whose origin is the
/// bottom left, so that it can be compared with a layout engine's coordinates directly.
/// </param>
/// <param name="Width">Its width in points.</param>
/// <param name="Height">Its height in points.</param>
public readonly record struct PdfFill(
    int PageIndex, double Left, double Top, double Width, double Height);

/// <summary>
/// Reads the filled rectangles out of a PDF LibreOffice wrote.
/// </summary>
/// <remarks>
/// <para>
/// Why this exists: everything a word processor draws that is not text is a filled shape, and none of it is
/// visible to <c>pdftotext</c>. A footnote separator, a cell border, a shaded table row — all of them are
/// paths, and until something could read them the only way to check any of it was to build the rasteriser
/// first and compare pixels. It turns out not to be necessary: LibreOffice's export writes each of them as an
/// explicit closed rectangle in the content stream, so the geometry is there to be compared at the same
/// resolution as everything else.
/// </para>
/// <para>
/// Rectangles only, and closed ones at that — five points where the last repeats the first, which is the shape
/// LibreOffice writes for a rule. A general path reader would be a much larger thing for no gain: what is
/// being checked is <em>where</em> a rectangle went, and a shape that is not a rectangle is not one of the
/// things this can check yet.
/// </para>
/// <para>
/// Deliberately not a PDF parser, for the same reason <see cref="PdfTextRuns"/> is not: it reads the shape
/// LibreOffice's own export writes and would need real object parsing for anything else.
/// </para>
/// </remarks>
public static partial class PdfFills
{
    /// <summary>Reads every filled rectangle in a PDF, in the order it was drawn.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    public static List<PdfFill> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        byte[] bytes = File.ReadAllBytes(pdfPath);
        double pageHeight = PageHeight(bytes);

        List<PdfFill> fills = [];
        int page = 0;

        foreach (string content in ContentStreams(bytes))
        {
            fills.AddRange(RectanglesIn(content, page, pageHeight));
            page++;
        }

        return fills;
    }

    /// <summary>
    /// The closed rectangles one content stream fills.
    /// </summary>
    /// <remarks>
    /// Two spellings, because PDF has two: <c>re</c> states a rectangle outright, and a run of
    /// <c>m</c>/<c>l</c> draws one as four lines — which is the one LibreOffice's own export uses for a rule.
    /// The painting operator is checked as well as the geometry, since a path that is only clipped or only
    /// stroked is not a fill: <c>f</c>, <c>F</c>, <c>f*</c>, <c>B</c> and <c>B*</c> all fill.
    /// </remarks>
    private static IEnumerable<PdfFill> RectanglesIn(string content, int page, double pageHeight)
    {
        foreach (Match match in ExplicitRectangle().Matches(content))
        {
            if (Numbers(match, 4) is not { } r) continue;

            yield return Fill(page, pageHeight, r[0], r[1], r[0] + r[2], r[1] + r[3]);
        }

        foreach (Match match in LineRectangle().Matches(content))
        {
            if (Numbers(match, 8) is not { } r) continue;

            double left = Math.Min(Math.Min(r[0], r[2]), Math.Min(r[4], r[6]));
            double right = Math.Max(Math.Max(r[0], r[2]), Math.Max(r[4], r[6]));
            double lower = Math.Min(Math.Min(r[1], r[3]), Math.Min(r[5], r[7]));
            double upper = Math.Max(Math.Max(r[1], r[3]), Math.Max(r[5], r[7]));

            yield return Fill(page, pageHeight, left, lower, right, upper);
        }
    }

    private static PdfFill Fill(
        int page, double pageHeight, double left, double lower, double right, double upper)
        => new(page, left, pageHeight - upper, right - left, upper - lower);

    private static double[]? Numbers(Match match, int count)
    {
        double[] values = new double[count];

        for (int i = 0; i < count; i++)
        {
            if (!double.TryParse(
                    match.Groups[i + 1].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out values[i]))
            {
                return null;
            }
        }

        return values;
    }

    /// <summary><c>x y w h re</c> followed by a filling operator.</summary>
    [GeneratedRegex(
        @"(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+re\s+(?:h\s+)?[fFB]\*?\b",
        RegexOptions.Singleline)]
    private static partial Regex ExplicitRectangle();

    /// <summary>A move and three lines closing back to the start, then a filling operator.</summary>
    [GeneratedRegex(
        @"(-?[\d.]+)\s+(-?[\d.]+)\s+m\s+(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+"
        + @"(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+-?[\d.]+\s+-?[\d.]+\s+l\s+h\s+[fFB]\*?\b",
        RegexOptions.Singleline)]
    private static partial Regex LineRectangle();

    /// <summary>The page height in points, for converting PDF space to top-down coordinates.</summary>
    private static double PageHeight(byte[] bytes)
    {
        Match box = MediaBox().Match(Encoding.Latin1.GetString(bytes));

        return box.Success
               && double.TryParse(
                   box.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                   out double height)
            ? height
            : 841.89;
    }

    [GeneratedRegex(@"/MediaBox\s*\[\s*[\d.-]+\s+[\d.-]+\s+[\d.-]+\s+([\d.-]+)\s*\]")]
    private static partial Regex MediaBox();

    /// <summary>Each page's decompressed content stream, in file order.</summary>
    private static List<string> ContentStreams(byte[] bytes)
    {
        List<string> streams = [];
        string text = Encoding.Latin1.GetString(bytes);

        foreach (Match match in StreamStart().Matches(text))
        {
            int start = match.Index + match.Length;
            int end = text.IndexOf("endstream", start, StringComparison.Ordinal);
            if (end < 0) continue;

            try
            {
                using MemoryStream compressed = new(bytes, start, end - start);
                using ZLibStream inflater = new(compressed, CompressionMode.Decompress);
                using MemoryStream plain = new();
                inflater.CopyTo(plain);

                string content = Encoding.Latin1.GetString(plain.ToArray());
                if (content.Contains("BT", StringComparison.Ordinal)) streams.Add(content);
            }
            catch (InvalidDataException)
            {
                // Not a Flate stream — a font file or an image. Skipped rather than reported: this reads the
                // shape LibreOffice's export writes and is not a general PDF reader.
            }
        }

        return streams;
    }

    [GeneratedRegex(@"stream\r?\n")]
    private static partial Regex StreamStart();
}
