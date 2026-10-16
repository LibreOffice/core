using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// Text that paints no ink must not reach the file, because a PDF's text layer is read by
/// things that never look at the page.
/// </summary>
/// <remarks>
/// <para>
/// A clip hides ink and does not remove glyphs from a content stream, and an em of nothing
/// draws nothing while still showing a <c>Tj</c>. Either way the picture is right and
/// <c>pdftotext</c> reads words no reader of the page can see — the inverse of the defect a
/// word-count comparison usually catches, and invisible to every pixel metric.
/// </para>
/// <para>
/// It is also a disagreement between our own two backends: the raster sink hands the clip to
/// Skia, which drops the glyphs outright, so one display list produced a picture without the
/// text and a PDF whose text layer had it.
/// </para>
/// </remarks>
public sealed class PdfInvisibleTextTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void AClipKeepsTheTextInsideIt()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.DrawGlyphRun(Run("Inside", 20, 100), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldContain("BT\n", customMessage: "the run is inside the clip");
        content.ShouldContain("Tj\n");
    }

    [Fact]
    public void AClipDropsTheTextOutsideIt()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.DrawGlyphRun(Run("Outside", 400, 500), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        // The clip itself is still written — what the fix removes is the text under it, not the
        // clip, because a fill or a picture drawn afterwards still needs it in force.
        content.ShouldContain("W n\n");
        content.ShouldNotContain("BT\n");
    }

    [Fact]
    public void ARestoredClipStopsDroppingText()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.Restore();
            sink.DrawGlyphRun(Run("After", 400, 500), Paint.Solid(Colour.Black));
        });

        content.ShouldContain("BT\n", customMessage: "the clip was released before the run");
    }

    [Fact]
    public void AClipIsJudgedInTheSpaceTheTransformPutsIt()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // The clip is stated before the translation and the run after it, so the two are in
        // different spaces and only a tracked matrix relates them. Without one the run's
        // untranslated coordinates fall inside the clip and it survives.
        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.Transform(AffineTransform.Translation(
                Length.FromPoints(400).Emu, Length.FromPoints(400).Emu));
            sink.DrawGlyphRun(Run("Moved", 20, 100), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldNotContain("BT\n");
    }

    [Fact]
    public void ATransformCanCarryTextIntoTheClip()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // The mirror image of the case above, and the one that catches a matrix applied the
        // wrong way round: the run's own coordinates are outside the clip and the transform
        // brings them in.
        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.Transform(AffineTransform.Translation(
                Length.FromPoints(-400).Emu, Length.FromPoints(-400).Emu));
            sink.DrawGlyphRun(Run("Brought back", 420, 500), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldContain("BT\n");
    }

    [Fact]
    public void NestedTransformsComposeInnermostFirst()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // Two transforms in force at once, chosen so that the two composition orders put the run
        // on opposite sides of the clip. Applied innermost-first — the shrink last — the run
        // lands at (100, 100) and is kept; applied the other way it lands at (1000, 1000) and is
        // dropped. A single translation cannot tell the two apart, which is why this case exists
        // in addition to the one above.
        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.Transform(AffineTransform.Scale(0.1, 0.1));
            sink.Transform(AffineTransform.Translation(
                Length.FromPoints(1000).Emu, Length.FromPoints(1000).Emu));
            sink.DrawGlyphRun(Run("Nested", 0, 0), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldContain("BT\n");
    }

    [Fact]
    public void AnUnclippedPageKeepsEverything()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // A run well off the sheet is still written when nothing clipped it, because the sheet
        // edge is not a clip and treating it as one would be a different change with a different
        // risk.
        string content = ContentOf(sink =>
            sink.DrawGlyphRun(Run("Off the page", 2000, 2000), Paint.Solid(Colour.Black)));

        content.ShouldContain("BT\n");
    }

    [Fact]
    public void AnEmOfNothingDrawsNothing()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun sized = Run("Shrunk to nothing", 50, 100);
        GlyphRun collapsed = sized with { FontSize = Length.Zero };

        ContentOf(sink => sink.DrawGlyphRun(sized, Paint.Solid(Colour.Black)))
            .ShouldContain("BT\n");

        ContentOf(sink => sink.DrawGlyphRun(collapsed, Paint.Solid(Colour.Black)))
            .ShouldNotContain("BT\n");
    }

    [Fact]
    public void AClipThatKeepsTextWritesTheGlyphsOutsideItAnyway()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // The exception the rule above needs, and it is measured rather than a preference. A
        // spreadsheet cell whose text overruns the columns its page prints is cut at the block's
        // edge by LibreOffice too — and every glyph stays in the reference PDF's text layer. On
        // `Data-Architecture-Tool-Fit-Assessment-Template.xlsx` the two renderings' ink ends at
        // the same 241.8 pt while `pdftotext` reads all 2230 of the reference's words; dropping
        // them cost 124 of ours. Those are the document's own words, not a picture's spare corner.
        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPathKeepingText(Rectangle(0, 0, 200, 200));
            sink.DrawGlyphRun(Run("Outside", 400, 500), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldContain("W n\n", customMessage: "the ink is still cut");
        content.ShouldContain("Tj\n", customMessage: "the words are still readable");
    }

    [Fact]
    public void AnEnclosingTextHidingClipStillApplies()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // The two kinds nest, and the permissive one must not cancel the strict one — otherwise a
        // cell drawn inside a picture frame would put the frame's hidden text back in the file.
        string content = ContentOf(sink =>
        {
            sink.Save();
            sink.ClipPath(Rectangle(0, 0, 200, 200));
            sink.ClipPathKeepingText(Rectangle(0, 0, 100, 100));
            sink.DrawGlyphRun(Run("Outside both", 400, 500), Paint.Solid(Colour.Black));
            sink.Restore();
        });

        content.ShouldNotContain("BT\n");
    }

    private static GlyphRun Run(string text, double x, double y)
        => TestFace.Run(
            text,
            new DocPoint(Length.FromPoints(x), Length.FromPoints(y)),
            Length.FromPoints(12));

    private static GraphicsPath Rectangle(double x, double y, double width, double height)
        => GraphicsPath.Rectangle(new DocRect(
            Length.FromPoints(x), Length.FromPoints(y),
            Length.FromPoints(width), Length.FromPoints(height)));

    private static string ContentOf(Action<IDrawingSink> draw)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(DrawnPage.A4, draw)), buffer);

        return PdfFile.Parse(buffer.ToArray()).ContentStreams().Single();
    }
}
