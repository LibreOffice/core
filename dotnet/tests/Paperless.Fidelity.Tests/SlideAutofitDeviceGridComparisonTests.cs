using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// An autofitted shape whose text already fits measures its lines at a different em from a plain
/// shape holding the same text at the same size.
/// </summary>
/// <remarks>
/// <para>
/// <c>ImpEditEngine::SeekCursor</c> takes a separate branch whenever <c>maStatus.DoStretch()</c>,
/// which <c>SdrTextObj::ImpSetupDrawOutlinerForPaint</c> sets for <c>IsFitToSize() || IsAutoFit()</c>
/// and for nothing else. It pushes the font at the device, reads the size back out of the device's
/// own metric and puts that on the font (<c>editeng/source/editeng/impedit3.cxx</c>:2985-3062,
/// 24.2.7), so during formatting the 1.2 rule is applied to the item height rounded to whole
/// reference-device pixels and back.
/// </para>
/// <para>
/// The corpus cannot separate that from the several other differences a slide carries at once, so
/// <c>slide-autofit-device-grid.pptx</c> is authored for it: one slide per size, the same three
/// paragraphs of two lines each — the second forced by a hard break so the line count cannot move
/// — in an <c>a:noAutofit</c> box at x = 20 and an <c>a:normAutofit</c> box at x = 380, both far
/// taller than their text so the search settles on scale 1. A throwaway shape goes first on every
/// slide, because LibreOffice's shared draw outliner formats the first text object on a page
/// before <c>SetFixedCellHeight</c> takes hold.
/// </para>
/// <para>
/// <strong>12 pt is a control and passes under either reading</strong> — 423 units come back 423
/// through the 600 dpi grid — so a blanket shift cannot pass this. The other four move in both
/// directions: the autofitted line is longer at 8 and 20 pt and shorter at 10 and 28.
/// </para>
/// <para>
/// The tolerance is a hundredth of a point, which is a third of the one hundredth of a millimetre
/// the effect is measured in. It has to be that tight: the differences under test are one to three
/// units, where the paragraph-space test next door separates whole points.
/// </para>
/// </remarks>
public sealed class SlideAutofitDeviceGridComparisonTests : IDisposable
{
    private const string Deck = "slide-autofit-device-grid.pptx";

    /// <summary>A hundredth of a point — a third of the unit the effect lives in.</summary>
    private const double TolerancePoints = 0.01;

    /// <summary>
    /// Per slide: the stated size, the plain box's line pitch and the autofitted box's, both in
    /// hundredths of a millimetre as read out of LibreOffice 24.2.7.2's own PDF.
    /// </summary>
    private static readonly (int Points, long Plain, long Autofitted)[] Expected =
    [
        (8, 338, 341),
        (10, 424, 421),
        (12, 508, 508),
        (20, 847, 848),
        (28, 1186, 1183),
    ];

    /// <summary>Hundredths of a millimetre in a point.</summary>
    private const double Mm100PerPoint = 2540.0 / 72.0;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-autofit-device-grid").FullName;

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
    public void AnAutofittedBodyMeasuresItsLinesOnTheDevicesGrid()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<PdfTextRun> ours = PdfTextRuns.Read(Ours());
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require(Deck), _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "no text runs came back from the reference");

        for (int page = 0; page < Expected.Length; page++)
        {
            (int points, long plain, long autofitted) = Expected[page];

            double plainPoints = plain / Mm100PerPoint;
            double autofittedPoints = autofitted / Mm100PerPoint;

            // Ours as literals first, so this tests Paperless rather than LibreOffice: comparing
            // only against the reference passes whatever the two of us happen to agree on.
            Pitch(ours, page, left: 20).ShouldBe(plainPoints, TolerancePoints, $"plain {points} pt");
            Pitch(ours, page, left: 380)
                .ShouldBe(autofittedPoints, TolerancePoints, $"autofitted {points} pt");

            // And the reference against the same two.
            Pitch(theirs, page, left: 20)
                .ShouldBe(plainPoints, TolerancePoints, $"the reference's plain {points} pt");
            Pitch(theirs, page, left: 380)
                .ShouldBe(autofittedPoints, TolerancePoints, $"the reference's autofitted {points} pt");
        }
    }

    /// <summary>
    /// The baseline pitch inside the box whose left edge is at <paramref name="left"/> points.
    /// </summary>
    /// <remarks>
    /// Keyed on the pen rather than on the drawing order, because the two writers do not emit the
    /// shapes in the same sequence. The vertical cut is what excludes the warm-up shape, which
    /// shares the left box's pen and ends 25 points above where the measured boxes start. Every
    /// gap inside a box is one line, since the paragraphs carry no space of their own — asserted
    /// rather than assumed, so a body that lost a line cannot pass by measuring a different gap.
    /// </remarks>
    private static double Pitch(List<PdfTextRun> runs, int page, double left)
    {
        const double belowTheWarmUp = 45.0;

        List<double> baselines =
        [
            .. runs.Where(r => r.PageIndex == page && Math.Abs(r.X - left) < 1.0
                               && r.Y > belowTheWarmUp)
                   .Select(r => r.Y).Order(),
        ];

        baselines.Count.ShouldBe(6, $"baselines in the box at x = {left} on page {page + 1}");

        List<double> gaps = [.. Enumerable.Range(1, baselines.Count - 1)
            .Select(i => baselines[i] - baselines[i - 1])];

        gaps.Max().ShouldBe(gaps.Min(), 0.02, $"the box at x = {left} on page {page + 1} is evenly spaced");

        return gaps[0];
    }

    private string Ours()
    {
        string source = Corpus.Require(Deck);
        string destination = Path.Combine(_workDirectory, "autofit-device-grid-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
