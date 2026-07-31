using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a cell's shading and its four borders are read, in every format that can state them.
/// </summary>
/// <remarks>
/// <para>
/// Structural rather than positional, which is the point of it: the comparison tests check where the fills and
/// the strokes land against LibreOffice's own render, and that needs LibreOffice installed. These check what
/// the reader made of the file, which is where the per-format traps are — and every one of the four spellings
/// has one. DOCX puts the colour in <c>w:fill</c> and the <em>pattern's</em> colour in <c>w:color</c>; RTF
/// separates <c>\clcbpat</c> from <c>\clcfpat</c> the same way; ODF uses a CSS shorthand for the border and a
/// separate <c>none</c> that has to beat it. DOC is the odd one and needs the most: it states a shade as a
/// foreground, a background and a <em>pattern index</em>, so the colour the document means has to be blended
/// out of the three, and its borders arrive as a border code applied to a range of cells.
/// </para>
/// <para>
/// Reading the shade as the pattern's foreground rather than blending it would give black where the document
/// says light grey — a difference no positional test would catch, since a fill in the right place is still in
/// the right place whatever colour it is.
/// </para>
/// </remarks>
public sealed class TableDecorationTests
{
    /// <summary>The grey the corpus shades its first row with.</summary>
    private static readonly Colour Grey = Colour.FromRgb(0xCCCCCC);

    /// <summary>The red the corpus draws its borders in.</summary>
    private static readonly Colour Red = Colour.FromRgb(0xFF0000);

    [Theory]
    [InlineData("table-shading.fodt")]
    [InlineData("table-shading.odt")]
    [InlineData("table-shading.docx")]
    [InlineData("table-shading.rtf")]
    [InlineData("table-shading.doc")]
    public void AShadedCellKnowsWhichColourItIs(string fileName)
    {
        PageTable table = Table(fileName);

        // The first row is shaded and the rest are not, which is what makes this more than a null check: a
        // reader taking a table-level default and applying it everywhere would shade all four rows.
        foreach (PageTableCell cell in table.Rows[0].Cells)
        {
            cell.Shading.ShouldNotBeNull($"{fileName}: a cell of the shaded row");
            cell.Shading.Value.ShouldBe(Grey, $"{fileName}: the shade's colour");
        }

        foreach (PageTableRow row in table.Rows.Skip(1))
        {
            foreach (PageTableCell cell in row.Cells)
            {
                cell.Shading.ShouldBeNull($"{fileName}: a cell of an unshaded row");
            }
        }
    }

    [Theory]
    [InlineData("table-borders.fodt")]
    [InlineData("table-borders.odt")]
    [InlineData("table-borders.docx")]
    [InlineData("table-borders.rtf")]
    [InlineData("table-borders.doc")]
    public void ABorderedCellKnowsAllFourOfItsEdges(string fileName)
    {
        PageTable table = Table(fileName);
        Length half = Length.FromPoints(0.5);

        foreach (PageTableRow row in table.Rows)
        {
            foreach (PageTableCell cell in row.Cells)
            {
                foreach ((string side, TableBorder border) in Sides(cell.Borders))
                {
                    border.Width.ShouldBe(half, $"{fileName}: the {side} border's width");
                    border.Colour.ShouldBe(Red, $"{fileName}: the {side} border's colour");
                }
            }
        }
    }

    [Theory]
    [InlineData("table-grid.fodt")]
    [InlineData("table-grid.odt")]
    [InlineData("table-grid.docx")]
    [InlineData("table-grid.rtf")]
    [InlineData("table-grid.doc")]
    public void ACellWithNoDecorationSaysSo(string fileName)
    {
        // The other half of the same assertion, and the one that fails when a reader invents a default: this
        // table states neither shading nor borders, so every cell must come back with none of either.
        foreach (PageTableRow row in Table(fileName).Rows)
        {
            foreach (PageTableCell cell in row.Cells)
            {
                cell.Shading.ShouldBeNull($"{fileName}: an undecorated cell's shading");
                cell.Borders.IsNone.ShouldBeTrue($"{fileName}: an undecorated cell's borders");
            }
        }
    }

    // ------------------------------------------------------------------------- the machinery

    private static IEnumerable<(string Side, TableBorder Border)> Sides(CellBorders borders)
    {
        yield return ("left", borders.Left);
        yield return ("right", borders.Right);
        yield return ("top", borders.Top);
        yield return ("bottom", borders.Bottom);
    }

    private static PageTable Table(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().First();
    }
}
