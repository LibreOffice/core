using System.Globalization;
using System.Text;
using System.Text.Json;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;

namespace Paperless.Cli;

/// <summary>
/// The <c>extract</c> and <c>metadata</c> subcommands.
/// </summary>
/// <remarks>
/// These are the Paperless side of the <c>extraction-comparison</c> skill, so the output
/// layout is part of its contract: with <c>--outdir</c>, each input produces
/// <c>&lt;stem&gt;.txt</c> in that directory, which is what the skill's
/// <c>compare-text.py</c> is pointed at.
/// </remarks>
internal static class ExtractCommand
{
    /// <summary>Runs <c>paperless extract</c>.</summary>
    public static int Extract(string[] args)
    {
        if (!Options.TryParse(args, out Options options, out int usageExit)) return usageExit;
        if (options.WantsHelp) { PrintExtractUsage(); return Program.ExitSuccess; }
        if (options.Paths.Count == 0)
        {
            Console.Error.WriteLine("paperless extract: no files given.");
            PrintExtractUsage(Console.Error);
            return Program.ExitUsage;
        }

        int exitCode = Program.ExitSuccess;
        foreach (string path in options.Paths)
        {
            try
            {
                using IDocument document = PaperlessDocument.Open(path);
                string output = options.Json
                    ? JsonSerializer.Serialize(Describe(document), Program.JsonOptions)
                    : document.Content.GetText();

                Write(options, path, options.Json ? "json" : "txt", output);
                ReportDiagnostics(options, path, document.Diagnostics);
            }
            catch (Exception ex) when (IsExpected(ex))
            {
                Console.Error.WriteLine($"paperless extract: {path}: {ex.Message}");
                exitCode = ExitCodeFor(ex, exitCode);
            }
        }
        return exitCode;
    }

    /// <summary>Runs <c>paperless metadata</c>.</summary>
    public static int Metadata(string[] args)
    {
        if (!Options.TryParse(args, out Options options, out int usageExit)) return usageExit;
        if (options.WantsHelp) { PrintMetadataUsage(); return Program.ExitSuccess; }
        if (options.Paths.Count == 0)
        {
            Console.Error.WriteLine("paperless metadata: no files given.");
            PrintMetadataUsage(Console.Error);
            return Program.ExitUsage;
        }

        int exitCode = Program.ExitSuccess;
        bool multiple = options.Paths.Count > 1;

        foreach (string path in options.Paths)
        {
            try
            {
                using IDocument document = PaperlessDocument.Open(path);
                string output = options.Json
                    ? JsonSerializer.Serialize(document.Metadata, Program.JsonOptions)
                    : FormatMetadata(document, multiple ? path : null);

                Write(options, path, options.Json ? "json" : "txt", output);
            }
            catch (Exception ex) when (IsExpected(ex))
            {
                Console.Error.WriteLine($"paperless metadata: {path}: {ex.Message}");
                exitCode = ExitCodeFor(ex, exitCode);
            }
        }
        return exitCode;
    }

    private static void Write(Options options, string inputPath, string extension, string content)
    {
        if (options.OutputDirectory is null)
        {
            Console.Out.Write(content);
            if (!content.EndsWith('\n')) Console.Out.WriteLine();
            return;
        }

        Directory.CreateDirectory(options.OutputDirectory);
        string stem = Path.GetFileNameWithoutExtension(inputPath);
        string target = Path.Combine(options.OutputDirectory, $"{stem}.{extension}");
        File.WriteAllText(target, content, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
        Console.WriteLine($"{inputPath} -> {target} ({content.Length:N0} chars)");
    }

    private static void ReportDiagnostics(Options options, string path, IReadOnlyList<Diagnostic> diagnostics)
    {
        // Diagnostics go to stderr so they never contaminate extracted text that a comparison
        // script is reading from stdout.
        if (!options.ShowDiagnostics || diagnostics.Count == 0) return;
        Console.Error.WriteLine($"{path}: {diagnostics.Count} diagnostic(s)");
        foreach (Diagnostic diagnostic in diagnostics) Console.Error.WriteLine($"  {diagnostic}");
    }

    private static string FormatMetadata(IDocument document, string? path)
    {
        StringBuilder text = new();
        if (path is not null) text.AppendLine(CultureInfo.InvariantCulture, $"{path}:");

        DocumentMetadata metadata = document.Metadata;
        Field("format", document.Format.ToString());
        Field("family", document.Family.ToString());
        Field("title", metadata.Title);
        Field("subject", metadata.Subject);
        Field("author", metadata.Author);
        Field("last saved by", metadata.LastModifiedBy);
        Field("description", metadata.Description);
        Field("keywords", metadata.Keywords.Count == 0 ? null : string.Join(", ", metadata.Keywords));
        Field("language", metadata.Language);
        Field("created", metadata.Created?.ToString("u", CultureInfo.InvariantCulture));
        Field("modified", metadata.Modified?.ToString("u", CultureInfo.InvariantCulture));
        Field("printed", metadata.Printed?.ToString("u", CultureInfo.InvariantCulture));
        Field("generator", metadata.GeneratorApplication);
        Field("revision", metadata.RevisionNumber?.ToString(CultureInfo.InvariantCulture));
        Field("editing time", metadata.TotalEditingTime?.ToString());

        if (metadata.Statistics is { } statistics)
        {
            Field("pages", statistics.PageCount?.ToString(CultureInfo.InvariantCulture));
            Field("sheets", statistics.SheetCount?.ToString(CultureInfo.InvariantCulture));
            Field("slides", statistics.SlideCount?.ToString(CultureInfo.InvariantCulture));
            Field("words", statistics.WordCount?.ToString(CultureInfo.InvariantCulture));
            Field("characters", statistics.CharacterCount?.ToString(CultureInfo.InvariantCulture));
            Field("paragraphs", statistics.ParagraphCount?.ToString(CultureInfo.InvariantCulture));
            Field("tables", statistics.TableCount?.ToString(CultureInfo.InvariantCulture));
            Field("images", statistics.ImageCount?.ToString(CultureInfo.InvariantCulture));
        }

        foreach ((string name, object? value) in metadata.CustomProperties)
            Field($"custom:{name}", value?.ToString());

        return text.ToString();

        // Absent fields are omitted rather than printed empty: "not recorded" and "recorded
        // as empty" are different, and a wall of blanks hides the fields that do have values.
        void Field(string label, string? value)
        {
            // Padded, then always at least one space: a custom property's name can exceed the
            // column width, and without the space the label would run into the value.
            if (!string.IsNullOrEmpty(value))
                text.AppendLine(CultureInfo.InvariantCulture, $"{label.PadRight(13)} {value}");
        }
    }

    /// <summary>
    /// A structural summary of the extracted content, for <c>extract --json</c>.
    /// </summary>
    /// <remarks>
    /// Deliberately a summary rather than the whole tree: the point is to see at a glance what
    /// was found — how many sections, of what kind, with which headings and tables — and a
    /// full serialisation of every run would bury that.
    /// </remarks>
    private static ExtractResult Describe(IDocument document)
    {
        List<SectionResult> sections = [];
        foreach (ContentNode node in document.Content.Children)
        {
            if (node is not ContentSection section) continue;

            int paragraphs = 0, tables = 0, images = 0;
            List<string> headings = [];
            Count(section);

            sections.Add(new SectionResult
            {
                Kind = section.Kind.ToString(),
                Index = section.Index,
                Name = section.Name,
                IsHidden = section.IsHidden ? true : null,
                Paragraphs = paragraphs,
                Tables = tables,
                Images = images,
                Headings = headings,
                Characters = section.GetText().Length,
            });

            void Count(ContentNode parent)
            {
                foreach (ContentNode child in parent.Children)
                {
                    switch (child)
                    {
                        case ContentParagraph paragraph:
                            paragraphs++;
                            if (paragraph.HeadingLevel is not null)
                                headings.Add(paragraph.GetText().TrimEnd('\n'));
                            break;
                        case ContentTable:
                            tables++;
                            break;
                        case ContentImage:
                            images++;
                            break;
                    }
                    Count(child);
                }
            }
        }

        return new ExtractResult
        {
            Format = document.Format.ToString(),
            Family = document.Family.ToString(),
            Characters = document.Content.GetText().Length,
            Sections = sections,
            Diagnostics = [.. document.Diagnostics.Select(d => d.ToString())],
        };
    }

    private static bool IsExpected(Exception ex)
        => ex is PaperlessException or IOException or UnauthorizedAccessException;

    private static int ExitCodeFor(Exception ex, int current)
        => ex is UnsupportedFormatException ? Program.ExitUnsupportedFormat
           : current == Program.ExitSuccess || current == Program.ExitUnsupportedFormat
               ? Program.ExitFailure
               : current;

    private static void PrintExtractUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            Usage: paperless extract [--outdir DIR] [--json] [--diagnostics] FILE...

            Extracts each document's text, tables and structure without laying it out.

            Options:
              --outdir DIR    Write <stem>.txt per input instead of printing to stdout
              --json          Emit a structural summary as JSON instead of plain text
              --diagnostics   Report non-fatal problems on stderr

            Exit codes:
              0   every file was extracted
              1   a file could not be read
              2   bad usage
              65  a file's format is not supported
            """);
    }

    private static void PrintMetadataUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            Usage: paperless metadata [--outdir DIR] [--json] FILE...

            Reports each document's metadata. Fields the file does not record are omitted,
            so that "not recorded" stays distinguishable from "recorded as empty".

            Options:
              --outdir DIR    Write <stem>.txt per input instead of printing to stdout
              --json          Emit JSON instead of text
            """);
    }

    /// <summary>The options both subcommands accept.</summary>
    private readonly record struct Options(
        List<string> Paths, string? OutputDirectory, bool Json, bool ShowDiagnostics, bool WantsHelp)
    {
        public static bool TryParse(string[] args, out Options options, out int usageExit)
        {
            List<string> paths = [];
            string? outputDirectory = null;
            bool json = false, diagnostics = false, help = false;
            usageExit = Program.ExitUsage;

            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i])
                {
                    case "--json": json = true; break;
                    case "--diagnostics": diagnostics = true; break;
                    case "-h" or "--help": help = true; break;
                    case "--outdir":
                        if (i + 1 >= args.Length)
                        {
                            Console.Error.WriteLine("paperless: --outdir needs a directory.");
                            options = default;
                            return false;
                        }
                        outputDirectory = args[++i];
                        break;
                    default:
                        if (args[i].StartsWith('-'))
                        {
                            Console.Error.WriteLine($"paperless: unknown option '{args[i]}'.");
                            options = default;
                            return false;
                        }
                        paths.Add(args[i]);
                        break;
                }
            }

            options = new Options(paths, outputDirectory, json, diagnostics, help);
            return true;
        }
    }

    private sealed record ExtractResult
    {
        public required string Format { get; init; }
        public required string Family { get; init; }
        public int Characters { get; init; }
        public IReadOnlyList<SectionResult> Sections { get; init; } = [];
        public IReadOnlyList<string> Diagnostics { get; init; } = [];
    }

    private sealed record SectionResult
    {
        public required string Kind { get; init; }
        public int Index { get; init; }
        public string? Name { get; init; }
        public bool? IsHidden { get; init; }
        public int Paragraphs { get; init; }
        public int Tables { get; init; }
        public int Images { get; init; }
        public int Characters { get; init; }
        public IReadOnlyList<string> Headings { get; init; } = [];
    }
}
