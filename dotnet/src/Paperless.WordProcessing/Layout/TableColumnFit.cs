using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>Whose rules decide a width-less table's columns.</summary>
/// <remarks>
/// Two families and two entirely different answers for the same document, which is the whole reason this is
/// an enumeration rather than one algorithm: see <see cref="TableColumnFit"/>.
/// </remarks>
public enum TableWidthRule
{
    /// <summary>Writer's ODF importer, <c>SwXMLTableContext::MakeTable_</c>.</summary>
    OpenDocument,

    /// <summary>
    /// The Word family — DOCX, RTF and DOC — which all reach Writer through <c>dmapper</c>.
    /// </summary>
    Word,
}

/// <summary>
/// How to size a table's columns when the file does not state all of their widths.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is not content-based auto-layout, and that is the finding rather than the shortcut.</b> The
/// obvious guess — and the guess this item was written under — is that Writer measures each column's text
/// and sizes to it, the way CSS's automatic table layout does. It does not. Writer owns exactly one
/// content-measuring table layout, <c>SwHTMLTableLayout</c> in <c>sw/source/core/doc/htmltbl.cxx</c>, whose
/// <c>AutoLayoutPass1</c> calls <c>SwTextNode::GetMinMaxSize</c> per cell — and the only thing that ever
/// installs it is the HTML filter (<c>SwTable::SetHTMLTableLayout</c> is called from
/// <c>sw/source/filter/html/htmltab.cxx</c> and nowhere else). A table read from ODF, DOCX, RTF or DOC has
/// no such layout, so nothing about its columns depends on a single character of its text.
/// </para>
/// <para>
/// Measured rather than assumed: the corpus table with every column style stripped renders its columns at
/// 160.6, 107.1 and 214.1 pt — and moving the one long paragraph from the third column to the second, or
/// deleting it outright, moves nothing at all. The same experiment on the DOCX spelling is equally inert.
/// </para>
/// <para>
/// What the two families do instead is arithmetic on the *declared* grid, and they disagree:
/// </para>
/// <list type="bullet">
///   <item>
///     <b>ODF</b> gives a column stating no width <c>MINLAY</c> — 23 twips — as a <em>relative</em> width
///     and then distributes (<see cref="ResolveOpenDocument"/>). Which of two distributions depends on
///     whether the table itself states an absolute width, and they give different answers for the same
///     columns: three width-less columns come out equal when it does not and in the ratio 3:2:4 when it
///     does. The 3:2:4 is the 160.6/107.1/214.1 above.
///   </item>
///   <item>
///     <b>Word</b> reaches Writer through <c>dmapper</c>, which states the grid as relative column
///     separators and lets Writer place the dividers of a table that starts out with equal columns. A
///     separator at zero — which is what a width-less column produces — is silently dropped, so its divider
///     stays where the equal division put it (<see cref="ResolveWord"/>).
///   </item>
/// </list>
/// </remarks>
public sealed record TableColumnFit
{
    /// <summary>Writer's minimum frame width, 23 twips, which is what a width-less ODF column gets.</summary>
    /// <remarks><c>sw/inc/swtypes.hxx</c>, <c>#define MINLAY 23</c>.</remarks>
    public const int MinLay = 23;

    /// <summary>The relative unit <c>dmapper</c> states a column separator in.</summary>
    /// <remarks>
    /// <c>UNO_TABLE_COLUMN_SUM</c>, 10000. A grid of 1:2:1 and one of 20:40:20 are the same table, which is
    /// why the separators are relative and the table's own width is stated apart from them.
    /// </remarks>
    public const int ColumnSum = 10000;

    /// <summary>True for each column the file left without a width of its own.</summary>
    public required IReadOnlyList<bool> IsAuto { get; init; }

    /// <summary>
    /// The table's own declared width, or null when it states none and takes whatever it is given.
    /// </summary>
    /// <remarks>
    /// Null is not "zero" and not "the columns added up": it is Writer's <c>HoriOrientation::FULL</c>, where
    /// the table frame is as wide as the area it sits in — the body's text width for a body table, the
    /// enclosing cell's for a nested one. ODF reaches it whenever <c>table:align</c> is absent or says
    /// <c>margins</c>, and then <em>ignores a stated width</em>: <c>SwXMLTableContext::MakeTable</c>,
    /// "Even if a size is specified, it will be ignored!".
    /// </remarks>
    public Length? TableWidth { get; init; }

    /// <summary>Which family's distribution applies.</summary>
    public required TableWidthRule Rule { get; init; }

    /// <summary>
    /// The columns' final widths.
    /// </summary>
    /// <param name="declared">
    /// What the file stated per column; the entries <see cref="IsAuto"/> marks are ignored.
    /// </param>
    /// <param name="available">
    /// How wide the table may be — the body's text width, or the enclosing cell's inner width for a nested
    /// table. Used only when <see cref="TableWidth"/> is null.
    /// </param>
    public IReadOnlyList<Length> Resolve(IReadOnlyList<Length> declared, Length available)
    {
        ArgumentNullException.ThrowIfNull(declared);

        int count = declared.Count;
        if (count == 0) return declared;

        // Everything below is integer arithmetic in twips, because Writer's is: the answers differ from the
        // same sums in EMUs by a twip here and there, and a twip is what a comparison against a reference
        // rendering resolves.
        int[] widths = new int[count];
        for (int i = 0; i < count; i++) widths[i] = (int)Math.Clamp(declared[i].Twips, 0, int.MaxValue);

        bool[] auto = new bool[count];
        for (int i = 0; i < count; i++) auto[i] = i < IsAuto.Count && IsAuto[i];

        int total = TableWidth is { } stated
            ? (int)Math.Clamp(stated.Twips, 0, int.MaxValue)
            : (int)Math.Clamp(available.Twips, 0, int.MaxValue);

        if (total <= 0) return declared;

        int[] result = Rule == TableWidthRule.OpenDocument
            ? ResolveOpenDocument(widths, auto, total, TableWidth is not null)
            : ResolveWord(widths, auto, total);

        return [.. result.Select(twips => Length.FromTwips(twips))];
    }

    /// <summary>
    /// Writer's ODF importer, ported: <c>SwXMLTableContext::MakeTable_</c>,
    /// <c>sw/source/filter/xml/xmltbli.cxx</c>:2202.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every column carries a width and a flag saying whether it is <em>relative</em>. A column stating a
    /// <c>style:column-width</c> is absolute; one stating nothing is relative and worth
    /// <see cref="MinLay"/> (line 693, <c>sal_Int32 nWidth = MINLAY; bool bRelWidth = true;</c>). The
    /// table is relative too unless <c>table:align</c> gave it a real orientation and it stated a width.
    /// </para>
    /// <para>
    /// <b>The absolute branch does not divide proportionally, and this is the trap.</b> It computes each
    /// relative column as <c>width × remaining / totalRelative</c> while decrementing <c>remaining</c> and
    /// never <c>totalRelative</c>, so three equal columns come out 1/3, 2/9, 4/9 — the ratio 3:2:4 — and
    /// four come out 16:12:9:27. That is not a plausible reading of the file and it is not a rounding
    /// artefact; it is what LibreOffice renders, verified for three columns and four, and any port that
    /// "fixed" it would disagree with every reference rendering by tens of points. The last relative column
    /// takes the remainder outright, which is why two columns come out right and three do not.
    /// </para>
    /// </remarks>
    /// <param name="widths">Each column's declared width in twips.</param>
    /// <param name="auto">True for each column that stated none.</param>
    /// <param name="total">The width to fill.</param>
    /// <param name="tableIsAbsolute">True when the table itself stated a width to be honoured.</param>
    private static int[] ResolveOpenDocument(int[] widths, bool[] auto, int total, bool tableIsAbsolute)
    {
        int count = widths.Length;

        // InsertColumn clamps every declared width onto [MINLAY, MAX_WIDTH] (xmltbli.cxx:1333). A column
        // stating nothing arrives as MINLAY, relative.
        int[] column = new int[count];
        bool[] relative = new bool[count];
        for (int i = 0; i < count; i++)
        {
            relative[i] = auto[i];
            column[i] = auto[i] ? MinLay : Math.Clamp(widths[i], MinLay, ushort.MaxValue);
        }

        // MakeTable clamps the table's own width up to one MINLAY per column before MakeTable_ runs.
        if (tableIsAbsolute) total = Math.Max(total, Math.Min(count * MinLay, ushort.MaxValue));

        int absoluteWidth = 0, minAbsolute = 0, relativeWidth = 0, minRelative = 0, relativeCount = 0;
        for (int i = 0; i < count; i++)
        {
            if (relative[i])
            {
                relativeWidth += column[i];
                if (minRelative == 0 || column[i] < minRelative) minRelative = column[i];
                relativeCount++;
            }
            else
            {
                absoluteWidth += column[i];
                if (minAbsolute == 0 || column[i] < minAbsolute) minAbsolute = column[i];
            }
        }

        int absoluteCount = count - relativeCount;

        if (!tableIsAbsolute)
        {
            // The relative branch. Absolute columns are first restated as relative ones in the same
            // proportion, pegged so that the narrowest of them is worth as much as the narrowest column
            // that was already relative — which is why a width-less column beside a 3 cm and a 5 cm one
            // comes out exactly as wide as the 3 cm, whatever the page is.
            if (absoluteCount > 0 && minAbsolute > 0)
            {
                if (minRelative == 0) minRelative = minAbsolute;

                for (int i = 0; i < count && absoluteCount > 0; i++)
                {
                    if (relative[i]) continue;

                    int share = (int)((long)column[i] * minRelative / minAbsolute);
                    column[i] = share;
                    relative[i] = true;
                    relativeWidth += share;
                    absoluteCount--;
                }
            }

            if (relativeWidth == 0) return column;

            // Writer scales to MAX_WIDTH here and lets HoriOrientation::FULL fit the frame to its area
            // afterwards; scaling straight to the area is the same answer to within a twip and skips a
            // round trip through a 16-bit ceiling.
            double scale = (double)total / relativeWidth;
            int used = 0;
            for (int i = 0; i < count - 1; i++)
            {
                column[i] = (int)(column[i] * scale);
                used += column[i];
            }

            column[^1] = total - used;
            return column;
        }

        // The absolute branch, where the accumulator bug lives.
        if (relativeCount > 0)
        {
            int forRelative = total > absoluteWidth ? total - absoluteWidth : 0;
            int extraRelative = relativeWidth - (relativeCount * minRelative);
            int minimumAbsolute = relativeCount * MinLay;
            int extraAbsolute = forRelative > minimumAbsolute ? forRelative - minimumAbsolute : 0;

            bool allMinimum = false;
            bool minimumPlusExtra = false;

            if (forRelative <= minimumAbsolute)
            {
                forRelative = minimumAbsolute;
                allMinimum = true;
            }
            else if (minRelative > 0 && forRelative <= (long)relativeWidth * MinLay / minRelative)
            {
                minimumPlusExtra = true;
            }

            for (int i = 0; i < count && relativeCount > 0; i++)
            {
                if (!relative[i]) continue;

                int share;
                if (relativeCount == 1)
                {
                    // The last relative column takes whatever is left, which is what keeps the total exact.
                    share = forRelative;
                }
                else if (allMinimum)
                {
                    share = MinLay;
                }
                else if (minimumPlusExtra)
                {
                    share = extraRelative == 0
                        ? MinLay
                        : MinLay + (int)((long)(column[i] - minRelative) * extraAbsolute / extraRelative);
                }
                else
                {
                    share = (int)((long)column[i] * forRelative / relativeWidth);
                }

                column[i] = share;
                relative[i] = false;
                forRelative -= share;
                absoluteWidth += share;
                relativeCount--;
            }
        }

        if (absoluteWidth <= 0) return column;

        if (absoluteWidth < total)
        {
            // The stated columns are narrower than the stated table, so each is widened in proportion and
            // the last one absorbs the rounding.
            int extra = total - absoluteWidth;
            int last = column[^1] + extra;
            for (int i = 0; i < count - 1; i++)
            {
                int share = (int)((long)column[i] * extra / absoluteWidth);
                column[i] += share;
                last -= share;
            }

            column[^1] = last;
        }
        else if (absoluteWidth > total)
        {
            // Wider than the table, so every column falls back to the minimum plus a proportional share of
            // whatever is left over — not a proportional shrink, which would leave nothing for a narrow one.
            int extra = total - (count * MinLay);
            int last = MinLay + extra;
            for (int i = 0; i < count - 1; i++)
            {
                int share = (int)((long)column[i] * extra / absoluteWidth);
                column[i] = MinLay + share;
                last -= share;
            }

            column[^1] = last;
        }

        return column;
    }

    /// <summary>
    /// The Word family's, ported from <c>dmapper</c> and <c>SwTable::NewSetTabCols</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word's grid never reaches Writer as widths. <c>DomainMapperTableManager::endOfRowAction</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableManager.cxx</c>:735) turns it into a sequence of
    /// <c>TableColumnSeparator</c>s — cumulative positions out of <see cref="ColumnSum"/> — and the table
    /// itself is created with <em>equal</em> columns before they are applied.
    /// </para>
    /// <para>
    /// <b>A separator at zero is dropped rather than applied.</b> <c>SwTable::NewSetTabCols</c>
    /// (<c>sw/source/core/table/swtable.cxx</c>:1195) records a divider's move only
    /// <c>if( nOldPos != nNewPos &amp;&amp; nNewPos &gt; 0 &amp;&amp; nOldPos &gt; 0 )</c>, so a width-less
    /// column's divider simply stays where the equal division put it. That single condition is the whole of
    /// Word's answer: a grid of all zeroes moves no divider at all and the table stays equal, and a grid of
    /// <c>0, 2835, 5102</c> moves only the second — measured at 160.6, 11.5 and 309.7 pt, where a
    /// proportional reading would have given nothing, 172.1 and 309.7.
    /// </para>
    /// </remarks>
    /// <param name="widths">Each column's declared width in twips; zero for one stating none.</param>
    /// <param name="auto">True for each column that stated none.</param>
    /// <param name="total">The table's width.</param>
    private static int[] ResolveWord(int[] widths, bool[] auto, int total)
    {
        int count = widths.Length;
        int[] column = new int[count];

        int grid = 0;
        for (int i = 0; i < count; i++) grid += auto[i] ? 0 : Math.Max(0, widths[i]);

        // Where the dividers start: equal columns, as SwDoc::InsertTable makes them.
        int[] edge = new int[count];
        for (int i = 0; i < count; i++) edge[i] = (int)((long)total * (i + 1) / count);

        if (grid > 0)
        {
            long running = 0;
            for (int i = 0; i < count - 1; i++)
            {
                running += auto[i] ? 0 : Math.Max(0, widths[i]);

                // The relative separator, then back into twips of this table's width. Both conversions are
                // Writer's own, and both truncate.
                long separator = running * ColumnSum / grid;
                int moved = (int)(separator * total / ColumnSum);

                if (moved > 0) edge[i] = moved;
            }
        }

        // A guard rather than a port: the dividers above are placed one at a time and a mixture of moved
        // and unmoved ones can cross. Writer resolves that inside lcl_AdjustWidthsInLine; here it is enough
        // that no column comes out negative.
        int previous = 0;
        for (int i = 0; i < count; i++)
        {
            int at = i == count - 1 ? total : Math.Clamp(edge[i], previous, total);
            column[i] = at - previous;
            previous = at;
        }

        return column;
    }
}
