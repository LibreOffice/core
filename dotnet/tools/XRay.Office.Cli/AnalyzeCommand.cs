using System.Globalization;
using System.Text.Json;

namespace XRay.Office.Cli;

/// <summary>
/// <c>xray-office analyze</c>: reports a PDF's page count, page sizes, extractable words and font
/// embedding, reading the file in process.
/// </summary>
/// <remarks>
/// <para>
/// Written to replace three poppler binaries in the corpus gate — <c>pdfinfo</c> for the page
/// count, <c>pdftotext | wc -w</c> for the words, <c>pdffonts</c> for the faces. The reason is in
/// <see cref="PdfAnalysis"/>: poppler's version was an undeclared input to every figure this
/// project has recorded, and it was caught moving our own word counts on 169 of 200 documents with
/// the renderer's source provably unchanged.
/// </para>
/// <para>
/// One invocation answers all three questions from a single parse, where the shell version spawned
/// five processes and read the file five times. That is where the speed comes from; it is not a
/// faster PDF reader than poppler and does not claim to be.
/// </para>
/// </remarks>
internal static class AnalyzeCommand
{
    /// <summary>What the plain-text output should be.</summary>
    private enum OutputMode
    {
        /// <summary>One TSV row per document.</summary>
        Document,

        /// <summary>One TSV row per page.</summary>
        Pages,

        /// <summary>One TSV row per font face.</summary>
        Fonts,

        /// <summary>The extracted text, as <c>pdftotext FILE -</c> would give it.</summary>
        Text,
    }

    internal static int Analyze(string[] args)
    {
        bool json = false;
        bool header = true;
        OutputMode mode = OutputMode.Document;
        WordCountPolicy policy = WordCountPolicy.Alphanumeric;
        WordGrouping grouping = WordGrouping.NearestNeighbour;
        bool includeText = false;
        List<string> paths = [];

        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            switch (arg)
            {
                case "--json": json = true; break;
                case "--no-header": header = false; break;
                case "--pages": mode = OutputMode.Pages; break;
                case "--fonts": mode = OutputMode.Fonts; break;
                case "--text": mode = OutputMode.Text; includeText = true; break;
                case "-h" or "--help": PrintUsage(); return Program.ExitSuccess;
                case "--grouping":
                    if (++i >= args.Length) { Console.Error.WriteLine("xray-office analyze: --grouping needs a value."); return Program.ExitUsage; }
                    switch (args[i])
                    {
                        case "nearest": grouping = WordGrouping.NearestNeighbour; break;
                        case "simple": grouping = WordGrouping.Simple; break;
                        default:
                            Console.Error.WriteLine($"xray-office analyze: unknown grouping '{args[i]}'. Use nearest or simple.");
                            return Program.ExitUsage;
                    }
                    break;
                case "--words":
                    if (++i >= args.Length) { Console.Error.WriteLine("xray-office analyze: --words needs a policy."); return Program.ExitUsage; }
                    switch (args[i])
                    {
                        case "raw": policy = WordCountPolicy.Raw; break;
                        case "alnum": policy = WordCountPolicy.Alphanumeric; break;
                        default:
                            Console.Error.WriteLine($"xray-office analyze: unknown word policy '{args[i]}'. Use raw or alnum.");
                            return Program.ExitUsage;
                    }
                    break;
                default:
                    if (arg.StartsWith('-'))
                    {
                        Console.Error.WriteLine($"xray-office analyze: unknown option '{arg}'.");
                        return Program.ExitUsage;
                    }
                    paths.Add(arg);
                    break;
            }
        }

        if (paths.Count == 0)
        {
            Console.Error.WriteLine("xray-office analyze: no files given.");
            PrintUsage(Console.Error);
            return Program.ExitUsage;
        }

        List<PdfAnalysisResult> results = new(paths.Count);
        int exitCode = Program.ExitSuccess;

        foreach (string path in paths)
        {
            PdfAnalysisResult result = File.Exists(path)
                ? PdfAnalysis.Analyze(path, includeText, grouping)
                : new PdfAnalysisResult { File = path, Error = "File not found." };

            results.Add(result);
            if (result.Error is not null) exitCode = Program.ExitFailure;
        }

        if (json)
        {
            Console.WriteLine(JsonSerializer.Serialize(results, Program.JsonOptions));
        }
        else
        {
            WriteTable(results, mode, policy, header);
        }

        return exitCode;
    }

    private static void WriteTable(
        List<PdfAnalysisResult> results, OutputMode mode, WordCountPolicy policy, bool header)
    {
        switch (mode)
        {
            case OutputMode.Text:
                foreach (PdfAnalysisResult result in results) Console.Write(result.Text);
                return;

            case OutputMode.Pages:
                if (header) Console.WriteLine("file\tpage\twidthPt\theightPt\tmediaWidthPt\tmediaHeightPt\trotation");
                foreach (PdfAnalysisResult result in results)
                {
                    foreach (PdfPageInfo page in result.Pages)
                    {
                        Console.WriteLine(string.Join('\t',
                            result.File, page.Number,
                            Points(page.WidthPoints), Points(page.HeightPoints),
                            Points(page.MediaWidthPoints), Points(page.MediaHeightPoints),
                            Number(page.Rotation)));
                    }
                }
                return;

            case OutputMode.Fonts:
                if (header) Console.WriteLine("file\tname\ttype\tembedded\tsubset");
                foreach (PdfAnalysisResult result in results)
                {
                    foreach (PdfFontInfo font in result.Fonts)
                    {
                        Console.WriteLine(string.Join('\t',
                            result.File, font.Name, font.Type, YesNo(font.Embedded), YesNo(font.Subset)));
                    }
                }
                return;

            case OutputMode.Document:
            default:
                // Column order is a contract with the shell scripts that consume it. Append, never
                // reorder: batch-check.sh reads fields by index, and a mis-aligned index is exactly
                // the failure that once reported "534 of 534 documents changed" — the tell being
                // that the total equalled the corpus page count.
                if (header)
                {
                    Console.WriteLine("file\tpages\twords\twordsRaw\twordsAlnum\tbullets\tsymbols\tpunct"
                                      + "\tfonts\tunembedded\tsubset\tsizes\twidthPt\theightPt\trotation\terror");
                }

                foreach (PdfAnalysisResult result in results)
                {
                    int subset = 0;
                    foreach (PdfFontInfo font in result.Fonts) if (font.Subset) subset++;

                    PdfPageInfo? first = result.Pages.Count > 0 ? result.Pages[0] : null;

                    Console.WriteLine(string.Join('\t',
                        result.File,
                        Number(result.PageCount),
                        Number(result.WordCount(policy)),
                        Number(result.Words.Raw),
                        Number(result.Words.Alphanumeric),
                        Number(result.Words.Bullet),
                        Number(result.Words.PrivateUse),
                        Number(result.Words.Punctuation),
                        Number(result.Fonts.Count),
                        Number(result.UnembeddedFontCount),
                        Number(subset),
                        Number(result.DistinctPageSizeCount),
                        first is null ? "-" : Points(first.WidthPoints),
                        first is null ? "-" : Points(first.HeightPoints),
                        first is null ? "-" : Number(first.Rotation),
                        result.Error ?? ""));
                }

                return;
        }
    }

    // Invariant culture, explicitly, on every number that reaches the output. The solution sets
    // InvariantGlobalization=false, so CurrentCulture is whatever the machine's locale says — and a
    // locale that writes 595,304 for a page width turns a TSV into something awk reads as two
    // fields. The figures this tool prints are compared across machines; they are data, not prose.
    private static string Points(double value) => value.ToString("0.###", CultureInfo.InvariantCulture);

    private static string Number(int value) => value.ToString(CultureInfo.InvariantCulture);

    private static string YesNo(bool value) => value ? "yes" : "no";

    internal static void PrintUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            Usage: xray-office analyze [options] FILE...

            Reads each PDF in process and reports what a fidelity comparison needs: page count and
            page size, extractable words, and font embedding. Replaces pdfinfo, pdftotext and
            pdffonts, so the figures move with this repository rather than with the machine's
            poppler.

            Output is TSV with a header, one row per document, unless a mode below says otherwise.

            Options:
              --json          Emit JSON with every page, font and count
              --pages         TSV with one row per page instead of per document
              --fonts         TSV with one row per font face instead of per document
              --text          Write the extracted text, as `pdftotext FILE -` would
              --grouping HOW  How glyphs are grouped into words:
                                nearest  nearest-neighbour, orientation aware (default)
                                simple   line-then-gap; no concept of rotated text
              --words POLICY  Which tokens the `words` column counts:
                                alnum  tokens holding a Unicode letter or digit (default)
                                raw    every whitespace-delimited token
              --no-header     Omit the TSV header line
              -h, --help      Print this message

            The `words` column follows --words; `wordsRaw` and `wordsAlnum` are always both present,
            along with the counts of the tokens the two differ by (bullets, symbols, punct), so a
            comparison can show what a difference is made of rather than only how big it is. The
            default matches the corpus gate's `words_of()`: a token is a word iff it carries at
            least one Unicode letter or digit.

            Word boundaries are inferred from glyph geometry, here as in every PDF text extractor.
            Two extractors will disagree; the point of this one is that it is pinned in the
            repository and changes only when the code does.

            Exit codes:
              0   every file was read
              1   a file could not be read
              2   bad usage
            """);
    }
}
