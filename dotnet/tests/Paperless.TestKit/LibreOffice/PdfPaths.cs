using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One filled path as a PDF draws it.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="Points">
/// Its <em>on-curve</em> points, in order and in points from the page's top left: the destination
/// of every <c>m</c>, <c>l</c> and <c>c</c>. A curve's control points are left out deliberately —
/// see <see cref="PdfPaths"/>.
/// </param>
/// <param name="Curves">How many of the segments were curves.</param>
/// <param name="Colour">The non-stroking colour it was filled with, as <c>0xRRGGBB</c>.</param>
public readonly record struct PdfPath(
    int PageIndex, IReadOnlyList<(double X, double Y)> Points, int Curves, uint Colour)
{
    /// <summary>Its bounding box, in points from the page's top left.</summary>
    public (double Left, double Top, double Right, double Bottom) Bounds => Points.Count == 0
        ? (0, 0, 0, 0)
        : (Points.Min(p => p.X), Points.Min(p => p.Y),
           Points.Max(p => p.X), Points.Max(p => p.Y));
}

/// <summary>
/// Reads the filled paths out of a PDF, whatever shape they are.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="PdfFills"/> reads only axis-parallel rectangles, which is everything a word
/// processor draws and almost nothing a slide does: a preset shape is a hexagon, a star or an
/// arc. Checking that a preset evaluator produces the right polygon needs the polygon, so this
/// reads one.
/// </para>
/// <para>
/// <strong>On-curve points only, and that is the whole design.</strong> Both writers approximate
/// an arc with cubics and they do not choose the same number of them — LibreOffice emits three
/// per quarter turn where Paperless emits one — so the control points are not comparable and the
/// segment counts are not either. The points a curve <em>passes through</em> are, and for a
/// polygon they are the vertices outright. So a straight-edged preset compares vertex for vertex
/// and a curved one compares by its bounding box and its own on-curve points.
/// </para>
/// <para>
/// Deliberately not a PDF parser, for the same reason its two siblings are not: it reads the
/// shape both writers actually emit and would need real object parsing for anything else.
/// </para>
/// </remarks>
public static partial class PdfPaths
{
    /// <summary>Reads every filled path in a PDF, in the order it was painted.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    public static List<PdfPath> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        byte[] bytes = File.ReadAllBytes(pdfPath);
        double pageHeight = PageHeight(bytes);

        List<PdfPath> paths = [];
        int page = 0;

        foreach (string content in ContentStreams(bytes))
        {
            paths.AddRange(PathsIn(content, page, pageHeight));
            page++;
        }

        return paths;
    }

    private static IEnumerable<PdfPath> PathsIn(string content, int page, double pageHeight)
    {
        List<(int At, uint Colour)> colours = [.. Colours(content)];
        int next = 0;
        uint current = 0;

        foreach (Match match in FilledPath().Matches(content))
        {
            while (next < colours.Count && colours[next].At < match.Index)
            {
                current = colours[next++].Colour;
            }

            List<(double X, double Y)> points = [];
            int curves = 0;
            int subpath = 0;

            foreach (Match segment in Segment().Matches(match.Value))
            {
                double[] numbers =
                [
                    .. segment.Groups[1].Captures.Select(
                        capture => double.TryParse(
                            capture.Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                            out double value)
                            ? value
                            : 0.0),
                ];

                if (numbers.Length < 2) continue;
                if (segment.Groups[2].Value == "c") curves++;

                // The destination is the last pair, whatever the operator: `c` states two control
                // points first, and neither writer's choice of them is comparable.
                (double X, double Y) point = (numbers[^2], pageHeight - numbers[^1]);

                if (segment.Groups[2].Value == "m")
                {
                    Close(points, subpath);
                    subpath = points.Count;
                }

                points.Add(point);
            }

            Close(points, subpath);

            if (points.Count >= 3) yield return new PdfPath(page, points, curves, current);
        }
    }

    /// <summary>
    /// Drops a subpath's last point when it repeats its first.
    /// </summary>
    /// <remarks>
    /// LibreOffice writes a closed polygon's start point twice — once to begin and once before
    /// the <c>h</c> — and Paperless writes it once. Both draw the same hexagon; without this a
    /// vertex-for-vertex comparison reports seven against six and says nothing about the shape.
    /// </remarks>
    private static void Close(List<(double X, double Y)> points, int start)
    {
        if (points.Count - start < 2) return;

        const double Tolerance = 0.001;
        if (Math.Abs(points[^1].X - points[start].X) < Tolerance
            && Math.Abs(points[^1].Y - points[start].Y) < Tolerance)
        {
            points.RemoveAt(points.Count - 1);
        }
    }

    /// <summary>A move, then any run of line, curve, close and further move operators, then a fill.</summary>
    /// <remarks>
    /// Subpaths are joined rather than split, because a preset with a hole — <c>donut</c>,
    /// <c>frame</c> — is one path with two subpaths in both writers, and reading them as two
    /// would report twice as many shapes as either drew.
    /// </remarks>
    [GeneratedRegex(
        @"(-?[\d.]+\s+-?[\d.]+\s+m\s+(?:(?:-?[\d.]+\s+)*[lchm]\s+)+)[fFB]\*?(?![A-Za-z])",
        RegexOptions.Singleline)]
    private static partial Regex FilledPath();

    /// <summary>Two, four or six numbers followed by a path operator.</summary>
    [GeneratedRegex(@"(?:(-?[\d.]+)\s+){2,6}([mlc])(?![A-Za-z])")]
    private static partial Regex Segment();

    private static IEnumerable<(int At, uint Colour)> Colours(string content)
    {
        foreach (Match match in NonStrokingColour().Matches(content))
        {
            double[] values =
            [
                .. match.Groups[1].Captures.Select(
                    capture => double.TryParse(
                        capture.Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                        out double v)
                        ? v
                        : 0.0),
            ];

            uint colour = match.Groups[2].Value switch
            {
                "g" when values.Length == 1 => Pack(values[0], values[0], values[0]),
                "rg" when values.Length == 3 => Pack(values[0], values[1], values[2]),
                "k" when values.Length == 4 => Pack(
                    (1 - values[0]) * (1 - values[3]),
                    (1 - values[1]) * (1 - values[3]),
                    (1 - values[2]) * (1 - values[3])),
                _ => 0,
            };

            yield return (match.Index, colour);
        }
    }

    private static uint Pack(double r, double g, double b)
        => ((uint)Math.Clamp(Math.Round(r * 255), 0, 255) << 16)
           | ((uint)Math.Clamp(Math.Round(g * 255), 0, 255) << 8)
           | (uint)Math.Clamp(Math.Round(b * 255), 0, 255);

    [GeneratedRegex(@"(?:(-?[\d.]+)\s+){1,4}(rg|g|k)(?![A-Za-z])")]
    private static partial Regex NonStrokingColour();

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
                // A page's content stream, told apart from a CMap or a font program by the
                // operators it contains rather than by its text: a slide of nothing but shapes has
                // no BT at all, and testing for one alone drops that page and shifts every page
                // index after it. Both writers paint a page background, so " re" is on every page.
                if (content.Contains("BT", StringComparison.Ordinal)
                    || content.Contains(" re", StringComparison.Ordinal))
                {
                    streams.Add(content);
                }
            }
            catch (InvalidDataException)
            {
                // Not a Flate stream — a font file or an image, neither of which fills a path.
            }
        }

        return streams;
    }

    [GeneratedRegex(@"stream\r?\n")]
    private static partial Regex StreamStart();
}
