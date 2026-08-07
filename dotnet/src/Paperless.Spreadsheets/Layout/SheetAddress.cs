using System.Globalization;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Parses the A1 cell addresses print settings are written in.
/// </summary>
/// <remarks>
/// <para>
/// All three formats state a print area as text in some variant of A1 notation, and all three
/// variants are the same once the decoration is stripped: ODF writes
/// <c>Sales.$A$1:$D$7</c> in the OOO syntax, SpreadsheetML writes <c>Sales!$A$1:$D$7</c> in a
/// <c>definedName</c>, and BIFF stores the same thing as formula tokens whose sheet reference
/// has already been resolved by the time it reaches here.
/// </para>
/// <para>
/// The sheet name is stripped and discarded rather than checked. Checking it would mean
/// resolving names with spaces, quoted names and the <c>#REF!</c> a deleted sheet leaves behind,
/// and would buy nothing: a print area on a sheet other than its own is not a thing Calc or
/// Excel produces, and a range written without any sheet name at all is common enough that the
/// parser has to work without one anyway.
/// </para>
/// <para>
/// A whole-column reference — <c>$A:$D</c>, which is what "repeat these columns" is written as
/// — has no row part. It reads as the full column range spanning every row, which is what
/// <see cref="SheetPrintSetup.RepeatColumns"/> means by a column band.
/// </para>
/// </remarks>
public static class SheetAddress
{
    /// <summary>The last column of a sheet, zero-based: XFD.</summary>
    public const int MaxColumn = 16383;

    /// <summary>The last row of a sheet, zero-based.</summary>
    public const int MaxRow = 1048575;

    /// <summary>Parses a range, with or without a sheet qualifier.</summary>
    /// <param name="text">The range text.</param>
    /// <param name="range">The parsed range, when this returns true.</param>
    public static bool TryParseRange(string? text, out SheetRange range)
    {
        range = default;
        if (string.IsNullOrWhiteSpace(text)) return false;

        string body = text.Trim();

        // A range spanning two sheets qualifies both ends, so the separator has to be found
        // before the sheet names are stripped or the second one takes the colon with it.
        int colon = SplitAt(body);
        string first = colon < 0 ? body : body[..colon];
        string second = colon < 0 ? body : body[(colon + 1)..];

        if (!TryParseCell(first, out int firstColumn, out int firstRow)) return false;
        if (!TryParseCell(second, out int lastColumn, out int lastRow)) return false;

        // A whole-column reference states no row and a whole-row reference no column; the
        // missing half spans the sheet, which is what makes "$A:$D" mean every row of A to D.
        if (firstRow < 0 || lastRow < 0)
        {
            firstRow = 0;
            lastRow = colon < 0 ? 0 : MaxRow;
        }
        if (firstColumn < 0 || lastColumn < 0)
        {
            firstColumn = 0;
            lastColumn = colon < 0 ? 0 : MaxColumn;
        }

        range = new SheetRange(
            Math.Min(firstColumn, lastColumn),
            Math.Min(firstRow, lastRow),
            Math.Max(firstColumn, lastColumn),
            Math.Max(firstRow, lastRow));

        return range.IsValid;
    }

    /// <summary>
    /// Writes an address back in A1 notation.
    /// </summary>
    /// <remarks>
    /// The inverse of <see cref="TryParseCell"/>, undecorated: no dollars and no sheet name, which
    /// is what <c>ScAddress::Format</c> gives for <c>ScRefFlags::VALID</c> alone and what Calc
    /// prints beside a note (<c>sc/source/ui/view/printfun.cxx:1978</c>).
    /// </remarks>
    /// <param name="column">The zero-based column.</param>
    /// <param name="row">The zero-based row.</param>
    public static string Format(int column, int row)
    {
        if (column < 0 || row < 0) return string.Empty;

        Span<char> letters = stackalloc char[4];
        int at = letters.Length;
        int remaining = column;

        // Bijective base 26: A is 1 rather than 0, so Z is followed by AA and there is no digit
        // that means nothing. Subtracting one before each division is what makes it bijective.
        do
        {
            letters[--at] = (char)('A' + (remaining % 26));
            remaining = (remaining / 26) - 1;
        }
        while (remaining >= 0 && at > 0);

        return string.Concat(letters[at..], (row + 1).ToString(CultureInfo.InvariantCulture));
    }

    /// <summary>
    /// Parses one address, returning -1 for a coordinate the reference leaves out.
    /// </summary>
    /// <param name="text">The address, optionally sheet-qualified and dollar-decorated.</param>
    /// <param name="column">The zero-based column, or -1 when the reference states none.</param>
    /// <param name="row">The zero-based row, or -1 when the reference states none.</param>
    public static bool TryParseCell(string? text, out int column, out int row)
    {
        column = -1;
        row = -1;
        if (string.IsNullOrWhiteSpace(text)) return false;

        ReadOnlySpan<char> span = StripSheet(text.Trim());
        if (span.Length == 0) return false;

        int at = 0;
        if (at < span.Length && span[at] == '$') at++;

        int letters = at;
        long value = 0;
        while (at < span.Length && char.IsAsciiLetter(span[at]))
        {
            // Bijective base 26: A is one rather than zero, so AA is 27 and not 0. Taking one
            // off at the end is what turns it into an index.
            value = (value * 26) + (char.ToUpperInvariant(span[at]) - 'A' + 1);
            if (value > MaxColumn + 1) return false;
            at++;
        }

        bool hasColumn = at > letters;
        if (hasColumn) column = (int)value - 1;
        if (at < span.Length && span[at] == '$') at++;

        int digits = at;
        long number = 0;
        while (at < span.Length && char.IsAsciiDigit(span[at]))
        {
            number = (number * 10) + (span[at] - '0');
            if (number > MaxRow + 1) return false;
            at++;
        }

        bool hasRow = at > digits;
        if (at != span.Length || (!hasColumn && !hasRow)) return false;
        if (hasRow && number == 0) return false;

        if (hasRow) row = (int)number - 1;

        return column <= MaxColumn && row <= MaxRow;
    }

    /// <summary>
    /// Where the colon separating a range's two ends is, or -1 when there is none.
    /// </summary>
    /// <remarks>
    /// Searched from the right and outside quotes, because a sheet name may contain a colon and
    /// is then written quoted — <c>'a:b'!$A$1</c> is a real, if unkind, sheet name.
    /// </remarks>
    private static int SplitAt(string text)
    {
        bool quoted = false;
        for (int at = 0; at < text.Length; at++)
        {
            if (text[at] == '\'') quoted = !quoted;
            else if (text[at] == ':' && !quoted) return at;
        }
        return -1;
    }

    /// <summary>Drops a leading sheet qualifier, in either format's spelling.</summary>
    private static ReadOnlySpan<char> StripSheet(string text)
    {
        // A quoted name is stripped first, so that a dot or a bang inside it is not mistaken for
        // the separator.
        if (text.StartsWith('\''))
        {
            int close = text.IndexOf('\'', 1);
            while (close >= 0 && close + 1 < text.Length && text[close + 1] == '\'')
                close = text.IndexOf('\'', close + 2);

            if (close < 0) return [];
            int after = close + 1;
            if (after < text.Length && (text[after] == '.' || text[after] == '!')) after++;
            return text.AsSpan(after);
        }

        int bang = text.LastIndexOf('!');
        if (bang >= 0) return text.AsSpan(bang + 1);

        int dot = text.LastIndexOf('.');
        return dot >= 0 ? text.AsSpan(dot + 1) : text.AsSpan();
    }
}
