using Paperless.Core.Documents;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// <c>w:usePrinterMetrics</c> rounds every font metric onto a 300 dpi grid, on DOCX as well as
/// on DOC.
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
/// The two fixtures differ in that element and nothing else. Measured on LibreOffice 24.2.7.2,
/// the line pitch of 12 pt Arial — Liberation Sans after substitution — is <b>13.80 pt</b>
/// printer-independently and <b>13.95 pt</b> with the flag, and both packages set eleven lines.
/// </para>
/// <para>
/// The face and size are not arbitrary. Six pairs were swept
/// (<c>research/probes/words-r13/probe-grid.py</c>) and the flag separates them all except
/// 10 pt Times New Roman, where our printer-independent pitch is 11.50 against LibreOffice's
/// 11.55 — an older, unrelated defect that makes both packages come out at 11.55 and the flag
/// look inert. A fixture built on that pair would have proved the opposite of the truth.
/// </para>
/// </remarks>
public sealed class PrinterMetricsTests
{
    /// <summary>The pitch LibreOffice sets 12 pt Arial at when the metrics are the printer's.</summary>
    private const double PrinterPitch = 13.95;

    /// <summary>And what it sets the same paragraph at printer-independently.</summary>
    private const double IndependentPitch = 13.80;

    /// <summary>A tenth of a point: the two pitches under test are 0.15 apart.</summary>
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
