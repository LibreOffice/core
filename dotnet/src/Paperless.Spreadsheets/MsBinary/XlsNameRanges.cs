using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Pulls the cell ranges out of a <c>NAME</c> record's formula.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF name's value is an RPN token array, not text, so even a name that is nothing but a
/// range has to be decoded before the range can be seen. This decodes only what a print area or
/// a <c>Print_Titles</c> name can contain — reference tokens, the memory-area wrappers Excel
/// puts round a multi-range name, and the union operator that joins them. Anything else stops
/// the walk: a name whose value is an expression is not a print range, and a decoder that
/// guessed would attach a plausible wrong rectangle to the sheet.
/// </para>
/// <para>
/// Full formula decoding is deliberately not attempted here and is on the module's TODO. The
/// hard part of it is not the reference tokens but the class-selection problem — Excel encodes
/// the reference, value and array forms of one operator as three different opcodes — and none
/// of that is needed to read a rectangle.
/// </para>
/// <para>
/// The sheet reference is discarded along with the token's <c>ixti</c>. A print area on a sheet
/// other than the one whose name holds it is not something either application produces, and
/// resolving <c>ixti</c> means reading the <c>EXTERNSHEET</c> table for a result that would be
/// thrown away.
/// </para>
/// </remarks>
internal static class XlsNameRanges
{
    /// <summary>The last row of a BIFF8 sheet; BIFF5's is a quarter of it.</summary>
    private const int Biff8MaxRow = 65535;

    private const int Biff5MaxRow = 16383;

    /// <summary>The last column of a BIFF sheet, in every generation.</summary>
    private const int BiffMaxColumn = 255;

    /// <summary>Reads the ranges out of the next <paramref name="length"/> bytes of a record.</summary>
    /// <param name="stream">The record stream, positioned at the formula.</param>
    /// <param name="length">How many bytes the formula occupies.</param>
    /// <param name="version">Which BIFF generation the token layouts follow.</param>
    public static List<SheetRange> Read(BiffRecordReader stream, int length, BiffVersion version)
    {
        ArgumentNullException.ThrowIfNull(stream);

        List<SheetRange> ranges = [];
        if (length <= 0) return ranges;

        byte[] tokens = stream.ReadBytes(length);
        int at = 0;
        int maxRow = version == BiffVersion.Biff8 ? Biff8MaxRow : Biff5MaxRow;

        while (at < tokens.Length)
        {
            byte opcode = tokens[at++];

            // The three "classes" of a reference token — reference, value and array — differ
            // only in the top two bits of the opcode and describe the same rectangle, so they
            // are folded together before the layout is chosen.
            byte kind = (byte)(opcode >= 0x20 ? opcode & 0x3F : opcode);

            switch (kind)
            {
                case 0x3B when version == BiffVersion.Biff8: // tArea3d
                    if (!Take(tokens, ref at, 10)) return ranges;
                    Add(ranges, tokens, at - 8, wide: true, maxRow);
                    break;

                case 0x3B: // tArea3d, BIFF5: an external index and eight reserved bytes first
                    if (!Take(tokens, ref at, 16)) return ranges;
                    Add(ranges, tokens, at - 6, wide: false, maxRow);
                    break;

                case 0x3A when version == BiffVersion.Biff8: // tRef3d
                    if (!Take(tokens, ref at, 6)) return ranges;
                    AddCell(ranges, tokens, at - 4, wide: true);
                    break;

                case 0x3A: // tRef3d, BIFF5
                    if (!Take(tokens, ref at, 13)) return ranges;
                    AddCell(ranges, tokens, at - 3, wide: false);
                    break;

                case 0x25: // tArea, no sheet reference
                    if (!Take(tokens, ref at, version == BiffVersion.Biff8 ? 8 : 6)) return ranges;
                    Add(ranges, tokens, at - (version == BiffVersion.Biff8 ? 8 : 6),
                        version == BiffVersion.Biff8, maxRow);
                    break;

                case 0x24: // tRef
                    if (!Take(tokens, ref at, version == BiffVersion.Biff8 ? 4 : 3)) return ranges;
                    AddCell(ranges, tokens, at - (version == BiffVersion.Biff8 ? 4 : 3),
                        version == BiffVersion.Biff8);
                    break;

                case 0x26 or 0x28: // tMemArea, tMemNoMem: four reserved bytes and a length
                    if (!Take(tokens, ref at, 6)) return ranges;
                    break;

                case 0x29: // tMemFunc: a length, then the subexpression this loop keeps reading
                    if (!Take(tokens, ref at, 2)) return ranges;
                    break;

                case 0x10 or 0x0F or 0x11: // the union, intersection and range operators
                case 0x15: // tParen
                    break;

                case 0x19: // tAttr, which a name can carry as a "space before" hint
                    if (!Take(tokens, ref at, 3)) return ranges;
                    break;

                default:
                    // Not a plain range. Whatever has been read so far is still a real part of
                    // the name, so it is kept rather than discarded.
                    return ranges;
            }
        }

        return ranges;
    }

    private static bool Take(byte[] tokens, ref int at, int count)
    {
        if (at + count > tokens.Length) return false;
        at += count;
        return true;
    }

    /// <summary>
    /// Adds a rectangle, widening a whole-row or whole-column reference to Calc's own limits.
    /// </summary>
    /// <remarks>
    /// The widening is what makes a <c>Print_Titles</c> band recognisable. BIFF sheets are 256
    /// columns wide, so "every column" is stored as 0 to 255 — and the caller distinguishes a
    /// repeated-row band from a repeated-column one by whether it spans the whole sheet, which
    /// it cannot do against a limit of 255 that a real range might also reach.
    /// </remarks>
    private static void Add(List<SheetRange> ranges, byte[] tokens, int at, bool wide, int maxRow)
    {
        int firstRow = Read16(tokens, at);
        int lastRow = Read16(tokens, at + 2);
        int firstColumn;
        int lastColumn;

        if (wide)
        {
            firstColumn = Read16(tokens, at + 4) & 0x3FFF;
            lastColumn = Read16(tokens, at + 6) & 0x3FFF;
        }
        else
        {
            // BIFF5 keeps the relative flags in the row's top two bits and the column in one
            // byte, so the masks move from one field to the other.
            firstRow &= 0x3FFF;
            lastRow &= 0x3FFF;
            firstColumn = tokens[at + 4];
            lastColumn = tokens[at + 5];
        }

        ranges.Add(new SheetRange(
            Math.Min(firstColumn, lastColumn),
            Math.Min(firstRow, lastRow),
            lastColumn >= BiffMaxColumn ? SheetAddress.MaxColumn : Math.Max(firstColumn, lastColumn),
            lastRow >= maxRow ? SheetAddress.MaxRow : Math.Max(firstRow, lastRow)));
    }

    private static void AddCell(List<SheetRange> ranges, byte[] tokens, int at, bool wide)
    {
        int row = Read16(tokens, at);
        int column = wide ? Read16(tokens, at + 2) & 0x3FFF : tokens[at + 2];
        if (!wide) row &= 0x3FFF;

        ranges.Add(new SheetRange(column, row, column, row));
    }

    private static int Read16(byte[] tokens, int at) => tokens[at] | (tokens[at + 1] << 8);
}
