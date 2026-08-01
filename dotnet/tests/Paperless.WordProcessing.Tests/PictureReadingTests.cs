using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Pictures reaching layout with their bytes attached, in all four formats.
/// </summary>
/// <remarks>
/// <para>
/// The four state a picture in four unrelated ways — ODF names a package entry or inlines base64,
/// DOCX resolves a relationship, RTF writes hexadecimal inside a shape property, and DOC indexes a
/// blip store from an Escher property — so the interesting assertion is that all four arrive at the
/// same place: a <see cref="PageFrame"/> carrying a <see cref="RasterImage"/> whose bytes are the ones
/// the file stored.
/// </para>
/// <para>
/// Asserted on the <em>encoded</em> bytes and never on pixels, which is the layering under test as much
/// as the reading: nothing on this path may decode, because a decoder lives in
/// <c>Paperless.Rendering</c> and extraction must not pay for one. <see cref="RasterImage.IsDecoded"/>
/// being false is therefore part of the contract rather than an incidental detail.
/// </para>
/// </remarks>
public sealed class PictureReadingTests
{
    /// <summary>The PNG signature, which is what "these are the file's own bytes" looks like.</summary>
    private static readonly byte[] PngSignature = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];

    /// <summary>
    /// Every format finds the picture and hands it over still encoded.
    /// </summary>
    /// <remarks>
    /// <c>picture-flow</c> holds two pictures of the same 16 × 16 PNG — one inline and one floating —
    /// and every export of it kept both. The two corpus documents between them cover the four routes to
    /// the bytes: <c>picture-anchor.fodt</c> is the only one with an <c>office:binary-data</c>, its
    /// <c>.odt</c> the only one with an <c>xlink:href</c> into the package, and the RTF and DOC
    /// exports put their pictures in a shape property and a blip store respectively.
    /// </remarks>
    [Theory]
    [InlineData("picture-anchor.fodt", 1)]
    [InlineData("picture-anchor.odt", 1)]
    [InlineData("picture-anchor.docx", 1)]
    [InlineData("picture-anchor.doc", 1)]
    [InlineData("picture-flow.fodt", 2)]
    [InlineData("picture-flow.odt", 2)]
    [InlineData("picture-flow.docx", 2)]
    [InlineData("picture-flow.doc", 2)]
    [InlineData("picture-flow.rtf", 2)]
    public void EveryFormatCarriesThePicturesBytesToLayout(string name, int expected)
    {
        List<PlacedFrame> pictures = [.. Frames(name).Where(frame => frame.Frame.Image is not null)];

        pictures.Count.ShouldBe(expected, $"pictures in {name}");

        foreach (PlacedFrame placed in pictures)
        {
            RasterImage image = placed.Frame.Image.ShouldNotBeNull();

            image.IsDecoded.ShouldBeFalse("a reader must not decode; the backend does");
            image.EncodedMediaType.ShouldBe("image/png");
            image.EncodedBytes.Span[..8].ToArray().ShouldBe(
                PngSignature, "the bytes are the file's own, not a re-encoding");
        }
    }

    /// <summary>
    /// A picture's pixel size is read from its header, without a codec.
    /// </summary>
    /// <remarks>
    /// The two corpus documents carry different pictures on purpose — a 2 × 2 PNG in
    /// <c>picture-anchor</c> and a 16 × 16 one in <c>picture-flow</c> — so that a reader returning a
    /// plausible constant would fail. LibreOffice separates the same two questions for the same reason:
    /// <c>GraphicDescriptor</c> reports a picture's format and size from its header alone, leaving the
    /// decode to whoever needs pixels.
    /// </remarks>
    [Theory]
    [InlineData("picture-anchor.fodt", 2)]
    [InlineData("picture-anchor.doc", 2)]
    [InlineData("picture-flow.docx", 16)]
    [InlineData("picture-flow.rtf", 16)]
    public void APicturesSizeIsReadFromItsHeaderRatherThanDecoded(string name, int side)
    {
        RasterImage image = Frames(name)
            .Select(frame => frame.Frame.Image)
            .First(picture => picture is not null)
            .ShouldNotBeNull();

        image.Width.ShouldBe(side);
        image.Height.ShouldBe(side);
        image.Pixels.IsEmpty.ShouldBeTrue("the size came from the header, so nothing was decoded");
    }

    /// <summary>
    /// An inline picture hangs where its anchor character sits, not at the paragraph's corner.
    /// </summary>
    /// <remarks>
    /// Measured against LibreOffice's own PDF of <c>picture-anchor.fodt</c>, which draws the picture at
    /// 183.35 pt — the text area's 56.7 plus the width of "An inline picture follows: " — and puts its
    /// bottom edge on the line's baseline. Before an as-character frame was hung on its line it was
    /// placed against an origin like a floating one, which put every inline picture at the start margin
    /// on top of the paragraph's first words.
    /// </remarks>
    [Theory]
    [InlineData("picture-anchor.fodt")]
    [InlineData("picture-anchor.odt")]
    [InlineData("picture-anchor.docx")]
    [InlineData("picture-anchor.doc")]
    public void AnInlinePictureHangsAtItsAnchorOnTheLine(string name)
    {
        PlacedFrame inline = Frames(name)
            .Where(frame => frame.Frame.Anchor == FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        inline.Area.X.Points.ShouldBe(183.35, 0.05, "LibreOffice draws it at 183.35 pt");
        inline.Frame.Wrap.ShouldBe(
            TextWrap.Through, "an as-character frame is part of the text, never an obstacle");
    }

    /// <summary>
    /// An inline picture is never an obstacle, so the lines it shares a paragraph with keep their width.
    /// </summary>
    /// <remarks>
    /// Writer gives an as-character frame a <c>SwFlyCntPortion</c> — a portion of the line — rather than
    /// an entry in <c>SwTextFly</c>'s object list, so it takes room <em>on</em> the line instead of
    /// taking room <em>from</em> it. The distinction is invisible until a document has both kinds:
    /// <c>picture-flow</c> does, and its floating picture does narrow the lines beside it.
    /// </remarks>
    [Fact]
    public void AnInlinePictureDoesNotNarrowItsOwnParagraphsLines()
    {
        using IDocument document = Open("picture-flow.fodt");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        LaidOutPage page = pages.Pages[0];

        PlacedFrame inline = page.Frames
            .Where(frame => frame.Frame.Anchor == FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        // The paragraph the inline picture is anchored in — the second — against the first, which has
        // no frame near it at all. Equal starts mean the picture took nothing off the line.
        List<PlacedLine> first = [.. page.Lines.Where(line => line.ParagraphIndex == 0)];
        List<PlacedLine> second = [.. page.Lines.Where(line => line.ParagraphIndex == 1)];

        second.ShouldNotBeEmpty();
        second[0].Box.Left.ShouldBe(first[0].Box.Left);

        // And the floating one does the opposite, which is what makes the comparison meaningful.
        PlacedFrame floating = page.Frames
            .Where(frame => frame.Frame.Anchor != FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        floating.Frame.Wrap.ShouldBe(TextWrap.Right);

        List<PlacedLine> third = [.. page.Lines.Where(line => line.ParagraphIndex == 2)];
        third.ShouldNotBeEmpty();
        third[0].Box.Left.ShouldBeGreaterThan(
            first[0].Box.Left, "the floating picture pushes the text past its right edge");

        inline.Area.Width.ShouldBe(inline.Frame.Size.Width);
    }

    /// <summary>
    /// A document with no picture in it carries none, and pays nothing for the question.
    /// </summary>
    /// <remarks>
    /// The half of the classification that the DOC reader gets wrong most easily: a text box is a
    /// picture frame shape with no <c>pib</c>, so a reader keying off the shape type reports one phantom
    /// picture per text box. <c>frame-wrap.doc</c> is exactly that document.
    /// </remarks>
    [Theory]
    [InlineData("frame-wrap.doc")]
    [InlineData("frame-wrap.fodt")]
    [InlineData("frame-wrap.rtf")]
    public void AFrameWithNoPictureCarriesNoImage(string name)
    {
        foreach (PlacedFrame frame in Frames(name))
        {
            frame.Frame.Image.ShouldBeNull($"{name} has a text box and no picture");
        }
    }

    private static IReadOnlyList<PlacedFrame> Frames(string name)
    {
        using IDocument document = Open(name);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Pages.SelectMany(page => page.Frames)];
    }

    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));
}
