using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A picture frame's own fill: read from the file always, drawn only when the picture over it can
/// be seen through.
/// </summary>
/// <remarks>
/// <para>
/// Impress adds a picture frame's fill to the shape's decomposition only when the graphic is
/// transparent (<c>svx/source/sdr/primitive2d/sdrgrafprimitive2d.cxx:41-42</c>), and a
/// <em>metafile</em> loaded from a package entry does not count as transparent however much of its
/// frame it leaves unpainted. So a red plate behind an EMF is drawn by PowerPoint and not by
/// Impress, while the same plate behind a PNG with an alpha channel is drawn by both.
/// </para>
/// <para>
/// <c>picture-frame-fill-deck</c> is one slide on a pale yellow background carrying all three cases
/// over the same <c>#FF0000</c> fill: the 892-byte EMF the <c>vector-picture</c> fixtures already
/// use, a 120 × 120 PNG whose right half is fully clear, and a shape with no picture at all.
/// Rendered by LibreOffice 24.2.7.2 it shows the EMF's marks on the slide's own yellow,
/// green-then-red for the PNG, and red for the bare shape — identically from the ODP and from the
/// PPTX.
/// </para>
/// <para>
/// The PPTX is LibreOffice's own export of the ODP with the two <c>p:pic</c> fills put back by
/// hand, because that export drops them: Impress knows the fill will not be drawn and writes no
/// <c>a:solidFill</c> into a <c>p:spPr</c> it wrote a picture into. The element restored is the one
/// PowerPoint writes, and the one <c>slides/batch-014/pptx/2014BSA_Sunday_Killion.pptx</c> carries
/// on the picture frames whose white plates this rule removes.
/// </para>
/// </remarks>
public class SlidePictureFrameFillTests
{
    private static readonly Colour Plate = Colour.FromRgb(0xFF0000);

    /// <summary>The two that carry the metafile as a package entry of its own.</summary>
    public static TheoryData<string> Packaged =>
        ["picture-frame-fill-deck.odp", "picture-frame-fill-deck.pptx"];

    /// <summary>All three, including the flat one that inlines it.</summary>
    public static TheoryData<string> BothFormats =>
        ["picture-frame-fill-deck.odp", "picture-frame-fill-deck.pptx",
         "picture-frame-fill-deck.fodp"];

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void EveryFrameStillReadsItsFill(string document)
    {
        // The fill is a property of the shape and is read as one. Suppressing it at read time would
        // lose the PNG case with the EMF case, and would make the two formats disagree about what
        // the file says.
        foreach (string name in new[] { "emf-over-fill", "png-over-fill", "bare-fill" })
        {
            Shape(document, name).Fill.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Plate);
        }
    }

    [Theory]
    [MemberData(nameof(Packaged))]
    public void TheFillUnderAPackagedMetafileIsNotDrawn(string document)
    {
        // The frame is 8 × 6 cm at (1, 2) cm. Before this rule the plate was painted there and the
        // slide's own background disappeared behind it.
        PlatesDrawn(document).ShouldNotContain(
            rect => Near(rect.X, 1) && Near(rect.Y, 2),
            "the metafile's frame paints its own fill");
    }

    [Fact]
    public void TheFillUnderAnInlineMetafileIsDrawn()
    {
        // The flat ODP says exactly what the zipped one says and carries the same 892 bytes; the
        // only difference is that they are base64 inside office:binary-data rather than a package
        // entry, and LibreOffice 24.2.7.2 draws the plate under this one and not under that one.
        // The corpus's `.ppt` decks fall on this side of the line and need to — see
        // SlideDrawing.FillReachesThePage, which records what the five measured cases do and do
        // not settle about why.
        PlatesDrawn("picture-frame-fill-deck.fodp")
            .ShouldContain(rect => Near(rect.X, 1) && Near(rect.Y, 2));
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void TheFillUnderAPictureWithAnAlphaChannelIsDrawn(string document)
    {
        // 8 × 6 cm at (10, 2) cm: the reference shows the plate through the PNG's clear right half,
        // so the rule is about the graphic's kind rather than about whether it covers its frame.
        PlatesDrawn(document).ShouldContain(rect => Near(rect.X, 10) && Near(rect.Y, 2));
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void TheFillOfAShapeWithNoPictureIsDrawn(string document)
    {
        // The control: 8 × 4 cm at (1, 9) cm, and the case that would break if the rule were read
        // as "a stated fill under anything is decoration".
        PlatesDrawn(document).ShouldContain(rect => Near(rect.X, 1) && Near(rect.Y, 9));
    }

    [Theory]
    [MemberData(nameof(Packaged))]
    public void TheSlideDrawsThePlateExactlyTwice(string document)
    {
        PlatesDrawn(document).Count.ShouldBe(2);
    }

    [Fact]
    public void TheFlatSlideDrawsItThreeTimes()
    {
        PlatesDrawn("picture-frame-fill-deck.fodp").Count.ShouldBe(3);
    }

    private static bool Near(Length value, double centimetres)
        => Math.Abs(value.Millimetres - (centimetres * 10)) < 1.0;

    /// <summary>Where the <c>#FF0000</c> plate is actually painted, in slide coordinates.</summary>
    private static List<DocRect> PlatesDrawn(string document)
    {
        PlacedDrawingSink sink = new();
        SlideDrawing.Draw(Slide(document), sink);

        return
        [
            .. sink.Fills
                .Where(fill => fill.Paint is SolidPaint solid && solid.Colour == Plate)
                .Select(fill => fill.Bounds),
        ];
    }

    private static PlacedShape Shape(string document, string name)
    {
        LaidOutSlide slide = Slide(document);

        return slide.Shapes.FirstOrDefault(shape => shape.Name == name)
               ?? throw new InvalidOperationException(
                   $"the fixture has no shape named '{name}'; it has "
                   + string.Join(", ", slide.Shapes.Select(shape => shape.Name ?? "<unnamed>")));
    }

    private static LaidOutSlide Slide(string document)
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(document)));

        return ((SlidePages)((IPaginatedDocument)read).Layout()).Slides.ShouldHaveSingleItem();
    }
}
