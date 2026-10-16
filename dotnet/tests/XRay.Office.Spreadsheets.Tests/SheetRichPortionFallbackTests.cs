using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A rich cell's portion is never dropped, whatever it fails to say about its font.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="SheetText.ShapeRich"/> skipped a portion whose face would not resolve and one whose
/// size rounded to nought, which drops that portion's <strong>text</strong> — the worst failure
/// this path has, because the characters are in the file, in the extraction and in the content
/// tree, and only the page loses them. Silently: no diagnostic, no gap, nothing to notice but a
/// word that is not there. A portion drawn in the wrong face at the wrong size is wrong in a way
/// a reader can see and argue with, which is the trade the fallbacks make.
/// </para>
/// <para>
/// <strong>Only the size case can be provoked, and that is worth stating rather than working
/// around.</strong> <c>SheetFonts.For</c> goes through <c>SystemFontResolver</c>, which always
/// substitutes: asked for a family that is not installed it answers with one that is, so the face
/// branch cannot be reached from a test on any machine the suite runs on. It was reached in
/// theory only — measured on <c>Infotabelle_WLAN im Flugzeug.xlsx</c>, whose runs name
/// <c>Segoe UI</c>, absent from this container, the bold portion resolves to
/// <c>DejaVuSans-Bold.ttf</c> and shapes to a 54.44 pt segment. That document's missing word was
/// never a font-resolution failure at all; see <see cref="SheetCentredWrapBlankTests"/> for what
/// it actually was.
/// </para>
/// </remarks>
public sealed class SheetRichPortionFallbackTests
{
    [Fact]
    public void APortionStatingNoUsableSizeIsDrawnAtTheDefaultOne()
    {
        const string text = "before after";

        // A run whose rPr states sz="0", or an ODF span whose fo:font-size resolves to nothing.
        // Real files do this; rule 5 says repair it rather than refuse it.
        SheetTextPortion[] portions =
        [
            new(0, 7, SheetCellFormat.Default),
            new(7, 5, new SheetCellFormat { FontSize = Length.Zero }),
        ];

        SheetTextRun run = SheetText.ShapeRich(text, portions, scale: 1.0, start: 0, end: text.Length)!;

        string shaped = string.Concat(run.Segments.Select(segment => segment.Text));

        shaped.ShouldBe(text, "every character the portions cover reaches the page");
        run.Segments.Count.ShouldBe(2, "and it is still one segment per portion");

        // Not 0.113 pt, which is what it was: SnapFontSize floors a size at one device pixel, so
        // a portion stating none was drawn a ninth of a point tall — in the PDF's text layer and
        // invisible on the page, which is the worst of both answers.
        run.Segments[1].Size.ShouldBe(
            run.Segments[0].Size, "the sizeless portion falls back to the default ten point");
    }

    [Fact]
    public void AZeroSizedPortionIsAsWideAsItIsTall()
    {
        // The consequence that matters beyond the glyphs. The segments after a portion are laid
        // out from its width, so a portion drawn at a ninth of a point does not merely vanish —
        // it pulls the rest of the line 98% of its own width to the left.
        SheetTextPortion[] portions = [new(0, 5, new SheetCellFormat { FontSize = Length.Zero })];
        SheetTextPortion[] stated = [new(0, 5, SheetCellFormat.Default)];

        SheetTextRun run = SheetText.ShapeRich("width", portions, scale: 1.0, start: 0, end: 5)!;
        SheetTextRun reference = SheetText.ShapeRich("width", stated, scale: 1.0, start: 0, end: 5)!;

        run.Segments[0].Glyphs.Count.ShouldBe(5, "it has glyphs to draw");
        run.Width.Points.ShouldBe(reference.Width.Points, 0.01, "and the default size's width");
    }
}
