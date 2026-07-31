using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a footnote is read as a note anchored in a paragraph, whatever format states it.
/// </summary>
/// <remarks>
/// <para>
/// The four formats keep their notes in four different places — an inline <c>text:note</c>, a
/// <c>footnotes.xml</c> part keyed by <c>w:id</c>, a WW8 subdocument indexed by a
/// <c>PlcffndRef</c>/<c>PlcffndTxt</c> pair, and an RTF <c>{\*\footnote}</c> group sitting mid-sentence —
/// and all four have to arrive as a <see cref="PageNote"/> hanging off the paragraph that cites it, because
/// that is what makes the note a pagination matter: the page holding the citation is the page carrying the
/// note.
/// </para>
/// <para>
/// This exists as a structural test rather than only a rendering comparison because RTF cannot have one.
/// LibreOffice's own RTF import drops the character and paragraph formatting stated inside a footnote group
/// and falls back to the document's defaults — a note the file sets in Carlito at 10 pt renders in Liberation
/// Serif, with a hanging indent the file does not ask for. Paperless reads what the file says, so a
/// word-position comparison against LibreOffice would fail on a note that was read correctly. What can still
/// be checked is everything below: that the note is there, that it hangs off the right paragraph, that it is
/// numbered by counting rather than by believing the file, and that its body reads correctly.
/// </para>
/// </remarks>
public sealed class FootnoteReadingTests
{
    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.rtf")]
    [InlineData("footnotes.doc")]
    public void ANoteHangsOffTheParagraphThatCitesIt(string fileName)
    {
        List<PageParagraph> paragraphs = Paragraphs(fileName);
        List<PageParagraph> citing = [.. paragraphs.Where(p => p.Notes.Count > 0)];

        // The corpus document cites two notes, from two paragraphs, and states their citations as 2 and 5 so
        // that a reader taking the file at its word gets the numbering wrong.
        citing.Count.ShouldBe(2, $"{fileName}: expected two paragraphs to cite a note");

        for (int i = 0; i < citing.Count; i++)
        {
            PageNote note = citing[i].Notes.ShouldHaveSingleItem();

            note.IsEndnote.ShouldBeFalse($"{fileName}: note {i + 1} is a footnote, not an endnote");

            // The anchor is inside the paragraph rather than at either end of it: the citation sits after the
            // first sentence, and an offset off the end would put the note on whichever page the paragraph
            // ended on rather than the page the citation is on.
            note.Offset.ShouldBeGreaterThan(0, $"{fileName}: note {i + 1}'s anchor is at the very start");
            note.Offset.ShouldBeLessThan(
                citing[i].Text.Length, $"{fileName}: note {i + 1}'s anchor is past its paragraph's text");

            PageParagraph body = note.Blocks.ShouldHaveSingleItem().ShouldBeOfType<PageParagraph>();

            // Numbered by counting in document order — 1 and 2 — and drawn at the head of the note's own
            // first line, which is where LibreOffice draws it and where none of the four files has it.
            body.Text.ShouldBe(
                $"{i + 1}Note {(i == 0 ? 2 : 5)} text alpha bravo charlie.",
                $"{fileName}: note {i + 1}'s body should open with its own counted number, and the "
                + "number the file states should survive only as part of the text");

            // The note is set smaller than the body, which is what makes it take less room than the text it
            // was pushed out of — and what a reader that lost the note's own style would get wrong.
            body.EmSize.ShouldBeLessThan(
                citing[i].EmSize, $"{fileName}: note {i + 1} is not set smaller than the body");
        }
    }

    [Theory]
    [InlineData("endnotes.fodt")]
    [InlineData("endnotes.odt")]
    [InlineData("endnotes.docx")]
    [InlineData("endnotes.doc")]
    [InlineData("endnotes.rtf")]
    public void AnEndnoteIsMarkedAndNumberedInRomanNumerals(string fileName)
    {
        List<PageParagraph> citing = [.. Paragraphs(fileName).Where(p => p.Notes.Count > 0)];

        citing.Count.ShouldBe(2, $"{fileName}: expected two paragraphs to cite a note");

        for (int i = 0; i < citing.Count; i++)
        {
            PageNote note = citing[i].Notes.ShouldHaveSingleItem();

            note.IsEndnote.ShouldBeTrue(
                $"{fileName}: note {i + 1} is an endnote, and placing it as a footnote would take room off "
                + "the page that cites it");

            // Lower-roman, which is LibreOffice's default for endnotes and *not* what it uses for footnotes.
            // Measured: a two-endnote document renders "i" and "ii", in the sentence and at the note alike.
            string expected = i == 0 ? "i" : "ii";

            note.Blocks.ShouldHaveSingleItem().ShouldBeOfType<PageParagraph>().Text.ShouldBe(
                $"{expected}Endnote {(i == 0 ? 2 : 5)} text alpha bravo charlie.",
                $"{fileName}: endnote {i + 1} should be cited in lower-roman");

            citing[i].Runs.First(run => run.Start == note.Offset).Length.ShouldBe(
                expected.Length, $"{fileName}: the anchor's citation should be as long as its numeral");
        }
    }

    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.rtf")]
    [InlineData("footnotes.doc")]
    public void ACitationIsARaisedRunOfItsOwn(string fileName)
    {
        List<PageParagraph> paragraphs = Paragraphs(fileName);
        PageParagraph citing = paragraphs.First(p => p.Notes.Count > 0);

        citing.HasRuns.ShouldBeTrue($"{fileName}: a paragraph citing a note is not uniform");

        PageRun citation = citing.Runs.First(run => run.Start == citing.Notes[0].Offset);

        // Both halves, because a document can set either alone and a reader that applied only the shift draws
        // the number full size — 11 pt where LibreOffice draws 6.4, which pushes the rest of the line along.
        citation.Rise.ShouldBeGreaterThan(
            Core.Units.Length.Zero, $"{fileName}: the citation is not raised");

        citation.EmSize.ShouldBeLessThan(
            citing.EmSize, $"{fileName}: the citation is not set smaller than its sentence");
    }

    [Theory]
    [InlineData("note-numbering.fodt")]
    [InlineData("note-numbering.odt")]
    [InlineData("note-numbering.docx")]
    [InlineData("note-numbering.doc")]
    [InlineData("note-numbering.rtf")]
    public void ADocumentCanStateItsOwnSequenceAndStartValue(string fileName)
    {
        List<PageParagraph> citing = [.. Paragraphs(fileName).Where(p => p.Notes.Count > 0)];

        citing.Count.ShouldBe(2, $"{fileName}: expected two paragraphs to cite a note");

        // The corpus document asks for upper roman from eight, so the two notes are VIII and IX. Both halves
        // matter and both are easy to get wrong: a reader that ignored the format numbers them 8 and 9, and one
        // that took ODF's `text:start-value` for the first number rather than an offset gets VII and VIII.
        string[] expected = ["VIII", "IX"];

        for (int i = 0; i < citing.Count; i++)
        {
            PageNote note = citing[i].Notes.ShouldHaveSingleItem();

            note.Blocks.ShouldHaveSingleItem().ShouldBeOfType<PageParagraph>().Text.ShouldBe(
                $"{expected[i]}Note {(i == 0 ? 2 : 5)} text alpha bravo charlie.",
                $"{fileName}: note {i + 1} should be cited \"{expected[i]}\"");

            citing[i].Runs.First(run => run.Start == note.Offset).Length.ShouldBe(
                expected[i].Length,
                $"{fileName}: the anchor's citation should be as long as \"{expected[i]}\"");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>Every paragraph of a document's body, tables flattened away.</summary>
    private static List<PageParagraph> Paragraphs(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Blocks.OfType<PageParagraph>()];
    }
}
