using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where an inline <c>.doc</c> picture's bytes come from: its own container, never the
/// document's shared blip store.
/// </summary>
/// <remarks>
/// <para>
/// <b>A <c>pib</c> means two different things depending on where the shape is, and reading it
/// the wrong way draws the wrong picture at exactly the right place</b> — which is the worst
/// shape a defect can have, because every geometric assertion still passes. A floating shape's
/// <c>pib</c> is a one-based index into the document's <c>OfficeArtBStoreContainer</c>. An
/// inline picture is an <c>OfficeArtInlineSpContainer</c> — the shape, then its <em>own</em>
/// <c>FBSE</c> — and its <c>pib</c> is numbered from one inside that container, so the same
/// small number appears on every inline picture in the file and collides with the store
/// whenever the document also has floating shapes.
/// </para>
/// <para>
/// Measured on the corpus: <c>150_5300_13_chg10.doc</c> has twenty-five inline pictures whose
/// <c>pib</c> runs 1 to 22 and a shared store of twelve entries, and four of its inline figures
/// were each drawn as the same 197 x 77 grayscale JPEG belonging to a floating shape elsewhere
/// in the document. LibreOffice reaches the right answer by a different route and says why in
/// the code: <c>SwWW8ImplReader::ImportGraf</c> calls <c>DisableFallbackStream()</c> before
/// importing an inline shape, "##835## ... testing for existence in main stream may lead to an
/// incorrect fallback graphic being found" (<c>sw/source/filter/ww8/ww8graf2.cxx:531-537</c>).
/// </para>
/// <para>
/// <b>The fixture is newly authored and the collision in it is real rather than arranged.</b>
/// <c>picture-blip-collision.doc</c> holds one <em>anchored</em> PNG — which is what puts an
/// entry in the shared store — and one <em>inline</em> WMF cropped 10/20/30/40, and the inline
/// shape's <c>pib</c> is 1, which the store also answers. It is
/// <c>picture-blip-collision.docx</c> through <c>soffice --convert-to doc</c> and then patched
/// in place into the shape Word writes, the same two steps and the same reasoning as
/// <c>picture-crop-goal.doc</c>: a file round-tripped through the reference implementation is a
/// statement about the reference implementation, so the PICF crop is zeroed and the goal
/// shrunk to the visible extent before anything is asserted against it.
/// </para>
/// </remarks>
public sealed class InlineBlipLookupTests
{
    /// <summary>The 100 x 100 PNG the floating shape carries, in pixels.</summary>
    private const int FloatingPixels = 100;

    /// <summary>
    /// The inline picture is the metafile beside it in its own container, not the store's PNG.
    /// </summary>
    /// <remarks>
    /// Asserted on the picture rather than on its rectangle, because the rectangle is right
    /// either way: the frame comes from the <c>PICF</c> and the crop from the shape, so both the
    /// correct picture and the collided one are drawn at the same place and at the same size.
    /// That is exactly why this went unnoticed until a crop measurement disagreed with the
    /// reference on two figures and the disagreement turned out not to be about cropping at all.
    /// </remarks>
    [Fact]
    public void AnInlinePicturesPibDoesNotIndexTheDocumentBlipStore()
    {
        IReadOnlyList<DrawnPicture> pictures = PicturesOf("picture-blip-collision.doc");

        pictures.Count.ShouldBe(2);

        DrawnPicture inline = Inline(pictures);
        (inline.Image.Width == FloatingPixels && inline.Image.Height == FloatingPixels)
            .ShouldBeFalse(
                "the inline picture was drawn as the floating shape's 100 x 100 PNG, which is "
                + "what resolving its pib against the document's blip store produces");
    }

    /// <summary>
    /// The control: the floating picture in the same document <em>is</em> the store's PNG.
    /// </summary>
    /// <remarks>
    /// Without this the test above would pass on a reader that had stopped drawing the store's
    /// pictures at all, which would be a much larger regression wearing this fix's clothes.
    /// </remarks>
    [Fact]
    public void TheFloatingPictureInTheSameDocumentStillComesFromTheStore()
    {
        IReadOnlyList<DrawnPicture> pictures = PicturesOf("picture-blip-collision.doc");

        DrawnPicture floating = pictures.First(p => p.Destination != Inline(pictures).Destination);
        floating.Image.Width.ShouldBe(FloatingPixels);
        floating.Image.Height.ShouldBe(FloatingPixels);
    }

    /// <summary>
    /// And the inline picture is still drawn where it was, cropped as its shape asks.
    /// </summary>
    /// <remarks>
    /// A drift guard rather than a detector, and labelled: it repeats
    /// <see cref="FramePictureCropTests"/>'s arithmetic on a second fixture so that changing
    /// where an inline picture's bytes come from cannot quietly change where it lands. Every
    /// mutation that breaks it breaks a detector too.
    /// </remarks>
    [Fact]
    public void TheInlinePictureKeepsItsFrameAndItsCrop()
    {
        DocRect destination = Inline(PicturesOf("picture-blip-collision.doc")).Destination;

        // 288 / (1 - 0.1 - 0.3) = 480 pt across and 216 / (1 - 0.2 - 0.4) = 540 pt down.
        destination.Width.Points.ShouldBe(480, 1.0);
        destination.Height.Points.ShouldBe(540, 1.0);
    }

    // ------------------------------------------------------------------ helpers

    /// <summary>The larger of the two placements, which is the cropped inline picture.</summary>
    private static DrawnPicture Inline(IReadOnlyList<DrawnPicture> pictures)
        => pictures.MaxBy(p => p.Destination.Width.Emu);

    private static IReadOnlyList<DrawnPicture> PicturesOf(string name)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(name)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.SelectMany(page => page.Pictures)];
    }
}
