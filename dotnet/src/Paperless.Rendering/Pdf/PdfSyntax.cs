using System.Globalization;
using System.Text;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// The lexical layer of PDF: how a number, a name and a string are spelled.
/// </summary>
/// <remarks>
/// Separated from the writer because every one of these has a rule that is easy to get
/// subtly wrong and impossible to see in the output — a number written in a culture that
/// uses a decimal comma produces a file that opens and draws nothing, and a name holding
/// a space silently truncates the dictionary key.
/// </remarks>
internal static class PdfSyntax
{
    /// <summary>
    /// Formats a number for a content stream or a dictionary.
    /// </summary>
    /// <remarks>
    /// Four decimals, which is a twentieth of a twip and finer than anything a document
    /// states, and always invariant: PDF has no locale and a decimal comma is a syntax
    /// error rather than a rendering difference. Negative zero is normalised away so that
    /// two runs that differ only in the sign of a rounding artefact produce identical
    /// bytes, which is what makes a written PDF checksummable.
    /// </remarks>
    public static string Number(double value)
    {
        if (double.IsNaN(value) || double.IsInfinity(value)) return "0";

        double rounded = Math.Round(value, 4, MidpointRounding.AwayFromZero);
        if (rounded == 0) return "0";

        return rounded.ToString("0.####", CultureInfo.InvariantCulture);
    }

    /// <summary>Formats a colour component, which PDF states from 0 to 1.</summary>
    public static string Component(byte value) => Number(value / 255.0);

    /// <summary>
    /// Escapes a literal string, as a PDF <c>(…)</c> token.
    /// </summary>
    /// <remarks>
    /// Only the three characters that end or nest the token need escaping; everything else
    /// may pass through as a byte. Non-ASCII is written as UTF-16BE with a byte-order mark,
    /// which is what a PDF reader takes as the signal to decode it as text rather than as
    /// PDFDocEncoding.
    /// </remarks>
    public static string LiteralString(string value)
    {
        ArgumentNullException.ThrowIfNull(value);

        bool ascii = true;
        foreach (char c in value)
        {
            if (c is < ' ' or > '~') { ascii = false; break; }
        }

        if (!ascii) return HexString(Encoding.BigEndianUnicode.GetBytes("﻿" + value));

        StringBuilder text = new(value.Length + 2);
        text.Append('(');
        foreach (char c in value)
        {
            if (c is '(' or ')' or '\\') text.Append('\\');
            text.Append(c);
        }

        return text.Append(')').ToString();
    }

    /// <summary>Formats bytes as a PDF hexadecimal string.</summary>
    public static string HexString(ReadOnlySpan<byte> value)
    {
        StringBuilder text = new((value.Length * 2) + 2);
        text.Append('<');
        foreach (byte b in value) text.Append(b.ToString("X2", CultureInfo.InvariantCulture));
        return text.Append('>').ToString();
    }

    /// <summary>
    /// A PDF date, in the <c>D:YYYYMMDDHHmmSSOHH'mm</c> form the specification states.
    /// </summary>
    public static string Date(DateTimeOffset value)
    {
        string stamp = value.ToString("yyyyMMddHHmmss", CultureInfo.InvariantCulture);
        TimeSpan offset = value.Offset;

        if (offset == TimeSpan.Zero) return $"(D:{stamp}Z)";

        char sign = offset < TimeSpan.Zero ? '-' : '+';
        TimeSpan magnitude = offset.Duration();
        return string.Create(
            CultureInfo.InvariantCulture,
            $"(D:{stamp}{sign}{magnitude.Hours:D2}'{magnitude.Minutes:D2})");
    }
}
