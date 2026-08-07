using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests that a header or footer takes the band it prints in rather than the band its two
/// margins imply.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML states a footer band as <c>bottom - footer</c>, and Calc does not keep that
/// difference. Its filter measures the text crudely — a line is as tall as the largest bare
/// point size on it, with no ascent, descent or leading
/// (<c>HeaderFooterParser::getCurrHeight</c>, <c>sc/source/filter/oox/pagesettings.cxx:738-741</c>;
/// <c>XclImpHFConverter::GetMaxLineHeight</c>, <c>sc/source/filter/excel/xihelper.cxx:504-508</c>)
/// — stores <c>bodyDistance = statedBand - nominal</c>, and then throws the crude figure away at
/// print time: <c>ScPrintFunc::UpdateHFHeight</c> asks the EditEngine for the real height and adds
/// the stored distance back (<c>sc/source/ui/view/printfun.cxx:838-849</c>).
/// </para>
/// <para>
/// So the printed band is <c>statedBand + max(0, measured - nominal)</c>, and the difference comes
/// off the printable body on every page of the sheet.
/// </para>
/// <para>
/// <c>sheet-footer-band.xlsx</c> is built to that shape rather than copied from a corpus
/// workbook. Letter portrait; 0.75 in top and bottom margins against 0.3 in header and footer
/// margins, so the stated footer band is 0.45 in and the body the margins imply is
/// <c>792 - 54 - 21.6 - 32.4 = 684.0 pt</c>. Its footer is one line at 20 pt, which the filter
/// measures as 20 pt and the layout as 22.2 pt, so the band prints 2.2 pt taller and the body is
/// 681.8 pt. The 46 rows sum to 13658 twips — <strong>682.9 pt</strong>, which is inside
/// <c>(681.8, 684.0]</c>: it fits the band the margins imply and does not fit the band that
/// prints. LibreOffice 24.2.7.2 puts it on <strong>two</strong> pages.
/// </para>
/// <para>
/// Verified by reintroducing the defect rather than by watching this pass. The faithful wrong
/// implementation is the code this replaced — <c>headerBand = Length.FromInches(top - header)</c>
/// and <c>footerBand = Length.FromInches(bottom - footer)</c> in <c>XlsxPrintSetup</c>, the stated
/// band used as the printed one — and with it this document renders on a single page. That is a
/// stronger check than mutating <see cref="SheetBandHeight"/> itself, which any change to would
/// break: the point is that using the file's own arithmetic, correctly, is what is wrong.
/// </para>
/// </remarks>
public class SheetFooterBandTests
{
    private static IPaginatedDocument Open()
        => (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require("sheet-footer-band.xlsx")));

    [Fact]
    public void AFooterTakesMoreOfThePageThanItsMarginsImply()
    {
        using IPaginatedDocument document = Open();

        document.Layout().Count.ShouldBe(2);
    }

    [Fact]
    public void TheBandGrowsByWhatTheRealLineHeightAddsOverTheBarePointSize()
    {
        // 0.45 in stated, one 20 pt line: nominal 20 pt against a measured line height, so the
        // band is strictly taller than the margins imply and by less than a whole line.
        Length stated = Length.FromInches(0.45);
        Length printed = SheetBandHeight.Printed("&C&20Footer band", stated);

        printed.ShouldBeGreaterThan(stated);
        printed.ShouldBeLessThan(stated + Length.FromPoints(20));
    }

    [Fact]
    public void ABandWithNoTextIsLeftAlone()
    {
        Length stated = Length.FromInches(0.45);

        SheetBandHeight.Printed(null, stated).ShouldBe(stated);
        SheetBandHeight.Printed("", stated).ShouldBe(stated);
    }

    [Fact]
    public void ABandItsOwnTextOverflowsIsPinnedRatherThanGrown()
    {
        // #i23296, cited at both filters: a negative body distance means the text does not fit,
        // and Calc pins the band instead of making it dynamic. 40 pt of nominal text does not fit
        // a 0.45 in (32.4 pt) band, so the band stays exactly as stated.
        Length stated = Length.FromInches(0.45);

        SheetBandHeight.Printed("&C&40Too tall", stated).ShouldBe(stated);
    }

    [Fact]
    public void EachLineOfAMultiLineBandCounts()
    {
        Length stated = Length.FromInches(1.0);

        Length one = SheetBandHeight.Printed("&CFirst", stated);
        Length two = SheetBandHeight.Printed("&CFirst\nSecond", stated);

        two.ShouldBeGreaterThan(one);
    }
}
