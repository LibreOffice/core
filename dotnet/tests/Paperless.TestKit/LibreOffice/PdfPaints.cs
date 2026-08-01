using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;

namespace Paperless.TestKit.LibreOffice;

/// <summary>One gradient a PDF paints, as its shading dictionary states it.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="Type">
/// The PDF shading type: 1 function-based, 2 axial, 3 radial. LibreOffice writes 2 for a
/// linear or axial gradient and 1 for anything else; Paperless writes 2 and 3.
/// </param>
/// <param name="Coords">
/// The <c>/Coords</c> array as written: four numbers for an axial shading (the two ends of the
/// axis), six for a radial one (centre, inner radius, centre, outer radius). Empty for a
/// function-based shading, which states a <c>/Matrix</c> instead.
/// </param>
/// <param name="StartColour">The colour the function gives at the start of its domain, as <c>0xRRGGBB</c>.</param>
/// <param name="EndColour">The colour at the end of the domain.</param>
/// <param name="Clip">
/// The rectangle the shading was clipped to when it was painted, in points from the page's top
/// left — which is the shape the gradient fills, since <c>sh</c> paints the whole clip.
/// Null when no rectangular clip preceded it.
/// </param>
public readonly record struct PdfShading(
    int PageIndex,
    int Type,
    IReadOnlyList<double> Coords,
    uint StartColour,
    uint EndColour,
    PdfBox? Clip);

/// <summary>One placement of an image XObject.</summary>
/// <param name="PageIndex">Which page it is on, counted from zero.</param>
/// <param name="Resource">The XObject's resource name, such as <c>Im1</c>.</param>
/// <param name="Box">Where it was drawn, in points from the page's top left.</param>
/// <param name="PixelWidth">The image's own width in samples.</param>
/// <param name="PixelHeight">The image's own height in samples.</param>
/// <param name="Filter">
/// How the samples are stored: <c>DCTDecode</c> for a JPEG passed through untouched,
/// <c>FlateDecode</c> for deflated RGB.
/// </param>
/// <param name="HasSoftMask">True when the image carries an <c>/SMask</c> alpha channel.</param>
public readonly record struct PdfImageDraw(
    int PageIndex,
    string Resource,
    PdfBox Box,
    int PixelWidth,
    int PixelHeight,
    string Filter,
    bool HasSoftMask);

/// <summary>A rectangle in points, measured from the page's top left.</summary>
/// <param name="Left">Distance from the left edge.</param>
/// <param name="Top">Distance from the top edge.</param>
/// <param name="Width">Width.</param>
/// <param name="Height">Height.</param>
public readonly record struct PdfBox(double Left, double Top, double Width, double Height);

/// <summary>
/// Reads the gradients and the pictures out of a PDF.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why this is a separate reader rather than an extension of <see cref="PdfFills"/>.</b>
/// That one finds a page by inflating every stream and keeping the ones holding <c>BT</c> —
/// which is exactly right for a word-processing page and wrong for a slide, where a page of
/// four filled shapes and no text has no <c>BT</c> in it at all and would simply not be seen.
/// Changing its rule would renumber the pages every existing comparison reports, so this walks
/// the objects properly instead and leaves <c>PdfFills</c>, <c>PdfStrokes</c> and
/// <c>PdfTextRuns</c> exactly as they were.
/// </para>
/// <para>
/// <b>What it is for.</b> A gradient and a tiled fill are invisible to every reader that
/// existed before it: <c>PdfFills</c> would report a gradient-filled rectangle with whatever
/// colour was last set, since <c>/Pattern</c> and <c>sh</c> set none. Worse, the two sides
/// state the same picture completely differently — LibreOffice decomposes a shape gradient
/// into flat bands for PDF export (<c>vclmetafileprocessor2d.cxx</c>,
/// <c>processPolyPolygonGradientPrimitive2D</c>: "tdf#150551 for PDF export, use the
/// decomposition"), so its PDF of <c>tests/corpus/features/paint-fills.fodp</c> holds no
/// shading dictionary at all and 91602 bytes of content stream, where ours holds three
/// shadings and 2570 bytes. A fill-for-fill comparison of a gradient is therefore not a
/// meaningful question; what each side <em>states</em> is.
/// </para>
/// <para>
/// Deliberately not a PDF parser, like every other reader here. It walks numbered objects,
/// inflates the streams, and reads the two shapes both writers emit.
/// </para>
/// </remarks>
public static partial class PdfPaints
{
    /// <summary>Reads every shading a PDF paints, in the order it was painted.</summary>
    public static List<PdfShading> ReadShadings(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        Document document = Document.Load(pdfPath);
        List<PdfShading> shadings = [];

        for (int page = 0; page < document.Pages.Count; page++)
        {
            string content = document.Pages[page];

            foreach (Match match in ShadingPaint().Matches(content))
            {
                if (document.Resource(page, "Shading", match.Groups[1].Value) is not { } id) continue;
                if (document.Dictionary(id) is not { } body) continue;

                shadings.Add(new PdfShading(
                    page,
                    Integer(body, "ShadingType"),
                    Numbers(body, "Coords"),
                    document.FunctionColour(body, first: true),
                    document.FunctionColour(body, first: false),
                    ClipBefore(content, match.Index, document.PageHeight)));
            }
        }

        return shadings;
    }

    /// <summary>Reads every image placement in a PDF, in the order it was drawn.</summary>
    public static List<PdfImageDraw> ReadImageDraws(string pdfPath)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        Document document = Document.Load(pdfPath);
        List<PdfImageDraw> draws = [];

        for (int page = 0; page < document.Pages.Count; page++)
        {
            foreach (Match match in ImagePaint().Matches(document.Pages[page]))
            {
                string resource = match.Groups[5].Value;
                if (document.Resource(page, "XObject", resource) is not { } id) continue;
                if (document.Dictionary(id) is not { } body) continue;
                if (!body.Contains("/Subtype/Image", StringComparison.Ordinal)) continue;

                double width = Value(match, 1);
                double height = Value(match, 2);
                double left = Value(match, 3);
                double bottom = Value(match, 4);

                draws.Add(new PdfImageDraw(
                    page,
                    resource,
                    new PdfBox(left, document.PageHeight - bottom - height, width, height),
                    Integer(body, "Width"),
                    Integer(body, "Height"),
                    Filter(body),
                    body.Contains("/SMask", StringComparison.Ordinal)));
            }
        }

        return draws;
    }

    /// <summary>
    /// The rectangular clip in force at a point in a content stream.
    /// </summary>
    /// <remarks>
    /// The nearest preceding <c>re</c> that is used as a clip rather than filled, with any
    /// translation-only <c>cm</c> between the enclosing <c>q</c> and it applied. Both writers
    /// produce one of exactly two shapes — Paperless states the rectangle in page coordinates
    /// and LibreOffice translates first and states it at the origin — and a translation is all
    /// that separates them.
    /// </remarks>
    private static PdfBox? ClipBefore(string content, int at, double pageHeight)
    {
        int open = content.LastIndexOf('q', Math.Min(at, content.Length - 1));
        string window = content[(open < 0 ? 0 : open)..at];

        MatchCollection clips = ClipRectangle().Matches(window);
        if (clips.Count == 0) return null;

        Match clip = clips[^1];
        double dx = 0, dy = 0;

        foreach (Match shift in Translation().Matches(window[..clip.Index]))
        {
            dx += double.Parse(shift.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture);
            dy += double.Parse(shift.Groups[2].Value, NumberStyles.Float, CultureInfo.InvariantCulture);
        }

        double left = Value(clip, 1) + dx;
        double bottom = Value(clip, 2) + dy;
        double width = Value(clip, 3);
        double height = Value(clip, 4);

        return new PdfBox(left, pageHeight - bottom - height, width, height);
    }

    private static double Value(Match match, int group)
        => double.TryParse(
            match.Groups[group].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : 0;

    private static int Integer(string dictionary, string key)
    {
        Match match = Regex.Match(dictionary, $@"/{key}\s+(-?\d+)");
        return match.Success ? int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture) : 0;
    }

    private static List<double> Numbers(string dictionary, string key)
    {
        Match match = Regex.Match(dictionary, $@"/{key}\s*\[([^\]]*)\]");
        if (!match.Success) return [];

        return [.. match.Groups[1].Value
            .Split((char[])[' ', '\r', '\n', '\t'], StringSplitOptions.RemoveEmptyEntries)
            .Select(token => double.TryParse(
                token, NumberStyles.Float, CultureInfo.InvariantCulture, out double value) ? value : 0)];
    }

    private static string Filter(string dictionary)
    {
        Match match = Regex.Match(dictionary, @"/Filter\s*/(\w+)");
        return match.Success ? match.Groups[1].Value : string.Empty;
    }

    /// <summary><c>/Name sh</c>.</summary>
    [GeneratedRegex(@"/([A-Za-z0-9]+)\s+sh\b")]
    private static partial Regex ShadingPaint();

    /// <summary><c>w 0 0 h x y cm … /Name Do</c>, which is how both writers place an image.</summary>
    [GeneratedRegex(
        @"(-?[\d.]+)\s+0\s+0\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+cm\s*/([A-Za-z0-9]+)\s+Do\b",
        RegexOptions.Singleline)]
    private static partial Regex ImagePaint();

    /// <summary><c>x y w h re W n</c> — a rectangle used as a clip rather than painted.</summary>
    [GeneratedRegex(
        @"(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+re\s+W\*?\s+n\b",
        RegexOptions.Singleline)]
    private static partial Regex ClipRectangle();

    /// <summary><c>1 0 0 1 tx ty cm</c>, a transform that only moves.</summary>
    [GeneratedRegex(@"1\s+0\s+0\s+1\s+(-?[\d.]+)\s+(-?[\d.]+)\s+cm\b")]
    private static partial Regex Translation();

    // -------------------------------------------------------------------------------- objects

    /// <summary>
    /// A PDF walked object by object, far enough to look a resource up by name.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A page is found by its <c>/Type/Page</c> dictionary and its content and resources by the
    /// <c>/Contents</c> and <c>/Resources</c> references in it, rather than by guessing which
    /// stream is a page and which dictionary is a resource list. The guess is what a smaller
    /// reader would do and it is wrong in a way that is invisible: a shading dictionary
    /// contains the string <c>/ShadingType</c>, so "the dictionary mentioning <c>/Shading</c>"
    /// picks the last shading in the file instead of the resource list, and every lookup then
    /// silently returns nothing.
    /// </para>
    /// <para>
    /// Pages come out in the order their objects appear in the file, which is the order both
    /// writers emit them. Following the page tree's <c>/Kids</c> array would be more correct
    /// and neither writer needs it.
    /// </para>
    /// </remarks>
    private sealed partial class Document
    {
        private readonly Dictionary<int, string> _dictionaries = [];
        private readonly Dictionary<int, byte[]> _streams = [];
        private readonly List<int> _resources = [];

        public List<string> Pages { get; } = [];

        public double PageHeight { get; private set; } = 841.89;

        public static Document Load(string path)
        {
            byte[] bytes = File.ReadAllBytes(path);
            string text = Encoding.Latin1.GetString(bytes);
            Document document = new();

            Match box = MediaBox().Match(text);
            if (box.Success
                && double.TryParse(
                    box.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out double height))
            {
                document.PageHeight = height;
            }

            foreach (Match match in ObjectStart().Matches(text))
            {
                int id = int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
                int start = match.Index + match.Length;
                int end = text.IndexOf("endobj", start, StringComparison.Ordinal);
                if (end < 0) continue;

                Match opening = StreamStart().Match(text, start);
                if (!opening.Success || opening.Index > end)
                {
                    document._dictionaries[id] = text[start..end].Trim();
                    continue;
                }

                document._dictionaries[id] = text[start..opening.Index].Trim();

                int payload = opening.Index + opening.Length;
                int finish = text.IndexOf("endstream", payload, StringComparison.Ordinal);
                if (finish < payload) continue;

                document._streams[id] = Inflate(
                    bytes, payload, finish, document._dictionaries[id].Contains(
                        "/FlateDecode", StringComparison.Ordinal));
            }

            foreach ((int _, string dictionary) in document._dictionaries)
            {
                if (!dictionary.Contains("/Type/Page", StringComparison.Ordinal)) continue;
                if (dictionary.Contains("/Type/Pages", StringComparison.Ordinal)) continue;

                document._resources.Add(Reference(dictionary, "Resources") ?? 0);
                document.Pages.Add(
                    Reference(dictionary, "Contents") is { } contents
                    && document._streams.TryGetValue(contents, out byte[]? body)
                        ? Encoding.Latin1.GetString(body)
                        : string.Empty);
            }

            return document;
        }

        public string? Dictionary(int id) => _dictionaries.GetValueOrDefault(id);

        /// <summary>The object a named resource of a given kind refers to, on a given page.</summary>
        public int? Resource(int page, string kind, string name)
        {
            if (page < 0 || page >= _resources.Count) return null;
            if (Dictionary(_resources[page]) is not { } resources) return null;

            Match section = Regex.Match(resources, $@"/{kind}\s*<<(.*?)>>", RegexOptions.Singleline);
            if (!section.Success) return null;

            Match entry = Regex.Match(section.Groups[1].Value, $@"/{Regex.Escape(name)}\s+(\d+)\s+0\s+R");
            return entry.Success ? int.Parse(entry.Groups[1].Value, CultureInfo.InvariantCulture) : null;
        }

        private static int? Reference(string dictionary, string key)
        {
            Match match = Regex.Match(dictionary, $@"/{key}\s+(\d+)\s+0\s+R");
            return match.Success ? int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture) : null;
        }

        private static byte[] Inflate(byte[] bytes, int start, int end, bool deflated)
        {
            if (!deflated) return bytes[start..end];

            try
            {
                using MemoryStream compressed = new(bytes, start, end - start);
                using ZLibStream inflater = new(compressed, CompressionMode.Decompress);
                using MemoryStream plain = new();
                inflater.CopyTo(plain);
                return plain.ToArray();
            }
            catch (InvalidDataException)
            {
                // A stream that will not inflate is a stream this reader has nothing to say about.
                return [];
            }
        }

        /// <summary>
        /// The colour a shading's function gives at one end of its domain.
        /// </summary>
        /// <remarks>
        /// Three function types reach here and each states its endpoints differently. A type 2
        /// exponential names them outright as <c>/C0</c> and <c>/C1</c>; a type 3 stitches
        /// several, so the answer comes from the first or last of them; a type 0 is sampled and
        /// its endpoints are the first and last triples of its stream, which is what LibreOffice
        /// writes and what would otherwise read as no colour at all.
        /// </remarks>
        public uint FunctionColour(string shading, bool first)
        {
            Match reference = Regex.Match(shading, @"/Function\s+(\d+)\s+0\s+R");
            if (!reference.Success) return 0;

            return Resolve(int.Parse(reference.Groups[1].Value, CultureInfo.InvariantCulture), first, depth: 0);
        }

        private uint Resolve(int id, bool first, int depth)
        {
            if (depth > 4 || Dictionary(id) is not { } body) return 0;

            if (body.Contains("/FunctionType 3", StringComparison.Ordinal))
            {
                MatchCollection parts = Regex.Matches(body, @"(\d+)\s+0\s+R");
                if (parts.Count == 0) return 0;

                Match part = first ? parts[0] : parts[^1];
                return Resolve(int.Parse(part.Groups[1].Value, CultureInfo.InvariantCulture), first, depth + 1);
            }

            List<double> components = Numbers(body, first ? "C0" : "C1");
            if (components.Count < 3) return 0;

            return Pack(components[0], components[1], components[2]);
        }

        private static uint Pack(double r, double g, double b)
            => ((uint)Math.Clamp(Math.Round(r * 255), 0, 255) << 16)
               | ((uint)Math.Clamp(Math.Round(g * 255), 0, 255) << 8)
               | (uint)Math.Clamp(Math.Round(b * 255), 0, 255);

        [GeneratedRegex(@"(?<![0-9])(\d+) 0 obj\s*")]
        private static partial Regex ObjectStart();

        [GeneratedRegex(@"stream\r?\n")]
        private static partial Regex StreamStart();

        [GeneratedRegex(@"/MediaBox\s*\[\s*[\d.-]+\s+[\d.-]+\s+[\d.-]+\s+([\d.-]+)\s*\]")]
        private static partial Regex MediaBox();
    }
}
