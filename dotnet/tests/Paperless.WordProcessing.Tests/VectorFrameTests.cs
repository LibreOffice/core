using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Vector;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A word-processing frame holding a vector picture, in all four front ends.
/// </summary>
/// <remarks>
/// <para>
/// Until this landed, <c>PageFrame</c> could hold a <c>RasterImage</c> and nothing else, so a
/// document with an EMF logo laid out correctly and drew nothing — diagnostic <c>PL2370</c>. The
/// four front ends reach the bytes by four unrelated routes and none of them can be told what
/// format they are: DOCX names a relationship to a part whose name is a producer's choice, ODF a
/// package entry, RTF a control word its own exporters get wrong, and DOC an Escher blip whose
/// bytes are deflate-compressed behind a thirty-four-byte header. They meet at
/// <c>VectorImages.For</c>, which reads the bytes.
/// </para>
/// <para>
/// The whole-page comparison against LibreOffice is the render sweep; this pins what that rests on.
/// </para>
/// </remarks>
public class VectorFrameTests
{
    /// <summary>Every corpus document whose frame holds a picture this library decodes.</summary>
    public static TheoryData<string> Documents =>
    [
        "svg-picture.odt",
        "svg-picture.docx",
        "wmf-picture.odt",
        "wmf-picture.docx",
        "emf-picture.odt",
        "emf-picture.docx",
        "vector-picture-text.rtf",
    ];

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheFrameCarriesADisplayListRatherThanAnEmptyRectangle(string name)
    {
        IReadOnlyList<PageFrame> frames = Frames(name);

        frames.ShouldNotBeEmpty($"{name}: frames read");

        foreach (PageFrame frame in frames)
        {
            frame.IsImage.ShouldBeTrue($"{name}: {frame.Name} is declared a picture");
            frame.Vector.ShouldNotBeNull($"{name}: {frame.Name} decoded as a vector");
            frame.Vector.Value.IsEmpty.ShouldBeFalse($"{name}: {frame.Name} has ink");
        }
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void NothingIsDecodedWhileTheDocumentIsRead(string name)
    {
        // Extraction must not pay for a decode. The measured cost of getting this wrong is not
        // subtle: on this tree the first VectorImages.Decode in a process is 1044 ms for a WMF
        // carrying one text run, 381 ms for an EMF+ and 67 ms for a text-free EMF, against
        // 0.08-0.21 ms once warm — because the first one resolves and loads faces through
        // Paperless.Text. RTF and DOC read their pictures while parsing, so an eager decode there
        // would put a second of font work on a caller that only wanted the words.
        foreach (PageFrame frame in Frames(name))
        {
            frame.Vector.ShouldNotBeNull().IsValueCreated.ShouldBeFalse($"{name}: {frame.Name}");
        }
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheFramesWholePictureIsStretchedOntoItRatherThanItsInk(string name)
    {
        // The trap named in the seam, and the one a reader hits first. `VectorImage.Draw` maps the
        // picture's view box onto the destination; the ink lands wherever the picture's own margins
        // put it inside that. Fitting the *ink* instead makes every logo with a margin creep
        // outwards and clip — which reads as a mapping bug in the decoder and is not one.
        foreach (PageFrame frame in Frames(name))
        {
            VectorImage picture = frame.Vector.ShouldNotBeNull().Value;

            PlacedDrawingSink sink = new();
            DocRect destination = new(Length.Zero, Length.Zero, frame.Size.Width, frame.Size.Height);
            picture.Draw(sink, destination);

            DocRect ink = sink.Ink;
            ink.IsEmpty.ShouldBeFalse($"{name}: {frame.Name} drew nothing");

            // Inside the frame, and not filling it: every fixture's picture states margins.
            ink.X.Emu.ShouldBeGreaterThan(0, $"{name}: {frame.Name} starts inside its frame");
            ink.Width.Emu.ShouldBeLessThan(
                frame.Size.Width.Emu, $"{name}: {frame.Name} is narrower than its frame");
        }
    }

    [Theory]
    [InlineData("svg-picture.odt")]
    [InlineData("svg-picture.docx")]
    public void TheWordmarkInsideTheSvgIsARealGlyphRun(string name)
    {
        // The eighth word of `svg-picture.{odt,docx}`, and the reason both rows sat at 7/8 against
        // LibreOffice before this: the "PAPERLESS" wordmark is inside the picture, and our decoder
        // emits it as a GlyphRun so a PDF backend writes real searchable text for it.
        PageFrame frame = Frames(name).ShouldHaveSingleItem();

        PlacedDrawingSink sink = new();
        frame.Vector.ShouldNotBeNull().Value.Draw(
            sink, new DocRect(Length.Zero, Length.Zero, frame.Size.Width, frame.Size.Height));

        sink.Runs.ShouldHaveSingleItem().Run.Text.ShouldBe("PAPERLESS");
    }

    [Fact]
    public void TheDocxPrefersTheSvgAndKeepsTheRasterFallbackBesideIt()
    {
        // `BlipReference.Choose`. LibreOffice's own DOCX export writes both: 769 bytes of SVG in an
        // `asvg:svgBlip` extension and a 3 803-byte PNG on `r:embed`. Preferring the extension is
        // what vector import is for; keeping the raster is what an empty decode falls back to.
        PageFrame frame = Frames("svg-picture.docx").ShouldHaveSingleItem();

        frame.Vector.ShouldNotBeNull();
        frame.Image.ShouldNotBeNull().EncodedBytes.Length.ShouldBeGreaterThan(3000);
    }

    [Theory]
    [InlineData("svg-picture.odt")]
    [InlineData("wmf-picture.odt")]
    [InlineData("emf-picture.odt")]
    [InlineData("vector-picture-text.rtf")]
    public void AVectorFrameKeepsNoRasterWhereTheFormatOffersNoAlternative(string name)
    {
        // ODF lists alternatives as sibling `draw:image` elements and the first drawable one wins,
        // so there is nothing to keep beside it; RTF states one picture per `{\pict}`. Only the
        // DrawingML extension names two renderings of the same artwork.
        foreach (PageFrame frame in Frames(name))
        {
            frame.Image.ShouldBeNull($"{name}: {frame.Name}");
        }
    }

    [Fact]
    public void AnRtfMetafileIsFoundByItsBytesAndNotByItsControlWord()
    {
        // `vector-picture-text.rtf` states `\wmetafile8` for the WMF and `\emfblip` for the EMF,
        // and both are ignored: the sniff is what decides, because RTF exporters get the control
        // word wrong routinely. Measured on the same file: LibreOffice's own RTF export of this
        // document writes `\pngblip` for both pictures — it rasterises on the way out — so a
        // fixture that went through it would not exercise either word at all, which is why this
        // one is written by hand.
        IReadOnlyList<PageFrame> frames = Frames("vector-picture-text.rtf");

        frames.Count.ShouldBe(2);
        frames[0].Vector.ShouldNotBeNull().Value.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.1);
        frames[1].Vector.ShouldNotBeNull().Value.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.1);
    }

    [Fact]
    public void ADocsMetafileBlipIsInflatedOutOfItsEscherRecord()
    {
        // The route nothing else takes. An Escher metafile blip is not the picture: it is a
        // thirty-four-byte `OfficeArtMetafileHeader` followed by a **deflate stream**, where a
        // raster blip is one tag byte followed by the file. `SvxMSDffManager::GetBLIPDirect` sets
        // its ZCodec for the EMF, WMF and PICT cases and for no other (msdffimp.cxx:6518-6549).
        // A reader that skipped the header without inflating finds no placeable magic and no
        // METAHEADER, and declines the picture as an unrecognised blob — measured here: 892 bytes
        // of EMF arrive as 262 bytes of deflate behind the header.
        IReadOnlyList<PageFrame> frames = Frames("vector-picture-text.doc");

        frames.Count.ShouldBe(3);

        // Two of the three, and the third is the measurement worth keeping: **DOC has no blip type
        // for SVG**, so LibreOffice's own export rasterises it to a PNG on the way out where it
        // keeps the WMF and the EMF byte for byte. A vector picture therefore survives a round trip
        // through three of the four word-processing formats and not through this one.
        List<PageFrame> vectors = [.. frames.Where(frame => frame.Vector is not null)];
        vectors.Count.ShouldBe(2);
        frames.Count(frame => frame.Image is not null).ShouldBe(1);

        foreach (PageFrame frame in vectors)
        {
            frame.Vector.ShouldNotBeNull().Value.IsEmpty.ShouldBeFalse();

            // The frame's own 8 x 6 cm, and not the picture's intrinsic size — which for these two
            // is 270.93 mm, because LibreOffice re-generates the metafile at page scale on the way
            // out. The document's extent wins over the picture's own, exactly as `VectorImage.Draw`
            // says it does.
            frame.Size.Width.Millimetres.ShouldBe(80, 0.5);
            frame.Size.Height.Millimetres.ShouldBe(60, 0.5);
        }
    }

    private static IReadOnlyList<PageFrame> Frames(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        WordProcessingPages pages = (WordProcessingPages)document.Layout();

        return
        [
            .. pages.Pages
                .SelectMany(page => page.Frames)
                .Select(placed => placed.Frame)
                .Where(frame => frame.Vector is not null || frame.IsImage),
        ];
    }
}
