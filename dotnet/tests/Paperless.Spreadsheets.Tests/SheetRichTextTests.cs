using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What the three readers make of a cell whose text is not all in one format.
/// </summary>
/// <remarks>
/// Needs no LibreOffice and no font: the claim is about what was <em>read</em>, which is where the
/// three formats differ. Where the portions land on a page is
/// <c>SheetRichTextComparisonTests</c>'s business.
/// </remarks>
public sealed class SheetRichTextTests
{
    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void EveryReaderSplitsTheSameCellsIntoTheSamePortions(string name)
    {
        SheetLayout sheet = FirstSheet(name);

        // A1 is "One bold word here": plain, bold, plain. Three portions and not two, because the
        // characters before the first stated run are a portion of their own — BIFF states no run
        // for them at all and the other two do, and normalising here is what hides that.
        IReadOnlyList<SheetTextPortion> a1 = Portions(sheet, 0, 0, "One bold word here");

        a1.Count.ShouldBe(3, $"{name}: A1 portions");
        a1[0].Start.ShouldBe(0, $"{name}: A1 first portion starts at the beginning");
        a1[1].Length.ShouldBe(4, $"{name}: A1 the bold word is four characters");
        a1[1].Format.FontWeight.ShouldBe(700, $"{name}: A1 the middle portion is bold");
        a1[0].Format.FontWeight.ShouldBe(400, $"{name}: A1 the first portion is not");
        a1[2].Format.FontWeight.ShouldBe(400, $"{name}: A1 nor the last");

        // The portions partition the text exactly once, which is what the shared layouter needs
        // of the runs it measures across.
        Covers(a1, "One bold word here".Length).ShouldBeTrue($"{name}: A1 covers its text once");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void APortionCarriesItsOwnSizeAndColour(string name)
    {
        SheetLayout sheet = FirstSheet(name);

        // A2 is "Big RED middle" with a fourteen-point red word in the middle of ten-point black
        // text. The size is what makes the line taller and the colour is what the cell does not
        // state, so a reader that kept only the cell's format loses both.
        IReadOnlyList<SheetTextPortion> a2 = Portions(sheet, 1, 0, "Big RED middle");

        a2.Count.ShouldBe(3, $"{name}: A2 portions");
        a2[1].Format.FontSize.ShouldBe(Length.FromPoints(14), $"{name}: A2 the middle is 14 pt");
        a2[1].Format.Colour.ShouldBe(Colour.FromRgb(0xC00000), $"{name}: A2 the middle is red");
        a2[0].Format.FontSize.ShouldBe(Length.FromPoints(10), $"{name}: A2 the rest is 10 pt");
        a2[0].Format.Colour.ShouldBe(Colour.Black, $"{name}: A2 the rest is black");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void APortionKeepsWhatItDoesNotChange(string name)
    {
        SheetLayout sheet = FirstSheet(name);

        // B2 is a blue cell whose second half is bold, and both halves must stay blue. The three
        // formats reach that answer three different ways: ODF's span states only the weight and
        // inherits the colour, while SpreadsheetML's rPr and BIFF's FONT record both restate the
        // colour because neither inherits anything from the cell.
        IReadOnlyList<SheetTextPortion> b2 =
            Portions(sheet, 1, 1, "Blue cell with bold kept blue");

        b2.Count.ShouldBe(2, $"{name}: B2 portions");
        b2[0].Format.FontWeight.ShouldBe(400, $"{name}: B2 the first half is regular");
        b2[1].Format.FontWeight.ShouldBe(700, $"{name}: B2 the second half is bold");

        foreach (SheetTextPortion portion in b2)
            portion.Format.Colour.ShouldBe(Colour.FromRgb(0x204080), $"{name}: B2 stays blue");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void ACellWhoseFirstWordIsBoldDoesNotStayBold(string name)
    {
        SheetLayout sheet = FirstSheet(name);

        // B4 is "Starts bold" with only the first word bold, and it is the case that separates
        // the three formats' semantics. Saving it, LibreOffice writes the *cell's* own font as the
        // bold one — the cell attribute is taken from the first portion — and then writes the
        // second run stating a name and a size and no weight. Reading that run as a delta over the
        // cell leaves the whole cell bold, which is what its own rendering does not do:
        // Font::Font builds a portion's font from the workbook's default with every flag already
        // used (sc/source/filter/oox/stylesbuffer.cxx:584), so the cell's font never enters it.
        IReadOnlyList<SheetTextPortion> b4 = Portions(sheet, 3, 1, "Starts bold");

        b4.Count.ShouldBe(2, $"{name}: B4 portions");
        b4[0].Format.FontWeight.ShouldBe(700, $"{name}: B4 the first word is bold");
        b4[1].Format.FontWeight.ShouldBe(400, $"{name}: B4 and the rest is not");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void APlainCellHasNoPortionsAtAll(string name)
    {
        SheetLayout sheet = ((SpreadsheetPages)Document(name).Layout()).Sheets[2];

        // A cell of one format is not recorded, so a workbook whose text is all plain costs one
        // static instance rather than a dictionary entry per cell.
        sheet.RichText.At(0, 0, "Logo below").ShouldBeNull($"{name}: a plain cell");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void PortionsAreNotHandedOutForTextTheyDoNotDescribe(string name)
    {
        SheetLayout sheet = FirstSheet(name);

        // The portions index into the string the file stores, and what is drawn is that string run
        // through the cell's number format. Asking with anything else gets nothing rather than a
        // set of offsets into the wrong characters.
        sheet.RichText.At(0, 0, "One bold word here").ShouldNotBeNull($"{name}: the cell's own text");
        sheet.RichText.At(0, 0, "Something else entirely")
             .ShouldBeNull($"{name}: text the portions do not describe");
    }

    private static bool Covers(IReadOnlyList<SheetTextPortion> portions, int length)
    {
        int at = 0;
        foreach (SheetTextPortion portion in portions)
        {
            if (portion.Start != at) return false;
            at = portion.End;
        }
        return at == length;
    }

    private static IReadOnlyList<SheetTextPortion> Portions(
        SheetLayout sheet, int row, int column, string text)
    {
        IReadOnlyList<SheetTextPortion>? portions = sheet.RichText.At(row, column, text);
        portions.ShouldNotBeNull($"row {row + 1}, column {column + 1} has portions");
        return portions;
    }

    private static SheetLayout FirstSheet(string name)
        => ((SpreadsheetPages)Document(name).Layout()).Sheets[0];

    private static IPaginatedDocument Document(string name)
        => (IPaginatedDocument)PaperlessDocument.Open(Corpus.Require(name));
}
