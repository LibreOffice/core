using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a <c>vertOverflow="clip"</c> body is measured against, and what it is measured with.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="SheetShapeClipTests"/> pins the fact that the clip removes lines. This pins the two
/// quantities the decision is made from, because getting either wrong loses text the reference
/// draws or draws text it does not — and the corpus has both failures at once.
/// </para>
/// <para>
/// <strong>The rectangle is the preset's, not the anchor's box.</strong> A DrawingML preset may
/// carry an <c>a:rect</c> of four guide expressions, and <c>roundRect</c>'s insets it by
/// <c>x1 * 29289/100000</c> on every side, where <c>x1 = min(w, h) * adj/100000</c> — so a
/// stadium-shaped button at <c>adj="50000"</c> loses <c>0.1464 * min(w, h)</c> from each edge
/// before the text insets are taken. LibreOffice lays the text out in that rectangle
/// (<c>EnhancedCustomShape2d::GetTextRect</c>), and <c>aAnchorTextRange</c> — the height the clip
/// range is built from, <c>svx/source/svdraw/svdotextdecomposition.cxx:622-624</c> — is what is
/// left of it.
/// </para>
/// <para>
/// <strong>The test is on the portion's ink, not on its line box.</strong>
/// <c>TextHierarchyBreakupBlockText::processDrawPortionInfo</c> keeps a portion only when its
/// start position — the baseline — and both corners of
/// <c>TextLayouterDevice::getTextBoundRect</c> lie inside the clip range
/// (<c>svx/source/svdraw/svdoutl.cxx:120-160</c>), and its own header says why that is not
/// geometric clipping: "only text portions completely inside are to be accepted"
/// (<c>include/svx/svdoutl.hxx:56-59</c>). So a line whose box overflows still draws when its
/// letters do not reach the edge, and two labels of one size in one box get opposite answers when
/// one of them has a descender.
/// </para>
/// <para>
/// <strong>The fixture is four cells and changes one thing at a time.</strong> All four bodies are
/// one 20 pt Liberation Sans run in a 300 pt-wide shape stating <c>vertOverflow="clip"</c>; the
/// face is named rather than inherited so the arithmetic below is not at the mercy of what a
/// theme resolves to. Liberation Sans at 20 pt is an 18.11 pt ascent, a <b>22.34</b> pt line and a
/// 4.24 pt ink descender on <c>p</c> and <c>y</c>.
/// </para>
/// <para>
/// <strong>That line was 23.00 pt when this fixture was built and the four outcomes did not
/// move.</strong> Round 69 measured the shape line height off 26.2.4.2 and took the external
/// leading out of it — <see cref="SheetChartDeviceMetricTests"/> carries the measurement — which
/// is Liberation Sans' 67/2048, 0.65 pt at 20 pt. Every one of the four cases keeps its side of
/// its own threshold by more than that, which is the property the fixture was designed for and is
/// worth stating: the numbers below are recomputed, not the outcomes.
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>rect</c>, 27.5 pt tall, <c>CAPSFIT</c>. Room is 27.5 − 7.2 = 20.30 pt, so the 22.34 pt line
/// overflows and the clip engages; the ink reaches 18.11 pt and is drawn.
/// </description></item>
/// <item><description>
/// <c>rect</c>, 27.5 pt tall, <c>Capsfitpy</c>. The same box and the same line: the ink reaches
/// 22.35 pt and is not.
/// </description></item>
/// <item><description>
/// <c>roundRect adj="50000"</c>, 33 pt tall, <c>ROUNDLOST</c>. The box leaves 25.80 pt, which the
/// line fits inside, and the preset's rectangle leaves 16.13 pt, which the baseline alone
/// overruns.
/// </description></item>
/// <item><description>
/// <c>roundRect adj="50000"</c>, 50 pt tall, <c>ROUNDDRAWN</c>. The preset's rectangle leaves
/// 28.15 pt, so nothing is clipped — the rule is the rectangle and not the preset.
/// </description></item>
/// </list>
/// <para>
/// The expectations are not this painter's opinion. Both LibreOffice <b>26.2.4.2</b> and
/// <b>24.2.7.2</b> render the committed workbook to a PDF whose extracted text is exactly
/// <c>CAPSFIT</c> and <c>ROUNDDRAWN</c>, so the two shapes that are dropped are dropped by the
/// reference too and the version gap is not in this.
/// </para>
/// <para>
/// Measured on the corpus: <b>37 of 947</b> documents carry a sheet shape declaring the clip, and
/// they hold 592 of the 618 shape text bodies in the corpus's worksheet drawings.
/// <c>068_Blue_inventory_list…xlsx</c> is the first rule — its <c>homePlate</c> buttons put
/// 10 pt capitals in an 18 pt box, whose 11.64 pt line overflows the 10.80 pt of room while the
/// 9.28 pt of ink does not, and we drew none of them: 1141 alphanumerics against 1199, and 1199
/// now. <c>076_Inventory_list_accessibility_guide…xlsx</c> is the second — seven identical 204 x
/// 33 pt <c>roundRect</c> buttons of which the reference draws the two whose labels have no
/// descender: 4949 against 4782, and 4793 now.
/// </para>
/// </remarks>
public sealed class SheetShapePresetClipTests
{
    private const string Fixture = "sheet-shape-preset-clip.xlsx";

    private static string DrawnText()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(Fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return string.Join(" ", sink.Pages.SelectMany(page => page.Runs).Select(run => run.Text));
    }

    /// <summary>
    /// A line whose box overflows the room still draws when its own ink fits.
    /// </summary>
    /// <remarks>
    /// The failure this closes: measuring the line box instead of the ink dropped every capitalised
    /// button label in a box only a little shorter than its line, which is what
    /// <c>068_Blue_inventory_list…xlsx</c>'s three navigation buttons are.
    /// </remarks>
    [Fact]
    public void ALineWhoseInkFitsIsDrawnThoughItsBoxDoesNot()
    {
        DrawnText().ShouldContain("CAPSFIT");
    }

    /// <summary>
    /// The same box and the same line lose their text when a letter descends into the margin.
    /// </summary>
    /// <remarks>
    /// Asserted beside the positive because a painter that had simply stopped clipping would pass
    /// that one. Nothing differs between the two shapes but the string.
    /// </remarks>
    [Fact]
    public void ADescenderThatReachesPastTheRoomLosesTheWholePortion()
    {
        DrawnText().ShouldNotContain("Capsfitpy");
    }

    /// <summary>
    /// A preset's own text rectangle is what the text is measured against, not the anchor's box.
    /// </summary>
    /// <remarks>
    /// 33 pt of box leaves room for the line and 23.33 pt of <c>roundRect</c> text rectangle does
    /// not. This is the direction that draws text the reference does not, and it is the whole of
    /// <c>076_Inventory_list_accessibility_guide…xlsx</c>'s +167.
    /// </remarks>
    [Fact]
    public void ARoundedRectangleIsMeasuredInsideItsCornerRadius()
    {
        DrawnText().ShouldNotContain("ROUNDLOST");
    }

    /// <summary>
    /// A rounded rectangle with room to spare still draws, so the rule is the rectangle.
    /// </summary>
    /// <remarks>
    /// Same preset, same adjustment, same text size, 50 pt of box instead of 33: the preset's
    /// rectangle leaves 28.15 pt and the clip never engages.
    /// </remarks>
    [Fact]
    public void ARoundedRectangleTallEnoughForItsLineKeepsIt()
    {
        DrawnText().ShouldContain("ROUNDDRAWN");
    }
}
