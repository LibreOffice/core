using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests that a BIFF header or footer band whose text does not fit prints at exactly the height
/// its margins state, with no minimum applied.
/// </summary>
/// <remarks>
/// <para>
/// The BIFF filter splits on whether the band's text fits the distance the two margins leave
/// (<c>XclImpPageSettings::Finalize</c>, <c>sc/source/filter/excel/xipage.cxx:315-331</c>). When
/// it fits, the band is dynamic and no <c>ATTR_PAGE_SIZE</c> is written, so
/// <c>UpdateHFHeight</c>'s <c>nManHeight</c> floor is the page style's own 425 twips. When it
/// does not fit — <c>#i23296</c> — the band is marked fixed and <c>ATTR_PAGE_SIZE</c> is written
/// at the margin distance; <c>UpdateHFHeight</c> then returns on its first line
/// (<c>if (!(rParam.bEnable &amp;&amp; rParam.bDynamic)) return;</c>,
/// <c>sc/source/ui/view/printfun.cxx:793</c>) and the floor is never reached.
/// </para>
/// <para>
/// <c>sheet-pinned-band-xls.xls</c> is authored to that shape rather than copied. Letter
/// portrait, a 0.5 in top margin and no header; a 0.35 in bottom margin against a 0.25 in
/// footer margin, so the stated footer band is <strong>0.1 in — 144 twips</strong>, and one
/// centred footer line in the workbook's own 10 pt Liberation Sans, whose nominal height of
/// 200 twips does not fit it. The face is stated explicitly because a fixture naming Arial
/// resolves to the unwired default and would pass whatever the code did.
/// </para>
/// <para>
/// Its 34 rows are 0.75 cm each, 14450 twips in total. The body is
/// <c>15839 - 360 - 720 + 1 = 14760</c> twips less the footer band: <strong>14616</strong> at
/// the pinned 144, and <strong>14335</strong> at the floored 425. 14450 is inside the first and
/// past the second, so the sheet is one page if the floor is withheld and two if it is applied.
/// LibreOffice 24.2.7.2 renders it on <strong>one</strong>.
/// </para>
/// <para>
/// The corpus instance is <c>RMP 2011-2014 and Inventory.xls</c>, whose two sheets between them
/// exercise both branches and both bands: LibreOffice's flat-ODF export gives its first sheet a
/// dynamic header floored at 425 twips and a pinned footer of 176, and its second a pinned
/// header of 380 and a pinned footer of 113.
/// </para>
/// </remarks>
public class SheetPinnedBandTests
{
    private static IPaginatedDocument Open()
        => (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require("sheet-pinned-band-xls.xls")));

    [Fact]
    public void APinnedFooterDoesNotTakeTheDefaultBandFromTheBody()
    {
        using IPaginatedDocument document = Open();

        document.Layout().Count.ShouldBe(1);
    }

    [Fact]
    public void ThePinnedFooterKeepsTheHeightItsMarginsState()
    {
        using IPaginatedDocument document = Open();
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        SheetPrintSetup setup = pages.Sheets[0].Setup;

        // 0.1 in, not the 0.75 cm a dynamic band would be floored at.
        setup.FooterHeight.Twips.ShouldBe(144);
        setup.HeaderHeight.ShouldBe(Length.Zero);
    }

    [Fact]
    public void ABandThatFitsIsDynamicAndOneThatDoesNotIsNot()
    {
        // Ten points of text in a quarter inch of band: it fits, so Calc leaves it dynamic and
        // the page style's minimum still applies to it.
        SheetBandHeight.Printed(
            "&CPinned footer", Length.FromInches(0.25), null, out bool roomy);
        roomy.ShouldBeTrue();

        // The same line in a tenth of an inch does not fit, which is what pins the band.
        SheetBandHeight.Printed(
            "&CPinned footer", Length.FromInches(0.1), null, out bool cramped);
        cramped.ShouldBeFalse();
    }

    /// <summary>
    /// A band pinned at no height at all still draws, starting at the margin it has no room
    /// above.
    /// </summary>
    /// <remarks>
    /// <c>sheet-zero-band-xls.xls</c> is the same document with the footer band taken to nothing:
    /// a 0.25 in bottom margin against a 0.25 in footer margin, so the two are equal and Calc
    /// pins the band at zero. `PrintHF` clips the text to
    /// <c>tools::Rectangle(aStart, aPaperSize)</c>, and a VCL rectangle built from a zero-height
    /// <c>Size</c> is unbounded rather than empty, so the text is drawn and runs down into the
    /// margin. LibreOffice 24.2.7.2 puts the top of `Zero band footer` at <strong>773.95 pt</strong>
    /// on a 792 pt page, which is the bottom margin line; before this round we drew it 12.6 pt
    /// higher, inside a band we had floored at 425 twips, and paginated the sheet on two pages.
    /// </remarks>
    [Fact]
    public void AFooterPinnedAtNothingIsStillDrawnAtTheMargin()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(
                DocumentSource.FromFile(Corpus.Require("sheet-zero-band-xls.xls")));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        pages.Count.ShouldBe(1);

        SheetPrintSetup setup = pages.Sheets[0].Setup;
        setup.FooterHeight.ShouldBe(Length.Zero);
        setup.FooterGap.ShouldBe(Length.Zero);

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        List<DrawnWord> footer =
            [.. DrawnWords.On(sink.Pages[0]).Where(word => word.Text.StartsWith("Zero", StringComparison.Ordinal))];

        footer.Count.ShouldBe(1);

        // The baseline is the pen, so the top of the line is a shade above it; the reference's
        // own box starts at 773.95 on a 792 pt page whose bottom margin is 18.
        footer[0].Baseline.ShouldBeInRange(774.0, 786.0);
    }

    [Fact]
    public void ABandTheMarginsLeaveNoRoomForIsPinnedAtNothing()
    {
        // Both `RMP`'s second sheet and three other corpus workbooks state a footer margin equal
        // to the page margin. `GetTwipsFromInch(0)` is what Calc pins such a band at, and the
        // 425-twip minimum must not rescue it.
        SheetBandHeight.Printed("&CFooter", Length.Zero, null, out bool dynamic)
            .ShouldBe(Length.Zero);
        dynamic.ShouldBeFalse();
    }
}
