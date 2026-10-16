using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A line holding nothing but an as-character picture is as tall as the picture, and keeps none of
/// the paragraph font's descent.
/// </summary>
/// <remarks>
/// <para>
/// Writer builds a line's descent from the portions that carry text, and a fly-in-content is not one
/// of them, so a picture alone on its line has nothing hanging below its baseline. The paragraph
/// font still <em>floors</em> the line wherever a run exists — an empty paragraph is one line of its
/// own font tall — which is why only the descent is dropped and not the whole fallback.
/// </para>
/// <para>
/// Measured against the installed 24.2.7.2 by <c>dotnet/probes/words-r46/picture-alone-descent.py</c>,
/// sixteen rows over two formats, two shapes and four picture heights, reported as the gap between
/// the baseline above the picture's paragraph and the baseline below it. In a 12 pt Liberation Serif
/// document, a DOCX picture alone on its line at 20, 50 and 150 pt gives 33.80, 63.80 and 163.80,
/// and the same file with any text beside the picture gives 36.40, 66.40 and 166.40 — 2.60 pt more,
/// which is that font's descent to the tenth. At 5 pt both come back 27.60, the floor.
/// </para>
/// <para>
/// This is round 45's open item, which could not be acted on because
/// <see cref="InlineObject.Height"/>'s remarks cite an ODF fixture where LibreOffice <em>does</em>
/// add that descent. It does, and it is not a format difference:
/// <c>dotnet/tests/corpus/features/picture-anchor.fodt</c> reads "An inline picture follows:
/// &lt;picture&gt; and that was it", so its picture has text on its line and it is the with-text row.
/// The probe authors both shapes in both formats to settle it and LibreOffice's fodt and docx agree
/// to 0.00 pt in all eight pairs.
/// </para>
/// <para>
/// The two readers' shapes both appear below because they differ: DOCX and DOC emit an anchor
/// character for an inline picture and RTF and ODF emit nothing, so the same paragraph arrives as a
/// one-control-character line from one reader and as an empty line from another.
/// </para>
/// <para>
/// <strong>Which of these are pinned rules and which are drift guards</strong>, established with
/// <c>verify-test.sh</c> rather than asserted. Three are detectors: dropping the rule fails
/// <see cref="APictureAloneOnAnAnchorCharacterLineIsExactlyAsTallAsThePicture"/>; letting it fire on
/// a line that does hold text fails <see cref="TextBesideThePictureKeepsItsDescent"/> and two
/// existing tests elsewhere in this project; dropping the run's <em>height</em> beside its descent
/// fails <see cref="AShortPictureAloneStillGetsTheParagraphFontsLineAsAFloor"/>. The other two are
/// drift guards and are labelled as such —
/// <see cref="APictureOnAnEmptyLineIsExactlyAsTallAsThePicture"/> asserts behaviour this change did
/// not alter, and no mutation of the guard can fail
/// <see cref="AControlCharacterWithNoObjectIsUntouched"/>, because with nothing raising the ascent
/// the run's height dominates the maximum whether its descent is counted or not. That is an
/// equivalent formulation rather than an undetected defect, which is a different finding and is
/// recorded as one.
/// </para>
/// </remarks>
public class PictureAloneLineHeightTests
{
    private static readonly Length Twelve = Length.FromPoints(12);
    private static readonly Length Picture = Length.FromPoints(150);

    /// <summary>The DOCX shape: one anchor character, one object, no text.</summary>
    [Fact]
    public void APictureAloneOnAnAnchorCharacterLineIsExactlyAsTallAsThePicture()
    {
        OpenTypeFace face = Carlito();
        const string Text = "\u0001";

        (Length height, Length ascent, _) = MeasuredParagraph
            .Measure(
                Text,
                [new FormattedRun(0, 1, face, Twelve)],
                objects: [new InlineObject(0, Length.Zero, Picture)])
            .MeasureLine(0, Text.Length);

        height.ShouldBe(Picture);
        ascent.ShouldBe(Picture);
    }

    /// <summary>The RTF and ODF shape: no character at all, one object on an empty line.</summary>
    [Fact]
    public void APictureOnAnEmptyLineIsExactlyAsTallAsThePicture()
    {
        OpenTypeFace face = Carlito();

        (Length height, _, _) = MeasuredParagraph
            .Measure(
                string.Empty,
                [new FormattedRun(0, 0, face, Twelve)],
                objects: [new InlineObject(0, Length.Zero, Picture)])
            .MeasureLine(0, 0);

        height.ShouldBe(Picture);
    }

    /// <summary>
    /// Text beside the picture keeps its descent, which is the whole of the difference the probe
    /// measures and the refuted alternative — "an object's line never has a text descent" — pinned.
    /// </summary>
    [Fact]
    public void TextBesideThePictureKeepsItsDescent()
    {
        OpenTypeFace face = Carlito();
        const string Text = "\u0001x";

        (Length withText, _, _) = MeasuredParagraph
            .Measure(
                Text,
                [new FormattedRun(0, 2, face, Twelve)],
                objects: [new InlineObject(0, Length.Zero, Picture)])
            .MeasureLine(0, Text.Length);

        withText.ShouldBeGreaterThan(Picture);
    }

    /// <summary>
    /// A picture shorter than the line the paragraph's font wants does not shrink it: the run's
    /// height is still accumulated, and only its descent is dropped. Pins the other refuted
    /// alternative, which is to skip the fallback whole.
    /// </summary>
    [Fact]
    public void AShortPictureAloneStillGetsTheParagraphFontsLineAsAFloor()
    {
        OpenTypeFace face = Carlito();
        const string Text = "\u0001";
        Length small = Length.FromPoints(5);

        (Length height, _, _) = MeasuredParagraph
            .Measure(
                Text,
                [new FormattedRun(0, 1, face, Twelve)],
                objects: [new InlineObject(0, Length.Zero, small)])
            .MeasureLine(0, Text.Length);

        height.ShouldBeGreaterThan(Length.FromPoints(12));
    }

    /// <summary>
    /// A control character with no object on the line is not this case. The rule is about a line an
    /// object fills, not about control characters, and a field's or a note's mark is drawn.
    /// </summary>
    [Fact]
    public void AControlCharacterWithNoObjectIsUntouched()
    {
        OpenTypeFace face = Carlito();
        const string Text = "\u0001";

        (Length height, _, _) = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, 1, face, Twelve)])
            .MeasureLine(0, Text.Length);

        (Length plain, _, _) = MeasuredParagraph
            .Measure("x", [new FormattedRun(0, 1, face, Twelve)])
            .MeasureLine(0, 1);

        height.ShouldBe(plain);
    }

    private static OpenTypeFace Carlito()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;
            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }

        return null;
    }
}
