using System.Globalization;
using System.Text.RegularExpressions;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;
using XRay.Office.Text.Fonts;
using Shouldly;
using SkiaSharp;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// What a PDF holds for a glyph whose shape is a colour bitmap rather than an outline.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The defect these pin.</strong> A colour bitmap face carries <c>CBDT</c>/<c>CBLC</c> and
/// no outlines at all, so writing it as a simple <c>/Subtype/TrueType</c> font named a face, stated
/// its widths, embedded a program with no <c>glyf</c> in it and drew <em>nothing</em> — the right
/// face at the right advance and a blank on the page. Every gate the corpus harness has passed:
/// the page count, the extracted words and the embedded-font check were all correct.
/// </para>
/// <para>
/// <strong>The shape asserted here is LibreOffice 26.2.4.2's own.</strong> Measured on its PDF of a
/// <c>U+2714</c> probe: <c>pdffonts</c> reports <c>BAAAAA+NotoColorEmoji</c> as <em>Type 3, Custom
/// encoding, embedded, with a ToUnicode</em>; the font dictionary carries
/// <c>/FontMatrix[0.001 0 0 0.001 0 0]</c>, a <c>/CharProcs</c> keyed by glyph and an
/// <c>/Encoding /Differences</c> naming them; and each char proc is <c>… 0 d0</c> followed by
/// <c>q … cm /Im12 Do Q</c> over a <c>/DeviceRGB</c> image with an <c>/SMask</c>.
/// </para>
/// <para>
/// They assert the mechanism rather than a pixel count: that the strike the face states is the
/// image the file holds, that it is placed where the strike's own metrics put it, and that the text
/// layer is untouched — because it is the text layer being untouched that keeps a colour glyph
/// searchable.
/// </para>
/// </remarks>
public sealed partial class PdfColourGlyphTests
{
    /// <summary>A character with the Unicode Emoji property, which is what falls back to such a face.</summary>
    private const int Tick = 0x2714;

    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    /// <summary>The face reaches the file as a Type 3 font and not as a TrueType one.</summary>
    [Fact]
    public void AColourFaceIsWrittenAsAType3Font()
    {
        PdfFile pdf = Write();

        pdf.Text.ShouldContain("/Subtype/Type3");
        pdf.Text.ShouldContain("/FontMatrix[0.001 0 0 0.001 0 0]");
        pdf.Text.ShouldContain("/CharProcs<<");
        pdf.Text.ShouldContain("/Encoding<</Type/Encoding/Differences[1");

        // No font program: a Type 3 font *is* its char procs, and the program that used to be
        // embedded was a `glyf`-less TrueType that promised outlines it did not have.
        pdf.FontPrograms().ShouldBeEmpty();
        pdf.Text.ShouldNotContain("/FontFile2");
    }

    /// <summary>The char proc draws the strike where the strike's own metrics put it.</summary>
    /// <remarks>
    /// The four numbers of the <c>cm</c> are the strike's placement in design units scaled into the
    /// font matrix's thousandths of an em, and they are recomputed here from
    /// <see cref="ColourBitmap.PlacementIn(int)"/> rather than written out — the placement
    /// arithmetic itself is checked against the reference's constants in
    /// <c>XRay.Office.Text.Tests.ColourBitmapTests</c>, and what this asserts is that the writer
    /// composes it into the char proc unchanged.
    /// </remarks>
    [Fact]
    public void TheCharProcPlacesTheStrikeFromItsOwnMetrics()
    {
        PdfFile pdf = Write();

        OpenTypeFace face = TestColourFace.Face;
        ushort glyph = face.Characters.GlyphFor(Tick);
        ColourBitmap bitmap = ColourBitmaps.Of(face, glyph).ShouldNotBeNull();

        (int left, int bottom, int width, int height) = bitmap.PlacementIn(face.UnitsPerEm);
        double scale = 1000.0 / face.UnitsPerEm;

        Match proc = CharProc().Match(pdf.Text);
        proc.Success.ShouldBeTrue($"no char proc in the file: {Truncated(pdf.Text)}");

        // d0 rather than d1: a d1 glyph takes its colour from the page, and the whole content of
        // one of these is its own colour.
        double advance = face.AdvanceOf(glyph) * 1000.0 / face.UnitsPerEm;
        Number(proc.Groups[1]).ShouldBe(Math.Round(advance, 4), 0.0002);

        Number(proc.Groups[2]).ShouldBe(Math.Round(width * scale, 4), 0.0002);
        Number(proc.Groups[3]).ShouldBe(Math.Round(height * scale, 4), 0.0002);
        Number(proc.Groups[4]).ShouldBe(Math.Round(left * scale, 4), 0.0002);
        Number(proc.Groups[5]).ShouldBe(Math.Round(bottom * scale, 4), 0.0002);
    }

    /// <summary>The image the char proc draws is the face's own strike, decoded.</summary>
    /// <remarks>
    /// The strike is a PNG and PDF has no PNG filter, so it is decoded to <c>/DeviceRGB</c> with the
    /// alpha channel split out into an <c>/SMask</c> — the shape the reference writes and the one
    /// <c>PdfImages</c> already produces for every other picture. The dimensions are the assertion
    /// that it is <em>this</em> strike and not some other image on the page.
    /// </remarks>
    [Fact]
    public void TheImageIsTheStrikeItself()
    {
        PdfFile pdf = Write();

        ColourBitmap bitmap = ColourBitmaps
            .Of(TestColourFace.Face, TestColourFace.Face.Characters.GlyphFor(Tick))
            .ShouldNotBeNull();

        List<string> images = [.. pdf.Streams()
            .Select(s => s.Dictionary)
            .Where(d => d.Contains("/Subtype/Image", StringComparison.Ordinal))];

        // Two: the colour plane and the mask, which is itself an image XObject because PDF has no
        // RGBA colour space.
        images.Count.ShouldBe(2, $"a colour plane and its mask: {string.Join(" | ", images)}");

        string colour = images
            .Where(d => d.Contains("/DeviceRGB", StringComparison.Ordinal))
            .ShouldHaveSingleItem();
        string mask = images
            .Where(d => d.Contains("/DeviceGray", StringComparison.Ordinal))
            .ShouldHaveSingleItem();

        string width = bitmap.PixelWidth.ToString(CultureInfo.InvariantCulture);
        string height = bitmap.PixelHeight.ToString(CultureInfo.InvariantCulture);

        colour.ShouldContain($"/Width {width}");
        colour.ShouldContain($"/Height {height}");
        colour.ShouldContain("/SMask", customMessage: "an emoji is transparent outside its own shape");

        mask.ShouldContain($"/Width {width}");
        mask.ShouldContain($"/Height {height}");

        // The XObject belongs to the font's resources rather than the page's, because the char proc
        // is what draws it and a char proc resolves its names against the font.
        pdf.Text.ShouldMatch(@"/Resources \d+ 0 R/ToUnicode");
    }

    /// <summary>The text layer is untouched, so the character still extracts and still searches.</summary>
    /// <remarks>
    /// The whole reason for putting the bitmap inside a font rather than beside the text as a
    /// picture. The content stream still shows a code against a font resource and the font still
    /// carries a <c>ToUnicode</c> saying what that code means.
    /// </remarks>
    [Fact]
    public void TheCharacterIsStillExtractable()
    {
        PdfFile pdf = Write();

        pdf.ContentStreams().ShouldHaveSingleItem().ShouldContain("Tj");
        pdf.ToUnicode("F1").Values.ShouldContain(char.ConvertFromUtf32(Tick));
    }

    /// <summary>
    /// The advance is the face's own, so the pen moves as the layout that placed it decided.
    /// </summary>
    /// <remarks>
    /// Colour or not, the width array is what a reader advances the pen by, and it has to agree
    /// with the <c>d0</c> in the char proc — PDF 1.7 §9.6.5 makes the char proc's own declaration
    /// authoritative, so the two disagreeing would move every glyph after this one.
    /// </remarks>
    [Fact]
    public void TheWidthArrayAgreesWithTheCharProc()
    {
        PdfFile pdf = Write();

        Match widths = Widths().Match(pdf.Text);
        widths.Success.ShouldBeTrue();

        double stated = Number(CharProc().Match(pdf.Text).Groups[1]);
        double[] declared = [.. widths.Groups[1].Value
            .Split(' ', StringSplitOptions.RemoveEmptyEntries)
            .Select(v => double.Parse(v, CultureInfo.InvariantCulture))];

        declared.Length.ShouldBe(2, "notdef and the one glyph drawn");
        declared[1].ShouldBe(stated, 0.0002);
    }

    /// <summary>An ordinary outline face is untouched by any of this.</summary>
    /// <remarks>
    /// The negative control, and it is worth having: the Type 3 branch is chosen from the face's
    /// tables, so a face with no strikes must still reach the file as the simple TrueType font with
    /// an embedded program that every other document depends on.
    /// </remarks>
    [Fact]
    public void AnOutlineFaceIsStillASimpleTrueTypeFont()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable face on this machine");

        PdfFile pdf = Write(TestFace.Run("Ax", new DocPoint(Points(56.7), Points(200)), Points(24)));

        pdf.Text.ShouldContain("/Subtype/TrueType");
        pdf.Text.ShouldNotContain("/Subtype/Type3");
        pdf.FontPrograms().ShouldNotBeEmpty();
    }

    private static PdfFile Write(params GlyphRun[] runs)
    {
        if (runs.Length == 0)
        {
            Assert.SkipUnless(TestColourFace.IsAvailable, "no colour bitmap face on this machine");
            runs = [TestColourFace.Run(Tick, new DocPoint(Points(56.7), Points(200)), Points(24))];
        }

        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(
                DrawnPage.A4,
                sink =>
                {
                    foreach (GlyphRun run in runs) sink.DrawGlyphRun(run, Paint.Solid(Colour.Black));
                })),
            buffer);

        return PdfFile.Parse(buffer.ToArray());
    }

    private static double Number(Group group)
        => double.Parse(group.Value, CultureInfo.InvariantCulture);

    private static string Truncated(string value)
        => value.Length <= 400 ? value : value[..400];

    private static Length Points(double value) => Length.FromPoints(value);

    [GeneratedRegex(@"(-?[0-9.]+) 0 d0\nq (-?[0-9.]+) 0 0 (-?[0-9.]+) (-?[0-9.]+) (-?[0-9.]+) cm /Im\d+ Do Q")]
    private static partial Regex CharProc();

    [GeneratedRegex(@"/FirstChar 0/LastChar \d+/Widths\[([^\]]*)\]")]
    private static partial Regex Widths();
}

/// <summary>
/// What the rasteriser puts on the page for a colour bitmap glyph.
/// </summary>
/// <remarks>
/// <para>
/// The other half of the same defect, and the reason it is worth its own suite: the two backends
/// draw one display list, so a strike drawn by the PDF writer and not by the rasteriser — or drawn
/// by both in two different places — is a divergence between our own outputs before it is a
/// divergence from anyone else's.
/// </para>
/// <para>
/// The assertion is where the ink is, not how much: it must land inside the box the strike's own
/// metrics put it in, and that box is computed here from
/// <see cref="ColourBitmap.PlacementIn(int)"/> — the same call the PDF char proc is written from.
/// </para>
/// </remarks>
public sealed class RasterColourGlyphTests
{
    private const int Tick = 0x2714;

    /// <summary>The glyph is drawn, in colour, inside the box its own strike states.</summary>
    [Fact]
    public void AColourGlyphIsDrawnWhereItsStrikeSaysItGoes()
    {
        Assert.SkipUnless(TestColourFace.IsAvailable, "no colour bitmap face on this machine");

        const double dpi = 100;
        Length size = Length.FromPoints(24);
        DocPoint origin = new(Length.FromPoints(56.7), Length.FromPoints(100));

        using SKBitmap page = new RasterRenderer(new RasterRenderOptions { Dpi = dpi })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink => sink.DrawGlyphRun(
                    TestColourFace.Run(Tick, origin, size), Paint.Solid(Colour.Black))));

        OpenTypeFace face = TestColourFace.Face;
        ColourBitmap bitmap = ColourBitmaps
            .Of(face, face.Characters.GlyphFor(Tick), (int)Math.Round(size.Points * dpi / 72))
            .ShouldNotBeNull();

        (int left, int bottom, int width, int height) = bitmap.PlacementIn(face.UnitsPerEm);
        double unit = size.Points * dpi / 72 / face.UnitsPerEm;

        // The pixel box the strike asks for, grown by a pixel each way for the resampling edge.
        int x0 = (int)Math.Floor((origin.X.Points * dpi / 72) + (left * unit)) - 1;
        int y0 = (int)Math.Floor((origin.Y.Points * dpi / 72) - ((bottom + height) * unit)) - 1;
        int x1 = (int)Math.Ceiling(x0 + 2 + (width * unit)) + 1;
        int y1 = (int)Math.Ceiling(y0 + 2 + (height * unit)) + 1;

        int inside = 0, outside = 0, coloured = 0;

        for (int y = 0; y < page.Height; y++)
        {
            for (int x = 0; x < page.Width; x++)
            {
                SKColor pixel = page.GetPixel(x, y);
                if (pixel.Red > 245 && pixel.Green > 245 && pixel.Blue > 245) continue;

                if (x >= x0 && x <= x1 && y >= y0 && y <= y1) inside++; else outside++;

                // A monochrome outline would come out in the run's own paint, which is black.
                if (Math.Abs(pixel.Red - pixel.Green) > 12 || Math.Abs(pixel.Green - pixel.Blue) > 12)
                {
                    coloured++;
                }
            }
        }

        inside.ShouldBeGreaterThan(100, "the strike drew nothing at all");
        outside.ShouldBe(0, "every mark belongs inside the box the strike's metrics state");
        coloured.ShouldBeGreaterThan(20, "a colour strike is not the run's own paint");
    }

    /// <summary>Nothing about an ordinary outline face changes.</summary>
    [Fact]
    public void AnOutlineFaceIsStillDrawnFromItsOutlines()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable face on this machine");

        using SKBitmap page = new RasterRenderer(new RasterRenderOptions { Dpi = 100 })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink => sink.DrawGlyphRun(
                    TestFace.Run(
                        "Hamburgefonstiv",
                        new DocPoint(Length.FromPoints(56.7), Length.FromPoints(100)),
                        Length.FromPoints(24)),
                    Paint.Solid(Colour.Black))));

        int ink = 0;
        for (int y = 0; y < page.Height; y++)
        {
            for (int x = 0; x < page.Width; x++)
            {
                SKColor pixel = page.GetPixel(x, y);
                if (pixel.Red < 200 && pixel.Green < 200 && pixel.Blue < 200) ink++;
            }
        }

        ink.ShouldBeGreaterThan(500);
    }
}
