using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An icon-set or data-bar conditional format whose <c>showValue</c> is false draws its icon
/// <em>instead of</em> the cell's text — except in a band whose custom icon is <c>NoIcons</c>,
/// where the cell keeps its text.
/// </summary>
/// <remarks>
/// <para>
/// Calc clears <c>bDoCell</c> before a cell's string is laid out when the cell carries icon-set or
/// data-bar information whose <c>showValue</c> is false
/// (<c>sc/source/ui/view/output2.cxx:1691-1698</c>). The same code runs for printing, so a hidden
/// value is hidden in the PDF.
/// </para>
/// <para>
/// The part that is not obvious from the schema is the exception.
/// <c>ScIconSetFormat::GetIconSetInfo</c> returns <strong>nothing at all</strong> when the band a
/// value falls in has a <c>NoIcons</c> entry in a custom icon vector
/// (<c>sc/source/core/data/colorscale.cxx:1231-1239</c>; <c>IconSetRule::importIcon</c> stores
/// <c>NoIcons</c> as index −1), and a cell with no icon information keeps its text however the
/// rule's <c>showValue</c> reads. One rule therefore hides some of its cells and prints the rest.
/// That is exactly what <c>077_Inventory_list_with_highlighting</c> does — thirteen <c>0</c>s
/// drawn, twelve <c>1</c>s replaced by a red flag — and reading the rule as a property of the
/// range would have hidden all twenty-five.
/// </para>
/// <para>
/// Bands are chosen by the <strong>last</strong> threshold a value satisfies, not the first
/// (<c>colorscale.cxx:1200-1215</c>), and the comparison is <c>&gt;=</c> unless the threshold
/// carries <c>gte="0"</c> (<c>condformatbuffer.cxx:118-124</c>).
/// </para>
/// <para>
/// The fixture is authored, one worksheet per shape, and <strong>every expectation below is read
/// out of LibreOffice 26.2.4.2's own PDF of it</strong>, which extracts as:
/// </para>
/// <code>
/// CUSTOMR   11   22
/// PLAINROW   PLAINSTRING
/// SHOWNRO   88   99
/// BARROW
/// GTEROW   50
/// </code>
/// <para>
/// <c>dotnet/probes/sheets-r52-condvalue/make-fixture.py</c> authors the file and records what
/// each sheet varies.
/// </para>
/// </remarks>
public sealed class SheetHiddenValueTests
{
    private static IReadOnlyList<DrawnPage> Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-hidden-values-xlsx.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink.Pages;
    }

    /// <summary>The texts drawn on one sheet's page; the fixture is one page per worksheet.</summary>
    private static IReadOnlyList<string> TextsOn(int page)
        => [.. Pages()[page].Runs.Select(run => run.Text)];

    [Fact]
    public void ACellInACustomBandOfNoIconsKeepsItsText()
    {
        // 11 and 22 both fall in the first band, whose custom icon is NoIcons, so the rule
        // produces no icon information for them at all and they print.
        IReadOnlyList<string> texts = TextsOn(0);
        texts.ShouldContain("11");
        texts.ShouldContain("22");
    }

    [Fact]
    public void ACellInACustomBandWithARealIconLosesItsText()
    {
        // 33 reaches the second band (>= 30) and 44 the third (>= 40); both have a real icon.
        IReadOnlyList<string> texts = TextsOn(0);
        texts.ShouldNotContain("33");
        texts.ShouldNotContain("44");
    }

    [Fact]
    public void AnIconSetThatIsNotCustomHidesEveryNumberItCovers()
    {
        // No custom vector means every band has an icon, so showValue="0" reaches all of them.
        IReadOnlyList<string> texts = TextsOn(1);
        texts.ShouldNotContain("55");
        texts.ShouldNotContain("66");
        texts.ShouldNotContain("77");
    }

    [Fact]
    public void AStringCellInsideAHiddenRangeKeepsItsText()
    {
        // `ScRefCellValue::hasNumeric` gates both GetIconSetInfo and GetDataBarInfo, so a text
        // cell inside the rule's own range is untouched by it.
        TextsOn(1).ShouldContain("PLAINSTRING");
    }

    [Fact]
    public void AnIconSetWithNoShowValueHidesNothing()
    {
        // The control, and the guard against the over-general version of this rule: the same
        // icon set with the attribute simply absent defaults to showing the value.
        IReadOnlyList<string> texts = TextsOn(2);
        texts.ShouldContain("88");
        texts.ShouldContain("99");
    }

    [Fact]
    public void ADataBarWithTheValueHiddenDrawsNoNumber()
    {
        IReadOnlyList<string> texts = TextsOn(3);
        texts.ShouldNotContain("123");
        texts.ShouldNotContain("456");
    }

    [Fact]
    public void AThresholdMarkedGteZeroExcludesTheValueSittingExactlyOnIt()
    {
        // gte="0" turns the band boundary from >= into >, so 50 stays in the NoIcons band below
        // it and prints, while 51 crosses into the flag band and does not.
        IReadOnlyList<string> texts = TextsOn(4);
        texts.ShouldContain("50");
        texts.ShouldNotContain("51");
    }
}
