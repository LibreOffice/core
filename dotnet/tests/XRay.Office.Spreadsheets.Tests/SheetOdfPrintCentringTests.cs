using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// <c>style:table-centering</c>: whether the ODF reader reads the flag at all.
/// </summary>
/// <remarks>
/// <see cref="SheetPrintCentringTests"/> beside this asserts what centring <em>does</em> — that Calc
/// halves the remainder without looking at its sign, so a block wider than the page hangs off both
/// edges — and it does so through the OOXML reader, which spells the attribute correctly. This one
/// asserts that the ODF reader delivers the flag to it at all, which for the life of the feature it
/// did not.
/// </remarks>
/// <remarks>
/// <para>
/// <strong>The reader asked for <c>style:table-centring</c> for as long as it had the feature.</strong>
/// This file's convention is British wherever it names something of its own — <c>centring</c>,
/// <see cref="SheetPrintSetup.CentresHorizontally"/>, <c>SheetHorizontalAlignment.Centre</c> — and the
/// convention leaked one word too far, onto an ODF attribute name, which is the specification's to
/// spell. The British form matches nothing in any real file, so the two properties were never once
/// true and <c>SpreadsheetPages.BodyOrigin</c>, which has implemented the rule correctly throughout,
/// was never asked. That was the whole of seat O100.
/// </para>
/// <para>
/// [src] <c>xmloff/source/core/xmltoken.cxx</c>:1992 interns the token and
/// <c>PageMasterStyleMap.cxx</c>:97-98 maps the <em>one</em> attribute onto <em>both</em>
/// <c>PROP_CenterHorizontally</c> and <c>PROP_CenterVertically</c> with
/// <c>MID_FLAG_MERGE_ATTRIBUTE</c>; <c>ScPrintFunc::PrintPage</c>
/// (<c>sc/source/ui/view/printfun.cxx</c>:2144-2191) then adds half the slack to the left and top
/// space, <strong>unclamped</strong> — so a block wider than the paper hangs off both edges rather
/// than being left-aligned, which is the behaviour round 69 measured on the OOXML side.
/// </para>
/// <para>
/// [bin] Measured on <c>sheet-print-centring.fods</c>, whose four sheets differ in that one attribute
/// and nothing else. One 4 cm column on a 21 cm page with 2 cm margins leaves 13 cm of slack, so the
/// shift is 6.5 cm — <b>184.252 pt</b>, predicted with no free parameter. LibreOffice 26.2.4.2 draws
/// the arms at <c>(57.685, 66.332)</c>, <c>(241.937, 66.332)</c>, <c>(57.685, 424.177)</c> and
/// <c>(241.937, 424.177)</c>, and this tree agrees to <b>0.000 pt across and 0.001 pt down</b>.
/// Those <em>y</em> are baselines, read from the reference's own <c>Td</c> and turned the right way
/// up; the ink tops <c>pymupdf</c> reports are 9.056 pt above them, and comparing a baseline against
/// an ink box is a mistake <c>dotnet/CLAUDE.md</c> records a round nearly filing a font bug over.
/// <c>probes/odscentre-r150/</c>.
/// </para>
/// </remarks>
public sealed class SheetOdfPrintCentringTests
{
    /// <summary>The shift is half the slack: 13 cm of it on a 4 cm block in a 17 cm printable width.</summary>
    private const double SlackPoints = 184.252;

    /// <summary>An absent attribute leaves the block where the margins put it.</summary>
    /// <remarks>
    /// The control, and the reason the defect survived: every sheet in the corpus looked like this
    /// one, because the reader could not tell the other three from it.
    /// </remarks>
    [Fact]
    public void AnAbsentAttributeCentresNothing()
    {
        (double x, double y) = Block("none");

        x.ShouldBe(57.685, 0.05);
        y.ShouldBe(66.332, 0.05);
    }

    /// <summary>Each value moves the axis it names, and only that axis.</summary>
    [Theory]
    [InlineData("horizontal", true, false)]
    [InlineData("vertical", false, true)]
    [InlineData("both", true, true)]
    public void EachValueMovesTheAxisItNames(string arm, bool across, bool down)
    {
        (double bareX, double bareY) = Block("none");
        (double x, double y) = Block(arm);

        (x - bareX).ShouldBe(across ? SlackPoints : 0.0, 0.05);
        (y - bareY).ShouldBe(down ? 357.845 : 0.0, 0.05);
    }

    /// <summary>The attribute is read under the name ODF gives it, not under ours.</summary>
    /// <remarks>
    /// Asserted on the setup rather than on the ink so that a later tidy-up of the spelling fails
    /// here, where the reason is written down, instead of only in the two geometric tests above.
    /// </remarks>
    [Fact]
    public void TheAttributeIsReadUnderTheSpecificationsSpelling()
    {
        List<SheetPrintSetup> setups = Setups();

        setups[0].CentresHorizontally.ShouldBeFalse();
        setups[0].CentresVertically.ShouldBeFalse();

        setups[1].CentresHorizontally.ShouldBeTrue("style:table-centering=\"horizontal\"");
        setups[1].CentresVertically.ShouldBeFalse();

        setups[2].CentresHorizontally.ShouldBeFalse();
        setups[2].CentresVertically.ShouldBeTrue("style:table-centering=\"vertical\"");

        setups[3].CentresHorizontally.ShouldBeTrue("style:table-centering=\"both\"");
        setups[3].CentresVertically.ShouldBeTrue("style:table-centering=\"both\"");
    }

    private static readonly string[] Arms = ["none", "horizontal", "vertical", "both"];

    /// <summary>Where the named arm's one word is drawn, in points from the page's top left.</summary>
    private static (double X, double Y) Block(string arm)
    {
        RecordingDrawingSink sink = new();

        using (IPaginatedDocument document =
               (IPaginatedDocument)OfficeDocument.Open(Corpus.Require("sheet-print-centring.fods")))
        {
            SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
            pages.Pages.Count.ShouldBe(Arms.Length, "one sheet per arm, one page each");
            pages.Pages[Array.IndexOf(Arms, arm)].Draw(sink);
        }

        DrawnGlyphRun run = sink.Pages[0].Runs.ShouldHaveSingleItem();
        return (run.Origin.X.Points, run.Origin.Y.Points);
    }

    private static List<SheetPrintSetup> Setups()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require("sheet-print-centring.fods"));

        return [.. ((SpreadsheetPages)document.Layout()).Sheets.Select(sheet => sheet.Setup)];
    }
}
