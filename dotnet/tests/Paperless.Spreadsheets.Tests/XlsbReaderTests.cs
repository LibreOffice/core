using System.IO.Compression;
using System.Text;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the XLSB reader against workbooks assembled record by record.
/// </summary>
/// <remarks>
/// <para>
/// <strong>LibreOffice cannot write XLSB.</strong> Its filter is import-only, so there is no
/// `--convert-to xlsb` and no way to add a fixture to the corpus the way every other format's
/// fixtures were added. The ten files in <c>sc/qa/unit/data/xlsb/</c> are the only real ones on
/// the machine, and they are whole-file evidence rather than unit evidence: they say the reader
/// agrees with LibreOffice on seven of ten workbooks end to end, which is recorded with the
/// numbers in <c>src/Paperless.Spreadsheets/TODO.md</c>, but they cannot say <em>which</em> record
/// a regression broke.
/// </para>
/// <para>
/// So the records are written here instead. That is not a second-best: BIFF12 is a framing
/// format, and the failures worth guarding against are framing failures — a two-byte identifier
/// read as one, a string count read unsigned, an <c>XF</c> counted in the wrong container — each
/// of which desynchronises the rest of the part and shows up as garbage far from its cause. A
/// hand-written record puts the boundary exactly where the test needs it.
/// </para>
/// </remarks>
public class XlsbReaderTests
{
    private static IDocument Open(byte[] package)
        => new SpreadsheetReader().Read(DocumentSource.FromBytes(package, "synthetic.xlsb"));

    private static ContentTable TableOf(IDocument document, int sheetIndex = 0)
        => document.Content.Children.OfType<ContentSection>()
                   .Where(s => s.Kind == SectionKind.Sheet)
                   .ElementAt(sheetIndex)
                   .Children.OfType<ContentTable>()
                   .Single();

    private static ContentTableCell Cell(IDocument document, int row, int column, int sheet = 0)
        => TableOf(document, sheet).Children.Cast<ContentTableRow>()
                                   .Single(r => r.Index == row)
                                   .Children.Cast<ContentTableCell>()
                                   .Single(c => c.Column == column);

    private static SheetLayout Layout(IDocument document, int index = 0)
        => ((SpreadsheetPages)((IPaginatedDocument)document).Layout()).Sheets[index];

    [Fact]
    public void AWorkbookReadsThroughTheXlsbPath()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellSi(0, 0).CellRk(1, 12).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, strings: ["Region"]));

        document.Format.ShouldBe(DocumentFormat.Xlsb);
        document.Family.ShouldBe(DocumentFamily.Spreadsheet);
        document.Content.GetText().TrimEnd('\n').ShouldBe("Region\t12");
    }

    /// <summary>
    /// A record identifier of two bytes is read as two, and the stream stays in step.
    /// </summary>
    /// <remarks>
    /// The identifier and the size share one variable-length encoding — seven bits a byte, the
    /// top bit meaning "another follows" (<c>lclReadRecordHeader</c>,
    /// <c>oox/source/core/recordparser.cxx:255-290</c>) — so an identifier below 0x80 occupies one
    /// byte and one above it occupies two. <c>SHEETFORMATPR</c> is 0x01E5 and <c>MERGECELLS</c> is
    /// 0x00B1; a reader that assumes a fixed width reads the second byte of the first two-byte
    /// identifier as a length and never finds a cell again. This sheet puts a wide identifier
    /// before the data for exactly that reason.
    /// </remarks>
    [Fact]
    public void AWideRecordIdentifierDoesNotDesynchroniseTheStream()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Record(Ids.SheetFormatPr, w => w.Int32(-1).UInt16(8).UInt16(300).UInt16(0));
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellRk(0, 7).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        Cell(document, 0, 0).Value.ShouldBe(7.0);

        // And the wide record was read, not merely stepped over: 300 twips is 15 pt.
        Layout(document).Grid.Rows.SizeAt(0).Points.ShouldBe(15, 0.01);
    }

    [Fact]
    public void EachCellFamilyReadsItsOwnPayload()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0)
             .CellDouble(0, 4.5)
             .CellBool(1, true)
             .CellError(2, 0x07)
             .CellInlineString(3, "loose")
             .CellRichString(4, "rich")
             .End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        Cell(document, 0, 0).Value.ShouldBe(4.5);
        Cell(document, 0, 1).Value.ShouldBe(true);
        Cell(document, 0, 1).GetText().ShouldBe("TRUE");
        Cell(document, 0, 2).Value.ShouldBe(CellError.DivideByZero);
        Cell(document, 0, 3).Value.ShouldBe("loose");
        Cell(document, 0, 4).Value.ShouldBe("rich");
    }

    /// <summary>
    /// A <c>MULTCELL_*</c> carries no column of its own, and that is the whole difference.
    /// </summary>
    /// <remarks>
    /// <c>readCellHeader</c> reads an <c>int32</c> for a <c>CELL_*</c> and a <c>FORMULA_*</c> and
    /// pre-increments for a <c>MULTCELL_*</c> (<c>sheetdatacontext.cxx:456-460</c>), which is the
    /// format's run-length compression: a run of adjacent cells states its column once. A reader
    /// that reads a column from every record takes the first four bytes of the *value* as one, so
    /// a run of numbers lands scattered across the sheet with the wrong values in it.
    /// </remarks>
    [Fact]
    public void AMultipleCellRecordContinuesFromThePreviousColumn()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellRk(3, 10).MultCellRk(20).MultCellRk(30).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        Cell(document, 0, 3).Value.ShouldBe(10.0);
        Cell(document, 0, 4).Value.ShouldBe(20.0);
        Cell(document, 0, 5).Value.ShouldBe(30.0);
    }

    /// <summary>
    /// A formula cell reads its cached result and steps over the tokens after it.
    /// </summary>
    /// <remarks>
    /// The token array follows the value, so the record is longer than its value and the reader
    /// has to stop at the right place — but only within the record, which is why an undecoded
    /// token array costs nothing: the next record's offset comes from the header, not from how far
    /// this one was read. What matters is that the cached result is taken and <c>Formula</c> is
    /// left null, the same shape the BIFF8 reader produces and deliberately unlike XLSX's.
    /// </remarks>
    [Fact]
    public void AFormulaCellTakesItsCachedResultAndLeavesTheTokensAlone()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).FormulaDouble(0, 54).FormulaString(1, "cached").End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        Cell(document, 0, 0).Value.ShouldBe(54.0);
        Cell(document, 0, 0).Formula.ShouldBeNull();
        Cell(document, 0, 1).Value.ShouldBe("cached");
    }

    /// <summary>
    /// Every <c>SI</c> is a rich string, and the run list has to be consumed.
    /// </summary>
    /// <remarks>
    /// The flag byte in front of the text says which optional tails follow
    /// (<c>RichString::importString</c>, <c>sc/source/filter/oox/richstring.cxx:329</c>). Reading
    /// the text without it takes the flag as the low byte of the character count and the whole
    /// table desynchronises after the first entry — which is why the second string here is the one
    /// that matters.
    /// </remarks>
    [Fact]
    public void ASharedStringWithRunsLeavesTheTableInStep()
    {
        Biff12Writer strings = new();
        strings.Record(Ids.Sst, w => w.Int32(2).Int32(2));
        strings.Record(Ids.Si, w => w.Byte(0x01).String("formatted").Int32(2).UInt16(0).UInt16(0).UInt16(4).UInt16(1));
        strings.Record(Ids.Si, w => w.Byte(0x00).String("plain"));

        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellSi(0, 0).CellSi(1, 1).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, sharedStrings: strings));

        Cell(document, 0, 0).Value.ShouldBe("formatted");
        Cell(document, 0, 1).Value.ShouldBe("plain");
    }

    /// <summary>
    /// A style <c>XF</c> does not shift the cell formats after it.
    /// </summary>
    /// <remarks>
    /// <c>XF</c> is one identifier used inside both <c>CELLSTYLEXFS</c> and <c>CELLXFS</c>, and
    /// LibreOffice tells the two apart only by which container it is inside
    /// (<c>stylesfragment.cxx:302-306</c>). A flat walk collecting every <c>XF</c> builds a table
    /// whose indices are all off by however many named styles the workbook has — which reads as a
    /// number-format bug and is a parsing one. The workbook here has two style formats before the
    /// cell formats, so a flat walk would resolve cell format 1 to the general format and show
    /// <c>0.5</c> where the file says <c>50%</c>.
    /// </remarks>
    [Fact]
    public void AStyleFormatDoesNotShiftTheCellFormatIndices()
    {
        Biff12Writer styles = new();
        styles.Begin(Ids.CellStyleXfs);
        styles.Record(Ids.Xf, w => w.UInt16(0xFFFF).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));
        styles.Record(Ids.Xf, w => w.UInt16(0xFFFF).UInt16(14).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));
        styles.End(Ids.CellStyleXfs);
        styles.Begin(Ids.CellXfs);
        styles.Record(Ids.Xf, w => w.UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));

        // Number format 9 is the built-in `0%`.
        styles.Record(Ids.Xf, w => w.UInt16(0).UInt16(9).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));
        styles.End(Ids.CellXfs);

        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellDouble(0, 0.5, styleIndex: 1).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, styles: styles));

        Cell(document, 0, 0).GetText().ShouldBe("50%");
    }

    /// <summary>A <c>NUMFMT</c> record states a code the workbook then uses by id.</summary>
    [Fact]
    public void ACustomNumberFormatIsResolvedFromTheBinaryStylesPart()
    {
        Biff12Writer styles = new();
        styles.Begin(Ids.NumFmts);
        styles.Record(Ids.NumFmt, w => w.UInt16(180).String("0.000"));
        styles.End(Ids.NumFmts);
        styles.Begin(Ids.CellXfs);
        styles.Record(Ids.Xf, w => w.UInt16(0).UInt16(180).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));
        styles.End(Ids.CellXfs);

        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellDouble(0, 1.5, styleIndex: 0).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, styles: styles));

        Cell(document, 0, 0).GetText().ShouldBe("1.500");
    }

    /// <summary>
    /// A merged block is one cell with a span, not one cell and several empty ones.
    /// </summary>
    /// <remarks>
    /// <c>MERGECELLS</c> comes after the cell data in every file Excel writes, so it is read in a
    /// pass of its own; and its four fields are the row pair *before* the column pair
    /// (<c>BinRange::read</c>, <c>addressconverter.cxx:59</c>), which is the reverse of how a range
    /// is spoken. Reading them in the spoken order gives a plausible range that is wrong on every
    /// block that is not square — which is why this one is 2 columns by 3 rows.
    /// </remarks>
    [Fact]
    public void AMergedRangeBecomesOneSpanningCell()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellInlineString(0, "wide").End();
        sheet.End(Ids.SheetData);
        sheet.Begin(Ids.MergeCells);
        sheet.Record(Ids.MergeCell, w => w.Int32(0).Int32(2).Int32(0).Int32(1));
        sheet.End(Ids.MergeCells);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        Cell(document, 0, 0).ColumnSpan.ShouldBe(2);
        Cell(document, 0, 0).RowSpan.ShouldBe(3);
    }

    /// <summary>
    /// A column width is 256ths of a digit and a row height is twips, both unlike the XML.
    /// </summary>
    /// <remarks>
    /// The XML states a width as a fraction of one digit and a height in points; BIFF12 states the
    /// width in 256ths and the height in twentieths (<c>worksheetfragment.cxx:800, :827</c>).
    /// Reading an XLSB with the XML's scales gives columns 256 times too wide, which is one column
    /// to a page, and rows twenty times too tall.
    /// </remarks>
    [Fact]
    public void ColumnWidthsAreTwoHundredAndFiftySixthsAndRowHeightsAreTwips()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Record(Ids.Col, w => w.Int32(0).Int32(0).Int32(20 * 256).Int32(0).UInt16(0));
        sheet.Begin(Ids.SheetData);
        sheet.Row(0, height: 480).CellRk(0, 1).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));
        SheetGrid grid = Layout(document).Grid;

        // Twenty digits of the default font, which is 111 twips each.
        grid.Columns.SizeAt(0).Twips.ShouldBe(20 * 111);
        grid.Rows.SizeAt(0).Points.ShouldBe(24, 0.01);
    }

    /// <summary>
    /// The stated paper is applied only when the settings say the printer's are not to be used.
    /// </summary>
    /// <remarks>
    /// <c>mbValidSettings</c> is <c>!BIFF12_PAGESETUP_INVALID</c> and the paper size is written
    /// onto the page style only when <c>mbValidSettings</c> is false
    /// (<c>pagesettings.cxx:271, :935</c>) — so the flag whose name says "invalid" is the one that
    /// makes the index count. Getting it backwards puts every Excel workbook that states a page
    /// setup onto Calc's own default paper and every one that states none onto Letter.
    /// </remarks>
    [Theory]
    [InlineData(0x0004, 355.6)]
    [InlineData(0x0000, 297.0)]
    public void TheStatedPaperCountsOnlyWhenTheSettingsAreMarkedInvalid(
        int flags, double expectedLongEdgeMillimetres)
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellRk(0, 1).End();
        sheet.End(Ids.SheetData);

        // Paper size 5 is US Legal, 8.5 by 14 inches — 215.9 by 355.6 mm.
        sheet.Record(Ids.PageSetup, w => w
            .Int32(5).Int32(100).Int32(0).Int32(0).Int32(0).Int32(0).Int32(1).Int32(1)
            .UInt16((ushort)(flags | 0x0002)));
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet));

        // Landscape, so the long edge is the width: 355.6 mm for US Legal, 297 for A4.
        Layout(document).Setup.IsLandscape.ShouldBeTrue();
        Layout(document).Setup.PageSize.Width.Millimetres.ShouldBe(expectedLongEdgeMillimetres, 0.6);
    }

    /// <summary>A hidden sheet is extracted and flagged, never skipped.</summary>
    [Fact]
    public void AHiddenSheetIsFlaggedRatherThanDropped()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellInlineString(0, "quiet").End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, state: 1));

        ContentSection section = document.Content.Children.OfType<ContentSection>()
                                        .Single(s => s.Kind == SectionKind.Sheet);
        section.IsHidden.ShouldBeTrue();
        section.GetText().TrimEnd('\n').ShouldBe("quiet");
    }

    /// <summary>
    /// The 1904 epoch is a workbook-wide switch, and reading it wrong shifts every date by 1462 days.
    /// </summary>
    [Fact]
    public void TheNineteenOhFourEpochIsTakenFromTheWorkbookRecord()
    {
        Biff12Writer styles = new();
        styles.Begin(Ids.CellXfs);

        // Built-in number format 14 is the short date.
        styles.Record(Ids.Xf, w => w.UInt16(0).UInt16(14).UInt16(0).UInt16(0).UInt16(0).UInt16(0).UInt16(0));
        styles.End(Ids.CellXfs);

        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellDouble(0, 0, styleIndex: 0).End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, styles: styles, date1904: true));

        Cell(document, 0, 0).Value.ShouldBeOfType<DateTime>().Year.ShouldBe(1904);
    }

    /// <summary>
    /// A drawing part is XML in an XLSB, and the worksheet's relationship still points at it.
    /// </summary>
    /// <remarks>
    /// None of DrawingML has a binary encoding, so a chart, a picture and their anchors are
    /// byte-for-byte what an XLSX holds and are read by the same code. Only the part *name* comes
    /// out of BIFF12, and it comes out of the workbook rather than the sheet. Leaving this unwired
    /// cost a page and eight words on
    /// <c>sc/qa/unit/data/xlsb/tdf108017_calcProtection.xlsb</c>: its chart read into nothing.
    /// </remarks>
    [Fact]
    public void ADrawingHangingOffTheBinarySheetIsStillReadAsXml()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellInlineString(0, "cells").End();
        sheet.End(Ids.SheetData);
        sheet.End(Ids.Worksheet);

        using IDocument document = Open(XlsbPackage.Build(sheet, drawing: PictureDrawing));
        SheetLayout layout = Layout(document);

        layout.Drawings.Items.Count.ShouldBe(1);
        layout.Drawings.Items[0].From.Column.ShouldBe(8);

        // And it widens the printed block, which is the half that makes it visible on a page: the
        // one cell this sheet holds would otherwise print column A alone.
        layout.PrintedRange.LastColumn.ShouldBeGreaterThanOrEqualTo(11);
    }

    /// <summary>A worksheet part the package does not hold leaves a sheet rather than a throw.</summary>
    [Fact]
    public void AMissingWorksheetPartIsReportedAndTheWorkbookStillOpens()
    {
        using IDocument document = Open(XlsbPackage.Build(sheet: null));

        document.Content.Children.OfType<ContentSection>()
                .Count(s => s.Kind == SectionKind.Sheet).ShouldBe(1);
        document.Diagnostics.ShouldContain(d => d.Code == "PL2141");
    }

    /// <summary>A truncated part yields the records it had rather than throwing.</summary>
    /// <remarks>
    /// The same leniency the BIFF8 reader applies to a damaged record stream: a header that runs
    /// off the end of the part stops the walk, so a workbook whose last sheet was cut off still
    /// has readable earlier ones.
    /// </remarks>
    [Fact]
    public void ATruncatedRecordEndsTheWalkWithoutLosingWhatCameBefore()
    {
        Biff12Writer sheet = new();
        sheet.Begin(Ids.Worksheet);
        sheet.Begin(Ids.SheetData);
        sheet.Row(0).CellInlineString(0, "kept").End();

        // A header claiming forty bytes of payload where the part holds none, spliced onto the
        // end of a stream that is otherwise well formed.
        byte[] truncated = [.. sheet.ToArray(), (byte)Ids.CellDouble, 40];

        using IDocument document = Open(XlsbPackage.BuildFromBytes(truncated));
        Cell(document, 0, 0).Value.ShouldBe("kept");
    }

    /// <summary>An `xdr:pic` anchored to columns 8–11 of the first sheet.</summary>
    private const string PictureDrawing = """
        <?xml version="1.0" encoding="UTF-8"?>
        <xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
                  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
          <xdr:twoCellAnchor editAs="oneCell">
            <xdr:from><xdr:col>8</xdr:col><xdr:colOff>0</xdr:colOff>
                      <xdr:row>1</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>
            <xdr:to><xdr:col>11</xdr:col><xdr:colOff>9525</xdr:colOff>
                    <xdr:row>9</xdr:row><xdr:rowOff>9525</xdr:rowOff></xdr:to>
            <xdr:pic>
              <xdr:nvPicPr>
                <xdr:cNvPr id="2" name="Picture 1"/>
                <xdr:cNvPicPr/>
              </xdr:nvPicPr>
              <xdr:blipFill><a:blip/></xdr:blipFill>
              <xdr:spPr/>
            </xdr:pic>
            <xdr:clientData/>
          </xdr:twoCellAnchor>
        </xdr:wsDr>
        """;

    /// <summary>The BIFF12 identifiers these tests write.</summary>
    private static class Ids
    {
        public const int Workbook = 0x0083;
        public const int WorkbookPr = 0x0099;
        public const int Sheets = 0x008F;
        public const int Sheet = 0x009C;
        public const int Sst = 0x009F;
        public const int Si = 0x0013;
        public const int NumFmts = 0x0267;
        public const int NumFmt = 0x002C;
        public const int CellXfs = 0x0269;
        public const int CellStyleXfs = 0x0272;
        public const int Xf = 0x002F;
        public const int Worksheet = 0x0081;
        public const int SheetFormatPr = 0x01E5;
        public const int Col = 0x003C;
        public const int SheetData = 0x0091;
        public const int Row = 0x0000;
        public const int MergeCells = 0x00B1;
        public const int MergeCell = 0x00B0;
        public const int PageSetup = 0x01DE;
        public const int CellRk = 0x0002;
        public const int CellError = 0x0003;
        public const int CellBool = 0x0004;
        public const int CellDouble = 0x0005;
        public const int CellString = 0x0006;
        public const int CellSi = 0x0007;
        public const int CellRString = 0x003E;
        public const int FormulaString = 0x0008;
        public const int FormulaDouble = 0x0009;
        public const int MultCellRk = 0x000D;
    }

    /// <summary>
    /// Writes BIFF12 records: a variable-length identifier, a variable-length size, the payload.
    /// </summary>
    private sealed class Biff12Writer
    {
        private readonly List<byte> _bytes = [];

        public byte[] ToArray() => [.. _bytes];

        public Biff12Writer Record(int id, Action<Payload> body)
        {
            Payload payload = new();
            body(payload);
            byte[] bytes = payload.ToArray();

            WriteCompressed(id);
            WriteCompressed(bytes.Length);
            _bytes.AddRange(bytes);
            return this;
        }

        /// <summary>A container's start record, which carries nothing.</summary>
        public Biff12Writer Begin(int id) => Record(id, _ => { });

        /// <summary>Its end record, whose identifier is always the start's plus one.</summary>
        public Biff12Writer End(int id) => Record(id + 1, _ => { });

        public RowWriter Row(int index, int height = 0)
        {
            Record(Ids.Row, w => w
                .Int32(index).Int32(0).UInt16((ushort)height)
                .UInt16((ushort)(height > 0 ? 0x2000 : 0)).Byte(0).Int32(0));
            return new RowWriter(this);
        }

        private void WriteCompressed(int value)
        {
            uint left = (uint)value;
            while (true)
            {
                byte part = (byte)(left & 0x7F);
                left >>= 7;
                if (left == 0) { _bytes.Add(part); return; }
                _bytes.Add((byte)(part | 0x80));
            }
        }
    }

    /// <summary>Adds cells to the row most recently begun.</summary>
    private sealed class RowWriter(Biff12Writer sheet)
    {
        private int _column;

        public Biff12Writer End() => sheet;

        public RowWriter CellRk(int column, int integer)
        {
            _column = column;

            // Bit 1 set means the remaining 30 bits are a signed integer rather than the top
            // half of a double; bit 0 would divide it by a hundred.
            sheet.Record(Ids.CellRk, w => Header(w, column).Int32((integer << 2) | 0x02));
            return this;
        }

        public RowWriter MultCellRk(int integer)
        {
            _column++;
            sheet.Record(Ids.MultCellRk, w => Header(w, null).Int32((integer << 2) | 0x02));
            return this;
        }

        public RowWriter CellDouble(int column, double value, int styleIndex = 0)
        {
            _column = column;
            sheet.Record(Ids.CellDouble, w => Header(w, column, styleIndex).Double(value));
            return this;
        }

        public RowWriter CellBool(int column, bool value)
        {
            _column = column;
            sheet.Record(Ids.CellBool, w => Header(w, column).Byte((byte)(value ? 1 : 0)));
            return this;
        }

        public RowWriter CellError(int column, byte code)
        {
            _column = column;
            sheet.Record(Ids.CellError, w => Header(w, column).Byte(code));
            return this;
        }

        public RowWriter CellSi(int column, int index)
        {
            _column = column;
            sheet.Record(Ids.CellSi, w => Header(w, column).Int32(index));
            return this;
        }

        /// <summary>A string written in place, which carries no rich-text flag byte.</summary>
        public RowWriter CellInlineString(int column, string text)
        {
            _column = column;
            sheet.Record(Ids.CellString, w => Header(w, column).String(text));
            return this;
        }

        /// <summary>The same payload with the flag byte and the tails it announces.</summary>
        public RowWriter CellRichString(int column, string text)
        {
            _column = column;
            sheet.Record(Ids.CellRString, w => Header(w, column).Byte(0x00).String(text));
            return this;
        }

        public RowWriter FormulaDouble(int column, double value)
        {
            _column = column;
            sheet.Record(Ids.FormulaDouble, w => Header(w, column)
                .Double(value).UInt16(0).Int32(4).Byte(0x1E).Byte(0).Byte(0).Byte(0));
            return this;
        }

        public RowWriter FormulaString(int column, string text)
        {
            _column = column;
            // No flag byte: `importCellString` passes `bRich = false` and only `CELL_RSTRING`
            // passes true (`sheetdatacontext.cxx:551, :574`).
            sheet.Record(Ids.FormulaString, w => Header(w, column)
                .String(text).UInt16(0).Int32(0));
            return this;
        }

        private static Payload Header(Payload payload, int? column, int styleIndex = 0)
        {
            if (column is { } stated) payload.Int32(stated);
            return payload.Int32(styleIndex);
        }
    }

    /// <summary>One record's payload, in the little-endian primitives BIFF12 is made of.</summary>
    private sealed class Payload
    {
        private readonly List<byte> _bytes = [];

        public byte[] ToArray() => [.. _bytes];

        public Payload Byte(byte value)
        {
            _bytes.Add(value);
            return this;
        }

        public Payload UInt16(ushort value)
        {
            _bytes.AddRange(BitConverter.GetBytes(value));
            return this;
        }

        public Payload Int32(int value)
        {
            _bytes.AddRange(BitConverter.GetBytes(value));
            return this;
        }

        public Payload Double(double value)
        {
            _bytes.AddRange(BitConverter.GetBytes(value));
            return this;
        }

        /// <summary>A character count and that many UTF-16 code units.</summary>
        public Payload String(string text)
        {
            Int32(text.Length);
            foreach (char character in text) UInt16(character);
            return this;
        }
    }

    /// <summary>Wraps BIFF12 parts in the OPC package an XLSB is.</summary>
    private static class XlsbPackage
    {
        public static byte[] Build(
            Biff12Writer? sheet,
            IReadOnlyList<string>? strings = null,
            Biff12Writer? sharedStrings = null,
            Biff12Writer? styles = null,
            bool date1904 = false,
            int state = 0,
            string? drawing = null)
            => BuildFromBytes(sheet?.ToArray(), strings, sharedStrings, styles, date1904, state, drawing);

        public static byte[] BuildFromBytes(
            byte[]? sheet,
            IReadOnlyList<string>? strings = null,
            Biff12Writer? sharedStrings = null,
            Biff12Writer? styles = null,
            bool date1904 = false,
            int state = 0,
            string? drawing = null)
        {
            if (sharedStrings is null && strings is { Count: > 0 })
            {
                sharedStrings = new Biff12Writer();
                sharedStrings.Record(Ids.Sst, w => w.Int32(strings.Count).Int32(strings.Count));
                foreach (string text in strings)
                    sharedStrings.Record(Ids.Si, w => w.Byte(0x00).String(text));
            }

            Biff12Writer workbook = new();
            workbook.Begin(Ids.Workbook);
            workbook.Record(Ids.WorkbookPr, w => w.Int32(date1904 ? 1 : 0).String("en-GB"));
            workbook.Begin(Ids.Sheets);
            workbook.Record(Ids.Sheet, w => w.Int32(state).Int32(1).String("rId1").String("Sheet1"));
            workbook.End(Ids.Sheets);
            workbook.End(Ids.Workbook);

            using MemoryStream buffer = new();
            using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
            {
                Add(archive, "[Content_Types].xml", ContentTypes);
                Add(archive, "_rels/.rels", RootRelationships);
                Add(archive, "xl/_rels/workbook.bin.rels", WorkbookRelationships);
                Add(archive, "xl/workbook.bin", workbook.ToArray());

                if (sheet is not null) Add(archive, "xl/worksheets/sheet1.bin", sheet);
                if (sharedStrings is not null)
                    Add(archive, "xl/sharedStrings.bin", sharedStrings.ToArray());
                if (styles is not null) Add(archive, "xl/styles.bin", styles.ToArray());

                if (drawing is not null)
                {
                    Add(archive, "xl/worksheets/_rels/sheet1.bin.rels", SheetRelationships);
                    Add(archive, "xl/drawings/drawing1.xml", drawing);
                }
            }

            return buffer.ToArray();
        }

        private static void Add(ZipArchive archive, string name, string text)
            => Add(archive, name, Encoding.UTF8.GetBytes(text));

        private static void Add(ZipArchive archive, string name, byte[] bytes)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(bytes);
        }

        private const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Default Extension="bin" ContentType="application/vnd.ms-excel.sheet.binary.macroEnabled.main"/>
            </Types>
            """;

        private const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rIdWorkbook"
                Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"
                Target="xl/workbook.bin"/>
            </Relationships>
            """;

        private const string WorkbookRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1"
                Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet"
                Target="worksheets/sheet1.bin"/>
              <Relationship Id="rId2"
                Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings"
                Target="sharedStrings.bin"/>
              <Relationship Id="rId3"
                Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"
                Target="styles.bin"/>
            </Relationships>
            """;

        private const string SheetRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1"
                Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing"
                Target="../drawings/drawing1.xml"/>
            </Relationships>
            """;
    }
}
