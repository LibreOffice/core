using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The <c>w:tcPr</c> half of a table style's conditional layers — the cell shading a
/// <c>w:tblStylePr</c> carries, and the row and column banding that decides which cells get it.
/// </summary>
/// <remarks>
/// <para>
/// Round 62 established what was missing and where. On
/// <c>012_Project_Timeline_Template_Black_and_Brown_Theme</c> the reference draws <b>75 filled
/// rectangles on page 1 against our 19</b>, and the 56 missing ones are <em>not</em> <c>w:shd</c>:
/// the document holds twelve <c>w:shd</c> elements and we drew twelve fills from them. They come
/// from <c>PlainTable5</c>'s <c>firstCol</c> and <c>band1Horz</c> layers, whose <c>w:tcPr</c> this
/// reader never read — <c>WordStyle</c> skipped any layer with no <c>w:rPr</c>, and
/// <c>WordTableStyleConditions.Names</c> did not offer the band layers at all.
/// </para>
/// <para>
/// The fixture is authored rather than borrowed, and its answer is read off <b>LibreOffice
/// 26.2.4.2's own rendering</b> (<c>probes/words-r63/make-band-fixture.py</c>). Six rows by three
/// columns under one style stating a distinct fill on four layers at once, with
/// <c>w:tblStyleRowBandSize="2"</c> — so the reference's own fill operators name which layer won,
/// cell by cell, and four rules are separated that no one-layer fixture can separate:
/// </para>
/// <list type="bullet">
/// <item>a heading row takes <c>firstRow</c> and <b>no band at all</b>, rather than band nought;</item>
/// <item>a band size of two pairs the body rows, so ignoring <c>w:tblStyleRowBandSize</c> gets four
/// of the five body rows wrong;</item>
/// <item>the leading column takes <c>firstCol</c> over either band, an edge layer being the more
/// specific;</item>
/// <item>a cell's own <c>w:shd</c> beats every layer, and the fixture's last cell states one.</item>
/// </list>
/// <para>
/// The reference paints each cell's fill <b>twice</b> and adds one <c>#EDEDED</c> for the style's
/// unconditional <c>w:style/w:tcPr</c>, so its 37 operators are this fixture's 18 cells doubled
/// plus one. What is asserted here is the colour each cell resolves to, which is the thing the two
/// stacks can disagree about; the doubling is LibreOffice painting a cell background and a table
/// background over one another and is not a divergence.
/// </para>
/// </remarks>
public sealed class ConditionalCellShadingTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static readonly Colour FirstRowFill = new(0x44, 0x72, 0xC4);
    private static readonly Colour FirstColumnFill = new(0xFF, 0xF2, 0xCC);
    private static readonly Colour Band1Fill = new(0xD9, 0xE2, 0xF3);
    private static readonly Colour Band2Fill = new(0xFB, 0xE4, 0xD5);
    private static readonly Colour OwnFill = new(0x00, 0xB0, 0xF0);

    /// <summary>
    /// Every cell of the fixture resolves to the fill LibreOffice paints there.
    /// </summary>
    /// <remarks>
    /// One assertion per cell rather than a count, because a count is satisfied by the right colours
    /// in the wrong places — and the band rule is precisely a claim about *which* rows.
    /// </remarks>
    [Fact]
    public void EveryCellTakesTheLayerTheReferencePaints()
    {
        Dictionary<string, Colour?> shading = FixtureShading();

        for (int column = 0; column < 3; column++)
        {
            shading[$"R0C{column}"].ShouldBe(FirstRowFill, $"R0C{column} is in the heading row");
        }

        for (int row = 1; row < 6; row++)
        {
            shading[$"R{row}C0"].ShouldBe(FirstColumnFill, $"R{row}C0 is in the leading column");
        }

        // Body rows 1 and 2 are band 1, rows 3 and 4 are band 2, row 5 is band 1 again — which is
        // `w:tblStyleRowBandSize="2"` doing the work, and the heading row excluded from the count.
        foreach (string cell in (string[])["R1C1", "R1C2", "R2C1", "R2C2", "R5C1"])
        {
            shading[cell].ShouldBe(Band1Fill, $"{cell} is in an odd-numbered band");
        }

        foreach (string cell in (string[])["R3C1", "R3C2", "R4C1", "R4C2"])
        {
            shading[cell].ShouldBe(Band2Fill, $"{cell} is in an even-numbered band");
        }

        shading["R5C2"].ShouldBe(OwnFill, "the cell states its own w:shd, which is direct formatting");
    }

    /// <summary>
    /// The heading row is excluded from the band count, not counted as band nought.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The single assertion that separates the two readings, stated on its own so a failure names
    /// it. On <c>012</c>, whose band size is one, counting the heading row moves all 48 of its band
    /// fills from table rows 2, 4, 6 and 8 to rows 3, 5, 7 and 9.
    /// </para>
    /// <para>
    /// <strong>It has to be asked of the <em>second</em> body row, and the first draft asked the
    /// first.</strong> The fixture's band size is two, so counting the heading row leaves row 1 in
    /// band 0 either way and the test passed under its own mutation —
    /// <c>verify-test.sh</c> caught that: the defect was detected only by the whole-fixture test
    /// beside it, which is precisely the "assert the thing you named" failure. Rows 2 and 4 are the
    /// two the shift moves.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheHeadingRowIsNotCountedAsABand()
    {
        Dictionary<string, Colour?> shading = FixtureShading();

        shading["R2C1"].ShouldBe(Band1Fill, "body rows 1 and 2 are band 0 at a band size of two");
        shading["R4C1"].ShouldBe(Band2Fill, "body rows 3 and 4 are band 1");
    }

    /// <summary>A cell in the heading row is in no band, whatever its position.</summary>
    [Fact]
    public void ARowInAnEdgeRegionTakesNoBand()
    {
        WordTableStyleConditions heading = new(
            new WordTableLook(FirstRow: true, LastRow: false, FirstColumn: false, LastColumn: false,
                              HorizontalBanding: true, VerticalBanding: true),
            IsFirstRow: true, IsLastRow: false, IsFirstColumn: false, IsLastColumn: false,
            RowBand: null, ColumnBand: 0);

        heading.Names.ShouldBe(["firstRow", "band1Vert", "wholeTable"]);
    }

    /// <summary>The bands sit between the edge layers and the unconditional one.</summary>
    /// <remarks>
    /// §17.7.6 applies whole table, then the vertical bands, then the horizontal bands, then the
    /// columns, then the rows, then the corners — so reversed, a band must come after both edges
    /// and before <c>wholeTable</c>. A style whose <c>firstCol</c> and <c>band1Horz</c> both state a
    /// fill is exactly what <c>012</c> is, and the order is what puts white in its leading column
    /// instead of grey.
    /// </remarks>
    [Fact]
    public void ABandIsLessSpecificThanAnEdgeAndMoreThanTheWholeTable()
    {
        WordTableStyleConditions cell = new(
            new WordTableLook(FirstRow: false, LastRow: false, FirstColumn: true, LastColumn: false,
                              HorizontalBanding: true, VerticalBanding: false),
            IsFirstRow: false, IsLastRow: false, IsFirstColumn: true, IsLastColumn: false,
            RowBand: 1, ColumnBand: null);

        cell.Names.ShouldBe(["firstCol", "band2Horz", "wholeTable"]);
    }

    /// <summary>
    /// <c>noHBand</c> and <c>noVBand</c> are read the other way up, and the mask spelling too.
    /// </summary>
    /// <remarks>
    /// The one asymmetry in <c>w:tblLook</c>: the attribute says <em>no</em> banding, so a reader
    /// that treats it like <c>firstRow</c> bands every table that switches banding off and none that
    /// switches it on. <c>012</c>'s own look is <c>noHBand="0" noVBand="1"</c>, which is horizontal
    /// banding on and vertical off, and reading it backwards would put 48 grey fills down its
    /// columns instead of across its rows.
    /// </remarks>
    [Fact]
    public void TheBandBitsAreStatedAsProhibitions()
    {
        WordTableLook named = WordTableLook.Read(XElement.Parse(
            $"""<w:tblPr xmlns:w="{W}"><w:tblLook w:firstRow="1" w:noHBand="0" w:noVBand="1"/></w:tblPr>"""));

        named.HorizontalBanding.ShouldBeTrue("noHBand=\"0\"");
        named.VerticalBanding.ShouldBeFalse("noVBand=\"1\"");

        // 0x0400 is noVBand; 0x0200 is clear, so horizontal banding stays on.
        WordTableLook mask = WordTableLook.Read(XElement.Parse(
            $"""<w:tblPr xmlns:w="{W}"><w:tblLook w:val="0420"/></w:tblPr>"""));

        mask.HorizontalBanding.ShouldBeTrue();
        mask.VerticalBanding.ShouldBeFalse();
    }

    /// <summary>A table stating no look at all is not banded either.</summary>
    /// <remarks>
    /// The control for the test above, and the one a naive inversion fails: an absent
    /// <c>w:tblLook</c> asks for no conditional formatting rather than for the default, so
    /// <c>None</c> has to stay all-false even though the "no banding" bits are clear in a zero mask.
    /// </remarks>
    [Fact]
    public void ATableWithNoLookIsNotBanded()
    {
        WordTableLook look = WordTableLook.Read(XElement.Parse($"""<w:tblPr xmlns:w="{W}"/>"""));

        look.HorizontalBanding.ShouldBeFalse();
        look.VerticalBanding.ShouldBeFalse();
        look.ShouldBe(WordTableLook.None);
    }

    /// <summary>The band size comes off the style's <c>w:tblPr</c> chain and defaults to one.</summary>
    /// <remarks>
    /// Every style here states a <c>w:name</c>, and that is not decoration: <c>StyleSheetTable::sprm</c>
    /// appends a nameless style to neither the style table nor its identifier map on an OOXML import,
    /// so a nameless style cannot be referenced at all and this reader reproduces that. A first cut of
    /// this fixture omitted the names and every lookup came back empty.
    /// </remarks>
    [Fact]
    public void TheBandSizesComeFromTheStyle()
    {
        WordStyles styles = new();
        styles.Add(XElement.Parse($"""
            <w:styles xmlns:w="{W}">
              <w:style w:type="table" w:styleId="Base">
                <w:name w:val="Base"/>
                <w:tblPr><w:tblStyleRowBandSize w:val="3"/></w:tblPr>
              </w:style>
              <w:style w:type="table" w:styleId="Child">
                <w:name w:val="Child"/><w:basedOn w:val="Base"/>
                <w:tblPr><w:tblStyleColBandSize w:val="2"/></w:tblPr>
              </w:style>
              <w:style w:type="table" w:styleId="Silent"><w:name w:val="Silent"/></w:style>
            </w:styles>
            """));

        styles.TableStyleBandSizes("Child").ShouldBe((3, 2), "inherited rows, own columns");
        styles.TableStyleBandSizes("Silent").ShouldBe((1, 1), "a style stating neither bands at one");
        styles.TableStyleBandSizes(null).ShouldBe((1, 1));
    }

    /// <summary>
    /// A layer carrying only a <c>w:tcPr</c> is a layer, and used to be discarded whole.
    /// </summary>
    /// <remarks>
    /// The seat of the defect in one assertion: <c>WordStyle</c>'s constructor skipped any
    /// <c>w:tblStylePr</c> with no <c>w:rPr</c> inside it, and every band and column layer of
    /// <c>PlainTable5</c> is exactly that.
    /// </remarks>
    [Fact]
    public void ALayerWithNoRunPropertiesStillContributesItsCellProperties()
    {
        WordStyles styles = new();
        styles.Add(XElement.Parse($"""
            <w:styles xmlns:w="{W}">
              <w:style w:type="table" w:styleId="Shaded">
                <w:name w:val="Shaded"/>
                <w:tblStylePr w:type="band1Horz">
                  <w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="F2F2F2"/></w:tcPr>
                </w:tblStylePr>
              </w:style>
            </w:styles>
            """));

        WordTableStyleConditions band = new(
            new WordTableLook(FirstRow: false, LastRow: false, FirstColumn: false, LastColumn: false,
                              HorizontalBanding: true, VerticalBanding: false),
            IsFirstRow: false, IsLastRow: false, IsFirstColumn: false, IsLastColumn: false,
            RowBand: 0, ColumnBand: null);

        styles.TableStyleCellProperties("Shaded", band).Count.ShouldBe(1);
        styles.TableStyleRunProperties("Shaded", band)
            .ShouldBeEmpty("the layer carries no w:rPr, and that half is unchanged");
    }

    private static Dictionary<string, Colour?> FixtureShading()
    {
        using IDocument document = new WordProcessingReader().Read(
            DocumentSource.FromFile(Corpus.Require("table-style-bands.docx")));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        Dictionary<string, Colour?> found = new(StringComparer.Ordinal);
        foreach (PlacedTableCell cell in pages.Pages[0].Tables.SelectMany(table => table.Cells))
        {
            foreach (PageBlock block in cell.Content?.Blocks ?? [])
            {
                if (block is PageParagraph paragraph && paragraph.Text.Length > 0)
                    found[paragraph.Text] = cell.Cell.Shading;
            }
        }

        found.Count.ShouldBe(18, "six rows of three cells");
        return found;
    }
}
