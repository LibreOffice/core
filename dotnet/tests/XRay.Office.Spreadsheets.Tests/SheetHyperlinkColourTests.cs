using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A hyperlink cell's text is painted in the application's link colour, not the file's.
/// </summary>
/// <remarks>
/// <para>
/// The third consequence of a hyperlink cell being one <c>SvxURLField</c> rather than a styled
/// string — see <see cref="SheetHyperlinkFieldTests"/> for the two that move row heights. The
/// EditEngine paints a URL field in the configured <c>LINKS</c> colour, which is
/// <c>COL_BLUE</c> = <c>#000080</c> (<c>svtools/source/config/colorcfg.cxx:534</c>,
/// <c>include/tools/color.hxx:443</c>), so the character colour never reaches the page.
/// </para>
/// <para>
/// <strong>Unconditional, established by probe rather than assumed.</strong> A workbook holding a
/// hyperlink cell stated <c>#FF0000</c> and a second stated <c>#00B050</c>, each beside an
/// unlinked control in the same colour, comes out of the reference with both hyperlink cells
/// <c>#000080</c> and both controls untouched. Measured over a whole document:
/// <c>ans_mappings_of_eccairs_terms.xlsx</c>, whose <c>styles.xml</c> states
/// <c>&lt;color rgb="FF0000FF"/&gt;</c>, has 131 <c>#000080</c> text fills in the reference's PDF
/// and no <c>#0000FF</c> anywhere; we had 342 of <c>#0000FF</c> and none of <c>#000080</c>.
/// </para>
/// <para>
/// The fixture states no font colour at all, which makes it the sharper case rather than the
/// weaker one: black is what both cells would be painted without this, so the control's colour and
/// the file's stated colour cannot be confused for one another.
/// </para>
/// </remarks>
public sealed class SheetHyperlinkColourTests
{
    private static readonly Colour Link = Colour.FromRgb(0x000080);

    private static List<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-hyperlink-field.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return [.. sink.Pages[0].Runs];
    }

    /// <summary>The linked cell is navy and the unlinked one is not.</summary>
    /// <remarks>
    /// Both halves matter. Painting everything navy would satisfy the first assertion alone, and
    /// that is the shape a fix applied to the cell format rather than to the field would take.
    /// </remarks>
    [Fact]
    public void ALinkedCellIsPaintedInTheLinkColourAndAnUnlinkedOneIsNot()
    {
        List<DrawnGlyphRun> runs = Runs();

        runs.Where(run => run.Text.Contains("1206", StringComparison.Ordinal))
            .ShouldAllBe(run => run.Paint == Paint.Solid(Link));

        runs.Where(run => run.Text.Contains("1205", StringComparison.Ordinal))
            .ShouldAllBe(run => run.Paint != Paint.Solid(Link));
    }

    /// <summary>Nothing else on the page picks the colour up, and none of the cell misses it.</summary>
    /// <remarks>
    /// <para>
    /// <see cref="SheetLayout.HoldsField"/> covers one cell in this workbook, so the navy runs
    /// should be that cell's lines and nothing else. A predicate that widened to the whole
    /// hyperlink <em>range</em>, or to every underlined cell, would repaint more.
    /// </para>
    /// <para>
    /// Asserted as the concatenation rather than as a count, because the count is not a property of
    /// the colouring: a wrapping field is chopped into as many runs as it has lines
    /// (<see cref="SheetFieldChopTests"/>), and this used to read <c>ShouldBe(1)</c> only because a
    /// field was wrongly drawn on one line. The concatenation pins both halves — every line of the
    /// cell is navy, and nothing outside it is — without pinning the wrap.
    /// </para>
    /// </remarks>
    [Fact]
    public void ExactlyTheFieldCellIsRepainted()
    {
        string navy = string.Concat(
            Runs().Where(run => run.Paint == Paint.Solid(Link)).Select(run => run.Text));

        navy.ShouldBe("https://www.example.org/regulations/published/images/circular-1206.pdf");
    }
}
