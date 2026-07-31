using Paperless.Spreadsheets.Numbers;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the number-format code interpreter, which is what turns a stored double into the
/// text a spreadsheet shows.
/// </summary>
public class NumberFormatCodeTests
{
    private static readonly DateTime Epoch = new(1899, 12, 30);

    private static string Format(string code, double value)
        => NumberFormatCode.Parse(code).Format(value, Epoch);

    [Theory]
    [InlineData("General", 12, "12")]
    [InlineData("General", 4.5, "4.5")]
    [InlineData("General", 0.000123, "0.000123")]
    [InlineData("General", 1234567.891, "1234567.891")]
    [InlineData("General", 0, "0")]
    [InlineData("General", -3.25, "-3.25")]
    public void GeneralShowsAsManyDigitsAsTheValueNeeds(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("0", 4.5, "5")]
    [InlineData("0.00", 4.5, "4.50")]
    [InlineData("0.##", 1.5, "1.5")]
    [InlineData("0.##", 1.0, "1")]
    [InlineData("#,##0.00", 1234567.891, "1,234,567.89")]
    [InlineData("#,##0.00", 0.5, "0.50")]
    [InlineData("#,##0", 999, "999")]
    [InlineData("0%", 0.635, "64%")]
    [InlineData("0.0%", 0.635, "63.5%")]
    [InlineData("0.00E+00", 0.000123, "1.23E-04")]
    [InlineData("0.00E+00", 12345, "1.23E+04")]
    [InlineData("##0.0E+0", 12345, "12.3E+3")]
    public void DigitPlaceholdersLayTheDigitsOutAsThePatternAsks(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Fact]
    public void ATrailingCommaDividesByAThousandWhereAnInnerOneGroups()
    {
        // The same character, and only its position separates the two meanings.
        Format("#,##0", 1234567).ShouldBe("1,234,567");
        Format("0,", 1234567).ShouldBe("1235");
    }

    [Theory]
    [InlineData("\"Qty \"0.0\" units\"", 2.5, "Qty 2.5 units")]
    [InlineData("[$£-809]#,##0.00", 4.5, "£4.50")]
    [InlineData("\\R #,##0.00", 4.5, "R 4.50")]
    [InlineData("[RED]#,##0.00", 4.5, "4.50")]
    public void LiteralsAndBracketedDirectivesComeThroughAsText(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("#,##0.00;-#,##0.00", -1234.5, "-1,234.50")]
    [InlineData("#,##0.00;(#,##0.00)", -1234.5, "(1,234.50)")]
    [InlineData("0.00", -4.5, "-4.50")]
    [InlineData("0.00;;\"zero\"", 0, "zero")]
    public void SectionsSelectOnTheSignAndOnZero(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("yyyy\\-mm\\-dd", "2026-07-30")]
    [InlineData("DD/MM/YYYY", "30/07/2026")]
    [InlineData("d mmm yyyy", "30 Jul 2026")]
    [InlineData("dddd", "Thursday")]
    [InlineData("mmmm yy", "July 26")]
    public void DateFormatsRenderTheSerialAsACalendarDate(string code, string expected)
    {
        // 46233 is 2026-07-30 in the 1900 system.
        Format(code, 46233).ShouldBe(expected);
    }

    [Theory]
    [InlineData("hh:mm", "14:30")]
    [InlineData("h:mm AM/PM", "2:30 PM")]
    [InlineData("h:mm:ss", "14:30:00")]
    public void TimeFormatsRenderTheFractionOfADay(string code, string expected)
        => Format(code, 46233.604166666664).ShouldBe(expected);

    [Fact]
    public void TheSameLetterMeansMonthOrMinuteDependingOnItsNeighbours()
    {
        // The trap in the whole format language: an m after an hour or before seconds is
        // minutes, and everything else is a month. Getting it backwards turns 30/07 into
        // 30/30 in precisely the formats people use most.
        Format("dd/mm/yyyy hh:mm", 46233.604166666664).ShouldBe("30/07/2026 14:30");
        Format("mm:ss", 46233.604166666664).ShouldBe("30:00");
    }

    [Fact]
    public void BracketedHoursCountElapsedTimeRatherThanTellingTheClock()
    {
        NumberFormatCode elapsed = NumberFormatCode.Parse("[h]:mm:ss");

        elapsed.HasElapsedTime.ShouldBeTrue();
        elapsed.Format(1.5, Epoch).ShouldBe("36:00:00");
        NumberFormatCode.Parse("h:mm:ss").Format(1.5, Epoch).ShouldBe("12:00:00");
    }

    [Fact]
    public void AFormatSaysWhetherItsCellHoldsADateADurationOrANumber()
    {
        NumberFormatCode.Parse("yyyy-mm-dd").ValueKind
            .ShouldBe(NumberFormatCode.CellValueKind.DateTime);
        NumberFormatCode.Parse("hh:mm").ValueKind
            .ShouldBe(NumberFormatCode.CellValueKind.Duration);
        NumberFormatCode.Parse("[h]:mm").ValueKind
            .ShouldBe(NumberFormatCode.CellValueKind.Duration);
        NumberFormatCode.Parse("#,##0.00").ValueKind
            .ShouldBe(NumberFormatCode.CellValueKind.Number);

        // A format showing both is a date: it names a point in time, not a length of one.
        NumberFormatCode.Parse("dd/mm/yyyy hh:mm").ValueKind
            .ShouldBe(NumberFormatCode.CellValueKind.DateTime);
    }

    [Theory]
    [InlineData("# ??/??", 25.378, "25 31/82")]
    [InlineData("# ??/??", 0.389, "  7/18")]
    [InlineData("# ?/?", 2.5, "2 1/2")]
    [InlineData("# ?/?", 3.0, "3    ")]
    [InlineData("# ??/??", -1.25, "-1  1/4 ")]
    public void AFractionFormatApproximatesRatherThanLaysOutDigits(
        string code, double value, string expected)
    {
        // The pattern gives the denominator's width, not its value, so this is a search for
        // the closest fraction below a hundred rather than a digit walk. Measured against
        // LibreOffice's own rendering of sc/qa/unit/data/xls/formats.xls, which shows
        // 25 31/82 and 7/18 for the first two.
        //
        // The spaces are the pattern's: a placeholder with no digit to show pads instead, so
        // that a column of fractions lines up on its bars, and a value with nothing left over
        // shows a blank fraction rather than 0/1.
        Format(code, value).ShouldBe(expected);
    }

    [Fact]
    public void AConditionalSectionIsReportedRatherThanGuessedAt()
    {
        // A condition selects between sections on the value, which this does not reproduce.
        // Saying so lets the reader record a diagnostic instead of presenting a wrong answer
        // as the truth.
        NumberFormatCode.Parse("[>100]0.0;0.00").IsUnderstood.ShouldBeFalse();
        NumberFormatCode.Parse("#,##0.00").IsUnderstood.ShouldBeTrue();
        NumberFormatCode.Parse("# ??/??").IsUnderstood.ShouldBeTrue();
    }

    [Fact]
    public void ATextSectionAppliesToStringsAndOnlyToStrings()
    {
        NumberFormatCode code = NumberFormatCode.Parse("0.00;-0.00;0;\"[\"@\"]\"");

        code.FormatText("note").ShouldBe("[note]");
        code.Format(4.5, Epoch).ShouldBe("4.50");

        // With fewer than four sections a string is shown as it stands.
        NumberFormatCode.Parse("0.00").FormatText("note").ShouldBe("note");
    }

    [Fact]
    public void BuiltInIndicesResolveWithoutTheFileStatingThem()
    {
        // Most workbooks say "format 14" and expect the reader to know. Without the table a
        // majority of dates in a majority of files extract as five-digit numbers.
        BuiltInNumberFormats.Code(0).ShouldBe("General");
        BuiltInNumberFormats.Code(14).ShouldBe("DD/MM/YYYY");
        BuiltInNumberFormats.Code(9).ShouldBe("0%");
        BuiltInNumberFormats.Code(49).ShouldBe("@");

        // 27 is one of the international spellings, which resolve to the base format.
        BuiltInNumberFormats.Code(27).ShouldBe("DD/MM/YYYY");

        // Everything from 164 up is the file's own.
        BuiltInNumberFormats.Code(BuiltInNumberFormats.FirstUserIndex).ShouldBeNull();
    }
}
