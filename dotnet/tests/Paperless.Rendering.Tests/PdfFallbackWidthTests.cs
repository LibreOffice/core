using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Pdf;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// What the writer states as a glyph's width when it cannot read the face's file.
/// </summary>
/// <remarks>
/// <para>
/// A <see cref="FontReference.FaceKey"/> is a path, or <c>path#index</c> for one face of a
/// collection, and <c>FileFontProvider</c> reads it as one. A caller that builds a reference
/// by hand and puts the family name there instead gets a face that never loads — which is
/// documented as harmless, since "a PDF that names a font without embedding it still holds
/// the right text at the right pen positions".
/// </para>
/// <para>
/// It was not harmless. With no <c>hmtx</c> to read, every <c>/Widths</c> entry was zero, so
/// every glyph sat a whole advance from where the stated widths put the pen and the content
/// stream corrected each one with a <c>TJ</c> adjustment of the entire advance. Measured on
/// <c>tests/corpus/features/sheet-print-xlsx.xlsx</c>, whose layout names its face this way:
/// adjustments of <c>-722</c> and <c>-556</c> thousandths between adjacent glyphs, and
/// <c>pdftotext</c> reporting <b>13255</b> words over fourteen pages against LibreOffice's
/// <b>2281</b> — one word per character, because an adjustment that large is how a PDF
/// spells a word break. The pages looked right, which is why the operator-for-operator
/// comparison never saw it.
/// </para>
/// <para>
/// So the display list's own advances are the fallback, and these tests pin both halves: the
/// widths are stated, and the run therefore needs no per-glyph correction. With the fallback
/// in place that file extracts as 2281 words, exactly LibreOffice's count.
/// </para>
/// </remarks>
public sealed class PdfFallbackWidthTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void AFaceWhoseFileCannotBeReadStatesTheAdvancesTheRunCarries()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = Unloadable(TestFace.Run("Total", new DocPoint(Points(56.7), Points(120)), Points(11)));
        PdfFile pdf = Write(run);

        pdf.FontPrograms().ShouldBeEmpty("a face whose key is not a path cannot be embedded");

        List<double> widths = Widths(pdf);
        widths.Count.ShouldBe(run.Glyphs.Select(g => g.GlyphId).Distinct().Count() + 1);

        // Index 0 is .notdef, which nothing drew and which therefore has no measured advance.
        widths[0].ShouldBe(0);
        widths.Skip(1).ShouldAllBe(width => width > 0);
    }

    [Fact]
    public void TheStatedWidthsAreTheAdvancesLayoutAskedFor()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = Unloadable(TestFace.Run("Total", new DocPoint(Points(56.7), Points(120)), Points(11)));
        PdfFile pdf = Write(run);

        List<double> widths = Widths(pdf);

        // Codes are allocated in the order glyphs are first drawn, so the first glyph of the run
        // is code 1. A width that merely happened to be non-zero would pass the test above; this
        // one says it is the right number, in thousandths of the em the run was measured at.
        double expected = Math.Round(
            run.Glyphs[0].Advance.Emu * 1000.0 / run.FontSize.Emu, 4, MidpointRounding.AwayFromZero);

        widths[1].ShouldBe(expected, 0.0001);
    }

    [Fact]
    public void AnUnjustifiedRunNeedsNoPerGlyphCorrection()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = Unloadable(TestFace.Run(
            "Total revenue", new DocPoint(Points(56.7), Points(120)), Points(11)));

        string content = Write(run).ContentStreams().ShouldHaveSingleItem();

        // One Tj and no TJ array is the shape an unjustified line takes when the widths are
        // right — the same shape the word-processing path already produces and the same one
        // LibreOffice writes. A TJ here would mean the pen had drifted from where layout put
        // it, which is exactly the state that broke word segmentation.
        content.ShouldContain(">Tj");
        content.ShouldNotContain("]TJ");
    }

    /// <summary>
    /// The same run, re-pointed at a face reference whose key names a family rather than a file.
    /// </summary>
    /// <remarks>
    /// Exactly what <c>SheetText.Describe</c>, <c>SlideTextLayout.Reference</c> and
    /// <c>PageDrawing.Reference</c> build. Reproduced here rather than reached through one of
    /// them so that the writer's behaviour is pinned even after those callers are corrected.
    /// </remarks>
    private static GlyphRun Unloadable(GlyphRun run) => run with
    {
        Font = new FontReference
        {
            FamilyName = run.Font.FamilyName,
            Weight = run.Font.Weight,
            IsItalic = run.Font.IsItalic,
            FaceKey = run.Font.FamilyName,
        },
    };

    private static List<double> Widths(PdfFile pdf)
    {
        Match match = Regex.Match(pdf.Text, @"/Widths\[([^\]]*)\]");
        match.Success.ShouldBeTrue("the font object must state a /Widths array");

        return [.. match.Groups[1].Value
            .Split(' ', StringSplitOptions.RemoveEmptyEntries)
            .Select(value => double.Parse(value, CultureInfo.InvariantCulture))];
    }

    private static PdfFile Write(GlyphRun run)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(
                DrawnPage.A4, sink => sink.DrawGlyphRun(run, Paint.Solid(Colour.Black)))),
            buffer);

        return PdfFile.Parse(buffer.ToArray());
    }

    private static Length Points(double value) => Length.FromPoints(value);
}
