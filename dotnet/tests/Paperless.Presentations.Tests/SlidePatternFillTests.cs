using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// <c>a:pattFill</c>, on a shape and on a table cell.
/// </summary>
/// <remarks>
/// <para>
/// A pattern is not drawn as the 8×8 monochrome bitmap its preset names. LibreOffice
/// approximates each of the fifty-four with a hatch — <c>oox/inc/drawingml/hatchmap.hxx</c>,
/// whose own comment says so — and since the reference renderings this project is measured
/// against come out of LibreOffice, the hatch is the correct answer and the bitmap is not.
/// </para>
/// <para>
/// <b>Every expectation below is LibreOffice 24.2.7.2's own flat-ODF export of this fixture</b>,
/// read out of the <c>draw:hatch</c> elements it writes rather than inferred from the source:
/// <c>pct25</c> as <c>style="double" distance="0.2cm" rotation="450"</c>, <c>pct5</c> as
/// <c>single 0.25cm 450</c>, <c>wdUpDiag</c> as <c>single 0.1cm 450</c>, <c>wdDnDiag</c> as
/// <c>single 0.1cm 1350</c> and <c>pct50</c> as <c>double 0.125cm 450</c>, each beside a
/// <c>draw:fill-color</c> holding the pattern's <c>a:bgClr</c> and
/// <c>draw:fill-hatch-solid="true"</c>. Those five are the whole of the corpus: 65 pattern fills
/// across 7 decks name no other preset.
/// </para>
/// <para>
/// The table cell is not a variation on the shape. <c>DrawingTableCellBox.Fill</c> was a
/// <see cref="Colour"/> and had nowhere to put a hatch, and the corpus's largest instance —
/// page 24 of <c>slides/batch-011/pptx/171128IPAP.pptx</c>, eighteen fills on one slide — is a
/// table rather than a shape.
/// </para>
/// </remarks>
public class SlidePatternFillTests
{
    private const string Deck = "slide-pattern-fill.pptx";

    [Theory]
    [InlineData(0, HatchKind.TwoWay, 200, 45, 0xFF0000, 0xFFFFCC)]
    [InlineData(1, HatchKind.OneWay, 250, 45, 0x0000FF, 0xDDEEE0)]
    [InlineData(2, HatchKind.OneWay, 100, 45, 0x008000, 0xFFFFFF)]
    [InlineData(3, HatchKind.OneWay, 100, 135, 0x800080, 0xEEEEEE)]
    [InlineData(4, HatchKind.TwoWay, 125, 45, 0xFF8000, 0xFFFFFF)]
    public void EachPresetIsTheHatchLibreOfficeDrawsForIt(
        int shape, HatchKind kind, int mm100, int degrees, uint lines, uint background)
    {
        HatchPaint hatch = Shapes(0)[shape].Fill.ShouldBeOfType<HatchPaint>();

        hatch.Kind.ShouldBe(kind);
        hatch.Distance.ShouldBe(Length.FromMm100(mm100));
        hatch.Angle.ShouldBe(degrees * Math.PI / 180.0, 1e-9);
        hatch.LineColour.ShouldBe(Colour.FromRgb(lines));
        hatch.Background.ShouldBe(Colour.FromRgb(background));
    }

    [Fact]
    public void APatternNamingNoPresetIsItsBackgroundColourAlone()
    {
        // fillproperties.cxx:758 tests moPattPreset.has_value() before taking the hatch branch,
        // so a pattern with no preset falls through to `eFillStyle = FillStyle_SOLID` in its
        // background colour. LibreOffice's export of this shape is
        // draw:fill="solid" draw:fill-color="#ccccff", with no draw:hatch at all.
        Shapes(0)[5].Fill.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(0xCCCCFF));
    }

    [Fact]
    public void ATableCellStatesItsOwnPattern()
    {
        // Cells come first and the grid lines after, so the first four shapes are the cells in
        // row-major order. The corpus instance this exists for is exactly this shape.
        IReadOnlyList<PlacedShape> shapes = Shapes(1);

        HatchPaint first = shapes[0].Fill.ShouldBeOfType<HatchPaint>();
        first.Kind.ShouldBe(HatchKind.TwoWay);
        first.Distance.ShouldBe(Length.FromMm100(200));
        first.LineColour.ShouldBe(Colour.FromRgb(0x00B050));
        first.Background.ShouldBe(Colour.FromRgb(0xDDEEE0));

        // A solid cell beside it still reads as a solid, and a cell stating nothing has no fill.
        shapes[1].Fill.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(0xFFCC00));

        HatchPaint third = shapes[2].Fill.ShouldBeOfType<HatchPaint>();
        third.Kind.ShouldBe(HatchKind.OneWay);
        third.Distance.ShouldBe(Length.FromMm100(100));
        third.LineColour.ShouldBe(Colour.FromRgb(0x0070C0));

        shapes[3].Fill.ShouldBeNull();
    }

    private static IReadOnlyList<PlacedShape> Shapes(int slide)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[slide].Shapes;
    }
}
