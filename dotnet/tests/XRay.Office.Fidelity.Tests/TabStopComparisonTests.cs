using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using XRay.Office.WordProcessing;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

/// <summary>
/// Checks that a tab advances to the stop LibreOffice advances to.
/// </summary>
/// <remarks>
/// <para>
/// Word by word and absolutely, which for tabs is both possible and the point. A tab's width is not a
/// property of the font — it is the distance to the next stop — so the word after one starts at an absolute
/// position that the document states, and comparing it needs none of the differential machinery
/// justification does. The quantisation that spoils a long line does not accumulate here either, because
/// each stretch between tabs is short and starts afresh at its stop.
/// </para>
/// <para>
/// The corpus document exercises the four things that can go wrong separately: the default interval, which
/// is a quarter over a centimetre in LibreOffice and not the half inch Word uses; explicit left stops;
/// centre and right stops, where the stretch's own width decides where it starts; and a decimal stop, where
/// the position of a separator inside the stretch decides it.
/// </para>
/// </remarks>
// [2026-09-06, diagnosed, left failing on purpose] AListLabelsTabAdvancesToLibreOfficesStop fails
// on all four formats and `EveryTabAdvancesToLibreOfficesStop` passes on all four, at the same
// tolerance in the same file — and the pair is the cleanest control in the suite for what is
// actually being measured.
//
// In `tabbed.docx` every stretch after a tab is its own text object with its own `Td`, so the
// reference *states* each position: all three renderings agree at every word to the 0.100 pt pen
// offset below. In `list-label-overrun.docx` the label overruns its stop, so the whole line is one
// text object and every position after the first has to be reconstructed from the PDF's declared
// widths — which LibreOffice writes as **truncated** integer thousandths of an em (every one of
// this document's 26 is `floor(hmtx * 1000 / upem)`, mean deficit 0.482 thousandths). The three
// renderings therefore drift apart word by word: between the two reference binaries alone, 0.000,
// 0.011, 0.044, 0.066, 0.088, 0.099 pt.
//
// So this is not an advance divergence, and the paragraph above about the quantisation "not
// accumulating here" is right about a stretch that starts at a stop and wrong about one that does
// not. Measured through the reference's own `Td` pen, our laid-out widths agree with both binaries
// to 0.011% over 5 faces x 6 units x up to 11 sizes; see `probes/advance-ppem/`. Left failing: only
// writing our PDF with LibreOffice's truncated widths would close it.
public sealed class TabStopComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A tenth of a point, two twips — the same bound the run comparison uses. A stop read in the wrong
    /// unit, or a default interval of 720 twips where LibreOffice uses 709, misses by pounds rather than
    /// pence: eleven twips per tab, over half a point.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    /// <remarks>
    /// Measured at three different left margins; <see cref="MixedRunComparisonTests"/> records the
    /// evidence. Additive, horizontal only, and a property of the export rather than of the layout.
    /// </remarks>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("xray-office-tabbed").FullName;

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

    [Theory]
    [InlineData("tabbed.fodt")]
    [InlineData("tabbed.docx")]
    [InlineData("tabbed.doc")]
    [InlineData("tabbed.rtf")]
    public void EveryTabAdvancesToLibreOfficesStop(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = Drawn(path);
        List<PdfWord> rendered = InReadingOrder(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{fileName}: the drawn text differs from the rendered text");

        int afterTab = 0;
        for (int i = 0; i < rendered.Count; i++)
        {
            string where = $"{fileName}: word {i + 1} (\"{rendered[i].Text}\")";

            Math.Abs(drawn[i].Left - (rendered[i].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{where}: starts at {drawn[i].Left:F3} pt drawn, "
                    + $"{rendered[i].Left - PdfPenOffsetPoints:F3} pt rendered");

            // Words that a tab moved rather than words that simply follow a space: every word here but
            // the first of its line is one, since the corpus document separates them all with tabs.
            if (i > 0 && Math.Abs(rendered[i].Top - rendered[i - 1].Top) < 0.5) afterTab++;
        }

        afterTab.ShouldBeGreaterThan(
            20, $"{fileName}: only {afterTab} words followed a tab, which proves too little");
    }

    /// <summary>
    /// The tab that follows a list label advances to a stop, and not merely to the label's own end.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's list label really does end in a tab —
    /// <c>SvxNumberFormat::GetLabelFollowedByAsString</c> returns <c>"\t"</c>
    /// (<c>editeng/source/items/numitem.cxx:504</c>) — and that tab goes through the same
    /// <c>GetTabStop</c> as any other. It matters only when the label is wider than the room its level
    /// reserved, which is the whole of what the corpus document sets up: one list whose label overruns
    /// its stop and one whose label fits, so a fix for the first cannot quietly move the second.
    /// </para>
    /// <para>
    /// Four formats and not five. The RTF reader takes its label from the <c>{\listtext}</c> group and
    /// reads no level definition, so it cannot know the stop and marks every label
    /// <c>LabelFollow.Nothing</c>; on this document LibreOffice puts the overrunning item's text 35 pt
    /// further along than we do. That is a real gap and it is recorded rather than papered over.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("list-label-overrun.fodt")]
    [InlineData("list-label-overrun.odt")]
    [InlineData("list-label-overrun.docx")]
    [InlineData("list-label-overrun.doc")]
    public void AListLabelsTabAdvancesToLibreOfficesStop(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = Drawn(path);
        List<PdfWord> rendered = InReadingOrder(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{fileName}: the drawn text differs from the rendered text");

        for (int i = 0; i < rendered.Count; i++)
        {
            Math.Abs(drawn[i].Left - (rendered[i].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{fileName}: word {i + 1} (\"{rendered[i].Text}\") starts at "
                    + $"{drawn[i].Left:F3} pt drawn, {rendered[i].Left - PdfPenOffsetPoints:F3} pt "
                    + "rendered");
        }

        // Both lists, so the document proves the overrun case and the ordinary one at once.
        drawn.Count(word => word.Text == "Paragraph").ShouldBe(2);
        drawn.Count(word => word.Text is "1." or "2.").ShouldBe(2);
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words
            .OrderBy(word => word.PageIndex)
            .ThenBy(word => Math.Round(word.Top, 1))
            .ThenBy(word => word.Left)];

    private static List<DrawnWord> Drawn(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return
        [
            .. sink.Pages.SelectMany(page => DrawnWords.On(page)
                .OrderBy(word => Math.Round(word.Baseline, 1))
                .ThenBy(word => word.Left)),
        ];
    }
}
