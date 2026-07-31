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
/// What the PDF writer puts in the file, checked against what it was asked to draw.
/// </summary>
/// <remarks>
/// The companion to the operator-for-operator comparison against LibreOffice in
/// <c>Paperless.Fidelity.Tests</c>. That one answers "does this agree with the reference";
/// this one answers "does this agree with its own input", which is the question that has to
/// be settled first — a backend that draws a rectangle a point to the left of where it was
/// told would show up there as a layout disagreement and be chased in the wrong library.
/// </remarks>
public sealed class PdfWriterTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void APdfHasAHeaderACrossReferenceAndATrailer()
    {
        PdfFile pdf = Write(new DrawnPage(DrawnPage.A4, sink => { }));

        pdf.Text.ShouldStartWith("%PDF-1.7\n");
        pdf.Text.ShouldContain("\nxref\n");
        pdf.Text.ShouldContain("/Root ");
        pdf.Text.ShouldContain("startxref\n");
        pdf.Text.ShouldEndWith("%%EOF\n");

        // Every object's offset has to be in the table, or a reader that resolves by number
        // rather than by scanning finds nothing.
        Match table = Regex.Match(pdf.Text, @"xref\n0 (\d+)\n");
        table.Success.ShouldBeTrue();

        int declared = int.Parse(table.Groups[1].Value, CultureInfo.InvariantCulture);
        Regex.Count(pdf.Text, @"^\d{10} \d{5} [nf] $", RegexOptions.Multiline)
            .ShouldBe(declared, "one cross-reference entry per object, plus the free head");
    }

    [Fact]
    public void EveryPageTakesTheSheetSizeTheLayoutReports()
    {
        DocSize a5 = new(Length.FromMillimetres(148), Length.FromMillimetres(210));
        PdfFile pdf = Write(
            new DrawnPage(DrawnPage.A4, sink => { }),
            new DrawnPage(a5, sink => { }) { Index = 1 });

        List<(double Width, double Height)> sizes = pdf.PageSizes();
        sizes.Count.ShouldBe(2);

        // A backend that allocated one box for the document would put the second page on the
        // first's sheet, which is what a section with its own page setup looks like when it is
        // dropped.
        sizes[0].Width.ShouldBe(DrawnPage.A4.Width.Points, 0.001);
        sizes[0].Height.ShouldBe(DrawnPage.A4.Height.Points, 0.001);
        sizes[1].Width.ShouldBe(a5.Width.Points, 0.001);
        sizes[1].Height.ShouldBe(a5.Height.Points, 0.001);
    }

    [Fact]
    public void TheSameDocumentWrittenTwiceIsTheSameBytes()
    {
        static IPage Page() => new DrawnPage(
            DrawnPage.A4,
            sink =>
            {
                sink.FillPath(Rectangle(100, 200, 50, 20), Paint.Solid(Colour.Black));
                sink.StrokePath(Line(10, 20, 300, 20), new Stroke(Paint.Solid(Colour.Black), Points(0.5)));
            });

        // Determinism is what makes a checksum meaningful. The creation date is the only field
        // that varies by itself, which is why it can be pinned.
        Bytes(Page()).ShouldBe(Bytes(Page()));
    }

    [Fact]
    public void AFilledRectangleIsWrittenAsARectangleAtTheRightPlace()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4, sink => sink.FillPath(Rectangle(72, 100, 144, 36), Paint.Solid(Colour.Black))));

        string content = pdf.ContentStreams().Single();

        // `re` rather than four line segments, because that is how every PDF spells a rectangle
        // and it is the spelling the fidelity harness's fill reader looks for.
        Match rectangle = Regex.Match(content, @"([-0-9.]+) ([-0-9.]+) ([-0-9.]+) ([-0-9.]+) re\nf\n");
        rectangle.Success.ShouldBeTrue($"expected a filled rectangle in:\n{content}");

        double[] numbers = [.. rectangle.Groups.Cast<Group>().Skip(1)
            .Select(g => double.Parse(g.Value, CultureInfo.InvariantCulture))];

        // PDF's y grows upwards from the bottom of the sheet and a document's grows down from
        // the top, so the rectangle's bottom is the page height less its bottom edge.
        numbers[0].ShouldBe(72, 0.001);
        numbers[1].ShouldBe(DrawnPage.A4.Height.Points - 136, 0.001);
        numbers[2].ShouldBe(144, 0.001);
        numbers[3].ShouldBe(36, 0.001);
    }

    [Fact]
    public void AStrokeStatesItsPenWidthImmediatelyBeforeItsPath()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.StrokePath(
                Line(56.7, 200, 538.6, 200), new Stroke(Paint.Solid(Colour.Black), Points(0.5)))));

        string content = pdf.ContentStreams().Single();

        // The order matters and is not a matter of taste: the harness's stroke reader pairs a
        // line with the pen width stated just before it, allowing nothing containing an `m` or
        // an `S` in between. A writer that set the width once at the top of the page would
        // produce strokes with no thickness as far as any comparison is concerned.
        Match stroke = Regex.Match(
            content, @"([\d.]+) w\n[^mS]*?([-0-9.]+) ([-0-9.]+) m\n([-0-9.]+) ([-0-9.]+) l\nS\n");

        stroke.Success.ShouldBeTrue($"expected a stroked line in:\n{content}");
        double.Parse(stroke.Groups[1].Value, CultureInfo.InvariantCulture).ShouldBe(0.5, 0.001);
        double.Parse(stroke.Groups[2].Value, CultureInfo.InvariantCulture).ShouldBe(56.7, 0.001);
        double.Parse(stroke.Groups[4].Value, CultureInfo.InvariantCulture).ShouldBe(538.6, 0.001);
    }

    [Fact]
    public void AHairlineKeepsItsZeroWidth()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.StrokePath(Line(10, 10, 100, 10), new Stroke(Paint.Solid(Colour.Black), Length.Zero))));

        // Zero means "the thinnest the device can draw" in the office formats and it means the
        // same in PDF, so it passes straight through. A writer that treated it as invisible, or
        // substituted a nominal width, would be wrong in opposite directions on paper and screen.
        pdf.ContentStreams().Single().ShouldContain("0 w\n");
    }

    [Fact]
    public void ATransparentPaintDrawsNothing()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.FillPath(Rectangle(0, 0, 100, 100), Paint.Solid(Colour.Transparent))));

        // The page is still there — a blank page is a page — and its content stream holds nothing.
        pdf.PageSizes().Count.ShouldBe(1);
        pdf.ContentStreams().Single().ShouldBeEmpty();
    }

    [Fact]
    public void AHalfTransparentFillNamesAGraphicsState()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink =>
            {
                sink.FillPath(Rectangle(0, 0, 100, 100), Paint.Solid(Colour.Black.WithAlpha(128)));
                sink.FillPath(Rectangle(0, 200, 100, 100), Paint.Solid(Colour.Black.WithAlpha(128)));
            }));

        pdf.ContentStreams().Single().ShouldContain("/GS1 gs\n");
        pdf.Text.ShouldContain("/Type/ExtGState/ca 0.502");

        // One state object for two fills of the same opacity: a shaded table with a hundred rows
        // would otherwise carry a hundred identical dictionaries.
        Regex.Count(pdf.Text, "/Type/ExtGState").ShouldBe(1);
    }

    [Fact]
    public void AGlyphRunIsShownAtItsOwnBaseline()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        DocPoint origin = new(Points(56.7), Points(120.25));
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.DrawGlyphRun(
                TestFace.Run("Paperless", origin, Points(11)), Paint.Solid(Colour.Black))));

        string content = pdf.ContentStreams().Single();
        Match placed = Regex.Match(content, @"BT\n([-0-9.]+) ([-0-9.]+) Td\n/(F\d+) ([\d.]+) Tf\n");

        placed.Success.ShouldBeTrue($"expected a positioned text block in:\n{content}");

        // The run's origin is the start of the baseline, not the top of a box, and it reaches the
        // file untouched apart from the flip. This is the number the whole comparison against
        // LibreOffice's content stream rests on.
        double.Parse(placed.Groups[1].Value, CultureInfo.InvariantCulture).ShouldBe(56.7, 0.001);
        double.Parse(placed.Groups[2].Value, CultureInfo.InvariantCulture)
            .ShouldBe(DrawnPage.A4.Height.Points - 120.25, 0.001);
        double.Parse(placed.Groups[4].Value, CultureInfo.InvariantCulture).ShouldBe(11, 0.001);
    }

    [Fact]
    public void AnUnjustifiedRunIsOneShowOperator()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.DrawGlyphRun(
                TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11)),
                Paint.Solid(Colour.Black))));

        // Nothing to correct, so nothing is written: the font's own widths carry the pen and the
        // run is one hex string. The same shape LibreOffice writes for a line it did not stretch.
        pdf.ContentStreams().Single().ShouldMatch(@"<[0-9A-F]+>Tj\n");
    }

    [Fact]
    public void AJustifiedRunPlacesEveryGlyphWhereTheLayoutPutIt()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // Two twips added to every blank, which is what justification does to a line: the run's
        // advances then disagree with the font's, and the file has to say so or the words after
        // the first space land in the wrong place.
        GlyphRun run = TestFace.Run(
            "a wider line of text", new DocPoint(Points(56.7), Points(120)), Points(11),
            Length.FromTwips(2));

        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4, sink => sink.DrawGlyphRun(run, Paint.Solid(Colour.Black))));

        string content = pdf.ContentStreams().Single();
        content.Contains("]TJ\n", StringComparison.Ordinal)
            .ShouldBeTrue("a stretched line needs its pen corrected");

        // Replay the show operator the way a reader does — advance by the stated width, move back
        // by each adjustment — and check the pen lands on each glyph's own offset. This is the
        // assertion that makes the whole positioning scheme trustworthy rather than plausible.
        Dictionary<byte, double> widths = Widths(pdf);
        double pen = 56.7;
        double size = 11;
        int glyph = 0;

        foreach (Match item in Regex.Matches(
                     Regex.Match(content, @"\[(.*?)\]TJ", RegexOptions.Singleline).Groups[1].Value,
                     @"<([0-9A-F]+)>|(-?[\d.]+)"))
        {
            if (item.Groups[2].Success)
            {
                pen -= double.Parse(item.Groups[2].Value, CultureInfo.InvariantCulture) * size / 1000.0;
                continue;
            }

            string hex = item.Groups[1].Value;
            for (int i = 0; i < hex.Length; i += 2)
            {
                byte code = byte.Parse(
                    hex.AsSpan(i, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);

                double wanted = (run.Origin.X + run.Glyphs[glyph].Offset.X).Points;
                Math.Abs(pen - wanted).ShouldBeLessThanOrEqualTo(
                    0.005,
                    $"glyph {glyph + 1} is shown at {pen:F4} pt and belongs at {wanted:F4} pt");

                pen += widths.GetValueOrDefault(code) * size / 1000.0;
                glyph++;
            }
        }

        glyph.ShouldBe(run.Glyphs.Count, "every glyph of the run should be shown");
    }

    [Fact]
    public void AnImageIsWrittenAsAnXObjectScaledToItsDestination()
    {
        byte[] pixels = new byte[2 * 2 * 4];
        for (int i = 0; i < 4; i++)
        {
            pixels[(i * 4) + 0] = 200;
            pixels[(i * 4) + 3] = 255;
        }

        RasterImage image = new() { Width = 2, Height = 2, Pixels = pixels };
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.DrawImage(
                image,
                new DocRect(Points(72), Points(100), Points(144), Points(72)))));

        string content = pdf.ContentStreams().Single();

        // An image XObject draws into the unit square, so the placement is entirely in the
        // transform: width and height on the diagonal, bottom-left corner as the offset.
        content.ShouldContain(
            $"144 0 0 72 72 {PdfNumber(DrawnPage.A4.Height.Points - 172)} cm\n");
        content.ShouldContain("/Im1 Do\n");
        pdf.Text.ShouldContain("/Subtype/Image/Width 2/Height 2/ColorSpace/DeviceRGB/BitsPerComponent 8");
    }

    [Fact]
    public void AJpegIsPassedThroughRatherThanReEncoded()
    {
        byte[] jpeg = [0xFF, 0xD8, 0xFF, 0xE0, 0, 16, .. new byte[40], 0xFF, 0xD9];
        RasterImage image = new()
        {
            Width = 8,
            Height = 8,
            Pixels = new byte[8 * 8 * 4],
            EncodedBytes = jpeg,
            EncodedMediaType = "image/jpeg",
        };

        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink => sink.DrawImage(image, new DocRect(Points(0), Points(0), Points(72), Points(72)))));

        // PDF's DCTDecode filter and JPEG are the same thing, so re-encoding the pixels would
        // spend time to produce a larger and worse image.
        pdf.Text.ShouldContain("/Filter/DCTDecode");
        pdf.Streams().ShouldContain(s => !s.Deflated && s.Data.Length == jpeg.Length);
    }

    [Fact]
    public void ATransparencyGroupIsCompositedInOneStep()
    {
        PdfFile pdf = Write(new DrawnPage(
            DrawnPage.A4,
            sink =>
            {
                sink.BeginTransparencyGroup(0.5);
                sink.FillPath(Rectangle(0, 0, 100, 100), Paint.Solid(Colour.Black));
                sink.FillPath(Rectangle(50, 50, 100, 100), Paint.Solid(Colour.Black));
                sink.EndTransparencyGroup();
            }));

        // A real group rather than a constant alpha on each member, because the two differ:
        // where the two rectangles overlap, a group at half opacity shows one solid half-tone
        // and two half-opacity fills show a darker patch.
        pdf.Text.ShouldContain("/Subtype/Form");
        pdf.Text.ShouldContain("/Group<</Type/Group/S/Transparency/CS/DeviceRGB>>");
        pdf.ContentStreams().ShouldContain(c => c.Contains("/Fm1 Do\n", StringComparison.Ordinal));
    }

    [Fact]
    public void APageThatUnbalancesItsStateStackDoesNotSwallowTheNextOne()
    {
        PdfFile pdf = Write(
            new DrawnPage(DrawnPage.A4, sink => { sink.Save(); sink.Save(); }),
            new DrawnPage(DrawnPage.A4, sink => sink.FillPath(
                Rectangle(0, 0, 10, 10), Paint.Solid(Colour.Black))) { Index = 1 });

        // Two `q` with no `Q` is a bug in whatever drew the page, and the honest response is to
        // close them at the page boundary rather than to let the clip and transform leak into
        // every page after it.
        pdf.PageSizes().Count.ShouldBe(2);
        pdf.ContentStreams()[0].TrimEnd('\n').EndsWith('Q').ShouldBeTrue();
    }

    // ------------------------------------------------------------------------- the machinery

    private static PdfFile Write(params IPage[] pages) => PdfFile.Parse(Bytes(pages));

    private static byte[] Bytes(params IPage[] pages)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(new DrawnPages(pages), buffer);
        return buffer.ToArray();
    }

    private static Length Points(double value) => Length.FromPoints(value);

    private static string PdfNumber(double value)
        => Math.Round(value, 4).ToString("0.####", CultureInfo.InvariantCulture);

    private static GraphicsPath Rectangle(double x, double y, double width, double height)
        => GraphicsPath.Rectangle(new DocRect(Points(x), Points(y), Points(width), Points(height)));

    private static GraphicsPath Line(double x1, double y1, double x2, double y2)
        => new GraphicsPath()
            .MoveTo(new DocPoint(Points(x1), Points(y1)))
            .LineTo(new DocPoint(Points(x2), Points(y2)));

    /// <summary>The <c>/Widths</c> array of the file's first font, by code.</summary>
    private static Dictionary<byte, double> Widths(PdfFile pdf)
    {
        Match array = Regex.Match(pdf.Text, @"/FirstChar (\d+)/LastChar \d+/Widths\[([^\]]*)\]");
        array.Success.ShouldBeTrue("the font should state its widths");

        int first = int.Parse(array.Groups[1].Value, CultureInfo.InvariantCulture);
        Dictionary<byte, double> widths = [];

        string[] values = array.Groups[2].Value.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        for (int i = 0; i < values.Length; i++)
        {
            widths[(byte)(first + i)] = double.Parse(values[i], CultureInfo.InvariantCulture);
        }

        return widths;
    }
}
