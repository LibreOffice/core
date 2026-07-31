namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Decodes Word's packed <c>DTTM</c> stamp, which both DOC and RTF use for a revision's date.
/// </summary>
/// <remarks>
/// <para>
/// Thirty-two bits of bit fields rather than an epoch offset, laid out exactly as LibreOffice's
/// <c>DTTM2DateTime</c> documents it (<c>filter/source/msfilter/util.cxx</c>): six bits of minute,
/// five of hour, five of day, four of month, nine of year since 1900, and three of weekday that can
/// be ignored because the date already says which day it is.
/// </para>
/// <para>
/// Shared with the RTF reader, which states the same quantity in <c>\revdttm</c> and
/// <c>\revdttmdel</c> — RTF being Word's own interchange format, its date is Word's date and not an
/// ISO one.
/// </para>
/// </remarks>
public static class Ww8DateTime
{
    /// <summary>
    /// The instant a <c>DTTM</c> names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Zero is "no date", not midnight on the first of January 1900 — LibreOffice maps it to an empty
    /// <c>DateTime</c> for the same reason. It is what both legacy formats write when the producer had
    /// no timestamp or suppressed it, and the corpus's own revision documents carry exactly that.
    /// A stamp whose fields do not make a real date is also null rather than clamped: a month of 0 or
    /// 13 is corruption, and inventing January from it would be a worse answer than none.
    /// </remarks>
    public static DateTime? Decode(uint stamp)
    {
        if (stamp == 0) return null;

        // The masks, not the widths. Six bits of minute and five of hour do end at bit 11, but the
        // month starts at 16 rather than at 15 and the year at 20 rather than at 19 — the fields are
        // not packed tight, and LibreOffice's own comment gives the masks (0x0000003F, 0x000007C0,
        // 0x0000F800, 0x000F0000, 0x1FF00000) rather than the widths for exactly that reason.
        // Shifting by the widths instead reads one bit of the day as the month's low bit: the
        // corpus's revisions.doc, whose changes are stamped 1970-01-01, comes back 2040-02-01.
        int minute = (int)(stamp & 0x3F);
        int hour = (int)((stamp >> 6) & 0x1F);
        int day = (int)((stamp >> 11) & 0x1F);
        int month = (int)((stamp >> 16) & 0x0F);
        int year = (int)((stamp >> 20) & 0x1FF) + 1900;

        if (month is < 1 or > 12 || day < 1 || hour > 23 || minute > 59) return null;
        if (day > DateTime.DaysInMonth(year, month)) return null;

        return new DateTime(year, month, day, hour, minute, 0, DateTimeKind.Unspecified);
    }
}
