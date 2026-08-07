using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A form control the workbook marks unprintable is on the screen and not on the paper.
/// </summary>
/// <remarks>
/// <para>
/// <c>ftCmo</c>'s third field is a flag word whose <c>fPrint</c> bit
/// (<c>EXC_OBJCMO_PRINTABLE</c>, <c>sc/source/filter/inc/xlescher.hxx:228</c>) says whether the
/// object is printed. The reader skipped past it, so a button's caption reached the page. Excel
/// leaves "Print object" off for a button by default, which makes this the common case rather
/// than an exotic one: 16 of the corpus's 62 binary workbooks carry an unprintable control,
/// across batches 005 to 017.
/// </para>
/// <para>
/// The rule is narrower than the flag. Calc reads the bit for every object but acts on it in one
/// place only — <c>XclImpControlHelper::ProcessControl</c> writes it to the control model's
/// <c>Printable</c> property (<c>sc/source/filter/excel/xiescher.cxx:1998</c>) — while a plain
/// shape with the bit clear is merely traced by <c>DoPreProcessSdrObj</c>
/// (<c>xiescher.cxx:843-845</c>) and printed anyway. Applying it to every object would therefore
/// drop shapes the reference draws, which is why the fixture carries a non-control with the bit
/// clear as well as the two buttons.
/// </para>
/// <para>
/// Measured on <c>sheets/batch-010/xls/PC1000.xls</c>, whose two visible buttons put exactly nine
/// words on each of eight otherwise-exact pages: 957 words against LibreOffice's 873, and 863
/// after.
/// </para>
/// <para>
/// <b>The fixture.</b> <c>sheet-unprintable-control.xlsx</c> beside it is the authored source — two
/// VML form buttons, one stating <c>&lt;x:PrintObject&gt;False&lt;/x:PrintObject&gt;</c> and one
/// stating True, plus a DrawingML text box — and LibreOffice converted it to the <c>.xls</c>, so
/// the Escher stream and the OBJ records are a real producer's. One byte of that output was then
/// changed: the text box's <c>fPrint</c> bit was cleared, because LibreOffice's own BIFF export
/// does not carry <c>fPrintsWithSheet="0"</c> across and without it the fixture could not state
/// the control-only half of the rule. The resulting three flags — non-control clear, control
/// clear, control set — are exactly <c>PC1000.xls</c>'s own.
/// </para>
/// <para>
/// The expectations are not this reader's opinion of the format: LibreOffice 24.2.7.2 renders the
/// committed <c>.xls</c> to a PDF whose extracted text is <c>Anchor</c>, <c>Retained conspicuous
/// inscription</c> and <c>Ordinary quiescent textbox</c>, and no part of the suppressed caption.
/// No two labels share a word, so which object reached the page is readable without positions.
/// </para>
/// </remarks>
public sealed class SheetUnprintableControlTests
{
    private const string Fixture = "sheet-unprintable-control.xls";

    private const string Suppressed = "Suppressed clandestine caption";
    private const string Retained = "Retained conspicuous inscription";
    private const string TextBox = "Ordinary quiescent textbox";

    private static string DrawnText()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        return string.Join(" ", sink.Pages[0].Runs.Select(run => run.Text));
    }

    /// <summary>A control with <c>fPrint</c> clear draws nothing.</summary>
    [Fact]
    public void AnUnprintableControlIsNotDrawn()
    {
        DrawnText().ShouldNotContain("Suppressed", Case.Sensitive);
    }

    /// <summary>
    /// A control with <c>fPrint</c> set still draws, so the rule is the flag and not the type.
    /// </summary>
    /// <remarks>
    /// Asserted beside the negative because a reader that dropped every form control would pass
    /// the first test outright, and would be wrong on any workbook whose buttons are meant to
    /// print — the flag exists precisely because Excel lets an author turn it back on.
    /// </remarks>
    [Fact]
    public void APrintableControlIsStillDrawn()
    {
        DrawnText().ShouldContain(Retained);
    }

    /// <summary>
    /// A shape that is not a control draws whatever its own <c>fPrint</c> bit says.
    /// </summary>
    /// <remarks>
    /// The text box in the fixture carries the bit <em>clear</em>, so an implementation that
    /// tested the flag without testing the type would drop it. LibreOffice draws it, and so must
    /// this: the flag reaches the drawing only through a control model.
    /// </remarks>
    [Fact]
    public void ANonControlShapeIgnoresTheFlag()
    {
        DrawnText().ShouldContain(TextBox);
    }

    /// <summary>
    /// The suppressed control stays in the model, because its anchor still widens the print area.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The distinction this pins is the one that makes the fix safe. <c>ScDrawLayer::GetPrintArea</c>
    /// walks every object on the page and excludes one thing only — the hidden layer a closed
    /// comment's caption sits on (<c>sc/source/core/data/drwlayer.cxx:1395-1424</c>) — so an
    /// unprintable object anchored past the last cell still moves the page break. Dropping it at
    /// read time would draw the right page and paginate the wrong number of them, and the sweep
    /// would show it as a page loss on some unrelated workbook.
    /// </para>
    /// <para>
    /// The fixture makes that observable: all three objects are anchored across columns D to G on
    /// a sheet whose only cell is A1, so each of them alone widens the block.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheSuppressedControlStillCountsTowardsThePrintArea()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];

        sheet.Drawings.Items.Count.ShouldBe(3, "all three objects are read");

        sheet.Drawings.Items.Count(drawing => !drawing.IsPrintable)
             .ShouldBe(1, "only the unprintable button carries the flag");

        // Its anchor reaches past the sheet's single cell, which is what the print area is owed.
        SheetDrawing suppressed = sheet.Drawings.Items.Single(drawing => !drawing.IsPrintable);
        suppressed.To.Column.ShouldBeGreaterThan(0);
    }
}
