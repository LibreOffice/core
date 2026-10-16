using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>w:usePrinterMetrics</c> rounds every font metric onto the printer's pixel grid, on DOCX as
/// well as on DOC.
/// </summary>
/// <remarks>
/// <para>
/// It had been recorded in <see cref="Ooxml.WordCompatibility"/> as identified and inert, on the
/// grounds that headless LibreOffice ignores it. The importer says otherwise:
/// <c>DomainMapper_Impl::ApplySettingsTable</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10173</c>) sets
/// <c>PrinterIndependentLayout::DISABLED</c> from it, the same state
/// <c>WW8Dop::fUsePrinterMetrics</c> puts a DOC into and which <c>DocReader</c> has honoured all
/// along.
/// </para>
/// <para>
/// The two fixtures differ in that element and nothing else. Measured on the installed
/// <b>26.2.4.2</b>, the line pitch of 16 pt Arial — Liberation Sans after substitution — is
/// <b>18.450 pt</b> printer-independently and <b>18.250 pt</b> with the flag.
/// </para>
/// <para>
/// <strong>The size is not arbitrary and it had to be changed on 2026-08-15.</strong> The pair was
/// built at 12 pt, where a 300 dpi printer separated them by 0.15 pt; the printer on this
/// container is 600 dpi, which sets 12 pt at exactly 100 device pixels, so at that size the flag
/// changes <em>nothing</em> and this test asserted a difference the binary no longer makes. 16 pt
/// is the largest separation in a sweep of every half point from 8 to 16 — 4 twips, 0.20 pt — and
/// <c>probes/words-p1-01/remake-printer-metrics-docx.py</c> both rebuilds the pair and re-measures
/// it through <c>soffice</c>, because <em>which</em> size discriminates is a property of the
/// device rather than of the document and moves with it.
/// </para>
/// <para>
/// Note that the printer pitch is the <em>shorter</em> one here, which the 12 pt case never showed.
/// The grid is a coarser quantisation and not a systematic enlargement: it moves in both
/// directions with size. A fixture that only ever saw it grow invited the wrong generalisation.
/// </para>
/// </remarks>
public sealed class PrinterMetricsTests
{
    /// <summary>The pitch LibreOffice sets 16 pt Arial at when the metrics are the printer's.</summary>
    private const double PrinterPitch = 18.25;

    /// <summary>And what it sets the same paragraph at printer-independently.</summary>
    private const double IndependentPitch = 18.45;

    /// <summary>A thirtieth of a point: the two pitches under test are 0.20 apart.</summary>
    private const double Tolerance = 0.03;

    [Fact]
    public void TheFlagPutsTheLinesOnThePrintersGrid()
        => Pitch("printer-metrics.docx").ShouldBe(PrinterPitch, Tolerance);

    [Fact]
    public void WithoutItTheyAreMeasuredIndependently()
        => Pitch("printer-metrics-off.docx").ShouldBe(IndependentPitch, Tolerance);

    /// <summary>The mean baseline-to-baseline distance down the fixture's one paragraph.</summary>
    private static double Pitch(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        List<double> baselines =
            [.. DrawnWords.On(sink.Pages[0]).Select(word => word.Baseline).Distinct().Order()];
        baselines.Count.ShouldBeGreaterThan(4);

        return (baselines[^1] - baselines[0]) / (baselines.Count - 1);
    }
}
