using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The ink a shape's text run is drawn in, on a sheet.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF text box's <c>TXO</c> formatting runs each name a <c>FONT</c> index, and that record's
/// <c>icv</c> is a palette index — <c>0x7FFF</c> meaning automatic. <c>SheetShapePainter</c> drew
/// every run under <c>Paint.Solid(Colour.Black)</c>, so a run naming a colour was painted black.
/// </para>
/// <para>
/// <c>sheet-shape-run-colour.xls</c> is <c>sheet-shape-run-colour.fods</c> put through LibreOffice
/// 26.2.4.2's own <c>MS Excel 97</c> filter, so its <c>TXO</c> run array and its <c>FONT</c>
/// records are the reference's own writing rather than a hand-assembled record. 26.2.4.2's PDF of
/// the <c>.xls</c> holds four spans — <c>ZCELL</c> and <c>ZPLAINRUN</c> black, <c>ZREDRUN</c>
/// <c>#ff0000</c> and <c>ZBLUERUN</c> <c>#0000ff</c> — and this tree now draws the same four, each
/// within 0.35 pt of the reference's x.
/// </para>
/// <para>
/// The two black spans are the controls and they are not decoration: <c>ZCELL</c> is cell text,
/// which never went through this path, and <c>ZPLAINRUN</c> is a run of the same text box whose
/// <c>FONT</c> states automatic — so a rule that simply painted every shape run in the first run's
/// colour would fail on it.
/// </para>
/// </remarks>
public sealed class SheetShapeRunColourTests
{
    private static IReadOnlyList<(string Text, Colour Ink)> Spans()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-shape-run-colour.xls"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        RecordingDrawingSink sink = new();
        sink.BeginPage(pages[0].Size);
        pages[0].Draw(sink);
        sink.EndPage();

        return [.. sink.Pages[0].Runs
            .Where(run => run.Text.Trim().Length > 0)
            .Select(run => (run.Text.Trim(),
                            run.Paint is SolidPaint solid ? solid.Colour : Colour.Black))];
    }

    [Fact]
    public void ARunNamingAColourIsDrawnInItAndOneNamingNoneStaysBlack()
    {
        IReadOnlyList<(string Text, Colour Ink)> spans = Spans();

        spans.ShouldContain(span => span.Text == "ZREDRUN" && span.Ink == Colour.FromRgb(0xFF0000));
        spans.ShouldContain(span => span.Text == "ZBLUERUN" && span.Ink == Colour.FromRgb(0x0000FF));

        // The two controls: a run of the same box whose `FONT` states automatic, and the cell
        // text, which never went through the shape path at all.
        spans.ShouldContain(span => span.Text == "ZPLAINRUN" && span.Ink == Colour.Black);
        spans.ShouldContain(span => span.Text == "ZCELL" && span.Ink == Colour.Black);
    }
}
