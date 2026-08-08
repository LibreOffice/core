using Paperless.Core.Extraction;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Where the date and time a header prints come from.
/// </summary>
/// <remarks>
/// Two claims, and they are separate. One is correctness: <c>&amp;D</c> and <c>&amp;T</c> are
/// properties of the printout, so every page of one job prints the same instant however long the
/// job takes. The other is reproducibility: with <c>SOURCE_DATE_EPOCH</c> set, two renderings of
/// one document are byte-equal, which is what makes a corpus-wide before-and-after diff mean
/// anything.
/// </remarks>
public sealed class SheetPrintInstantTests
{
    /// <summary>
    /// A zone in which local time is not UTC, so a conversion to local time cannot pass unnoticed.
    /// </summary>
    private const string OffsetZone = "Pacific/Kiritimati";

    [Theory]
    [InlineData("1700000000")]
    [InlineData("  1700000000  ")]
    [InlineData("0")]
    public void AnEpochIsReadAsTheInstantItNames(string raw)
        => SheetPrintInstant.Parse(raw)
            .ShouldBe(DateTimeOffset.FromUnixTimeSeconds(long.Parse(raw.Trim(),
                          System.Globalization.CultureInfo.InvariantCulture)).UtcDateTime);

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("not-a-number")]
    [InlineData("1700000000.5")]
    [InlineData("-1")]
    [InlineData("999999999999")]     // Past 9999-12-31; FromUnixTimeSeconds would throw.
    public void AValueThatNamesNoInstantIsIgnoredRatherThanThrownOn(string? raw)
        => SheetPrintInstant.Parse(raw).ShouldBeNull();

    [Fact]
    public void EveryPageOfOnePrintoutCarriesTheSameInstant()
    {
        using EpochVariable pinned = new("1700000000");
        using TimeZone zone = new(OffsetZone);

        SpreadsheetPages pages = ThreePages();
        pages.Count.ShouldBeGreaterThan(1);

        DateTime expected = DateTimeOffset.FromUnixTimeSeconds(1700000000).UtcDateTime;
        pages.Pages.ShouldAllBe(page => page.Printed == expected);
    }

    [Fact]
    public void TwoPrintoutsMadeUnderOnePinnedEpochAgree()
    {
        using EpochVariable pinned = new("1700000000");

        ThreePages().Pages[0].Printed.ShouldBe(ThreePages().Pages[0].Printed);
    }

    [Fact]
    public void WithNoEpochSetTheClockIsRead()
    {
        using EpochVariable unset = new(null);

        DateTime before = DateTime.Now.AddMinutes(-1);
        SpreadsheetPages pages = ThreePages();

        pages.Pages[0].Printed.ShouldBeGreaterThan(before);

        // And still one instant for the whole job, which is the half that has nothing to do with
        // the variable: the pages differ by nothing at all rather than by a few ticks.
        pages.Pages.ShouldAllBe(page => page.Printed == pages.Pages[0].Printed);
    }

    /// <summary>A workbook of three sheets, so its printout has more than one page.</summary>
    private static SpreadsheetPages ThreePages()
        => new([Sheet("One"), Sheet("Two"), Sheet("Three")]);

    private static SheetLayout Sheet(string name)
    {
        ContentTable table = new();
        ContentTableRow row = new() { Index = 0 };
        ContentTableCell cell = new() { Row = 0, Column = 0, Value = name };
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = name });
        cell.Children.Add(paragraph);
        row.Children.Add(cell);
        table.Children.Add(row);

        return new SheetLayout { Name = name, Cells = table };
    }

    /// <summary>Sets <c>TZ</c> for the length of a test, so a local-time conversion shows up.</summary>
    private sealed class TimeZone : IDisposable
    {
        private readonly string? _previous = Environment.GetEnvironmentVariable("TZ");

        public TimeZone(string? value)
        {
            Environment.SetEnvironmentVariable("TZ", value);
            TimeZoneInfo.ClearCachedData();
        }

        public void Dispose()
        {
            Environment.SetEnvironmentVariable("TZ", _previous);
            TimeZoneInfo.ClearCachedData();
        }
    }

    /// <summary>Sets <c>SOURCE_DATE_EPOCH</c> for the length of a test and puts it back.</summary>
    private sealed class EpochVariable : IDisposable
    {
        private readonly string? _previous
            = Environment.GetEnvironmentVariable(SheetPrintInstant.EpochVariable);

        public EpochVariable(string? value)
            => Environment.SetEnvironmentVariable(SheetPrintInstant.EpochVariable, value);

        public void Dispose()
            => Environment.SetEnvironmentVariable(SheetPrintInstant.EpochVariable, _previous);
    }
}
