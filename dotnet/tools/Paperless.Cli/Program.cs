namespace Paperless.Cli;

/// <summary>
/// The <c>paperless</c> command-line tool.
/// </summary>
/// <remarks>
/// <para>
/// Serves two audiences. Humans use it to inspect and convert documents. The fidelity
/// comparison skills in <c>.claude/skills/</c> use it as the Paperless side of a
/// side-by-side against headless LibreOffice, which is why its subcommands mirror
/// LibreOffice's own conversion targets.
/// </para>
/// <para>
/// Planned subcommands:
/// </para>
/// <list type="table">
///   <listheader><term>Command</term><description>Purpose</description></listheader>
///   <item><term><c>identify</c></term>
///         <description>Report the detected format, container and encryption state.</description></item>
///   <item><term><c>extract</c></term>
///         <description>Write text, or the content tree as JSON.</description></item>
///   <item><term><c>metadata</c></term>
///         <description>Write document properties as JSON.</description></item>
///   <item><term><c>render</c></term>
///         <description>Render pages to PNG at a chosen resolution.</description></item>
///   <item><term><c>convert</c></term>
///         <description>Write PDF or SVG.</description></item>
/// </list>
/// </remarks>
internal static class Program
{
    private static int Main(string[] args)
    {
        // Argument parsing and the subcommands land here once the libraries can
        // actually read something. Kept deliberately inert for now rather than
        // half-wired: a CLI that accepts commands and then throws is worse than one
        // that says plainly it is not ready.
        _ = args;
        Console.Error.WriteLine(
            "paperless: not implemented yet. This is a skeleton — see dotnet/TODO.md for the plan.");
        return ExitSoftwareError;
    }

    /// <summary>
    /// Exit code 70, the conventional <c>EX_SOFTWARE</c> from <c>sysexits.h</c>, meaning
    /// an internal software state prevented the command from running.
    /// </summary>
    private const int ExitSoftwareError = 70;
}
