using Paperless.Core.Numbers;
using Shouldly;

namespace Paperless.Core.Tests;

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
    // "0" forces the digit that "#" would drop, so a whole number keeps its leading zero
    // position while its empty decimals go.
    [InlineData("0.##", 1.5, "1.5")]
    [InlineData("0.##", 1.0, "1")]
    [InlineData("0.00", 0.5, "0.50")]
    public void DigitPlaceholdersLayOutTheNumber(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // A half rounds *away from zero*, which is LibreOffice's rtl_math_RoundingMode_Corrected —
    // literally approxFloor(magnitude + 0.5) (sal/rtl/math.cxx:483). .NET's own "F0" is
    // IEEE-correct and rounds a half to even instead, so it turns 4.5 into 4 and 5.5 into 6:
    // right one time in two, and wrong on a whole column of prices shown without decimals.
    [InlineData("0", 4.5, "5")]
    [InlineData("0", 5.5, "6")]
    [InlineData("0", -4.5, "-5")]
    [InlineData("0.0", 0.25, "0.3")]
    [InlineData("0%", 0.635, "64%")]
    [InlineData("0.0%", 0.635, "63.5%")]
    public void AHalfRoundsAwayFromZeroRatherThanToEven(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // Percent multiplies by a hundred and a trailing comma divides by a thousand; both are
    // scaling, not decoration, and a reader that treats them as literals is out by 10^3.
    [InlineData("0.0%", 0.075, "7.5%")]
    [InlineData("0%", 0.25, "25%")]
    [InlineData("#,##0,", 1234567, "1,235")]
    [InlineData("#,##0,,\" M\"", 12345678, "12 M")]
    // The same character, and only its position separates the two meanings: without a
    // grouping comma anywhere in the code, a trailing one still scales.
    [InlineData("#,##0", 1234567, "1,234,567")]
    [InlineData("0,", 1234567, "1235")]
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
    [InlineData("# ?/8", 2.37, "2 3/8")]
    // The pattern gives the denominator's *width*, not its value, so this is a search for the
    // closest fraction below a hundred rather than a digit walk. Measured against
    // LibreOffice's own rendering of sc/qa/unit/data/xls/formats.xls, which shows 25 31/82.
    [InlineData("# ??/??", 25.378, "25 31/82")]
    [InlineData("# ??/??", 0.389, "  7/18")]
    [InlineData("# ?/?", 0.25, " 1/4")]
    [InlineData("# ?/?", 2.5, "2 1/2")]
    public void FractionsApproximateWithinTheDenominatorsWidth(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Fact]
    public void TheNumeratorPadsLeftAndTheDenominatorPadsRight()
    {
        // Not symmetry for its own sake: LibreOffice passes bInsertRightBlank for the
        // denominator alone and calls it "left alignment of denominator"
        // (svl/source/numbers/zformat.cxx, ImpNumberFill). That is what lines a column up on
        // its bars — "1/4 " under "??", never "1/ 4".
        Format("# ??/??", 1.25).ShouldBe("1  1/4 ");
        Format("# ??/??", -1.25).ShouldBe("-1  1/4 ");
    }

    [Fact]
    public void AWholeNumberInAFractionFormatShowsNoFraction()
    {
        // Excel blanks the fraction rather than writing "2 0/1", and the placeholders still
        // reserve their width so the column keeps its shape.
        Format("0 ?/?", 2.0).TrimEnd().ShouldBe("2");
        Format("# ?/?", 3.0).ShouldBe("3    ");
    }

    [Theory]
    // With an explicit negative subformat the sign comes from that subformat's literals, so
    // emitting one as well turns -1,234.50 into --1,234.50.
    [InlineData("#,##0.00;[RED]\\-#,##0.00", -1234.5, "-1,234.50")]
    [InlineData("#,##0.00;(#,##0.00)", -1234.5, "(1,234.50)")]
    [InlineData("0.00", -1234.5, "-1234.50")]
    // Three subformats put zero on its own branch — including when the middle one is empty,
    // which is how a code says "show nothing for a negative".
    [InlineData("0.0;-0.0;\"nil\"", 0, "nil")]
    [InlineData("0.00;;\"zero\"", 0, "zero")]
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
    // A condition beside a colour, which is the shape a real file writes.
    [InlineData("[>=100][RED]0.0;0.00", 150, "150.0")]
    [InlineData("[>=100][RED]0.0;0.00", 5, "5.00")]
    public void ConditionsOverrideThePositionalRule(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // 46233 is 2026-07-30; 0.604166666666667 of a day is 14:30.
    [InlineData("yyyy-mm-dd", 46233, "2026-07-30")]
    [InlineData("M/D/YYYY", 46233, "7/30/2026")]
    [InlineData("dd mmmm yyyy", 46233, "30 July 2026")]
    [InlineData("ddd d mmm yy", 46233, "Thu 30 Jul 26")]
    [InlineData("mmmmm", 46233, "J")]
    // "dddd" is the full weekday name and "DD/MM/YYYY" is what BIFF built-in 14 resolves to,
    // so both are on the path a real workbook takes.
    [InlineData("dddd", 46233, "Thursday")]
    [InlineData("DD/MM/YYYY", 46233, "30/07/2026")]
    [InlineData("mmmm yy", 46233, "July 26")]
    [InlineData("hh:mm", 0.604166666666667, "14:30")]
    [InlineData("h:mm AM/PM", 0.604166666666667, "2:30 PM")]
    [InlineData("h:mm:ss", 0.604166666666667, "14:30:00")]
    public void DateAndTimeFieldsFormatFromTheSerialNumber(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // A clock field truncates rather than rounds, which is LibreOffice's rule and a deliberate
    // one: tools::Time::GetClock says "do not round values (specifically not up), but truncate
    // to the next magnitude, so 23:59:59.99 is still 23:59:59 and not 24:00:00 (or even
    // 00:00:00 which Excel does)". Measured on sc/qa/unit/data/xls/formats.xls, whose Sheet3
    // holds 05:35:31.2 under hh:mm and renders 05:35.
    [InlineData("hh:mm", 0.23300000000, "05:35")]
    [InlineData("hh:mm:ss", 0.23300000000, "05:35:31")]
    [InlineData("hh:mm", 0.99999, "23:59")]
    // The serial is rounded to the millisecond first, so a stored 14:30 — 0.604166666… — does
    // not truncate to 14:29 on the floating-point residue.
    [InlineData("hh:mm", 0.604166666666666, "14:30")]
    public void AClockFieldTruncatesRatherThanRounds(string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // "m" is a minute when it follows an hour or precedes a second, and a month otherwise.
    // The same letter, two meanings, decided entirely by its neighbours.
    [InlineData("m", 46233.5, "7")]
    [InlineData("h:m", 46233.5, "12:0")]
    [InlineData("m:ss", 46233.5, "0:00")]
    [InlineData("mm/dd", 46233.5, "07/30")]
    // The hard case is one code holding both: the first "mm" is a month because a day follows
    // it, the second a minute because an hour precedes it. Getting it backwards turns 30/07
    // into 30/30 in precisely the format people use most.
    [InlineData("dd/mm/yyyy hh:mm", 46233.604166666664, "30/07/2026 14:30")]
    [InlineData("mm:ss", 46233.604166666664, "30:00")]
    public void MonthAndMinuteAreToldApartByTheirNeighbours(
        string code, double value, string expected)
        => Format(code, value).ShouldBe(expected);

    [Theory]
    // An elapsed field keeps the whole days rather than wrapping at midnight, which is the
    // whole point of the bracket.
    [InlineData("[h]:mm", 1.5, "36:00")]
    [InlineData("[h]:mm:ss", 1.5, "36:00:00")]
    [InlineData("[h]:mm:ss", 1.5208333333333, "36:30:00")]
    [InlineData("[mm]:ss", 0.5, "720:00")]
    [InlineData("[mm]:ss", 1.5208333333333, "2190:00")]
    [InlineData("h:mm", 1.5, "12:00")]
    [InlineData("h:mm:ss", 1.5, "12:00:00")]
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
    [InlineData("\"Qty \"0.0\" units\"", 2.5, "Qty 2.5 units")]
    [InlineData("\\$0.00", 4.5, "$4.50")]
    [InlineData("\\R #,##0.00", 4.5, "R 4.50")]
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
    [InlineData(4.5, "4.5")]
    [InlineData(0.000123, "0.000123")]
    [InlineData(1234567.891, "1234567.891")]
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

    // What a code says about its cell's *type* — IsDateTime, IsTimeOnly, HasElapsed — is
    // asserted once, in NumberFormatCodeTests, rather than in both files.

    [Theory]
    // Serial 60 is the 29 February 1900 that never happened — a Lotus 1-2-3 compatibility bug
    // baked into every file ever written — so everything above it is one day further back than
    // naive arithmetic gives.
    [InlineData(1, 1900, 1, 1)]
    [InlineData(59, 1900, 2, 28)]
    // Serial 60 itself is the phantom day. It lands on 1 March, sharing that day with 61,
    // which is what LibreOffice's own correction produces — it adds a day to everything
    // before 1900-03-01 (XclRoot::GetDateTimeFromDouble, sc/source/filter/excel/xlroot.cxx).
    [InlineData(60, 1900, 3, 1)]
    [InlineData(61, 1900, 3, 1)]
    [InlineData(46233, 2026, 7, 30)]
    public void The1900EpochAccountsForThePhantomLeapDay(
        double serial, int year, int month, int day)
        => SpreadsheetDate.FromSerial(serial, SpreadsheetDateSystem.Date1900)
                          .ShouldBe(new DateTime(year, month, day));

    /// <summary>
    /// A <c>*c</c> directive says where a column-filling repeat goes, and states nothing about
    /// how much of it there is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the whole of the accounting formats — built-in ids 5–8 and 41–44, and every
    /// <c>_("$"* #,##0.00_)</c> Excel writes — and dropping it silently is what puts an
    /// accounting cell's currency symbol against its digits instead of against the far edge of
    /// its column. Extraction has no column, so the default is still to drop it; the marker is
    /// for the one caller that has a width.
    /// </para>
    /// <para>
    /// The marker is LibreOffice's own: <c>U+001B</c> then the fill character
    /// (<c>lcl_appendStarFillChar</c>, <c>svl/source/numbers/zformat.cxx:2200</c>).
    /// </para>
    /// </remarks>
    [Theory]
    // The three numeric subformats of the accounting format, one per sign, and the digits each
    // shows — so that a change to the fill cannot quietly change the number beside it.
    [InlineData(1234.5, " $", "1,234.50 ")]
    [InlineData(-1234.5, " $", "(1,234.50)")]
    [InlineData(0.0, " $", "-   ")]
    public void AFillDirectiveMarksWhereItExpands(double value, string before, string after)
    {
        const string Accounting =
            "_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)";

        NumberFormatCode code = NumberFormatCode.Parse(Accounting);
        code.HasFillDirective.ShouldBeTrue();

        // The fill sits between the currency symbol and the digits, and its character — a space
        // here — is what a caller holding a column width repeats at that point.
        NumberFormatter.Format(code, value, keepFillMarkers: true)
                       .ShouldBe(before + NumberFormatter.FillMarker + " " + after);

        // Nothing of it survives into extracted text: neither the marker nor the one copy of the
        // fill character beside it, which would otherwise read as a stray space.
        NumberFormatter.Format(code, value).ShouldBe(before + after);
    }

    /// <summary>A code with no <c>*</c> says so, so the re-render is never paid for.</summary>
    [Theory]
    [InlineData("#,##0.00")]
    [InlineData("General")]
    [InlineData("[$£-809]#,##0.00;[RED]-#,##0.00")]
    public void ACodeWithoutAFillSaysSo(string code)
        => NumberFormatCode.Parse(code).HasFillDirective.ShouldBeFalse();

    [Fact]
    public void The1904EpochIs1462DaysLater()
        => SpreadsheetDate.FromSerial(46233 - SpreadsheetDate.Date1904Offset,
                                      SpreadsheetDateSystem.Date1904)
                          .ShouldBe(new DateTime(2026, 7, 30));
}
