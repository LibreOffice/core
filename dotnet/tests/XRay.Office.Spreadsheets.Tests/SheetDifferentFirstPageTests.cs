using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// <c>headerFooter/@differentFirst</c> gives the first page its own header and footer — including
/// when that means none at all.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured on LibreOffice 26.2.4.2.</strong> Found as a cluster: six workbooks in
/// <c>sheets/chartset-005…014</c> were failing the gate by exactly four words, all one page, and
/// the four were always the same — <c>Page</c>, <c>1</c>, <c>of</c>, <c>1</c>. We drew a
/// <c>Page &amp;P of &amp;N</c> footer that the reference did not, because
/// <c>differentFirst="1"</c> was declared and no <c>firstFooter</c> supplied, so the only page is
/// a first page and prints bare. <c>067_Basic_invoice_Use_this_template</c> went from 79 words
/// against 75 to 75 against 75.
/// </para>
/// <para>
/// <strong>All 49 corpus workbooks that set the flag supply no first-page content</strong>, which
/// is why reading the flag matters even though none of them has a <c>firstHeader</c> to draw.
/// Calc keeps the same distinction: <c>mbShareFirst = !bUseFirstContent</c>
/// (<c>sc/source/filter/oox/pagesettings.cxx:1019</c>) is set from the flag, not from whether any
/// first-page string was written.
/// </para>
/// <para>
/// <strong>It is the printout's first page, not each sheet's</strong>, and getting that wrong is
/// what the first attempt did. Calc decides with <c>bFirst = 0 == nPageNo</c>
/// (<c>sc/source/ui/view/printfun.cxx:1796</c>), and that <c>nPageNo</c> counts across the whole
/// job — the per-table value is <c>aTableParam.nFirstPageNo</c>, added to it at <c>:1828</c> to
/// make the number that gets printed. Measured on
/// <c>042_Business_monthly_budget_4e4d092f.xlsx</c>: four sheets of one page each, all four
/// declaring <c>differentFirst</c>, and the reference draws footers on pages 2, 3 and 4 while
/// leaving page 1 bare — even though every one of those is its own sheet's first page.
/// Suppressing per sheet cost that workbook 16 words instead of 4 and broke it.
/// </para>
/// <para>
/// <strong>The band is not resized, and that is why this moves no page boundary.</strong> Calc
/// takes one height for the sheet — <c>max(nOddHeight, nEvenHeight, nFirstHeight)</c> at
/// <c>pagesettings.cxx:1026</c> — and <c>mbHasContent</c> is the OR of the three. The space is
/// reserved on every page and only the ink differs.
/// </para>
/// </remarks>
public sealed class SheetDifferentFirstPageTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    private static SheetPrintSetup Setup(string headerFooter)
    {
        XElement worksheet = XElement.Parse(
            $"<worksheet xmlns=\"{Ns}\"><pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" "
            + $"bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>{headerFooter}</worksheet>");

        return XlsxPrintSetup.Read(worksheet, [], null, null).Setup;
    }

    /// <summary>The control: no flag, so the odd footer is the footer and there is no other.</summary>
    [Fact]
    public void WithoutTheFlagThereIsNoSeparateFirstPage()
    {
        SheetPrintSetup setup = Setup("<headerFooter><oddFooter>Page &amp;P of &amp;N</oddFooter></headerFooter>");

        setup.DifferentFirstPage.ShouldBeFalse();
        setup.Footer.ShouldNotBeNull();
        setup.FirstFooter.ShouldBeNull();
    }

    /// <summary>
    /// The corpus's actual shape: the flag is set and no first-page content is supplied, so the
    /// first page has a footer of nothing while later pages keep the odd one.
    /// </summary>
    [Fact]
    public void TheFlagWithoutFirstPageContentLeavesTheFirstPageBare()
    {
        SheetPrintSetup setup = Setup(
            "<headerFooter differentFirst=\"1\"><oddFooter>Page &amp;P of &amp;N</oddFooter></headerFooter>");

        setup.DifferentFirstPage.ShouldBeTrue();
        setup.Footer.ShouldNotBeNull("later pages still draw the odd footer");
        setup.FirstFooter.ShouldBeNull("and the first page draws nothing");
    }

    /// <summary>When first-page content IS supplied, it is read and kept apart from the odd pair.</summary>
    [Fact]
    public void FirstPageContentIsReadWhenItIsSupplied()
    {
        SheetPrintSetup setup = Setup(
            "<headerFooter differentFirst=\"1\"><oddFooter>Page &amp;P</oddFooter>"
            + "<firstFooter>Cover</firstFooter></headerFooter>");

        setup.FirstFooter.ShouldNotBeNull();
        setup.FirstFooter!.IsEmpty.ShouldBeFalse();
        setup.Footer.ShouldNotBeNull();
    }

    /// <summary>
    /// First-page content is ignored entirely when the flag is not set, which is what makes the
    /// flag rather than the element the thing that decides.
    /// </summary>
    [Fact]
    public void FirstPageContentWithoutTheFlagIsNotUsed()
    {
        SheetPrintSetup setup = Setup(
            "<headerFooter><oddFooter>Page &amp;P</oddFooter>"
            + "<firstFooter>Cover</firstFooter></headerFooter>");

        setup.DifferentFirstPage.ShouldBeFalse();
        setup.FirstFooter.ShouldBeNull();
    }

    /// <summary>
    /// A sheet whose ONLY header content is on its first page still reserves a band, because
    /// Calc's `mbHasContent` is the OR of the three variants.
    /// </summary>
    [Fact]
    public void AFirstPageOnlyHeaderStillReservesItsBand()
    {
        SheetPrintSetup setup = Setup(
            "<headerFooter differentFirst=\"1\"><firstHeader>Cover</firstHeader></headerFooter>");

        setup.HeaderHeight.ShouldBeGreaterThan(Core.Units.Length.Zero);
        setup.FirstHeader.ShouldNotBeNull();
        setup.Header.ShouldBeNull();
    }
}
