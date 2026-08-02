using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A BIFF workbook's drawing layer: the Escher shapes, and the text boxes among them.
/// </summary>
/// <remarks>
/// <para>
/// The BIFF reader had no drawing layer at all, though <c>Paperless.MsBinary/Escher</c> has served
/// DOC and PPT since the beginning and the SpreadsheetML side gained its text boxes two rounds ago.
/// A text box is the one thing on a sheet that no walk of the cells can find, so nothing built on
/// the grid could see the gap either: <c>apron-area.xls</c> measured 294 words against
/// LibreOffice's 431, and the 137 missing were all inside shapes.
/// </para>
/// <para>
/// Three record kinds and one assembly step. The Escher stream arrives split across every
/// <c>MSODRAWING</c> record in the substream — the split is arbitrary and a container routinely
/// straddles it, so reading them one at a time yields truncated containers — <c>OBJ</c> says what
/// each shape is, and <c>TXO</c> plus its <c>CONTINUE</c> records carry the string.
/// </para>
/// <para>
/// The fixture is LibreOffice's own conversion of a flat ODS carrying one text box, so the drawing
/// is written by a real producer rather than by hand; its reference PDF and ours agree to 0.00%
/// ink imbalance.
/// </para>
/// </remarks>
public sealed class XlsDrawingTests
{
    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void ATextBoxIsReadFromTheDrawingLayer()
    {
        SheetLayout sheet = Sheet("sheet-text-box.xls");

        sheet.Drawings.Items.Count.ShouldBe(1, "drawings on the sheet");

        SheetShapeText text = sheet.Drawings.Items[0].Text
            .ShouldNotBeNull("the TXO's string was read");

        string.Concat(text.Paragraphs.Select(paragraph => paragraph.Text))
              .ShouldBe("Escher text box on a sheet");
    }

    /// <summary>
    /// The shape is anchored where the client anchor says, not at the sheet's origin.
    /// </summary>
    /// <remarks>
    /// BIFF states the anchor as a cell plus a fraction of that cell — 1024ths of the column's
    /// width and 256ths of the row's height, which is the format's own asymmetry
    /// (<c>lclGetXFromCol</c> and <c>lclGetYFromRow</c>,
    /// <c>sc/source/filter/excel/xlescher.cxx:54-67</c>). The fixture puts the box half a
    /// centimetre from the left and 2.4 cm down over four-centimetre columns, so it starts inside
    /// the first column and several rows down.
    /// </remarks>
    [Fact]
    public void ATextBoxKeepsItsAnchor()
    {
        SheetDrawing drawing = Sheet("sheet-text-box.xls").Drawings.Items[0];

        drawing.Anchor.ShouldBe(SheetAnchorKind.TwoCell);
        drawing.From.Column.ShouldBe(0, "half a centimetre into a four-centimetre column");
        drawing.From.Row.ShouldBeGreaterThan(0, "2.4 cm down is past the first row");
        drawing.To.Column.ShouldBeGreaterThan(drawing.From.Column, "the box is eight centimetres wide");
    }

    [Fact]
    public void ATextBoxReachesThePage()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-text-box.xls"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        string.Join(" ", sink.Pages[0].Runs.Select(run => run.Text))
              .ShouldContain("Escher text box on a sheet");
    }

    /// <summary>
    /// A workbook with no drawing at all still reads, and pays nothing for the reader.
    /// </summary>
    [Fact]
    public void AWorkbookWithoutDrawingsHasNone()
        => Sheet("sheet-cell-text.xls").Drawings.IsEmpty.ShouldBeTrue();
}
