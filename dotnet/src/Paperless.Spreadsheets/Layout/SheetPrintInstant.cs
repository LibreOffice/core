using System.Globalization;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// When a printout is deemed to have been made, for the header fields that print it.
/// </summary>
/// <remarks>
/// <para>
/// <c>&amp;D</c> and <c>&amp;T</c> print the current date and time, so a workbook carrying either
/// renders differently every time the clock moves past the field's own resolution — a day for the
/// date, a minute for the time. That is correct behaviour and LibreOffice does the same; it is
/// also the whole of why two renderings of one corpus are not byte-comparable, which is how the
/// last three rounds measured a change's reach. Measured on the sheets track: rendering all 171
/// documents twice within the same minute produced no difference at all, and rendering them under
/// two time zones a day apart moved exactly the documents that state one of these fields.
/// </para>
/// <para>
/// So the instant is taken from one place, and that place honours <c>SOURCE_DATE_EPOCH</c> — the
/// reproducible-builds convention, seconds since the Unix epoch, UTC. Setting it pins every date
/// and time a rendering draws, so two runs of the same binary over the same input are byte-equal
/// and a diff between them is the change under study rather than the clock. Leaving it unset is
/// the ordinary case and reads the wall clock.
/// </para>
/// <para>
/// Read once and cached. The variable is a property of the run rather than of a document, and a
/// job that took a second to render would otherwise be free to disagree with itself about what
/// "now" means, which is the defect this type also fixes: the instant used to be read afresh in
/// every page's header context.
/// </para>
/// </remarks>
internal static class SheetPrintInstant
{
    /// <summary>The environment variable that pins the instant.</summary>
    public const string EpochVariable = "SOURCE_DATE_EPOCH";

    private static readonly DateTime? Pinned = ReadPinned();

    /// <summary>The instant a printout should date itself at.</summary>
    public static DateTime Now() => Pinned ?? DateTime.Now;

    /// <summary>
    /// The instant <c>SOURCE_DATE_EPOCH</c> names, or null when it names nothing usable.
    /// </summary>
    /// <remarks>
    /// A malformed value is ignored rather than thrown on. The variable is set by build systems
    /// and by whatever else shares the environment, and a workbook that will not render because
    /// something else in the process set a variable badly is a worse outcome than one dated from
    /// the clock. Converted to local time because that is what the fields print and what the
    /// unpinned path returns.
    /// </remarks>
    private static DateTime? ReadPinned()
    {
        string? raw = Environment.GetEnvironmentVariable(EpochVariable);
        if (string.IsNullOrWhiteSpace(raw)) return null;

        return long.TryParse(raw.Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture,
                             out long seconds)
               && seconds >= 0
               && seconds <= 253402300799L // 9999-12-31T23:59:59Z; beyond it FromUnixTimeSeconds throws.
            ? DateTimeOffset.FromUnixTimeSeconds(seconds).ToLocalTime().DateTime
            : null;
    }
}
