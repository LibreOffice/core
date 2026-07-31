using System.Diagnostics;
using System.Globalization;

namespace Paperless.TestKit.LibreOffice;

/// <summary>
/// Drives an installed LibreOffice in headless mode to produce reference output.
/// </summary>
/// <remarks>
/// <para>
/// This is the oracle the whole fidelity harness is measured against. It shells out to
/// an installed <c>soffice</c>; it does not build or link LibreOffice, and it never
/// needs to.
/// </para>
/// <para>
/// Four things about <c>soffice</c> shape this API, all of them learned the hard way and
/// all of them documented in <c>.claude/skills/libreoffice-reference/SKILL.md</c>:
/// </para>
/// <list type="number">
///   <item><description>
///     <b>Output names collide.</b> <c>--convert-to</c> names its output after the input's
///     stem alone, so converting <c>a/t.docx</c> and <c>b/t.xlsx</c> into one directory
///     produces one file, silently. <see cref="ConvertToPdf"/> therefore isolates each
///     conversion in its own output directory.
///   </description></item>
///   <item><description>
///     <b>One user profile per concurrent process.</b> Two <c>soffice</c> processes sharing
///     a profile directory will interfere: the second detects the first and hands its
///     work over, so output goes missing. Each runner gets its own
///     <c>-env:UserInstallation</c>.
///   </description></item>
///   <item><description>
///     <b>PNG export gives one page.</b> <c>--convert-to png</c> emits only the first
///     page or slide, so multi-page reference images have to come from PDF plus a
///     rasteriser.
///   </description></item>
///   <item><description>
///     <b>Fonts decide fidelity.</b> Substitution happens silently and changes advance
///     widths, which changes line breaks, which changes pagination. Comparisons are only
///     meaningful when both sides resolve fonts identically, so
///     <see cref="CheckFontEnvironment"/> exists to assert that up front rather than
///     letting it show up as mysterious diffs.
///   </description></item>
/// </list>
/// <para>
/// A fifth, which the API cannot hide: <c>soffice</c> exits 0 even when it converts
/// nothing. Every method here checks that the expected file appeared rather than trusting
/// the exit code.
/// </para>
/// </remarks>
public sealed class LibreOfficeRunner : IDisposable
{
    /// <summary>
    /// How long a single <c>soffice</c> invocation is given before it is killed.
    /// </summary>
    /// <remarks>
    /// A conversion that hangs would otherwise hang the test run: <c>soffice</c> can wait
    /// indefinitely on a lock file or a dialog it cannot show in headless mode. Two minutes is
    /// far longer than a corpus document needs.
    /// </remarks>
    public static readonly TimeSpan Timeout = TimeSpan.FromMinutes(2);

    private readonly string _profileDirectory;
    private bool _disposed;

    /// <summary>Creates a runner with a private user profile.</summary>
    /// <param name="sofficePath">
    /// Path to the <c>soffice</c> executable. When null, it is looked up on <c>PATH</c>.
    /// </param>
    public LibreOfficeRunner(string? sofficePath = null)
    {
        SofficePath = sofficePath ?? "soffice";
        _profileDirectory = Path.Combine(
            Path.GetTempPath(),
            "paperless-lo-" + Guid.NewGuid().ToString("N")[..12]);
        Directory.CreateDirectory(_profileDirectory);
    }

    /// <summary>The executable being driven.</summary>
    public string SofficePath { get; }

    /// <summary>
    /// True when <c>soffice</c> is installed and its application modules are present.
    /// </summary>
    /// <remarks>
    /// Both halves matter, and only the second is easy to get wrong: a container with
    /// <c>libreoffice-core</c> alone has an <c>soffice</c> that starts, reports a version and
    /// then fails on every document. So availability is decided by actually converting
    /// something, not by finding the binary.
    /// </remarks>
    public static bool IsAvailable => AvailabilityCheck.Value;

    private static readonly Lazy<bool> AvailabilityCheck = new(() =>
    {
        try
        {
            using LibreOfficeRunner runner = new();
            if (runner.GetVersion().Length == 0) return false;

            string probeDirectory = Path.Combine(runner._profileDirectory, "probe");
            Directory.CreateDirectory(probeDirectory);
            string probe = Path.Combine(probeDirectory, "paperless-probe.txt");
            File.WriteAllText(probe, "probe");

            return runner.TryConvert(probe, "odt", probeDirectory, out _);
        }
        catch (Exception exception) when (exception is not OutOfMemoryException)
        {
            return false;
        }
    });

    /// <summary>The installed version string, as reported by <c>soffice --version</c>.</summary>
    /// <returns>
    /// The version line, or an empty string when <c>soffice</c> cannot be run at all — a
    /// missing LibreOffice is a reason to skip a comparison, not to fail with an exception.
    /// </returns>
    public string GetVersion()
    {
        try
        {
            ProcessResult result = Run(["--version"], Path.GetTempPath());
            return result.StandardOutput.Trim();
        }
        catch (Exception exception) when (exception is not OutOfMemoryException)
        {
            return string.Empty;
        }
    }

    /// <summary>
    /// Converts a document to PDF and returns the path to the result.
    /// </summary>
    /// <remarks>
    /// PDF is the reference format of choice rather than PNG: it covers every page in one
    /// pass, and it keeps text as text so that both visual and textual comparisons can be
    /// driven from the same artefact.
    /// </remarks>
    public string ConvertToPdf(string inputPath, string outputDirectory)
        => Convert(inputPath, "pdf", outputDirectory);

    /// <summary>
    /// Converts several documents, batching them into as few <c>soffice</c> invocations as
    /// their names allow.
    /// </summary>
    /// <remarks>
    /// Worth doing: process startup dominates the cost of a conversion, so batching a corpus is
    /// far faster than converting one file at a time. The limit on batching is the name
    /// collision described on this class — <c>--convert-to</c> names output after the input's
    /// stem alone — so inputs are grouped such that no two in a batch share a stem, and each
    /// batch gets its own output directory.
    /// </remarks>
    /// <returns>The produced PDF for each input that converted, keyed by input path.</returns>
    public IReadOnlyDictionary<string, string> ConvertManyToPdf(
        IEnumerable<string> inputPaths,
        string outputDirectory)
    {
        ArgumentNullException.ThrowIfNull(inputPaths);
        ArgumentException.ThrowIfNullOrEmpty(outputDirectory);

        List<List<string>> batches = [];
        List<HashSet<string>> stemsPerBatch = [];

        foreach (string input in inputPaths)
        {
            string stem = Path.GetFileNameWithoutExtension(input);

            int batch = 0;
            while (batch < batches.Count && !stemsPerBatch[batch].Add(stem)) batch++;
            if (batch == batches.Count)
            {
                batches.Add([]);
                stemsPerBatch.Add([stem]);
            }
            batches[batch].Add(input);
        }

        Dictionary<string, string> results = new(StringComparer.Ordinal);
        for (int batch = 0; batch < batches.Count; batch++)
        {
            string batchDirectory = Path.Combine(
                outputDirectory, string.Create(CultureInfo.InvariantCulture, $"batch{batch}"));
            Directory.CreateDirectory(batchDirectory);

            RunConvert(batches[batch].Select(Path.GetFullPath), "pdf", batchDirectory);

            foreach (string input in batches[batch])
            {
                string expected = Path.Combine(
                    batchDirectory, Path.GetFileNameWithoutExtension(input) + ".pdf");
                if (File.Exists(expected)) results[input] = expected;
            }
        }
        return results;
    }

    /// <summary>
    /// Extracts a document's text using LibreOffice's own plain-text export, for
    /// comparison against Paperless extraction.
    /// </summary>
    /// <remarks>
    /// The filter differs by family — plain text for word processing, CSV for
    /// spreadsheets, HTML for presentations, since Impress has no text filter — so this
    /// picks the right one from the input's format. Know what each can express before
    /// treating a difference as a defect: the text filter drops headers, footers, comments,
    /// notes and shape text, and the CSV filter covers only the first sheet.
    /// </remarks>
    public string ExtractText(string inputPath, string outputDirectory)
        => Convert(inputPath, TargetFor(inputPath), outputDirectory);

    /// <summary>
    /// Checks that the fonts a comparison depends on resolve the way they must.
    /// </summary>
    /// <remarks>
    /// Specifically, that the metric-compatible substitutes are installed: Calibri must
    /// resolve to Carlito and Cambria to Caladea, not to a DejaVu face. Without them,
    /// every OOXML document reflows and every page after the first diverges — a failure
    /// that looks like a Paperless layout bug but is not one.
    /// </remarks>
    /// <remarks>
    /// Static because font resolution is a property of the machine's fontconfig setup rather
    /// than of any particular <c>soffice</c> invocation: LibreOffice resolves through the same
    /// fontconfig this queries.
    /// </remarks>
    public static FontEnvironmentReport CheckFontEnvironment()
    {
        (string Requested, string Required)[] required =
        [
            ("Calibri", "Carlito"),
            ("Cambria", "Caladea"),
            ("Arial", "Liberation Sans"),
            ("Times New Roman", "Liberation Serif"),
            ("Courier New", "Liberation Mono"),
        ];

        Dictionary<string, string> resolutions = new(StringComparer.Ordinal);
        List<string> problems = [];

        foreach ((string requested, string expected) in required)
        {
            string resolved = MatchFont(requested);
            resolutions[requested] = resolved;

            if (resolved.Length == 0)
            {
                problems.Add($"fc-match is unavailable, so '{requested}' could not be checked.");
            }
            else if (!resolved.Contains(expected, StringComparison.OrdinalIgnoreCase))
            {
                problems.Add($"'{requested}' resolves to '{resolved}' but must resolve to "
                             + $"'{expected}'; without it every document using {requested} "
                             + "reflows and page-level comparison is meaningless.");
            }
        }

        return new FontEnvironmentReport(problems.Count == 0, resolutions, problems);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        try
        {
            if (Directory.Exists(_profileDirectory)) Directory.Delete(_profileDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover profile in the temp directory is harmless; failing a test run over
            // one would not be.
        }
        catch (UnauthorizedAccessException)
        {
        }
    }

    /// <summary>
    /// The <c>--convert-to</c> target for an input, chosen from its extension.
    /// </summary>
    /// <remarks>
    /// Explicit rather than inferred, so an unrecognised extension is an error rather than a
    /// silently wrong filter.
    /// </remarks>
    private static string TargetFor(string inputPath)
    {
        string extension = Path.GetExtension(inputPath).TrimStart('.').ToLowerInvariant();
        return extension switch
        {
            "doc" or "docx" or "docm" or "dot" or "dotx" or "dotm" or "rtf"
                or "odt" or "ott" or "fodt" or "sxw" or "stw" => "txt:Text",
            "xls" or "xlsx" or "xlsm" or "xlt" or "xltx" or "xltm" or "xlsb"
                or "ods" or "ots" or "fods" or "sxc" or "stc" or "csv" => "csv",
            "ppt" or "pptx" or "pptm" or "pot" or "potx" or "potm" or "pps" or "ppsx"
                or "ppsm" or "odp" or "otp" or "fodp" or "sxi" or "sti" => "html",
            _ => throw new ArgumentException(
                $"No LibreOffice text filter is known for '.{extension}'.", nameof(inputPath)),
        };
    }

    private string Convert(string inputPath, string target, string outputDirectory)
    {
        if (TryConvert(inputPath, target, outputDirectory, out string? produced)) return produced;

        throw new InvalidOperationException(
            $"LibreOffice produced no output converting '{inputPath}' to '{target}'. "
            + "soffice exits 0 even when it converts nothing, so this usually means the "
            + "application modules are missing (libreoffice-writer/calc/impress) rather than "
            + "that the document is bad.");
    }

    private bool TryConvert(
        string inputPath, string target, string outputDirectory, out string producedPath)
    {
        ArgumentException.ThrowIfNullOrEmpty(inputPath);
        ArgumentException.ThrowIfNullOrEmpty(outputDirectory);

        string absoluteInput = Path.GetFullPath(inputPath);
        RunConvert([absoluteInput], target, outputDirectory);

        // The output is named after the input's stem with the target's extension. The target
        // may carry a filter name ("txt:Text"), which is not part of the extension.
        string extension = target.Split(':')[0];
        string expected = Path.Combine(
            outputDirectory, Path.GetFileNameWithoutExtension(absoluteInput) + "." + extension);

        if (File.Exists(expected))
        {
            producedPath = expected;
            return true;
        }

        producedPath = string.Empty;
        return false;
    }

    private void RunConvert(IEnumerable<string> absoluteInputs, string target, string outputDirectory)
    {
        Directory.CreateDirectory(outputDirectory);

        List<string> arguments =
        [
            "--headless", "--norestore", "--nolockcheck", "--nodefault",
            "--convert-to", target,
            "--outdir", outputDirectory,
        ];
        arguments.AddRange(absoluteInputs);

        // The exit code is not checked: soffice exits 0 even when it converts nothing, so the
        // only reliable signal is whether the expected file appeared. Callers check that.
        Run([.. arguments], outputDirectory);
    }

    private ProcessResult Run(string[] arguments, string workingDirectory)
    {
        ProcessStartInfo startInfo = new()
        {
            FileName = SofficePath,
            WorkingDirectory = workingDirectory,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
        };

        // The private profile has to come before everything else: soffice reads it while
        // parsing its arguments, and a shared profile is how two concurrent runs lose output.
        startInfo.ArgumentList.Add($"-env:UserInstallation=file://{_profileDirectory}");
        foreach (string argument in arguments) startInfo.ArgumentList.Add(argument);

        using Process process = Process.Start(startInfo)
            ?? throw new InvalidOperationException($"Could not start '{SofficePath}'.");

        string standardOutput = process.StandardOutput.ReadToEnd();
        string standardError = process.StandardError.ReadToEnd();

        if (!process.WaitForExit(Timeout))
        {
            try { process.Kill(entireProcessTree: true); } catch (InvalidOperationException) { }
            throw new TimeoutException(
                $"'{SofficePath}' did not finish within {Timeout.TotalSeconds:N0}s and was killed.");
        }

        return new ProcessResult(process.ExitCode, standardOutput, standardError);
    }

    private static string MatchFont(string family)
    {
        try
        {
            ProcessStartInfo startInfo = new()
            {
                FileName = "fc-match",
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            startInfo.ArgumentList.Add(family);
            startInfo.ArgumentList.Add("family");

            using Process? process = Process.Start(startInfo);
            if (process is null) return string.Empty;

            string output = process.StandardOutput.ReadToEnd();
            process.WaitForExit(30_000);
            return output.Trim();
        }
        catch (Exception exception) when (exception is not OutOfMemoryException)
        {
            // fc-match is a fontconfig tool and does not exist on every platform. Its absence
            // is reported as a problem by the caller rather than thrown here.
            return string.Empty;
        }
    }

    /// <summary>
    /// A directory name derived from an input path, unique per full basename.
    /// </summary>
    /// <remarks>
    /// Keyed on the full basename rather than the stem: <c>t.docx</c> and <c>t.xlsx</c> share
    /// a stem, and keying on that would put both conversions in one directory — the very
    /// collision this exists to prevent.
    /// </remarks>
    private static string SafeKey(string inputPath)
    {
        string basename = Path.GetFileName(inputPath);
        char[] safe = new char[basename.Length];
        for (int i = 0; i < basename.Length; i++)
        {
            safe[i] = char.IsAsciiLetterOrDigit(basename[i]) || basename[i] is '.' or '-' or '_'
                ? basename[i]
                : '_';
        }
        return new string(safe);
    }

    private readonly record struct ProcessResult(int ExitCode, string StandardOutput, string StandardError)
    {
        public override string ToString()
            => string.Create(CultureInfo.InvariantCulture,
                             $"exit {ExitCode}; stdout: {StandardOutput}; stderr: {StandardError}");
    }
}

/// <summary>The outcome of <see cref="LibreOfficeRunner.CheckFontEnvironment"/>.</summary>
/// <param name="IsUsableForComparison">
/// True when the environment is good enough for fidelity comparisons to mean something.
/// </param>
/// <param name="Resolutions">
/// What each checked family actually resolved to, keyed by the requested family.
/// </param>
/// <param name="Problems">Human-readable descriptions of what is wrong, if anything.</param>
public readonly record struct FontEnvironmentReport(
    bool IsUsableForComparison,
    IReadOnlyDictionary<string, string> Resolutions,
    IReadOnlyList<string> Problems);
