namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One stretch of a cell's text set in its own character format.
/// </summary>
/// <remarks>
/// <para>
/// A whole <see cref="SheetCellFormat"/> rather than a set of overrides, because the reader has
/// already resolved the cell's format and only has to lay the run's own statements over it. The
/// alignment, wrapping and rotation fields of a portion are therefore the cell's and are ignored
/// here: a run inside a cell cannot state them, and all three formats agree about that — ODF's
/// <c>text:span</c> carries a text style, SpreadsheetML's <c>rPr</c> a font, and BIFF's formatting
/// run nothing but a <c>FONT</c> index.
/// </para>
/// <para>
/// Portions partition the cell's text and never overlap, which is what
/// <see cref="Paperless.Text.Layout.FormattedRun"/> requires of the runs it measures across.
/// </para>
/// </remarks>
/// <param name="Start">The first character the portion covers, as an index into the cell's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Format">The format it is drawn in.</param>
public readonly record struct SheetTextPortion(int Start, int Length, SheetCellFormat Format)
{
    /// <summary>One past the portion's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// The cells of one sheet whose text is not all in one format.
/// </summary>
/// <remarks>
/// <para>
/// Kept apart from <see cref="SheetCellFormats"/> and shaped differently on purpose. A cell format
/// is pooled because a sheet has a handful of them and a million cells; rich text is the opposite —
/// every rich cell has its own list, and almost no cell is rich — so this is a plain dictionary of
/// the exceptions rather than a pool with a fallback chain. A sheet with none of them costs one
/// static instance.
/// </para>
/// <para>
/// Rendering only. Extraction reports a rich cell's text exactly as it reports any other cell's:
/// the runs change how the text is drawn and not what it says.
/// </para>
/// </remarks>
public sealed class SheetRichText
{
    private readonly Dictionary<(int Row, int Column), Entry> _cells;

    private SheetRichText(Dictionary<(int, int), Entry> cells) => _cells = cells;

    /// <summary>A sheet whose every cell is in one format.</summary>
    public static SheetRichText Empty { get; } = new([]);

    /// <summary>True when the sheet holds no rich cell at all.</summary>
    public bool IsEmpty => _cells.Count == 0;

    /// <summary>
    /// The portions a cell's text is split into, or null when it is all in one format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The text is checked rather than trusted, and it is the one guard this type needs. A cell's
    /// portions are read from the file's own runs — an <c>si</c>'s <c>r</c> elements, a cell's
    /// <c>text:span</c>s, a <c>LABELSST</c>'s formatting runs — and index into the string
    /// <em>stored</em> there, while what is drawn is that string run through the cell's number
    /// format. Almost always they are the same characters, since <c>General</c> and <c>@</c> both
    /// pass a string through untouched; a format code carrying literals of its own would shift
    /// every offset and mis-colour the whole cell. Comparing is cheaper than reasoning about which
    /// codes can do that, and it covers the three readers at once.
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="text">The text the cell draws, which the portions must describe.</param>
    public IReadOnlyList<SheetTextPortion>? At(int row, int column, string text)
        => _cells.TryGetValue((row, column), out Entry entry)
           && string.Equals(entry.Text, text, StringComparison.Ordinal)
            ? entry.Portions
            : null;

    private readonly record struct Entry(string Text, IReadOnlyList<SheetTextPortion> Portions);

    /// <summary>Accumulates a sheet's rich cells while it is being read.</summary>
    public sealed class Builder
    {
        private readonly Dictionary<(int, int), Entry> _cells = [];

        /// <summary>
        /// Records one cell's portions, normalising them against the text they describe.
        /// </summary>
        /// <remarks>
        /// Sorted, clipped to the text and gap-filled from the cell's own format, because the
        /// three formats state runs in three different ways and only one of them guarantees
        /// coverage: SpreadsheetML's <c>r</c> elements partition the string by construction, BIFF's
        /// formatting runs are start offsets with no end and routinely leave the leading characters
        /// unstated, and an ODF cell mixes bare text nodes with <c>text:span</c>s. Normalising here
        /// means the layouter never has to ask which.
        /// </remarks>
        /// <param name="row">The zero-based row.</param>
        /// <param name="column">The zero-based column.</param>
        /// <param name="text">The cell's text, which the portions index into.</param>
        /// <param name="cellFormat">What an unstated stretch of the text is drawn in.</param>
        /// <param name="portions">The stated portions, in any order.</param>
        public void Set(
            int row,
            int column,
            string text,
            SheetCellFormat cellFormat,
            IReadOnlyList<SheetTextPortion> portions)
        {
            ArgumentNullException.ThrowIfNull(text);
            ArgumentNullException.ThrowIfNull(cellFormat);
            ArgumentNullException.ThrowIfNull(portions);

            if (row < 0 || column < 0 || text.Length == 0) return;

            List<SheetTextPortion> normalised = Normalise(text.Length, cellFormat, portions);

            // One portion covering the whole cell in the cell's own format is not rich text: it is
            // what every cell already draws as, so recording it would cost a dictionary entry and
            // a different code path for no difference in the output.
            if (normalised.Count == 1 && normalised[0].Format == cellFormat) return;

            _cells[(row, column)] = new Entry(text, normalised);
        }

        /// <summary>The finished lookup.</summary>
        public SheetRichText Build() => _cells.Count == 0 ? Empty : new SheetRichText(_cells);

        private static List<SheetTextPortion> Normalise(
            int length, SheetCellFormat cellFormat, IReadOnlyList<SheetTextPortion> portions)
        {
            List<SheetTextPortion> sorted =
            [
                .. portions
                   .Where(portion => portion.Length > 0 && portion.Start < length)
                   .OrderBy(portion => portion.Start),
            ];

            List<SheetTextPortion> result = [];
            int at = 0;

            foreach (SheetTextPortion portion in sorted)
            {
                int start = Math.Max(portion.Start, at);
                int end = Math.Min(portion.End, length);
                if (end <= start) continue;

                if (start > at) result.Add(new SheetTextPortion(at, start - at, cellFormat));

                result.Add(new SheetTextPortion(start, end - start, portion.Format));
                at = end;
            }

            if (at < length) result.Add(new SheetTextPortion(at, length - at, cellFormat));
            if (result.Count == 0) result.Add(new SheetTextPortion(0, length, cellFormat));

            return result;
        }
    }
}
