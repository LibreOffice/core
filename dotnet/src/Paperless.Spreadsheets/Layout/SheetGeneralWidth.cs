using System.Globalization;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// The <c>General</c> format rendered to fit a stated number of characters.
/// </summary>
/// <remarks>
/// <para>
/// The one place a spreadsheet's <em>displayed text</em> depends on its column width, and the
/// reason `###` is not the whole story: a numeric cell whose formatted text will not fit is only
/// hashed when its format is something other than <c>General</c>. A <c>General</c> cell is
/// re-rendered with fewer digits first, and falls back to scientific notation when even that is
/// too long — which is why 123 456 789 012 in a 43 pt column draws as <c>1.2E+11</c> rather than
/// as <c>###</c>. Measured against LibreOffice 24.2.7.2 on exactly that cell.
/// </para>
/// <para>
/// Ported from <c>SvNumberformat::GetOutputString(double, sal_uInt16 nCharCount, …)</c>
/// (<c>svl/source/numbers/zformat.cxx:2429</c>) and its scientific companion
/// <c>lcl_GetOutputStringScientific</c> (<c>:2277</c>). The character count is not a measurement:
/// Calc divides the available width by the width of the <em>widest digit</em> and passes the
/// quotient (<c>ScDrawStringsVars::SetTextToWidthOrHash</c>, <c>sc/source/ui/view/output2.cxx:645</c>),
/// so a column that fits seven digits is allowed seven characters whatever they turn out to be.
/// </para>
/// <para>
/// Rendering-only, deliberately. Nothing here is reachable from extraction, and
/// <c>paperless extract</c> keeps reporting the full text — a recorded decision, because the
/// shortened form is a function of a column width that extracted text does not have.
/// </para>
/// </remarks>
internal static class SheetGeneralWidth
{
    /// <summary>Below this magnitude, scientific notation is preferred outright.</summary>
    /// <remarks><c>EXP_LOWER_BOUND</c>, <c>zformat.cxx:61</c>.</remarks>
    private const double ExponentialLowerBound = 1.0E-4;

    /// <summary>The most decimals either branch will write.</summary>
    private const int MaximumPrecision = 14;

    /// <summary>
    /// A number written to fit a character budget, or null when the budget cannot be met.
    /// </summary>
    /// <param name="value">The cell's value.</param>
    /// <param name="characters">How many characters the column has room for.</param>
    public static string Render(double value, int characters)
    {
        if (double.IsNaN(value) || double.IsInfinity(value)) return "###";

        int budget = Math.Max(characters, 0);
        double magnitude = Math.Abs(value);
        bool negative = double.IsNegative(value);

        if (magnitude < ExponentialLowerBound) return Scientific(value, budget);

        // Values below one still have a digit before the point, which is why this is not simply
        // the count of integer digits.
        double exponent = Math.Log10(magnitude);
        int digitsBefore = exponent >= 0.0 ? (int)Math.Ceiling(exponent) : 1;
        if (digitsBefore > 15) return Scientific(value, budget);

        int precision = budget >= digitsBefore ? budget - digitsBefore : 0;
        if (precision > 0 && negative) precision--;
        if (precision > 0) precision--;         // the decimal point takes a character too

        string fixedForm = Fixed(value, precision);
        return fixedForm.Length > budget ? Scientific(value, budget) : fixedForm;
    }

    /// <summary>
    /// Fixed notation to a precision, with trailing decimal zeros stripped.
    /// </summary>
    /// <remarks>
    /// Stripping is unconditional in LibreOffice and the comment beside it says why: binary
    /// rounding makes it impossible to tell a value the user typed as 844.1 from one stored as
    /// 844.10000000000002 (<c>ImpGetOutputStdToPrecision</c>, <c>zformat.cxx:2128</c>).
    /// </remarks>
    private static string Fixed(double value, int precision)
    {
        int places = Math.Clamp(precision, 0, MaximumPrecision);
        double rounded = Math.Round(value, places, MidpointRounding.AwayFromZero);
        string text = rounded.ToString(
            "F" + places.ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);

        if (text.Contains('.', StringComparison.Ordinal))
        {
            text = text.TrimEnd('0').TrimEnd('.');
        }

        // Never "-0": a negative value that rounds away to nothing is zero.
        return text is "-0" or "-" ? "0" : text;
    }

    /// <summary>
    /// Scientific notation sized to the budget.
    /// </summary>
    /// <remarks>
    /// The six characters the shape costs are <c>1</c>, <c>.</c>, <c>E</c>, the sign and two
    /// exponent digits; a three-digit exponent costs a seventh. Whatever is left of the budget is
    /// the mantissa's precision, less one more when the value is negative.
    /// </remarks>
    private static string Scientific(double value, int budget)
    {
        double exponent = Math.Log10(Math.Abs(value));
        if (exponent < 0.0) exponent = 1.0 - exponent;

        int shape = 6 + (exponent >= 100.0 ? 1 : 0);
        int precision = budget > shape ? budget - shape : 0;
        if (precision > 0 && double.IsNegative(value)) precision--;
        precision = Math.Min(precision, MaximumPrecision);

        string text = value.ToString(
            "0." + new string('0', Math.Max(precision, 0)) + "E+00", CultureInfo.InvariantCulture);

        // Trailing zeros are stripped from the mantissa, the same way the fixed branch strips
        // them: rtl::math::doubleToUString is called with bEraseTrailingDecZeros set in both.
        int e = text.IndexOf('E', StringComparison.Ordinal);
        if (e > 0 && text.AsSpan(0, e).Contains('.'))
        {
            string mantissa = text[..e].TrimEnd('0').TrimEnd('.');
            text = mantissa + text[e..];
        }

        return text;
    }
}
