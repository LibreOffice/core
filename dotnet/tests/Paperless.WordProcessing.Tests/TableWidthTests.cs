using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks how a table whose columns state no width divides itself.
/// </summary>
/// <remarks>
/// <para>
/// ODF lets a column say nothing, and the answer is not the obvious one. Two documents with the same three
/// width-less columns come out differently depending on whether the <em>table</em> states a width: the one
/// that does not divides the text area evenly, and the one that says 17 cm gives its columns 3213, 2142 and
/// 4283 twips — a ratio of 3 : 2 : 4 out of three identical columns.
/// </para>
/// <para>
/// That second set looks like a bug in the reader and is not; it is what
/// <c>SwXMLTableContext::MakeTable</c> computes, and the ratio falls out of a loop that shrinks the space
/// remaining after each column while still dividing by the full sum of the weights. It is pinned here because
/// it is exactly the kind of number a later simplification would "correct" into an even division, which would
/// then disagree with every LibreOffice render of such a document by fifty points of column width.
/// </para>
/// </remarks>
public sealed class TableWidthTests
{
    [Fact]
    public void WidthlessColumnsDivideTheTextAreaEvenly()
    {
        // A4 with 2 cm margins: 9638 twips of text, which does not divide by three — so the remainder has to
        // land where Writer's own division puts it rather than being spread arbitrarily.
        Widths("table-autofit.fodt").ShouldBe([3212, 3213, 3213]);
    }

    [Fact]
    public void WidthlessColumnsInAStatedWidthDivideAsLibreOfficeDivides()
    {
        Widths("table-autofit-stated.fodt").ShouldBe([3212, 2142, 4284]);
    }

    [Fact]
    public void StatedColumnWidthsAreLeftAlone()
    {
        // The control: the same table with 3, 5 and 9 cm columns keeps them, so the resolution above cannot
        // be reached by a document that says what it wants.
        Widths("table-grid.fodt").ShouldBe([1701, 2835, 5102]);
    }

    private static List<long> Widths(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageTable table = pages.Blocks.OfType<PageTable>().First();

        return [.. table.ColumnWidths.Select(width => width.Twips)];
    }
}
