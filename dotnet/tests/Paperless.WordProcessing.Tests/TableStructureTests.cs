using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests table structure across all four word-processing readers: nesting, spans and header rows.
/// </summary>
/// <remarks>
/// <para>
/// One document in five files — a flat ODF original and the odt, doc, docx and rtf LibreOffice
/// converted it to. It holds the three things a table reader gets wrong independently of the text: a
/// table nested inside a cell, a horizontal span, and a vertical span.
/// </para>
/// <para>
/// Each format expresses all three differently, and every one of them has a way of being read that
/// produces plausible output rather than an error. A nested table's cells are extra columns of the
/// enclosing row if the depth is ignored; a merge LibreOffice writes carries no flag at all and has to
/// come from the column grid; a vertical merge is marked on the continuation rather than on the cell
/// that spans. So the strongest assertion is that all five agree, which is what
/// <see cref="EveryFormatProducesTheSameStructure"/> checks.
/// </para>
/// </remarks>
public class TableStructureTests
{
    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static List<ContentTable> TopLevelTables(IDocument document)
        => [.. document.Content.Children.OfType<ContentSection>()
                .SelectMany(s => s.Children).OfType<ContentTable>()];

    [Theory]
    [InlineData("tables.fodt")]
    [InlineData("tables.odt")]
    [InlineData("tables.doc")]
    [InlineData("tables.docx")]
    [InlineData("tables.rtf")]
    public void ANestedTableNestsRatherThanFlattening(string name)
    {
        using IDocument document = Open(name);
        List<ContentTable> tables = TopLevelTables(document);

        tables.Count.ShouldBe(2, $"{name}: the nested table belongs inside a cell, not beside it");

        ContentTable outer = tables[0];
        outer.ColumnCount.ShouldBe(2, $"{name}: the nested table's cells are not the outer table's");

        ContentTableCell hostCell = (ContentTableCell)((ContentTableRow)outer.Children[1]).Children[0];
        ContentTable inner = hostCell.Children.OfType<ContentTable>().Single();

        inner.ColumnCount.ShouldBe(2);
        inner.Children.Count.ShouldBe(2);
        inner.GetText().ShouldBe("Inner one\tInner two\nInner three\tInner four\n");

        // The cell holds text on both sides of the nested table, so a reader that closes the inner
        // table too early or too late loses one of them.
        hostCell.Children.OfType<ContentParagraph>().Count().ShouldBe(2);
        hostCell.GetText().ShouldStartWith("Outer body left");
        hostCell.GetText().ShouldEndWith("After the inner table.");
    }

    [Theory]
    [InlineData("tables.fodt")]
    [InlineData("tables.odt")]
    [InlineData("tables.doc")]
    [InlineData("tables.docx")]
    [InlineData("tables.rtf")]
    public void HorizontalAndVerticalSpansBecomeOneCellEach(string name)
    {
        using IDocument document = Open(name);
        ContentTable spans = TopLevelTables(document)[1];

        spans.ColumnCount.ShouldBe(3);
        List<ContentTableRow> rows = [.. spans.Children.Cast<ContentTableRow>()];
        rows.Count.ShouldBe(2);

        // Two cells, not three: the covered column is part of the wide cell.
        rows[0].Children.Count.ShouldBe(2);
        ContentTableCell wide = (ContentTableCell)rows[0].Children[0];
        wide.ColumnSpan.ShouldBe(2);
        wide.GetText().ShouldBe("Wide cell");

        // The tall cell spans two rows, and the row below has no cell in its column.
        ContentTableCell tall = (ContentTableCell)rows[0].Children[1];
        tall.Column.ShouldBe(2);
        tall.RowSpan.ShouldBe(2);
        tall.GetText().ShouldBe("Tall cell");

        rows[1].Children.Count.ShouldBe(2);
        rows[1].GetText().ShouldBe("Plain a\tPlain b\n");
    }

    [Theory]
    [InlineData("tables.fodt")]
    [InlineData("tables.odt")]
    [InlineData("tables.doc")]
    [InlineData("tables.docx")]
    public void ARepeatingHeaderRowIsReportedAsOne(string name)
    {
        using IDocument document = Open(name);

        // Only the run of header rows at the top counts, so a table whose first row repeats reports
        // one however many rows follow it.
        TopLevelTables(document)[0].HeaderRowCount.ShouldBe(1);
        TopLevelTables(document)[1].HeaderRowCount.ShouldBe(0);
    }

    [Fact]
    public void RtfLosesTheHeaderRowBecauseLibreOfficeDoesNotWriteIt()
    {
        // Not a defect in the reader: LibreOffice's RTF export omits \trhdr for this document, so the
        // file does not say the row repeats. Paperless reports what the file says rather than what the
        // document it came from said. The reader does honour \trhdr — see
        // RtfReaderTests.ARepeatingHeaderRowComesFromTrhdr.
        using IDocument document = Open("tables.rtf");
        TopLevelTables(document)[0].HeaderRowCount.ShouldBe(0);
    }

    [Fact]
    public void EveryFormatProducesTheSameStructure()
    {
        string expected = Shape(Open("tables.fodt"));

        foreach (string name in new[] { "tables.odt", "tables.doc", "tables.docx", "tables.rtf" })
        {
            using IDocument document = Open(name);
            Shape(document).ShouldBe(expected, name);
        }
    }

    /// <summary>
    /// A document's table structure as text, ignoring everything the formats legitimately differ on.
    /// </summary>
    /// <remarks>
    /// Style names are excluded because DOC names the default paragraph style and ODF leaves it
    /// implicit, and the header-row count because LibreOffice's RTF export drops it — both are
    /// differences between the files rather than between the readers.
    /// </remarks>
    private static string Shape(IDocument document)
    {
        StringBuilder shape = new();
        Walk(document.Content, 0, shape);
        return shape.ToString();

        static void Walk(ContentNode node, int depth, StringBuilder shape)
        {
            string? label = node switch
            {
                ContentTable t => $"table cols={t.ColumnCount}",
                ContentTableRow r => $"row {r.Index}",
                ContentTableCell c => $"cell {c.Row},{c.Column} {c.ColumnSpan}x{c.RowSpan}",
                ContentParagraph p => $"paragraph \"{p.GetText().Trim()}\"",
                ContentRun => null,
                _ => node.GetType().Name,
            };

            if (label is not null)
            {
                shape.Append(new string(' ', depth * 2)).Append(label).Append('\n');
            }

            foreach (ContentNode child in node.Children) Walk(child, depth + 1, shape);
        }
    }
}
