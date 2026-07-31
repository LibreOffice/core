using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One straight stroked line as a PDF positions it.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="FromX">Where it starts, in points from the page's left.</param>
/// <param name="FromY">
/// Where it starts, in points from the page's <em>top</em> — converted from PDF space so that it compares
/// directly with a layout engine's coordinates.
/// </param>
/// <param name="ToX">Where it ends horizontally.</param>
/// <param name="ToY">Where it ends vertically, again from the page's top.</param>
/// <param name="Width">The pen width in points.</param>
public readonly record struct PdfStroke(
    int PageIndex, double FromX, double FromY, double ToX, double ToY, double Width)
{
    /// <summary>True when the line is horizontal, within a rounding of a twentieth of a point.</summary>
    public bool IsHorizontal => Math.Abs(FromY - ToY) < 0.05;

    /// <summary>True when it is vertical.</summary>
    public bool IsVertical => Math.Abs(FromX - ToX) < 0.05;

    /// <summary>How long it is along its own axis.</summary>
    public double Length => IsHorizontal ? Math.Abs(ToX - FromX) : Math.Abs(ToY - FromY);
}

/// <summary>
/// Reads the stroked straight lines out of a PDF LibreOffice wrote.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <see cref="PdfFills"/>, and it exists for the one thing LibreOffice strokes rather than
/// fills: a table's cell borders. It writes them <em>consolidated</em> — one stroke per grid line across the
/// whole table rather than four round each cell — so a comparison has to see the same shapes, and until
/// something could read them there was no way to check that at all.
/// </para>
/// <para>
/// Straight two-point lines only, which is what a border is: <c>x y m x y l S</c> with a pen width from the
/// nearest preceding <c>w</c>. A longer path or a curve is not one of the things this can check.
/// </para>
/// </remarks>
public static partial class PdfStrokes
{
    /// <summary>Reads every stroked straight line in a PDF, in the order it was drawn.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    public static List<PdfStroke> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        byte[] bytes = File.ReadAllBytes(pdfPath);
        double pageHeight = PageHeight(bytes);

        List<PdfStroke> strokes = [];
        int page = 0;

        foreach (string content in ContentStreams(bytes))
        {
            strokes.AddRange(LinesIn(content, page, pageHeight));
            page++;
        }

        return strokes;
    }

    /// <summary>
    /// The straight lines one content stream strokes.
    /// </summary>
    /// <remarks>
    /// The pen width comes from the last <c>w</c> before the line rather than from the line itself, because
    /// that is how PDF states it — LibreOffice writes <c>q 0.5 w 0 J 1 j</c> and then the two points, so a
    /// reader looking only at the <c>m</c>/<c>l</c> pair gets the geometry and misses the thickness, which for
    /// a border is half of what is being checked.
    /// </remarks>
    private static IEnumerable<PdfStroke> LinesIn(string content, int page, double pageHeight)
    {
        foreach (Match match in StrokedLine().Matches(content))
        {
            double width = double.TryParse(
                match.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                out double stated)
                ? stated
                : 0;

            if (Numbers(match, 2, 4) is not { } points) continue;

            yield return new PdfStroke(
                page,
                points[0], pageHeight - points[1],
                points[2], pageHeight - points[3],
                width);
        }
    }

    private static double[]? Numbers(Match match, int firstGroup, int count)
    {
        double[] values = new double[count];

        for (int i = 0; i < count; i++)
        {
            if (!double.TryParse(
                    match.Groups[firstGroup + i].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out values[i]))
            {
                return null;
            }
        }

        return values;
    }

    /// <summary>A pen width, then a two-point path, then a stroke.</summary>
    /// <remarks>
    /// The width and the points are matched together rather than tracked as state, which works because
    /// LibreOffice writes them together — one <c>q … w … m … l S Q</c> group per border. Anything that set the
    /// width further away is simply not matched, which is the honest failure for a test harness.
    /// </remarks>
    [GeneratedRegex(
        @"([\d.]+)\s+w\b[^mS]*?(-?[\d.]+)\s+(-?[\d.]+)\s+m\s+(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+S\b",
        RegexOptions.Singleline)]
    private static partial Regex StrokedLine();

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
                // Not a Flate stream — a font file or an image, neither of which strokes anything.
            }
        }

        return streams;
    }

    [GeneratedRegex(@"stream\r?\n")]
    private static partial Regex StreamStart();
}
