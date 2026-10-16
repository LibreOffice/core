using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The BIFF half of a worksheet shape's fill and outline: Escher's own four properties.
/// </summary>
/// <remarks>
/// <para>
/// <c>features/sheet-shape-ink.xls</c> is LibreOffice 26.2.4.2's own conversion of
/// <c>sheet-shape-ink.xlsx</c>, so all three of this project's spreadsheet families answer for the
/// same five shapes. The <c>.xls</c> states them as Escher properties rather than as markup:
/// <c>fillColor</c> (385), <c>fFilled</c> (443), <c>lineColor</c> (448), <c>lineWidth</c> (459)
/// and <c>fLine</c> (508), none of which anything in this tree read — <c>git grep</c> found no use
/// of any of the five identifiers before round 84, although they had been named for years.
/// </para>
/// <para>
/// The converter has already resolved the theme, so 26.2.4.2's <c>.xls</c> draws the themed box in
/// the same <c>#4472C4</c> at the same 2.013 pt as its <c>.xlsx</c>: the interesting thing here is
/// not the resolution but that an <c>MSO_CLR</c> is <c>0x00BBGGRR</c> and reads inverted if it is
/// taken as an <c>0x00RRGGBB</c>.
/// </para>
/// <para>
/// <strong>Two things this path still does not do, and both are recorded rather than fixed.</strong>
/// Escher states a rotation in property 4 and nothing reads it, so the turned elbow is drawn
/// upright; and a group's leaves carry no <c>ftCmo</c> of their own, so the two ellipses reach
/// neither the model nor the page. Both are visible against the reference's own marks, and neither
/// is what this fixture is asserting.
/// </para>
/// </remarks>
public sealed class XlsShapeInkTests
{
    private const string Fixture = "sheet-shape-ink.xls";

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    /// <summary>Escher's fill and line colours are read, and the line's width with them.</summary>
    /// <remarks>
    /// The colours are the fixture's own <c>#FF0000</c> and <c>#0000FF</c>, which are also the
    /// pair that tells a right <c>MSO_CLR</c> decode from a byte-reversed one.
    /// </remarks>
    [Fact]
    public void AShapesEscherFillAndLineAreRead()
    {
        SheetDrawings drawings = Pages().Sheets[0].Drawings;

        SheetDrawing star = drawings.Items
            .FirstOrDefault(drawing => drawing.Fill == Colour.FromRgb(0xFF0000))
            .ShouldNotBeNull("the star's own red fill");

        star.Stroke.ShouldBe(Colour.FromRgb(0x0000FF));
        star.StrokeWidth.Points.ShouldBe(2.25, 0.02);
    }

    /// <summary>
    /// A shape carrying neither ink nor text nor a picture is dropped, and the page it would have
    /// kept is one page short of the reference. <strong>Pinned deliberately.</strong>
    /// </summary>
    /// <remarks>
    /// <para>
    /// 26.2.4.2 prints this file on <b>two</b> pages, the second empty, because the rightmost
    /// object on the sheet is a rectangle with no fill and no line and
    /// <c>ScDrawLayer::GetPrintArea</c> covers every object on the draw page
    /// (<c>sc/source/core/data/drwlayer.cxx</c>:1397-1414). The <c>.ods</c> twin of this fixture
    /// asserts the two pages and gets them; this one asserts <b>one</b>, and the difference is
    /// measured rather than chosen.
    /// </para>
    /// <para>
    /// Keeping every BIFF shape reproduces this page and <b>costs two gate verdicts</b> on the
    /// sheets track — <c>activespecs.xls</c> 267 pages against 266 and
    /// <c>orbus_togaf_tool_csq.xls</c> 74 against 75 — because BIFF has two guards before an
    /// object reaches the draw page and this reader has neither: <c>IsProcessSdrObj()</c> is
    /// <c>mbProcessSdr &amp;&amp; !mbHidden</c> (<c>sc/source/filter/inc/xiescher.hxx</c>:118), so
    /// a hidden BIFF object is dropped where a hidden DrawingML one is not; and
    /// <c>XclImpDrawObjBase::IsValidSize</c> (<c>sc/source/filter/excel/xiescher.cxx</c>:414-420)
    /// rejects an anchor under 3/100 mm by 1/100 mm, which <c>ProcessObj</c> applies at
    /// <c>:3658-3665</c> against *"invisible phantom objects from deleted rows or columns"*.
    /// Calc's <em>ODF</em> import has no equivalent, which is why the same change is right there
    /// and wrong here.
    /// </para>
    /// <para>
    /// So this assertion is the record of a known gap rather than of correct behaviour: it fails
    /// the moment either guard is implemented, which is the point.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnUninkedShapeIsDroppedAndCostsThePageTheReferenceKeeps()
        => Pages().Pages.Count.ShouldBe(1, "26.2.4.2 prints two; see the remarks");
}
