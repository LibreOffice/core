using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.Core.Numbers;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the cell-text rules that a rendering comparison cannot isolate.
/// </summary>
/// <remarks>
/// The fidelity harness proves the whole path agrees with LibreOffice; these prove the individual
/// decisions inside it, because a rendered cell shows one answer and several rules had to agree to
/// produce it. The <c>General</c>-at-a-width rendering is the clearest case: a column either fits
/// a number or does not, so a document can demonstrate one budget per cell, and the rule has a
/// different branch for every magnitude.
/// </remarks>
public class SheetCellTextTests
{
    [Theory]
    // Fits outright: nothing is dropped and no notation is introduced.
    [InlineData(1234.5, 10, "1234.5")]
    // The budget takes the decimals first, integer digits being unshortenable.
    [InlineData(1234.5678, 7, "1234.57")]
    // Twelve integer digits will not fit seven characters at all, so it becomes scientific with
    // one decimal — six of the seven characters being "1", ".", "E", "+" and two exponent digits.
    [InlineData(123456789012.0, 7, "1.2E+11")]
    // A wider column buys mantissa digits rather than dropping the notation.
    [InlineData(123456789012.0, 10, "1.2346E+11")]
    // Below the exponential lower bound, scientific is chosen outright rather than as a fallback.
    [InlineData(0.000012345, 9, "1.235E-05")]
    // The sign costs a character of precision, which is why this keeps one decimal fewer.
    [InlineData(-1234.5678, 7, "-1234.6")]
    public void GeneralIsRenderedToFitTheColumn(double value, int characters, string expected)
        => SheetGeneralWidth.Render(value, characters).ShouldBe(expected);

    [Fact]
    public void ADeviceLengthGoesThroughWholeTwipsAndThenTruncates()
    {
        // 0.178 inch is 452.1 hundredths of a millimetre directly and 451 through Calc's own
        // twips, which is the difference that makes an eighty-row page drift 1.5 pt.
        SheetDeviceUnits.Snap(Length.FromInches(0.178)).Points.ShouldBe(12.7843, 0.0005);

        // A margin of twenty twips is 0.9921 pt, not one point.
        SheetDeviceUnits.Snap(Length.FromTwips(20)).Points.ShouldBe(0.9921, 0.0005);

        // A font height rounds instead, which is why ten-point text is emitted at 10.0063 pt.
        SheetDeviceUnits.SnapFontSize(Length.FromPoints(10)).Points.ShouldBe(10.0063, 0.0005);
        SheetDeviceUnits.SnapFontSize(Length.FromPoints(14)).Points.ShouldBe(14.0031, 0.0005);
    }

    [Fact]
    public void AFormatIsLookedUpFromTheCellThenTheRowThenTheColumn()
    {
        SheetCellFormats.Builder builder = new();
        int cell = builder.Intern(new SheetCellFormat { FontWeight = 700 });
        int row = builder.Intern(new SheetCellFormat { IsItalic = true });
        int column = builder.Intern(new SheetCellFormat { FontSize = Length.FromPoints(20) });

        builder.SetCell(1, 1, cell);
        builder.SetRow(1, row);
        builder.SetColumn(1, column);

        SheetCellFormats formats = builder.Build();

        formats.At(1, 1).FontWeight.ShouldBe(700);

        // The row beats the column, which is only visible where the two cross — and is what a
        // sheet that formats both a row and a column depends on.
        formats.At(1, 5).IsItalic.ShouldBeTrue();
        formats.At(5, 1).FontSize.ShouldBe(Length.FromPoints(20));
        formats.At(5, 5).ShouldBe(SheetCellFormat.Default);
    }

    [Fact]
    public void APooledFormatIsSharedBetweenIdenticalCells()
    {
        SheetCellFormats.Builder builder = new();

        // Two different file-level indices resolving to the same text format share one entry,
        // which is what keeps the pool the size of the sheet's distinct formats rather than of
        // its style table.
        int first = builder.Intern(new SheetCellFormat { FontWeight = 700 });
        int second = builder.Intern(new SheetCellFormat { FontWeight = 700 });

        second.ShouldBe(first);
        builder.Intern(SheetCellFormat.Default).ShouldBe(0);
    }

    [Fact]
    public void TheDefaultFormatIsTenPointAndGeneralInEveryDirection()
    {
        SheetCellFormat format = SheetCellFormat.Default;

        format.FontSize.ShouldBe(Length.FromPoints(10));
        format.Horizontal.ShouldBe(SheetHorizontalAlignment.General);
        format.Vertical.ShouldBe(SheetVerticalAlignment.Standard);
        format.Colour.ShouldBe(Colour.Black);
        format.HasGeneralFormat.ShouldBeTrue();

        // General counts as a plain number format, which is what stops a wide number wrapping.
        format.HasPlainNumberFormat.ShouldBeTrue();
        format.IsRotated.ShouldBeFalse();
    }

    [Fact]
    public void ADateFormatIsNotAPlainNumberFormat()
    {
        // The distinction decides whether a wrapping cell actually wraps: Calc disables automatic
        // line breaks for number formats and leaves them on for dates (output2.cxx:1834).
        new SheetCellFormat { NumberFormatKind = NumberFormatKind.DateTime }
            .HasPlainNumberFormat.ShouldBeFalse();

        new SheetCellFormat { NumberFormatKind = NumberFormatKind.Number }
            .HasPlainNumberFormat.ShouldBeTrue();
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    [InlineData("sheet-cell-text.xls")]
    public void EveryReaderResolvesTheSameCellFormats(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];
        SheetCellFormats formats = sheet.Formats;

        // Row 1 is the alignment row: stated left, centre and right, then two cells that state
        // nothing and are resolved from their value's type when they are drawn.
        formats.At(0, 0).Horizontal.ShouldBe(SheetHorizontalAlignment.Left, $"{name}: A1");
        formats.At(0, 1).Horizontal.ShouldBe(SheetHorizontalAlignment.Centre, $"{name}: B1");
        formats.At(0, 2).Horizontal.ShouldBe(SheetHorizontalAlignment.Right, $"{name}: C1");
        formats.At(0, 3).Horizontal.ShouldBe(SheetHorizontalAlignment.General, $"{name}: D1");
        formats.At(0, 4).Horizontal.ShouldBe(SheetHorizontalAlignment.General, $"{name}: E1");

        // Row 2: vertical alignment, an indent, and the wrap flag.
        formats.At(1, 0).Vertical.ShouldBe(SheetVerticalAlignment.Top, $"{name}: A2");
        formats.At(1, 1).Vertical.ShouldBe(SheetVerticalAlignment.Centre, $"{name}: B2");
        formats.At(1, 2).Vertical.ShouldBe(SheetVerticalAlignment.Bottom, $"{name}: C2");
        formats.At(1, 3).Indent.ShouldBeGreaterThan(Length.Zero, $"{name}: D2 indent");
        formats.At(1, 4).Wraps.ShouldBeTrue($"{name}: E2 wrap");

        // Row 3: the font attributes, and shrink-to-fit.
        formats.At(2, 0).FontWeight.ShouldBe(700, $"{name}: A3 weight");
        formats.At(2, 1).IsItalic.ShouldBeTrue($"{name}: B3 posture");
        formats.At(2, 1).FontSize.ShouldBe(Length.FromPoints(14), $"{name}: B3 size");
        formats.At(2, 2).Colour.ShouldBe(Colour.FromRgb(0xC00000), $"{name}: C3 colour");
        // The resolved face rather than the requested family, because the two Excel exports
        // rename it: LibreOffice writes Liberation Serif out as Times New Roman, which the font
        // resolver substitutes back metric-compatibly. Asserting the requested name would make
        // this a test of what the converter wrote.
        SheetFonts.For(formats.At(2, 3))?.Reference.FamilyName
                  .ShouldContain("Serif", Case.Insensitive, $"{name}: D3 face");
        formats.At(2, 4).ShrinksToFit.ShouldBeTrue($"{name}: E3 shrink");

        // Row 4: the two number-format kinds the ### rule turns on.
        formats.At(3, 2).HasGeneralFormat.ShouldBeTrue($"{name}: C4 General");
        formats.At(3, 3).HasGeneralFormat.ShouldBeFalse($"{name}: D4 fixed");
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    [InlineData("sheet-cell-text.xls")]
    public void RotationSurvivesEveryFormat(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SheetLayout turned = ((SpreadsheetPages)document.Layout()).Sheets[1];

        turned.Formats.At(0, 0).RotationDegrees.ShouldBe(45, $"{name}: A1 rotation");
        turned.Formats.At(0, 1).RotationDegrees.ShouldBe(90, $"{name}: B1 rotation");
        turned.Formats.At(0, 2).IsRotated.ShouldBeFalse($"{name}: C1 is upright");
    }
}
