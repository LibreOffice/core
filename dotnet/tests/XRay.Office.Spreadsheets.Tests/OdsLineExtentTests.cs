using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Three ODF drawing elements state no rectangle at all: their geometry is a pair of points.
/// </summary>
/// <remarks>
/// <para>
/// <c>draw:line</c>, <c>draw:connector</c> and <c>draw:measure</c> state <c>svg:x1</c>,
/// <c>svg:y1</c>, <c>svg:x2</c> and <c>svg:y2</c> and neither <c>svg:width</c> nor <c>svg:x</c>.
/// <c>SdXMLLineShapeContext::startFastElement</c> (<c>xmloff/source/draw/ximpshap.cxx</c>:1045-1097)
/// takes the smaller of each coordinate as the shape's position, puts the pair in the polygon and
/// leaves the stated size at 1 × 1, so the object's bounding rectangle is the polygon's;
/// <c>SdXMLConnectorShapeContext</c> (<c>:1963-2035</c>) sets <c>StartPosition</c> and
/// <c>EndPosition</c> from the same four attributes. A reader that looks only for
/// <c>svg:width</c> gives every one of them a zero-sized box — and a zero-sized box widens no
/// print area, because a drawing reaches the printed block through
/// <c>ScDrawLayer::GetPrintArea</c> (<c>sc/source/core/data/drwlayer.cxx</c>:1397-1414) maxed into
/// <c>ScDocument::GetPrintArea</c> (<c>documen2.cxx</c>:644-664). See
/// <see cref="SheetDrawingArea"/>.
/// </para>
/// <para>
/// <strong>A connector's <c>table:end-cell-address</c> is the cell it is anchored in, not the far
/// corner of the line</strong>, so taking the end cell instead of the endpoints collapses the
/// object to a point. The fixture states both, exactly as Calc writes them.
/// </para>
/// <para>
/// The fixture's three rows of cells are 0.15 in each and its line runs from 0.5 in to 14 in below
/// A1 on a 11 in page, so the line alone decides the second page. Measured: 26.2.4.2 prints
/// <strong>two</strong> pages of it, the second carrying the line's tail and no text, and
/// <strong>one</strong> with the <c>draw:connector</c> element deleted. It is the shape of
/// <c>017_Timeline_Templates_for_Excel_b88faee6.ods</c>, whose <c>Straight Connector 2</c> is a
/// timeline's spine anchored in A1 and running to <c>svg:y2="15.7957in"</c> over 66 rows of cells
/// that add up to 12.93 in: this tree printed 2 pages of that workbook against the reference's 3
/// and prints 3 now. Reach over the 307 converted <c>.ods</c> is <strong>3 documents and 72
/// elements</strong>. <c>probes/ods-residue-r95/results.md</c> §3.
/// </para>
/// </remarks>
public sealed class OdsLineExtentTests
{
    private const string Fixture = "sheet-line-extent.fods";

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    private static SheetDrawing Connector()
        => Pages().Sheets[0].Drawings.Items
                  .SingleOrDefault(drawing => drawing.Name == "Spine")
                  .ShouldNotBeNull("a drawing named Spine");

    /// <summary>The connector's rectangle is the box its two endpoints bound.</summary>
    /// <remarks>
    /// 13.5 inches tall and nothing wide, which is what <c>svg:y1="0.5in"</c> and
    /// <c>svg:y2="14in"</c> at one <c>svg:x</c> describe. The height is the assertion that matters;
    /// a reader taking <c>svg:width</c> answers zero for both.
    /// </remarks>
    [Fact]
    public void AConnectorsRectangleIsTheBoxItsTwoEndpointsBound()
    {
        SheetDrawing spine = Connector();

        spine.Extent.Height.Twips.ShouldBe(19440);   // 13.5 in
        spine.Extent.Width.ShouldBe(Length.Zero);
    }

    /// <summary>And its own anchor is the upper of the two points, not the anchor cell's corner.</summary>
    [Fact]
    public void ItsOriginIsTheSmallerOfEachCoordinate()
        => Connector().Position.Y.Twips.ShouldBe(720);   // 0.5 in

    /// <summary>
    /// The end cell must not be taken, because Calc writes the anchor cell there for a connector.
    /// </summary>
    /// <remarks>
    /// The fixture states <c>table:end-cell-address="Probe.A1"</c> with a two-hundredth of an inch
    /// of offset. Read as a two-cell anchor it would make the line a point at the top-left of A1,
    /// which is how this tree lost the page.
    /// </remarks>
    [Fact]
    public void ACachedEndCellOnAConnectorIsNotItsFarCorner()
        => Connector().Anchor.ShouldBe(SheetAnchorKind.OneCell);

    /// <summary>The line reaches a second page, and that page is what the reference prints.</summary>
    [Fact]
    public void ALineReachingPastTheLastCellKeepsItsPage()
        => Pages().Pages.Count.ShouldBe(2);
}
