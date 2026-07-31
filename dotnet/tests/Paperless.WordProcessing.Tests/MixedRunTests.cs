using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that per-run formatting reaches the paginator as runs.
/// </summary>
/// <remarks>
/// <para>
/// The formats state emphasis in four different ways — an ODF automatic style behind a
/// <c>text:span</c>, an OOXML <c>w:rPr</c> on a <c>w:r</c>, a WW8 CHPX, an RTF control word — and all
/// four have to arrive as the same thing, because measurement and drawing take runs and know nothing
/// about where they came from.
/// </para>
/// <para>
/// Deliberately asserting on the paragraphs rather than on the drawn output: a run that resolves to the
/// wrong size still draws somewhere, so a test on the drawing would report a position and leave the cause
/// to be guessed at. The positions are checked against LibreOffice separately, in the fidelity tests.
/// </para>
/// </remarks>
public sealed class MixedRunTests
{
    [Theory]
    [InlineData("mixed-runs.fodt")]
    [InlineData("mixed-runs.docx")]
    [InlineData("mixed-runs.doc")]
    [InlineData("mixed-runs.rtf")]
    public void ASpanBecomesARunWithItsOwnFace(string fileName)
    {
        PageParagraph paragraph = ParagraphStartingWith(fileName, "One.");

        paragraph.HasRuns.ShouldBeTrue("a paragraph with a bold span is not uniform");

        // The runs partition the text: every character is in exactly one, and they are in order. A gap
        // would silently drop that text's width from every line after it.
        paragraph.Runs.Sum(run => run.Length).ShouldBe(paragraph.Text.Length);
        paragraph.Runs[0].Start.ShouldBe(0);

        PageRun bold = RunFor(paragraph, "emboldened");
        bold.Face.Weight.ShouldBeGreaterThanOrEqualTo(600, "the span resolves to a bold face");
        bold.Face.ShouldNotBeSameAs(paragraph.Face);

        // And the paragraph's own face is still the regular one, because that is what its mark carries and
        // what an empty paragraph would be as tall as.
        paragraph.Face.Weight.ShouldBeLessThan(600);
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    [InlineData("mixed-runs.docx")]
    [InlineData("mixed-runs.doc")]
    [InlineData("mixed-runs.rtf")]
    public void ALargerSpanKeepsItsOwnSize(string fileName)
    {
        PageParagraph paragraph = ParagraphStartingWith(fileName, "Two.");

        RunFor(paragraph, "twentytwo").EmSize.ShouldBe(Length.FromPoints(22));

        // The text either side of it stays at the paragraph's size: a span sets a size for its own range
        // and not for the paragraph.
        paragraph.EmSize.ShouldBe(Length.FromPoints(11));
        paragraph.Runs[0].EmSize.ShouldBe(Length.FromPoints(11));
        paragraph.Runs[^1].EmSize.ShouldBe(Length.FromPoints(11));
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    public void ARelativeSizeIsAFractionOfTheDefaultSize(string fileName)
    {
        // ODF only: the percentage is an ODF spelling, and every export of this document resolves it to
        // an absolute 24 pt on the way out — which is itself worth knowing, since it means a round trip
        // through any other format loses the relativity.
        PageParagraph paragraph = ParagraphStartingWith(fileName, "Seven.");

        // 200% of the item pool's twelve points — not 200 pt, and not twice the paragraph's eleven. ODF
        // writes the percentage in the same attribute as an absolute length, so the unit decides the
        // meaning; and LibreOffice resolves it against the pool default rather than against the enclosing
        // text, which a document rendered at 22 pt instead of 24 would prove.
        RunFor(paragraph, "containing").EmSize.ShouldBe(Length.FromPoints(24));

        // A bold span nested inside it inherits the size while overriding the weight. That is ordinary
        // containment inheritance, and it is the case a cascade resolved one property at a time gets wrong.
        PageRun nested = RunFor(paragraph, "bold");
        nested.EmSize.ShouldBe(Length.FromPoints(24));
        nested.Face.Weight.ShouldBeGreaterThanOrEqualTo(600);
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    [InlineData("mixed-runs.docx")]
    [InlineData("mixed-runs.doc")]
    [InlineData("mixed-runs.rtf")]
    public void AColouredSpanCarriesItsColour(string fileName)
    {
        PageParagraph paragraph = ParagraphStartingWith(fileName, "Six.");

        PageRun coloured = RunFor(paragraph, "coloured");
        coloured.EffectiveColour.ShouldBe(Colour.FromRgb(0xC9211E));

        // The text around it draws in the document's text colour rather than in nothing: a default colour
        // is transparent black, which would draw an invisible paragraph.
        paragraph.Runs[0].EffectiveColour.ShouldBe(Colour.Black);
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    [InlineData("mixed-runs.docx")]
    [InlineData("mixed-runs.doc")]
    [InlineData("mixed-runs.rtf")]
    public void AUniformParagraphCarriesNoRuns(string fileName)
    {
        // Not merely an optimisation. A paragraph split into runs it does not need loses the shaping
        // context at each boundary, so a kern pair straddling one would not apply and the line would
        // measure very slightly wide — which eventually moves a break. Every format states runs its own
        // way and every one of them restates formatting that has not changed, so this is the assertion
        // that each reader collapses those rather than passing them on.
        ParagraphStartingWith(fileName, "Eight.").HasRuns.ShouldBeFalse();
    }

    private static PageRun RunFor(PageParagraph paragraph, string word)
    {
        int at = paragraph.Text.IndexOf(word, StringComparison.Ordinal);
        at.ShouldBeGreaterThanOrEqualTo(0, $"'{word}' is in the corpus document");

        return paragraph.Runs
            .Where(run => run.Start <= at && at < run.End)
            .ShouldHaveSingleItem();
    }

    private static PageParagraph ParagraphStartingWith(string fileName, string prefix)
    {
        string path = Corpus.Require(fileName);

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Paragraphs
            .Where(paragraph => paragraph.Text.StartsWith(prefix, StringComparison.Ordinal))
            .ShouldHaveSingleItem();
    }
}
