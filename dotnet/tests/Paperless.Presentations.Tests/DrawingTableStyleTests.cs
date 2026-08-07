using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A DrawingML table style — <c>tableStyles.xml</c> — resolved onto a table's cells.
/// </summary>
/// <remarks>
/// <para>
/// A PowerPoint-authored table states <c>&lt;a:tblPr firstRow="1" bandRow="1"&gt;</c> and a style
/// GUID, gives every cell an empty <c>&lt;a:tcPr/&gt;</c>, and puts every fill and every border in
/// the style part. Nothing read it, so such a table drew its text and nothing else — and on a
/// header row whose style makes the text white, that means white text on white paper.
/// </para>
/// <para>
/// Measured on <c>BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx</c>, the largest single
/// image difference in <c>slides/batch-001</c>: its third page's ink imbalance against
/// LibreOffice's own rendering was 12.39% and its second page's 10.55%, and both are now under
/// the image comparison's threshold. The deck matches word for word either way, which is why the
/// word gate never saw it.
/// </para>
/// </remarks>
public class DrawingTableStyleTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private const string Guid = "{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}";

    private static readonly Colour Accent = Colour.FromRgb(0x4F81BD);
    private static readonly Colour Light = Colour.FromRgb(0xFFFFFF);
    private static readonly Colour WholeTable = Colour.FromRgb(0xDCE6F1);
    private static readonly Colour Band1 = Colour.FromRgb(0xB8CCE4);

    private static XElement Theme() => XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}">
           <a:themeElements>
             <a:clrScheme name="t">
               <a:dk1><a:srgbClr val="000000"/></a:dk1>
               <a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
               <a:dk2><a:srgbClr val="1F497D"/></a:dk2>
               <a:lt2><a:srgbClr val="EEECE1"/></a:lt2>
               <a:accent1><a:srgbClr val="4F81BD"/></a:accent1>
             </a:clrScheme>
           </a:themeElements>
         </a:theme>
         """);

    /// <summary>A style shaped like the ones PowerPoint ships: a header, banding, a grid.</summary>
    private static XElement Styles() => XElement.Parse(
        $"""
         <a:tblStyleLst xmlns:a="{A}" def="{Guid}">
           <a:tblStyle styleId="{Guid}" styleName="Medium 2">
             <a:wholeTbl>
               <a:tcStyle>
                 <a:tcBdr>
                   <a:top><a:ln w="12700"><a:solidFill><a:srgbClr val="112233"/></a:solidFill></a:ln></a:top>
                   <a:insideH><a:ln w="6350"><a:solidFill><a:srgbClr val="FFFFFF"/></a:solidFill></a:ln></a:insideH>
                 </a:tcBdr>
                 <a:fill><a:solidFill><a:srgbClr val="DCE6F1"/></a:solidFill></a:fill>
               </a:tcStyle>
             </a:wholeTbl>
             <a:band1H>
               <a:tcStyle><a:tcBdr/>
                 <a:fill><a:solidFill><a:srgbClr val="B8CCE4"/></a:solidFill></a:fill>
               </a:tcStyle>
             </a:band1H>
             <a:band2H><a:tcStyle><a:tcBdr/></a:tcStyle></a:band2H>
             <a:firstRow>
               <a:tcTxStyle b="on"><a:fontRef idx="minor"><a:prstClr val="black"/></a:fontRef>
                 <a:schemeClr val="lt1"/></a:tcTxStyle>
               <a:tcStyle><a:tcBdr/>
                 <a:fill><a:solidFill><a:schemeClr val="accent1"/></a:solidFill></a:fill>
               </a:tcStyle>
             </a:firstRow>
           </a:tblStyle>
         </a:tblStyleLst>
         """);

    private static DrawingTableStyle Style(string? id = Guid)
        => DrawingTableStyle.Read(Styles(), id).ShouldNotBeNull();

    private static DrawingTableCellStyle At(
        int row, int column, DrawingTableStyleOptions options, int lastRow = 3, int lastColumn = 2)
        => Style().Resolve(options, row, lastRow, column, lastColumn, DrawingTheme.Read(Theme()), null);

    private static readonly DrawingTableStyleOptions HeaderAndBands =
        new(FirstRow: true, LastRow: false, FirstColumn: false, LastColumn: false,
            BandRow: true, BandColumn: false);

    [Fact]
    public void AHeaderRowTakesTheFirstRowPartsFillAndItsWhiteBoldText()
    {
        DrawingTableCellStyle header = At(row: 0, column: 1, HeaderAndBands);

        header.Fill.ShouldBe(Accent);
        header.TextColour.ShouldBe(Light);
        header.Bold.ShouldBe(true);
    }

    [Fact]
    public void TheHeaderRowCountsAsABandSoTheBandingStartsOnTheOtherFoot()
    {
        // nBand = nRow; if (firstRow) nBand++ — tablecell.cxx:384-386 — and an odd band takes
        // band2H, which here states no fill and so leaves the whole table's. Off by one inverts
        // the shading of every banded table, which is why the same two rows are asserted with the
        // header flag and without it.
        At(row: 1, column: 1, HeaderAndBands).Fill.ShouldBe(Band1);
        At(row: 2, column: 1, HeaderAndBands).Fill.ShouldBe(WholeTable);

        DrawingTableStyleOptions bandsOnly = HeaderAndBands with { FirstRow = false };
        At(row: 1, column: 1, bandsOnly).Fill.ShouldBe(WholeTable);
        At(row: 2, column: 1, bandsOnly).Fill.ShouldBe(Band1);
    }

    [Fact]
    public void WithoutTheBandFlagEveryBodyRowTakesTheWholeTablesFill()
    {
        DrawingTableStyleOptions headerOnly = HeaderAndBands with { BandRow = false };

        At(row: 1, column: 1, headerOnly).Fill.ShouldBe(WholeTable);
        At(row: 2, column: 1, headerOnly).Fill.ShouldBe(WholeTable);
    }

    [Fact]
    public void WithoutTheHeaderFlagRowZeroIsAnOrdinaryRow()
    {
        // The flags are what decide, not the position: a table that declares no header row has
        // none, however much its style has to say about one.
        DrawingTableCellStyle plain =
            At(row: 0, column: 1, HeaderAndBands with { FirstRow = false });

        plain.Fill.ShouldNotBe(Accent);
        plain.TextColour.ShouldBeNull();
        plain.Bold.ShouldBeNull();
    }

    [Fact]
    public void TheWholeTablesFourSidesAreTheTablesFrameAndNotEveryCells()
    {
        // a:wholeTbl's left/right/top/bottom are the *table's* outer frame — tablecell.cxx:199-215
        // guards each with a grid-position test — with insideH and insideV carrying the interior.
        // Applying them per cell rules every cell on all four sides.
        At(row: 0, column: 1, HeaderAndBands).Top.ShouldNotBeNull();
        At(row: 1, column: 1, HeaderAndBands).Top.ShouldBeNull();

        At(row: 1, column: 1, HeaderAndBands).InsideHorizontal.ShouldNotBeNull();
    }

    [Fact]
    public void AStyleTheDeckDoesNotCarryResolvesToNothingRatherThanToTheDefault()
    {
        // A table naming a style the package has not got has no style, not the wrong one.
        DrawingTableStyle.Read(Styles(), "{00000000-0000-0000-0000-000000000000}").ShouldBeNull();
        DrawingTableStyle.Read(tableStyles: null, Guid).ShouldBeNull();
    }

    [Fact]
    public void ATableNamingNoStyleGetsNoStyle()
    {
        // Not the list's own `def`, which reads exactly like the deck's default table look and is
        // what PowerPoint applies. LibreOffice searches the list only when the id is non-empty and
        // otherwise uses a static, empty TableStyle
        // (oox/source/drawingml/table/tableproperties.cxx:89-124).
        //
        // Measured, because the source alone does not say which renderer to follow. Page 8 of
        // slides/batch-011/pptx/section_1_our_rights_presentation.pptx is a three-column table
        // with firstRow, firstCol and bandRow set and no a:tableStyleId: the reference leaves its
        // first column white and we filled it accent1 with white text on it. Putting the id of the
        // style the package declares as `def` into that a:tblPr makes the reference draw exactly
        // what we drew, so the fallback was the whole of the difference. Removing it took the
        // document from 123.54 unaccounted ink over 19 major pages to 8.34 over 2.
        Styles().Attribute("def").ShouldNotBeNull().Value.ShouldBe(Guid);

        DrawingTableStyle.Read(Styles(), styleId: null).ShouldBeNull();
        DrawingTableStyle.Read(Styles(), styleId: "").ShouldBeNull();
    }

    [Fact]
    public void ACellsOwnPropertiesBeatTheStyle()
    {
        XElement table = XElement.Parse(
            $"""
             <a:tbl xmlns:a="{A}">
               <a:tblPr firstRow="1" bandRow="1"><a:tableStyleId>{Guid}</a:tableStyleId></a:tblPr>
               <a:tblGrid><a:gridCol w="914400"/><a:gridCol w="914400"/></a:tblGrid>
               <a:tr h="360000">
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody>
                   <a:tcPr><a:solidFill><a:srgbClr val="FF0000"/></a:solidFill></a:tcPr></a:tc>
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody><a:tcPr/></a:tc>
               </a:tr>
               <a:tr h="360000">
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody>
                   <a:tcPr><a:lnT><a:noFill/></a:lnT></a:tcPr></a:tc>
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody><a:tcPr/></a:tc>
               </a:tr>
             </a:tbl>
             """);

        DrawingTableBox box = DrawingTableGeometry.Read(
            table, DrawingTheme.Read(Theme()), Style(), matrix: null);

        // Stated fill wins over the header part's; the cell beside it still takes the style's.
        box.Cells[0].Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0xFF0000)));
        box.Cells[1].Fill.ShouldBe(Paint.Solid(Accent));
        box.Cells[1].TextColour.ShouldBe(Light);

        // An explicit a:noFill on a border is a decision and removes the style's interior rule;
        // its neighbour, which states nothing, keeps it.
        box.Cells[2].Top.ShouldBeNull();
        box.Cells[3].Top.ShouldNotBeNull();
    }

    /// <summary>
    /// A cell's own <c>a:noFill</c> empties it, where an absent element takes the style's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two had been read as the same thing, and the difference is the largest single figure
    /// on the slides track. <c>tablecell.cxx:550</c> builds the style part's fill and then lays
    /// the cell's own over it with <c>assignUsed</c>, which copies <c>moFillType</c> whenever the
    /// cell stated one — so <c>a:noFill</c> is a decision on a cell exactly as it is on a shape.
    /// </para>
    /// <para>
    /// Measured on <c>slides/batch-012/pptx/NAS-Infrastructure-Roadmaps-v16.0.pptx</c>, whose
    /// layout carries a seventeen-column year ruler under <c>Medium Style 2 - Accent 1</c> with
    /// <c>a:noFill</c> on every cell: 368.41 unaccounted ink over 77 major pages to 225.33 over
    /// 66, against the same reference PDF. Eight corpus decks state it, 215 cells between them.
    /// </para>
    /// </remarks>
    [Fact]
    public void ACellsOwnNoFillEmptiesItWhereAnAbsentElementTakesTheStyles()
    {
        XElement table = XElement.Parse(
            $"""
             <a:tbl xmlns:a="{A}">
               <a:tblPr firstRow="1" bandRow="1"><a:tableStyleId>{Guid}</a:tableStyleId></a:tblPr>
               <a:tblGrid><a:gridCol w="914400"/><a:gridCol w="914400"/></a:tblGrid>
               <a:tr h="360000">
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody>
                   <a:tcPr><a:lnL><a:solidFill><a:srgbClr val="112233"/></a:solidFill></a:lnL>
                     <a:noFill/></a:tcPr></a:tc>
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody><a:tcPr/></a:tc>
               </a:tr>
               <a:tr h="360000">
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody>
                   <a:tcPr><a:noFill/></a:tcPr></a:tc>
                 <a:tc><a:txBody><a:bodyPr/><a:p/></a:txBody><a:tcPr/></a:tc>
               </a:tr>
             </a:tbl>
             """);

        DrawingTableBox box = DrawingTableGeometry.Read(
            table, DrawingTheme.Read(Theme()), Style(), matrix: null);

        // The header row: stated nothing gets the style's accent, stated a:noFill gets nothing —
        // and the noFill cell keeps the border it also states, so this is not "the tcPr was
        // ignored wholesale".
        box.Cells[0].Fill.ShouldBeNull();
        box.Cells[0].Left.ShouldNotBeNull();
        box.Cells[1].Fill.ShouldBe(Paint.Solid(Accent));

        // And in the body, where the style's fill is the banding rather than the header.
        box.Cells[2].Fill.ShouldBeNull();
        box.Cells[3].Fill.ShouldBe(Paint.Solid(Band1));
    }
}
