using Paperless.Core.Charts;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A chart held by a word-processing frame: read, placed, and drawn.
/// </summary>
/// <remarks>
/// <para>
/// Until this landed, Writer was the one family that never drew a chart. Both readers already
/// existed and were family-blind by design — <c>DrawingChart</c> reads any <c>c:chartSpace</c> and
/// <c>OdfChart</c> any <c>chart:chart</c> — and what was missing was the two call sites and a third
/// case in <c>PageFrame</c> beside <c>Image</c> and <c>Vector</c>. So this file pins the seam
/// rather than the geometry, which <c>Paperless.Core.Tests</c> owns.
/// </para>
/// <para>
/// The whole-page comparison against LibreOffice is the render sweep, where the three
/// <c>chart-bar-text</c> rows are 41/41 each.
/// </para>
/// </remarks>
public class FrameChartTests
{
    /// <summary>The corpus's three Writer chart documents, one per route to the same chart.</summary>
    public static TheoryData<string> Documents =>
        ["chart-bar-text.odt", "chart-bar-text.docx", "chart-bar-text.fodt"];

    /// <summary>Words that belong to the chart rather than to the page's own sentences.</summary>
    private static readonly HashSet<string> ChartWords =
        ["Regional revenue", "Units", "Quarter", "North", "South", "Q1", "Q2", "Q3", "Q4", "180", "0"];

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheFrameCarriesAChartRatherThanAnEmptyRectangle(string name)
    {
        PageFrame frame = ChartFrames(name).ShouldHaveSingleItem();

        ChartPlot plot = frame.Chart.ShouldNotBeNull();
        plot.Title.ShouldBe("Regional revenue");
        plot.Series.Count.ShouldBe(2);
        plot.Categories.Count.ShouldBe(4);
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheChartWinsOverTheReplacementPictureBesideIt(string name)
    {
        // ODF lists a frame's children as alternatives in decreasing order of preference and writes
        // a chart as a `draw:object` followed by a `draw:image` of it — `chart-bar-text.odt` carries
        // 22 kB of `ObjectReplacements/Object 1`. Looking at the picture first is what recorded
        // every chart in every ODS as a plain picture and then painted nothing, all of those
        // streams being `VCLMTF`, which no decoder here reads.
        PageFrame frame = ChartFrames(name).ShouldHaveSingleItem();

        frame.IsImage.ShouldBeFalse();
        frame.Vector.ShouldBeNull();
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheFrameKeepsTheDocumentsOwnExtent(string name)
    {
        // 12 x 7 cm, stated on the `draw:frame` and on the `wp:extent`. The chart's own
        // `svg:width` says the same thing here and is not what decides: a Writer chart is composed
        // in the frame rather than rendered at its own size and stretched, which is the one place
        // this differs from a sheet's.
        PageFrame frame = ChartFrames(name).ShouldHaveSingleItem();

        frame.Size.Width.Millimetres.ShouldBe(120, 0.5);
        frame.Size.Height.Millimetres.ShouldBe(70, 0.5);
    }

    [Fact]
    public void ADocxChartNamesItsOwnFaceWhereAnOdtChartTakesTheOfficeDefault()
    {
        // The family cannot ride on `ChartPlot`, which carries type sizes and no face, so it sits
        // beside it — and it has to be read, because Writer is the family where the two vocabularies
        // disagree. Measured with `pdffonts` on LibreOffice's own PDFs of `chart2/qa/extras/data/`:
        // `odt/chart.odt` embeds Liberation Sans and `docx/chart.docx` embeds Carlito, from the same
        // chart, because an OOXML chart's text takes the theme's minor latin face (Calibri there)
        // and an ODF chart's takes the office default.
        //
        // This fixture states its own instead: LibreOffice's DOCX export writes `a:latin
        // typeface="Arial"` into the chart part, Arial being the metric-compatible name for the
        // Liberation Sans the ODF source was set in. Both PDFs then embed Liberation Sans, so the
        // reading is confirmed by what comes out rather than by the string.
        ChartFrames("chart-bar-text.docx").ShouldHaveSingleItem().ChartFontFamily.ShouldBe("Arial");

        // Null, meaning the drawing code's own default, which is Liberation Sans.
        ChartFrames("chart-bar-text.odt").ShouldHaveSingleItem().ChartFontFamily.ShouldBeNull();
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void DrawingTheChartPaintsItsBarsItsAxesAndItsLabels(string name)
    {
        // The end of the seam. A chart becomes ordinary fills, strokes and glyph runs, so nothing
        // in the display list knows a chart happened — the same arrangement a slide and a sheet
        // use, over one layout engine in `Paperless.Core.Charts`.
        RecordingSink sink = new();
        Draw(name, sink);

        // Ten value ticks (0…180), four category names, two legend keys, two axis titles and the
        // title itself. The count is a floor rather than an equality because the reference's own
        // label thinning is the chart engine's business, not this seam's.
        List<string> labels = [.. sink.Runs.Select(run => run.Text).Where(ChartWords.Contains)];

        labels.Count.ShouldBeGreaterThanOrEqualTo(10, $"{name}: labels");
        labels.ShouldContain("Regional revenue");
        labels.ShouldContain("180");
        labels.ShouldContain("Q4");
        labels.ShouldContain("North");

        // Eight bars, plus the wall. A chart drawing no fills at all is the shape an unresolved
        // series colour takes, and it reads as a data bug rather than a colour one.
        sink.Fills.ShouldBeGreaterThanOrEqualTo(9, $"{name}: filled marks");
        sink.Strokes.ShouldBeGreaterThan(0, $"{name}: axes and ticks");
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void EveryMarkLandsInsideTheFramesOwnRectangle(string name)
    {
        // The check that catches a chart composed about the page's origin rather than the frame's,
        // which draws a plausible picture in the top-left corner of the sheet whatever the frame
        // says. A quarter-point tolerance because a stroke is centred on its path.
        //
        // Over the paths only, and over the labels named below. The sink is fed the *whole* page,
        // and the page's two sentences sit above and below the frame quite legitimately — recording
        // every glyph run is what made the first version of this test fail 2.9 pt above the frame's
        // top, which is where the first paragraph's baseline is. The fixture's frame has no border
        // and its paragraphs are text, so every path on the page belongs to the chart.
        RecordingSink sink = new();
        DocRect area = Draw(name, sink);
        Length slack = Length.FromPoints(0.25);

        List<DocPoint> marks =
        [
            .. sink.Points,
            .. sink.Runs.Where(run => ChartWords.Contains(run.Text)).Select(run => run.At),
        ];

        marks.Count.ShouldBeGreaterThan(20, $"{name}: marks recorded");

        foreach (DocPoint point in marks)
        {
            point.X.ShouldBeGreaterThanOrEqualTo(area.X - slack, $"{name}: left of the frame");
            point.X.ShouldBeLessThanOrEqualTo(area.Right + slack, $"{name}: right of the frame");
            point.Y.ShouldBeGreaterThanOrEqualTo(area.Y - slack, $"{name}: above the frame");
            point.Y.ShouldBeLessThanOrEqualTo(area.Bottom + slack, $"{name}: below the frame");
        }
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void ExtractionReportsTheChartsNumbersAsATable(string name)
    {
        // The DOCX half of this was the missing call site: before it, `chart.docx` extracted to
        // nothing while the same chart as `chart.odt` extracted its whole table, because
        // `OdfContentReader`'s `draw:object` case already served ODT and nothing in the DOCX walk
        // looked at `a:graphicData/@uri`. Both now build the section `DrawingChart` and `OdfChart`
        // agree on, so a caller indexing a mixed corpus never branches on the family.
        using IDocument document = PaperlessDocument.Open(Corpus.Require(name));

        ContentTable table = Descendants(document.Content)
            .OfType<ContentTable>()
            .ShouldHaveSingleItem();

        table.HeaderRowCount.ShouldBe(1);

        List<ContentTableRow> rows = [.. table.Children.Cast<ContentTableRow>()];
        rows.Count.ShouldBe(5);
        rows[0].GetText().ShouldBe("\tNorth\tSouth\n");
        rows[4].GetText().ShouldBe("Q4\t168\t121\n");
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }

    /// <summary>Lays a document out and draws the page its chart landed on.</summary>
    /// <returns>The rectangle the chart's frame was placed at.</returns>
    private static DocRect Draw(string name, RecordingSink sink)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        WordProcessingPages pages = (WordProcessingPages)document.Layout();

        foreach (LaidOutPage page in pages.Pages)
        {
            if (page.Frames.All(placed => placed.Frame.Chart is null)) continue;

            PageDrawing.Draw(page, pages.Blocks, sink);
            return page.Frames.First(placed => placed.Frame.Chart is not null).Area;
        }

        throw new InvalidOperationException($"{name}: no page carries a chart.");
    }

    private static IReadOnlyList<PageFrame> ChartFrames(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        WordProcessingPages pages = (WordProcessingPages)document.Layout();

        return
        [
            .. pages.Pages
                .SelectMany(page => page.Frames)
                .Select(placed => placed.Frame)
                .Where(frame => frame.Chart is not null),
        ];
    }

    /// <summary>
    /// Counts what a page painted and remembers where.
    /// </summary>
    /// <remarks>
    /// Deliberately thin: what these tests assert is that a chart reached the sink at all and did
    /// so inside its frame. Which shade of blue a bar is and how tall it stands are
    /// <c>Paperless.Core.Charts</c>'s, and are measured against LibreOffice's PDF rather than here.
    /// </remarks>
    private sealed class RecordingSink : IDrawingSink
    {
        private int _depth;

        public List<(string Text, DocPoint At)> Runs { get; } = [];

        public List<DocPoint> Points { get; } = [];

        public int Fills { get; private set; }

        public int Strokes { get; private set; }

        public void BeginPage(DocSize size)
        {
        }

        public void EndPage()
        {
        }

        public void Save() => _depth++;

        public void Restore() => _depth--;

        public void Transform(AffineTransform transform)
        {
        }

        public void BeginTransparencyGroup(double opacity)
        {
        }

        public void EndTransparencyGroup()
        {
        }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero)
        {
        }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
        {
            Fills++;
            Record(path);
        }

        public void StrokePath(GraphicsPath path, Stroke stroke)
        {
            Strokes++;
            Record(path);
        }

        public void DrawGlyphRun(GlyphRun run, Paint paint)
        {
            // Nothing drawn under a transform, because its coordinates are that transform's and not
            // the page's. The only such run a chart draws is the value axis' rotated title, which is
            // laid out about the origin and turned on the sink's state stack — so its recorded
            // origin is -10.25 pt, ten points to the *left of the page*, and quite correct.
            if (_depth == 0) Runs.Add((run.Text, run.Origin));
        }

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
        {
        }

        private void Record(GraphicsPath path)
        {
            foreach (PathCommand command in path.Commands)
            {
                // A Close carries no coordinate of its own — its point is the default, which is the
                // page's own corner and would fail every bounds check for a reason that is not one.
                if (command.Verb == PathVerb.Close) continue;

                Points.Add(command.Point);

                if (command.Verb != PathVerb.CubicTo) continue;

                Points.Add(command.Control1);
                Points.Add(command.Control2);
            }
        }
    }
}
