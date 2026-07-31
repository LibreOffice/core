using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a right-to-left <em>section</em> fills its rightmost column first.
/// </summary>
/// <remarks>
/// <para>
/// A section's direction is a separate statement from its paragraphs' — <c>w:sectPr/w:bidi</c>,
/// <c>style:writing-mode</c> on the page layout, <c>sprmSFBiDi</c> — and the one thing it changes
/// that shows on a page is the order of its columns. The corpus document is deliberately ordinary
/// otherwise: one plain Latin paragraph, left to right, so nothing but the section can move it.
/// </para>
/// <para>
/// Three formats and not four. LibreOffice's RTF export drops the section's direction entirely —
/// the converted file carries <c>\cols2</c> and no <c>\rtlsect</c> at all — so an RTF round trip
/// has nothing to read. The control word is still handled, because a file written by anything else
/// may carry it, and LibreOffice's own importer maps it too
/// (<c>sw/source/writerfilter/rtftok/rtfdispatchflag.cxx:653</c>).
/// </para>
/// </remarks>
public sealed class BidiColumnTests
{
    [Theory]
    [InlineData("bidi-columns.fodt")]
    [InlineData("bidi-columns.docx")]
    [InlineData("bidi-columns.doc")]
    public void TheFirstColumnOfARightToLeftSectionIsTheRightmost(string fileName)
    {
        LaidOutPage page = FirstPageOf(fileName);

        page.ColumnCount.ShouldBe(2, $"{fileName}: the section's columns");
        page.IsRightToLeft.ShouldBeTrue($"{fileName}: the section reads right to left");

        page.ColumnArea(0).X.ShouldBeGreaterThan(
            page.ColumnArea(1).X, $"{fileName}: column one is to the right of column two");

        // And it is the right-hand column of the page rather than merely the further one: its right
        // edge is the text area's. A reversal that also moved the columns would pass the test above.
        // Within a twip, because a column's width is the text area's divided by the count and the
        // remainder of that division lands on the last column.
        page.ColumnArea(0).Right.Points.ShouldBe(
            page.BodyArea.Right.Points, 0.05, $"{fileName}: column one reaches the right margin");
    }

    [Theory]
    [InlineData("bidi-columns.fodt")]
    [InlineData("bidi-columns.docx")]
    [InlineData("bidi-columns.doc")]
    public void ItsLinesAreDrawnInThatColumn(string fileName)
    {
        LaidOutPage page = FirstPageOf(fileName);

        page.Lines.ShouldNotBeEmpty($"{fileName}: the paragraph laid out");
        page.Lines[0].Column.ShouldBe(0, $"{fileName}: the first line is in the first column");

        // 319 pt, which is where LibreOffice draws it — see the corpus document's own comment.
        page.ColumnArea(page.Lines[0].Column).X.Points.ShouldBe(
            319.0, 0.5, $"{fileName}: where the first line's column starts");
    }

    private static LaidOutPage FirstPageOf(string fileName)
    {
        string path = Corpus.Require(fileName);

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Pages[0];
    }
}
