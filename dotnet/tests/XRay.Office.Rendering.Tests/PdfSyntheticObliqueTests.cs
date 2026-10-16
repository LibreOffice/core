using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// A run whose italic went unmet leans, and it leans by shearing its text matrix rather than by
/// moving anything.
/// </summary>
/// <remarks>
/// <para>
/// The reference decides this at
/// <c>LogicalFontInstance::NeedsArtificialItalic()</c> — the request is italic, the face is not —
/// and carries it out in <c>drawHorizontalGlyphs</c>, which uses <c>Td</c> only when the angle,
/// the x-scale and the skew are all zero and a <c>Tm</c> otherwise
/// (<c>vcl/source/pdf/pdfwriter_impl.cxx:5767-5787</c>).
/// </para>
/// <para>
/// Measured over the 302-document slides corpus at the round-54 tree: the reference writes
/// <b>587</b> sheared text matrices and we wrote <b>0</b>, over 157 pages. Over the other two
/// tracks' reference renderings it is 5420 blocks on words and 464 on sheets — 6471 in all, none
/// of which we drew. The families it fires on here are the ones with no italic installed: DejaVu
/// Sans, DejaVu Serif, and everything that substitutes onto them.
/// </para>
/// </remarks>
public sealed class PdfSyntheticObliqueTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void AnUnmetItalicIsDrawnAsAShearedTextMatrix()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string content = ContentOf(sink =>
            sink.DrawGlyphRun(Run(oblique: true), Paint.Solid(Colour.Black)));

        // 0.3463 is the four decimals a PDF number carries of `tan(ARTIFICIAL_ITALIC_SKEW)`; the
        // reference writes ten of them and every one of the corpus's 587 occurrences reads
        // 0.3462535606. `b` stays zero: this is a shear, never a rotation, which is exactly the
        // distinction the round-54 rotation census got wrong.
        content.ShouldContain("1 0 0.3463 1 ");
        content.ShouldContain(" Tm\n");
        content.ShouldNotContain(" Td\n");
    }

    [Fact]
    public void AMetItalicKeepsThePlainPenMove()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string content = ContentOf(sink =>
            sink.DrawGlyphRun(Run(oblique: false), Paint.Solid(Colour.Black)));

        // The control, and it is doing real work: it says the shear above comes from the flag and
        // not from anything else this sink does to a run.
        content.ShouldContain(" Td\n");
        content.ShouldNotContain("0.3463");
    }

    [Fact]
    public void TheShearMovesNoPenAndNoAdvance()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        string upright = ContentOf(sink =>
            sink.DrawGlyphRun(Run(oblique: false), Paint.Solid(Colour.Black)));
        string leaning = ContentOf(sink =>
            sink.DrawGlyphRun(Run(oblique: true), Paint.Solid(Colour.Black)));

        // This is the invariant the whole change rests on, and it is why nothing reflows: the
        // reference passes the same slant to HarfBuzz through `hb_font_set_synthetic_slant`, which
        // moves outlines and leaves advances alone. Measured on an authored deck through 26.2.4.2 —
        // the roman and italic halves of a DejaVu Sans line carry the same TJ array and the same
        // origin at 12, 24 and 40 pt. So the two streams differ in the positioning operator and in
        // nothing after it.
        Show(upright).ShouldBe(Show(leaning));

        // And the origin the shear is taken about is the one the pen would have had.
        leaning.ShouldContain($"1 0 0.3463 1 {Origin(upright)} Tm\n");
    }

    /// <summary>Everything a content stream says after its last positioning operator.</summary>
    private static string Show(string content)
    {
        int tm = content.LastIndexOf(" Tm\n", StringComparison.Ordinal);
        int td = content.LastIndexOf(" Td\n", StringComparison.Ordinal);
        int at = Math.Max(tm, td);
        return at < 0 ? content : content[(at + 4)..];
    }

    /// <summary>The two numbers of an upright stream's first <c>Td</c>.</summary>
    private static string Origin(string content)
    {
        int td = content.IndexOf(" Td\n", StringComparison.Ordinal);
        int start = content.LastIndexOf('\n', td) + 1;
        return content[start..td];
    }

    private static GlyphRun Run(bool oblique)
    {
        GlyphRun run = TestFace.Run(
            "Hxbdp",
            new DocPoint(Length.FromPoints(72), Length.FromPoints(200)),
            Length.FromPoints(24));

        return run with { Font = run.Font with { SyntheticOblique = oblique } };
    }

    private static string ContentOf(Action<IDrawingSink> draw)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(DrawnPage.A4, draw)), buffer);

        return PdfFile.Parse(buffer.ToArray()).ContentStreams().Single();
    }
}
