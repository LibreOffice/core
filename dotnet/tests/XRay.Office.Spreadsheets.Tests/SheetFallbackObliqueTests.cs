using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell character drawn from a fallback face keeps the lean of the cell's own font.
/// </summary>
/// <remarks>
/// <para>
/// <c>SheetFonts.ForFallback</c> names a substituted face through the reverse lookup that has no
/// request to compare against, so an italic cell holding a character its face cannot draw had that
/// character drawn upright beside its leaning neighbours.
/// </para>
/// <para>
/// The sheets corpus barely witnesses it — 10 of 307 workbooks draw a face no document names, and
/// the reference leans 4 glyphs there where we leaned none (<c>dragon-175066A.xlsx</c>) — but the
/// seat is shared with the other two tracks and the rule was measured through <c>.xlsx</c> and
/// <c>.fods</c> among four other filters (<c>probes/words-r58/</c>): six sheared glyphs on the
/// reference and none here, with an upright control at nought on both sides.
/// </para>
/// <para>
/// The cache is the second half of this. It is keyed on the face <em>and</em> the request, because
/// one substituted face answers both an upright cell and an italic one — keyed on the face alone,
/// whichever cell was laid out first would decide the lean for every other cell in the workbook.
/// </para>
/// </remarks>
public sealed class SheetFallbackObliqueTests
{
    /// <summary>U+6C49 汉, which no Latin face installed for this project covers.</summary>
    private const int Han = 0x6C49;

    private const string Japanese = "汉汉汉";

    /// <summary>A family with no italic anywhere, so its own italic is synthetic.</summary>
    private const string NoItalic = "Zqxwv Nonesuch";

    private static readonly Length Size = Length.FromPoints(10);

    private static SheetFace Face(bool italic)
        => SheetFonts.ForFamily("Liberation Serif", bold: false, italic)
            ?? throw new InvalidOperationException("Liberation Serif is not installed");

    [Fact]
    public void TheCellsFaceCoversNoneOfTheFixtureAndTheSubstituteHasNoItalic()
    {
        Face(italic: false).Face.HasGlyphFor(Han).ShouldBeFalse();
        Face(italic: true).Face.HasGlyphFor(Han).ShouldBeFalse();
        SheetFonts.Fallback.FallbackFor(Han).ShouldNotBeNull().IsItalic.ShouldBeFalse();
    }

    [Fact]
    public void AFallbackFaceInAnItalicCellIsDrawnLeaning()
        => Covering(italic: true).Reference.SyntheticOblique.ShouldBeTrue();

    [Fact]
    public void AFallbackFaceInAnUprightCellIsNotDrawnLeaning()
        // The control, and it is also the cache assertion: run in either order, the two answers
        // have to stay apart.
        => Covering(italic: false).Reference.SyntheticOblique.ShouldBeFalse();

    [Fact]
    public void TheTwoAnswersDoNotOverwriteEachOtherInTheCache()
    {
        // Asked in both orders in one test, because the cache is static and a per-test order is
        // not something a test class can rely on.
        bool first = Covering(italic: true).Reference.SyntheticOblique;
        bool second = Covering(italic: false).Reference.SyntheticOblique;
        bool third = Covering(italic: true).Reference.SyntheticOblique;

        first.ShouldBeTrue();
        second.ShouldBeFalse();
        third.ShouldBeTrue();
    }

    [Fact]
    public void AFallbackFaceInheritsALeanTheCellOnlyHasSynthetically()
    {
        SheetFace primary = SheetFonts.ForFamily(NoItalic, bold: false, italic: true)
            ?? throw new InvalidOperationException("nothing resolved for the nonesuch family");

        primary.Face.IsItalic.ShouldBeFalse("the fixture needs a family with no italic installed");
        primary.Reference.SyntheticOblique.ShouldBeTrue();
        primary.Face.HasGlyphFor(Han).ShouldBeFalse();

        SheetTextRun run = SheetText.Shape(Japanese, primary, Size).ShouldNotBeNull();
        run.Segments.Single(segment => segment.Face.Face.HasGlyphFor(Han))
            .Face.Reference.SyntheticOblique.ShouldBeTrue();
    }

    [Fact]
    public void TheLeaningFallbackIsStillNamedWellEnoughToBeEmbedded()
        => Covering(italic: true).Reference.FaceKey.ShouldNotBeNullOrEmpty();

    private static SheetFace Covering(bool italic)
    {
        SheetTextRun run = SheetText.Shape(Japanese, Face(italic), Size).ShouldNotBeNull();
        return run.Segments.Single(segment => segment.Face.Face.HasGlyphFor(Han)).Face;
    }
}
