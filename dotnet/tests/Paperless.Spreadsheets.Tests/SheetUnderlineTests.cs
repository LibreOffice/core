using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The lines a cell's font asks for, read and drawn.
/// </summary>
/// <remarks>
/// <para>
/// An underline is a font property in all three formats and none of the three readers looked at
/// it, so a workbook's hyperlinks and its ruled-off column headings put no line on the paper at
/// all. It is the commonest decoration a spreadsheet has: the hyperlink style <em>is</em> an
/// underlined blue font, so every workbook carrying a link has several.
/// </para>
/// <para>
/// Measured on <c>FMMO_NMPF_37C.xlsx</c>, whose three source links and every column heading are
/// underlined: the page's ink imbalance against LibreOffice's own rendering was 0.45% and is
/// 0.09%, which takes it from the image comparison's <c>MAJOR</c> to <c>shifted</c>.
/// </para>
/// <para>
/// The fixture is flat ODF because ODF is the format that states the thing awkwardly — an
/// underline is a dash pattern plus a separate single-or-double count, so a plain underline
/// states no count at all and must not read as "none". SpreadsheetML's <c>&lt;u/&gt;</c> and
/// BIFF's underline byte are three-line mappings with no corpus fixture of their own; the
/// corpus measurement above is the evidence for the SpreadsheetML one.
/// </para>
/// </remarks>
public sealed class SheetUnderlineTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-underline.fods"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Theory]
    [InlineData(0, SheetUnderline.None, false)]
    // A style with an explicit single type, and one with a style and no type at all — the second
    // is what a producer usually writes and it must not read as "none".
    [InlineData(1, SheetUnderline.SingleLine, false)]
    [InlineData(2, SheetUnderline.DoubleLine, false)]
    [InlineData(3, SheetUnderline.None, true)]
    public void TheCellFormatCarriesTheLinesItsStyleStates(
        int row, SheetUnderline underline, bool struckThrough)
    {
        SheetCellFormat format = Sheet().Formats.At(row, 0);

        format.Underline.ShouldBe(underline);
        format.IsStruckThrough.ShouldBe(struckThrough);
    }

    /// <summary>
    /// Each line reaches the page as a filled rule under or through its own run.
    /// </summary>
    /// <remarks>
    /// Filled rather than stroked so the thickness is exactly what the face's <c>post</c> table
    /// asks for, and asserted through the page's own drawing commands because the format carrying
    /// the flag and the painter ignoring it is exactly the state this replaced.
    /// </remarks>
    [Fact]
    public void EachLineIsDrawnAgainstItsOwnRun()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-underline.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        List<DrawnGlyphRun> runs = [.. sink.Pages[0].Runs];
        runs.Count.ShouldBe(4, "one run per row");

        // Four rules: one for the single underline, two for the double, one for the strike. The
        // plain first row contributes none, which is the assertion that matters most — a painter
        // that ruled every cell would pass every other check here.
        List<DrawnFill> rules = [.. sink.Pages[0].FilledPaths];
        rules.Count.ShouldBe(4);

        DrawnGlyphRun plain = runs.First(r => r.Text.StartsWith("Plain", StringComparison.Ordinal));
        rules.ShouldAllBe(rule => rule.Bounds.Y > plain.Origin.Y);

        // The single underline sits just below its baseline and spans exactly its run.
        DrawnGlyphRun single = runs.First(r => r.Text.StartsWith("Single", StringComparison.Ordinal));
        DrawnFill under = rules.MinBy(rule => Math.Abs((rule.Bounds.Y - single.Origin.Y).Emu));

        under.Bounds.X.ShouldBe(single.Origin.X);
        under.Bounds.Width.Points.ShouldBe(single.Width.Points, 0.01);
        under.Bounds.Y.ShouldBeGreaterThan(single.Origin.Y);
        (under.Bounds.Y - single.Origin.Y).Points.ShouldBeLessThan(2.0);

        // The strike sits above its own baseline, which is the whole difference between the two.
        DrawnGlyphRun struck = runs.First(r => r.Text.StartsWith("Struck", StringComparison.Ordinal));
        rules.ShouldContain(rule => rule.Bounds.Y < struck.Origin.Y
                                    && rule.Bounds.Y > struck.Origin.Y - Core.Units.Length.FromPoints(6));
    }
}
