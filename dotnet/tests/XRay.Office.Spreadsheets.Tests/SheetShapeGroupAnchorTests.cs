using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Where a BIFF shape's text lands: inside the group that holds the shape, and inside the
/// margin the shape asks the host for.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A shape inside a group states no client anchor.</strong> It carries an
/// <c>msofbtChildAnchor</c> — a rectangle in the group's own coordinate space — and only the
/// group's own shape says where that space lands on the sheet. A reader that asks every shape
/// for a client anchor therefore draws none of a group's members, which is what this tree did:
/// <c>ZGROUPA</c> and <c>ZGROUPB</c> are absent from the base rendering altogether.
/// </para>
/// <para>
/// <strong>A shape's text margin is two different rules.</strong> One that sets
/// <c>fAutoTextMargin</c> states no lengths and the host answers with a constant — Excel's is
/// 20000 EMU, 1.5748 pt, on all four sides. One that does not sets <c>dxTextLeft</c> and its
/// three siblings itself, and LibreOffice's own <c>MS Excel 97</c> filter writes all four as
/// zero.
/// </para>
/// <para>
/// <c>sheet-shape-group-anchor.xls</c> is <c>sheet-shape-group-anchor.fods</c> put through
/// LibreOffice 26.2.4.2's own <c>MS Excel 97</c> filter, so its Escher group, its child anchors
/// and its stated margins are the reference's own writing.
/// <c>sheet-shape-group-margin.xls</c> is that same file with two bits changed: property 191 is
/// the text boolean group and the filter never sets <c>fAutoTextMargin</c> in it, so the flag has
/// to be turned on by hand for any fixture to carry it. Nothing else differs and no length
/// anywhere changes — see <c>probes/sheet-shape-r103/patch-automargin.py</c>.
/// </para>
/// <para>
/// Every expected number is the baseline origin 26.2.4.2 writes into its own PDF of the same
/// file. The sheet's columns are 1 cm and 6 cm alternating and its rows 0.5 cm and 3 cm, so a
/// child mapped by interpolating column <em>indices</em> rather than lengths lands nowhere near
/// these.
/// </para>
/// </remarks>
public sealed class SheetShapeGroupAnchorTests
{
    private static Dictionary<string, DocPoint> Origins(string fixture)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        RecordingDrawingSink sink = new();
        sink.BeginPage(pages[0].Size);
        pages[0].Draw(sink);
        sink.EndPage();

        Dictionary<string, DocPoint> origins = [];
        foreach (DrawnGlyphRun run in sink.Pages[0].Runs)
        {
            string text = run.Text.Trim();
            if (text.Length > 0) origins.TryAdd(text, run.Origin);
        }

        return origins;
    }

    private static void ShouldBeAt(
        Dictionary<string, DocPoint> origins, string text, double x, double y)
    {
        origins.ShouldContainKey(text);
        origins[text].X.Points.ShouldBe(x, 0.4);
        origins[text].Y.Points.ShouldBe(y, 0.4);
    }

    [Fact]
    public void AGroupsMembersAreLaidInsideTheGroupsOwnRectangle()
    {
        Dictionary<string, DocPoint> origins = Origins("sheet-shape-group-anchor.xls");

        // Absent at the base: neither states a client anchor.
        ShouldBeAt(origins, "ZGROUPA", 84.95, 195.00);
        ShouldBeAt(origins, "ZGROUPB", 311.70, 251.77);

        // The controls: an ungrouped text box on the same sheet, whose four stated margins are
        // zero, and a cell, which never goes through the shape path at all.
        ShouldBeAt(origins, "ZLONE", 70.81, 132.58);
        ShouldBeAt(origins, "ZCELL", 58.68, 123.45);
    }

    [Fact]
    public void AShapeAskingTheHostForItsMarginGetsExcelsTwentyThousandEmu()
    {
        Dictionary<string, DocPoint> origins = Origins("sheet-shape-group-margin.xls");

        // 1.5748 pt right of and below where the same box sits without the flag.
        ShouldBeAt(origins, "ZLONE", 72.40, 134.16);

        // Only that one shape carries the flag, so the other three must not move.
        ShouldBeAt(origins, "ZGROUPA", 84.95, 195.00);
        ShouldBeAt(origins, "ZGROUPB", 311.70, 251.77);
        ShouldBeAt(origins, "ZCELL", 58.68, 123.45);
    }
}
