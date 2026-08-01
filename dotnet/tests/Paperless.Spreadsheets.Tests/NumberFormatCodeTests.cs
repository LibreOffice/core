using Paperless.Core.Numbers;
using Paperless.Spreadsheets.Ooxml;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests what a parsed format code <em>says</em>, as distinct from what it renders.
/// </summary>
/// <remarks>
/// The two halves are separable in the decomposed engine and separable here too:
/// <c>NumberFormatterTests</c> — which moved into <c>Paperless.Core.Tests</c> with the engine it
/// exercises — asserts the text a code produces, and this asserts the
/// classification a reader acts on before any text exists — whether a stored double is a date,
/// a duration or a number, and which built-in code an index stands for. Those are the
/// decisions that change a cell's *type* in the content tree, so they are worth pinning
/// separately from its display string.
/// </remarks>
public class NumberFormatCodeTests
{
    [Theory]
    // A spreadsheet has no date type: the format is the only thing that says a serial number
    // is a date, so a reader that skips this reports 46233 where the file shows a Thursday.
    [InlineData("yyyy-mm-dd", true, false, false)]
    [InlineData("DD/MM/YYYY", true, false, false)]
    [InlineData("dd/mm/yyyy hh:mm", true, false, false)]
    [InlineData("hh:mm", true, true, false)]
    [InlineData("[h]:mm", true, true, true)]
    [InlineData("[h]:mm:ss", true, true, true)]
    [InlineData("#,##0.00", false, false, false)]
    [InlineData("General", false, false, false)]
    public void AFormatSaysWhetherItsCellHoldsADateADurationOrANumber(
        string code, bool isDateTime, bool isTimeOnly, bool isElapsed)
    {
        NumberFormatCode parsed = NumberFormatCode.Parse(code);

        parsed.IsDateTime.ShouldBe(isDateTime);
        parsed.IsTimeOnly.ShouldBe(isTimeOnly);

        // The elapsed flag is the third state, and it is the one both readers branch on: a
        // time-only format yields a TimeSpan either way, but "[h]:mm" keeps the whole days
        // where "hh:mm" wraps at midnight, so 1.5 is 36 hours under one and 12 under the
        // other. A format showing both a date and a time is a point in time, not a length.
        parsed.Sections[0].HasElapsed.ShouldBe(isElapsed);
    }

    [Fact]
    public void AConditionSelectsASubformatRatherThanDefeatingTheReader()
    {
        // This used to be reported as not understood. It is reproduced now, which matters
        // beyond the exotic-looking syntax: built-in ids 5 to 8 and 41 to 44 are
        // conditional-shaped accounting formats, so the path is a common one.
        NumberFormatCode conditional = NumberFormatCode.Parse("[>100]0.0;0.00");

        conditional.Sections.Count.ShouldBe(2);
        conditional.Sections[0].Condition.ShouldNotBeNull();
        conditional.SelectFor(500).ShouldBe(conditional.Sections[0]);
        conditional.SelectFor(5).ShouldBe(conditional.Sections[1]);

        // A code with no condition falls back to the positional rule, and that is not the
        // same as "the first subformat always wins".
        NumberFormatCode positional = NumberFormatCode.Parse("0.0;-0.0;\"nil\"");
        positional.SelectFor(-1).ShouldBe(positional.Sections[1]);
        positional.SelectFor(0).ShouldBe(positional.Sections[2]);
    }

    [Fact]
    public void ADirectiveThatChangesTheDigitsIsReportedRatherThanGuessedAt()
    {
        // What is genuinely not reproduced is a substitution of the characters themselves: a
        // numeral system (LibreOffice's [NatNum]/[DBNum] modifiers,
        // svl/source/numbers/zforscan.cxx:215) or another calendar's era. Ignoring one
        // produces plausible Western digits that are not what the cell shows, so the readers
        // raise a diagnostic instead.
        NumberFormatCode.Parse("[NatNum1]0").IsFullyReproduced.ShouldBeFalse();
        NumberFormatCode.Parse("[DBNum2]0").IsFullyReproduced.ShouldBeFalse();
        NumberFormatCode.Parse("[~buddhist]yyyy").IsFullyReproduced.ShouldBeFalse();

        // A colour, a condition and a currency symbol all leave the characters alone.
        NumberFormatCode.Parse("#,##0.00").IsFullyReproduced.ShouldBeTrue();
        NumberFormatCode.Parse("# ??/??").IsFullyReproduced.ShouldBeTrue();
        NumberFormatCode.Parse("[>100]0.0;0.00").IsFullyReproduced.ShouldBeTrue();
        NumberFormatCode.Parse("[$£-809]#,##0.00").IsFullyReproduced.ShouldBeTrue();
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

    [Fact]
    public void TheBiffAndOoxmlBuiltInTablesDisagreeAndAreMeantTo()
    {
        // The one place the two readers deliberately do not share a table. LibreOffice reads
        // BIFF through spBuiltInFormats_DONTKNOW (sc/source/filter/excel/xlstyle.cxx:819) and
        // OOXML through the per-locale table in
        // sc/source/filter/oox/numberformatsbuffer.cxx:436, whose en_US row is the one used
        // here — and the two spell the same index differently. Merging them would silently
        // change what one of the readers extracts, so the disagreement is asserted rather
        // than left to be tidied away by somebody who notices the duplication.
        NumberFormatCode biff = NumberFormatCode.Parse(BuiltInNumberFormats.Code(14));
        NumberFormatCode ooxml = XlsxStyles.Empty.FormatForId(14);

        biff.Code.ShouldBe("DD/MM/YYYY");
        ooxml.Code.ShouldBe("M/D/YYYY");

        // Same day, two orders — which is the locale problem, not a defect in either reader.
        NumberFormatter.Format(biff, 46233).ShouldBe("30/07/2026");
        NumberFormatter.Format(ooxml, 46233).ShouldBe("7/30/2026");

        // Index 37 differs in the same way: BIFF signs a negative and OOXML brackets it.
        NumberFormatter.Format(NumberFormatCode.Parse(BuiltInNumberFormats.Code(37)), -1234)
                       .ShouldBe("-1,234");
        NumberFormatter.Format(XlsxStyles.Empty.FormatForId(37), -1234)
                       .ShouldBe("(1,234)");
    }
}
