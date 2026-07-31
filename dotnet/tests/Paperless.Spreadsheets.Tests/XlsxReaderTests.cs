using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the XLSX reader against corpus workbooks LibreOffice itself wrote.
/// </summary>
/// <remarks>
/// The assertions deliberately mirror <c>OdsReaderTests</c> case for case. A caller indexing a
/// mixed corpus is promised the same content tree from either format, and two test files that
/// assert the same things about the same workbook are what keeps that promise honest.
/// </remarks>
public class XlsxReaderTests
{
    private static IDocument Open(string name)
        => new SpreadsheetReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static ContentSection Sheet(IDocument document, string name)
        => document.Content.Children.OfType<ContentSection>()
                   .Single(s => s.Kind == SectionKind.Sheet && s.Name == name);

    private static ContentTable TableOf(ContentSection sheet)
        => sheet.Children.OfType<ContentTable>().Single();

    private static ContentTableCell Cell(ContentSection sheet, int row, int column)
        => TableOf(sheet).Children.Cast<ContentTableRow>()
                         .Single(r => r.Index == row)
                         .Children.Cast<ContentTableCell>()
                         .Single(c => c.Column == column);

    [Theory]
    [InlineData("sheet-xlsx.xlsx", DocumentFormat.Xlsx)]
    [InlineData("sheet-ooxml-features.xlsx", DocumentFormat.Xlsx)]
    [InlineData("sheet-ooxml-template.xltx", DocumentFormat.Xltx)]
    public void EveryOoxmlSpreadsheetVariantReadsThroughTheSamePath(
        string name, DocumentFormat expected)
    {
        using IDocument document = Open(name);

        document.Format.ShouldBe(expected);
        document.Family.ShouldBe(DocumentFamily.Spreadsheet);
        document.Content.GetText().ShouldContain("Region\tUnits\tPrice\tTotal");
    }

    [Fact]
    public void EachSheetIsItsOwnSectionInWorkbookOrder()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        List<ContentSection> sheets =
            [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == SectionKind.Sheet)];

        // The order is the workbook's <sheets> list, not the part names: sheet3.xml is only the
        // third sheet by coincidence, and a workbook that has had sheets deleted breaks that
        // coincidence.
        sheets.Select(s => s.Name).ShouldBe(["Sales", "Types", "Formats", "Hidden"]);
        sheets.Select(s => s.Index).ShouldBe([0, 1, 2, 3]);
    }

    [Fact]
    public void AHiddenSheetIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection hidden = Sheet(document, "Hidden");

        // LibreOffice's own CSV and PDF exports drop a hidden sheet entirely. Paperless keeps
        // it and flags it, because a caller indexing content wants it and a caller reproducing
        // what a reader sees can check the flag.
        hidden.IsHidden.ShouldBeTrue();
        hidden.GetText().ShouldContain("Content of a hidden sheet");

        Sheet(document, "Sales").IsHidden.ShouldBeFalse();
    }

    [Fact]
    public void CellsCarryBothTheirValueAndTheirDisplayedText()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection sales = Sheet(document, "Sales");

        // SpreadsheetML caches no display text at all — unlike ODF, which writes it beside the
        // value — so "£4.50" exists only once the cell's number format has been resolved and
        // applied. Getting this wrong shows the cell as "4.5".
        ContentTableCell price = Cell(sales, 1, 2);
        price.Value.ShouldBe(4.5);
        price.GetText().ShouldBe("£4.50");

        Cell(sales, 0, 0).Value.ShouldBe("Region");
        Cell(sales, 1, 1).Value.ShouldBe(12.0);
    }

    [Fact]
    public void FormulasKeepTheFilesOwnSyntax()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");

        // A1 references, not ODF's bracketed ones: the expression is left exactly as the file
        // writes it, because translating it would misreport what the document says.
        Cell(Sheet(document, "Sales"), 1, 3).Formula.ShouldBe("B2*C2");
        Cell(Sheet(document, "Sales"), 4, 3).Formula.ShouldBe("SUM(D2:D3)");
        Cell(Sheet(document, "Sales"), 0, 0).Formula.ShouldBeNull();
    }

    [Fact]
    public void ACachedFormulaResultIsTheCellsValue()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");

        // Nothing is recalculated. The cached result is what a reference renderer displays, so
        // it is what the cell reports.
        Cell(Sheet(document, "Sales"), 4, 3).Value.ShouldBe(85.5);
        Cell(Sheet(document, "Sales"), 4, 3).GetText().ShouldBe("£85.50");
    }

    [Fact]
    public void AnErrorCellBecomesACellError()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentTableCell error = Cell(Sheet(document, "Sales"), 6, 1);

        // t="e" stores the error as the literal string a reader shows, which is the only
        // statement of which error it was.
        error.Value.ShouldBe(CellError.DivideByZero);
        error.Formula.ShouldBe("1/0");
        error.GetText().ShouldBe("#DIV/0!");
    }

    [Fact]
    public void TypedValuesBecomeTheirClrEquivalents()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection types = Sheet(document, "Types");

        // Both of these are stored as plain numbers — 46233 and 0.604166666666667 — and only
        // the number format says they are a date and a time.
        Cell(types, 0, 1).Value.ShouldBe(new DateTime(2026, 7, 30));
        Cell(types, 1, 1).Value.ShouldBe(new TimeSpan(14, 30, 0));
        Cell(types, 2, 1).Value.ShouldBe(true);
        // A string that looks numeric stays a string, leading zeros and all.
        Cell(types, 3, 1).Value.ShouldBe("007");
        Cell(types, 4, 1).Value.ShouldBe("Spans two rows");
    }

    [Fact]
    public void ABooleanCellDisplaysTheWordsItsFormatAsks()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");

        // LibreOffice exports a boolean as t="b" with the number format "TRUE";"TRUE";"FALSE",
        // so the display text comes out of the format rather than out of the value.
        Cell(Sheet(document, "Types"), 2, 1).GetText().ShouldBe("TRUE");
    }

    [Fact]
    public void MergedRangesBecomeSpansAndCoveredCellsAreNotInvented()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection types = Sheet(document, "Types");

        // The merge is declared once in <mergeCells>, not on the cells, so a reader that only
        // walks <sheetData> reports no span at all.
        Cell(types, 4, 1).RowSpan.ShouldBe(2);

        TableOf(types).Children.Cast<ContentTableRow>().Single(r => r.Index == 5)
                      .Children.Count.ShouldBe(1);
    }

    [Fact]
    public void AddressingGapsBecomeEmptyCellsButTrailingPaddingDoesNot()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentTable sales = TableOf(Sheet(document, "Sales"));

        // Row 5 holds only A5 and D5. The two missing cells have to be materialised or the
        // row's text loses its columns; the ones past D5 must not be, or a sheet addressing
        // column XFD would materialise sixteen thousand of them.
        ContentTableRow grandTotal = sales.Children.Cast<ContentTableRow>().Single(r => r.Index == 4);
        grandTotal.Children.Count.ShouldBe(4);
        grandTotal.GetText().ShouldBe("Grand total\t\t\t£85.50\n");

        sales.Children.Count.ShouldBe(7);
        sales.Children.Cast<ContentTableRow>().Single(r => r.Index == 3).Children.ShouldBeEmpty();
        sales.ColumnCount.ShouldBe(4);
    }

    [Fact]
    public void ACellCommentBecomesItsOwnSection()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection comment = document.Content.Children.OfType<ContentSection>()
                                         .Single(s => s.Kind == SectionKind.Comment);

        // Comments hang off the worksheet part rather than the workbook, so their relationship
        // has to be resolved against the sheet.
        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A cell comment.");
    }

    [Fact]
    public void SheetTextIsOneLinePerRowWithTabSeparatedCells()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");

        Sheet(document, "Sales").GetText().ShouldStartWith(
            "Region\tUnits\tPrice\tTotal\nNorth\t12\t£4.50\t£54.00\n");
    }

    [Fact]
    public void NumberFormatsFromTheStyleSheetProduceTheTextLibreOfficeShows()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentSection formats = Sheet(document, "Formats");

        // Every one of these was checked against the text layer of LibreOffice's own PDF
        // export of this workbook.
        Cell(formats, 1, 1).GetText().ShouldBe("1234.5");        // General
        Cell(formats, 2, 1).GetText().ShouldBe("1234.50");       // 0.00
        Cell(formats, 3, 1).GetText().ShouldBe("1,234,567.89");  // #,##0.00
        Cell(formats, 4, 1).GetText().ShouldBe("7.5%");          // 0.0%
        Cell(formats, 5, 1).GetText().ShouldBe("1.23E+04");      // 0.00E+00
        Cell(formats, 6, 1).GetText().ShouldBe("72.5 kg");       // 0.0" kg"
        Cell(formats, 7, 1).GetText().ShouldBe("-1,234.50");     // #,##0.00;[RED]\-#,##0.00
        Cell(formats, 8, 1).GetText().ShouldBe("2 1/4");         // 0 ?/?
        Cell(formats, 9, 1).GetText().ShouldBe("30 July 2026");  // dd\ mmmm\ yyyy
    }

    [Fact]
    public void ThePercentageCellStoresTheFractionAndDisplaysThePercent()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");
        ContentTableCell share = Cell(Sheet(document, "Sales"), 5, 1);

        share.Value.ShouldBe(0.635);
        share.GetText().ShouldBe("63.5%");
    }

    [Fact]
    public void MetadataComesFromTheDocPropsParts()
    {
        using IDocument document = Open("sheet-ooxml-features.xlsx");

        document.Metadata.Title.ShouldBe("Paperless OOXML feature workbook");
        document.Metadata.Subject.ShouldBe("XLSX extraction");
        document.Metadata.CustomProperties["Project"].ShouldBe("Paperless");

        // OOXML records a workbook's sheet count as the "Sheets" extended property, and
        // LibreOffice's exporter simply does not write one — where its ODF exporter always
        // writes meta:table-count. It stays null rather than being counted from the sheets,
        // because null means "the file does not say" and inventing a value would make that
        // indistinguishable from a file that does.
        document.Metadata.Statistics!.SheetCount.ShouldBeNull();
    }

    [Fact]
    public void TheMinimalWorkbookExtractsTheSameCellsAsItsOdsTwin()
    {
        using IDocument xlsx = Open("sheet-xlsx.xlsx");
        using IDocument ods = Open("sheet-ods.ods");

        // Same source document, two formats, one content tree. This is the promise the whole
        // extraction design rests on.
        xlsx.Content.GetText().ShouldBe(ods.Content.GetText());
    }
}
