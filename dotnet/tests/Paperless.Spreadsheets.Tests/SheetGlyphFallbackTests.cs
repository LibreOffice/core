using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A character the cell's face cannot draw is measured, broken and drawn in a face that can.
/// </summary>
/// <remarks>
/// <para>
/// A cell's face is chosen once, from the family name in its format; coverage is a property of a
/// character. So a workbook whose cells name a Latin face and hold Japanese asked that face to draw
/// ideographs, and got its missing-glyph box at its own <c>.notdef</c> advance — invisible text, and
/// an advance nothing like the full-width ideograph the reference renderer lays out after its own
/// fallback (<c>OutputDevice::ImplGlyphFallbackLayout</c>).
/// </para>
/// <para>
/// The wrong advance is the half that shows on the scoreboard, which is why the breaking pass is
/// asserted separately from the drawing one: the two reach the split by different routes —
/// <see cref="SheetTextLayout.LineCount"/> through a <c>ParagraphLayouter</c> built over
/// <see cref="FallbackShaper"/>, <see cref="SheetText.Shape"/> through its own itemisation — and a
/// fix to one alone gives a cell whose glyphs are right and whose row height is not, or the reverse.
/// Measured on <c>seihon_zassi_kikou_20221215.xlsx</c> against LibreOffice's own flat-ODF
/// <c>style:row-height</c>: 121 of 5159 rows came out one line short, each by exactly one line pitch
/// of 268.8 twips, and the workbook paginated 83 pages against 84.
/// </para>
/// <para>
/// Nothing here names a font. Which face covers CJK depends on what is installed, so each assertion
/// asks the face it got whether it has the character rather than asking what it is called.
/// </para>
/// </remarks>
public sealed class SheetGlyphFallbackTests
{
    /// <summary>U+6C49 汉, which no Latin face installed for this project covers.</summary>
    private const int Han = 0x6C49;

    /// <summary>Eight ideographs — long enough that their real width overruns the column below.</summary>
    private const string Japanese = "汉汉汉汉汉汉汉汉";

    private static readonly Length Size = Length.FromPoints(10);

    private static SheetFace Face =>
        SheetFonts.ForFamily("Liberation Serif")
        ?? throw new InvalidOperationException("Liberation Serif is not installed");

    [Fact]
    public void TheCellsOwnFaceCoversNoneOfTheFixture()
        // The premise everything below rests on. Were Liberation Serif to grow a CJK range, every
        // assertion here would pass while testing nothing, and this says so first.
        => Face.Face.HasGlyphFor(Han).ShouldBeFalse();

    [Fact]
    public void ShapingACellPutsTheCharacterInAFaceThatHasIt()
    {
        SheetTextRun run = SheetText.Shape(Japanese, Face, Size).ShouldNotBeNull();

        // Every glyph drawn and none of them .notdef. The face alone is not enough to assert: a run
        // handed to the right face at the wrong offsets still draws boxes.
        run.Segments.SelectMany(segment => segment.Glyphs)
            .ShouldAllBe(glyph => glyph.GlyphId != 0);
        run.Segments.ShouldContain(segment => segment.Face.Face.HasGlyphFor(Han));
    }

    [Fact]
    public void TheFallbackFaceIsNamedWellEnoughToBeEmbedded()
    {
        SheetTextRun run = SheetText.Shape(Japanese, Face, Size).ShouldNotBeNull();

        // A face is enough to shape with and not enough to embed: the PDF writer loads the font
        // program through the reference's face key, so a fallback carrying only a family name is
        // announced in the file without being embedded, and a reader without that font sees nothing.
        run.Segments.ShouldAllBe(segment => segment.Face.Reference.FaceKey.Length > 0);
    }

    [Fact]
    public void ACellMeasuresTheCoveringFacesAdvanceAndNotItsOwnNotdef()
    {
        Length measured = SheetText.Measure(Japanese, Face, Size);
        Length notdef = TextShaper.Default.Shape(Face.Face, Japanese, SheetText.NoKerning)
            .Width(Size);

        // The defect stated as a number rather than as a face name: the whole point is that the two
        // widths differ, because it is the width that decides where the cell's lines break.
        notdef.ShouldBeGreaterThan(Length.Zero, "the fixture has to measure as something");
        measured.ShouldNotBe(notdef);
    }

    [Fact]
    public void TheLineBreakerCountsTheLinesTheCoveringFaceNeeds()
    {
        // Wide enough for the .notdef run and not for the real one, so the count separates them.
        Length notdef = TextShaper.Default.Shape(Face.Face, Japanese, SheetText.NoKerning)
            .Width(Size);
        Length available = notdef + Length.FromTwips(20);

        SheetTextLayout.LineCount(Japanese, Face, Size, available).ShouldBeGreaterThan(1);
    }

    [Fact]
    public void TextTheCellsOwnFaceCoversReachesTheShaperInOnePiece()
    {
        // The no-op case, and it is not merely tidiness: shaping is contextual, so a run split it
        // did not need measures very slightly differently and can break a line somewhere else.
        SheetTextRun run = SheetText.Shape("Ordinary Latin prose", Face, Size).ShouldNotBeNull();

        run.Segments.Count.ShouldBe(1);
        run.Segments[0].Face.ShouldBe(Face);
    }
}
