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
/// </remarks>
public sealed class LibreOfficeRunner : IDisposable
{
    /// <summary>Creates a runner with a private user profile.</summary>
    /// <param name="sofficePath">
    /// Path to the <c>soffice</c> executable. When null, it is looked up on <c>PATH</c>.
    /// </param>
    public LibreOfficeRunner(string? sofficePath = null) => SofficePath = sofficePath ?? "soffice";

    /// <summary>The executable being driven.</summary>
    public string SofficePath { get; }

    /// <summary>The installed version string, as reported by <c>soffice --version</c>.</summary>
    public string GetVersion() => throw new NotImplementedException();

    /// <summary>
    /// Converts a document to PDF and returns the path to the result.
    /// </summary>
    /// <remarks>
    /// PDF is the reference format of choice rather than PNG: it covers every page in one
    /// pass, and it keeps text as text so that both visual and textual comparisons can be
    /// driven from the same artefact.
    /// </remarks>
    public string ConvertToPdf(string inputPath, string outputDirectory)
        => throw new NotImplementedException();

    /// <summary>
    /// Converts several documents in a single <c>soffice</c> invocation.
    /// </summary>
    /// <remarks>
    /// Worth doing: process startup dominates the cost of a conversion, so batching a
    /// corpus is far faster than converting one file at a time. Each input still gets its
    /// own output directory to sidestep the name collision described on this class.
    /// </remarks>
    public IReadOnlyDictionary<string, string> ConvertManyToPdf(
        IEnumerable<string> inputPaths,
        string outputDirectory) => throw new NotImplementedException();

    /// <summary>
    /// Extracts a document's text using LibreOffice's own plain-text export, for
    /// comparison against Paperless extraction.
    /// </summary>
    /// <remarks>
    /// The filter differs by family — plain text for word processing, CSV for
    /// spreadsheets, HTML for presentations, since Impress has no text filter — so this
    /// picks the right one from the input's format.
    /// </remarks>
    public string ExtractText(string inputPath, string outputDirectory)
        => throw new NotImplementedException();

    /// <summary>
    /// Checks that the fonts a comparison depends on resolve the way they must.
    /// </summary>
    /// <remarks>
    /// Specifically, that the metric-compatible substitutes are installed: Calibri must
    /// resolve to Carlito and Cambria to Caladea, not to a DejaVu face. Without them,
    /// every OOXML document reflows and every page after the first diverges — a failure
    /// that looks like a Paperless layout bug but is not one.
    /// </remarks>
    public FontEnvironmentReport CheckFontEnvironment() => throw new NotImplementedException();

    /// <inheritdoc/>
    public void Dispose() { }
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
