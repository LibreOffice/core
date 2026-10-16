using XRay.Office.Core.Numbers;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The formats an id below 164 stands for when the file spells none of them out.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Which table applies is decided by the locale of the application doing the reading,
/// not by the file.</strong> <c>XclNumFmtBuffer::InsertBuiltinFormats</c> walks from
/// <c>meSysLang</c> — <c>rRoot.GetSysLanguage()</c>, and
/// <c>sc/source/filter/inc/xlstyle.hxx</c>:469 calls it <em>"Current system language"</em> —
/// up through the parent tables to <c>spBuiltInFormats_DONTKNOW</c>
/// (<c>sc/source/filter/excel/xlstyle.cxx</c>:1437-1470); the OOXML filter does the same with
/// its own locale (<c>sc/source/filter/oox/numberformatsbuffer.cxx</c>:1919-1975). So the right
/// table for this tree is <em>en-US</em>, and the four cases below are the ones where en-US and
/// the "unknown language" fallback genuinely disagree.
/// </para>
/// <para>
/// Every expectation is a measurement, not a reading of either table:
/// <c>dotnet/probes/numfmt-r68/make-codes.py</c> puts one cell per built-in id in a workbook that
/// declares no <c>&lt;numFmt&gt;</c> at all and renders it through both installed binaries.
/// <strong>24.2.7.2 and 26.2.4.2 agree on all seventeen ids</strong>, so a divergence here is
/// ours. This mattered on <c>Template Pilot Logbook JAR-FCL V3.0.xls</c>, which reads
/// <c>00:00</c> in 126 cells of built-in 20 against both references' <c>0:00</c>, and prints one
/// built-in 14 date as <c>10/11/2003</c> against their <c>11/10/2003</c>.
/// </para>
/// </remarks>
public class BuiltInNumberFormatTests
{
    /// <summary>2:20 as a duration, and 21 August 2022 at 02:20 as a date.</summary>
    private const double Flight = 0.0972222222222222;
    private const double Sunday = 44794.09722222222;

    private static string Format(int id, double value)
    {
        string code = BuiltInNumberFormats.Code(id).ShouldNotBeNull(
            $"id {id} must be a built-in, or the case measures nothing");
        return NumberFormatter.Format(NumberFormatCode.Parse(code), value);
    }

    [Theory]
    // The hour is one letter, so a time before ten in the morning has no leading zero. The
    // "unknown language" table's NF_TIME_HHMM would pad it.
    [InlineData(20, Flight, "2:20")]
    [InlineData(20, 0.0, "0:00")]
    [InlineData(21, Flight, "2:20:00")]
    [InlineData(18, Flight, "2:20 AM")]
    [InlineData(19, Flight, "2:20:00 AM")]
    public void TheTimeBuiltInsUseAOneLetterHour(int id, double value, string expected)
        => Format(id, value).ShouldBe(expected);

    [Theory]
    // Month before day, and a four-digit year. DD/MM/YYYY would swap the first two.
    [InlineData(14, "8/21/2022")]
    [InlineData(22, "8/21/2022 2:20")]
    public void TheDateBuiltInsPutTheMonthFirst(int id, string expected)
        => Format(id, Sunday).ShouldBe(expected);

    [Theory]
    // 37 to 40 are the currency four with a blank symbol, and en-US brackets the negative
    // rather than signing it: NUMFMT_CURRENCY_OPEN_SYMBOL_NUMBER_CLOSE
    // (numberformatsbuffer.cxx:294-298), matching xlstyle.cxx:944-947.
    [InlineData(37, -100.0, "(100)")]
    [InlineData(38, -100.0, "(100)")]
    [InlineData(39, -100.0, "(100.00)")]
    [InlineData(40, -100.0, "(100.00)")]
    [InlineData(40, 1000.0, "1,000.00 ")]
    public void TheBlindCurrencyBuiltInsBracketANegative(int id, double value, string expected)
        => Format(id, value).ShouldBe(expected);

    [Fact]
    public void TheBiffFilterHasNoCurrencyOrAccountingBuiltInsAtAll()
    {
        // The locale row is shared; the covered *set* of ids is not. Both BIFF tables say so in
        // as many words — "5...8 contained in file" (xlstyle.cxx:826) and "41...44 contained in
        // file" (:862) — and carry no entry for either run, while the OOXML tables state all
        // eight (numberformatsbuffer.cxx:294-320, :802). A BIFF file that uses one without
        // writing its own FORMAT record therefore gets the standard format.
        foreach (int id in new[] { 5, 6, 7, 8, 41, 42, 43, 44 })
        {
            BuiltInNumberFormats.BiffCode(id).ShouldBeNull($"BIFF has no built-in {id}");
            BuiltInNumberFormats.Code(id).ShouldNotBeNull($"OOXML has a built-in {id}");
        }

        // 63-66 are built in on both sides and carry the same four codes: the en-US BIFF table
        // names them outright (xlstyle.cxx:949-952) and the OOXML base table reuses 5-8.
        foreach (int id in new[] { 63, 64, 65, 66 })
        {
            BuiltInNumberFormats.BiffCode(id).ShouldBe(BuiltInNumberFormats.Code(id));
            BuiltInNumberFormats.BiffCode(id).ShouldNotBeNull();
        }

        // Everything else is the same table for both.
        foreach (int id in new[] { 0, 1, 9, 12, 14, 20, 22, 37, 40, 45, 49, 76 })
        {
            BuiltInNumberFormats.BiffCode(id).ShouldBe(BuiltInNumberFormats.Code(id));
        }
    }

    [Fact]
    public void TheAccountingBuiltInsCarryAFillAndTheirZeroDash()
    {
        // 41-44 hold a `*` fill and, in the two-decimal pair, the `??` that keeps the dash clear
        // of the column's decimal point. The fill marker is where a caller with a column width
        // pads; the `?` is U+2007 on 26.2.4.2, which is the tree's target.
        NumberFormatCode.Parse(BuiltInNumberFormats.Code(43)!).HasFillDirective.ShouldBeTrue();
        NumberFormatCode.Parse(BuiltInNumberFormats.Code(44)!).HasFillDirective.ShouldBeTrue();

        Format(43, -100.0).ShouldContain("(100.00)");
        Format(44, -100.0).ShouldContain("$");
    }

    [Fact]
    public void TheInternationalAliasesResolveToTheirBase()
    {
        // 23-36 and 50-81 are locale spellings of an earlier entry rather than formats of their
        // own (NUMFMT_REUSE, numberformatsbuffer.cxx:466-524).
        BuiltInNumberFormats.Code(76).ShouldBe(BuiltInNumberFormats.Code(20));
        BuiltInNumberFormats.Code(71).ShouldBe(BuiltInNumberFormats.Code(14));
        BuiltInNumberFormats.Code(63).ShouldBe(BuiltInNumberFormats.Code(5));
        BuiltInNumberFormats.Code(23).ShouldBe("General");

        // 82 to 163 are reserved and must not resolve, so a file using one is reported rather
        // than silently given somebody else's format.
        BuiltInNumberFormats.Code(82).ShouldBeNull();
        BuiltInNumberFormats.Code(163).ShouldBeNull();
        BuiltInNumberFormats.Code(BuiltInNumberFormats.FirstUserIndex).ShouldBeNull();
    }
}
