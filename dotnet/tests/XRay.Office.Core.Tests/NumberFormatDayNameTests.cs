using XRay.Office.Core.Numbers;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The day-name keys — Excel's <c>AAA</c>/<c>AAAA</c> and LibreOffice's <c>NN</c>/<c>NNN</c>/
/// <c>NNNN</c> — and the calendar switch they drag in with them.
/// </summary>
/// <remarks>
/// <para>
/// They share one case in <c>svl/source/numbers/zformat.cxx</c>:3983-4008: <c>NN</c> and
/// <c>AAA</c> both write <c>SHORT_DAY_NAME</c>, <c>NNN</c> and <c>AAAA</c> both write
/// <c>LONG_DAY_NAME</c>, and <c>NNNN</c> alone appends the locale's day-of-week separator (:4004).
/// The keyword table (<c>zforscan.cxx</c>:60-77) holds exactly those five spellings, so a run of
/// one or two <c>a</c>, or a lone <c>n</c>, is not a key and stays a literal.
/// </para>
/// <para>
/// Every expectation here was read off a rendered page rather than off that table:
/// <c>dotnet/probes/numfmt-r68/make-codes.py</c> builds one cell per code and renders it through
/// both installed binaries. <strong>24.2.7.2 and 26.2.4.2 agree on all of it</strong>, so a
/// disagreement here is ours and not the version gap — which is what
/// <c>065_Weight_loss_tracker_ff1c89af.xlsx</c> showed, printing its <c>mm/dd/yy\ aaaa</c> cells
/// as <c>08/21/22 aaaa</c> where both binaries print a day name.
/// </para>
/// </remarks>
public class NumberFormatDayNameTests
{
    /// <summary>21 August 2022, a Sunday, at 02:20 — one serial answers every case below.</summary>
    private const double SundaySerial = 44794.09722222222;

    private static string Format(string code, double value)
        => NumberFormatter.Format(NumberFormatCode.Parse(code), value);

    [Theory]
    [InlineData("aaa", "Sun")]
    [InlineData("AAA", "Sun")]
    [InlineData("aaaa", "Sunday")]
    [InlineData("AAAA", "Sunday")]
    // LibreOffice's own spellings, and their lengths do not line up with Excel's: zformat.cxx
    // :3983-4004 pairs NN with AAA on the short name and **NNN with AAAA on the long one**, so
    // `nnn` is a long name. Only NNNN adds the locale's day-of-week separator, which is ", " in
    // en-US. Both binaries draw Sun, Sunday and "Sunday, " for the three.
    [InlineData("nn", "Sun")]
    [InlineData("nnn", "Sunday")]
    [InlineData("nnnn", "Sunday, ")]
    public void ADayNameKeyWritesTheWeekday(string code, string expected)
        => Format(code, SundaySerial).ShouldBe(expected);

    [Theory]
    // Not keys: one or two `a`, and a lone `n`. They stay literals, because widening the run
    // would swallow text a format states.
    [InlineData("a", "a")]
    [InlineData("aa", "aa")]
    [InlineData("n", "n")]
    public void ARunThatIsNotAKeyStaysLiteral(string code, string expected)
        => Format(code, SundaySerial).ShouldBe(expected);

    [Theory]
    // An over-long run takes the longest key at its *start* and leaves the tail, which is the
    // half a keyword table does not tell you. Measured on both binaries: `aaaaa` draws
    // `Sundaya`, not `aSunday`.
    [InlineData("aaaaa", "Sundaya")]
    [InlineData("aaaaaa", "Sundayaa")]
    [InlineData("nnnnn", "Sunday, n")]
    public void AnOverLongRunTakesTheLongestKeyFirst(string code, string expected)
        => Format(code, SundaySerial).ShouldBe(expected);

    /// <summary>
    /// The day name sits beside the rest of the date, and the rest of the date is what the file
    /// says — not what LibreOffice draws.
    /// </summary>
    /// <remarks>
    /// <c>ImpIsOtherCalendar</c> (<c>zformat.cxx</c>:3453-3480) answers true for a subformat
    /// holding <c>AAA</c>, <c>AAAA</c>, <c>EC</c>, <c>EEC</c>, <c>R</c>, <c>RR</c>, <c>G</c>,
    /// <c>GG</c> or <c>GGG</c>, and <c>SwitchToOtherCalendar</c> (:3486-3512) then renders the
    /// month and the day in the locale's first non-Gregorian calendar, leaving the year
    /// Gregorian. Under en-US that is the Jewish calendar: measured on both binaries, serial
    /// 46194 — 21 June 2026 — draws <c>04/06/26 Sunday</c> under <c>mm/dd/yy aaaa</c> and
    /// <c>Tammuz 06 2026 Sunday</c> under <c>mmmm dd yyyy aaaa</c>, against <c>06/21/26</c> for
    /// the same serial under <c>mm/dd/yy</c> alone.
    ///
    /// That calendar is not reproduced, so the date beside the day name is Gregorian here. The
    /// format reports it rather than hiding it, which is what <see cref="ADayNameKeyIsReported"/>
    /// asserts and what lets a reader raise a diagnostic.
    /// </remarks>
    [Fact]
    public void TheDateBesideADayNameStaysGregorian()
    {
        // 26.2.4.2 and 24.2.7.2 both draw `05/24/22 Sunday` here — 24 Av 5782 — and both draw
        // `08/21/22 Sunday` for the N spelling, which does not switch the calendar.
        Format("mm/dd/yy\\ aaaa", SundaySerial).ShouldBe("08/21/22 Sunday");
        Format("mm/dd/yy\\ nnn", SundaySerial).ShouldBe("08/21/22 Sunday");
    }

    [Fact]
    public void ADayNameKeyIsReported()
    {
        NumberFormatCode.Parse("mm/dd/yy\\ aaaa").IsFullyReproduced.ShouldBeFalse(
            "AAAA switches the month and day to another calendar, which is not reproduced");
        NumberFormatCode.Parse("aaa").IsFullyReproduced.ShouldBeFalse();

        // The N keys are NOT in ImpIsOtherCalendar's list and do not switch the calendar, so a
        // format using one is reproduced exactly. Measured: `mm/dd/yy nnn` draws 08/21/22 Sunday
        // on both binaries where `mm/dd/yy aaaa` draws 05/24/22 Sunday for the same serial.
        NumberFormatCode.Parse("nnnn").IsFullyReproduced.ShouldBeTrue();
        NumberFormatCode.Parse("mm/dd/yy\\ nnn").IsFullyReproduced.ShouldBeTrue();

        // The control: a date format with no day-name key is reproduced exactly.
        NumberFormatCode.Parse("mm/dd/yy").IsFullyReproduced.ShouldBeTrue();
        NumberFormatCode.Parse("dddd").IsFullyReproduced.ShouldBeTrue();
    }

    /// <summary>
    /// <c>ddd</c> and <c>dddd</c> are the ordinary day-of-month key at three and four letters and
    /// keep the Gregorian calendar, which is the whole reason the two families are separate.
    /// </summary>
    [Theory]
    [InlineData("ddd", "Sun")]
    [InlineData("dddd", "Sunday")]
    [InlineData("mm/dd/yy\\ dddd", "08/21/22 Sunday")]
    public void TheDayOfMonthKeyAlsoNamesTheWeekday(string code, string expected)
        => Format(code, SundaySerial).ShouldBe(expected);

    /// <summary>A day name is a date part, so a format holding one is a date format.</summary>
    [Fact]
    public void ADayNameMakesTheFormatADate()
    {
        NumberFormatCode parsed = NumberFormatCode.Parse("aaaa");
        parsed.IsDateTime.ShouldBeTrue();
        parsed.Sections[0].HasDatePart.ShouldBeTrue();
        parsed.IsTimeOnly.ShouldBeFalse();
    }
}
