using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks the colours a themed <c>w:shd</c> and a themed <c>w:tblBorders</c> resolve to, against
/// the colours LibreOffice paints for the DrawingML chain each one stands for.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why the comparison is indirect.</b> LibreOffice does not resolve either of these. Measured
/// on <c>theme-table.docx</c>: of its six shaded cells it fills exactly <em>one</em> — the one
/// with a cached <c>w:fill</c> — and leaves the other five unpainted; of its five bordered rows
/// it strokes the one with a cached <c>w:color</c> in that colour and the other four in
/// <b>black</b>. <c>CellColorHandler</c> takes the fill straight from <c>w:fill</c>
/// (<c>sw/source/writerfilter/dmapper/CellColorHandler.cxx</c>:113) and keeps
/// <c>w:themeFill</c> only as a grab-bag entry for round-tripping, exactly as
/// <c>DomainMapper</c> does with <c>w:color</c>. So a themed table part cannot be compared with
/// LibreOffice's own answer, because LibreOffice has not got one.
/// </para>
/// <para>
/// <b>What it can be compared with.</b> A Word modifier is a DrawingML transform chain — a
/// <c>w:themeFillShade="BF"</c> is a <c>lumMod</c> of 74902 — and LibreOffice <em>does</em>
/// resolve a shape's <c>a:solidFill</c>, through <c>oox/source/drawingml/color.cxx</c>. So the
/// corpus document carries a twin shape for every themed table part, stating the equivalent
/// chain, and the test requires the table part and its twin to come out the same colour. That
/// makes the arithmetic measurable against LibreOffice even though the element carrying it is
/// not, and it is the same technique the shape-fill comparison uses for <c>w:color</c>.
/// </para>
/// <para>
/// The twins are laid out first on the page and are 113.4 pt wide against the tables' 220, so
/// the two sets separate by width without the test having to know the document's order.
/// </para>
/// </remarks>
public sealed class ThemedTablePartComparisonTests : IDisposable
{
    /// <summary>The width, in points, above which a filled rectangle is a table cell and not a twin.</summary>
    private const double TwinWidthCeiling = 150;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-themed-table").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Fact]
    public void AThemedCellShadeIsTheColourLibreOfficePaintsForTheSameChain()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("theme-table.docx");
        List<uint> twins = Twins(path);

        List<uint> shades =
            [.. Rendered(path)[0].FilledPaths
                .Where(fill => fill.Bounds.Width > Length.FromPoints(TwinWidthCeiling))
                .OrderBy(fill => fill.Bounds.Top)
                .Select(fill => Rgb(fill.Paint))];

        // Five themed, then one whose cached w:fill disagrees with its own theme reference — it
        // states accent1 darkened, and caches accent2's literal. The cache is what Word shows and
        // what LibreOffice paints, so it has to win over the reference beside it.
        shades.Count.ShouldBe(6);

        uint[] expected = [twins[0], twins[1], twins[2], twins[3], twins[4], 0xC0504D];

        for (int i = 0; i < expected.Length; i++)
        {
            shades[i].ShouldBe(
                expected[i],
                $"cell shade {i} resolved to #{shades[i]:X6}, expected #{expected[i]:X6}");
        }
    }

    [Fact]
    public void AThemedBorderIsTheColourLibreOfficePaintsForTheSameChain()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("theme-table.docx");
        List<uint> twins = Twins(path);

        List<uint> borders =
            [.. Rendered(path)[0].StrokedPaths
                .OrderBy(stroke => stroke.Bounds.Top)
                .Select(stroke => Rgb(stroke.Stroke.Paint))];

        // Four stated on the cell and one on the table: the last row carries no w:tcBorders at
        // all, so its bottom edge is the table's own themed w:tblBorders. That is the only route
        // by which a themed *table* border reaches a cell, and it is why the row is last —
        // w:tblBorders states the table's outer edges, and LibreOffice draws the bottom one on
        // the final row only.
        borders.Count.ShouldBe(5);

        uint[] expected = [twins[5], twins[6], twins[7], 0x00B050, twins[8]];

        for (int i = 0; i < expected.Length; i++)
        {
            borders[i].ShouldBe(
                expected[i],
                $"border {i} resolved to #{borders[i]:X6}, expected #{expected[i]:X6}");
        }
    }

    /// <summary>
    /// Pins the divergence itself, so that it is a recorded measurement rather than a belief.
    /// </summary>
    /// <remarks>
    /// If a later LibreOffice starts resolving these, this is the test that says so — and at
    /// that point the two comparisons above can become direct ones. Worth stating what "black"
    /// means here: it is not a failure to find the theme, it is <c>m_nColor</c>'s and the border
    /// handler's initial value surviving, the same zero that leaves a themed <c>w:color</c> run
    /// black in <c>theme-colours.docx</c>.
    /// </remarks>
    [Fact]
    public void LibreOfficeResolvesNeitherOfThem()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("theme-table.docx");
        string pdf = _libreOffice.ConvertToPdf(path, _workDirectory);

        // Distinct, because LibreOffice's DOCX render fills each shaded cell twice — once for the
        // cell and once for the paragraph inside it, at the same rectangle in the same colour.
        // The table comparison hit the same thing; counting the operators rather than the
        // rectangles reports twice as many shaded cells as the document has.
        List<PdfFill> cells =
            [.. PdfFills.Read(pdf)
                .Where(fill => fill.PageIndex == 0 && fill.Width > TwinWidthCeiling)
                .Distinct()
                .OrderBy(fill => fill.Top)];

        // One of six. The five themed-only shades are not painted at all, because an unresolved
        // w:themeFill leaves w:fill at "auto" and auto means "let what is behind show".
        cells.Count.ShouldBe(1);
        cells[0].Colour.ShouldBe(0xC0504Du);

        List<uint> strokes =
            [.. PdfStrokes.Read(pdf)
                .Where(stroke => stroke.PageIndex == 0)
                .OrderBy(stroke => stroke.FromY)
                .Select(stroke => stroke.Colour)];

        strokes.ShouldBe([0x000000u, 0x000000u, 0x000000u, 0x00B050u, 0x000000u]);
    }

    /// <summary>
    /// The nine twin shapes' painted colours, in document order.
    /// </summary>
    /// <remarks>
    /// From LibreOffice's own render rather than from Paperless's, so that the expectations are
    /// the reference's numbers and not this library's. They are, in order: accent1; accent1 with
    /// <c>lumMod</c> 74902; accent1 lightened 40%; accent3 lightened 80%; background2 darkened
    /// 50%; accent2; accent2 darkened 25%; accent4 lightened 60%; accent5.
    /// </remarks>
    private List<uint> Twins(string path)
    {
        List<uint> twins =
            [.. PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(fill => fill.PageIndex == 0 && fill.Width <= TwinWidthCeiling)
                .OrderBy(fill => fill.Top)
                .Select(fill => fill.Colour)];

        twins.Count.ShouldBe(9, "the reference did not paint every twin shape");
        return twins;
    }

    /// <summary>A paint's colour as <c>0xRRGGBB</c>; anything but a solid paint fails the test.</summary>
    private static uint Rgb(Paint paint)
    {
        SolidPaint solid = paint.ShouldBeOfType<SolidPaint>();
        return ((uint)solid.Colour.R << 16) | ((uint)solid.Colour.G << 8) | solid.Colour.B;
    }

    /// <summary>Lays a document out and records what it drew, page by page.</summary>
    private static IReadOnlyList<DrawnPage> Rendered(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink.Pages;
    }
}
