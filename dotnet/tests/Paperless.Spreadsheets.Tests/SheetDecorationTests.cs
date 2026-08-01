using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The rules that decide what a printed sheet draws behind and around its cells.
/// </summary>
/// <remarks>
/// Unit tests for the two pieces that are pure logic and expensive to debug through a rendering:
/// which of two neighbours wins the edge they share, and what a header's <c>&amp;</c>-codes mean.
/// Both are ported rather than invented, so the assertions are what LibreOffice does rather than
/// what seems reasonable.
/// </remarks>
public sealed class SheetDecorationTests
{
    [Fact]
    public void TheHeavierOfTwoNeighbouringBordersWinsTheEdgeTheyShare()
    {
        SheetBorder thick = SheetBorder.Line(Length.FromTwips(50), Colour.FromRgb(0xFF0000));
        SheetBorder hair = SheetBorder.Line(Length.FromTwips(1), Colour.Black);

        // std::max over svx::frame::Style::operator<, which orders by total width first
        // (framelink.cxx:320). Stated either way round, the thick one is drawn — which is the
        // whole of what sheet-decor's rows 2 and 3 exercise.
        SheetCellBorders.Resolve(thick, hair).ShouldBe(thick);
        SheetCellBorders.Resolve(hair, thick).ShouldBe(thick);
    }

    [Fact]
    public void AnEqualEdgeGoesToTheCellBeingAskedAbout()
    {
        SheetBorder red = SheetBorder.Line(Length.FromTwips(15), Colour.FromRgb(0xFF0000));
        SheetBorder blue = SheetBorder.Line(Length.FromTwips(15), Colour.FromRgb(0x0000FF));

        // Colour is not part of the ordering, so the two are equal — and std::max returns its
        // first argument when neither is less. The first argument is always the cell's own edge.
        SheetCellBorders.Resolve(red, blue).ShouldBe(red);
        SheetCellBorders.Resolve(blue, red).ShouldBe(blue);
    }

    [Fact]
    public void ADoubleRuleBeatsASingleOneOfTheSameTotalWidth()
    {
        SheetBorder single = SheetBorder.Line(Length.FromTwips(30), Colour.Black);
        SheetBorder doubled = new(
            Length.FromTwips(10), Length.FromTwips(10), Length.FromTwips(10), Colour.Black);

        doubled.Width.ShouldBe(single.Width);
        SheetCellBorders.Resolve(single, doubled).ShouldBe(doubled);
    }

    [Fact]
    public void TwoHairlinesAreSettledByTheirPattern()
    {
        // The last clause of operator<: at one unit wide, and only there, the pattern decides,
        // and the lower enumerator wins — so a solid hairline beats a dotted one.
        SheetBorder solid = SheetBorder.Line(Length.FromTwips(1), Colour.Black);
        SheetBorder dotted = SheetBorder.Line(
            Length.FromTwips(1), Colour.Black, SheetBorderPattern.Dotted);

        SheetCellBorders.Resolve(dotted, solid).ShouldBe(solid);
        SheetCellBorders.Resolve(solid, dotted).ShouldBe(solid);
    }

    [Fact]
    public void ACellsOwnFormatBeatsItsColumns()
    {
        SheetFormatting formatting = new();
        int blue = formatting.Intern(new SheetCellDecoration(Colour.FromRgb(0x0000FF), default));

        formatting.SetColumns(1, 1, blue);
        formatting.SetCell(6, 1, 0);

        // Zero means "this cell states a style and it paints nothing", which has to override the
        // column. Saving the corpus document as ODS produced exactly this: Calc moved a single
        // blue cell onto its whole column and cancelled it seven rows down with
        // table:style-name="Default" on the one cell that must stay white.
        formatting.At(0, 1).Background.ShouldBe(Colour.FromRgb(0x0000FF));
        formatting.At(6, 1).Background.ShouldBeNull();
    }

    [Theory]
    [InlineData("&LLeft&CMiddle&RRight", "Left", "Middle", "Right")]
    [InlineData("Bare text", "", "Bare text", "")]
    [InlineData("&L&\"Arial,Bold\"&12Sized", "Sized", "", "")]
    [InlineData("&CA && B", "", "A & B", "")]
    [InlineData("&R&KFF0000Red", "", "", "Red")]
    public void TheAmpersandCodeLanguageSplitsIntoThreeParts(
        string source, string left, string centre, string right)
    {
        SheetHeaderFooter band = SheetHeaderFooter.ParseCodes(source);
        SheetHeaderContext context = new();

        band.Left.Resolve(context).ShouldBe(left);
        band.Centre.Resolve(context).ShouldBe(centre);
        band.Right.Resolve(context).ShouldBe(right);
    }

    [Fact]
    public void TheFieldsExpandToWhatThePageStandsFor()
    {
        SheetHeaderFooter band =
            SheetHeaderFooter.ParseCodes("&L&A&CPage &P of &N&R&F");

        SheetHeaderContext context = new()
        {
            PageNumber = 3,
            PageCount = 7,
            SheetName = "Decor",
            FileName = "sheet-decor-ods.ods",
        };

        band.Left.Resolve(context).ShouldBe("Decor");
        band.Centre.Resolve(context).ShouldBe("Page 3 of 7");

        // With its extension, which is measurable rather than assumed: LibreOffice's own
        // rendering of a footer holding &F prints "sheet-decor-ods.ods".
        band.Right.Resolve(context).ShouldBe("sheet-decor-ods.ods");
    }

    [Fact]
    public void ThePathCodeSwallowsTheFileNameThatFollowsIt()
    {
        // "&Z&F" is one field, not two: Excel writes the path and the name separately and Calc
        // drops the second (xihelper.cxx:356-361). Taking both prints the name twice.
        SheetHeaderFooter band = SheetHeaderFooter.ParseCodes("&L&Z&F");
        SheetHeaderContext context = new() { FilePath = "/tmp/book.xlsx", FileName = "book.xlsx" };

        band.Left.Resolve(context).ShouldBe("/tmp/book.xlsx");
    }

    [Theory]
    [InlineData("sheet-decor-ods.ods")]
    [InlineData("sheet-decor-xlsx.xlsx")]
    [InlineData("sheet-decor-xls.xls")]
    public void EveryFormatReadsTheSameFillsAndTheSameSharedEdges(string name)
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(
                DocumentSource.FromFile(Corpus.Require(name)));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];
        SheetFormatting formatting = sheet.Formatting;

        // Row 1 is three fills; the same three colours survive all three formats because they
        // are stated as explicit RGB rather than by palette or theme index.
        formatting.At(0, 0).Background.ShouldBe(Colour.FromRgb(0xFFFF00));
        formatting.At(0, 1).Background.ShouldBe(Colour.FromRgb(0x729FCF));
        formatting.At(0, 2).Background.ShouldBe(Colour.FromRgb(0xCCCCCC));

        // Row 2 states the heavy border on the left cell's right edge and row 3 on the right
        // cell's left, so a rule that only ever consulted one side gets exactly one of the two.
        SheetBorder second = SheetCellBorders.Resolve(
            formatting.At(1, 1).Borders.Right, formatting.At(1, 2).Borders.Left);
        SheetBorder third = SheetCellBorders.Resolve(
            formatting.At(2, 1).Borders.Right, formatting.At(2, 2).Borders.Left);

        second.Colour.ShouldBe(Colour.FromRgb(0xFF0000), $"{name}: row 2's shared edge");
        second.Width.ShouldBe(Length.FromTwips(50), $"{name}: row 2's shared edge width");
        third.Colour.ShouldBe(Colour.FromRgb(0x0000FF), $"{name}: row 3's shared edge");
        third.Width.ShouldBe(Length.FromTwips(50), $"{name}: row 3's shared edge width");

        // The horizontal one, between B5's heavy bottom and B6's hairline top.
        SheetBorder shared = SheetCellBorders.Resolve(
            formatting.At(4, 1).Borders.Bottom, formatting.At(5, 1).Borders.Top);

        shared.Colour.ShouldBe(Colour.FromRgb(0x008000), $"{name}: the shared horizontal edge");

        // And the flags that decide whether any of the furniture prints at all.
        sheet.Setup.PrintsGrid.ShouldBeTrue($"{name}: the grid should print");
        sheet.Setup.PrintsHeadings.ShouldBeTrue($"{name}: the headings should print");
        sheet.Setup.Header.ShouldNotBeNull($"{name}: the header should have been parsed");
        sheet.Setup.Footer.ShouldNotBeNull($"{name}: the footer should have been parsed");
    }
}
