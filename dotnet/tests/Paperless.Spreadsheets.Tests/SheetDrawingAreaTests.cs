using System.Text;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What a drawing does to the block of cells a sheet prints.
/// </summary>
/// <remarks>
/// <para>
/// A chart floating to the right of the last cell is not decoration: it moves the right-hand page
/// break, because Calc's print area is the maximum of what the cells reach and what the drawing
/// layer covers (<c>ScDocument::GetPrintArea</c>,
/// <c>sc/source/core/data/documen2.cxx:644-664</c>). Getting it wrong does not shift a picture, it
/// deletes a page — measured on <c>chart2/qa/extras/data/xlsx/bubble_chart_simple.xlsx</c>, which
/// rendered one page against LibreOffice's two with a complete chart composed off the paper.
/// </para>
/// <para>
/// Assembled here rather than taken from the corpus because the corpus has no such workbook:
/// every spreadsheet LibreOffice wrote for it anchors its chart inside the cells it plots, which
/// is exactly the case that cannot see this.
/// </para>
/// </remarks>
public sealed class SheetDrawingAreaTests
{
    /// <summary>A sheet of default columns and rows, which is what a bare workbook has.</summary>
    private static SheetGrid Grid => SheetGrid.Standard;

    [Fact]
    public void ADrawingRightOfTheLastCellWidensThePrintedBlock()
    {
        SheetRange used = new(0, 0, 2, 4);
        SheetDrawings drawings = new([Anchored(from: 11, to: 18, fromRow: 10, toRow: 24)]);

        SheetRange extended = SheetDrawingArea.Extend(used, drawings, Grid);

        // The end cell is the one the far edge falls in, not the one after it: the anchor ends
        // 3600 EMU into column 18, so column 18 is the last printed column.
        extended.LastColumn.ShouldBe(18);
        extended.LastRow.ShouldBe(24);

        // And only the end moves. The print area always begins at A1 — AdjustPrintArea(true) sets
        // both starts to zero — so a drawing cannot push the origin about.
        extended.FirstColumn.ShouldBe(0);
        extended.FirstRow.ShouldBe(0);
    }

    [Fact]
    public void ADrawingInsideTheUsedRangeChangesNothing()
    {
        SheetRange used = new(0, 0, 20, 40);
        SheetDrawings drawings = new([Anchored(from: 1, to: 3, fromRow: 1, toRow: 3)]);

        SheetDrawingArea.Extend(used, drawings, Grid).ShouldBe(used);
    }

    /// <summary>
    /// A hidden drawing paints nothing and still moves the page break.
    /// </summary>
    /// <remarks>
    /// This reads backwards and is nevertheless the rule. <c>ScDrawLayer::GetPrintArea</c> skips an
    /// object only when it sits on <c>SC_LAYER_HIDDEN</c> (<c>drwlayer.cxx:1408</c>), and that
    /// layer holds exactly one kind of thing: the caption of a comment nobody has pinned open
    /// (<c>sc/source/core/data/postit.cxx:84</c>). A shape whose <c>cNvPr</c> says
    /// <c>hidden="1"</c> is not on it — <c>oox</c> gives that shape <c>Visible = false</c> and
    /// <c>Printable = false</c> and leaves it on the standard layer (<c>shape.cxx:1436-1442</c>) —
    /// and the line immediately above the layer test admits as much: <c>//TODO: test Flags
    /// (hidden?)</c>. So the flag is read, <c>SheetPageGraphics</c> honours it, and the print area
    /// does not. Measured on <c>sc/qa/unit/data/xlsb/universal-content.xlsb</c>, whose only drawing
    /// is a hidden comment shape reaching column 12 and row 50: one page here against
    /// LibreOffice's four, and 11 words against 20, until this stopped skipping it.
    /// </remarks>
    [Fact]
    public void AHiddenDrawingStillWidensThePrintedBlock()
    {
        SheetRange used = new(0, 0, 2, 4);
        SheetDrawings drawings = new(
            [Anchored(from: 11, to: 18, fromRow: 10, toRow: 24) with { IsHidden = true }]);

        SheetRange extended = SheetDrawingArea.Extend(used, drawings, Grid);

        extended.LastColumn.ShouldBe(18);
        extended.LastRow.ShouldBe(24);
    }

    [Fact]
    public void ASheetHoldingNothingButADrawingStillPrints()
    {
        // ScTable::GetPrintArea leaves its outputs at zero when it finds nothing, and the
        // document's maximum is then taken against those zeroes rather than against "no area", so
        // a workbook whose only content is a chart is not an empty workbook.
        SheetRange nothing = new(0, 0, -1, -1);
        nothing.IsValid.ShouldBeFalse();

        SheetRange extended = SheetDrawingArea.Extend(
            nothing, new SheetDrawings([Anchored(from: 2, to: 5, fromRow: 2, toRow: 6)]), Grid);

        extended.IsValid.ShouldBeTrue();
        extended.LastColumn.ShouldBe(5);
    }

    [Fact]
    public void AnAbsolutelyAnchoredDrawingIsMeasuredFromTheSheetOrigin()
    {
        // Two standard columns wide from x = 0 reaches into the third, so the last column is 2.
        SheetDrawing sheetAnchored = new()
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(Length.Zero, Length.Zero),
            Extent = new DocSize(
                SheetGrid.StandardColumnWidth * 2.5, SheetGrid.StandardRowHeight * 1.5),
        };

        SheetRange extended = SheetDrawingArea.Extend(
            new SheetRange(0, 0, 0, 0), new SheetDrawings([sheetAnchored]), Grid);

        extended.LastColumn.ShouldBe(2);
        extended.LastRow.ShouldBe(1);
    }

    /// <summary>
    /// A sheet-anchored ODF frame reaches a page, and its chart with it.
    /// </summary>
    /// <remarks>
    /// <c>table:shapes</c> is the table's own container for drawings fastened to the sheet rather
    /// than to a cell, and walking only the cells missed it entirely.
    /// <c>chart2/qa/extras/data/ods/tdf166428_Low_High_StockChart_LO248.ods</c> is the file that
    /// found it: its stock chart read into a correct model and then had nowhere to go, 24 words
    /// rendered against LibreOffice's 60. The flat source here is the same shape, cut to the frame.
    /// </remarks>
    [Fact]
    public void AFrameInTableShapesIsAnchoredToTheSheet()
    {
        using IDocument document = new SpreadsheetReader().Read(
            DocumentSource.FromBytes(Encoding.UTF8.GetBytes(SheetShapesSource), "shapes.fods"));

        SheetLayout sheet = ((SpreadsheetPages)((IPaginatedDocument)document).Layout()).Sheets[0];

        sheet.Drawings.Items.Count.ShouldBe(1);
        SheetDrawing drawing = sheet.Drawings.Items[0];

        // Not a cell anchor: svg:x and svg:y are measured from the table's own origin, so the
        // frame keeps its place when a column is inserted.
        drawing.Anchor.ShouldBe(SheetAnchorKind.Absolute);
        drawing.Position.X.Millimetres.ShouldBe(79.24, 0.01);
        drawing.Position.Y.Millimetres.ShouldBe(5.81, 0.01);
        drawing.Extent.Width.Millimetres.ShouldBe(77.95, 0.01);

        // And it widens the printed block, which is the half that makes it visible: the one cell
        // this sheet holds would otherwise print column A alone.
        sheet.PrintedRange.LastColumn.ShouldBeGreaterThan(0);
    }

    private static SheetDrawing Anchored(int from, int to, int fromRow, int toRow)
        => new()
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = new SheetCellPoint(from, Length.Zero, fromRow, Length.Zero),
            To = new SheetCellPoint(to, Length.FromEmu(3600), toRow, Length.FromEmu(3600)),
        };

    /// <summary>A flat ODF sheet whose only drawing sits in <c>table:shapes</c>.</summary>
    private const string SheetShapesSource = """
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
            xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
            xmlns:xlink="http://www.w3.org/1999/xlink"
            office:version="1.3"
            office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
          <office:body>
            <office:spreadsheet>
              <table:table table:name="Sheet1">
                <table:shapes>
                  <draw:frame draw:z-index="0" draw:name="Object 1"
                              svg:width="77.95mm" svg:height="66.03mm"
                              svg:x="79.24mm" svg:y="5.81mm">
                    <draw:image xlink:href="Pictures/none.png" xlink:type="simple"/>
                  </draw:frame>
                </table:shapes>
                <table:table-column/>
                <table:table-row>
                  <table:table-cell office:value-type="string"><text:p>one</text:p></table:table-cell>
                </table:table-row>
              </table:table>
            </office:spreadsheet>
          </office:body>
        </office:document>
        """;
}
