using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Numbers;

namespace Paperless.OpenDocument.Styles;

/// <summary>
/// Compiles an ODF <c>number:*-style</c> into the format code the number engine parses.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why a translation rather than a second formatter.</strong> ODF states a number format
/// as a tree of elements — <c>number:number number:decimal-places="2" number:grouping="true"</c>
/// — where OOXML states it as the string <c>#,##0.00</c>. The two describe the same thing and
/// LibreOffice keeps exactly one formatter for both: <c>SvNumberformat</c>, which ODF's import
/// reaches by <em>building a format string</em> from the elements
/// (<c>xmloff/source/style/xmlnumfi.cxx</c>'s <c>SvXMLNumFormatContext::CreateAndInsert</c>, which
/// assembles <c>aFormatCode</c> piece by piece and hands it to <c>SvNumberFormatter</c>). This is
/// that assembly, and it is the reason <c>Paperless.Core.Numbers</c> needs no ODF path at all.
/// </para>
/// <para>
/// <strong>What it is for.</strong> An ODF chart's axis names a data style through
/// <c>style:data-style-name</c>, and that is the only statement of how its ticks are written —
/// there is no cached text on an axis the way there is on a cell, so a percentage axis draws
/// <c>0.05</c> instead of <c>5%</c> without this.
/// </para>
/// <para>
/// <strong>The trap, named.</strong> <c>number:minutes</c> and <c>number:month</c> both compile
/// to <c>M</c>, which is the same ambiguity the format-code language has and resolves the same
/// way — an <c>M</c> between an hour and a second is minutes. So the pieces must be emitted in
/// document order and not gathered by kind; sorting them, or emitting the date part before the
/// time part regardless of what the style says, turns <c>13:45</c> into month 45 of year 13.
/// </para>
/// </remarks>
public static class OdfNumberFormat
{
    /// <summary>
    /// The format code a data style states, or null when the style is not one this compiles.
    /// </summary>
    /// <param name="style">The <c>number:*-style</c> element.</param>
    public static string? Code(XElement? style)
    {
        if (style is null) return null;

        StringBuilder code = new();

        foreach (XElement piece in style.Elements())
        {
            if (piece.Name.NamespaceName != OdfNamespaces.Number) continue;
            Append(code, piece);
        }

        string built = code.ToString();
        return built.Length == 0 ? null : built;
    }

    /// <summary>The parsed code a data style states, or null.</summary>
    /// <param name="style">The <c>number:*-style</c> element.</param>
    public static NumberFormatCode? Parse(XElement? style)
    {
        if (Code(style) is not { Length: > 0 } code) return null;

        NumberFormatCode parsed = NumberFormatCode.Parse(code);
        return parsed.IsGeneral ? null : parsed;
    }

    private static void Append(StringBuilder code, XElement piece)
    {
        switch (piece.Name.LocalName)
        {
            case "number": Number(code, piece); break;
            case "scientific-number": Scientific(code, piece); break;
            case "fraction": Fraction(code, piece); break;

            // A literal. ODF writes the per cent sign, the currency symbol and every separator as
            // one of these, so quoting is what keeps a stray "d" or "m" in a suffix out of the
            // date vocabulary.
            case "text" or "currency-symbol": Literal(code, piece.Value); break;
            case "text-content": code.Append('@'); break;

            case "year": code.Append(Long(piece) ? "YYYY" : "YY"); break;
            case "month":
                code.Append(Flag(piece, "textual") == true
                    ? (Long(piece) ? "MMMM" : "MMM")
                    : (Long(piece) ? "MM" : "M"));
                break;
            case "day": code.Append(Long(piece) ? "DD" : "D"); break;
            case "day-of-week": code.Append(Long(piece) ? "NNNN" : "NNN"); break;
            case "quarter": code.Append(Long(piece) ? "QQ" : "Q"); break;
            case "week-of-year": code.Append("WW"); break;
            case "era": code.Append(Long(piece) ? "GGG" : "G"); break;

            case "hours": code.Append(Long(piece) ? "HH" : "H"); break;
            case "minutes": code.Append(Long(piece) ? "MM" : "M"); break;
            case "seconds": Seconds(code, piece); break;
            case "am-pm": code.Append("AM/PM"); break;

            case "boolean": code.Append("BOOLEAN"); break;

            default: break;
        }
    }

    private static void Number(StringBuilder code, XElement piece)
    {
        int integers = Integer(piece, "min-integer-digits") ?? 1;
        int decimals = Integer(piece, "decimal-places") ?? 0;
        int minimum = Integer(piece, "min-decimal-places") ?? decimals;
        bool grouping = Flag(piece, "grouping") == true;

        // The integer part is grouped by writing the group separator into it, which is what the
        // format-code language means by "#,##0": one hash-comma-hash-hash before the digits.
        if (grouping) code.Append("#,##");

        code.Append(integers <= 0 ? "#" : new string('0', integers));

        if (decimals <= 0) return;

        code.Append('.');
        code.Append('0', Math.Clamp(minimum, 0, decimals));
        code.Append('#', decimals - Math.Clamp(minimum, 0, decimals));
    }

    private static void Scientific(StringBuilder code, XElement piece)
    {
        int integers = Integer(piece, "min-integer-digits") ?? 1;
        int decimals = Integer(piece, "decimal-places") ?? 0;
        int exponent = Integer(piece, "min-exponent-digits") ?? 2;

        code.Append(integers <= 0 ? "#" : new string('0', integers));
        if (decimals > 0) code.Append('.').Append('0', decimals);
        code.Append("E+").Append('0', Math.Max(exponent, 1));
    }

    private static void Fraction(StringBuilder code, XElement piece)
    {
        int integers = Integer(piece, "min-integer-digits") ?? 0;
        int numerator = Integer(piece, "min-numerator-digits") ?? 1;
        int denominator = Integer(piece, "min-denominator-digits") ?? 1;
        int value = Integer(piece, "denominator-value") ?? 0;

        if (integers > 0) code.Append('#', integers).Append(' ');
        code.Append('?', Math.Max(numerator, 1)).Append('/');

        // A stated denominator is written as itself — "?/8" rather than "?/?" — which is what
        // makes eighths eighths rather than the nearest single-digit fraction.
        if (value > 0) code.Append(value.ToString(CultureInfo.InvariantCulture));
        else code.Append('?', Math.Max(denominator, 1));
    }

    private static void Seconds(StringBuilder code, XElement piece)
    {
        code.Append(Long(piece) ? "SS" : "S");

        int decimals = Integer(piece, "decimal-places") ?? 0;
        if (decimals > 0) code.Append('.').Append('0', decimals);
    }

    /// <summary>A literal, quoted so that its letters are not read as directives.</summary>
    private static void Literal(StringBuilder code, string? text)
    {
        if (text is not { Length: > 0 }) return;

        code.Append('"');
        foreach (char character in text)
        {
            if (character == '"') code.Append('\'');
            else code.Append(character);
        }

        code.Append('"');
    }

    private static bool Long(XElement piece)
        => piece.Attribute(XName.Get("style", OdfNamespaces.Number))?.Value == "long";

    private static bool? Flag(XElement piece, string name)
        => piece.Attribute(XName.Get(name, OdfNamespaces.Number))?.Value switch
        {
            "true" => true,
            "false" => false,
            _ => null,
        };

    private static int? Integer(XElement piece, string name)
        => int.TryParse(
            piece.Attribute(XName.Get(name, OdfNamespaces.Number))?.Value,
            NumberStyles.Integer,
            CultureInfo.InvariantCulture,
            out int parsed)
            ? parsed
            : null;
}
