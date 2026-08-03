using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A row height stated without <c>customHeight</c> is the writer's cache, and Calc replaces it.
/// </summary>
/// <remarks>
/// <para>
/// <c>WorksheetGlobals::convertRows</c> imports the stated height whatever the flag says —
/// "always import the row height, ensures better layout" — and calls <c>SetManualHeight</c> only
/// when <c>mbCustomHeight</c> (<c>sc/source/filter/oox/worksheethelper.cxx:1268-1286</c>). Every
/// other row is then re-derived from its content before anything is drawn, by
/// <c>WorkbookGlobals::finalize</c> (<c>sc/source/filter/oox/workbookhelper.cxx:659</c>).
/// </para>
/// <para>
/// The expected numbers are LibreOffice 24.2.7.2's own, read out of its flat-ODF export of this
/// fixture: <c>0.3398in</c>, <c>0.2083in</c>, <c>0.5555in</c> and <c>0.1772in</c>, which are 489,
/// 300, 800 and 255 twips. Asserting those rather than the formula's output is the point — the
/// formula was derived from them, so a test of the formula against itself would prove nothing.
/// </para>
/// <para>
/// The four rows are chosen to separate the three things that decide the answer. Row 1 is the
/// arithmetic on its own; row 2 is the same arithmetic at a different size; row 3 states
/// <c>customHeight</c> and must be left exactly as written; row 4 asks for less than the sheet's
/// <c>defaultRowHeight</c> and is held up to it, which is the only place
/// <c>ScTable::GetOptimalMinRowHeight</c> shows.
/// </para>
/// </remarks>
public sealed class SheetOptimalRowHeightTests
{
    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void AHintedHeightIsRederivedFromTheFontSize()
    {
        SheetAxis rows = Sheet("sheet-row-height-hint.xlsx").Grid.Rows;

        // 20 pt: trunc(400 x 1.18) = 472, plus 40 twips of margin, less 23.
        rows.SizeAt(0).Twips.ShouldBe(489, "LibreOffice writes 0.3398in for this row");

        // 12 pt: trunc(240 x 1.18) = 283, plus 40, less 23 — the National-Reports figure.
        rows.SizeAt(1).Twips.ShouldBe(300, "LibreOffice writes 0.2083in for this row");
    }

    [Fact]
    public void AChosenHeightIsLeftAlone()
    {
        SheetAxis rows = Sheet("sheet-row-height-hint.xlsx").Grid.Rows;

        // Same font and same stated height as row 2, and customHeight on it.
        rows.SizeAt(2).Twips.ShouldBe(800, "LibreOffice writes 0.5555in for this row");
    }

    [Fact]
    public void AHintedHeightIsHeldUpToTheSheetMinimum()
    {
        SheetAxis rows = Sheet("sheet-row-height-hint.xlsx").Grid.Rows;

        // 8 pt asks for trunc(160 x 1.18) + 40 - 23 = 205, and the sheet's defaultRowHeight of
        // 12.75 pt is 255 twips.
        rows.SizeAt(3).Twips.ShouldBe(255, "LibreOffice writes 0.1772in for this row");
    }
}
