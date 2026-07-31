using System.Text.Json;
using System.Text.Json.Serialization;
using Paperless.Containers;
using Paperless.Core.Formats;

namespace Paperless.Cli;

/// <summary>
/// The <c>paperless</c> command-line tool.
/// </summary>
/// <remarks>
/// Serves two audiences: humans inspecting documents, and the fidelity-comparison skills in
/// <c>.claude/skills/</c>, which use it as the Paperless side of a side-by-side against
/// headless LibreOffice.
/// <para>
/// Argument parsing is hand-rolled rather than using System.CommandLine. The surface is a
/// handful of subcommands with two or three options each, and hand-rolling keeps the tool
/// dependency-free — worth more here than the features a parser library would add.
/// </para>
/// </remarks>
internal static class Program
{
    internal const int ExitSuccess = 0;
    internal const int ExitFailure = 1;
    internal const int ExitUsage = 2;

    /// <summary>Exit code 65, <c>EX_DATAERR</c>: the input was not a supported document.</summary>
    internal const int ExitUnsupportedFormat = 65;

    private static int Main(string[] args)
    {
        if (args.Length == 0 || IsHelp(args[0])) { PrintUsage(); return args.Length == 0 ? ExitUsage : ExitSuccess; }

        string command = args[0];
        string[] rest = args[1..];

        return command switch
        {
            "identify" => Identify(rest),
            "extract" => ExtractCommand.Extract(rest),
            "metadata" => ExtractCommand.Metadata(rest),
            "version" => PrintVersion(),
            _ => Unknown(command),
        };

        static bool IsHelp(string a) => a is "-h" or "--help" or "help";

        static int Unknown(string command)
        {
            Console.Error.WriteLine($"paperless: unknown command '{command}'.");
            PrintUsage(Console.Error);
            return ExitUsage;
        }
    }

    // ------------------------------------------------------------------------- identify

    private static int Identify(string[] args)
    {
        bool json = false;
        List<string> paths = [];
        foreach (string arg in args)
        {
            switch (arg)
            {
                case "--json": json = true; break;
                case "-h" or "--help": PrintIdentifyUsage(); return ExitSuccess;
                default:
                    if (arg.StartsWith('-'))
                    {
                        Console.Error.WriteLine($"paperless identify: unknown option '{arg}'.");
                        return ExitUsage;
                    }
                    paths.Add(arg);
                    break;
            }
        }

        if (paths.Count == 0)
        {
            Console.Error.WriteLine("paperless identify: no files given.");
            PrintIdentifyUsage(Console.Error);
            return ExitUsage;
        }

        List<IdentifyResult> results = [];
        int exitCode = ExitSuccess;

        foreach (string path in paths)
        {
            IdentifyResult result = IdentifyOne(path);
            results.Add(result);

            // Report the strongest problem across all inputs, so a batch run's exit code
            // still means something.
            if (result.Error is not null) exitCode = ExitFailure;
            else if (result.Format == nameof(DocumentFormat.Unknown) && exitCode == ExitSuccess)
                exitCode = ExitUnsupportedFormat;
        }

        if (json)
        {
            Console.WriteLine(JsonSerializer.Serialize(results, JsonOptions));
        }
        else
        {
            bool multiple = results.Count > 1;
            foreach (IdentifyResult r in results) PrintHuman(r, multiple);
        }
        return exitCode;
    }

    private static IdentifyResult IdentifyOne(string path)
    {
        if (!File.Exists(path))
        {
            return new IdentifyResult
            {
                File = path,
                Format = nameof(DocumentFormat.Unknown),
                Error = "File not found.",
            };
        }

        try
        {
            using FileStream stream = File.OpenRead(path);
            FormatIdentification id = FormatIdentifier.Instance.Identify(stream, Path.GetFileName(path));
            FormatInfo? info = FormatCatalogue.Instance.GetInfo(id.Format);

            return new IdentifyResult
            {
                File = path,
                Format = id.Format.ToString(),
                Family = info?.Family.ToString(),
                Container = id.Container.ToString(),
                Confidence = id.Confidence.ToString(),
                DisplayName = info?.DisplayName,
                MediaType = info?.MediaType,
                IsEncrypted = id.IsEncrypted,
                IsTemplate = info?.IsTemplate,
                CanCarryMacros = info?.CanCarryMacros,
                IsReadSupported = info?.IsReadSupported ?? false,
                Detail = id.Detail,
                SizeBytes = new FileInfo(path).Length,
            };
        }
        catch (IOException ex)
        {
            return new IdentifyResult { File = path, Format = nameof(DocumentFormat.Unknown), Error = ex.Message };
        }
        catch (UnauthorizedAccessException ex)
        {
            return new IdentifyResult { File = path, Format = nameof(DocumentFormat.Unknown), Error = ex.Message };
        }
    }

    private static void PrintHuman(IdentifyResult r, bool showFileName)
    {
        if (showFileName) Console.WriteLine($"{r.File}:");
        string indent = showFileName ? "  " : "";

        if (r.Error is not null)
        {
            Console.WriteLine($"{indent}error       {r.Error}");
            return;
        }

        Console.WriteLine($"{indent}format      {r.Format}"
                          + (r.DisplayName is null ? "" : $"  ({r.DisplayName})"));
        if (r.Family is not null) Console.WriteLine($"{indent}family      {r.Family}");
        Console.WriteLine($"{indent}container   {r.Container}");
        Console.WriteLine($"{indent}confidence  {r.Confidence}");
        if (r.MediaType is not null) Console.WriteLine($"{indent}media type  {r.MediaType}");
        Console.WriteLine($"{indent}size        {r.SizeBytes:N0} bytes");

        // Only mention the flags that are true; a wall of "no" is noise.
        List<string> flags = [];
        if (r.IsEncrypted) flags.Add("encrypted (a password is needed to read it)");
        if (r.IsTemplate == true) flags.Add("template");
        if (r.CanCarryMacros == true) flags.Add("can carry macros (Paperless never executes them)");
        if (flags.Count > 0) Console.WriteLine($"{indent}flags       {string.Join("; ", flags)}");

        Console.WriteLine($"{indent}readable    {(r.IsReadSupported ? "yes" : "not yet implemented")}");
        if (r.Detail is not null) Console.WriteLine($"{indent}basis       {r.Detail}");
        if (showFileName) Console.WriteLine();
    }

    // -------------------------------------------------------------------------- plumbing

    private static int PrintVersion()
    {
        string version = typeof(Program).Assembly.GetName().Version?.ToString() ?? "unknown";
        Console.WriteLine($"paperless {version}");
        return ExitSuccess;
    }

    private static void PrintUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            paperless - content extraction and rendering for office documents

            Usage:
              paperless <command> [options] [files...]

            Commands:
              identify FILE...   Report each file's real format, determined from its content
              extract  FILE...   Extract text, tables and structure
              metadata FILE...   Report the document's metadata
              version            Print the version
              help               Print this message

            Not yet implemented: render, convert, diagnose.
            See dotnet/TODO.md for the plan.
            """);
    }

    private static void PrintIdentifyUsage(TextWriter? writer = null)
    {
        writer ??= Console.Out;
        writer.WriteLine("""
            Usage: paperless identify [--json] FILE...

            Determines each file's format from its content, not its extension.

            Options:
              --json    Emit machine-readable JSON instead of text

            Exit codes:
              0   every file was identified
              1   a file could not be read
              2   bad usage
              65  a file's format was not recognised
            """);
    }

    internal static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        // camelCase: the conventional shape for a machine-readable CLI contract, and what
        // the comparison scripts expect.
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
    };

    /// <summary>
    /// One file's identification result.
    /// </summary>
    /// <remarks>
    /// This is the JSON schema the comparison harness parses, so treat the property names as
    /// public API: renaming one breaks the scripts in <c>.claude/skills/</c>. Serialised as
    /// camelCase, so <c>SizeBytes</c> appears as <c>sizeBytes</c>.
    /// </remarks>
    private sealed record IdentifyResult
    {
        public required string File { get; init; }
        public required string Format { get; init; }
        public string? Family { get; init; }
        public string? Container { get; init; }
        public string? Confidence { get; init; }
        public string? DisplayName { get; init; }
        public string? MediaType { get; init; }
        public bool IsEncrypted { get; init; }
        public bool? IsTemplate { get; init; }
        public bool? CanCarryMacros { get; init; }
        public bool IsReadSupported { get; init; }
        public string? Detail { get; init; }
        public long SizeBytes { get; init; }
        public string? Error { get; init; }
    }
}
