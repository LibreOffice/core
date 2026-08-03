using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A cell comment in a BIFF workbook is not a shape on the page.
/// </summary>
/// <remarks>
/// <para>
/// A comment reaches a <c>.xls</c> as an ordinary drawing: an <c>OBJ</c> record with a
/// <c>TXO</c> holding its text and an Escher shape holding its box, indistinguishable from a
/// text box except by the <c>ftCmo</c> type, 25 — <c>EXC_OBJTYPE_NOTE</c>,
/// <c>sc/source/filter/inc/xlescher.hxx:69</c>. Calc's importer takes it apart instead of
/// inserting it: <c>XclImpNoteObj</c>'s constructor calls <c>SetInsertSdrObj(false)</c> with the
/// comment "caption object will be created manually", and its <c>DoPreProcessSdrObj</c> turns the
/// text into a <c>ScPostIt</c> on the cell, giving it a caption only when the <c>NOTE</c> record
/// marks the comment visible (<c>sc/source/filter/excel/xiescher.cxx:1852-1883</c>).
/// </para>
/// <para>
/// Measured on <c>Background_Declaration_Template.xls</c>, which carries one hidden comment
/// anchored across a column break: we drew "W. Post: need to adapt to logic rules per record"
/// on two pages and embedded a fourth font for it, and LibreOffice's own rendering of the file
/// contains none of those words on any of its 25 pages — 201 extractable words against 181.
/// The drawing twice was not itself wrong; a shape really is drawn on every page it reaches
/// (<c>ScOutputData::PrePrintDrawingLayer</c>). Drawing it at all was.
/// </para>
/// <para>
/// The fixture is LibreOffice 24.2.7.2's own XLS export of a sheet with one hidden comment, and
/// its PDF of that export carries "Cell with a hidden comment" and "Plain neighbour" and not
/// "HIDDENCOMMENTBODY".
/// </para>
/// </remarks>
public sealed class SheetCellCommentTests
{
    private static string DrawnText(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        return string.Join(" ", sink.Pages.SelectMany(page => page.Runs).Select(run => run.Text));
    }

    [Fact]
    public void AHiddenCommentIsNotDrawn()
    {
        string drawn = DrawnText("sheet-cell-comment.xls");

        drawn.ShouldNotContain("HIDDENCOMMENTBODY");

        // The control, without which a renderer that draws nothing at all passes.
        drawn.ShouldContain("Plain neighbour");
    }
}
