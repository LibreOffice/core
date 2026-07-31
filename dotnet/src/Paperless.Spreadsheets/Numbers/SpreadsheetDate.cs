namespace Paperless.Spreadsheets.Numbers;

/// <summary>Which epoch a workbook counts its date serials from.</summary>
/// <remarks>
/// Excel shipped two, and a workbook says which it uses. Reading a 1904 workbook as 1900 shifts
/// every date in it by 1462 days, which looks like data corruption rather than a date-system
/// mistake.
/// </remarks>
public enum SpreadsheetDateSystem
{
    /// <summary>Serial 1 is 1 January 1900. The default, and what Windows Excel writes.</summary>
    Date1900,

    /// <summary>Serial 0 is 1 January 1904. Classic Mac Excel, and still selectable.</summary>
    Date1904,
}

/// <summary>
/// Converts between a spreadsheet's date serial numbers and .NET date and time types.
/// </summary>
/// <remarks>
/// A spreadsheet has no date type: a date is a number of days since the epoch and a time is the
/// fraction of a day, and only the number format says which a cell means. That is why the
/// conversion lives here rather than in a reader.
/// </remarks>
public static class SpreadsheetDate
{
    /// <summary>
    /// The 1900 system's zero point, chosen so that serials from 61 upwards add directly.
    /// </summary>
    /// <remarks>
    /// It is 30 December 1899, not 31 December, because Excel treats 1900 as a leap year — an
    /// intentional Lotus 1-2-3 compatibility bug that gives serial 60 to a 29 February that
    /// never existed. Every serial above it is therefore one day further back than the naive
    /// arithmetic suggests, and the offset is baked into every file ever written.
    /// </remarks>
    private static readonly DateTime Epoch1900 = new(1899, 12, 30);

    private static readonly DateTime Epoch1904 = new(1904, 1, 1);

    /// <summary>The number of days between the two epochs.</summary>
    public const int Date1904Offset = 1462;

    /// <summary>
    /// Converts a serial number to a date and time, or null when it is out of range.
    /// </summary>
    public static DateTime? FromSerial(double serial, SpreadsheetDateSystem system)
    {
        if (double.IsNaN(serial) || double.IsInfinity(serial)) return null;

        double days = serial;
        DateTime epoch;

        if (system == SpreadsheetDateSystem.Date1904)
        {
            epoch = Epoch1904;
        }
        else
        {
            epoch = Epoch1900;
            // Serials below 61 predate the phantom 29 February 1900, so they are one day
            // ahead of the epoch arithmetic; 60 itself is that phantom day and clamps.
            if (days < 60) days += 1;
            else if (days < 61) days = 60;
        }

        // A whole day is 86400 seconds; rounding to the millisecond keeps 0.604166666666667
        // from surfacing as 14:29:59.999.
        double totalMilliseconds = days * 86400000.0;
        if (totalMilliseconds is < -3.2e13 or > 3.2e14) return null;

        try
        {
            return epoch.AddMilliseconds(Math.Round(totalMilliseconds));
        }
        catch (ArgumentOutOfRangeException)
        {
            return null;
        }
    }

    /// <summary>
    /// The time-of-day part of a serial, as a duration since midnight.
    /// </summary>
    /// <remarks>
    /// A <see cref="TimeSpan"/> rather than a time-of-day, because a spreadsheet does not
    /// distinguish 14:30 from an elapsed fourteen and a half hours — the stored number is the
    /// same and only the format tells them apart. An elapsed format keeps the whole days, a
    /// clock format keeps only the remainder.
    /// </remarks>
    public static TimeSpan ToTimeOfDay(double serial, bool keepWholeDays)
    {
        double days = keepWholeDays ? serial : serial - Math.Truncate(serial);
        return TimeSpan.FromMilliseconds(Math.Round(days * 86400000.0));
    }
}
