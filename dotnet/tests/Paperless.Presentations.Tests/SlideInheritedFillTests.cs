using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Where a shape's fill comes from when the shape does not state one: the placeholder it stands
/// in for, the slide's background, or the theme's format matrix.
/// </summary>
/// <remarks>
/// <para>
/// <c>slide-inherited-fill-deck.pptx</c> is <strong>hand-written</strong>, because neither
/// behaviour survives a round trip through LibreOffice: <c>p:sp/@useBgFill</c> is written by its
/// PPTX export only for a shape whose fill is already the page's
/// (<c>oox/source/export/shapes.cxx:874</c>), and Impress resolves a placeholder's inherited
/// properties on import, so an export states them outright and the inheritance under test is gone.
/// Its eleven parts are the minimum a PresentationML package needs.
/// </para>
/// <para>
/// One slide on a <c>#FFFFCC</c> background carrying three shapes, and LibreOffice 24.2.7.2 draws
/// them as follows:
/// </para>
/// <list type="bullet">
/// <item><c>moved-title</c> — a title placeholder stating its own <c>a:xfrm</c> and no fill —
/// drawn at the <em>slide's</em> rectangle in the <em>layout's</em> <c>#C00000</c>. Moving a
/// placeholder does not cut it off from the rest of what it inherits.</item>
/// <item><c>uses-background</c> — <c>useBgFill="1"</c>, no fill of its own, an
/// <c>a:fillRef</c> naming <c>accent1</c> — drawn in the background's <c>#FFFFCC</c> and not in
/// the accent.</item>
/// <item><c>uses-theme</c> — the same shape without the attribute — drawn in <c>accent1</c>,
/// which the theme makes <c>#0000FF</c>. It is the control that keeps the rule from being read
/// as "a themed fill never applies".</item>
/// </list>
/// </remarks>
public class SlideInheritedFillTests
{
    private const string Document = "slide-inherited-fill-deck.pptx";

    private static readonly Colour Plate = Colour.FromRgb(0xC00000);
    private static readonly Colour Background = Colour.FromRgb(0xFFFFCC);
    private static readonly Colour Accent = Colour.FromRgb(0x0000FF);

    [Fact]
    public void AMovedPlaceholderKeepsTheFillItInherits()
    {
        Fill("moved-title").ShouldBe(Plate);
    }

    [Fact]
    public void AMovedPlaceholderIsStillWhereTheSlidePutIt()
    {
        // The other half of the same rule, and the one that would hide a regression in it: if the
        // layout's whole p:spPr started winning instead of merging, the fill would be right and
        // the title would jump to the layout's rectangle at (5000000, 5000000) EMU.
        PlacedShape title = Shape("moved-title");

        title.Bounds.X.ShouldBe(Length.FromEmu(914400));
        title.Bounds.Y.ShouldBe(Length.FromEmu(457200));
    }

    [Fact]
    public void AShapeAskingForTheBackgroundGetsTheBackground()
    {
        Fill("uses-background").ShouldBe(Background);
    }

    [Fact]
    public void TheSameShapeWithoutTheAttributeGetsTheThemesFill()
    {
        Fill("uses-theme").ShouldBe(Accent);
    }

    [Fact]
    public void TheSlideBackgroundIsTheColourTheOtherTwoAreJudgedAgainst()
    {
        Slide().Background.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Background);
    }

    private static Colour Fill(string name)
        => Shape(name).Fill.ShouldBeOfType<SolidPaint>().Colour;

    private static PlacedShape Shape(string name)
    {
        LaidOutSlide slide = Slide();

        return slide.Shapes.FirstOrDefault(shape => shape.Name == name)
               ?? throw new InvalidOperationException(
                   $"the fixture has no shape named '{name}'; it has "
                   + string.Join(", ", slide.Shapes.Select(shape => shape.Name ?? "<unnamed>")));
    }

    private static LaidOutSlide Slide()
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Document)));

        return ((SlidePages)((IPaginatedDocument)read).Layout()).Slides.ShouldHaveSingleItem();
    }
}
