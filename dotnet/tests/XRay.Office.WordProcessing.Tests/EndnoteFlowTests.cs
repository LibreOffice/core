using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Checks where a document's endnotes go, which depends on the filter that opened it and not on the notes.
/// </summary>
/// <remarks>
/// <para>
/// Endnotes collecting at the end of the document are placed two ways, and LibreOffice chooses between
/// them on <c>DocumentSettingId::CONTINUOUS_ENDNOTES</c> — <em>"Render endnotes at the end of document
/// inline, rather than on a separate page"</em> (<c>sw/inc/strings.hrc</c>:1540). Set, the notes go into a
/// section frame inserted behind the last body content (<c>sw/source/core/layout/ftnfrm.cxx</c>:1644-1684);
/// clear, the <c>else</c> arm inserts a page marked <c>SetEndNotePage</c> and they start there.
/// </para>
/// <para>
/// <b>Both Word filters set it and neither ODF one nor the RTF one does</b>, so the same content pages
/// differently depending only on which spelling it was saved in. That is the assertion here: one document
/// in five formats, one page for the two Word ones and two for the other three.
/// </para>
/// <para>
/// <b>Measured, and it is the reference's own behaviour that moved.</b> Converted through both binaries to
/// FODT, <c>endnotes.docx</c>'s <c>ContinuousEndnotes</c> setting reads <c>false</c> under LibreOffice
/// 24.2.7.2 and <c>true</c> under 26.2.4.2, and the same file renders 2 pages against 1. The DOC has been
/// on the inline branch under both. <c>XRay.Office.Fidelity.Tests.EndnoteComparisonTests</c> compares this
/// against the installed binary; this file pins it without one.
/// </para>
/// </remarks>
public sealed class EndnoteFlowTests
{
    [Theory]
    [InlineData("endnotes.docx")]
    [InlineData("endnotes.doc")]
    public void AWordDocumentsEndnotesFollowItsBodyOnTheSamePage(string fileName)
    {
        IReadOnlyList<LaidOutPage> pages = Pages(fileName);

        pages.Count.ShouldBe(1, $"{fileName}: the notes take no page of their own");

        PlacedFlow notes = pages[^1].Notes.ShouldNotBeNull(
            $"{fileName}: the last page carries the endnote flow");

        // Below the body rather than bottom-aligned under it: the endnote section is body-flow content
        // that begins where the text ended, and the reference draws it there. Compared against the last
        // body line so the assertion is about the gap and not about this document's pagination.
        Length bodyBottom = pages[^1].Lines[^1].Top + pages[^1].Lines[^1].Box.Height;
        Length notesTop = notes.Area.Y + notes.Lines[0].Top;

        (notesTop - (pages[^1].BodyArea.Y + bodyBottom)).Points.ShouldBeInRange(
            0.0, 20.0,
            $"{fileName}: the notes begin just below the body, past the separator's reservation");
    }

    [Theory]
    [InlineData("endnotes.odt")]
    [InlineData("endnotes.fodt")]
    [InlineData("endnotes.rtf")]
    public void EveryOtherDocumentsEndnotesTakeAPageOfTheirOwn(string fileName)
    {
        IReadOnlyList<LaidOutPage> pages = Pages(fileName);

        pages.Count.ShouldBe(2, $"{fileName}: the notes start a page");

        pages[0].Notes.ShouldBeNull($"{fileName}: and take nothing off the page that cites them");
        pages[1].Lines.ShouldNotBeEmpty($"{fileName}: the endnote page holds the notes as body text");
    }

    // ------------------------------------------------------------------------- the machinery

    private static IReadOnlyList<LaidOutPage> Pages(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }
}
