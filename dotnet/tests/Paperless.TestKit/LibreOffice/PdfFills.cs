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
/// <param name="Colour">
/// The non-stroking colour in force when it was painted, as <c>0xRRGGBB</c>.
/// <para>
/// It is a property of the graphics state rather than of the path, so it is tracked across the
/// content stream rather than read out of the fill operator — a PDF writer sets a colour once
/// and paints several shapes with it, and LibreOffice's export omits the operator entirely when
/// the colour has not changed. Defaults to black, which is the PDF initial state.
/// </para>
/// </param>
public readonly record struct PdfFill(
    int PageIndex, double Left, double Top, double Width, double Height, uint Colour = 0);

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
    /// <c>m</c>/<c>l</c> draws one as a closed polygon — which is the one LibreOffice's own export uses,
    /// for a rule and for a rectangular shape alike. The number of segments is not fixed: a footnote rule
    /// comes out as four, while a <c>prstGeom prst="rect"</c> comes out as five, because the traversal
    /// starts at the middle of an edge and returns to it. So the polygon is read as points and accepted
    /// when every one of them lies on the boundary of their own bounding box, which is what makes it a
    /// rectangle however many times the path stops along the way.
    /// The painting operator is checked as well as the geometry, since a path that is only clipped or only
    /// stroked is not a fill: <c>f</c>, <c>F</c>, <c>f*</c>, <c>B</c> and <c>B*</c> all fill.
    /// </remarks>
    private static IEnumerable<PdfFill> RectanglesIn(string content, int page, double pageHeight)
    {
        List<(int At, PdfFill Fill)> found = [];

        foreach (Match match in ExplicitRectangle().Matches(content))
        {
            if (Numbers(match, 4) is not { } r) continue;

            found.Add((match.Index, Fill(page, pageHeight, r[0], r[1], r[0] + r[2], r[1] + r[3])));
        }

        foreach (Match match in ClosedPolygon().Matches(content))
        {
            if (Rectangle(match) is not { } bounds) continue;

            found.Add((match.Index,
                       Fill(page, pageHeight, bounds.Left, bounds.Lower, bounds.Right, bounds.Upper)));
        }

        found.Sort((a, b) => a.At.CompareTo(b.At));

        // One pass over the colour operators, merged with the fills by position, because the
        // colour belongs to the graphics state: the operator that set it may be thousands of
        // bytes earlier and may serve several fills.
        List<(int At, uint Colour)> colours = [.. Colours(content)];
        int next = 0;
        uint current = 0;

        foreach ((int at, PdfFill fill) in found)
        {
            while (next < colours.Count && colours[next].At < at) current = colours[next++].Colour;

            yield return fill with { Colour = current };
        }
    }

    /// <summary>
    /// Every change to the non-stroking colour, with where in the stream it happened.
    /// </summary>
    /// <remarks>
    /// Three operators, because PDF has three colour spaces in play and LibreOffice writes all
    /// of them: <c>rg</c> for RGB, <c>g</c> for greyscale — which is what it uses for pure black
    /// and pure white — and <c>k</c> for CMYK. Reading only <c>rg</c> leaves a black rule
    /// reported as whatever colour was set before it.
    /// </remarks>
    private static IEnumerable<(int At, uint Colour)> Colours(string content)
    {
        foreach (Match match in NonStrokingColour().Matches(content))
        {
            double[] values =
            [
                .. match.Groups[1].Captures
                    .Select(capture => double.TryParse(
                        capture.Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double v)
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

    /// <summary>One to four numbers followed by a lower-case colour operator.</summary>
    /// <remarks>
    /// Lower case only: <c>RG</c>, <c>G</c> and <c>K</c> set the <em>stroking</em> colour, which
    /// is the pen a border is drawn with rather than the paint a shape is filled with.
    /// </remarks>
    [GeneratedRegex(@"(?:(-?[\d.]+)\s+){1,4}(rg|g|k)(?![A-Za-z])")]
    private static partial Regex NonStrokingColour();

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

    /// <summary>A move, three or more lines, an explicit close, then a filling operator.</summary>
    [GeneratedRegex(
        @"(-?[\d.]+)\s+(-?[\d.]+)\s+m\s+(?:(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+){3,}h\s+[fFB]\*?\b",
        RegexOptions.Singleline)]
    private static partial Regex ClosedPolygon();

    /// <summary>
    /// The bounding box of a closed polygon, when the polygon is a rectangle.
    /// </summary>
    /// <remarks>
    /// "Is a rectangle" is tested as "every segment is axis-parallel", not as "every vertex is a
    /// corner of the bounding box" — the latter rejects the very shape this exists to read,
    /// since LibreOffice starts a rectangular shape's path at the <em>middle</em> of its bottom
    /// edge and comes back to it. A triangle fails the axis-parallel test on its hypotenuse and
    /// an L fails on none of its segments, so an L would be read as its bounding box; that is
    /// acceptable because nothing LibreOffice draws for a rule or a rectangle is one.
    /// </remarks>
    private static (double Left, double Right, double Lower, double Upper)? Rectangle(Match match)
    {
        List<double> xs = [];
        List<double> ys = [];

        if (!double.TryParse(
                match.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double x0)
            || !double.TryParse(
                match.Groups[2].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double y0))
        {
            return null;
        }

        xs.Add(x0);
        ys.Add(y0);

        for (int i = 0; i < match.Groups[3].Captures.Count; i++)
        {
            if (!double.TryParse(
                    match.Groups[3].Captures[i].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double x)
                || !double.TryParse(
                    match.Groups[4].Captures[i].Value, NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double y))
            {
                return null;
            }

            xs.Add(x);
            ys.Add(y);
        }

        const double Tolerance = 0.01;
        for (int i = 0; i < xs.Count; i++)
        {
            int next = (i + 1) % xs.Count;
            bool horizontal = Math.Abs(ys[i] - ys[next]) < Tolerance;
            bool vertical = Math.Abs(xs[i] - xs[next]) < Tolerance;
            if (!horizontal && !vertical) return null;
        }

        double left = xs.Min();
        double right = xs.Max();
        double lower = ys.Min();
        double upper = ys.Max();

        return right - left < Tolerance || upper - lower < Tolerance
            ? null
            : (left, right, lower, upper);
    }

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
                // Not a Flate stream — a font file or an image. Skipped rather than reported: this reads the
                // shape LibreOffice's export writes and is not a general PDF reader.
            }
        }

        return streams;
    }

    [GeneratedRegex(@"stream\r?\n")]
    private static partial Regex StreamStart();
}
