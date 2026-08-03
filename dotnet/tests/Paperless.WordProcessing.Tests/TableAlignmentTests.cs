using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where a table's left edge goes when the document aligns it rather than indenting it.
/// </summary>
/// <remarks>
/// <para>
/// Two unrelated attributes say this and neither was read. <c>w:tblPr/w:jc</c> — a different element
/// from the paragraph alignment of the same name, and reachable only as a direct child of
/// <c>w:tblPr</c> — aligns an ordinary table; <c>w:tblpPr/@tblpXSpec</c> aligns a *positioned* one.
/// <c>ConversionHelper::convertTableJustification</c>
/// (<c>sw/source/writerfilter/dmapper/ConversionHelper.cxx:473</c>) maps <c>center</c> and
/// <c>right</c>/<c>end</c> onto orientations and leaves everything else on <c>LEFT_AND_WIDTH</c>, so
/// only those two move a table at all.
/// </para>
/// <para>
/// It matters most where it looks least likely to: a table <em>wider</em> than the text area. Left
/// where an indent puts it, its right-hand columns fall off the paper and their ink is clipped —
/// text that neither draws nor extracts, with nothing else to show for it. That is exactly how it
/// was found: <c>part-147_approval list_20230119.docx</c> in <c>words/batch-005</c> has an 11022-twip
/// table centred on a 9070-twip text area, and 30 of its cells' contents were being clipped away.
/// </para>
/// </remarks>
public sealed class TableAlignmentTests
{
    /// <summary>The single table of <c>table-centred.docx</c>.</summary>
    /// <remarks>
    /// Written by LibreOffice's own DOCX export from the flat-ODF beside it, which is why the fixture
    /// exercises <c>w:jc</c> and not <c>w:tblpPr</c>: the export writes a centred table as
    /// <c>&lt;w:jc w:val="center"/&gt;</c> and has no way to write the positioned form at all. The
    /// <c>w:tblpPr</c> half is covered by <see cref="LeftWithin"/> below and by the corpus measurement.
    /// </remarks>
    private static PageTable Table()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("table-centred.docx"));
        using DocxFile file = DocxFile.Open(stream);

        DocxLayoutSource source = new(file.Styles, file.Settings);
        return source.Read(file.Body!).OfType<PageTable>().Single();
    }

    [Fact]
    public void ACentredTableIsRead()
        => Table().HorizontalPosition.ShouldBe(FrameHorizontalAlignment.Centre);

    [Fact]
    public void ACentredTableWiderThanTheAreaOverhangsBothMarginsEqually()
    {
        PageTable table = Table();

        // The fixture's table is nine inches across a page that has nothing like nine inches of text
        // area, which is the case the whole thing exists for.
        Length area = Length.FromInches(6);
        table.Width.ShouldBeGreaterThan(area);

        Length left = table.LeftWithin(area);
        left.ShouldBeLessThan(Length.Zero);

        // Equal overhang: as far off the left edge as it runs past the right one.
        Length right = left + table.WidthWithin(area) - area;
        Math.Abs((right + left).Emu).ShouldBeLessThanOrEqualTo(1);
    }

    [Fact]
    public void AnUnalignedTableKeepsItsIndent()
    {
        PageTable table = new()
        {
            ColumnWidths = [Length.FromInches(2)],
            Rows = [],
            LeftIndent = Length.FromInches(1),
        };

        table.HorizontalPosition.ShouldBeNull();
        table.LeftWithin(Length.FromInches(6)).ShouldBe(Length.FromInches(1));
    }

    [Theory]
    [InlineData(FrameHorizontalAlignment.Left, 0)]
    [InlineData(FrameHorizontalAlignment.Centre, 2)]
    [InlineData(FrameHorizontalAlignment.Right, 4)]
    public void LeftWithin(FrameHorizontalAlignment alignment, double inchesFromTheEdge)
    {
        PageTable table = new()
        {
            ColumnWidths = [Length.FromInches(2)],
            Rows = [],

            // Deliberately non-zero: an aligned table ignores its indent, which is what
            // LEFT_AND_WIDTH being the *other* branch means.
            LeftIndent = Length.FromInches(1),
            HorizontalPosition = alignment,
        };

        table.LeftWithin(Length.FromInches(6)).ShouldBe(Length.FromInches(inchesFromTheEdge));
    }
}
