using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cropped picture on a sheet: the four Escher properties, the larger rectangle, and the clip
/// that makes it read as a crop rather than as a picture spilled across the grid.
/// </summary>
/// <remarks>
/// <para>
/// <b>The crop cannot be applied where it is read, and that is the whole shape of this.</b> A
/// sheet's drawing is anchored to cells, so its rectangle does not exist until the page's column
/// widths and row heights are resolved — <c>XlsDrawing</c> reads the fractions,
/// <see cref="SheetDrawing.Crop"/> carries them, and <c>SheetPageGraphics</c> grows the rectangle
/// and takes the clip.
/// </para>
/// <para>
/// <b>The clip is new here.</b> Nothing on this path clipped a picture before: <c>DrawImage</c>
/// was called with the anchor's box and no <c>Save</c>/<c>ClipPath</c>/<c>Restore</c> around it.
/// A larger destination without that clip does not crop a picture, it draws the whole of it over
/// the cells on every side — so the two halves are one change and are tested together.
/// </para>
/// <para>
/// <b>The fixture is round-tripped through LibreOffice on purpose.</b> <c>picture-crop.xlsx</c>
/// states <c>a:srcRect l="10000" t="20000" r="30000" b="40000"</c> on a picture 288 × 216 pt;
/// <c>picture-crop.xls</c> is that file through <c>soffice --convert-to xls</c>, which is what
/// turns the <c>a:srcRect</c> into Escher's 256–259. The image is 100 × 100 pixels and the size
/// is load-bearing: BIFF export states the crop in the <em>bitmap's</em> pixels, so an 8-pixel
/// image brings a 10% crop back as 0.0990.
/// </para>
/// <para>
/// <b>Both halves of the pair are cropped, and each is the other's check.</b> The SpreadsheetML
/// reader dropped <c>a:srcRect</c> when this file was written and the test below pinned it in
/// that state; <c>XlsxDrawings</c> now reads it into the same <see cref="SheetDrawing.Crop"/> the
/// Escher path fills. LibreOffice reaches one answer from both spellings too — <c>oox</c> turns
/// <c>a:srcRect</c> into a <c>text::GraphicCrop</c> against the graphic's original size
/// (<c>fillproperties.cxx</c>:844-873) — so what the pair is for is the two paths agreeing.
/// </para>
/// </remarks>
public sealed class SheetPictureCropTests
{
    /// <summary>The picture's own frame: 288 × 216 pt, stated by both halves of the pair.</summary>
    private const double FrameWidthPoints = 288;
    private const double FrameHeightPoints = 216;

    // ------------------------------------------------------------------ the property read

    [Fact]
    public void TheFourEscherPropertiesReachTheModel()
    {
        SheetDrawing drawing = Only("picture-crop.xls");

        drawing.Crop.IsNone.ShouldBeFalse("a crop was read");
        drawing.Crop.Left.ShouldBe(0.10, 0.001);
        drawing.Crop.Top.ShouldBe(0.20, 0.001);
        drawing.Crop.Right.ShouldBe(0.30, 0.001);
        drawing.Crop.Bottom.ShouldBe(0.40, 0.001);
    }

    /// <summary>
    /// The SpreadsheetML half states the same crop as <c>a:srcRect</c>, and reads it.
    /// </summary>
    /// <remarks>
    /// Exact where the BIFF half is not: <c>a:srcRect</c> is stated in thousandths of a percent
    /// of the source, where BIFF export restates the crop in the bitmap's own pixels and brings
    /// it back a little off.
    /// </remarks>
    [Fact]
    public void TheSrcRectReachesTheModelOnTheXlsxPath()
    {
        SheetDrawing drawing = Only("picture-crop.xlsx");

        drawing.Crop.IsNone.ShouldBeFalse("a crop was read");
        drawing.Crop.Left.ShouldBe(0.10, 0.0001);
        drawing.Crop.Top.ShouldBe(0.20, 0.0001);
        drawing.Crop.Right.ShouldBe(0.30, 0.0001);
        drawing.Crop.Bottom.ShouldBe(0.40, 0.0001);
    }

    // ------------------------------------------------------------------ the rectangle drawn

    [Fact]
    public void ACroppedPictureIsDrawnLargerThanItsAnchor()
    {
        DocRect destination = OnlyImageOf("picture-crop.xls");

        // 288 / (1 - 0.1 - 0.3) = 480 pt across and 216 / (1 - 0.2 - 0.4) = 540 pt down. The
        // figures asserted are LibreOffice 26.2.4.2's own: it draws the whole picture at
        // 479.565 x 539.405 pt, its distance from the nominal 480 x 540 being the row heights the
        // one-cell anchor's extent is resolved through on both sides.
        destination.Width.Points.ShouldBe(479.57, 1.0);
        destination.Height.Points.ShouldBe(539.41, 1.0);
    }

    /// <summary>
    /// The visible part lands at the anchor: the whole picture starts 10% of its own width to the
    /// left of it and 20% of its own height above it, in both formats.
    /// </summary>
    /// <remarks>
    /// This is what distinguishes a crop from a resize — the anchor's box is where the surviving
    /// part goes, and the picture grows outside it — so it is asserted against the anchor's own
    /// 288 × 216 pt rather than against the other fixture.
    /// </remarks>
    [Theory]
    [InlineData("picture-crop.xls")]
    [InlineData("picture-crop.xlsx")]
    public void TheVisiblePartOfThePictureLandsAtTheAnchor(string name)
    {
        DocRect destination = OnlyImageOf(name);

        (0.6 * destination.Width.Points).ShouldBe(FrameWidthPoints, 1.0, name);
        (0.4 * destination.Height.Points).ShouldBe(FrameHeightPoints, 1.0, name);
    }

    /// <summary>
    /// The two formats crop to the same rectangle, which is the pair's reason for existing.
    /// </summary>
    /// <remarks>
    /// A point and a half of tolerance rather than a fiftieth, and the two halves are genuinely
    /// that far apart: the OOXML crop is exact where the BIFF one is the bitmap's pixels rounded
    /// — 479.57 × 539.41 pt against a nominal 480 × 540, which is LibreOffice's own figure for
    /// the same file and is asserted as such above. Tightening this would be asserting that a
    /// round trip through <c>.xls</c> is lossless, which it is not.
    /// </remarks>
    [Fact]
    public void TheTwoFormatsCropToTheSameRectangle()
    {
        DocRect biff = OnlyImageOf("picture-crop.xls");
        DocRect ooxml = OnlyImageOf("picture-crop.xlsx");

        ooxml.X.Points.ShouldBe(biff.X.Points, 1.5);
        ooxml.Y.Points.ShouldBe(biff.Y.Points, 1.5);
        ooxml.Width.Points.ShouldBe(biff.Width.Points, 1.5);
        ooxml.Height.Points.ShouldBe(biff.Height.Points, 1.5);
    }

    // ------------------------------------------------------------------ the clip

    /// <summary>
    /// The clip is what turns a larger rectangle into a crop. Without it this change would be
    /// strictly worse than doing nothing: the picture would be drawn 480 × 540 pt over whatever
    /// the sheet has on all four sides of it.
    /// </summary>
    [Theory]
    [InlineData("picture-crop.xls")]
    [InlineData("picture-crop.xlsx")]
    public void ACroppedPictureIsClipped(string name)
        => ClipsOf(name).ShouldBeGreaterThan(0);

    /// <summary>
    /// And an uncropped one takes no clip at all when its picture stays inside the printed block.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>picture-crop.xlsx</c> was this control by not reading its own <c>a:srcRect</c>;
    /// <c>picture-watermark.xlsx</c> is one on purpose — a picture over cells, stating an
    /// <c>a:alphaModFix</c> and no crop.
    /// </para>
    /// <para>
    /// <strong>Two rounds wrote the drawing-layer clip independently and disagreed about this
    /// assertion; it is back at zero, and the reason is measured.</strong> The clip itself is real
    /// — <c>ScOutputData::PrePrintDrawingLayer</c> hands its rectangle to
    /// <c>BeginDrawLayers</c> as the paint region (<c>sc/source/ui/view/output3.cxx</c>:41-102).
    /// What was in dispute is whether it should be taken <em>unconditionally</em> and whether it
    /// should remove text. One round asserted 1 here, on the ground that "the reference's counts
    /// already have the clipped text removed"; the other took the clip only where a drawing
    /// actually leaves the block, and as <c>ClipPathKeepingText</c>.
    /// </para>
    /// <para>
    /// The conditional, text-keeping rule is the one that survives measurement.
    /// <b>Of 834 reference pages carrying a genuine (non-page-sized) drawing-layer clip, 733 still
    /// extract text lying wholly outside it</b> — so the reference's counts plainly do <em>not</em>
    /// have that text removed. And the sheets gate at the commit carrying the conditional rule
    /// reads <b>278 of 307</b>, which is exactly what the ODS round measured for its own change
    /// alone: the clip moved no verdict either way. The 262→287 attributed to the unconditional
    /// form was measured at load 15, where <c>dotnet/CLAUDE.md</c> records that the gate
    /// undercounts on the reference side, and its 262 base disagrees by 12 rows with an
    /// independent sweep of a neighbouring commit.
    /// </para>
    /// <para>
    /// Left open, and narrower than what was claimed: whether a clipped <em>shape's own</em> text
    /// should leave the text layer. The 733 pages above are mostly cell text, which the drawing
    /// layer's clip never governed.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnUncroppedPictureIsNotClipped()
        => ClipsOf("picture-watermark.xlsx").ShouldBe(0);

    // ------------------------------------------------------------------ helpers

    /// <summary>The single drawing on the fixture's single sheet.</summary>
    private static SheetDrawing Only(string name)
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings.Items.ShouldHaveSingleItem();
    }

    /// <summary>Where the fixture's one picture is actually drawn.</summary>
    private static DocRect OnlyImageOf(string name) => Draw(name).Images.ShouldHaveSingleItem();

    /// <summary>How many clips the fixture's rendering takes.</summary>
    private static int ClipsOf(string name)
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(name));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink.Clips;
    }

    /// <summary>The one page the fixture renders to.</summary>
    private static DrawnPage Draw(string name)
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(name));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        return sink.Pages.Single(page => page.Images.Count > 0);
    }
}
