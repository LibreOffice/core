using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The two clips a shape on a sheet is subject to: its own box, and the page's cell block.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Vertically, a body stating <c>vertOverflow="clip"</c> loses the lines that do not
/// fit.</strong> <c>SdrTextObj::impDecomposeBlockTextPrimitive</c> builds a clip range of the box's
/// height (<c>svx/source/svdraw/svdotextdecomposition.cxx:581-624</c>) and
/// <c>TextHierarchyBreakupBlockText</c> accepts "only text portions completely inside"
/// (<c>include/svx/svdoutl.hxx:56-59</c>), so the overflow is absent from the output rather than
/// merely invisible in it. Measured on <c>Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx</c>, whose
/// notes box is 1.37 inches tall and holds five paragraphs of caveats: 1556 words against
/// LibreOffice's 1504, and 1530 now.
/// </para>
/// <para>
/// <strong>Horizontally, what bounds it is the page's own cell block and not the paper.</strong>
/// <c>PrePrintDrawingLayer</c> builds a rectangle from the page's first printed column to its last
/// and hands that to <c>BeginDrawLayers</c> as the paint region
/// (<c>sc/source/ui/view/output3.cxx:41-95</c>). The rectangle is a <c>tools::Rectangle</c>, whose
/// edges are inclusive, so a shape whose left edge sits exactly on the block's right edge is still
/// drawn and one half an inch past it is not — both halves are in this fixture, and both were
/// checked against LibreOffice 24.2.7.2's own PDF for it. Measured on
/// <c>Part_375_Operators.xlsx</c>, whose two table slicers sit in the third of its three column
/// bands and fitted on the paper of the other two: 2251 words against 2197, and 2197 now.
/// </para>
/// </remarks>
public sealed class SheetShapeClipTests
{
    private static IReadOnlyList<DrawnPage> Draw(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages;
    }

    private static string TextOf(DrawnPage page)
        => string.Join(" ", page.Runs.Select(run => run.Text));

    [Fact]
    public void TextTallerThanItsBoxIsNotDrawn()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-shape-clip.xlsx");
        pages.Count.ShouldBe(2, "six wide columns take two pages side by side");

        string second = TextOf(pages[1]);

        // The box is half an inch tall and its text is ten lines long. Two lines fit.
        second.ShouldContain("ONTHEEDGE");
        second.ShouldContain("LOSTLINE2");
        second.ShouldNotContain("LOSTLINE3");
        second.ShouldNotContain("LOSTLINE11");
    }

    [Fact]
    public void AShapePastThePagesBlockIsNotDrawnOnIt()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-shape-clip.xlsx");
        string first = TextOf(pages[0]);

        // Both boxes are anchored in column D — the second band — and both fit on the first band's
        // paper. The one whose left edge is on the band's own right edge is drawn there; the one
        // half an inch past it is not.
        first.ShouldContain("ONTHEEDGE");
        first.ShouldNotContain("PASTTHEEDGE");
        TextOf(pages[1]).ShouldContain("PASTTHEEDGE");
    }
}
