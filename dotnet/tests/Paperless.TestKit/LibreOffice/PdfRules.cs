using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One straight stroked line, with the pen width in force when it was drawn.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="FromX">Where it starts, in points from the page's left.</param>
/// <param name="FromY">Where it starts, in points from the page's <em>top</em>.</param>
/// <param name="ToX">Where it ends horizontally.</param>
/// <param name="ToY">Where it ends vertically, again from the page's top.</param>
/// <param name="Width">The pen width in points.</param>
/// <param name="Colour">The stroking colour, as <c>0xRRGGBB</c>.</param>
public readonly record struct PdfRule(
    int PageIndex, double FromX, double FromY, double ToX, double ToY, double Width, uint Colour)
{
    /// <summary>True when the line is horizontal, within a twentieth of a point.</summary>
    public bool IsHorizontal => Math.Abs(FromY - ToY) < 0.05;

    /// <summary>True when it is vertical.</summary>
    public bool IsVertical => Math.Abs(FromX - ToX) < 0.05;

    /// <summary>How long it is along its own axis.</summary>
    public double Length => IsHorizontal ? Math.Abs(ToX - FromX) : Math.Abs(ToY - FromY);
}

/// <summary>
/// Reads every stroked straight line in a PDF, tracking the pen as graphics state.
/// </summary>
/// <remarks>
/// <para>
/// The difference from <see cref="PdfStrokes"/>, and the reason both exist: that one pairs a
/// <c>w</c> with the line that immediately follows it, which is exactly the shape LibreOffice
/// writes for a <em>table border</em> — <c>q 0.5 w 0 J 1 j … m … l S Q</c> — and is deliberately
/// narrow so a stray line cannot be misread. A spreadsheet's grid is not written that way. Calc
/// draws it with <c>DrawLine</c> and no width at all, so the export emits the coordinates alone
/// and the pen comes from the stream's initial <c>0.1 w</c>, thousands of bytes earlier and with
/// several other strokes in between. Read by the stricter rule, LibreOffice's gridlines are
/// invisible; every one of the seventeen on <c>sheet-decor-ods.ods</c> is missed.
/// </para>
/// <para>
/// So this tracks the width, the stroking colour and the <c>q</c>/<c>Q</c> stack the way a PDF
/// consumer does, and reports every two-point <c>m … l S</c>. It sees a superset of what
/// <see cref="PdfStrokes"/> sees; it does not replace it, because the narrower reader is the
/// right tool where the pen really is adjacent.
/// </para>
/// <para>
/// Rectangles are reported too, as their four sides, because Calc draws a heading cell and the
/// frame round a printed block as <c>re S</c> rather than as four lines.
/// </para>
/// </remarks>
public static partial class PdfRules
{
    /// <summary>Reads every stroked straight line in a PDF, in the order it was drawn.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    public static List<PdfRule> Read(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        byte[] bytes = File.ReadAllBytes(pdfPath);
        double pageHeight = PageHeight(bytes);

        List<PdfRule> rules = [];
        int page = 0;

        foreach (string content in ContentStreams(bytes))
        {
            rules.AddRange(LinesIn(content, page, pageHeight));
            page++;
        }

        return rules;
    }

    /// <summary>The straight lines one content stream strokes, rectangles unpacked.</summary>
    private static IEnumerable<PdfRule> LinesIn(string content, int page, double pageHeight)
    {
        // The graphics state that matters here is two numbers and a stack. PDF's initial pen is
        // one unit wide, but LibreOffice sets 0.1 before anything else and never restores past
        // it, so starting at zero and letting the first `w` win is both correct and enough.
        double width = 0;
        uint colour = 0;
        Stack<(double Width, uint Colour)> saved = new();

        foreach (Match match in Operator().Matches(content))
        {
            if (match.Groups["q"].Success)
            {
                saved.Push((width, colour));
                continue;
            }

            if (match.Groups["Q"].Success)
            {
                if (saved.Count > 0) (width, colour) = saved.Pop();
                continue;
            }

            if (match.Groups["w"].Success)
            {
                width = Value(match.Groups["w"].Value);
                continue;
            }

            if (match.Groups["cop"].Success)
            {
                colour = Pack(match.Groups["cnum"].Captures, match.Groups["cop"].Value);
                continue;
            }

            if (match.Groups["lx1"].Success)
            {
                yield return new PdfRule(
                    page,
                    Value(match.Groups["lx1"].Value), pageHeight - Value(match.Groups["ly1"].Value),
                    Value(match.Groups["lx2"].Value), pageHeight - Value(match.Groups["ly2"].Value),
                    width, colour);
                continue;
            }

            if (!match.Groups["rx"].Success) continue;

            double x = Value(match.Groups["rx"].Value);
            double y = Value(match.Groups["ry"].Value);
            double w = Value(match.Groups["rw"].Value);
            double h = Value(match.Groups["rh"].Value);

            double top = pageHeight - (y + h);
            double bottom = pageHeight - y;

            yield return new PdfRule(page, x, top, x + w, top, width, colour);
            yield return new PdfRule(page, x + w, top, x + w, bottom, width, colour);
            yield return new PdfRule(page, x + w, bottom, x, bottom, width, colour);
            yield return new PdfRule(page, x, bottom, x, top, width, colour);
        }
    }

    private static double Value(string text)
        => double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : 0;

    private static uint Pack(CaptureCollection numbers, string op)
    {
        double[] values = [.. numbers.Select(capture => Value(capture.Value))];

        return op switch
        {
            "G" when values.Length >= 1 => Rgb(values[0], values[0], values[0]),
            "RG" when values.Length >= 3 => Rgb(values[0], values[1], values[2]),
            "K" when values.Length >= 4 => Rgb(
                (1 - values[0]) * (1 - values[3]),
                (1 - values[1]) * (1 - values[3]),
                (1 - values[2]) * (1 - values[3])),
            _ => 0,
        };
    }

    private static uint Rgb(double r, double g, double b)
        => ((uint)Math.Clamp(Math.Round(r * 255), 0, 255) << 16)
           | ((uint)Math.Clamp(Math.Round(g * 255), 0, 255) << 8)
           | (uint)Math.Clamp(Math.Round(b * 255), 0, 255);

    /// <summary>Every operator this cares about, matched in stream order.</summary>
    [GeneratedRegex(
        @"(?<q>\bq\b)"
        + @"|(?<Q>\bQ\b)"
        + @"|(?<w>-?[\d.]+)\s+w\b"
        + @"|(?:(?<cnum>-?[\d.]+)\s+){1,4}(?<cop>RG|G|K)(?![A-Za-z])"
        + @"|(?<rx>-?[\d.]+)\s+(?<ry>-?[\d.]+)\s+(?<rw>-?[\d.]+)\s+(?<rh>-?[\d.]+)\s+re\s+S\b"
        + @"|(?<lx1>-?[\d.]+)\s+(?<ly1>-?[\d.]+)\s+m\s+(?<lx2>-?[\d.]+)\s+(?<ly2>-?[\d.]+)\s+l\s+S\b",
        RegexOptions.Singleline)]
    private static partial Regex Operator();

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

            byte[] raw = Encoding.Latin1.GetBytes(text[start..end]);

            try
            {
                using MemoryStream compressed = new(raw);
                using ZLibStream inflate = new(compressed, CompressionMode.Decompress);
                using MemoryStream plain = new();
                inflate.CopyTo(plain);

                string content = Encoding.Latin1.GetString(plain.ToArray());
                if (content.Length > 0) streams.Add(content);
            }
            catch (InvalidDataException)
            {
                // Not a Flate stream — a font, an image, or an object stream. Skipping it is
                // right: only the page content streams hold drawing operators.
            }
        }

        return streams;
    }

    [GeneratedRegex(@"stream\r?\n")]
    private static partial Regex StreamStart();
}
