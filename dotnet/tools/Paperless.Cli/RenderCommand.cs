using System.Globalization;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.Rendering.Raster;

namespace Paperless.Cli;

/// <summary>
/// The <c>render</c> subcommand: lay a document out and write it as PDF, PNG or JPEG.
/// </summary>
/// <remarks>
/// <para>
/// The Paperless side of the <c>render-comparison</c> skill, so its output layout is part
/// of that skill's contract. The skill's own example is
/// <c>paperless render --dpi 150 --outdir /tmp/actual document.docx</c> followed by a
/// comparison pointed straight at <c>/tmp/actual</c>, so a single input writes
/// <c>page-1.png</c>, <c>page-2.png</c> … into the directory it was given — the same names
/// <c>pdftoppm</c> gives the reference. Several inputs would collide, so each gets a
/// subdirectory named after the file, which is what <c>lo-convert.sh</c> does with the
/// reference side.
/// </para>
/// <para>
/// PNG is the default raster format and the only one worth comparing. JPEG is offered for
/// thumbnails and says so in the usage text, because a JPEG page image silently turns a
/// fidelity question into a question about quantisation.
/// </para>
/// </remarks>
internal static class RenderCommand
{
    /// <summary>Runs <c>paperless render</c>.</summary>
    public static int Render(string[] args)
    {
        if (!Options.TryParse(args, out Options options, out int usageExit)) return usageExit;
        if (options.WantsHelp) { PrintUsage(); return Program.ExitSuccess; }

        if (options.Paths.Count == 0)
        {
            Console.Error.WriteLine("paperless render: no files given.");
            PrintUsage(Console.Error);
            return Program.ExitUsage;
        }

        int exitCode = Program.ExitSuccess;
        foreach (string path in options.Paths)
        {
            try
            {
                exitCode = Math.Max(exitCode, RenderOne(path, options));
            }
            catch (Exception ex) when (ex is IOException or PaperlessException or NotSupportedException
                                          or InvalidOperationException or UnauthorizedAccessException)
            {
                Console.Error.WriteLine($"paperless render: {path}: {ex.Message}");
                exitCode = Program.ExitFailure;
            }
        }

        return exitCode;
    }

    private static int RenderOne(string path, Options options)
    {
        using IDocument document = PaperlessDocument.Open(path);

        if (document is not IPaginatedDocument paginated)
        {
            Console.Error.WriteLine(
                $"paperless render: {path}: {document.Format} can be read but not yet laid out.");
            return Program.ExitUnsupportedFormat;
        }

        IPageSequence pages = paginated.Layout();
        List<IPage> chosen = Select(pages, options.Pages);

        if (chosen.Count == 0)
        {
            Console.Error.WriteLine(
                $"paperless render: {path}: the page range selects none of the {pages.Count} pages.");
            return Program.ExitFailure;
        }

        string stem = Path.GetFileNameWithoutExtension(path);
        string directory = options.Several
            ? Path.Combine(options.OutputDirectory, Path.GetFileName(path))
            : options.OutputDirectory;

        Directory.CreateDirectory(directory);

        if (options.Format == "pdf")
        {
            string file = Path.Combine(directory, stem + ".pdf");
            using FileStream output = File.Create(file);
            new PdfRenderer(new PdfRenderOptions { CreationDate = PinnedDate() })
                .Render(chosen, output);

            if (!options.Quiet) Console.WriteLine($"{file}: {chosen.Count} page(s)");
            return Program.ExitSuccess;
        }

        RasterRenderer renderer = new(new RasterRenderOptions { Dpi = options.Dpi });
        RasterFormat format = options.Format == "jpeg" ? RasterFormat.Jpeg : RasterFormat.Png;
        string extension = format == RasterFormat.Jpeg ? ".jpg" : ".png";

        foreach (IPage page in chosen)
        {
            string file = Path.Combine(
                directory,
                string.Create(CultureInfo.InvariantCulture, $"page-{page.Index + 1}{extension}"));

            using FileStream output = File.Create(file);
            renderer.Render(page, output, format);
        }

        if (!options.Quiet)
        {
            Console.WriteLine(
                $"{directory}: {chosen.Count} page(s) at {options.Dpi.ToString(CultureInfo.InvariantCulture)} dpi");
        }

        return Program.ExitSuccess;
    }

    /// <summary>
    /// The creation date a PDF should carry, or null to use the clock.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SOURCE_DATE_EPOCH</c>, the reproducible-builds convention: seconds since the Unix
    /// epoch, UTC. Setting it makes two renderings of one document byte-equal, which is what
    /// the corpus workflow needs — reach is measured by rendering a track twice and diffing,
    /// and a timestamp that moves between the runs puts a false-positive floor under every
    /// such measurement. The layout side honours the same variable for the <c>&amp;D</c> and
    /// <c>&amp;T</c> header fields, so pinning it pins the whole file rather than half of it.
    /// </para>
    /// <para>
    /// Unset — the ordinary case — leaves <c>PdfRenderOptions.CreationDate</c> null and the
    /// writer stamps the current time, which is what a PDF is supposed to carry.
    /// </para>
    /// </remarks>
    private static DateTimeOffset? PinnedDate()
    {
        string? raw = Environment.GetEnvironmentVariable("SOURCE_DATE_EPOCH");
        if (string.IsNullOrWhiteSpace(raw)) return null;

        return long.TryParse(raw.Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture,
                             out long seconds)
               && seconds >= 0 && seconds <= 253402300799L
            ? DateTimeOffset.FromUnixTimeSeconds(seconds).ToLocalTime()
            : null;
    }

    /// <summary>The pages a range names, in page order and without repeats.</summary>
    private static List<IPage> Select(IPageSequence pages, string? range)
    {
        List<IPage> chosen = [];
        if (string.IsNullOrWhiteSpace(range))
        {
            for (int i = 0; i < pages.Count; i++) chosen.Add(pages[i]);
            return chosen;
        }

        SortedSet<int> wanted = [];
        foreach (string part in range.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            int dash = part.IndexOf('-', StringComparison.Ordinal);
            if (dash < 0)
            {
                if (int.TryParse(part, CultureInfo.InvariantCulture, out int one)) wanted.Add(one);
                continue;
            }

            string fromText = part[..dash].Trim();
            string toText = part[(dash + 1)..].Trim();

            int from = fromText.Length == 0 ? 1 : int.Parse(fromText, CultureInfo.InvariantCulture);
            int to = toText.Length == 0 ? pages.Count : int.Parse(toText, CultureInfo.InvariantCulture);

            for (int page = from; page <= to; page++) wanted.Add(page);
        }

        foreach (int page in wanted)
        {
            if (page >= 1 && page <= pages.Count) chosen.Add(pages[page - 1]);
        }

        return chosen;
    }

    private static void PrintUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            Usage: paperless render [options] FILE...

            Lays a document out and writes its pages.

            Options:
              --format FMT   pdf (default), png or jpeg
              --outdir DIR   Where to write. Defaults to the current directory
              --pages RANGE  Which pages, as 1-based numbers: "2", "1-3", "1-3,7", "5-"
              --dpi N        Resolution for png and jpeg. Default 150
              --quiet        Say nothing on success

            Output:
              pdf            DIR/<stem>.pdf
              png, jpeg      DIR/page-1.png, page-2.png ... for one input;
                             DIR/<file>/page-1.png ... when several files are given

            PNG is lossless and deterministic, so a page image can be checksummed and
            compared. JPEG is for thumbnails; do not compare one against anything.

            Exit codes:
              0   every file rendered
              1   a file could not be read or laid out
              2   bad usage
              65  a file's format is not laid out yet
            """);
    }

    /// <summary>The parsed command line.</summary>
    private sealed class Options
    {
        public List<string> Paths { get; } = [];

        public string Format { get; private set; } = "pdf";

        public string OutputDirectory { get; private set; } = ".";

        public string? Pages { get; private set; }

        public double Dpi { get; private set; } = 150;

        public bool Quiet { get; private set; }

        public bool WantsHelp { get; private set; }

        public bool Several => Paths.Count > 1;

        public static bool TryParse(string[] args, out Options options, out int usageExit)
        {
            options = new Options();
            usageExit = Program.ExitUsage;

            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                switch (arg)
                {
                    case "-h" or "--help":
                        options.WantsHelp = true;
                        return true;

                    case "--quiet":
                        options.Quiet = true;
                        break;

                    case "--format":
                        if (!Next(args, ref i, arg, out string? format)) return false;
                        if (format is not ("pdf" or "png" or "jpeg" or "jpg"))
                        {
                            Console.Error.WriteLine($"paperless render: unknown format '{format}'.");
                            return false;
                        }

                        options.Format = format == "jpg" ? "jpeg" : format;
                        break;

                    case "--outdir":
                        if (!Next(args, ref i, arg, out string? outdir)) return false;
                        options.OutputDirectory = outdir;
                        break;

                    case "--pages":
                        if (!Next(args, ref i, arg, out string? pages)) return false;
                        options.Pages = pages;
                        break;

                    case "--dpi":
                        if (!Next(args, ref i, arg, out string? dpi)) return false;
                        if (!double.TryParse(dpi, CultureInfo.InvariantCulture, out double value)
                            || value <= 0 || value > 2400)
                        {
                            Console.Error.WriteLine($"paperless render: '{dpi}' is not a usable resolution.");
                            return false;
                        }

                        options.Dpi = value;
                        break;

                    default:
                        if (arg.StartsWith('-'))
                        {
                            Console.Error.WriteLine($"paperless render: unknown option '{arg}'.");
                            return false;
                        }

                        options.Paths.Add(arg);
                        break;
                }
            }

            return true;
        }

        private static bool Next(string[] args, ref int i, string option, out string value)
        {
            if (i + 1 >= args.Length)
            {
                Console.Error.WriteLine($"paperless render: {option} needs a value.");
                value = string.Empty;
                return false;
            }

            value = args[++i];
            return true;
        }
    }
}
