using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A cropped picture in a <c>.doc</c>: the four Escher properties, the larger rectangle, and the
/// clip that makes it read as a crop.
/// </summary>
/// <remarks>
/// <para>
/// <b>This closes "Escher picture cropping, implemented nowhere in the word path".</b> The
/// arithmetic has been in <c>XRay.Office.Core.Geometry.PictureCrop</c> and the property read in
/// <c>XRay.Office.MsBinary.Escher.EscherPicture</c> since the slide round; what was missing was the
/// plumbing, and the plumbing is not one call. A frame has no rectangle until <c>FrameLayout</c>
/// has placed it, so the fractions travel on <see cref="FramePicture.Crop"/> and
/// <see cref="PageFrame.Crop"/> and <c>PageDrawing</c> does the arithmetic where
/// <c>PlacedFrame.Area</c> exists.
/// </para>
/// <para>
/// <b>The clip is new and is half the change.</b> <c>PageDrawing.DrawFrame</c> called
/// <c>DrawImage(image, frame.Area)</c> bare. Drawing into a larger rectangle without a clip does
/// not crop a picture — it puts the whole of it over the text on every side — so a round that
/// shipped only the rectangle would be a regression rather than a fix.
/// </para>
/// <para>
/// <b>There are three fixtures because a round trip cannot produce the file the corpus is made
/// of.</b> <c>picture-crop.docx</c> states <c>a:srcRect l="10000" t="20000" r="30000" b="40000"</c>
/// on an inline picture 288 × 216 pt; <c>picture-crop.doc</c> is that file through
/// <c>soffice --convert-to doc</c>, which is what turns the <c>a:srcRect</c> into Escher's 256–259
/// and puts the shape in the <c>Data</c> stream where an inline picture lives; and
/// <c>picture-crop-goal.doc</c> is that file patched into the shape Word writes. See the theory
/// below for why the third is not redundant.
/// </para>
/// <para>
/// <b>The <c>.docx</c> half reads <c>a:srcRect</c> too, and the three fixtures are the check on
/// each other.</b> It did not when this file was written, and the assertions here pinned it in
/// that state so the fix would show up as a move off them; it now goes through
/// <c>DocxPictures</c> into <see cref="FramePicture.Crop"/> and out through <c>DocxFrames</c> and
/// <c>DocxVmlFrames</c> onto the <see cref="PageFrame"/>, which is the same pair of hops the
/// <c>.doc</c> path takes. LibreOffice reaches one answer from both spellings as well — <c>oox</c>
/// turns <c>a:srcRect</c> into a <c>text::GraphicCrop</c> against the graphic's original size
/// (<c>fillproperties.cxx</c>:844-873) and the Escher reader produces the same property — so the
/// three fixtures agreeing to a fiftieth of a point is the assertion worth making.
/// </para>
/// </remarks>
public sealed class FramePictureCropTests
{
    /// <summary>
    /// Both shapes a <c>.doc</c> can state the same crop in, held to the same rectangle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The two differ in what <c>dxaGoal</c> means, and that is the whole trap.</b>
    /// <c>picture-crop.doc</c> is LibreOffice's own export: it states the crop twice, in the
    /// PICF's <c>dxaCropLeft</c> and siblings <em>and</em> in Escher 256–259, and sizes the goal
    /// to the whole 480 × 540 pt picture. <c>picture-crop-goal.doc</c> is that file patched into
    /// the shape Word writes and the corpus is made of — no PICF crop at all and a goal that is
    /// already the visible 288 × 216 pt.
    /// </para>
    /// <para>
    /// One formula covers both: the frame is the goal <em>less the PICF crop</em>, and the
    /// destination is that grown by the Escher fractions. An implementation that insets the goal
    /// by the Escher fractions instead passes on the exported file and is wrong on every real
    /// one — which is exactly what this round shipped first, and what the second fixture exists
    /// to make impossible.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("picture-crop.docx")]
    [InlineData("picture-crop.doc")]
    [InlineData("picture-crop-goal.doc")]
    public void ACroppedPictureIsDrawnLargerThanItsFrame(string name)
    {
        DocRect destination = OnlyImageOf(name);

        // 288 / (1 - 0.1 - 0.3) = 480 pt across and 216 / (1 - 0.2 - 0.4) = 540 pt down.
        destination.Width.Points.ShouldBe(480, 1.0, name);
        destination.Height.Points.ShouldBe(540, 1.0, name);
    }

    /// <summary>
    /// The visible part lands where the frame is: the whole picture starts 10% of its own width
    /// to the left of it and 20% of its own height above it.
    /// </summary>
    /// <remarks>
    /// The frame is the inline picture's <c>wp:extent</c>, 288 × 216 pt, at the one-inch margin —
    /// so this is the assertion that the crop moved the <em>picture</em> and left the
    /// <em>frame</em> alone, which is what distinguishes a crop from a resize and what the
    /// reference PDFs show.
    /// </remarks>
    [Theory]
    [InlineData("picture-crop.docx")]
    [InlineData("picture-crop.doc")]
    [InlineData("picture-crop-goal.doc")]
    public void TheVisiblePartOfThePictureLandsAtTheFrame(string name)
    {
        DocRect destination = OnlyImageOf(name);

        (destination.X.Points + (0.1 * destination.Width.Points)).ShouldBe(72, 1.5, name);
        (destination.Y.Points + (0.2 * destination.Height.Points)).ShouldBe(72, 1.5, name);
        (0.6 * destination.Width.Points).ShouldBe(288, 1.0, name);
        (0.4 * destination.Height.Points).ShouldBe(216, 1.0, name);
    }

    /// <summary>
    /// The same document in three formats crops to the same rectangle.
    /// </summary>
    /// <remarks>
    /// The cross-format check the fixtures exist for, and the one that would have caught the
    /// <c>.docx</c> arm dropping <c>a:srcRect</c> while both <c>.doc</c> arms read Escher's
    /// 256-259: the three agree to a fiftieth of a point in all four coordinates.
    /// </remarks>
    [Fact]
    public void TheThreeFormatsCropToTheSameRectangle()
    {
        DocRect docx = OnlyImageOf("picture-crop.docx");

        foreach (string name in new[] { "picture-crop.doc", "picture-crop-goal.doc" })
        {
            DocRect other = OnlyImageOf(name);

            other.X.Points.ShouldBe(docx.X.Points, 0.02, name);
            other.Y.Points.ShouldBe(docx.Y.Points, 0.02, name);
            other.Width.Points.ShouldBe(docx.Width.Points, 0.02, name);
            other.Height.Points.ShouldBe(docx.Height.Points, 0.02, name);
        }
    }

    /// <summary>
    /// A cropped picture is clipped, in every format — drawing into a larger rectangle without a
    /// clip does not crop a picture, it puts the whole of it over the text on every side.
    /// </summary>
    [Theory]
    [InlineData("picture-crop.docx")]
    [InlineData("picture-crop.doc")]
    [InlineData("picture-crop-goal.doc")]
    public void ACroppedPictureIsClipped(string name) => ClipsOf(name).ShouldBeGreaterThan(0);

    /// <summary>
    /// And an uncropped one is not. Clipping unconditionally would be invisible on the page and
    /// would change the bytes of every rendering in the corpus that carries a picture, which is
    /// exactly the kind of reach a round cannot then attribute to anything.
    /// </summary>
    /// <remarks>
    /// <c>picture-crop.docx</c> used to be this control, by not reading its own <c>a:srcRect</c>.
    /// <c>picture-anchor.docx</c> is one on purpose: it carries a picture and states no crop.
    /// </remarks>
    [Fact]
    public void AnUncroppedPictureIsNotClipped() => ClipsOf("picture-anchor.docx").ShouldBe(0);

    // ------------------------------------------------------------------ helpers

    /// <summary>Where the fixture's one picture is drawn.</summary>
    private static DocRect OnlyImageOf(string name)
    {
        RecordingDrawingSink sink = Draw(name);
        return sink.Pages.SelectMany(page => page.Images).ShouldHaveSingleItem();
    }

    /// <summary>How many clips the fixture's rendering takes.</summary>
    private static int ClipsOf(string name) => Draw(name).Clips;

    private static RecordingDrawingSink Draw(string name)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(name)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink;
    }
}
