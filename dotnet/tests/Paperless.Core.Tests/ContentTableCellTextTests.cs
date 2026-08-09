using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// A table cell's text drops the newline its last paragraph contributes, and nothing else.
/// </summary>
/// <remarks>
/// <para>
/// Every <see cref="ContentParagraph"/> terminates itself, so <see cref="ContentTableCell"/> has
/// to remove one newline for a row to be able to join its cells onto a line. It used to remove
/// every trailing newline, and the second one it took was not a terminator but an <em>empty final
/// paragraph</em> — a hard break at the end of a cell, which every spreadsheet format can express.
/// </para>
/// <para>
/// The consequence was a row a line short. Calc reserves a whole line for that paragraph:
/// measured on <c>flightstandards-doc-Cross-reference-table_version02.xlsx</c>, whose D936 ends
/// <c>…GENERAL\r\n</c>, LibreOffice's own flat-ODF export states <c>style:row-height</c> for that
/// row as 700.7 twips — three lines — against the two a cell with the paragraph erased asks for.
/// Thirty-eight of that document's forty-five rows disagreeing with LibreOffice were this one
/// character.
/// </para>
/// </remarks>
public sealed class ContentTableCellTextTests
{
    private static ContentTableCell Cell(params string[] paragraphs)
    {
        ContentTableCell cell = new();
        foreach (string text in paragraphs)
        {
            ContentParagraph paragraph = new();
            paragraph.Children.Add(new ContentRun { Text = text });
            cell.Children.Add(paragraph);
        }

        return cell;
    }

    /// <summary>The ordinary case, unchanged: one paragraph, no trailing newline.</summary>
    [Fact]
    public void OneParagraphKeepsNoTerminator() => Cell("Alpha").GetText().ShouldBe("Alpha");

    /// <summary>Paragraphs are joined by the newline each of them but the last contributes.</summary>
    [Fact]
    public void ParagraphsAreJoinedByOneNewline()
        => Cell("Alpha", "Bravo", "Charlie").GetText().ShouldBe("Alpha\nBravo\nCharlie");

    /// <summary>
    /// An empty final paragraph survives as the trailing newline that is its own terminator's
    /// predecessor.
    /// </summary>
    /// <remarks>
    /// This is the assertion the fix exists for. Stripping every trailing newline gives
    /// <c>Alpha\nBravo</c>, which is a different document: two lines rather than three.
    /// </remarks>
    [Fact]
    public void AnEmptyFinalParagraphSurvivesAsATrailingNewline()
        => Cell("Alpha", "Bravo", string.Empty).GetText().ShouldBe("Alpha\nBravo\n");

    /// <summary>Two empty final paragraphs are two lines, not one and not none.</summary>
    [Fact]
    public void TwoEmptyFinalParagraphsSurvive()
        => Cell("Alpha", string.Empty, string.Empty).GetText().ShouldBe("Alpha\n\n");

    /// <summary>A cell holding nothing but an empty paragraph is still empty text.</summary>
    /// <remarks>
    /// The guard that keeps the removal from running past the start of this cell's own
    /// contribution, which matters because a row appends its cells into one builder.
    /// </remarks>
    [Fact]
    public void AnEmptyCellIsStillEmpty() => Cell(string.Empty).GetText().ShouldBe(string.Empty);

    /// <summary>
    /// A row joins its cells onto one line, which is the behaviour the single removal exists to
    /// keep.
    /// </summary>
    [Fact]
    public void ARowStillJoinsItsCells()
    {
        ContentTableRow row = new();
        row.Children.Add(Cell("Alpha"));
        row.Children.Add(Cell("Bravo"));

        row.GetText().TrimEnd('\n').ShouldNotContain("\n");
    }
}
