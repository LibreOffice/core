using Paperless.Spreadsheets.Numbers;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the Excel number-format code engine directly.
/// </summary>
/// <remarks>
/// Through the engine rather than through a workbook, because a format code has far more
/// behaviour than any one corpus file can reach and each case here is a rule of the format
/// language rather than a property of a document.
/// </remarks>
public class NumberFormatterTests
{
    private static string Format(string code, double value)
        => NumberFormatter.Format(NumberFormatCode.Parse(code), value);

    [Theory]
    // Digit placeholders differ only in what they do when there is no digit: 0 writes a zero,
    // ? a space so columns line up, # nothing at all.
    [InlineData("0", 5, "5")]
    [InlineData("0000", 5, "0005")]
    [InlineData("#", 0, "")]
    [InlineData("0", 0, "0")]
    [InlineData("??0", 5, "  5")]
    [InlineData("0.00", 1234.5, "1234.50")]
    [InlineData("#,##0.00", 1234567.891, "1,234,567.89")]
    [InlineData("#,##0", 999, "999")]
    [InlineData("#.##", 5, "5")]
    [InlineData("#.##", 5.5, "5.5")]
    [InlineData("0.0", 0.05, "0.1")]
    public void DigitPlaceholdersLayOutTheNumber(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // Percent multiplies by a hundred and a trailing comma divides by a thousand; both are
    // scaling, not decoration, and a reader that treats them as literals is out by 10^3.
    [InlineData("0.0%", 0.075, "7.5%")]
    [InlineData("0%", 0.25, "25%")]
    [InlineData("#,##0,", 1234567, "1,235")]
    [InlineData("#,##0,,\" M\"", 12345678, "12 M")]
    public void PercentAndTrailingCommasScaleTheValue(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("0.00E+00", 12345.6789, "1.23E+04")]
    [InlineData("0.00E+00", 0.000123, "1.23E-04")]
    [InlineData("0.0E-0", 12345.0, "1.2E4")]
    // Engineering notation: three integer placeholders keep the exponent a multiple of three.
    [InlineData("##0.0E+0", 12345.0, "12.3E+3")]
    public void ScientificNotationHonoursTheMantissaWidth(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("0 ?/?", 2.25, "2 1/4")]
    // "?" pads with a space so a column of fractions lines up on its bar, so a one-digit
    // numerator under "??" is preceded by one.
    [InlineData("# ??/??", 2.7, "2  7/10")]
    // A fixed denominator is written as literal digits rather than placeholders.
    [InlineData("# ?/8", 2.25, "2 2/8")]
    public void FractionsApproximateWithinTheDenominatorsWidth(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Fact]
    public void AWholeNumberInAFractionFormatShowsNoFraction()
    {
        // Excel blanks the fraction rather than writing "2 0/1".
        Format("0 ?/?", 2.0).TrimEnd().ShouldBe("2");
    }

    [Theory]
    // With an explicit negative subformat the sign comes from that subformat's literals, so
    // emitting one as well turns -1,234.50 into --1,234.50.
    [InlineData("#,##0.00;[RED]\\-#,##0.00", -1234.5, "-1,234.50")]
    [InlineData("#,##0.00;(#,##0.00)", -1234.5, "(1,234.50)")]
    [InlineData("0.00", -1234.5, "-1234.50")]
    // Three subformats put zero on its own branch.
    [InlineData("0.0;-0.0;\"nil\"", 0, "nil")]
    [InlineData("\"TRUE\";\"TRUE\";\"FALSE\"", 1, "TRUE")]
    [InlineData("\"TRUE\";\"TRUE\";\"FALSE\"", 0, "FALSE")]
    public void SubformatsSelectByTheValuesSign(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // A condition overrides the positional rule entirely: the branches are tested in order and
    // the first unconditional one is the else.
    [InlineData("[>100]\"big\";\"small\"", 500, "big")]
    [InlineData("[>100]\"big\";\"small\"", 5, "small")]
    [InlineData("[<=0]\"none\";0.0", 0, "none")]
    public void ConditionsOverrideThePositionalRule(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // 46233 is 2026-07-30; 0.604166666666667 of a day is 14:30.
    [InlineData("yyyy-mm-dd", 46233, "2026-07-30")]
    [InlineData("M/D/YYYY", 46233, "7/30/2026")]
    [InlineData("dd mmmm yyyy", 46233, "30 July 2026")]
    [InlineData("ddd d mmm yy", 46233, "Thu 30 Jul 26")]
    [InlineData("mmmmm", 46233, "J")]
    [InlineData("hh:mm", 0.604166666666667, "14:30")]
    [InlineData("h:mm AM/PM", 0.604166666666667, "2:30 PM")]
    [InlineData("h:mm:ss", 0.604166666666667, "14:30:00")]
    public void DateAndTimeFieldsFormatFromTheSerialNumber(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // "m" is a minute when it follows an hour or precedes a second, and a month otherwise.
    // The same letter, two meanings, decided entirely by its neighbours.
    [InlineData("m", 46233.5, "7")]
    [InlineData("h:m", 46233.5, "12:0")]
    [InlineData("m:ss", 46233.5, "0:00")]
    [InlineData("mm/dd", 46233.5, "07/30")]
    public void MonthAndMinuteAreToldApartByTheirNeighbours(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // An elapsed field keeps the whole days rather than wrapping at midnight, which is the
    // whole point of the bracket.
    [InlineData("[h]:mm", 1.5, "36:00")]
    [InlineData("[mm]:ss", 0.5, "720:00")]
    [InlineData("h:mm", 1.5, "12:00")]
    public void BracketedUnitsMeasureElapsedTime(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("[$£-809]#,##0.00", 4.5, "£4.50")]
    [InlineData("[$€-407]#,##0.00", 4.5, "€4.50")]
    // A colour name selects a colour, and extraction produces text.
    [InlineData("[BLUE]0.00", 4.5, "4.50")]
    public void BracketsThatAreNotConditionsAreCurrencyOrColour(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData("0.0\" kg\"", 72.5, "72.5 kg")]
    [InlineData("\\$0.00", 4.5, "$4.50")]
    // "_x" reserves the width of x and "*x" repeats x to fill the column; neither has a width
    // to work with during extraction, so one becomes a space and the other nothing.
    [InlineData("_(0.00_)", 4.5, " 4.50 ")]
    [InlineData("*-0.00", 4.5, "4.50")]
    public void LiteralsAndSpacingTokens(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    [InlineData(1234.5, "1234.5")]
    [InlineData(12.0, "12")]
    [InlineData(0.0, "0")]
    [InlineData(-31.5, "-31.5")]
    // Fifteen significant digits, where a double stops being exactly decimal. Round-tripping
    // at seventeen would surface floating-point noise no spreadsheet displays.
    [InlineData(0.1 + 0.2, "0.3")]
    public void GeneralIsTheValueAsShortAsItRoundTrips(double value, string expected)
        => Format("General", value).ShouldBe(expected);

    [Fact]
    public void AStringPassesThroughUnlessTheCodeHasATextSubformat()
    {
        NumberFormatter.Format(NumberFormatCode.Parse("0.00"), "hello").ShouldBe("hello");
        NumberFormatter.Format(NumberFormatCode.Parse("@"), "hello").ShouldBe("hello");
        NumberFormatter.Format(NumberFormatCode.Parse("0;0;0;\"[\"@\"]\""), "hello")
                       .ShouldBe("[hello]");
    }

    [Fact]
    public void AnUnparseableCodeDegradesToGeneralRatherThanThrowing()
    {
        // A cell with a strange format is still a cell with a value.
        NumberFormatCode.Parse(null).IsGeneral.ShouldBeTrue();
        NumberFormatCode.Parse("").IsGeneral.ShouldBeTrue();
        Format("\"unterminated", 5).ShouldBe("unterminated");
    }

    [Theory]
    // A date-carrying format is what makes a stored number a date at all.
    [InlineData("yyyy-mm-dd", true, false)]
    [InlineData("hh:mm", true, true)]
    [InlineData("[h]:mm", true, true)]
    [InlineData("yyyy-mm-dd hh:mm", true, false)]
    [InlineData("#,##0.00", false, false)]
    public void ACodeReportsWhetherItShowsADateOrOnlyATime(
        string code, bool isDateTime, bool isTimeOnly)
    {
        NumberFormatCode parsed = NumberFormatCode.Parse(code);
        parsed.IsDateTime.ShouldBe(isDateTime);
        parsed.IsTimeOnly.ShouldBe(isTimeOnly);
    }

    [Theory]
    // Serial 60 is the 29 February 1900 that never happened — a Lotus 1-2-3 compatibility bug
    // baked into every file ever written — so everything above it is one day further back than
    // naive arithmetic gives.
    [InlineData(1, 1900, 1, 1)]
    [InlineData(59, 1900, 2, 28)]
    [InlineData(61, 1900, 3, 1)]
    [InlineData(46233, 2026, 7, 30)]
    public void The1900EpochAccountsForThePhantomLeapDay(
        double serial, int year, int month, int day)
        => SpreadsheetDate.FromSerial(serial, SpreadsheetDateSystem.Date1900)
                          .ShouldBe(new DateTime(year, month, day));

    [Fact]
    public void The1904EpochIs1462DaysLater()
        => SpreadsheetDate.FromSerial(46233 - SpreadsheetDate.Date1904Offset,
                                      SpreadsheetDateSystem.Date1904)
                          .ShouldBe(new DateTime(2026, 7, 30));
}
