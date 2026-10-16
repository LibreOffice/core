using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.OpenDocument;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Three rules an <c>.odt</c>'s tables need and which this reader answered with silence: the document
/// setting that puts a row's insets into its declared height, a covered cell's stated rules, and a page
/// break stated on the table's own style.
/// </summary>
/// <remarks>
/// <para>
/// The fixture is 26.2.4.2's own conversion of <see cref="TableCoveredCellRuleTests"/>'s
/// <c>words-vmerge-covered-rule.docx</c>, so the two read the same eight arms through two readers and
/// the expectations are the same numbers. Its assertions are the reference's own rendering of it: after
/// these three rules every baseline and every rule of all eight pages agrees to the hundredth.
/// </para>
/// <para>
/// [bin] Measured on 26.2.4.2 over the 34 arms of <c>probes/vmergetop-r154/</c>'s four fixtures converted
/// to <c>.odt</c>: <b>13 differed and 0 do now</b>, and the fixture that went from one page to eight is
/// this one. <c>probes/odtrowheight-r155/</c>.
/// </para>
/// </remarks>
public sealed class OdtTableRowHeightTests
{
    private const string Fixture = "odt-table-row-height.fodt";

    private const int FloorCovered = 0;
    private const int FloorBare = 1;
    private const int BandCovered = 2;
    private const int BandBare = 3;
    private const int BelowCovered = 4;
    private const int BelowBare = 5;
    private const int OuterCovered = 6;
    private const int OuterBare = 7;

    /// <summary>
    /// <c>MinRowHeightInclBorder</c> is a <em>document setting</em>, not a property of the Word filters,
    /// and an absent one means false.
    /// </summary>
    /// <remarks>
    /// The one filter that sets it is WW8's (<c>sw/source/filter/ww8/ww8par.cxx</c>:1966); LibreOffice's
    /// ODF export then writes the resulting state into <c>settings.xml</c>, which is why every one of the
    /// 337 converted <c>.odt</c> carries it. Writer's own default is false
    /// (<c>DocumentSettingManager.cxx</c>:113), so an unread setting is a missing charge.
    /// </remarks>
    [Theory]
    [InlineData("true", true)]
    [InlineData("false", false)]
    [InlineData(null, false)]
    public void TheRowHeightSettingIsReadAndDefaultsToFalse(string? stated, bool expected)
    {
        XElement? settings = stated is null
            ? null
            : XElement.Parse(
                $"""
                 <office:settings xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
                                  xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0">
                   <config:config-item-set config:name="ooo:configuration-settings">
                     <config:config-item config:name="MinRowHeightInclBorder"
                                         config:type="boolean">{stated}</config:config-item>
                   </config:config-item-set>
                 </office:settings>
                 """);

        OdtLayoutSource.RowHeightsIncludeInsets(settings).ShouldBe(expected);
    }

    /// <summary>
    /// A page break stated on the table's own style, which is how a Word document's break before a table
    /// survives an ODF round trip.
    /// </summary>
    /// <remarks>
    /// Each of the fixture's eight arms opens a page. Before <c>fo:break-before</c> was read on a
    /// <c>style:table-properties</c> this tree drew all eight on <b>one</b> page, and 26.2.4.2 draws
    /// eight; 12 of the 337 converted <c>.odt</c> state it, over 40 table styles.
    /// </remarks>
    [Fact]
    public void ATableStyleCanAskForAPageOfItsOwn() => Pages().Count.ShouldBe(8);

    /// <summary>The covered cell's rules, exactly as <see cref="TableCoveredCellRuleTests"/> asserts them.</summary>
    [Fact]
    public void ACoveredCellsTopRuleRaisesTheRowsDeclaredHeight()
    {
        RowHeight(FloorCovered).ShouldBe(23.50, 0.01);
        RowHeight(FloorBare).ShouldBe(22.50, 0.01);
    }

    /// <summary>And is charged to the band above the row, with nothing drawn for it.</summary>
    [Fact]
    public void TheBandAboveTheRowIsChargedForItAndNoRuleIsDrawn()
    {
        BaselineGap(BandCovered).ShouldBe(11.20, 0.01);
        BaselineGap(BandBare).ShouldBe(9.20, 0.01);

        Rules(BandCovered).Count.ShouldBe(1);
        Rules(BandBare).Count.ShouldBe(1);
    }

    /// <summary>Its stated bottom is charged below it, and that one is drawn.</summary>
    [Fact]
    public void TheBandBelowItIsChargedForItAndThatRuleIsDrawn()
    {
        BaselineGap(BelowCovered).ShouldBe(11.20, 0.01);
        BaselineGap(BelowBare).ShouldBe(9.20, 0.01);

        Rules(BelowCovered).Count.ShouldBe(3);
        Rules(BelowBare).Count.ShouldBe(2);
    }

    /// <summary>And the band below the table's last row.</summary>
    [Fact]
    public void TheBandBelowTheTablesLastRowIsChargedForItToo()
    {
        Rules(OuterCovered).Count.ShouldBe(2);
        Rules(OuterBare).Count.ShouldBe(1);

        BaselineGap(OuterCovered).ShouldBe(11.20, 0.01);
        BaselineGap(OuterBare).ShouldBe(9.20, 0.01);
    }

    private static double RowHeight(int arm)
    {
        List<double> rules = Rules(arm);
        rules.Count.ShouldBe(3);
        return rules[2] - rules[1];
    }

    private static double BaselineGap(int arm)
    {
        List<double> baselines =
            [.. DrawnWords.On(Pages()[arm]).Select(word => word.Baseline).Distinct().Order()];
        baselines.Count.ShouldBeGreaterThanOrEqualTo(2);
        return baselines[^1] - baselines[^2];
    }

    private static List<double> Rules(int arm) =>
        [.. Pages()[arm].StrokedPaths
            .Where(stroke => stroke.Bounds.Height <= Length.FromPoints(0.01))
            .Select(stroke => (stroke.Bounds.Y - (stroke.Stroke.Width / 2)).Points)
            .Distinct()
            .Order()];

    private static List<DrawnPage> Pages()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages];
    }
}
