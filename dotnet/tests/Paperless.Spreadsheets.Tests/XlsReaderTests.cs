using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the BIFF8 reader against corpus workbooks LibreOffice itself wrote.
/// </summary>
/// <remarks>
/// The expectations here are what LibreOffice's own rendering of the same file shows, not
/// what its CSV export shows. The two disagree on currency — the CSV filter writes 4.5 where
/// the rendering shows £4.50, and does the same to the ODS version of the same workbook — so
/// the rendering is the one that says what a reader sees.
/// </remarks>
public class XlsReaderTests
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

    [Fact]
    public void AWorkbookReadsThroughTheXlsPath()
    {
        using IDocument document = Open("sheet-xls.xls");

        document.Format.ShouldBe(DocumentFormat.Xls);
        document.Family.ShouldBe(DocumentFamily.Spreadsheet);

        // Character for character what LibreOffice's own CSV export of this file produces,
        // with its commas as the tabs a content table joins cells with.
        document.Content.GetText().ShouldBe(
            "Region\tUnits\tPrice\tTotal\n"
            + "North\t12\t4.5\t54\n"
            + "South\t7\t4.5\t31.5\n"
            + "East\t23\t3.25\t74.75\n"
            + "West\t4\t3.25\t13\n");
    }

    [Fact]
    public void EachSheetIsItsOwnSectionInFileOrder()
    {
        using IDocument document = Open("xls-features.xls");
        List<ContentSection> sheets =
            [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == SectionKind.Sheet)];

        sheets.Select(s => s.Name).ShouldBe(["Sales", "Types", "Strings", "Hidden"]);
        sheets.Select(s => s.Index).ShouldBe([0, 1, 2, 3]);
    }

    [Fact]
    public void AHiddenSheetIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection hidden = Sheet(document, "Hidden");

        hidden.IsHidden.ShouldBeTrue();
        hidden.GetText().ShouldContain("Content of a hidden sheet");
        Sheet(document, "Sales").IsHidden.ShouldBeFalse();
    }

    [Fact]
    public void CellsCarryBothTheirValueAndTheTextTheirFormatProduces()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection sales = Sheet(document, "Sales");

        // BIFF stores 4.5 and a format; unlike ODF it caches no displayed text, so the text
        // below exists only because the format was applied.
        ContentTableCell price = Cell(sales, 1, 2);
        price.Value.ShouldBe(4.5);
        price.GetText().ShouldBe("£4.50");

        Cell(sales, 0, 0).Value.ShouldBe("Region");
        Cell(sales, 1, 1).Value.ShouldBe(12.0);
    }

    [Fact]
    public void ACompactRkNumberDecodesToTheSameValueAsAFullDouble()
    {
        using IDocument document = Open("xls-features.xls");

        // Written as a MULRK: 12 as a 30-bit integer, 4.5 as an integer of hundredths. Both
        // flag combinations produce a plausible number from the same bits when misread, which
        // is why they are asserted rather than assumed.
        Cell(Sheet(document, "Sales"), 1, 1).Value.ShouldBe(12.0);
        Cell(Sheet(document, "Sales"), 1, 2).Value.ShouldBe(4.5);

        // And a NUMBER record's full double, for contrast.
        Cell(Sheet(document, "Types"), 5, 1).Value.ShouldBe(1234567.891);
    }

    [Fact]
    public void AFormulasCachedResultIsReportedForEachOfItsFourKinds()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection sales = Sheet(document, "Sales");

        // A double, in the record itself.
        Cell(sales, 1, 3).Value.ShouldBe(54.0);
        Cell(sales, 1, 3).GetText().ShouldBe("£54.00");

        // An error code.
        Cell(sales, 8, 1).Value.ShouldBe(CellError.DivideByZero);
        Cell(sales, 8, 1).GetText().ShouldBe("#DIV/0!");

        // A string, which lives in the STRING record that follows the FORMULA.
        Cell(sales, 9, 1).Value.ShouldBe("NORTH");

        // A boolean.
        Cell(sales, 10, 1).Value.ShouldBe(true);
    }

    [Fact]
    public void FormulasThemselvesAreNotReportedBecauseTheirTokensAreNotDecoded()
    {
        using IDocument document = Open("xls-features.xls");

        // Deliberate: the cached result is what a reference renderer shows, and reporting a
        // half-decoded expression would be worse than reporting none. The difference from the
        // ODS reader, which does populate Formula, is visible here on purpose.
        Cell(Sheet(document, "Sales"), 1, 3).Formula.ShouldBeNull();
    }

    [Fact]
    public void ADateIsTypedAndDisplayedByWhatItsNumberFormatSays()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection types = Sheet(document, "Types");

        Cell(types, 0, 1).Value.ShouldBe(new DateTime(2026, 7, 30));
        Cell(types, 0, 1).GetText().ShouldBe("2026-07-30");

        // A time is a TimeSpan, not a time-of-day DateTime: the file records a fraction of a
        // day and cannot say whether it meant a clock reading or an elapsed duration.
        Cell(types, 1, 1).Value.ShouldBe(new TimeSpan(14, 30, 0));
        Cell(types, 1, 1).GetText().ShouldBe("14:30");

        Cell(types, 2, 1).Value.ShouldBe(new DateTime(2026, 7, 30, 14, 30, 0));
        Cell(types, 2, 1).GetText().ShouldBe("30/07/2026 14:30");
    }

    [Fact]
    public void NumberFormatsProduceTheTextTheRenderingShows()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection types = Sheet(document, "Types");

        Cell(types, 5, 1).GetText().ShouldBe("1,234,567.89");
        Cell(types, 6, 1).GetText().ShouldBe("1.23E-04");
        Cell(types, 7, 1).GetText().ShouldBe("Qty 2.5 units");
        Cell(types, 8, 1).GetText().ShouldBe("-1,234.50");
        Cell(types, 9, 1).GetText().ShouldBe("42");
        Cell(Sheet(document, "Sales"), 7, 1).GetText().ShouldBe("63.5%");
    }

    [Fact]
    public void ABooleanCellReadsAsTrueRatherThanAsOne()
    {
        using IDocument document = Open("xls-features.xls");

        // A named divergence. Calc has no boolean cell type, so its own rendering of this
        // file shows 1; Excel, which wrote the format, shows TRUE, and so does the ODS
        // reader. The value carries the boolean either way.
        Cell(Sheet(document, "Types"), 3, 1).Value.ShouldBe(true);
        Cell(Sheet(document, "Types"), 3, 1).GetText().ShouldBe("TRUE");
    }

    [Fact]
    public void TextThatLooksNumericStaysText()
    {
        using IDocument document = Open("xls-features.xls");
        Cell(Sheet(document, "Types"), 4, 1).Value.ShouldBe("007");
    }

    [Fact]
    public void NonAsciiTextSurvivesTheCompressedUnicodeEncoding()
    {
        using IDocument document = Open("xls-features.xls");

        // BIFF8 stores a string whose characters all fit in a byte as one byte each, and
        // those bytes are Latin-1 code points rather than code-page bytes. An em dash does
        // not fit, so this string is stored wide and exercises the other branch.
        Cell(Sheet(document, "Types"), 10, 1).Value.ShouldBe("René Maître — café");
    }

    [Fact]
    public void AStringTableLargerThanOneRecordIsReadAcrossItsContinuations()
    {
        using IDocument document = Open("xls-features.xls");
        ContentTable strings = TableOf(Sheet(document, "Strings"));

        // The table in this workbook runs past the 8224-byte record ceiling, so the last
        // rows are only readable if the CONTINUE records were followed — and only readable
        // *correctly* if the flags byte at each boundary was re-read.
        strings.Children.Count.ShouldBe(49);
        ((ContentTableRow)strings.Children[^1]).GetText()
            .ShouldStartWith("Row 47: the quick brown fox");
        document.Content.GetText().ShouldNotContain("�");
    }

    [Fact]
    public void AMergedRangeBecomesASpanAndTheCellsItHidesAreNotInvented()
    {
        using IDocument document = Open("xls-features.xls");
        ContentSection sales = Sheet(document, "Sales");

        ContentTableCell merged = Cell(sales, 12, 0);
        merged.ColumnSpan.ShouldBe(3);
        merged.GetText().ShouldBe("Merged across three columns");

        // BIFF really does write the hidden cells — as a MULBLANK across the rest of the
        // range — so without dropping them the row would gain two phantom columns.
        TableOf(sales).Children.Cast<ContentTableRow>().Single(r => r.Index == 12)
                      .Children.Count.ShouldBe(1);
    }

    [Fact]
    public void AGapInARowKeepsItsColumnsInPlace()
    {
        using IDocument document = Open("xls-features.xls");

        // Nothing is written for an untouched cell, so the two columns between "Grand total"
        // and its sum exist only because the reader fills the gap. Without that the total
        // would extract two columns too far left.
        ContentTableRow total = TableOf(Sheet(document, "Sales"))
            .Children.Cast<ContentTableRow>().Single(r => r.Index == 6);

        total.Children.Count.ShouldBe(4);
        total.GetText().ShouldBe("Grand total\t\t\t£173.25\n");
    }

    [Fact]
    public void EmptyRowsInsideTheUsedRangeSurvive()
    {
        using IDocument document = Open("xls-features.xls");
        ContentTable sales = TableOf(Sheet(document, "Sales"));

        sales.Children.Cast<ContentTableRow>().Single(r => r.Index == 5).Children.ShouldBeEmpty();
        sales.ColumnCount.ShouldBe(4);
    }

    [Fact]
    public void SheetTextIsOneLinePerRowWithTabSeparatedCells()
    {
        using IDocument document = Open("xls-features.xls");

        Sheet(document, "Sales").GetText().ShouldStartWith(
            "Region\tUnits\tPrice\tTotal\nNorth\t12\t£4.50\t£54.00\n");
    }

    [Fact]
    public void MetadataComesFromTheOlePropertySetsAndTheSheetDirectory()
    {
        using IDocument document = Open("xls-features.xls");

        document.Metadata.Title.ShouldBe("Paperless XLS feature workbook");
        document.Metadata.Subject.ShouldBe("BIFF8 record coverage");

        // No BIFF record states a sheet count, so it is the size of the sheet directory —
        // hidden sheets included, which is what ODF's meta:table-count means as well.
        document.Metadata.Statistics!.SheetCount.ShouldBe(4);
    }

    [Fact]
    public void AWorkbookReadsWithoutComplaint()
    {
        using IDocument document = Open("xls-features.xls");

        // Diagnostics are how this reader reports damage, so a file LibreOffice wrote
        // producing any at all would mean the reader misunderstands something.
        document.Diagnostics.ShouldBeEmpty();
    }
}
