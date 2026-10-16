using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// The row grid the reference re-derives for a <c>.ppt</c> table group, and the map from the
/// group's own rectangles onto it.
/// </summary>
/// <remarks>
/// <para>
/// A <c>.ppt</c> writes a table as a group of rectangles and lines, and the reference throws the
/// group away: <c>CreateTable</c> (<c>filter/source/msfilter/svdfppt.cxx</c>:7569) builds one
/// <c>SdrTableObj</c> whose rows are <em>seeded</em> from the members' snap-rect tops —
/// <c>CreateTableRows</c> at :7339, the last row's height measured to the group's own bottom —
/// and then re-laid out by <c>TableLayouter::LayoutTableHeight</c>
/// (<c>svx/source/table/tablelayouter.cxx</c>:724). That takes <c>max(stated, minimum)</c> per
/// row, where the minimum is the tallest single-row cell's own
/// <c>Cell::getMinimumHeight</c> (<c>svx/source/table/cell.cxx</c>:686) — its text measured at the
/// cell's inner width, <b>plus one unit</b>, plus the cell's upper and lower text distances — and
/// a cell spanning rows only ever grows its <em>last</em> row.
/// </para>
/// <para>
/// So the group's own rectangles are a lower bound on the reference's rows and never the answer.
/// On <c>Thailand17.ppt</c> page 11 the rectangles are a 38.75 pt header and eight rows of 35.87;
/// the reference draws <b>39.20 and eight of 36.20</b>, 0.33 pt per row and cumulative.
/// </para>
/// <para>
/// <strong>Two things the reference does here are not what they look like.</strong> The rows are
/// <em>sized</em> from the first member's top but <em>positioned</em> from the group's snap top,
/// because <c>CreateTable</c> hands the table object the group's rectangle
/// (<c>svdfppt.cxx</c>:7712) and the layouter lays out inside it; and the deferred minimum of a
/// row-spanning cell has the rows it covers subtracted from it only when the span does
/// <em>not</em> start at row 0 — <c>(nMRow &gt; 0) &amp;&amp; (nMRow &lt; nRow)</c> fails on its
/// first evaluation and the whole loop is skipped (<c>tablelayouter.cxx</c>:830). Both are
/// reproduced rather than corrected.
/// </para>
/// </remarks>
internal sealed class PptTableRows
{
    private readonly long[] _stated;
    private readonly long[] _laid;

    private PptTableRows(long[] stated, long[] laid)
    {
        _stated = stated;
        _laid = laid;
    }

    /// <summary>
    /// Builds the grid, or returns null when nothing moves.
    /// </summary>
    /// <param name="edges">
    /// The stated row edges in the group's own space, ascending: every member rectangle's top,
    /// then the group's bottom.
    /// </param>
    /// <param name="top">The group's own top, which is where the laid-out table starts.</param>
    /// <param name="bottom">The group's own bottom; the frame the finished table is fitted to.</param>
    /// <param name="minimums">Each row's minimum height, in the same space.</param>
    internal static PptTableRows? Of(long[] edges, long top, long bottom, long[] minimums)
    {
        ArgumentNullException.ThrowIfNull(edges);
        ArgumentNullException.ThrowIfNull(minimums);
        if (edges.Length < 2 || minimums.Length != edges.Length - 1) return null;

        long[] sizes = new long[minimums.Length];
        long total = 0;

        for (int row = 0; row < sizes.Length; row++)
        {
            sizes[row] = Math.Max(edges[row + 1] - edges[row], minimums[row]);
            total += sizes[row];
        }

        Distribute(sizes, minimums, (bottom - top) - total);

        long[] laid = new long[edges.Length];
        laid[0] = top;
        bool moves = top != edges[0];

        for (int row = 0; row < sizes.Length; row++)
        {
            laid[row + 1] = laid[row] + sizes[row];
            if (sizes[row] != edges[row + 1] - edges[row]) moves = true;
        }

        return moves ? new PptTableRows(edges, laid) : null;
    }

    /// <summary>
    /// <c>TableLayouter::distribute</c>, which is what fits a grown table back into its frame.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The second layout is the one that decides, and it is a shrink.</strong>
    /// <c>CreateTable</c> fills the cells first — every one of which re-lays the table out at
    /// <c>max(stated, minimum)</c> — and only then hands it the group's rectangle
    /// (<c>svdfppt.cxx</c>:7712). <c>SdrTableObj::NbcSetLogicRect</c>
    /// (<c>svx/source/table/svdotable.cxx</c>:1925-1938) sees the height change and passes
    /// <c>bFitHeight</c> <em>true</em> into the layout, so
    /// <c>TableLayouter::LayoutTableHeight</c>:849 calls
    /// <c>distribute(maRows, frame - current)</c> with a negative amount.
    /// </para>
    /// <para>
    /// <strong>And it does not shrink proportionally.</strong> The loop
    /// (<c>tablelayouter.cxx</c>:499-547) takes each row's share of the deficit, clamps any row
    /// that fell below its minimum, and runs again — but <c>nDistribute</c> is <em>not</em> reset
    /// between runs, and the clamping step subtracts the shortfall from it a second time. So a
    /// deficit large enough to push one row under its minimum drives every remaining row down to
    /// its own minimum. Measured: <c>2015-Civil-Rights-Website-training.ppt</c> page 48 states
    /// rows of 1270, 1102, 1098, 2037 and 4150 hundredths of a millimetre and minimums of 1108,
    /// 1108, 1108, 1108 and 5024, and 26.2.4.2's own flat ODP gives all four upper rows
    /// <c>1.108cm</c> and the last <c>5.024cm</c> — every one of them the minimum, none of them
    /// the stated height, and 201 short of the frame.
    /// </para>
    /// <para>
    /// The one hundred iterations are the reference's own guard against a developer error, and it
    /// exits as soon as no row is under its minimum.
    /// </para>
    /// </remarks>
    private static void Distribute(long[] sizes, long[] minimums, long amount)
    {
        for (int safety = 0; safety < 100; safety++)
        {
            bool broken = false;

            for (int i = 0; i < sizes.Length; i++)
            {
                if (sizes[i] >= minimums[i]) continue;

                amount -= minimums[i] - sizes[i];
                sizes[i] = minimums[i];
            }

            long current = 0;
            for (int i = 0; i < sizes.Length; i++)
            {
                if (amount > 0 || sizes[i] > minimums[i]) current += sizes[i];
            }

            if (current == 0 || amount == 0) return;

            long left = amount;
            for (int i = 0; i < sizes.Length; i++)
            {
                if (amount <= 0 && sizes[i] <= minimums[i]) continue;

                long share = i == sizes.Length - 1 ? left : amount * sizes[i] / current;
                sizes[i] += share;
                left -= share;

                if (sizes[i] < minimums[i]) broken = true;
            }

            if (!broken) return;
        }
    }

    /// <summary>The rectangle a member occupies once the table has been laid out.</summary>
    internal DocRect Remap(DocRect local)
    {
        long top = At(local.Y.Emu);
        long bottom = At(local.Y.Emu + local.Height.Emu);

        return new DocRect(
            local.X, Length.FromEmu(top), local.Width, Length.FromEmu(bottom - top));
    }

    /// <summary>
    /// Where one coordinate goes: the edge at or above it, plus whatever it sat past that edge.
    /// </summary>
    private long At(long y)
    {
        int index = Array.BinarySearch(_stated, y);
        if (index < 0) index = Math.Max(0, ~index - 1);

        return _laid[index] + (y - _stated[index]);
    }
}
