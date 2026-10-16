using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// When a numeric cell that will not fit draws <c>###</c>, and when it draws a shorter number
/// instead.
/// </summary>
/// <remarks>
/// <para>
/// The rule is <c>ScDrawStringsVars::SetTextToWidthOrHash</c>
/// (<c>sc/source/ui/view/output2.cxx:610-716</c>), reached only when
/// <c>bCellIsValue &amp;&amp; (mbLeftClip || mbRightClip)</c> (<c>:1974</c>). It has three
/// branches, and only the middle one was implemented here before this file existed:
/// </para>
/// <list type="number">
/// <item>a format other than <c>General</c> hashes <strong>outright</strong>, with no attempt to
/// shorten;</item>
/// <item>a <c>General</c> format is re-rendered with as many characters as the column has
/// max-digit widths, dropping decimals and then falling back to scientific notation;</item>
/// <item><strong>and if the re-rendered text is <em>still</em> wider than the column, the cell
/// hashes after all</strong> — "Even after the decimal adjustment the text doesn't fit. Give
/// up." (<c>:704-710</c>).</item>
/// </list>
/// <para>
/// Leaving out (3) is what made a column 0.43 characters wide draw <c>1E+00</c> where Calc draws
/// <c>###</c>. Measured on
/// <c>ODs-February-2022-Airbus-Commercial-Aircraft.xlsx</c>: it holds <strong>exactly 1101</strong>
/// numeric cells in a column A of width 0.42578125, LibreOffice's PDF holds <strong>exactly
/// 1101</strong> <c>###</c> tokens, and ours held <strong>two</strong> — the two whose format is
/// not <c>General</c>, which branch (1) already caught.
/// </para>
/// <para>
/// <c>sheet-hash.fods</c> is authored, not collected (<c>probes/sheets-e-01/mkhash.py</c>):
/// fourteen rows, one variable each, across twenty column widths from 0.10 cm to 4.00 cm, so a
/// single render sweeps the boundary rather than sampling one side of it. Every expectation below
/// is <strong>LibreOffice 26.2.4.2's own answer</strong>, read out of its PDF of this file by
/// <c>probes/sheets-e-01/cells.py</c>, which assigns each drawn word to a column by its
/// <em>right</em> edge — a <c>###</c> that does not fit overhangs to the left of its own cell, so
/// a census keyed on the left edge or the centre mis-files exactly the cells this file is about.
/// </para>
/// </remarks>
public sealed class SheetHashOverflowTests
{
    /// <summary>The fixture's column widths, in centimetres, in order.</summary>
    private static readonly double[] Widths =
    [
        0.10, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80,
        0.90, 1.00, 1.20, 1.40, 1.60, 1.80, 2.00, 2.50, 3.00, 4.00,
    ];

    private const string Hash = "###";

    /// <summary>
    /// What each row of the sweep draws, cell by cell, as measured from 26.2.4.2's own PDF.
    /// </summary>
    /// <remarks>
    /// Rows are in the fixture's order. A row is twenty entries long; the last two fall on the
    /// second page, which is why the count is asserted rather than assumed.
    /// </remarks>
    public static TheoryData<int, string[]> Sweep => new()
    {
        // General, value 1. Four hashes: "1" is 5.56 pt at ten point and a 0.25 cm column
        // leaves 5.09 pt after its two 20-twip margins.
        { 0, [.. Repeat(Hash, 4), .. Repeat("1", 16)] },

        // General, 12345. The column has to fit all five digits or the cell hashes: there is no
        // intermediate form for an integer, because dropping a digit would state a wrong number.
        { 1, [.. Repeat(Hash, 12), .. Repeat("12345", 8)] },

        // General, 123456789012 — the case that shows branch (2) is real and branch (3) is the
        // fallback rather than the rule. Twelve integer digits never fit, so the cell goes to
        // scientific notation as soon as *that* fits, and hashes below it.
        {
            2,
            [
                .. Repeat(Hash, 12), "1E+11", "1E+11", "1.2E+11", "1.23E+11", "1.235E+11",
                .. Repeat("123456789012", 3),
            ]
        },

        // General, 1.5. The decimal is dropped first — three columns draw "2", rounded away from
        // zero — and only the columns too narrow for a single digit hash.
        { 3, [.. Repeat(Hash, 4), "2", "2", "2", .. Repeat("1.5", 13)] },

        // General, -1. The sign costs a column: one more hash than the unsigned control.
        { 4, [.. Repeat(Hash, 5), .. Repeat("-1", 15)] },

        // Fixed 0.00, value 1. **Never shortened** — it is 1.00 or ### and nothing between,
        // which is branch (1). Nine hashes against the General control's four.
        { 5, [.. Repeat(Hash, 9), .. Repeat("1.00", 11)] },

        // Fixed 0, value 1. The control on branch (1): its output is the same string the General
        // format produces, so it hashes at the same width. A non-General format is not
        // *more* eager to hash; it simply never shortens.
        { 6, [.. Repeat(Hash, 4), .. Repeat("1", 16)] },

        // Percent. Longer output, so it hashes further out — and again never shortens.
        { 7, [.. Repeat(Hash, 13), .. Repeat("50.00%", 7)] },

        // A date is a number format like any other and hashes; it is not text.
        { 8, [.. Repeat(Hash, 16), .. Repeat("28/02/2022", 4)] },

        // A **string** never hashes at any width. It is clipped, and shortened to the characters
        // that fit, and that is all.
        { 10, [.. Repeat("12345", 20)] },
    };

    /// <summary>
    /// Every cell of one sweep row, against 26.2.4.2's own render of the same file.
    /// </summary>
    [Theory]
    [MemberData(nameof(Sweep))]
    public void ASweepRowDrawsWhatLibreOfficeDraws(int row, string[] expected)
    {
        expected.Length.ShouldBe(Widths.Length);

        IReadOnlyList<string> drawn = Drawn(row);
        drawn.Count.ShouldBe(Widths.Length);

        for (int i = 0; i < expected.Length; i++)
        {
            drawn[i].ShouldBe(
                expected[i], $"row {row}, column {i} ({Widths[i]:0.00} cm)");
        }
    }

    /// <summary>
    /// A value never borrows an empty neighbour's width, and a string always does.
    /// </summary>
    /// <remarks>
    /// <c>GetOutputArea</c> is called with <c>bCellIsValue</c>, and that argument gates the whole
    /// spill loop (<c>output2.cxx:1330</c>). So the same 0.30 cm column hashes a number and shows
    /// a string whole — the asymmetry that makes a spreadsheet's overflow rule surprising, and the
    /// reason the fix cannot be "hash anything that does not fit". Measured on the fixture's
    /// second sheet: <c>### · ABCDEFGH · A(Z) · ###</c>.
    /// </remarks>
    [Fact]
    public void AValueHashesWhereAStringSpills()
    {
        IReadOnlyList<string> spill = Page(2);

        spill[0].ShouldBe(Hash);              // General 12345 beside an empty cell
        spill[1].ShouldBe("ABCDEFGH");        // a string beside an empty cell spills whole
        spill[2].ShouldBe("A");               // a string beside an occupied cell is shortened
        spill[3].ShouldBe("Z");
        spill[4].ShouldBe(Hash);              // a fixed-format value beside an empty cell
    }

    /// <summary>
    /// Shrink-to-fit suppresses <c>###</c> outright, at every width.
    /// </summary>
    /// <remarks>
    /// <c>bShrink</c> scales the font down before the hash gate is reached, so the re-measured
    /// text is no longer clipped (<c>output2.cxx:1854-1885</c>). Measured: the shrink row draws
    /// <c>12345</c> in all twenty columns, down to the 0.10 cm one. This is the control that
    /// stops the fix from being read as "a number that does not fit hashes".
    /// </remarks>
    [Fact]
    public void ShrinkToFitNeverHashes()
        => Drawn(10).ShouldAllBe(text => text == "12345");

    /// <summary>
    /// A wrapping cell hashes exactly as an unwrapping one does, and draws the hash on one line.
    /// </summary>
    /// <remarks>
    /// Automatic line breaks are disabled for a plain number format (i#111387,
    /// <c>output2.cxx:1834</c>), so wrapping neither saves a wide number nor changes where it
    /// starts hashing. And the hash itself is never broken: Calc replaces the *engine text* after
    /// the paper has been decided (<c>:3605</c>), so there is nothing left to break. We drew three
    /// lines of one <c>#</c> and made the row three times as tall, which moves every row under it.
    /// </remarks>
    [Fact]
    public void AWrappingCellHashesOnOneLine()
    {
        IReadOnlyList<string> drawn = Drawn(11);

        for (int i = 0; i < 12; i++) drawn[i].ShouldBe(Hash, $"column {i}");
        for (int i = 12; i < 20; i++) drawn[i].ShouldBe("12345", $"column {i}");
    }

    // ------------------------------------------------------------------------------ harness

    private static string[] Repeat(string text, int count)
        => [.. Enumerable.Repeat(text, count)];

    /// <summary>
    /// The twenty cells of one sweep row, in column order, across both of the sheet's pages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Indexed by <strong>paint order</strong>, not by geometry. <c>SheetPage.Draw</c> walks its
    /// placed rows and then its placed columns, so the runs arrive row-major and a row is a fixed
    /// slice of the list. Keying on a baseline instead would file the shrink row's cells under
    /// four different rows — shrink-to-fit moves a baseline — which is the same trap
    /// <c>sheets-d-01</c> hit when it keyed a border run on a coordinate.
    /// </para>
    /// <para>
    /// The two run counts are asserted rather than assumed, so a cell that ever draws two runs
    /// fails here instead of silently shifting every column by one.
    /// </para>
    /// </remarks>
    private static IReadOnlyList<string> Drawn(int row)
    {
        RecordingDrawingSink sink = Render();

        const int Rows = 14;
        const int OnFirstPage = 18;
        const int OnSecondPage = 2;

        List<DrawnGlyphRun> first = sink.Pages[0].Runs;
        List<DrawnGlyphRun> second = sink.Pages[1].Runs;

        first.Count.ShouldBe(Rows * OnFirstPage);
        second.Count.ShouldBe(Rows * OnSecondPage);

        return
        [
            .. first.Skip(row * OnFirstPage).Take(OnFirstPage).Select(run => run.Text),
            .. second.Skip(row * OnSecondPage).Take(OnSecondPage).Select(run => run.Text),
        ];
    }

    private static IReadOnlyList<string> Page(int page)
        => [.. Render().Pages[page].Runs.Select(run => run.Text)];

    private static RecordingDrawingSink Render()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-hash.fods"));

        RecordingDrawingSink sink = new();
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink;
    }
}
