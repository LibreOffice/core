using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Checks that the rule above a page's notes follows the rules of the application that owns the format.
/// </summary>
/// <remarks>
/// <para>
/// Writer draws it as a quarter of the column; Word draws a fixed two inches and puts it 60 % of the way
/// down a reservation taken from the default paragraph style rather than a fixed distance above the notes.
/// LibreOffice implements both and chooses between them on
/// <c>DocumentSettingId::CONTINUOUS_ENDNOTES</c>, which <em>both</em> its Word filters set
/// unconditionally — <c>WriterFilter.cxx</c>:338 for DOCX, <c>ww8par.cxx</c>:2050 for DOC — and which
/// neither its RTF filter nor either of its ODF filters sets.
/// </para>
/// <para>
/// <b>The RTF row is the point of this file.</b> X-Ray.Office routes DOCX, DOC <em>and RTF</em> through
/// <see cref="PaginationOptions.Word"/>, so the obvious place to put this rule is that preset — and doing
/// so would silently give RTF a separator LibreOffice does not draw for it. The five spellings here are
/// one document, so the assertion is not "these numbers are plausible" but "the same content is ruled two
/// different ways and the split falls exactly here".
/// </para>
/// <para>
/// Measured against the installed LibreOffice 26.2.4.2 on an authored probe before any of this was
/// written (<c>probes/fidelity-b-01/separator-probe.py</c>): one document converted to all five
/// spellings, rendered, and the rule read out of the PDF's own path. DOCX and DOC drew 144.000 pt with
/// the column at 481.890 pt <em>and</em> at 255.118 pt — which is what tells an absolute two inches from
/// a proportion — while FODT, ODT and RTF each drew 25.0 % of whatever the column was.
/// </para>
/// </remarks>
public sealed class NoteSeparatorRuleTests
{
    /// <summary>Word's rule, and the reason 2 in is stated here as its own literal.</summary>
    /// <remarks>
    /// Written out rather than taken from <see cref="PaginationOptions.WordNoteSeparatorLength"/>, so that
    /// changing that constant is caught here rather than agreed with.
    /// </remarks>
    private const double WordRulePoints = 144.0;

    /// <summary>Writer's, as a fraction of the column.</summary>
    private const double WriterRuleFraction = 0.25;

    [Theory]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.doc")]
    public void AWordDocumentsRuleIsTwoInchesWhateverItsColumn(string fileName)
    {
        DocRect rule = SeparatorOf(fileName);

        rule.Width.Points.ShouldBe(
            WordRulePoints, 0.01,
            $"{fileName}: Word's note separator is a fixed two inches");
    }

    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    // Not a Word format for this purpose, whatever else it shares with one. LibreOffice's RTF import is a
    // different filter class from its DOCX one and sets none of the DOCX compatibility settings.
    [InlineData("footnotes.rtf")]
    public void EveryOtherDocumentsRuleIsAQuarterOfItsColumn(string fileName)
    {
        (DocRect rule, Length column) = SeparatorAndColumn(fileName);

        rule.Width.Points.ShouldBe(
            column.Points * WriterRuleFraction, 0.05,
            $"{fileName}: Writer's note separator is a quarter of the text width");

        // And it is not two inches by coincidence of this document's column, which is the failure a
        // width-only assertion would pass through: 481.9 pt of column would put a quarter at 120.5.
        Math.Abs(rule.Width.Points - WordRulePoints).ShouldBeGreaterThan(
            0.5, $"{fileName}: and is not Word's rule");
    }

    /// <summary>
    /// The same document, ruled by the two engines, differs vertically by the amount LibreOffice differs by.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A width comparison cannot see the second half of the switch, and the second half is what makes the
    /// first half worth shipping: a fix that set the length and not the position leaves the rule 2.2 pt out
    /// and turns no comparison green.
    /// </para>
    /// <para>
    /// 2.200 pt is measured, not derived — the authored probe's DOCX rule sits at 73.789 pt and its ODT
    /// rule at 71.589 pt on the same page, and the corpus document reproduces the same 2.200 between
    /// LibreOffice's own two renderings. Asserted as a <em>difference</em> between two spellings of one
    /// document so that it says something about the rule rather than about this page's furniture.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheWordRuleSitsTwoPointTwoHigherAboveItsNotesThanWritersDoes()
    {
        double word = RiseAboveNotes("footnotes.docx");
        double writer = RiseAboveNotes("footnotes.odt");

        (word - writer).ShouldBe(
            2.200, 0.1,
            $"the Word rule rises {word:F3} pt above its notes and the Writer rule {writer:F3} pt");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>How far above the first note line's box the rule's top sits, in points.</summary>
    /// <remarks>
    /// Measured from the notes rather than from the page, because that is the only end of the reservation
    /// that is fixed: the note area is bottom-aligned in the body, so its top is where the notes landed and
    /// the rule is positioned from there. Comparing absolute page positions instead would compare two
    /// documents' pagination.
    /// </remarks>
    private static double RiseAboveNotes(string fileName)
    {
        LaidOutPage page = PageWithNotes(fileName);

        PlacedFlow notes = page.Notes.ShouldNotBeNull($"{fileName}: the page should carry notes");
        DocRect rule = page.NoteSeparator.ShouldNotBeNull($"{fileName}: and a rule above them");

        Length notesTop = notes.Area.Y + notes.Lines[0].Top;
        return (notesTop - rule.Y).Points;
    }

    private static DocRect SeparatorOf(string fileName) => SeparatorAndColumn(fileName).Rule;

    private static (DocRect Rule, Length Column) SeparatorAndColumn(string fileName)
    {
        LaidOutPage page = PageWithNotes(fileName);

        return (page.NoteSeparator.ShouldNotBeNull($"{fileName}: a rule above the notes"),
                page.BodyArea.Width);
    }

    /// <summary>The first page that carries notes, which is the one the rule is on.</summary>
    private static LaidOutPage PageWithNotes(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Pages.FirstOrDefault(page => page.NoteSeparator is not null)
               ?? throw new InvalidOperationException($"{fileName}: no page carries a note separator");
    }
}
