using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An ODF header or footer band is <c>max(fo:min-height, textHeight + gap)</c>, its text is
/// measured in the face and size the band's own spans state, and a band stating
/// <c>svg:height</c> does not grow at all.
/// </summary>
/// <remarks>
/// <para>
/// Calc computes the band twice over and ODF states both of its terms directly:
/// <c>rParam.nHeight = nMaxHeight + rParam.nDistance</c> and then
/// <c>if (rParam.nHeight &lt; rParam.nManHeight) rParam.nHeight = rParam.nManHeight</c>
/// (<c>ScPrintFunc::UpdateHFHeight</c>, <c>sc/source/ui/view/printfun.cxx:838</c>,
/// <c>:848-849</c>). <c>fo:min-height</c> reaches <c>nManHeight</c> through
/// <c>ATTR_PAGE_SIZE</c> (<c>lcl_FillHFParam</c>, <c>:666</c> and <c>:683</c>) and the header's
/// <c>fo:margin-bottom</c> — the footer's <c>fo:margin-top</c> — reaches <c>nDistance</c>
/// through <c>ATTR_ULSPACE</c> (<c>:898</c>, <c>:913</c>).
/// </para>
/// <para>
/// <strong><c>OdsPrintSetup</c> read the declared height alone</strong>, which is right only
/// while <c>text + gap</c> stays under it. An <c>.ods</c> converted from a workbook carries the
/// workbook's header margin as the gap, so the pair is routinely a 21.26 pt band with a
/// 25.99 pt gap, and 58 of the converted corpus's 307 <c>.ods</c> declare a band smaller than
/// one line plus its gap.
/// </para>
/// <para>
/// Every figure below is read off LibreOffice 26.2.4.2's own rendering of the fixture, as the
/// shift of the first printed cell row against an otherwise identical page with no band —
/// <c>dotnet/probes/ods-band-r75/</c>, which establishes the same rule over 73 authored probes
/// with a worst error of 0.23 pt.
/// </para>
/// </remarks>
public sealed class SheetOdfBandHeightTests
{
    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require("sheet-ods-band-height.fods")));

        return ((SpreadsheetPages)document.Layout()).Sheets.Single(sheet => sheet.Name == name);
    }

    /// <summary>The band the reference prints, in points, and what decides it.</summary>
    /// <remarks>
    /// <c>Grows</c> and <c>Floor</c> answer the same number for different reasons, which is the
    /// pair that discriminates: the first is a plain 10 pt line and the second a 6 pt one, whose
    /// band is decided by the two empty areas beside it rather than by its own text.
    /// </remarks>
    [Theory]
    [InlineData("Grows", 39.487)]
    [InlineData("Declared", 56.693)]
    [InlineData("Fixed", 5.641)]
    [InlineData("Sized", 55.191)]
    [InlineData("Floor", 39.487)]
    public void AHeaderBandIsTheGreaterOfItsDeclaredHeightAndItsTextPlusTheGap(
        string name, double points)
        => Sheet(name).Setup.HeaderHeight.Points.ShouldBe(points, 0.25);

    [Fact]
    public void AFooterBandFollowsTheSameRuleWithItsOwnTopMarginAsTheGap()
    {
        SheetLayout sheet = Sheet("Footed");

        sheet.Setup.HeaderHeight.ShouldBe(Length.Zero);
        sheet.Setup.FooterHeight.Points.ShouldBe(39.487, 0.25);
        sheet.Setup.FooterGap.Points.ShouldBe(28.346, 0.05);
    }

    [Fact]
    public void ABandStatingSvgHeightIsNotDynamicAndDoesNotGrow()
    {
        // 0.2 cm declared against a 0.25 cm gap and an 11.14 pt line: the dynamic rule would
        // make this 18.23 pt. `svg:height` fills HeaderIsDynamicHeight with false where
        // `fo:min-height` fills it with true (XMLPageMasterPropSetMapper's finished,
        // xmloff/source/style/PageMasterImportPropMapper.cxx:324-330), that property is
        // ATTR_PAGE_DYNAMIC (sc/source/ui/unoobj/styleuno.cxx:341-343), and UpdateHFHeight
        // returns before it measures anything when the flag is off (printfun.cxx:793).
        Sheet("Fixed").Setup.HeaderHeight.Points.ShouldBe(5.669, 0.05);
    }

    [Fact]
    public void ABandsSpanKeepsTheSizeItStates()
    {
        // The reader used to drop a text:span's formatting outright — "a text:span carries
        // formatting and nothing else here" — which sized every ODF band in the workbook's
        // default font whatever the file said, and drew it in that font too.
        SheetHeaderFooter band = Sheet("Sized").Setup.Header.ShouldNotBeNull();

        band.Centre.Segments.ShouldAllBe(segment => segment.Size == Length.FromPoints(24));
    }

    [Fact]
    public void ABandNamingNoFaceTakesTheWorkbooksOwnDefaultCellFont()
    {
        // ScPrintFunc::MakeEditEngine fills the band's EditEngine defaults from
        // getDefaultCellAttribute (sc/source/ui/view/printfun.cxx:1765-1772), so a band stating
        // nothing is set in whatever a plain cell of that workbook is. The other three readers
        // have passed this since round 56; the ODF one passed nothing and inherited a fixed
        // ten-point Liberation Sans.
        SheetDefaultFont font = Sheet("Grows").Setup.BandFont.ShouldNotBeNull();

        // The name arrives as `svg:font-family` spells it, apostrophes and all, which is what
        // every ODF cell format already carries and what the font resolver already strips — the
        // Liberation Serif and Liberation Mono probes resolve to their own metrics through the
        // same quoting.
        font.Family.ShouldNotBeNull().Trim('\'').ShouldBe("Liberation Sans");
        font.Size.ShouldBe(Length.FromPoints(10));
    }
}
