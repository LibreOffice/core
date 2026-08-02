using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Blank paragraphs inside a table cell, which are content and not whitespace.
/// </summary>
/// <remarks>
/// <para>
/// A run of empty paragraphs is how a Word document pads a cell so that its block fills the rest of the
/// page, and it is common in exactly the documents where pagination matters most — forms, checklists and
/// equipment lists. The DOC layout assembler used to add a paragraph to the open cell only when it had
/// text or closed the cell, so a cell padded with five blank lines laid out as one.
/// </para>
/// <para>
/// The extraction pass never dropped them, so the two passes disagreed about the same document. That
/// disagreement is what this asserts, rather than a height in points: it holds whatever the fixture's
/// font and margins turn out to be, and it is the property that was actually violated. Measured on
/// <c>words/batch-018/doc/A_320.doc</c>, which pads each of its 106 entries with twenty-one blank
/// paragraphs: 100 pages against LibreOffice's 150 before, 135 after.
/// </para>
/// </remarks>
public sealed class BlankCellLineTests
{
    private const string Fixture = "table-blank-lines.doc";

    /// <summary>The laid-out cell holds every paragraph the extracted cell does.</summary>
    [Fact]
    public void ALaidOutCellHoldsAsManyParagraphsAsTheExtractedOne()
    {
        using IDocument document = Open();

        ContentTableCell first = Walk(document.Content).OfType<ContentTableCell>()
            .First(cell => cell is { Row: 0, Column: 0 });

        int extracted = Walk(first).OfType<ContentParagraph>().Count();
        extracted.ShouldBe(6, "one line of text and five blank ones");

        PageTable laid = LaidOutTables(document).First();
        laid.Rows[0].Cells[0].Blocks.OfType<PageParagraph>().Count().ShouldBe(extracted);
    }

    /// <summary>
    /// The padded cell is several lines taller than its one-line neighbour.
    /// </summary>
    /// <remarks>
    /// The consequence the page count depends on, asserted as a comparison between the two cells of one
    /// row rather than as a height in points — the dropped-paragraph bug made them equal.
    /// </remarks>
    [Fact]
    public void ThePaddedCellIsTallerThanItsNeighbour()
    {
        using IDocument document = Open();
        PageTable laid = LaidOutTables(document).First();

        laid.Rows[0].Cells.Count.ShouldBe(2);
        laid.Rows[0].Cells[0].Blocks.Count
            .ShouldBeGreaterThan(laid.Rows[0].Cells[1].Blocks.Count * 4);
    }

    private static IDocument Open()
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(Fixture)));

    private static IEnumerable<ContentNode> Walk(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode deeper in Walk(child)) yield return deeper;
        }
    }

    private static IEnumerable<PageTable> LaidOutTables(IDocument document)
        => ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Blocks.OfType<PageTable>();
}
