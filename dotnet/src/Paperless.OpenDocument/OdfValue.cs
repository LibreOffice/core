using System.Globalization;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.OpenDocument;

/// <summary>
/// Parsers for the attribute value types ODF uses.
/// </summary>
/// <remarks>
/// <para>
/// Every one of these returns null rather than throwing on input it cannot make sense of.
/// ODF attribute values are frequently written by tools other than an office suite, and a
/// single unparseable measurement is not a reason to abandon a document — the caller
/// treats "unparseable" the same as "absent" and falls back to the inherited or default
/// value, which is what LibreOffice's own import does.
/// </para>
/// <para>
/// Numbers are parsed with <see cref="CultureInfo.InvariantCulture"/> throughout. ODF
/// measurements are XSL/CSS values and always use a full stop for the decimal separator,
/// so parsing under the ambient culture would silently misread every fractional
/// measurement in a comma-decimal locale.
/// </para>
/// </remarks>
public static class OdfValue
{
    /// <summary>
    /// Parses an ODF length such as <c>2.54cm</c>, <c>-0.5in</c> or <c>12pt</c>.
    /// </summary>
    /// <remarks>
    /// The unit is mandatory in ODF, but files written by converters often omit it; a
    /// bare number is read as 1/100 mm, which is ODF's own internal unit and therefore
    /// the least surprising guess. Relative units (<c>em</c>, <c>%</c>) cannot become an
    /// absolute <see cref="Length"/> without a context and return null — use
    /// <see cref="ParsePercentage"/> for those.
    /// </remarks>
    public static Length? ParseLength(string? value)
    {
        if (value is null) return null;
        ReadOnlySpan<char> text = value.AsSpan().Trim();
        if (text.IsEmpty) return null;

        int unitStart = text.Length;
        while (unitStart > 0 && !char.IsAsciiDigit(text[unitStart - 1]) && text[unitStart - 1] != '.')
            unitStart--;

        ReadOnlySpan<char> number = text[..unitStart];
        ReadOnlySpan<char> unit = text[unitStart..].Trim();

        if (!double.TryParse(number, NumberStyles.Float, CultureInfo.InvariantCulture, out double magnitude))
            return null;

        return unit switch
        {
            "cm" => Length.FromMillimetres(magnitude * 10),
            "mm" => Length.FromMillimetres(magnitude),
            "in" => Length.FromInches(magnitude),
            "pt" => Length.FromPoints(magnitude),
            // A pica is 12 points; ODF inherits the unit from XSL.
            "pc" => Length.FromPoints(magnitude * 12),
            // CSS fixes px at 1/96 inch. ODF does not define a device resolution, and
            // LibreOffice's own import uses the same convention.
            "px" => Length.FromPixels(magnitude, 96),
            "" => Length.FromMm100((long)Math.Round(magnitude)),
            _ => null,
        };
    }

    /// <summary>
    /// Parses a percentage such as <c>115%</c>, returning it as a fraction (1.15).
    /// </summary>
    public static double? ParsePercentage(string? value)
    {
        if (value is null) return null;
        ReadOnlySpan<char> text = value.AsSpan().Trim();
        if (text.Length < 2 || text[^1] != '%') return null;
        return double.TryParse(text[..^1], NumberStyles.Float, CultureInfo.InvariantCulture, out double percent)
            ? percent / 100.0
            : null;
    }

    /// <summary>Parses a plain ODF number, invariant-culture.</summary>
    public static double? ParseDouble(string? value)
        => value is not null
           && double.TryParse(value.AsSpan().Trim(), NumberStyles.Float, CultureInfo.InvariantCulture, out double d)
            ? d
            : null;

    /// <summary>Parses a plain ODF integer, invariant-culture.</summary>
    public static int? ParseInt(string? value)
        => value is not null
           && int.TryParse(value.AsSpan().Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture, out int i)
            ? i
            : null;

    /// <summary>Parses an ODF boolean: the literals <c>true</c> and <c>false</c>.</summary>
    /// <remarks>
    /// ODF booleans are XSD booleans, so <c>1</c> and <c>0</c> are also legal and appear
    /// in files written by non-LibreOffice tools.
    /// </remarks>
    public static bool? ParseBoolean(string? value) => value?.Trim() switch
    {
        "true" or "1" => true,
        "false" or "0" => false,
        _ => null,
    };

    /// <summary>
    /// Parses an ODF colour: <c>#rrggbb</c>, or the keyword <c>transparent</c>.
    /// </summary>
    /// <remarks>
    /// ODF colours are always fully opaque six-digit hex; transparency is carried by a
    /// separate opacity property, except for the <c>transparent</c> keyword that
    /// <c>fo:background-color</c> accepts to mean "no fill at all".
    /// </remarks>
    public static Colour? ParseColour(string? value)
    {
        if (value is null) return null;
        ReadOnlySpan<char> text = value.AsSpan().Trim();
        if (text.Equals("transparent", StringComparison.OrdinalIgnoreCase)) return Colour.Transparent;
        if (text.Length != 7 || text[0] != '#') return null;
        return uint.TryParse(text[1..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint rgb)
            ? Colour.FromRgb(rgb)
            : null;
    }

    /// <summary>
    /// Parses an ODF date or date-time (<c>2026-07-30</c> or <c>2026-07-30T18:14:07.12</c>).
    /// </summary>
    /// <remarks>
    /// ODF timestamps usually carry no zone. Such a value is taken as local-unspecified
    /// and returned with a zero offset rather than being shifted by the reading machine's
    /// zone: shifting it would make the same file report different timestamps on different
    /// machines, which is worse than reporting an unknown zone as UTC.
    /// </remarks>
    public static DateTimeOffset? ParseDateTime(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        string text = value.Trim();

        if (DateTimeOffset.TryParse(text, CultureInfo.InvariantCulture,
                                    DateTimeStyles.RoundtripKind, out DateTimeOffset withZone)
            && HasExplicitZone(text))
            return withZone;

        return DateTime.TryParse(text, CultureInfo.InvariantCulture,
                                 DateTimeStyles.NoCurrentDateDefault, out DateTime naive)
            ? new DateTimeOffset(DateTime.SpecifyKind(naive, DateTimeKind.Unspecified), TimeSpan.Zero)
            : null;

        static bool HasExplicitZone(string text)
            => text.EndsWith('Z') || text.LastIndexOfAny(['+', '-']) > text.IndexOf('T');
    }

    /// <summary>
    /// Parses an ISO 8601 duration as used by <c>meta:editing-duration</c>, e.g.
    /// <c>PT2H30M15S</c>.
    /// </summary>
    /// <remarks>
    /// <see cref="System.Xml.XmlConvert"/> would do this, but it throws on the malformed
    /// durations that appear in real files and on the <c>P0D</c> that some writers emit
    /// for "no time recorded"; hand-parsing keeps the lenient-read rule.
    /// </remarks>
    public static TimeSpan? ParseDuration(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        ReadOnlySpan<char> text = value.AsSpan().Trim();

        bool negative = false;
        if (text[0] == '-') { negative = true; text = text[1..]; }
        if (text.IsEmpty || text[0] != 'P') return null;
        text = text[1..];

        double days = 0, hours = 0, minutes = 0, seconds = 0;
        bool inTime = false, sawAny = false;

        while (!text.IsEmpty)
        {
            if (text[0] == 'T') { inTime = true; text = text[1..]; continue; }

            int digits = 0;
            while (digits < text.Length && (char.IsAsciiDigit(text[digits]) || text[digits] == '.')) digits++;
            if (digits == 0 || digits == text.Length) return null;

            if (!double.TryParse(text[..digits], NumberStyles.Float, CultureInfo.InvariantCulture, out double magnitude))
                return null;

            char designator = text[digits];
            text = text[(digits + 1)..];
            sawAny = true;

            switch (designator)
            {
                case 'D': days = magnitude; break;
                case 'H': hours = magnitude; break;
                // 'M' is months before the T and minutes after it. Months cannot become an
                // exact TimeSpan, and an editing duration never uses them, so a month
                // component means this is not an editing duration and is rejected.
                case 'M' when inTime: minutes = magnitude; break;
                case 'S': seconds = magnitude; break;
                default: return null;
            }
        }

        if (!sawAny) return null;
        TimeSpan total = TimeSpan.FromDays(days) + TimeSpan.FromHours(hours)
                       + TimeSpan.FromMinutes(minutes) + TimeSpan.FromSeconds(seconds);
        return negative ? -total : total;
    }
}
