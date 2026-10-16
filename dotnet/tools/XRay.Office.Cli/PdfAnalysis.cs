using System.Text;
using UglyToad.PdfPig;
using UglyToad.PdfPig.Content;
using UglyToad.PdfPig.Core;
using UglyToad.PdfPig.Tokenization.Scanner;
using UglyToad.PdfPig.Tokens;
using UglyToad.PdfPig.DocumentLayoutAnalysis.WordExtractor;
using UglyToad.PdfPig.Util;

namespace XRay.Office.Cli;

/// <summary>
/// Reads a PDF in process and reports the three figures a fidelity gate compares: page count and
/// page size, extractable text, and font embedding.
/// </summary>
/// <remarks>
/// <para>
/// This exists to replace <c>pdfinfo</c>, <c>pdftotext</c> and <c>pdffonts</c>, and the reason is
/// worth stating because it cost a round to find. Shelling out to poppler makes poppler's version
/// an undeclared input to every figure the project records, and nothing in the harness declares
/// it. Measured over the words track with our renderer's source provably unchanged, our own word
/// counts moved on <b>169 of 200</b> documents when the container's poppler changed — and
/// <b>86</b> of them moved by exactly the amount the reference moved. A term that shifts both
/// sides of a comparison equally belongs to neither renderer. Owning the extractor and pinning it
/// in the repository is the fix; tuning the metric is not.
/// </para>
/// <para>
/// <b>This is a heuristic, not ground truth, and it must not be read as one.</b> A PDF stores
/// positioned glyphs, not words: every extractor — poppler's, ours, anyone's — re-infers word
/// boundaries from geometry, and two of them will disagree. What changes here is only that the
/// heuristic is ours, pinned to a package version, and moves when the code moves. The token
/// classes are reported alongside the total for the same reason: a future round comparing two
/// numbers should be able to see whether a difference is real text or a boundary rule.
/// </para>
/// </remarks>
public static class PdfAnalysis
{
    /// <summary>Reads <paramref name="pdfPath"/> and reports everything the gate needs.</summary>
    /// <param name="pdfPath">The PDF to read.</param>
    /// <param name="includeText">Whether to retain the extracted text on the result.</param>
    /// <param name="grouping">How glyphs are grouped into words.</param>
    /// <returns>
    /// The analysis. A file that cannot be parsed comes back with <see cref="PdfAnalysisResult.Error"/>
    /// set rather than throwing: a corpus sweep must be able to record a broken document as broken
    /// and carry on.
    /// </returns>
    public static PdfAnalysisResult Analyze(
        string pdfPath,
        bool includeText = false,
        WordGrouping grouping = WordGrouping.NearestNeighbour)
    {
        ArgumentNullException.ThrowIfNull(pdfPath);

        IWordExtractor extractor = grouping switch
        {
            WordGrouping.NearestNeighbour => NearestNeighbour,
            WordGrouping.Simple => DefaultWordExtractor.Instance,
            _ => throw new ArgumentOutOfRangeException(nameof(grouping)),
        };

        try
        {
            // Lenient parsing, and missing fonts skipped rather than fatal: the corpus is real-world
            // output from two renderers, and refusing a document outright is strictly worse than
            // reporting what could be read from it.
            using PdfDocument document = PdfDocument.Open(pdfPath, new ParsingOptions
            {
                UseLenientParsing = true,
                SkipMissingFonts = true,
                ClipPaths = false,
            });

            List<PdfPageInfo> pages = new(document.NumberOfPages);
            FontCollector fonts = new(document.Structure.TokenScanner);
            WordTally tally = new();
            StringBuilder? text = includeText ? new StringBuilder() : null;

            for (int number = 1; number <= document.NumberOfPages; number++)
            {
                Page page = document.GetPage(number);

                pages.Add(new PdfPageInfo(
                    number,
                    Round(page.Width),
                    Round(page.Height),
                    Round(page.MediaBox.Bounds.Width),
                    Round(page.MediaBox.Bounds.Height),
                    page.Rotation.Value));

                fonts.Collect(page);

                bool first = true;
                foreach (Word word in extractor.GetWords(OnPage(page)))
                {
                    tally.Add(word.Text);

                    if (text is not null)
                    {
                        if (!first) text.Append(' ');
                        text.Append(word.Text);
                        first = false;
                    }
                }

                text?.Append('\n');
            }

            return new PdfAnalysisResult
            {
                File = pdfPath,
                PageCount = document.NumberOfPages,
                Pages = pages,
                Words = tally.ToCounts(),
                Fonts = fonts.ToList(),
                Text = text?.ToString(),
            };
        }
        catch (Exception exception) when (exception is not OutOfMemoryException
                                              and not StackOverflowException)
        {
            // PdfPig throws a wide and undocumented range on malformed input — PdfDocumentFormatException,
            // InvalidOperationException, IndexOutOfRangeException, KeyNotFoundException have all been
            // seen. Enumerating them would be a guess that fails silently on the next one, and the
            // caller's contract is "report what you found"; so this catches broadly and records the
            // type in the message rather than pretending to know the set.
            return new PdfAnalysisResult
            {
                File = pdfPath,
                Error = $"{exception.GetType().Name}: {Flatten(exception.Message)}",
            };
        }
    }

    /// <summary>
    /// PdfPig's orientation-aware word grouper, run single-threaded.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>MaxDegreeOfParallelism = 1</c> for two reasons. The extractor otherwise fans out over the
    /// five orientation buckets and merges them under a lock, so the <em>order</em> of the words it
    /// returns is not reproducible — the count is, but a tool that prints text must not depend on
    /// that distinction holding. And this runs inside a corpus sweep that is already parallel across
    /// documents; a second layer of threads underneath it buys nothing and costs scheduling.
    /// </para>
    /// <para>
    /// The remaining defaults are taken as they come, deliberately. Every one of them is a tuning
    /// constant over glyph geometry, and re-tuning them here would put the heuristic back in our
    /// hands and out of the pinned package — which is the whole point of pinning it.
    /// </para>
    /// </remarks>
    private static readonly NearestNeighbourWordExtractor NearestNeighbour =
        new(new NearestNeighbourWordExtractor.NearestNeighbourWordExtractorOptions
        {
            MaxDegreeOfParallelism = 1,
        });

    /// <summary>The glyphs of a page that are actually on it.</summary>
    /// <remarks>
    /// <para>
    /// A glyph counts when the pen position it was drawn at falls inside the crop box. Everything
    /// else is text the file contains and no reader ever shows — a spreadsheet cell whose contents
    /// overflow past the printable area, a slide's off-canvas scratch area, a drawing parked beside
    /// the page. It is not visible, so for a question of the form "does our rendering carry the same
    /// text as LibreOffice's" it is not text.
    /// </para>
    /// <para>
    /// This is the single largest term in the difference between this reader and <c>pdftotext</c>,
    /// and it was found by measurement rather than assumed. Poppler discards out-of-bounds
    /// characters in <c>TextPage::addChar</c>; the first version of this code did not, and on
    /// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.xls</c> — <b>50.8% of whose 125 086 glyphs lie off the
    /// page</b> — it reported 18 106 words against poppler's 9290, very nearly a factor of two.
    /// Applying the rule takes that document to 9015, and the whole-corpus Σ|Δ| against poppler from
    /// 120 122 tokens to 74 088.
    /// </para>
    /// <para>
    /// The pen position rather than the glyph's ink box, tested and chosen: an ink-box intersection
    /// test scores 442 documents inside the gate's 2%+3 band against this rule's 446, and is
    /// marginally worse on every track, because a glyph half off the edge is drawn clipped and
    /// poppler drops it too. The crop box rather than the media box, because the crop box is what a
    /// viewer shows (ISO 32000-2 14.11.2) and it has already been clipped to the media box.
    /// </para>
    /// </remarks>
    private static List<Letter> OnPage(Page page)
    {
        PdfRectangle box = page.CropBox.Bounds;
        double left = Math.Min(box.BottomLeft.X, box.TopRight.X);
        double right = Math.Max(box.BottomLeft.X, box.TopRight.X);
        double bottom = Math.Min(box.BottomLeft.Y, box.TopRight.Y);
        double top = Math.Max(box.BottomLeft.Y, box.TopRight.Y);

        IReadOnlyList<Letter> letters = page.Letters;
        List<Letter> onPage = new(letters.Count);

        foreach (Letter letter in letters)
        {
            PdfPoint pen = letter.StartBaseLine;
            if (pen.X >= left && pen.X <= right && pen.Y >= bottom && pen.Y <= top) onPage.Add(letter);
        }

        return onPage;
    }

    /// <summary>Rounds a point measurement to a thousandth, so two runs print the same digits.</summary>
    /// <remarks>
    /// PDF coordinates arrive as doubles through a transformation matrix. Printing them raw makes a
    /// diff of two sweeps noisy in the last bit for no gain; a thousandth of a point is 2.5 nm.
    /// </remarks>
    private static double Round(double points) => Math.Round(points, 3, MidpointRounding.AwayFromZero);

    private static string Flatten(string message)
        => message.Replace('\t', ' ').Replace('\r', ' ').Replace('\n', ' ');

    // ------------------------------------------------------------------------------ words

    /// <summary>
    /// Counts tokens and splits them into the classes a word metric might or might not want.
    /// </summary>
    /// <remarks>
    /// The classes partition the total exactly — <c>Raw == Alphanumeric + Bullet + PrivateUse +
    /// Punctuation</c> — which is asserted by a test, because a decomposition that does not add up
    /// is how a term goes missing without anyone noticing.
    /// </remarks>
    private struct WordTally
    {
        private int raw;
        private int alphanumeric;
        private int bullet;
        private int privateUse;
        private int punctuation;

        public void Add(string token)
        {
            // A Word from the extractor holds no whitespace, but tokenise anyway: the contract this
            // reports is "what `wc -w` would say about the text we emit", and that must not depend on
            // an invariant of somebody else's class.
            foreach (Range range in Tokenise(token))
            {
                ReadOnlySpan<char> span = token.AsSpan()[range];
                if (span.IsEmpty) continue;

                raw++;

                if (ContainsAlphanumeric(span)) alphanumeric++;
                else if (AllBullet(span)) bullet++;
                else if (ContainsPrivateUse(span)) privateUse++;
                else punctuation++;
            }
        }

        public readonly PdfWordCounts ToCounts()
            => new(raw, alphanumeric, bullet, privateUse, punctuation);

        /// <summary>Splits on whitespace the way Python's <c>str.split()</c> with no argument does.</summary>
        /// <remarks>
        /// The gate's <c>words_of()</c> is <c>text.split()</c> in Python, so this has to agree with it
        /// and not merely with <c>wc -w</c>. The two differ in one place: Python's whitespace set
        /// includes the C0 file, group, record and unit separators U+001C–U+001F, which
        /// <see cref="char.IsWhiteSpace(char)"/> does not. They are rare and they do turn up in text
        /// lifted out of legacy binary formats, so they are added rather than argued about.
        /// </remarks>
        private static List<Range> Tokenise(string token)
        {
            List<Range> ranges = [];
            int start = -1;
            for (int i = 0; i < token.Length; i++)
            {
                bool space = char.IsWhiteSpace(token[i]) || token[i] is >= '\u001C' and <= '\u001F';
                if (!space && start < 0) start = i;
                else if (space && start >= 0) { ranges.Add(start..i); start = -1; }
            }
            if (start >= 0) ranges.Add(start..token.Length);
            return ranges;
        }

        /// <summary>Whether a token carries at least one Unicode letter or digit.</summary>
        /// <remarks>
        /// <para>
        /// This is the corpus gate's word definition, decided and merged by the round that owns it,
        /// and reimplemented here rather than re-decided. The gate expresses it as Python's
        /// <c>any(c.isalnum() for c in w)</c>, and <c>str.isalnum()</c> is true for the letter
        /// categories <em>and</em> for every numeric category — Nd, Nl and No. So the predicate is
        /// <c>IsLetter || IsNumber</c>, not <c>IsLetterOrDigit</c>: the latter stops at Nd and would
        /// score "½", "Ⅻ" and a superscript "²" as not-a-word where the gate scores them as words.
        /// </para>
        /// <para>
        /// Enumerated as runes rather than chars. Python iterates code points; .NET iterates UTF-16
        /// units, and <see cref="char.IsLetter(char)"/> is false for either half of a surrogate pair.
        /// A CJK ideograph above the BMP — extension B onwards, which real documents do carry — would
        /// otherwise be counted as punctuation by a reader that believes it handles Unicode.
        /// </para>
        /// </remarks>
        private static bool ContainsAlphanumeric(ReadOnlySpan<char> span)
        {
            foreach (Rune rune in span.EnumerateRunes())
            {
                if (Rune.IsLetter(rune) || Rune.IsNumber(rune)) return true;
            }

            return false;
        }

        private static bool ContainsPrivateUse(ReadOnlySpan<char> span)
        {
            for (int i = 0; i < span.Length; i++)
            {
                if (char.IsHighSurrogate(span[i]) && i + 1 < span.Length && char.IsLowSurrogate(span[i + 1]))
                {
                    // Planes 15 and 16 are Supplementary Private Use Area A and B. A Symbol or
                    // Wingdings glyph nearly always lands in the BMP area below, but a CID font
                    // re-mapped by a producer can land up here, so both are checked.
                    int scalar = char.ConvertToUtf32(span[i], span[i + 1]);
                    if (scalar is >= 0xF0000 and <= 0x10FFFD) return true;
                    i++;
                    continue;
                }

                if (span[i] is >= '\uE000' and <= '\uF8FF') return true; // BMP Private Use Area
            }

            return false;
        }

        /// <summary>
        /// Whether every character is a bullet-like mark.
        /// </summary>
        /// <remarks>
        /// Deliberately narrow: the glyphs a list bullet is actually drawn with, not "any symbol".
        /// A Symbol/Wingdings bullet does not appear here at all — it maps to U+F0B7 in the Private
        /// Use Area and is counted as <see cref="PdfWordCounts.PrivateUse"/>, which is the more
        /// informative answer, because the two cases have different causes and a metric may want to
        /// treat them differently.
        /// </remarks>
        private static bool AllBullet(ReadOnlySpan<char> span)
        {
            foreach (char c in span)
            {
                bool isBullet = c switch
                {
                    '·' => true, // MIDDLE DOT
                    '•' or '‣' or '⁃' or '⁌' or '⁍' => true, // BULLET and friends
                    '∙' => true, // BULLET OPERATOR
                    '■' or '□' or '▪' or '▫' => true, // squares
                    '○' or '●' or '◘' or '◙' or '◦' => true, // circles
                    '▶' or '►' or '◆' or '◇' => true, // triangles and diamonds
                    '❖' or '➔' or '➢' => true, // dingbat bullets and arrows
                    _ => false,
                };

                if (!isBullet) return false;
            }

            return true;
        }
    }

    // ------------------------------------------------------------------------------ fonts

    /// <summary>
    /// Walks the page resource dictionaries and reports each distinct font face.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Deliberately the same question <c>pdffonts</c> answers, so the two columns are comparable:
    /// every font reachable from a page's resources, including through form XObjects and tiling
    /// patterns, whether or not any content stream actually draws with it, de-duplicated by the
    /// indirect reference of its font dictionary.
    /// </para>
    /// <para>
    /// It is <em>not</em> the same question as "which fonts did the text on this page use". Reading
    /// the fonts off the extracted letters instead would silently drop a face that is declared and
    /// unused — exactly the case an <c>unembedded</c> check exists to catch — and would count a face
    /// that appears on ten pages once per page unless separately de-duplicated.
    /// </para>
    /// <para>
    /// <b>Embedded</b> means the font dictionary's descriptor carries a font program:
    /// <c>/FontFile</c> (Type 1), <c>/FontFile2</c> (TrueType) or <c>/FontFile3</c> (a program whose
    /// format the stream's own <c>/Subtype</c> names). For a Type 0 font the descriptor lives on the
    /// descendant CIDFont, not on the Type 0 dictionary, so it is read from there. This is the
    /// semantics the shell version got at by column position, and the lesson that cost is worth
    /// keeping: <c>pdffonts</c>'s <c>emb</c> column is <c>$(NF-4)</c>, not <c>$(NF-3)</c>, because
    /// the row ends <c>emb sub uni object id</c> — reading <c>NF-3</c> gets <c>sub</c> instead, and
    /// it happens to agree only for a font whose type name is a single field. Every font X-Ray.Office
    /// writes is "TrueType", one field, so that check tested nothing about our own output until it
    /// was corrected. Nothing here parses a column, and that is the point.
    /// </para>
    /// </remarks>
    private sealed class FontCollector(IPdfTokenScanner scanner)
    {
        private readonly List<PdfFontInfo> fonts = [];
        private readonly HashSet<string> seen = [];

        /// <summary>
        /// Every form XObject and pattern already descended into, for the whole document.
        /// </summary>
        /// <remarks>
        /// <b>This set is not an optimisation; without it the walk does not terminate on real files.</b>
        /// Resource dictionaries are shared: a deck's ten slides point at one resources dictionary
        /// holding <i>n</i> form XObjects, and each of those points back at a resources dictionary
        /// holding the same <i>n</i>. Descending without remembering therefore costs O(<i>n</i>ᵈ) —
        /// the depth cap alone does not save it, it only bounds the exponent — and the first version
        /// of this code hung on 17 of the corpus's 534 documents, indefinitely rather than slowly:
        /// <c>1-secretariat__ppt.pdf</c> is 10 pages and 2402 glyphs, reads in 0.7 s with the walk
        /// removed, and had not finished the walk after 570 s. Keyed on the indirect reference,
        /// document-wide, so each object is entered exactly once however many pages reach it.
        /// </remarks>
        private readonly HashSet<string> descended = [];

        public void Collect(Page page)
        {
            // /Resources is inheritable through the page tree (ISO 32000-2 7.7.3.4), so a page
            // dictionary that does not carry one is not a page without fonts — it is a page whose
            // fonts are on an ancestor. Missing this reports zero fonts for whole documents.
            DictionaryToken? resources = FindResources(page.Dictionary, 0);
            Walk(resources, 0);
        }

        public List<PdfFontInfo> ToList() => fonts;

        private DictionaryToken? FindResources(DictionaryToken dictionary, int depth)
        {
            // A /Parent cycle in a malformed file would otherwise spin forever.
            if (depth > 64) return null;
            if (dictionary.TryGet(NameToken.Resources, scanner, out DictionaryToken? resources)) return resources;
            return dictionary.TryGet(NameToken.Parent, scanner, out DictionaryToken? parent)
                ? FindResources(parent, depth + 1)
                : null;
        }

        private void Walk(DictionaryToken? resources, int depth)
        {
            // Form XObjects nest; eight levels is far past anything a real producer emits and stops
            // a self-referential resource tree from recursing without end.
            if (resources is null || depth > 8) return;

            if (resources.TryGet(NameToken.Font, scanner, out DictionaryToken? fontResources))
            {
                foreach (KeyValuePair<string, IToken> entry in fontResources.Data)
                {
                    if (Resolve(entry.Value) is not DictionaryToken font) continue;
                    if (!seen.Add(Identity(entry.Value, font))) continue;
                    fonts.Add(Describe(font));
                }
            }

            if (resources.TryGet(NameToken.Xobject, scanner, out DictionaryToken? xobjects))
            {
                foreach (KeyValuePair<string, IToken> entry in xobjects.Data)
                {
                    if (!ShouldDescend("x", entry.Value)) continue;

                    if (Resolve(entry.Value) is StreamToken form
                        && form.StreamDictionary.TryGet(NameToken.Resources, scanner, out DictionaryToken? nested))
                    {
                        Walk(nested, depth + 1);
                    }
                }
            }

            if (resources.TryGet(NameToken.Pattern, scanner, out DictionaryToken? patterns))
            {
                foreach (KeyValuePair<string, IToken> entry in patterns.Data)
                {
                    if (!ShouldDescend("p", entry.Value)) continue;

                    IToken? resolved = Resolve(entry.Value);
                    DictionaryToken? pattern = resolved as DictionaryToken
                                               ?? (resolved as StreamToken)?.StreamDictionary;
                    if (pattern is not null && pattern.TryGet(NameToken.Resources, scanner, out DictionaryToken? nested))
                    {
                        Walk(nested, depth + 1);
                    }
                }
            }
        }

        private IToken? Resolve(IToken token)
            => token is IndirectReferenceToken reference ? scanner.Get(reference.Data)?.Data : token;

        /// <summary>
        /// Whether this nested resource holder has not been descended into yet, recording it if so.
        /// </summary>
        /// <remarks>
        /// Only an indirect reference can be remembered, and only an indirect reference can be shared
        /// — a dictionary written inline is reachable from one place by construction, so it cannot be
        /// the thing that multiplies. The <paramref name="kind"/> prefix keeps the XObject and Pattern
        /// namespaces apart; an object can legitimately be both.
        /// </remarks>
        private bool ShouldDescend(string kind, IToken token)
            => token is not IndirectReferenceToken reference || descended.Add(kind + reference.Data);

        /// <summary>The key a font is de-duplicated by.</summary>
        /// <remarks>
        /// The indirect reference when there is one, which is what <c>pdffonts</c> uses and is exact:
        /// two subsets of one typeface embedded under the same base name are two faces and must count
        /// as two. A font dictionary written directly into a resource dictionary has no reference, so
        /// it falls back to its own content — imperfect, in that two identical inline fonts on
        /// different pages collapse into one, but that is rarer than the alternative failure of
        /// counting one shared face once per page.
        /// </remarks>
        private static string Identity(IToken token, DictionaryToken font)
            => token is IndirectReferenceToken reference
                ? reference.Data.ToString()
                : "inline:" + font;

        private PdfFontInfo Describe(DictionaryToken font)
        {
            string type = font.TryGet(NameToken.Subtype, scanner, out NameToken? subtype) ? subtype.Data : "Unknown";
            string name = font.TryGet(NameToken.BaseFont, scanner, out NameToken? baseFont)
                ? baseFont.Data
                : font.TryGet(NameToken.Name, scanner, out NameToken? resourceName) ? resourceName.Data : "";

            DictionaryToken descriptorHolder = font;
            if (type == "Type0"
                && font.TryGet(NameToken.DescendantFonts, scanner, out ArrayToken? descendants)
                && descendants.Data.Count > 0
                && Resolve(descendants.Data[0]) is DictionaryToken descendant)
            {
                descriptorHolder = descendant;
                if (descendant.TryGet(NameToken.Subtype, scanner, out NameToken? descendantType))
                {
                    type = descendantType.Data;
                }
            }

            bool embedded = descriptorHolder.TryGet(NameToken.FontDescriptor, scanner, out DictionaryToken? descriptor)
                            && (descriptor.ContainsKey(NameToken.FontFile)
                                || descriptor.ContainsKey(NameToken.FontFile2)
                                || descriptor.ContainsKey(NameToken.FontFile3));

            // A Type 3 font's glyphs *are* content streams inside the file, so there is nothing left
            // to embed and nothing a viewer would have to substitute. pdffonts says yes here too.
            if (type == "Type3") embedded = true;

            // The subset prefix from ISO 32000-2 9.6.4: six upper-case letters and a plus sign, e.g.
            // "BAAAAA+LiberationSerif". Tested for shape rather than merely for the '+', because a
            // plus sign is legal anywhere in a font name.
            bool subset = name.Length > 7
                          && name[6] == '+'
                          && name.AsSpan(0, 6).ContainsOnlyUpperAscii();

            return new PdfFontInfo(name, type, embedded, subset);
        }
    }
}

internal static class SpanExtensions
{
    public static bool ContainsOnlyUpperAscii(this ReadOnlySpan<char> span)
    {
        foreach (char c in span) if (c is < 'A' or > 'Z') return false;
        return true;
    }
}

/// <summary>One page's geometry, in PostScript points.</summary>
/// <param name="Number">The page number, counted from one.</param>
/// <param name="WidthPoints">
/// The visible width — the crop box, clipped to the media box, with <paramref name="Rotation"/>
/// applied. This is the size a viewer or a rasteriser shows, so it is the one a "page size differs"
/// signal should compare.
/// </param>
/// <param name="HeightPoints">The visible height, on the same basis.</param>
/// <param name="MediaWidthPoints">The media box width, unrotated — what <c>pdfinfo</c> prints.</param>
/// <param name="MediaHeightPoints">The media box height, unrotated.</param>
/// <param name="Rotation">The page's <c>/Rotate</c>, in degrees clockwise.</param>
public sealed record PdfPageInfo(
    int Number,
    double WidthPoints,
    double HeightPoints,
    double MediaWidthPoints,
    double MediaHeightPoints,
    int Rotation);

/// <summary>One font face a page's resources reach.</summary>
/// <param name="Name">The <c>/BaseFont</c> name, subset prefix included.</param>
/// <param name="Type">The font subtype; for a Type 0 font, its descendant CIDFont's subtype.</param>
/// <param name="Embedded">Whether the file carries the font program.</param>
/// <param name="Subset">Whether the name carries the six-letter subset prefix.</param>
public sealed record PdfFontInfo(string Name, string Type, bool Embedded, bool Subset);

/// <summary>
/// The token counts, split so that a caller can choose a word metric rather than inherit one.
/// </summary>
/// <param name="Raw">
/// Every whitespace-delimited token — the closest analogue of <c>pdftotext | wc -w</c>.
/// </param>
/// <param name="Alphanumeric">Tokens holding at least one letter or digit.</param>
/// <param name="Bullet">Tokens made entirely of bullet marks.</param>
/// <param name="PrivateUse">
/// Tokens with no letter or digit that reach into a Private Use Area — a Symbol or Wingdings glyph
/// that a subsetting producer mapped there, most often a list bullet.
/// </param>
/// <param name="Punctuation">Everything else with no letter or digit.</param>
public readonly record struct PdfWordCounts(
    int Raw,
    int Alphanumeric,
    int Bullet,
    int PrivateUse,
    int Punctuation);

/// <summary>What <see cref="PdfAnalysis.Analyze"/> found.</summary>
public sealed record PdfAnalysisResult
{
    /// <summary>The file that was read.</summary>
    public required string File { get; init; }

    /// <summary>Why it could not be read, or null when it could.</summary>
    public string? Error { get; init; }

    /// <summary>The number of pages.</summary>
    public int PageCount { get; init; }

    /// <summary>Each page's geometry.</summary>
    public IReadOnlyList<PdfPageInfo> Pages { get; init; } = [];

    /// <summary>The token counts.</summary>
    public PdfWordCounts Words { get; init; }

    /// <summary>Every distinct font face reachable from a page's resources.</summary>
    public IReadOnlyList<PdfFontInfo> Fonts { get; init; } = [];

    /// <summary>The extracted text, when it was asked for.</summary>
    public string? Text { get; init; }

    /// <summary>How many faces the file names without carrying the font program.</summary>
    public int UnembeddedFontCount
    {
        get
        {
            int count = 0;
            foreach (PdfFontInfo font in Fonts) if (!font.Embedded) count++;
            return count;
        }
    }

    /// <summary>How many distinct visible page sizes the document holds.</summary>
    /// <remarks>
    /// One for almost every document. More than one is worth surfacing on its own: a landscape
    /// section, or a renderer that lost a page-size change mid-document.
    /// </remarks>
    public int DistinctPageSizeCount
    {
        get
        {
            HashSet<(double, double)> sizes = [];
            foreach (PdfPageInfo page in Pages) sizes.Add((page.WidthPoints, page.HeightPoints));
            return sizes.Count;
        }
    }

    /// <summary>The word count under a given policy.</summary>
    public int WordCount(WordCountPolicy policy) => policy switch
    {
        WordCountPolicy.Alphanumeric => Words.Alphanumeric,
        WordCountPolicy.Raw => Words.Raw,
        _ => throw new ArgumentOutOfRangeException(nameof(policy)),
    };
}

/// <summary>How glyphs are grouped into words.</summary>
/// <remarks>
/// <para>
/// Both are PdfPig's, not ours, and both are heuristics over glyph geometry. The choice was made by
/// measurement against poppler over all 534 canonical reference PDFs, under the gate's own metric
/// and its own 2%+3 band — see <c>dotnet/probes/tooling-01/results.md</c>. It is exposed rather than
/// fixed so the next round can re-take that measurement instead of trusting this sentence.
/// </para>
/// <para>
/// <b>Result: 480 documents in band for <see cref="NearestNeighbour"/> against 460 for
/// <see cref="Simple"/></b>, 37 documents moving in and 17 out.
/// </para>
/// </remarks>
public enum WordGrouping
{
    /// <summary>
    /// Nearest-neighbour clustering, bucketed by text orientation first.
    /// </summary>
    /// <remarks>
    /// The default, and the one that is structurally right rather than merely better on average:
    /// <see cref="Simple"/> has no concept of orientation at all, so a rotated chart axis label —
    /// every glyph at a different height — becomes one word per glyph. On
    /// <c>Keywords_Mapping_Graphs_and_Charts.xlsx</c> that is 8225 words against poppler's 4519;
    /// this grouping reports 4431. Costs about four times the word-grouping time, which is a small
    /// share of a run dominated by content-stream parsing.
    /// </remarks>
    NearestNeighbour,

    /// <summary>
    /// PdfPig's simpler line-then-gap grouper.
    /// </summary>
    /// <remarks>
    /// Kept because it is better on one identifiable shape: a wide spreadsheet whose columns sit
    /// close together, where nearest-neighbour clustering bridges the gap between two cells and
    /// merges their contents. Three sheets carry most of that cost, <c>Laser Report 2024 FOIA
    /// (Oct).xlsx</c> alone 22 308 words of it.
    /// </remarks>
    Simple,
}

/// <summary>Which tokens count as words.</summary>
/// <remarks>
/// <para>
/// Named and selectable rather than a constant inside the counter, because <em>what a word is</em>
/// is a separate decision from <em>how to measure it</em>. Both counts are always reported, along
/// with the classes they differ by; this only chooses which one the single <c>words</c> column
/// carries.
/// </para>
/// <para>
/// The default is <see cref="Alphanumeric"/> because that is the corpus gate's decided metric —
/// measured and merged by the round that owns the definition, not chosen here. <see cref="Raw"/>
/// remains so that any figure recorded before that decision can still be reproduced; a figure
/// quoted without saying which of the two produced it is ambiguous.
/// </para>
/// </remarks>
public enum WordCountPolicy
{
    /// <summary>
    /// Tokens holding at least one Unicode letter or digit. The gate's metric.
    /// </summary>
    Alphanumeric,

    /// <summary>
    /// Every whitespace-delimited token, including bullets, rendering markers and lone currency
    /// symbols. What <c>pdftotext | wc -w</c> counted, kept for reconciling stored figures.
    /// </summary>
    Raw,
}
