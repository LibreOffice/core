using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A worksheet shape whose own interior is a bitmap: <c>a:blipFill</c> inside
/// <c>xdr:sp/xdr:spPr</c>, which is a fill and not an <c>xdr:pic</c>.
/// </summary>
/// <remarks>
/// <para>
/// It was drawn with no interior at all. <c>XlsxDrawings</c> returned early for a shape because
/// its <c>picture</c> is the <c>xdr:pic</c> element and there is none, and <c>XlsxShapeInk</c>
/// resolves <c>a:solidFill</c>, <c>a:gradFill</c>, <c>a:grpFill</c> and the style matrix and has
/// no blip branch — its <c>StatesFill</c> guard names <c>blipFill</c> only so
/// as to suppress the theme fallback, so the shape ended with no fill whatever. That was a
/// deliberate choice while <c>Ink</c> carried colours alone: its remarks recorded that painting a
/// picture fill's first colour flat "would be a confident wrong answer rather than an absent one".
/// The answer is to draw the bitmap, not to approximate it.
/// </para>
/// <para>
/// <strong>Only the spreadsheet reader had the hole.</strong> <c>PptxSlideLayout</c> and
/// <c>DocxPictures</c> both resolve the same element, which is why this is routed through the
/// picture path below the shape rather than into a second image path of its own: the blip choice,
/// <c>a:alphaModFix</c> and <c>a:srcRect</c> are already done there.
/// </para>
/// <para>
/// <strong>Only <c>a:stretch</c>.</strong> A stretched bitmap fills the shape's rectangle, which
/// is exactly what the picture path draws, so the two are the same operation. <c>a:tile</c> is a
/// repeat at the blip's own size with its own offsets and is left unfilled as before — a tile
/// painted stretched would be a confident wrong answer where today there is an absent one. No
/// corpus shape tiles: the ten are all <c>prstGeom</c> <c>rect</c>, <c>a:stretch</c>, unrotated,
/// and nine of the ten also state <c>a:srcRect</c>.
/// </para>
/// <para>
/// Reach: <b>10 of 240 corpus <c>xlsx</c>, 10 shapes</b>, every one a
/// <c>Volunteer_Sign_Up_Sheet_Template_*</c> and every one previously drawing nothing in that
/// rectangle. Rendering the whole <c>xlsx</c> track twice under <c>SOURCE_DATE_EPOCH</c> moves
/// exactly those ten and leaves the other 230 byte-identical, and <b>no page count and no
/// alphanumeric count moves on any of them</b> — which is why no gate column could ever see this.
/// </para>
/// <para>
/// <c>features/sheet-shape-picture-fill.xlsx</c> is built by
/// <c>probes/sheetfill-r138/make-fixture.py</c> and is the corpus's shape reduced to one anchor.
/// LibreOffice <b>26.2.4.2</b>'s own PDF of it places the image once, at
/// <c>143.972 0 0 71.972 100.998 686.324 cm</c>; this tree places it at
/// <c>144 0 0 72 101.9622 685.2858</c> — the extent agreeing to <b>0.03 pt</b>, the origin to the
/// two writers' own constant. On the ten corpus documents the worst of the six placement numbers
/// is <b>0.11 pt</b> from the reference's.
/// </para>
/// </remarks>
public sealed class SheetShapePictureFillTests
{
    private const string Fixture = "sheet-shape-picture-fill.xlsx";

    private static SheetDrawings Drawings()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings;
    }

    [Fact]
    public void AShapeWhoseOwnFillIsAPictureCarriesThatPicture()
    {
        SheetDrawings drawings = Drawings();

        drawings.Items.Count.ShouldBe(1, "the fixture holds one anchor");

        SheetDrawing shape = drawings.Items[0];

        shape.Image.ShouldNotBeNull(
            "an a:blipFill in the shape's own spPr is the shape's interior, and it is drawn");
        shape.Image.EncodedBytes.Length.ShouldBeGreaterThan(
            0, "the blip resolved to the package's image part");
        shape.Image.EncodedBytes.Span[..4].ToArray()
            .ShouldBe([(byte)0x89, (byte)'P', (byte)'N', (byte)'G'], "and it is the PNG that part holds");
    }

    [Fact]
    public void ThePictureFillsTheShapesOwnRectangle()
    {
        SheetDrawing shape = Drawings().Items[0];

        // `editAs="oneCell"` keeps the shape's own `a:ext`, which is 1828800 x 914400 EMU — 144 by
        // 72 points. A stretched fill takes that rectangle whole, so the reference's
        // 143.972 x 71.972 is the shape's extent and not the blip's own 2 x 2 pixels.
        shape.Extent.Width.Points.ShouldBe(144, 0.5);
        shape.Extent.Height.Points.ShouldBe(72, 0.5);
    }
}
