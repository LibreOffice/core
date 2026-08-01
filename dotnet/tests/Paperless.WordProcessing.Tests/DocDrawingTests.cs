using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.MsBinary.Escher;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The DOC reader's half of the Escher drawing layer: the <c>FSPA</c> anchors, the shapes they
/// name, and the two questions those answer — whether a special character is an image, and where a
/// floating shape goes.
/// </summary>
/// <remarks>
/// The corpus documents are deliberately two. <c>frame-wrap.doc</c> holds one text box with a wrap,
/// which is the placement case; <c>picture-anchor.doc</c> holds an inline picture <em>and</em> a
/// text box, which is the classification case — and the classification cannot be tested with one of
/// them alone, because a reader that answers "picture" to everything and one that answers "shape"
/// to everything each pass half the corpus.
/// </remarks>
public sealed class DocDrawingTests
{
    /// <summary>
    /// The <c>FSPA</c>'s fields decode to what the document says, bitfield included.
    /// </summary>
    /// <remarks>
    /// Against the file's own numbers rather than against a rendering, because this is the one place
    /// a shift can be checked directly: <c>nwr</c> is four bits at 5 and <c>nwrk</c> four at 9, so an
    /// off-by-one in either produces a wrap that is merely a different legal wrap.
    /// </remarks>
    [Fact]
    public void AnFspaGivesTheShapesRectangleItsOriginsAndItsWrap()
    {
        Ww8Drawings drawings = DrawingsOf("frame-wrap.doc");

        drawings.AnchorCount.ShouldBe(1, "anchors in the body");

        Ww8ShapeAnchor anchor = drawings
            .AnchorAt(369)
            .ShouldNotBeNull("the anchor at the corpus document's shape position");

        anchor.ShapeId.ShouldBe(1026);

        // 4 cm by 3 cm at the paragraph's own left edge, which is what the ODF source asked for.
        anchor.Left.ShouldBe(0);
        anchor.Width.ShouldBe(2267, "the shape's width in twips");
        anchor.Height.ShouldBe(1692, "the shape's height in twips");

        anchor.HorizontalOrigin.ShouldBe(Ww8ShapeOrigin.Text);
        anchor.VerticalOrigin.ShouldBe(Ww8ShapeOrigin.Text);
        anchor.Wrap.ShouldBe(2, "nwr: wrap around an absolute object");
        anchor.WrapSide.ShouldBe(2, "nwrk: text on the right only");
        anchor.IsBelowText.ShouldBeTrue();
        anchor.IsHeaderAnchor.ShouldBeFalse();
    }

    /// <summary>
    /// The Escher blob is walked past the label byte Word puts before each drawing.
    /// </summary>
    /// <remarks>
    /// The one structural detail of the DOC drawing that cannot be inferred from the record grammar:
    /// an <c>OfficeArtWordDrawing</c> is one label byte and then a <c>DgContainer</c>, so a reader
    /// that walks the blob as plain sibling records reads the label plus three bytes of the next
    /// header as a record type and finds no shapes at all. This asserts shapes were found, which is
    /// the only symptom that failure has.
    /// </remarks>
    [Fact]
    public void TheDrawingIsFoundPastTheLabelByteThatPrecedesIt()
    {
        Ww8Drawings drawings = DrawingsOf("frame-wrap.doc");

        // The patriarch group's own shape and the text box: the reader drops the drawing's own
        // record and keeps everything drawn.
        drawings.ShapeCount.ShouldBeGreaterThanOrEqualTo(1, "shapes in the drawing");

        EscherShape shape = drawings.Shape(1026)
            .ShouldNotBeNull("the shape the body's only anchor names");

        shape.ShapeType.ShouldBe(EscherShapeTypes.TextBox);
        shape.Name.ShouldBe("Frame1");

        // In the tertiary table, which is Word's own and not the master's — the distinction the
        // shared reader keeps because the two tables share a layout and mean different things.
        shape.TertiaryProperties
            .Value(EscherPropertyIds.HorizontalRelation)
            .ShouldBe((uint)Ww8ShapeOrigin.Text);
    }

    /// <summary>
    /// A boolean property is read from its group's word rather than from its own identifier.
    /// </summary>
    /// <remarks>
    /// The corpus shape states <c>fFilled</c> false and <c>fLine</c> true, and both live in other
    /// properties: bit 4 of 447 and bit 3 of 511. Asking for 443 and 508 directly finds nothing, so
    /// a reader taking the absence as a default fills the frame and draws no border — the exact
    /// inverse of the document.
    /// </remarks>
    [Fact]
    public void TheShapesFillAndLineComeFromTheirGroupsBooleanWord()
    {
        EscherShape shape = DrawingsOf("frame-wrap.doc").Shape(1026)!;

        shape.Properties.Boolean(EscherPropertyIds.Filled, fallback: true).ShouldBeFalse();
        shape.Properties.StatesBoolean(EscherPropertyIds.Filled).ShouldBeTrue(
            "the shape does say so, rather than leaving it default");

        shape.Properties.Boolean(EscherPropertyIds.Lined, fallback: false).ShouldBeTrue();
        shape.Properties.Value(EscherPropertyIds.LineColour).ShouldBe(
            0x00A46534u, "the MSO_CLR whose channels run the other way: #3465A4");
    }

    /// <summary>
    /// A text box is not reported as an image, and a picture beside it still is.
    /// </summary>
    /// <remarks>
    /// The whole point of reading the record streams during extraction. Both anchors in
    /// <c>picture-anchor.doc</c> are special characters that carry no description of themselves, and
    /// LibreOffice's own export makes them look more alike than they are: the picture is written as
    /// a U+0001 whose <c>PICF</c> says mapping mode <c>0x64</c>, which means "the bytes after this
    /// header are an Escher shape" — the same thing an inline drawing says. Only the shape's own
    /// type, <c>mso_sptPictureFrame</c>, separates them.
    /// </remarks>
    [Fact]
    public void APictureIsReportedAndATextBoxIsNot()
    {
        using IDocument document = Open("picture-anchor.doc");

        Descendants(document.Content).OfType<ContentImage>().Count().ShouldBe(
            1, "one picture, and the text box beside it is not one");

        // And the text box's own text still arrives, as its own frame section rather than as an
        // image — the second record of the shape that a false picture would duplicate.
        document.Content.Children
            .OfType<ContentSection>()
            .Where(section => section.Kind == SectionKind.Frame)
            .ShouldHaveSingleItem()
            .GetText().Trim().ShouldBe("Shape text.");
    }

    /// <summary>A document with no drawing at all costs nothing and reports nothing.</summary>
    [Fact]
    public void ADocumentWithNoDrawingHasNoAnchorsAndNoShapes()
    {
        Ww8Drawings drawings = DrawingsOf("tables.doc");

        drawings.IsEmpty.ShouldBeTrue();
        drawings.AnchorAt(0).ShouldBeNull();
        drawings.Shape(1024).ShouldBeNull();
    }

    /// <summary>
    /// The frame the reader hands layout carries the geometry from both records.
    /// </summary>
    /// <remarks>
    /// Where <c>DocFrameComparisonTests</c> checks the result against LibreOffice's rendering, this
    /// checks the arithmetic in units the file states, so that a failure names the field rather than
    /// a pen position. The wrap spacing is the interesting one: 181 twips of drawing-layer default
    /// plus half the shape's 15-twip line, because Word states the rectangle as the outline's centre
    /// line and Writer keeps text clear of the bounding box the stroke straddles.
    /// </remarks>
    [Fact]
    public void TheFrameCarriesTheRectangleTheWrapAndTheDistances()
    {
        using IDocument document = Open("frame-wrap.doc");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PlacedFrame placed = pages.Pages[0].Frames.ShouldHaveSingleItem();
        PageFrame frame = placed.Frame;

        frame.Size.Width.Twips.ShouldBe(2267);
        frame.Size.Height.Twips.ShouldBe(1692);
        frame.Anchor.ShouldBe(
            FrameAnchor.Character, "Word has no paragraph anchor for a drawing");
        frame.Wrap.ShouldBe(TextWrap.Right);
        frame.HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Paragraph);
        frame.VerticalOrigin.ShouldBe(FrameVerticalOrigin.Paragraph);
        frame.HorizontalAlignment.ShouldBe(FrameHorizontalAlignment.Offset);
        frame.HorizontalOffset.Twips.ShouldBe(0);
        frame.VerticalOffset.Twips.ShouldBe(8);

        frame.Spacing.Left.Twips.ShouldBe(188, "181 of default plus half of a 15-twip line");
        frame.Spacing.Right.Twips.ShouldBe(188);
        frame.Spacing.Top.Twips.ShouldBe(7, "the vertical default is nothing, so only the half-line");
        frame.Spacing.Bottom.Twips.ShouldBe(7);

        // Stated as zero rather than left out, which is a different answer: the drawing layer's own
        // default is 0.1 inch horizontally.
        frame.Padding.Left.Twips.ShouldBe(0);
        frame.Padding.Top.Twips.ShouldBe(0);

        frame.Name.ShouldBe("Frame1");
        frame.IsImage.ShouldBeFalse("the shape has its own text");
        placed.Content.ShouldNotBeNull().Lines.Count.ShouldBe(2);
    }

    /// <summary>
    /// <c>posh</c> and <c>posv</c> beat the <c>FSPA</c>'s coordinate when they name an edge.
    /// </summary>
    /// <remarks>
    /// <c>picture-anchor.doc</c>'s text box states <c>posh</c> 2 — centred — and its <c>FSPA</c>
    /// still carries the left edge it last had, 3685 twips. A reader that always takes the
    /// coordinate places the shape somewhere plausible and wrong, which is why this is asserted on
    /// the alignment rather than only through a rendering. LibreOffice reads the same pair from the
    /// tertiary table at <c>msdffimp.cxx:5216</c>, and its own round trip of this document writes
    /// <c>style:horizontal-pos="center"</c>.
    /// </remarks>
    [Fact]
    public void AnEdgeAlignmentBeatsTheAnchorsCoordinate()
    {
        using IDocument document = Open("picture-anchor.doc");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        // The floating one. The page also carries the document's inline picture, which is placed by
        // hanging it on its line rather than against an origin and so has no alignment to check.
        PageFrame frame = pages.Pages[0].Frames
            .Select(placed => placed.Frame)
            .Where(candidate => candidate.Anchor != FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        frame.HorizontalAlignment.ShouldBe(FrameHorizontalAlignment.Centre);
        frame.VerticalAlignment.ShouldBe(FrameVerticalAlignment.Top);
        frame.Wrap.ShouldBe(TextWrap.Both, "nwr 2 with nwrk 0");

        // Stated rather than defaulted: 0.2 cm, which is 114 twips and not the 181 a shape that says
        // nothing gets. This shape's line is a single twip, so half of it rounds away.
        frame.Spacing.Left.Twips.ShouldBe(114);
    }

    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    /// <summary>
    /// The drawings of a corpus document, read through the reader's own streams.
    /// </summary>
    /// <remarks>
    /// Constructed directly rather than reached through <see cref="IDocument"/>, because the anchor
    /// table and the shape index are what is under test and the document deliberately does not
    /// expose them — a caller wants frames and images, not <c>FSPA</c>s.
    /// </remarks>
    private static Ww8Drawings DrawingsOf(string name)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(name));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        byte[] wordDocument = StreamOf(file, DocReader.WordDocumentStreamName)!;
        Ww8Fib fib = Ww8Fib.Parse(wordDocument);
        byte[] table = StreamOf(file, fib.UsesTable1Stream ? "1Table" : "0Table") ?? [];

        List<Diagnostic> diagnostics = [];
        Ww8DocumentReader reader = new(
            wordDocument, table, fib, diagnostics, StreamOf(file, DocReader.PictureStreamName));

        Ww8Drawings drawings = reader.Drawings;
        diagnostics.ShouldNotContain(
            diagnostic => diagnostic.Severity == DiagnosticSeverity.Error, $"{name} diagnostics");

        return drawings;
    }

    private static byte[]? StreamOf(CompoundFile file, string name)
    {
        IPackagePart? part = file.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode inner in Descendants(child)) yield return inner;
        }
    }
}
