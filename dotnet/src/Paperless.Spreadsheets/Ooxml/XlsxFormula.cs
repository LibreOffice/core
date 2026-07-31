using System.Globalization;
using System.Text;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// The little formula handling extraction needs: keeping a formula in the file's own syntax,
/// and reconstructing the ones a shared formula group leaves out.
/// </summary>
/// <remarks>
/// No parsing to tokens and no evaluation. The settled policy is that a cached result is what a
/// reference renderer displays, so extraction trusts it and keeps the expression as written —
/// translating it into another grammar would misreport what the document says.
/// </remarks>
internal static class XlsxFormula
{
    /// <summary>
    /// Rewrites a shared formula for a cell other than the group's master.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shared formula is written once, on the top-left cell of its range; every other cell in
    /// the group carries only <c>&lt;f t="shared" si="N"/&gt;</c> with no text at all. Excel
    /// writes these constantly — a filled-down column is one group — so leaving them empty
    /// would drop the formula from the overwhelming majority of cells in a real workbook. The
    /// group members differ from the master only in their <em>relative</em> references, offset
    /// by the distance from the master, which is exactly the rewrite performed here and what
    /// LibreOffice reconstructs in <c>sc/source/filter/oox/formulabuffer.cxx</c>.
    /// </para>
    /// <para>
    /// The trap is deciding what is a reference. <c>LOG10(x)</c> and <c>Tax_2020</c> both look
    /// like a column followed by a row, so a reference is recognised only when it is not glued
    /// to a neighbouring identifier character and is not immediately followed by an opening
    /// parenthesis. Quoted strings and quoted sheet names are skipped whole, because a cell
    /// address inside a string literal is text.
    /// </para>
    /// </remarks>
    /// <param name="formula">The master's expression, without a leading <c>=</c>.</param>
    /// <param name="rowOffset">Rows from the master to this cell.</param>
    /// <param name="columnOffset">Columns from the master to this cell.</param>
    public static string Shift(string formula, int rowOffset, int columnOffset)
    {
        ArgumentNullException.ThrowIfNull(formula);
        if (rowOffset == 0 && columnOffset == 0) return formula;

        StringBuilder output = new(formula.Length + 8);

        for (int i = 0; i < formula.Length;)
        {
            char c = formula[i];

            if (c == '"')
            {
                int end = SkipQuoted(formula, i, '"');
                output.Append(formula, i, end - i);
                i = end;
                continue;
            }

            if (c == '\'')
            {
                int end = SkipQuoted(formula, i, '\'');
                output.Append(formula, i, end - i);
                i = end;
                continue;
            }

            if (TryReadReference(formula, i, out int length, out ParsedReference reference))
            {
                output.Append(Rewrite(reference, rowOffset, columnOffset));
                i += length;
                continue;
            }

            output.Append(c);
            i++;
        }

        return output.ToString();
    }

    private static int SkipQuoted(string text, int start, char quote)
    {
        int i = start + 1;
        while (i < text.Length)
        {
            if (text[i] == quote)
            {
                // A doubled quote is an escaped one and does not close the literal.
                if (i + 1 < text.Length && text[i + 1] == quote) { i += 2; continue; }
                return i + 1;
            }
            i++;
        }
        return text.Length;
    }

    private readonly record struct ParsedReference(
        bool AbsoluteColumn, int Column, bool AbsoluteRow, int Row);

    private static bool TryReadReference(
        string text, int start, out int length, out ParsedReference reference)
    {
        length = 0;
        reference = default;

        // Glued to the identifier before it, this is the tail of a name, not a reference.
        if (start > 0 && IsIdentifierCharacter(text[start - 1]) && text[start - 1] != '!')
            return false;

        int i = start;
        bool absoluteColumn = false;
        if (i < text.Length && text[i] == '$') { absoluteColumn = true; i++; }

        int column = 0;
        int letters = 0;
        while (i < text.Length && char.IsAsciiLetter(text[i]))
        {
            column = (column * 26) + (char.ToUpperInvariant(text[i]) - 'A' + 1);
            letters++;
            i++;
        }
        if (letters is 0 or > 3) return false;

        bool absoluteRow = false;
        if (i < text.Length && text[i] == '$') { absoluteRow = true; i++; }

        int row = 0;
        int digits = 0;
        while (i < text.Length && char.IsAsciiDigit(text[i]))
        {
            row = (row * 10) + (text[i] - '0');
            digits++;
            i++;
        }
        if (digits is 0 or > 7 || row == 0) return false;

        // A name may end in digits — "LOG10(" — and a function call is the give-away.
        if (i < text.Length && (text[i] == '(' || IsIdentifierCharacter(text[i]))) return false;

        length = i - start;
        reference = new ParsedReference(absoluteColumn, column - 1, absoluteRow, row - 1);
        return true;
    }

    private static bool IsIdentifierCharacter(char c)
        => char.IsAsciiLetterOrDigit(c) || c is '_' or '.' or '$' or '!';

    private static string Rewrite(ParsedReference reference, int rowOffset, int columnOffset)
    {
        int column = reference.AbsoluteColumn ? reference.Column : reference.Column + columnOffset;
        int row = reference.AbsoluteRow ? reference.Row : reference.Row + rowOffset;

        // A reference shifted off the sheet is what Excel reports as #REF!, and saying so is
        // more honest than clamping it back onto a cell the formula never meant.
        if (column < 0 || row < 0) return "#REF!";

        StringBuilder output = new();
        if (reference.AbsoluteColumn) output.Append('$');
        output.Append(ColumnName(column));
        if (reference.AbsoluteRow) output.Append('$');
        output.Append((row + 1).ToString(CultureInfo.InvariantCulture));
        return output.ToString();
    }

    /// <summary>The A1-style name of a zero-based column index.</summary>
    public static string ColumnName(int column)
    {
        Span<char> buffer = stackalloc char[4];
        int position = buffer.Length;
        int value = column + 1;
        while (value > 0 && position > 0)
        {
            int remainder = (value - 1) % 26;
            buffer[--position] = (char)('A' + remainder);
            value = (value - 1) / 26;
        }
        return new string(buffer[position..]);
    }
}
