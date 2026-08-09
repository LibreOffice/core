using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A collapsed outline group hides its detail rows even when nothing says they are hidden.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML states an <c>outlineLevel</c> per row and a <c>collapsed</c> flag on the summary
/// row beside a group, and leaves the consequence to be derived. Excel usually also writes
/// <c>hidden="1"</c> on every detail row, which is why this is invisible on most files — and a
/// file whose writer omitted it paginates entirely differently without the derivation. Measured
/// on <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>, whose sheet 3 states no
/// <c>hidden</c> anywhere: <strong>329 of its 1033 rows</strong> are hidden by this rule alone,
/// and the workbook printed 18 pages against LibreOffice's 14 and drew 26353 extractable words
/// against 17718.
/// </para>
/// <para>
/// The rule is <c>WorksheetGlobals::convertOutlines</c>
/// (<c>sc/source/filter/oox/worksheethelper.cxx:1307-1334</c>) — see
/// <see cref="SheetOutlineCollapse"/> for the algorithm and the citations.
/// </para>
/// <para>
/// <c>sheet-outline-collapse.xlsx</c> is authored to separate the three decisions the rule makes
/// rather than copied from that workbook, and every expectation below is LibreOffice 24.2.7.2's
/// own answer, read out of its flat-ODF export (<c>table:visibility="collapse"</c>) and its PDF:
/// </para>
/// <list type="bullet">
/// <item>rows 4–8 are a level-1 group and row 9 closes it carrying <c>collapsed</c> — hidden;</item>
/// <item>rows 10–14 are the same shape and row 15 closes it <em>without</em> the flag — visible,
/// which is what stops the rule from being "any grouped row is hidden";</item>
/// <item>rows 16–18 are level 1 and 19–21 level 2, and row 22 closes the inner group carrying the
/// flag — only 19–21 are hidden, because <c>bCollapsed</c> is cleared after the first pop, and
/// 16–18 stay visible although their group is closed by the same walk.</item>
/// </list>
/// <para>
/// Verified by reintroducing the defect: with the two <c>SheetOutlineCollapse.Apply</c> calls
/// removed from <c>XlsxPrintSetup.ReadGrid</c> — which is the code as it stood before round
/// thirty-one — every case here fails.
/// </para>
/// </remarks>
public sealed class SheetOutlineCollapseTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-outline-collapse.xlsx"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void ACollapsedGroupHidesItsDetailRows()
    {
        SheetGrid grid = Sheet().Grid;

        // Rows are zero-based here and one-based in the file.
        for (int row = 3; row <= 7; row++)
            grid.Rows.IsHidden(row).ShouldBeTrue($"row {row + 1}");
    }

    [Fact]
    public void TheSummaryRowThatCarriesTheFlagStaysVisible()
    {
        SheetGrid grid = Sheet().Grid;

        grid.Rows.IsHidden(8).ShouldBeFalse("row 9 closes the group and is not part of it");
        grid.Rows.IsHidden(2).ShouldBeFalse("row 3 is outside the group");
    }

    [Fact]
    public void AGroupClosedWithoutTheFlagKeepsItsRows()
    {
        SheetGrid grid = Sheet().Grid;

        for (int row = 9; row <= 13; row++)
            grid.Rows.IsHidden(row).ShouldBeFalse($"row {row + 1}");
    }

    [Fact]
    public void OnlyTheInnermostGroupOneRowClosesIsCollapsed()
    {
        SheetGrid grid = Sheet().Grid;

        for (int row = 18; row <= 20; row++)
            grid.Rows.IsHidden(row).ShouldBeTrue($"row {row + 1}, the inner group");

        for (int row = 15; row <= 17; row++)
            grid.Rows.IsHidden(row).ShouldBeFalse($"row {row + 1}, the outer group");
    }

    /// <summary>
    /// The whole point of the rule, stated as the thing a reader can see: the hidden rows do not
    /// print.
    /// </summary>
    /// <remarks>
    /// LibreOffice's own PDF of this fixture draws R1, R2, R3, R9, R10–R18, R22 and R23 and
    /// nothing else — seventeen of the twenty-three rows.
    /// </remarks>
    [Fact]
    public void TheHiddenRowsAreNotDrawn()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-outline-collapse.xlsx"));

        RecordingDrawingSink sink = new();
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        pages.Count.ShouldBe(1);
        pages.Pages[0].Draw(sink);

        HashSet<string> drawn = [.. sink.Pages[0].Runs.Select(run => run.Text.Trim())];

        foreach (int row in (int[])[1, 2, 3, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23])
            drawn.ShouldContain($"R{row}");

        foreach (int row in (int[])[4, 5, 6, 7, 8, 19, 20, 21])
            drawn.ShouldNotContain($"R{row}");
    }

    /// <summary>
    /// The rule itself, over the runs a reader hands it, with the gap behaviour a sheet that
    /// states no row for part of its range depends on.
    /// </summary>
    /// <remarks>
    /// A gap takes the default model — level zero, no flag — so it closes every open group and
    /// collapses none of them (<c>convertRows</c>, <c>worksheethelper.cxx:1243-1257</c>). Without
    /// that, a group left open by a sheet that simply stops stating rows would swallow everything
    /// after it.
    /// </remarks>
    [Fact]
    public void AGapClosesAnOpenGroupWithoutCollapsingIt()
    {
        List<SheetOutlineRun> runs = [];
        SheetOutlineCollapse.Append(runs, 0, 2, 0, false);
        SheetOutlineCollapse.Append(runs, 3, 7, 1, false);

        // Nothing for rows 8 onwards, then a collapsed row well past the gap.
        SheetOutlineCollapse.Append(runs, 20, 20, 0, true);

        SheetOutlineCollapse.Hidden(runs).ShouldBeEmpty();
    }

    /// <summary>Consecutive rows agreeing on both fields merge into one run.</summary>
    [Fact]
    public void AppendMergesRunsThatAgree()
    {
        List<SheetOutlineRun> runs = [];
        for (int row = 0; row < 5; row++) SheetOutlineCollapse.Append(runs, row, row, 1, false);

        runs.Count.ShouldBe(1);
        runs[0].ShouldBe(new SheetOutlineRun(0, 4, 1, false));
    }
}
