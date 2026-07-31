using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the ODS reader against corpus workbooks written by LibreOffice itself.
/// </summary>
public class OdsReaderTests
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
    [InlineData("sheet-ods.ods", DocumentFormat.Ods)]
    [InlineData("sheet-ots.ots", DocumentFormat.Ots)]
    [InlineData("sheet-fods.fods", DocumentFormat.Fods)]
    public void EveryOdfSpreadsheetVariantReadsThroughTheSamePath(string name, DocumentFormat expected)
    {
        using IDocument document = Open(name);

        document.Format.ShouldBe(expected);
        document.Family.ShouldBe(DocumentFamily.Spreadsheet);
        document.Content.GetText().ShouldContain("Region\tUnits\tPrice\tTotal");
    }

    [Fact]
    public void EachSheetIsItsOwnSectionInFileOrder()
    {
        using IDocument document = Open("sheet-features.ods");
        List<ContentSection> sheets =
            [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == SectionKind.Sheet)];

        sheets.Select(s => s.Name).ShouldBe(["Sales", "Types", "Hidden"]);
        sheets.Select(s => s.Index).ShouldBe([0, 1, 2]);
    }

    [Fact]
    public void AHiddenSheetIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentSection hidden = Sheet(document, "Hidden");

        // A caller indexing content wants a hidden sheet; a caller reproducing what a reader
        // sees does not. Flagging rather than skipping serves both.
        hidden.IsHidden.ShouldBeTrue();
        hidden.GetText().ShouldContain("Content of a hidden sheet");

        Sheet(document, "Sales").IsHidden.ShouldBeFalse();
    }

    [Fact]
    public void CellsCarryBothTheirValueAndTheirDisplayedText()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentSection sales = Sheet(document, "Sales");

        // The value is the number; the children are the number run through its format. The
        // two genuinely differ and consumers want different ones.
        ContentTableCell price = Cell(sales, 1, 2);
        price.Value.ShouldBe(4.5);
        price.GetText().ShouldBe("£4.50");

        Cell(sales, 0, 0).Value.ShouldBe("Region");
        Cell(sales, 1, 1).Value.ShouldBe(12.0);
    }

    [Fact]
    public void FormulasKeepTheFilesOwnSyntaxWithoutThePrefix()
    {
        using IDocument document = Open("sheet-features.ods");

        // The "of:" namespace prefix names the formula language and is not part of the
        // expression; the bracket references are, so they stay.
        Cell(Sheet(document, "Sales"), 1, 3).Formula.ShouldBe("[.B2]*[.C2]");
        Cell(Sheet(document, "Sales"), 4, 3).Formula.ShouldBe("SUM([.D2:.D3])");
        Cell(Sheet(document, "Sales"), 0, 0).Formula.ShouldBeNull();
    }

    [Fact]
    public void AnErrorResultBecomesACellError()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentTableCell error = Cell(Sheet(document, "Sales"), 6, 1);

        // ODF cannot say "this cell is an error": LibreOffice marks it in its own namespace
        // and the kind is only recoverable from the displayed text.
        error.Value.ShouldBe(CellError.DivideByZero);
        error.Formula.ShouldBe("1/0");
        error.GetText().ShouldBe("#DIV/0!");
    }

    [Fact]
    public void TypedValuesBecomeTheirClrEquivalents()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentSection types = Sheet(document, "Types");

        Cell(types, 0, 1).Value.ShouldBe(new DateTime(2026, 7, 30));
        // A time is a TimeSpan, not a time-of-day DateTime: the file records a duration since
        // midnight and inventing a date would be inventing data.
        Cell(types, 1, 1).Value.ShouldBe(new TimeSpan(14, 30, 0));
        Cell(types, 2, 1).Value.ShouldBe(true);
        // A string that looks numeric stays a string, leading zero and all.
        Cell(types, 3, 1).Value.ShouldBe("007");
        Cell(types, 4, 1).Value.ShouldBe("Spans two rows");
    }

    [Fact]
    public void PercentagesAndCurrenciesAreStoredAsTheirUnderlyingNumber()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentTableCell share = Cell(Sheet(document, "Sales"), 5, 1);

        share.Value.ShouldBe(0.635);
        share.GetText().ShouldBe("63.5%");
    }

    [Fact]
    public void RowSpansAreRecordedAndCoveredCellsAreNotInvented()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentSection types = Sheet(document, "Types");

        Cell(types, 4, 1).RowSpan.ShouldBe(2);

        // The covered cell the span hides holds nothing of its own, so the row below has only
        // its first cell rather than a phantom second one.
        TableOf(types).Children.Cast<ContentTableRow>().Single(r => r.Index == 5)
                      .Children.Count.ShouldBe(1);
    }

    [Fact]
    public void EmptyRowsInsideTheUsedRangeSurviveButTrailingPaddingDoesNot()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentTable sales = TableOf(Sheet(document, "Sales"));

        // A spreadsheet pads every row to the sheet's full width and the sheet to its full
        // height. Expanding that padding would materialise millions of cells; dropping the
        // blank row in the middle would lose a real blank line.
        sales.Children.Count.ShouldBe(7);
        sales.Children.Cast<ContentTableRow>().Single(r => r.Index == 3).Children.ShouldBeEmpty();
        sales.ColumnCount.ShouldBe(4);
    }

    [Fact]
    public void ACellCommentBecomesItsOwnSection()
    {
        using IDocument document = Open("sheet-features.ods");
        ContentSection comment = document.Content.Children.OfType<ContentSection>()
                                         .Single(s => s.Kind == SectionKind.Comment);

        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A cell comment.");
    }

    [Fact]
    public void SheetTextIsOneLinePerRowWithTabSeparatedCells()
    {
        using IDocument document = Open("sheet-features.ods");

        // Not one line per cell: a table's extracted text is unusable that way, and this is
        // also the shape LibreOffice's CSV export produces.
        Sheet(document, "Sales").GetText().ShouldStartWith(
            "Region\tUnits\tPrice\tTotal\nNorth\t12\t£4.50\t£54.00\n");
    }

    [Fact]
    public void MetadataReadsTheSheetCountFromTheTableCounter()
    {
        using IDocument document = Open("sheet-features.ods");

        // ODF records a spreadsheet's sheet count in meta:table-count, the same attribute
        // that means "tables" in a text document.
        document.Metadata.Statistics!.SheetCount.ShouldBe(3);
        document.Metadata.Statistics!.TableCount.ShouldBeNull();
        document.Metadata.Title.ShouldBe("Paperless feature workbook");
    }

    [Fact]
    public void AFormatWithNoReaderYetIsReportedAsUnsupported()
    {
        UnsupportedFormatException unimplemented = Should.Throw<UnsupportedFormatException>(
            () => Open("sheet-csv.csv"));
        unimplemented.Format.ShouldBe(DocumentFormat.Csv);
        unimplemented.Message.ShouldContain("not implemented yet");
    }
}
