using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Spreadsheets.Csv;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the separated-value reader, whose whole job is guessing well and saying what it
/// guessed.
/// </summary>
/// <remarks>
/// There is no reference to compare against here in the way the other formats have one.
/// LibreOffice's own CSV <em>import</em> uses whatever options its caller passed rather than
/// detecting anything, so a headless conversion of the semicolon file below reads it as one
/// column. That is not a better answer than this reader's, only a differently-configured one
/// — which is exactly why every decision is recorded as a diagnostic.
/// </remarks>
public class CsvReaderTests
{
    private static IDocument Open(string name)
        => new SpreadsheetReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static ContentTable TableOf(IDocument document)
        => document.Content.Children.OfType<ContentSection>().Single()
                   .Children.OfType<ContentTable>().Single();

    private static string CellText(IDocument document, int row, int column)
        => TableOf(document).Children.Cast<ContentTableRow>().Single(r => r.Index == row)
                            .Children.Cast<ContentTableCell>().Single(c => c.Column == column)
                            .GetText();

    private static CsvDialect Detect(string text, Encoding encoding, List<Diagnostic> diagnostics)
        => CsvDialect.Detect(encoding.GetBytes(text), null, diagnostics);

    [Fact]
    public void ACommaFileReadsAsATableOfItsFields()
    {
        using IDocument document = Open("sheet-csv.csv");

        document.Format.ShouldBe(DocumentFormat.Csv);
        document.Family.ShouldBe(DocumentFamily.Spreadsheet);
        TableOf(document).ColumnCount.ShouldBe(4);
        CellText(document, 0, 0).ShouldBe("Region");
        CellText(document, 1, 1).ShouldBe("12");
    }

    [Fact]
    public void FieldsAreExtractedAsWrittenRatherThanInterpreted()
    {
        using IDocument document = Open("sheet-csv.csv");

        // Calc's import would compile this into a formula and show 54 instead. Extraction
        // reports what the file says; a file of text is text until somebody decides otherwise.
        CellText(document, 1, 3).ShouldBe("=B2*C2");

        // And 4.50 keeps its trailing zero, which converting to a number would lose.
        CellText(document, 1, 2).ShouldBe("4.50");
    }

    [Fact]
    public void AFieldsValueIsANumberOnlyWhenTheTextIsOneExactly()
    {
        using IDocument document = Open("sheet-csv.csv");
        ContentTableRow row = TableOf(document).Children.Cast<ContentTableRow>().Single(r => r.Index == 1);
        List<ContentTableCell> cells = [.. row.Children.Cast<ContentTableCell>()];

        cells[1].Value.ShouldBe(12.0);

        // 4.50 does not format back to itself, so it stays the text the file holds.
        cells[2].Value.ShouldBe("4.50");
    }

    [Fact]
    public void ASemicolonFileIsDetectedByHowConsistentlyItSplits()
    {
        using IDocument document = Open("csv-semicolon.csv");

        TableOf(document).ColumnCount.ShouldBe(4);
        CellText(document, 1, 0).ShouldBe("North");

        // The decimal comma is a field, not a separator — which is the whole reason this file
        // uses semicolons and the whole reason detection has to look at consistency.
        CellText(document, 1, 2).ShouldBe("4,50");
    }

    [Fact]
    public void QuotingHidesSeparatorsLineBreaksAndQuotes()
    {
        using IDocument document = Open("csv-semicolon.csv");

        CellText(document, 1, 3).ShouldBe("Contains a ; separator");
        CellText(document, 2, 3).ShouldBe("Two\nlines");
        CellText(document, 3, 3).ShouldBe("He said \"yes\"");
    }

    [Fact]
    public void ByteThatAreNotValidUtf8FallBackToASingleByteEncoding()
    {
        using IDocument document = Open("csv-latin1.csv");

        // The tell of getting this wrong is systematic corruption of the accented characters
        // while the ASCII stays perfect.
        CellText(document, 1, 0).ShouldBe("München");
        CellText(document, 2, 2).ShouldBe("naïve");

        document.Diagnostics.ShouldContain(
            d => d.Code == "PL2340" && d.Message.Contains("windows-1252", StringComparison.Ordinal));
    }

    [Fact]
    public void EveryDetectionDecisionIsReportedAsADiagnostic()
    {
        using IDocument document = Open("csv-semicolon.csv");

        // A mismatch against another reader is usually a different guess about an ambiguous
        // file rather than a defect, and that is only arguable if the guess is visible.
        document.Diagnostics.ShouldContain(d => d.Code == "PL2340");
        document.Diagnostics.ShouldContain(
            d => d.Code == "PL2341" && d.Message.Contains("semicolon", StringComparison.Ordinal));
        document.Diagnostics.ShouldAllBe(d => d.Severity == DiagnosticSeverity.Information);
    }

    [Fact]
    public void TheDialectIsReachableSoACallerThatKnowsBetterCanSeeWhatWasAssumed()
    {
        using IDocument document = Open("csv-semicolon.csv");

        CsvDocument csv = document.ShouldBeOfType<CsvDocument>();
        csv.Dialect.Separator.ShouldBe(';');
        csv.Dialect.Quote.ShouldBe('"');
    }

    [Theory]
    [InlineData("a,b,c\nd,e,f\n", ',')]
    [InlineData("a;b;c\nd;e;f\n", ';')]
    [InlineData("a\tb\tc\nd\te\tf\n", '\t')]
    [InlineData("a|b|c\nd|e|f\n", '|')]
    public void EachCandidateSeparatorIsFoundWhenItIsTheConsistentOne(string text, char expected)
        => Detect(text, Encoding.UTF8, []).Separator.ShouldBe(expected);

    [Fact]
    public void ProseIsNotSplitOnItsPunctuation()
    {
        // Frequency alone would choose the comma here: it occurs more often than any real
        // separator would. Consistency is what says it is not one.
        List<Diagnostic> diagnostics = [];
        CsvDialect dialect = Detect(
            "The first line, which has a comma\nThe second, and the third, have two\n",
            Encoding.UTF8,
            diagnostics);

        // Two lines with one comma and two commas: inconsistent, so the file is one column of
        // prose rather than a two-column table.
        dialect.Separator.ShouldBe(',');
        diagnostics.ShouldContain(d => d.Code == "PL2341" && d.Message.Contains("disagree", StringComparison.Ordinal));
    }

    [Fact]
    public void AByteOrderMarkSettlesTheEncodingOutright()
    {
        List<Diagnostic> diagnostics = [];
        byte[] bytes = [.. Encoding.UTF8.GetPreamble(), .. Encoding.UTF8.GetBytes("a,é\n")];

        CsvDialect dialect = CsvDialect.Detect(bytes, null, diagnostics);
        dialect.EncodingFromByteOrderMark.ShouldBeTrue();
        dialect.Decode(bytes).ShouldBe("a,é\n");
        diagnostics.ShouldContain(d => d.Message.Contains("byte-order mark", StringComparison.Ordinal));
    }

    [Fact]
    public void AFileWithNoQuotedFieldTreatsQuotesAsOrdinaryText()
    {
        List<Diagnostic> diagnostics = [];
        CsvDialect dialect = Detect("size,note\n6\",a 6\" pipe\n", Encoding.UTF8, diagnostics);

        // An inch mark is not quoting, and treating it as such would swallow the rest of the
        // file into one field.
        dialect.Quote.ShouldBeNull();
        diagnostics.ShouldContain(d => d.Code == "PL2342");
    }

    [Fact]
    public void AnEmptyFileIsASheetWithNothingInIt()
    {
        // Read directly rather than through the reader's front door: identification declines
        // to call an empty file anything at all, which is its business rather than this
        // reader's.
        using DocumentSource source = DocumentSource.FromBytes(Array.Empty<byte>(), "empty.csv");
        using CsvDocument document = CsvReader.Read(source, DocumentFormat.Csv);

        document.Content.GetText().Trim().ShouldBeEmpty();
        document.Metadata.Statistics!.SheetCount.ShouldBe(1);
    }
}
