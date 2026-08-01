using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the SmartArt diagram a slide draws against the one LibreOffice draws, in its own PDF.
/// </summary>
/// <remarks>
/// <para>
/// <strong>What is being compared, and why it is comparable at all.</strong> A diagram in an
/// OOXML package is five parts: a data model, a layout definition, a quick style, a colour
/// transform — the four a <c>dgm:relIds</c> names — and a fifth it does not, the
/// <c>diagramDrawing</c> the authoring application bakes beside them, holding the diagram
/// already laid out as ordinary DrawingML shapes. LibreOffice prefers that fifth part and
/// evaluates the layout algorithms only when it is missing or empty
/// (<c>oox/source/drawingml/diagram/diagram.cxx:701</c>,
/// <c>bCreate = pShape-&gt;getExtDrawings().empty()</c>). Paperless does the same, so on a
/// document that carries one the two renderers are drawing the same shape tree and every
/// coordinate should agree.
/// </para>
/// <para>
/// <strong>The corpus deck is hand-written, because nothing can generate one.</strong>
/// LibreOffice cannot author SmartArt — it only preserves what it imported, through the
/// interop grab bag — so a diagram fixture cannot come out of a conversion the way the rest of
/// the presentation corpus does. <c>slide-diagram-baked.pptx</c> is therefore written part by
/// part on the skeleton of <c>slide-table-grid.pptx</c>, with every offset a round number of
/// EMUs, so that a disagreement here is a bug rather than a rounding.
/// </para>
/// <para>
/// The one place this deliberately does not follow LibreOffice is the text rectangle; see
/// <see cref="EveryLabelIsDrawnWhereLibreOfficeDrawsIt"/>.
/// </para>
/// </remarks>
public sealed class SlideDiagramComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// How far a centred label's pen may differ, in points.
    /// </summary>
    /// <remarks>
    /// Wider than the tenth of a point everything else here uses, and only because centring
    /// <em>halves</em> a width difference and then adds it to a rounded rectangle edge.
    /// LibreOffice measures a string in hundredths of a millimetre before it centres it, so its
    /// "Gamma" is 0.09 pt wider than ours and its whole page sits 0.028 pt up and to the left;
    /// the two put the pen 0.11 pt apart. A real error in a stated text rectangle is tens of
    /// points, not tenths — reading the diagram-space offset as a shape-local one moves this
    /// label by 468.
    /// </remarks>
    private const double CentringTolerancePoints = 0.15;

    private const string Deck = "slide-diagram-baked.pptx";

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-diagrams").FullName;

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

    /// <summary>
    /// The colours the fixture's three solidly filled nodes carry, which is how each is found.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Matching by colour rather than by paint order, because the two writers do not agree on
    /// how many paths a slide is. Ours draws an axis-aligned rectangle with PDF's own <c>re</c>
    /// and a gradient with one <c>sh</c> inside a clip; LibreOffice writes every rectangle as a
    /// closed polygon and every gradient as a stack of about fifty filled bands. So the
    /// reference's content stream holds 54 filled paths against our 2 for the same four shapes,
    /// and any comparison that counts paths measures the writers rather than the diagram.
    /// </para>
    /// <para>
    /// A colour is a stable handle instead: each of the three appears exactly once in the file,
    /// and none of them is a shade of the gradient's amber ramp or the page's white.
    /// </para>
    /// </remarks>
    private static readonly uint[] NodeColours = [0xC5E0B4, 0xBDD7EE, 0xA6A6A6];

    /// <summary>
    /// Every node of the diagram is filled where LibreOffice fills it, in the same colour.
    /// </summary>
    /// <remarks>
    /// The presence of the shapes is half the assertion. A reader that found the diagram's four
    /// parts, read its text and drew nothing — which is what a diagram did before this —
    /// produces a page with the right words on it and no shapes at all, and every text
    /// comparison passes.
    /// </remarks>
    [Fact]
    public void EveryNodeIsFilledWhereLibreOfficeFillsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        string ourPdf = Ours(path);
        string theirPdf = _libreOffice.ConvertToPdf(path, _workDirectory);

        Assert.SkipWhen(PdfPaths.Read(theirPdf).Count == 0,
                        "pdftotext is not available; install poppler-utils");

        foreach (uint colour in NodeColours)
        {
            string where = $"node {colour:X6}";

            (double left, double top, double right, double bottom) mine =
                Painted(ourPdf, colour).ShouldNotBeNull($"{where}: not drawn at all");
            (double left, double top, double right, double bottom) reference =
                Painted(theirPdf, colour).ShouldNotBeNull($"{where}: not in the reference");

            mine.left.ShouldBe(reference.left, TolerancePoints, $"{where}: left");
            mine.top.ShouldBe(reference.top, TolerancePoints, $"{where}: top");
            mine.right.ShouldBe(reference.right, TolerancePoints, $"{where}: right");
            mine.bottom.ShouldBe(reference.bottom, TolerancePoints, $"{where}: bottom");
        }
    }

    /// <summary>
    /// The connector's outline is the polygon LibreOffice draws, vertex for vertex.
    /// </summary>
    /// <remarks>
    /// A bounding box is not a shape: a <c>rightArrow</c> whose two adjustment values were
    /// ignored has the same box as a correct one and a visibly different head. This is the one
    /// shape in the fixture both writers spell as a plain polygon, so it is the one where the
    /// vertices are comparable at all.
    /// </remarks>
    [Fact]
    public void TheConnectorIsTheSamePolygonLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfPath> ours = [.. PdfPaths.Read(Ours(path)).Where(p => p.Colour == 0xA6A6A6)];
        List<PdfPath> theirs =
        [
            .. PdfPaths.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(p => p.Colour == 0xA6A6A6),
        ];

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        PdfPath mine = ours.ShouldHaveSingleItem();
        PdfPath reference = theirs.ShouldHaveSingleItem();

        mine.Points.Count.ShouldBe(reference.Points.Count, "connector: vertex count");

        for (int p = 0; p < reference.Points.Count; p++)
        {
            mine.Points[p].X.ShouldBe(
                reference.Points[p].X, TolerancePoints, $"connector: vertex {p + 1} across");
            mine.Points[p].Y.ShouldBe(
                reference.Points[p].Y, TolerancePoints, $"connector: vertex {p + 1} down");
        }
    }

    /// <summary>
    /// Every node's outline is drawn with the pen LibreOffice draws it with.
    /// </summary>
    /// <remarks>
    /// A diagram's pens come from its colour transform by way of the authoring application,
    /// which bakes the resolved colour into <c>dsp:spPr/a:ln</c> — so this checks the rename
    /// carried the line properties through, not the colour chain. The widths in the fixture are
    /// 1 pt, 1.5 pt and 0.5 pt, chosen to be far enough apart that a hairline default would show.
    /// </remarks>
    [Fact]
    public void EveryNodeIsOutlinedWithLibreOfficesPen()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfStroke> ours = PdfStrokes.Read(Ours(path));
        List<PdfStroke> theirs = PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of outline strokes");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfStroke mine = ours[i];
            PdfStroke reference = theirs[i];
            string where = $"outline {i + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: pen width");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: pen colour");
        }
    }

    /// <summary>
    /// Every node's label is drawn where LibreOffice draws it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A measured divergence lives here.</strong> A baked shape may state its text area
    /// as <c>dsp:txXfrm</c>, and Paperless honours it directly because a text rectangle is a
    /// parameter of its text layout. LibreOffice cannot: <c>Transform2DContext</c>
    /// (<c>oox/source/drawingml/transform2dcontext.cxx:296-391</c>) says in its own comment that
    /// it "cannot change the text area rectangle directly, because currently we depend on the
    /// geometry definition of the preset", and works around it by turning the difference between
    /// the preset's text rectangle and the stated one into four indents. That workaround needs a
    /// preset text rectangle, which <c>ConstructPresetTextRectangle</c> hand-implements for
    /// fourteen presets and refuses for the rest — and on a refusal the <c>dsp:txXfrm</c> is
    /// dropped entirely.
    /// </para>
    /// <para>
    /// So the two agree wherever LibreOffice implements the preset, because its indents move the
    /// text block onto exactly the stated rectangle. Measured over the 469 baked shapes in
    /// LibreOffice's own corpus: 286 carry a <c>dsp:txXfrm</c>, and 273 of those — 95% — use one
    /// of the fourteen. The fixture's three labelled shapes are an ellipse, a roundRect and a
    /// rect, all implemented, so this comparison is exact. The remaining 5% are documents where
    /// Paperless will place a label where PowerPoint does and LibreOffice will not;
    /// <c>sd/qa/unit/data/pptx/tdf149551_SmartArt_Gear.pptx</c> is one, on a <c>gear9</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void EveryLabelIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs =
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of drawn labels");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"label {i + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.X.ShouldBe(reference.X, CentringTolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, CentringTolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, TolerancePoints, $"{where}: em size");
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    /// <summary>
    /// The rectangle everything painted in one colour covers, or null when nothing was.
    /// </summary>
    /// <remarks>
    /// Both readers, unioned, because neither sees every spelling: <see cref="PdfPaths"/> reads
    /// paths that begin with an <c>m</c> and so misses PDF's own <c>re</c>, and
    /// <see cref="PdfFills"/> reads rectangles and so misses an ellipse. A union over one colour
    /// is the same rectangle either way round, since each colour belongs to a single shape.
    /// </remarks>
    private static (double Left, double Top, double Right, double Bottom)? Painted(
        string pdfPath, uint colour)
    {
        List<(double Left, double Top, double Right, double Bottom)> boxes =
        [
            .. PdfPaths.Read(pdfPath).Where(path => path.Colour == colour)
                .Select(path => path.Bounds),
            .. PdfFills.Read(pdfPath).Where(fill => fill.Colour == colour)
                .Select(fill => (fill.Left, fill.Top,
                                 fill.Left + fill.Width, fill.Top + fill.Height)),
        ];

        return boxes.Count == 0
            ? null
            : (boxes.Min(b => b.Left), boxes.Min(b => b.Top),
               boxes.Max(b => b.Right), boxes.Max(b => b.Bottom));
    }

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
