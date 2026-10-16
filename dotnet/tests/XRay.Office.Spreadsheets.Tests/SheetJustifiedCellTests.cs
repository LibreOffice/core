using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A justified cell's wrapped lines are stretched to the width they were broken against.
/// </summary>
/// <remarks>
/// <para>
/// Calc maps both <c>horizontal="justify"</c> and <c>horizontal="distributed"</c> to
/// <c>SvxAdjust::Block</c>, and EditEngine then shares each line's spare width among its
/// <em>blanks</em> — <c>ImpEditEngine::ImpAdjustBlocks</c>
/// (<c>editeng/source/editeng/impedit3.cxx:2306-2420</c>), called from <c>CreateLines</c> for every
/// line with room left over (<c>:1694-1701</c>). A paragraph's last line is exempt (<c>!bEOC</c>),
/// and <c>distributed</c> is exactly the setting that lifts that exemption:
/// <c>bDistLastLine = GetJustifyMethod(nPara) == SvxCellJustifyMethod::Distribute</c> (<c>:1696</c>).
/// </para>
/// <para>
/// <c>sheet-justified-cell.xlsx</c> is authored for this: one 40-character column, Liberation
/// Sans 11 pt, and the same three-line string set three times over — justified in A1, left in A3
/// and distributed in A5. Read off both references' own pages, as the right edge of each line:
/// </para>
/// <code>
///   cell            line 1   line 2   line 3
///   A1 justify      293.02   293.13   285.86     (24.2.7.2)
///   A1 justify      293.02   293.04   285.83     (26.2.4.2)
///   A3 left         285.20   284.10   285.86     (24.2.7.2)
///   A5 distributed  293.02   293.13   293.08     (24.2.7.2)
/// </code>
/// <para>
/// The assertions below are written as comparisons between the three cells rather than against
/// those numbers, because the absolute figures carry this machine's column width — ours sit a
/// constant 0.3 pt right of the references' on the <em>left-aligned</em> control too, so a
/// literal expectation would be pinning that and not this rule.
/// </para>
/// </remarks>
public sealed class SheetJustifiedCellTests
{
    /// <summary>The right edge of every drawn line, in page points, top to bottom.</summary>
    private static IReadOnlyList<(double Top, double Right)> Lines()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-justified-cell.xlsx"));

        PlacedDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        Dictionary<double, double> rows = [];
        foreach ((GlyphRun run, Core.Geometry.DocPoint origin) in sink.Runs)
        {
            if (run.Glyphs.Count == 0) continue;

            // The pen after the last glyph that is not a space. A wrapped line keeps the blank it
            // broke after and a justified one keeps its widened advance, so including them would
            // measure the trailing space rather than the ink — which is exactly the quantity
            // `pdftotext`'s word boxes leave out, and the one the reference figures above are.
            Length pen = origin.X;
            Length right = origin.X;

            for (int glyph = 0; glyph < run.Glyphs.Count; glyph++)
            {
                pen += run.Glyphs[glyph].Advance;

                int cluster = glyph < run.ClusterMap.Count ? run.ClusterMap[glyph] : -1;
                bool blank = cluster >= 0 && cluster < run.Text.Length && run.Text[cluster] == ' ';
                if (!blank) right = pen;
            }

            double key = Math.Round(origin.Y.Points, 1);
            rows[key] = rows.TryGetValue(key, out double already)
                ? Math.Max(already, right.Points)
                : right.Points;
        }

        return [.. rows.OrderBy(row => row.Key).Select(row => (row.Key, row.Value))];
    }

    /// <summary>The three cells, three lines each, in the order the sheet states them.</summary>
    private static (double[] Justify, double[] Left, double[] Distributed) Cells()
    {
        double[] all = [.. Lines().Select(line => line.Right)];
        all.Length.ShouldBe(9, "three cells of three lines each");
        return (all[..3], all[3..6], all[6..]);
    }

    [Fact]
    public void AJustifiedLineReachesFurtherRightThanTheSameLineSetLeft()
    {
        (double[] justify, double[] left, _) = Cells();

        // Both of the first two lines have room left over, and justifying spends it.
        justify[0].ShouldBeGreaterThan(left[0] + 5.0);
        justify[1].ShouldBeGreaterThan(left[1] + 5.0);
    }

    [Fact]
    public void EveryJustifiedLineOfAParagraphEndsAtTheSamePlace()
    {
        (double[] justify, _, _) = Cells();

        // The paper is one width, so every line stretched to it ends on it.
        justify[1].ShouldBe(justify[0], 0.05);
    }

    [Fact]
    public void AParagraphsLastLineIsNotJustified()
    {
        (double[] justify, double[] left, _) = Cells();

        // `!bEOC`. The last line is where a justified cell and a left one agree exactly.
        justify[2].ShouldBe(left[2], 0.05);
    }

    [Fact]
    public void DistributedJustifiesTheLastLineToo()
    {
        (double[] justify, _, double[] distributed) = Cells();

        // `bDistLastLine`, and the discriminator between the two settings: everything else about
        // them is the same, and only this line moves.
        distributed[0].ShouldBe(justify[0], 0.05);
        distributed[2].ShouldBe(justify[0], 0.05);
    }

    [Fact]
    public void JustifyingMovesNoLineBreak()
    {
        // The control that says this is a placement rule and not a measuring one: all three cells
        // break in the same places, so no row height and no page boundary can move with it.
        (double[] justify, double[] left, double[] distributed) = Cells();
        justify.Length.ShouldBe(left.Length);
        distributed.Length.ShouldBe(left.Length);

        IReadOnlyList<(double Top, double Right)> lines = Lines();
        (lines[1].Top - lines[0].Top).ShouldBe(lines[4].Top - lines[3].Top, 0.05);
        (lines[2].Top - lines[1].Top).ShouldBe(lines[5].Top - lines[4].Top, 0.05);
    }
}
