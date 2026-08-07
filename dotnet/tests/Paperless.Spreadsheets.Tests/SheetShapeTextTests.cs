using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The text inside a shape anchored on a sheet.
/// </summary>
/// <remarks>
/// <para>
/// A text box on a sheet is a drawing rather than a cell, so nothing that walks the grid can find
/// it and no check built on the cells sees it either way. Calc reads one through the same drawing
/// layer as a picture — <c>GroupShapeContext::createShapeContext</c> takes <c>sp</c> beside
/// <c>pic</c> and <c>graphicFrame</c> (<c>sc/source/filter/oox/drawingfragment.cxx:198</c>) — and
/// prints it with <c>PrintDrawingLayer</c> like any other object.
/// </para>
/// <para>
/// It was read for its <em>anchor</em> alone, which kept the print area right and put none of the
/// words on the paper. Measured across the corpus's sheets track: ten workbooks carry shape text
/// and between them 1083 words of it, and on
/// <c>SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx</c> — whose entire methodology note
/// is one text box — that was 163 words against LibreOffice's 550.
/// </para>
/// <para>
/// The fixture is LibreOffice's own conversion of a flat ODS carrying one text box, so the drawing
/// part is written by a real producer rather than by hand.
/// </para>
/// </remarks>
public sealed class SheetShapeTextTests
{
    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void AShapesTextIsRead()
    {
        SheetLayout sheet = Sheet("sheet-shape-text.xlsx");

        sheet.Drawings.Items.Count.ShouldBe(1, "drawings on the sheet");

        SheetShapeText text = sheet.Drawings.Items[0].Text
            .ShouldNotBeNull("the shape's text was read");

        text.IsEmpty.ShouldBeFalse();
        string.Concat(text.Paragraphs.Select(p => p.Text))
              .ShouldContain("Ships in a text box");
    }

    /// <summary>
    /// A run's stated size is kept, because it decides the line height and the wrap.
    /// </summary>
    /// <remarks>
    /// <c>sz</c> is in hundredths of a point, and it is kept per run rather than per paragraph:
    /// the EditEngine breaks a portion at every run boundary, so a body mixing 7 pt and 12 pt runs
    /// wraps each stretch at its own size.
    /// </remarks>
    [Fact]
    public void ARunKeepsTheSizeItStates()
    {
        SheetLayout sheet = Sheet("sheet-shape-text.xlsx");
        SheetShapeText text = sheet.Drawings.Items[0].Text.ShouldNotBeNull();

        SheetShapeParagraph paragraph = text.Paragraphs
            .First(p => p.Text.Contains("Ships", StringComparison.Ordinal));

        paragraph.Runs[0].Size.Points.ShouldBe(11.0, 0.01);
    }

    /// <summary>
    /// A shape carrying text is drawn, where before it reached the print area and stopped.
    /// </summary>
    /// <remarks>
    /// Asserted through the page's own drawing commands rather than the model, because the model
    /// carrying the text and the painter ignoring it is exactly the state this replaced: the
    /// anchor was read all along and <c>SheetPageGraphics</c> skipped any drawing holding neither
    /// picture nor chart.
    /// </remarks>
    [Fact]
    public void AShapesTextReachesThePage()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-shape-text.xlsx"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        string drawn = string.Join(" ", sink.Pages[0].Runs.Select(run => run.Text));
        drawn.ShouldContain("Ships in a text box");
    }
}
