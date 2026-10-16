using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A rich cell's underline is drawn under the run that asked for it, not across the line.
/// </summary>
/// <remarks>
/// <para>
/// The decoration used to be taken from the <em>cell's</em> format and drawn across the whole
/// line, on the stated grounds that "the run geometry to place a partial rule with does not exist
/// yet". It did — <c>SheetTextSegment</c> has carried <c>Offset</c> and <c>Width</c> all along —
/// and the consequence of not using them was that a cell underlining only its first run drew no
/// rule at all whenever the cell's own font was not underlined. That is the commonest shape this
/// takes: a heading ruled off inside a cell that also holds unruled text after it.
/// </para>
/// <para>
/// Pinned here because the change that fixed it landed without a test and was reported as not
/// having worked. It had: measured on <c>Infotabelle_WLAN im Flugzeug.xlsx</c> page 2 at 150 dpi,
/// the rule is a single row of black at y = 802 px running x = 105 to 350 px, which is baseline
/// + 0.36 pt and exactly the first portion's own span — it stops where the colon after
/// <c>Innereuropäische Flüge</c> begins. The reference draws its own rule at baseline + 1 px over
/// a narrower span, because it substitutes a narrower face for the absent <c>Segoe UI</c>.
/// </para>
/// </remarks>
public sealed class SheetRichUnderlineTests
{
    [Fact]
    public void OnlyTheUnderlinedRunOfARichCellIsRuled()
    {
        DrawnPage page = Draw();

        DrawnGlyphRun ruled = page.Runs.First(run => run.Text.StartsWith("Ruled", StringComparison.Ordinal));
        DrawnGlyphRun rest = page.Runs.First(run => run.Text.StartsWith(" and not", StringComparison.Ordinal));

        // One rule on the whole page for this row, and it is the width of the first span alone.
        // "and not" is wider than "Ruled", so a whole-line rule would be more than twice as long
        // and a missing one would leave nothing to find at all.
        List<DrawnFill> rules = Rules(page, ruled.Origin.Y);

        rules.Count.ShouldBe(1, "the underlined span is ruled and the plain one is not");
        rules[0].Bounds.X.Points.ShouldBe(ruled.Origin.X.Points, 0.01, "the rule starts where the span does");
        rules[0].Bounds.Width.Points.ShouldBe(ruled.Width.Points, 0.01, "and is exactly as wide");
        rules[0].Bounds.X.ShouldBeLessThan(rest.Origin.X, "and stops before the text that follows");
    }

    [Fact]
    public void AnUnderlinedCellWithNoRunsIsStillRuledRightAcross()
    {
        // The control: a plain cell keeps the whole-line rule, which is one fill and not one per
        // segment. Losing this is how a fix aimed at rich cells would cost every ordinary
        // underlined heading its rule.
        DrawnPage page = Draw();

        DrawnGlyphRun whole = page.Runs.First(run => run.Text.StartsWith("Whole cell", StringComparison.Ordinal));
        List<DrawnFill> rules = Rules(page, whole.Origin.Y);

        rules.Count.ShouldBe(1, "one rule for the line");
        rules[0].Bounds.Width.Points.ShouldBe(whole.Width.Points, 0.01, "as wide as the whole line");
    }

    /// <summary>The filled rules sitting just under one baseline, which is where an underline is.</summary>
    private static List<DrawnFill> Rules(DrawnPage page, Length baseline)
        => [.. page.FilledPaths.Where(fill =>
               fill.Bounds.Y > baseline
               && fill.Bounds.Y - baseline < Length.FromPoints(3)
               && fill.Bounds.Height < Length.FromPoints(3))];

    private static DrawnPage Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-rich-underline.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        return sink.Pages[0];
    }
}
