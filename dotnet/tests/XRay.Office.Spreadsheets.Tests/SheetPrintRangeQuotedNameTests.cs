using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A print range is a list separated by a character a sheet name may itself contain.
/// </summary>
/// <remarks>
/// <para>
/// ODF's <c>table:print-ranges</c> is a <em>space</em>-separated list of range addresses in the
/// OOO convention, and a sheet name holding a space is written quoted — which Calc's own export
/// does on every range it writes, because it always qualifies both ends with the sheet name. So
/// a perfectly ordinary sheet called <c>Q1 Sales</c> produces
/// <c>'Q1 Sales'.B2:'Q1 Sales'.C4</c>, and splitting that on every space gives three fragments.
/// </para>
/// <para>
/// The failure is silent rather than loud, which is why it survived: two of the three fragments
/// fail to parse and the third — <c>Sales'.C4</c> — parses, because stripping the sheet
/// qualifier takes everything up to the last dot. It yields the single cell the range ended at.
/// A valid print area comes back, the sheet paginates to one page holding one cell, and nothing
/// anywhere reports a problem.
/// </para>
/// <para>
/// Calc tokenises the same list with <c>ScRangeStringConverter::GetTokenByOffset</c>, whose
/// <c>IndexOf</c> carries a <c>bQuoted</c> flag toggled by every apostrophe and stops at the
/// separator only outside one (<c>sc/source/core/tool/rangeutl.cxx</c>:552-576, :35-56). The
/// separator defaults to a space and the quote to an apostrophe
/// (<c>sc/inc/rangeutl.hxx</c>:119-120). Toggling per apostrophe is also what makes the OOO
/// convention's doubled <c>''</c> escape inside a name come out right: two toggles leave the
/// flag where it was.
/// </para>
/// <para>
/// SpreadsheetML has the identical problem with a comma — <c>'Q1, Q2'!$A$1</c> — and
/// <c>XlsxPrintNames</c> had solved it there since before this was found, which is why the
/// splitting is now <see cref="SheetAddress.SplitList"/>'s and only the separator is the
/// format's.
/// </para>
/// <para>
/// Ground truth for the fixture is 26.2.4.2's own PDF of it: one page carrying B2, B3, B4, C2,
/// C3 and C4 and nothing else.
/// </para>
/// </remarks>
public sealed class SheetPrintRangeQuotedNameTests
{
    /// <summary>The attribute Calc writes for a range on a sheet whose name holds a space.</summary>
    private const string Declared = "'Q1 Sales'.B2:'Q1 Sales'.C4";

    /// <summary>A separator inside a quoted name does not end a token.</summary>
    [Fact]
    public void AQuotedSheetNameKeepsItsSeparator()
    {
        SheetAddress.SplitList(Declared, ' ').ShouldBe([Declared]);

        // The same list with two ranges on it still splits into two, so the fix is not simply a
        // refusal to split.
        SheetAddress.SplitList("'Q1 Sales'.B2:'Q1 Sales'.C4 'Q1 Sales'.E1", ' ')
                    .ShouldBe(["'Q1 Sales'.B2:'Q1 Sales'.C4", "'Q1 Sales'.E1"]);

        // A doubled apostrophe is the OOO convention's escape for one inside a name. Two toggles
        // leave the flag where it was, so the space after it is still inside the quotes.
        SheetAddress.SplitList("'O''Hara Q1'.A1:'O''Hara Q1'.B2", ' ')
                    .ShouldBe(["'O''Hara Q1'.A1:'O''Hara Q1'.B2"]);

        // And the comma spelling, which is the same function with the other separator.
        SheetAddress.SplitList("'Q1, Q2'!$A$1,'Q1, Q2'!$C$3", ',')
                    .ShouldBe(["'Q1, Q2'!$A$1", "'Q1, Q2'!$C$3"]);
    }

    /// <summary>The declared range survives the split, rather than collapsing to its last cell.</summary>
    [Fact]
    public void ThePrintAreaIsTheDeclaredBlockAndNotItsLastCell()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(
                DocumentSource.FromFile(Corpus.Require("sheet-print-range-quoted.fods")));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetPrintSetup setup = pages.Sheets[0].Setup;

        setup.PrintAreas.Count.ShouldBe(1);

        // B2:C4, zero-based: columns 1 to 2, rows 1 to 3. The naive split's answer is the single
        // cell C4 — (2, 3, 2, 3) — and the whole sheet is (0, 0, 3, 5).
        setup.PrintAreas[0].ShouldBe(new SheetRange(1, 1, 2, 3));
    }

    /// <summary>And the page holds that block's cells and no others.</summary>
    /// <remarks>
    /// The assertion is on the drawn words rather than on the range alone because the range is
    /// only half of it: a print area is cut back to the used range before it paginates, and the
    /// three readings this fixture separates — the declared block, the whole sheet and the single
    /// last cell — all give exactly one page. What tells them apart is what is on it.
    /// </remarks>
    [Fact]
    public void OnlyTheDeclaredBlockIsPrinted()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(
                DocumentSource.FromFile(Corpus.Require("sheet-print-range-quoted.fods")));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        pages.Count.ShouldBe(1);

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        List<string> drawn =
            [.. DrawnWords.On(sink.Pages[0]).Select(word => word.Text).Order(StringComparer.Ordinal)];

        drawn.ShouldBe(["B2", "B3", "B4", "C2", "C3", "C4"]);
    }
}
