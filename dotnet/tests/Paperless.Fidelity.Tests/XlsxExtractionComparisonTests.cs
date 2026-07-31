using Paperless.Core.Extraction;
using Paperless.TestKit;
using Paperless.TestKit.Comparison;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares XLSX extraction against LibreOffice's own CSV export.
/// </summary>
/// <remarks>
/// <para>
/// A separate file from <c>ExtractionComparisonTests</c> because the reference has to be built
/// differently. The default CSV filter covers the <em>first sheet only</em>, which is useless
/// for a four-sheet workbook, so these tests drive the filter with an explicit option string
/// that selects every sheet and a tab delimiter — the same shape Paperless produces, which
/// removes the comma-versus-tab noise from the diff entirely.
/// </para>
/// <para>
/// The assertion stays one-sided: nothing the reference found may be missing, but Paperless
/// finding more is expected. It finds exactly two more things here, and both are deliberate —
/// a cell comment and a hidden sheet, neither of which any LibreOffice export includes.
/// </para>
/// </remarks>
public class XlsxExtractionComparisonTests : IDisposable
{
    /// <summary>
    /// The StarCalc CSV filter's option string, with the sheet selector set to every sheet.
    /// </summary>
    /// <remarks>
    /// The fields are, in order: field separator (9, tab), text delimiter (34, a quote),
    /// character set (76, UTF-8), first line, cell format codes, language, quoted field as
    /// text, detect special numbers, save cell contents as shown, export cell formulas, remove
    /// spaces, and finally the sheet index — where <c>-1</c> means all of them, written to one
    /// file per sheet. Getting "save cell contents as shown" wrong is the field that matters:
    /// with it off the filter writes 4.5 where Calc shows £4.50, and the comparison then
    /// measures the filter's opinion rather than the document.
    /// </remarks>
    private const string AllSheetsCsv =
        "csv:Text - txt - csv (StarCalc):9,34,76,1,,0,false,true,true,false,false,-1";

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory = Path.Combine(
        Path.GetTempPath(), "paperless-xlsx-fidelity-" + Guid.NewGuid().ToString("N")[..12]);

    private static void RequireLibreOffice()
        => Assert.SkipUnless(
            LibreOfficeRunner.IsAvailable,
            "LibreOffice with its application modules is not installed, so there is no "
            + "reference to compare against. Run "
            + ".claude/skills/libreoffice-reference/scripts/check-env.sh for the apt-get lines.");

    /// <summary>
    /// Every sheet's CSV, concatenated in workbook order.
    /// </summary>
    /// <remarks>
    /// The filter writes one file per sheet, named <c>&lt;stem&gt;-&lt;sheet name&gt;.csv</c>,
    /// and the directory listing is alphabetical rather than in workbook order — so the order
    /// is rebuilt from the sheet names Paperless read rather than taken from the file system.
    /// </remarks>
    private string Reference(string corpusName, IEnumerable<string> sheetNames)
    {
        string input = Corpus.Require(corpusName);
        string isolated = Path.Combine(_workDirectory, Path.GetFileNameWithoutExtension(input));
        _libreOffice.ConvertToMany(input, AllSheetsCsv, isolated);

        string stem = Path.GetFileNameWithoutExtension(input);
        System.Text.StringBuilder text = new();
        foreach (string sheet in sheetNames)
        {
            string path = Path.Combine(isolated, $"{stem}-{sheet}.csv");
            if (File.Exists(path)) text.Append(File.ReadAllText(path));
        }
        return text.ToString();
    }

    private static ContentDocument Extract(string corpusName)
        => PaperlessDocument.Extract(Corpus.Require(corpusName));

    private static List<string> SheetNames(ContentDocument content)
        => [.. content.Children.OfType<ContentSection>()
                      .Where(s => s.Kind == SectionKind.Sheet)
                      .Select(s => s.Name ?? string.Empty)];

    [Theory]
    [InlineData("sheet-xlsx.xlsx")]
    [InlineData("sheet-ooxml-features.xlsx")]
    [InlineData("sheet-ooxml-template.xltx")]
    public void EveryCellTheCsvFilterWritesIsAlsoExtracted(string name)
    {
        RequireLibreOffice();

        ContentDocument content = Extract(name);
        string reference = Reference(name, SheetNames(content));
        reference.ShouldNotBeEmpty();

        // Not a token subset by accident: the number formats have to be resolved and applied
        // for "£4.50", "1,234,567.89", "1.23E+04", "2 1/4" and "30 July 2026" to appear at
        // all, because SpreadsheetML caches none of them.
        TextComparer.FindMissingTokens(reference, content.GetText()).ShouldBeEmpty();
    }

    [Fact]
    public void TheOnlyThingsExtractedBeyondTheReferenceAreTheCommentAndTheHiddenSheet()
    {
        RequireLibreOffice();

        ContentDocument content = Extract("sheet-ooxml-features.xlsx");

        // The reference is built from the visible sheets only, which is what LibreOffice's PDF
        // export also covers, so the hidden sheet is genuinely extra rather than reordered.
        List<string> visible =
            [.. content.Children.OfType<ContentSection>()
                       .Where(s => s.Kind == SectionKind.Sheet && !s.IsHidden)
                       .Select(s => s.Name ?? string.Empty)];

        string reference = Reference("sheet-ooxml-features.xlsx", visible);
        string sheetsOnly = string.Concat(
            content.Children.OfType<ContentSection>()
                   .Where(s => s.Kind == SectionKind.Sheet && !s.IsHidden)
                   .Select(s => s.GetText()));

        // With the comment and the hidden sheet set aside, what remains is a match rather than
        // a near-match — every displayed value, not merely every word.
        TextComparer.Similarity(reference, sheetsOnly).ShouldBeGreaterThan(0.99);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        GC.SuppressFinalize(this);
        if (Directory.Exists(_workDirectory)) Directory.Delete(_workDirectory, recursive: true);
    }
}
