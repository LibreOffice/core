using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Pdf;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// What a written PDF calls the faces it embeds.
/// </summary>
/// <remarks>
/// <para>
/// <c>/BaseFont</c> names a <em>face</em>, not a family (PDF 1.7 §9.6.2.1). Taking it from the
/// family name instead announces a document's regular and bold faces under one name, and the
/// reason that is worth a test of its own is how completely invisible it is to everything else:
/// the glyphs, the widths, the pagination and the extracted text are all correct, so no page
/// count, word count or pixel comparison can see it.
/// </para>
/// <para>
/// Measured on <c>sheets/batch-005/…/Praktikastellen_-_chinesischsprachiger_Kulturraum.xls</c>,
/// which passes every gate the corpus harness has — 34 pages against 34, 1828 words against
/// 1828, both faces embedded. Ours named <c>Carlito</c> twice where LibreOffice's own export
/// names <c>Carlito-Bold</c> and <c>Carlito-Regular</c>, and an operator-level diff read that
/// as 81 records drawn in the wrong face on a document that is pixel-identical to the
/// reference.
/// </para>
/// </remarks>
public sealed partial class PdfBaseFontNameTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    /// <summary>
    /// The regression: two weights of one family must not arrive under one name.
    /// </summary>
    /// <remarks>
    /// This is the assertion the family-name spelling fails. It deliberately says nothing about
    /// what the names are — only that a reader can tell the two apart — because that is the
    /// property a consumer of the file actually depends on.
    /// </remarks>
    [Fact]
    public void TwoWeightsOfOneFamilyAreNamedApart()
    {
        Assert.SkipUnless(TestFacePair.IsAvailable, "no family with two distinct weights on this machine");

        PdfFile pdf = Write(
            TestFacePair.Run(TestFacePair.Regular, "Regular", new DocPoint(Points(56.7), Points(200)), Points(11)),
            TestFacePair.Run(TestFacePair.Bold, "Bold", new DocPoint(Points(56.7), Points(180)), Points(11)));

        List<string> names = BaseFontNames(pdf);

        names.Count.ShouldBe(2, "one font per face");
        names.Distinct(StringComparer.Ordinal).Count()
            .ShouldBe(2, $"the two faces must be named apart, not both as the family: {string.Join(", ", names)}");
    }

    /// <summary>
    /// The name is the face's own PostScript name, which is what LibreOffice's export writes.
    /// </summary>
    [Fact]
    public void ABoldFaceIsNamedByItsPostScriptName()
    {
        Assert.SkipUnless(TestFacePair.IsAvailable, "no family with two distinct weights on this machine");

        PdfFile pdf = Write(
            TestFacePair.Run(TestFacePair.Bold, "Bold", new DocPoint(Points(56.7), Points(180)), Points(11)));

        string expected = Sanitised(TestFacePair.Bold.Face.PostScriptName!);

        BaseFontNames(pdf).ShouldHaveSingleItem().ShouldBe(expected);
    }

    /// <summary>
    /// The subset prefix is still there and still six upper-case letters and a plus.
    /// </summary>
    /// <remarks>
    /// Worth pinning beside the name itself: the prefix is how a reader knows the program is a
    /// subset (PDF 1.7 §9.6.4), and it is written by the same expression the name is.
    /// </remarks>
    [Fact]
    public void TheSubsetPrefixSurvivesTheNameChange()
    {
        Assert.SkipUnless(TestFacePair.IsAvailable, "no family with two distinct weights on this machine");

        PdfFile pdf = Write(
            TestFacePair.Run(TestFacePair.Bold, "Bold", new DocPoint(Points(56.7), Points(180)), Points(11)));

        TaggedBaseFont().Count(pdf.Text).ShouldBe(1);
    }

    /// <summary>
    /// The descriptor's <c>/FontName</c> agrees with the font's <c>/BaseFont</c>.
    /// </summary>
    /// <remarks>
    /// PDF 1.7 §9.8.1 requires them equal. They are written from one expression, so this is a
    /// guard against the two drifting apart rather than a separate claim.
    /// </remarks>
    [Fact]
    public void TheDescriptorAgreesWithTheFont()
    {
        Assert.SkipUnless(TestFacePair.IsAvailable, "no family with two distinct weights on this machine");

        PdfFile pdf = Write(
            TestFacePair.Run(TestFacePair.Regular, "Regular", new DocPoint(Points(56.7), Points(200)), Points(11)),
            TestFacePair.Run(TestFacePair.Bold, "Bold", new DocPoint(Points(56.7), Points(180)), Points(11)));

        List<string> fonts = BaseFontNames(pdf);
        List<string> descriptors = [.. FontName().Matches(pdf.Text).Select(m => m.Groups[1].Value)];

        descriptors.Order(StringComparer.Ordinal).ShouldBe(fonts.Order(StringComparer.Ordinal));
    }

    /// <summary>Every <c>/BaseFont</c> in the file, subset prefix stripped.</summary>
    private static List<string> BaseFontNames(PdfFile pdf)
        => [.. BaseFont().Matches(pdf.Text).Select(m => m.Groups[1].Value)];

    /// <summary>The writer's own name sanitising, so the expectation is not a second spelling of it.</summary>
    private static string Sanitised(string value)
        => new([.. value.Where(c =>
            c is (>= 'A' and <= 'Z') or (>= 'a' and <= 'z') or (>= '0' and <= '9') or '-')]);

    private static PdfFile Write(params GlyphRun[] runs)
    {
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

    private static Length Points(double value) => Length.FromPoints(value);

    [GeneratedRegex(@"/BaseFont/[A-Z]{6}\+([A-Za-z0-9-]+)")]
    private static partial Regex BaseFont();

    [GeneratedRegex(@"/BaseFont/[A-Z]{6}\+[A-Za-z0-9-]+")]
    private static partial Regex TaggedBaseFont();

    [GeneratedRegex(@"/FontName/[A-Z]{6}\+([A-Za-z0-9-]+)")]
    private static partial Regex FontName();
}
