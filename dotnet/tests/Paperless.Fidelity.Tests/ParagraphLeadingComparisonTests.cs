using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Pins where a baseline lands when the font size changes from one paragraph to the next.
/// </summary>
/// <remarks>
/// <para>
/// The question no other comparison in this project asks. A pitch comparison cancels the error — the
/// space proportional line spacing adds leaves one paragraph and arrives at the next, so a block of
/// paragraphs is the same height whichever of them is charged for it — and a word-box comparison cannot
/// see it either, because a box top carries the font's own ascent. It shows only as an <em>absolute</em>
/// baseline, and only where the spacing or the size changes across a paragraph boundary.
/// </para>
/// <para>
/// The rule being pinned is Writer's, and it is not the obvious one: the leading above a paragraph's
/// first line belongs to the paragraph <em>above</em> it, measured against that paragraph's last line.
/// <c>SwTextFormatter::CalcRealHeight</c> skips the inter-line spacing for a paragraph's own first line
/// and says where it is applied instead — <em>"for the _first_ line the line spacing of the previous
/// paragraph is applied in SwFlowFrame::CalcUpperSpace()"</em> — and that method adds it as
/// <c>nPrevLineSpacing</c>. See <c>Layout/ParagraphLeading.cs</c> for the full citations.
/// </para>
/// <para>
/// <c>paginated.*</c> is the case, in all four formats: 11 pt body paragraphs at 115% line spacing with
/// seven 16 pt headings at 100% among them. Before the rule was applied each heading's baseline sat
/// 1.95 pt too high and the body line after it 1.95 pt too low, with the block between them the right
/// height either way.
/// </para>
/// </remarks>
public sealed class ParagraphLeadingComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-leading").FullName;

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
    /// Every baseline, compared as an absolute distance down the page rather than as a pitch.
    /// </summary>
    /// <remarks>
    /// The stronger form of <c>PdfOutputComparisonTests.EveryBaselineFallsOnLibreOfficesPitch</c>, and it
    /// is only possible because both sides state a real baseline. A pitch comparison is blind to any
    /// error that one paragraph makes and the next unmakes; this is blind to nothing.
    /// </remarks>
    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    // Contextual spacing between two *different* styles that resolve to the same properties. Writer
    // compares the paragraphs' format collections (`lcl_IdenticalStyles`,
    // sw/source/core/layout/flowfrm.cxx:1503), so a heading style based on a contextual body style
    // keeps its space above; comparing resolved properties instead says the two are identical and
    // swallows it. The Word formats only — see tests/corpus/README.md for why the document is
    // hand-written and why there is no ODF member of the set.
    [InlineData("contextual-spacing-styles.docx")]
    [InlineData("contextual-spacing-styles.doc")]
    [InlineData("contextual-spacing-styles.rtf")]
    // Whether two paragraphs' spacings add or the larger wins — Writer's PARA_SPACE_MAX, which the four
    // formats answer differently and none of them by the same default. Each fixture is eight paragraphs
    // each carrying 12 pt of space-before and 8 pt of space-after on 12 pt exact lines, so every one of
    // the seven boundaries is 8 pt apart between the two answers and the error accumulates down the page.
    // paragraph-spacing-settings.docx is the counter-example rather than a defect: same paragraphs, but
    // in a package that has a settings part, and it must keep collapsing. See
    // ParagraphSpacingAccumulationTests for which rule each format follows and why.
    [InlineData("paragraph-spacing-no-settings.docx")]
    [InlineData("paragraph-spacing-settings.docx")]
    [InlineData("paragraph-spacing-collapsed.rtf")]
    [InlineData("paragraph-spacing-collapsed.odt")]
    [InlineData("paragraph-spacing-collapsed.fodt")]
    public void EveryBaselineIsWhereLibreOfficePutsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<PdfTextRun> ours, List<PdfTextRun> theirs) = Rendered(fileName);

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn lines");

        for (int i = 0; i < theirs.Count; i++)
        {
            ours[i].PageIndex.ShouldBe(theirs[i].PageIndex, $"{fileName}: line {i + 1} page");

            Math.Abs(ours[i].Y - theirs[i].Y).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: line {i + 1} on page {theirs[i].PageIndex + 1} sits at "
                + $"{ours[i].Y:F3} pt, {theirs[i].Y:F3} pt rendered");
        }
    }

    /// <summary>
    /// The two halves of a size change, each on its own, rather than only their sum.
    /// </summary>
    /// <remarks>
    /// What the sum hides: the block from the last body baseline to the heading's and on to the next body
    /// baseline is 57.650 pt whether the leading is charged to the heading or to the body above it, so
    /// only splitting it in two says which. Asserting a count as well, because a comparison that found no
    /// size change at all would pass while proving nothing.
    /// </remarks>
    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    public void ASizeChangeSplitsTheBlockWhereWriterSplitsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<PdfTextRun> ours, List<PdfTextRun> theirs) = Rendered(fileName);
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn lines");

        int compared = 0;
        for (int i = 1; i < theirs.Count; i++)
        {
            if (theirs[i].PageIndex != theirs[i - 1].PageIndex) continue;
            if (Math.Abs(theirs[i].FontSize - theirs[i - 1].FontSize) <= 0.001) continue;

            double drawn = ours[i].Y - ours[i - 1].Y;
            double rendered = theirs[i].Y - theirs[i - 1].Y;

            Math.Abs(drawn - rendered).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: the gap into line {i + 1}, where the size changes from "
                + $"{theirs[i - 1].FontSize} pt to {theirs[i].FontSize} pt, is {drawn:F3} pt drawn "
                + $"and {rendered:F3} pt rendered");

            compared++;
        }

        // Seven headings, each entered and left again: fourteen boundaries, less any that a page break
        // separates. Anything under ten would mean the document had stopped exercising this.
        compared.ShouldBeGreaterThanOrEqualTo(
            10, $"{fileName}: too few size changes to prove anything");
    }

    // ------------------------------------------------------------------------- the machinery

    private (List<PdfTextRun> Ours, List<PdfTextRun> Theirs) Rendered(string fileName)
    {
        string path = Corpus.Require(fileName);

        return (
            LinesOf(PdfTextRuns.Read(Ours(path))),
            LinesOf(PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory))));
    }

    /// <summary>Renders a document to a PDF of our own, and returns its path.</summary>
    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-leading.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    /// <summary>
    /// One entry per drawn line: the first run on each baseline.
    /// </summary>
    /// <remarks>
    /// The same grouping <see cref="PdfOutputComparisonTests"/> uses, and for the same reason —
    /// LibreOffice ends a text block at every formatting change and at the blank a wrapped line ends
    /// with, so a run is not a line on its side and is on ours.
    /// </remarks>
    private static List<PdfTextRun> LinesOf(List<PdfTextRun> runs)
    {
        List<PdfTextRun> lines = [];

        foreach (PdfTextRun run in runs)
        {
            if (lines.Count > 0
                && lines[^1].PageIndex == run.PageIndex
                && Math.Abs(lines[^1].Y - run.Y) <= 0.5)
            {
                continue;
            }

            lines.Add(run);
        }

        return lines;
    }
}
