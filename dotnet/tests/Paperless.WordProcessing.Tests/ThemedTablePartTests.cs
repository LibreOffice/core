using Paperless.Core.Graphics;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the two themed colours that sit on a table rather than on a run: <c>w:shd</c>'s
/// <c>w:themeFill</c> and a border's <c>w:themeColor</c>.
/// </summary>
/// <remarks>
/// <para>
/// The arithmetic is checked against LibreOffice in <c>ThemedTablePartComparisonTests</c>, which
/// pairs every case here with a shape stating the equivalent DrawingML chain. What is pinned
/// here is the resulting numbers, so that a change to the chain names which case moved without
/// needing an installed LibreOffice to say so — and so that the Word-facing half is tested on
/// its own: which of the four attributes each element reads, and what an absent one means.
/// </para>
/// <para>
/// <b>The trap this exists to catch.</b> <c>w:shd</c> is the one element in WordprocessingML
/// carrying two themed colours at once, and they are not the pair a reader expects.
/// <c>w:themeFill</c> is the shading's <em>background</em> — the colour a cell actually appears
/// in — while <c>w:themeColor</c> on the same element is the pattern's foreground, which shows
/// through only when <c>w:val</c> names a pattern. Reading the fill from <c>w:themeColor</c>
/// compiles, resolves, and gives a believable colour from the wrong slot.
/// </para>
/// </remarks>
public class ThemedTablePartTests
{
    /// <summary>The tables of <c>theme-table.docx</c>, in document order.</summary>
    private static List<PageTable> Tables()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("theme-table.docx"));
        using DocxFile file = DocxFile.Open(stream);

        DocxLayoutSource source = new(file.Styles, file.Settings, theme: file.Theme);

        return [.. source.Read(file.Body!).OfType<PageTable>()];
    }

    /// <summary>The first (and only) cell of each row, which is where every case sits.</summary>
    private static List<PageTableCell> Cells(PageTable table)
        => [.. table.Rows.Select(row => row.Cells[0])];

    [Fact]
    public void AThemedCellShadeResolvesThroughTheThemeWhenNoFillIsCached()
    {
        List<PageTableCell> cells = Cells(Tables()[0]);

        cells.Count.ShouldBe(6);

        // accent1 itself; the theme states 4F81BD.
        cells[0].Shading.ShouldBe(Colour.FromRgb(0x4F81BD));

        // w:themeFillShade="BF" is Word's "darker 25%", and it is a lumMod of 74902 in HSL
        // rather than a DrawingML a:shade — which would give a much darker 3A5F8B here. Word's
        // own picker labels this swatch 365F91; LibreOffice paints 376092 for the same chain and
        // Paperless agrees with LibreOffice, which is the one-unit-per-channel rounding
        // difference already measured across the 271 combinations in its test data.
        cells[1].Shading.ShouldBe(Colour.FromRgb(0x376092));

        // w:themeFillTint="99" is "lighter 40%": lumMod 60000 *and* lumOff 40000. Read as an
        // a:tint of 60% it would come out far paler.
        cells[2].Shading.ShouldBe(Colour.FromRgb(0x95B3D7));

        // 0x33 is exactly 20% of 255, so accent3 lightened 80%.
        cells[3].Shading.ShouldBe(Colour.FromRgb(0xEBF1DE));

        // background2, which reaches lt2 through the colour map rather than directly, darkened
        // 50% — 0x80 is 50196 in DrawingML units, not 50000.
        cells[4].Shading.ShouldBe(Colour.FromRgb(0x948A54));

        // A cached w:fill that disagrees with the theme reference beside it: the file says
        // accent1 darkened 25% (366092) and caches C0504D. The cache wins, because it is what
        // Word last computed and what every other reader paints.
        cells[5].Shading.ShouldBe(Colour.FromRgb(0xC0504D));
    }

    [Fact]
    public void AThemedBorderResolvesThroughTheThemeAndTheTablesReachesTheCell()
    {
        List<PageTableCell> cells = Cells(Tables()[1]);

        cells.Count.ShouldBe(5);

        cells[0].Borders.Bottom.Colour.ShouldBe(Colour.FromRgb(0xC0504D));
        cells[1].Borders.Bottom.Colour.ShouldBe(Colour.FromRgb(0x953735));
        cells[2].Borders.Bottom.Colour.ShouldBe(Colour.FromRgb(0xCCC1DA));
        cells[3].Borders.Bottom.Colour.ShouldBe(Colour.FromRgb(0x00B050));

        // No w:tcBorders at all on the last row, so its bottom edge is the table's own — which
        // is themed, and is the only way a themed w:tblBorders is ever observable.
        cells[4].Borders.Bottom.Colour.ShouldBe(Colour.FromRgb(0x4BACC6));

        // The sides are all `none`, so nothing else acquired a colour by accident: a border that
        // is not there must stay not there rather than becoming a black hairline.
        foreach (PageTableCell cell in cells)
        {
            cell.Borders.Top.Width.ShouldBe(Core.Units.Length.Zero);
            cell.Borders.Left.Width.ShouldBe(Core.Units.Length.Zero);
            cell.Borders.Right.Width.ShouldBe(Core.Units.Length.Zero);
        }
    }

    /// <summary>
    /// Without a theme part the reference resolves to nothing, rather than to a guess.
    /// </summary>
    /// <remarks>
    /// A shade that cannot be resolved has to leave the cell unshaded — inventing black would
    /// put a solid block behind the text. A border is the other way round: it is still a border,
    /// so it keeps its width and falls back to black, which is what a <c>w:color="auto"</c>
    /// border means anyway.
    /// </remarks>
    [Fact]
    public void WithoutAThemeAThemedShadeIsNothingAndAThemedBorderIsBlack()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("theme-table.docx"));
        using DocxFile file = DocxFile.Open(stream);

        DocxLayoutSource source = new(file.Styles, file.Settings, theme: null);
        List<PageTable> tables = [.. source.Read(file.Body!).OfType<PageTable>()];

        Cells(tables[0])[0].Shading.ShouldBeNull();
        Cells(tables[0])[5].Shading.ShouldBe(Colour.FromRgb(0xC0504D));

        Cells(tables[1])[0].Borders.Bottom.Colour.ShouldBe(Colour.Black);
        Cells(tables[1])[0].Borders.Bottom.Width.ShouldBe(Core.Units.Length.FromPoints(3));
    }
}
