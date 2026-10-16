using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell the importers store as an <c>EditTextObject</c> is laid out by EditEngine, not by
/// <c>DrawStrings</c> — and the two disagree about both its text and its height.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::LayoutStrings</c> asks the question before it looks at a single character:
/// <c>else if (aCell.getType() == CELLTYPE_EDIT) bUseEditEngine = true</c>
/// (<c>sc/source/ui/view/output2.cxx:1710-1712</c>). Both importers build such a cell from a
/// string that carries formatting runs — <c>putRichString</c>
/// (<c>sc/source/filter/oox/sheetdatabuffer.cxx:125-133</c>) and
/// <c>XclImpString::SetToDocument</c> (<c>sc/source/filter/excel/xihelper.cxx:246-256</c>) — and,
/// on the OOXML side, from one holding a hard break.
/// </para>
/// <para>
/// Two consequences, and the fixtures separate them because the triggers are not the same.
/// <em>Clipping</em>: <c>DrawStrings</c> shortens a string that will not fit before showing it and
/// <c>DrawEdit</c> keeps every character behind a clip, so only the second leaves the hidden tail
/// in the PDF's text layer — which is the half a word count scores. <em>Height</em>: an edit cell
/// is measured through <c>GetNeededSize</c>'s EditEngine branch even when it does not wrap, and one
/// EditEngine line is not the arithmetic height <c>lcl_GetAttribHeight</c> gives.
/// </para>
/// <para>
/// Both fixtures come from <c>dotnet/probes/sheets-rest-01/mkclipprobe.py</c>, five rows differing
/// in one property each — plain, rich, hard-broken, both, and plain-but-wrapping — in a column too
/// narrow for their text with the neighbour occupied so that nothing may spill. Measured against
/// the installed LibreOffice 26.2.4.2: the plain row's text layer holds <strong>23</strong> of its
/// 130 characters and the rich, broken and rich-plus-broken rows hold all <strong>130</strong>;
/// with automatic heights the plain row takes <strong>276 twips</strong> — the arithmetic answer
/// for Calibri 11 exactly — and the other three take <strong>298</strong>.
/// </para>
/// <para>
/// <strong>Two properties of the fixture are load-bearing and were both got wrong first.</strong>
/// It is Calibri 11 and not Arial 9, because Arial 9's line is shorter than
/// <c>ScGlobal::nStdRowHeight</c> and every row came back at the 256-twip floor with plain and
/// rich indistinguishable. And every <c>cellXf</c> states <c>applyFont="1"</c> beside a
/// <c>cellStyles</c>, because without them LibreOffice drew the plain rows in its own application
/// default while the rich rows took the face their <c>rPr</c> named — and that version of the
/// fixture said a hard break did *not* reach the EditEngine height, which is the opposite of the
/// truth.
/// </para>
/// <para>
/// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.xls</c> is the clipping half in the corpus: its
/// remediation and audit columns hold paragraphs with blank lines between their parts, and the
/// 1440 words it was short were exactly their hidden tails. <c>SIL_TDB648.xlsx</c> is the height
/// half: 285 twips a row for its plain rows against 298 for its rich footnote rows, which is
/// enough to fit two extra rows on a page.
/// </para>
/// </remarks>
public sealed class SheetEditCellTests
{
    [Fact]
    public void APlainCellTooWideForItsColumnIsShortened()
    {
        // The control. Without it every assertion below could be satisfied by never shortening
        // anything, which is a different renderer and a wrong one.
        Drawn("sheet-edit-cell-clip.xlsx")[0]
            .TrimEnd().Length.ShouldBeLessThan(40, "DrawStrings drops what will not fit");
    }

    [Theory]
    [InlineData(1)]  // rich
    [InlineData(2)]  // a hard break, cell not wrapping
    [InlineData(3)]  // both
    public void AnEditCellKeepsEveryCharacterBehindTheClip(int row)
    {
        // 130 characters of body text. The reference keeps all of them and so must we; the number
        // is asserted as "as many as the plain row lost" rather than exactly, because the break
        // character itself is not shown.
        Drawn("sheet-edit-cell-clip.xlsx")[row]
            .TrimEnd().Length.ShouldBeGreaterThan(
                120, "DrawEdit clips the ink and keeps the text");
    }

    [Fact]
    public void AnEditCellThatDoesNotWrapTakesAnEditEngineLineRatherThanTheArithmeticHeight()
    {
        SheetAxis rows = AutoHeightRows();

        // Calibri 11 through the EditEngine branch: an ascent and a descent quantised to whole
        // 96 dpi pixels, plus a pixel of margin either side. LibreOffice writes 0.2071in for
        // both rich rows of this fixture.
        rows.SizeAt(1).Twips.ShouldBe(298L, "the rich row is one EditEngine line");
        rows.SizeAt(2).Twips.ShouldBe(298L, "and so is the one holding only a hard break");
        rows.SizeAt(3).Twips.ShouldBe(298L, "and so is the rich one holding a break");
    }

    [Fact]
    public void APlainCellIsStillMeasuredArithmetically()
    {
        SheetAxis rows = AutoHeightRows();

        // The control, and the bound at the other end: the arithmetic answer for Calibri 11 is
        // trunc(220 x 1.18) + 40 - 23, and a plain cell still gets it.
        rows.SizeAt(0).Twips.ShouldBe(276L, "a plain row keeps the arithmetic answer");
    }

    /// <summary>
    /// The recomputed heights of the fixture's rows.
    /// </summary>
    /// <remarks>
    /// Through <see cref="SheetAxis.SizeAt"/> rather than through <c>Runs</c>: a row that comes out
    /// at the axis default carries no run of its own, so reading the runs would find nothing for
    /// exactly the two control rows this has to check.
    /// </remarks>
    private static SheetAxis AutoHeightRows()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-edit-cell-height.xlsx"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        return pages.Sheets[0].Grid.Rows;
    }

    /// <summary>Column A's text on each of the fixture's first four rows, in row order.</summary>
    /// <remarks>
    /// <para>
    /// Grouped by baseline in <em>emission</em> order rather than by coordinate, which is not a
    /// tidiness choice: the fifth row wraps, its eight lines overflow a twelve-point row, and
    /// their baselines interleave with the baselines of the four rows above. Sorting by y would
    /// put the wrapped cell's second line between rows one and two and shift every index.
    /// </para>
    /// <para>
    /// The text is joined across runs rather than taken from one, because a rich cell is drawn as
    /// two — a bold word and the rest. The single <c>X</c> in column B is what occupies the
    /// neighbour and is dropped.
    /// </para>
    /// </remarks>
    private static List<string> Drawn(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        List<int> order = [];
        Dictionary<int, string> byBaseline = [];

        foreach (DrawnGlyphRun run in sink.Pages[0].Runs)
        {
            if (run.Text == "X") continue;

            int baseline = (int)Math.Round(run.Origin.Y.Points);
            if (!byBaseline.ContainsKey(baseline))
            {
                order.Add(baseline);
                byBaseline[baseline] = string.Empty;
            }

            byBaseline[baseline] += run.Text;
        }

        return [.. order.Take(4).Select(baseline => byBaseline[baseline])];
    }
}
