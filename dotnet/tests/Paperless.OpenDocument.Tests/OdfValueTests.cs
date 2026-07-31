using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>Tests for the ODF attribute value syntaxes.</summary>
public class OdfValueTests
{
    [Theory]
    [InlineData("2.54cm", 914400)]
    [InlineData("1in", 914400)]
    [InlineData("72pt", 914400)]
    [InlineData("6pc", 914400)]
    [InlineData("25.4mm", 914400)]
    [InlineData("96px", 914400)]
    [InlineData("-0.5in", -457200)]
    [InlineData("0cm", 0)]
    // A unitless value is read as 1/100 mm, ODF's own internal unit — files written by
    // converters do omit the unit.
    [InlineData("2540", 914400)]
    public void LengthsParseToExactEmus(string value, long expectedEmu)
        => OdfValue.ParseLength(value)!.Value.ShouldBe(Length.FromEmu(expectedEmu));

    [Theory]
    [InlineData("21.001cm")]
    [InlineData("0.0835in")]
    public void CommonPageMeasurementsRoundTripThroughMm100(string value)
    {
        // ODF's own unit is 1/100 mm, so anything LibreOffice writes must survive the trip
        // without drift; this is the property that makes EMUs the right storage choice.
        Length length = OdfValue.ParseLength(value)!.Value;
        Length.FromMm100(length.Mm100).Mm100.ShouldBe(length.Mm100);
    }

    [Theory]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("12em")]
    [InlineData("115%")]
    [InlineData("wide")]
    public void AnUnparseableLengthIsNullRatherThanAnException(string value)
        => OdfValue.ParseLength(value).ShouldBeNull();

    [Fact]
    public void FractionalLengthsDoNotDependOnTheAmbientCulture()
    {
        // ODF measurements are XSL values and always use a full stop. Parsing under a
        // comma-decimal culture would silently misread every fractional measurement.
        System.Globalization.CultureInfo original = System.Globalization.CultureInfo.CurrentCulture;
        try
        {
            System.Globalization.CultureInfo.CurrentCulture = new System.Globalization.CultureInfo("de-DE");
            OdfValue.ParseLength("2.5cm")!.Value.Millimetres.ShouldBe(25, 0.001);
        }
        finally
        {
            System.Globalization.CultureInfo.CurrentCulture = original;
        }
    }

    [Theory]
    [InlineData("115%", 1.15)]
    [InlineData("100%", 1.0)]
    [InlineData("-33%", -0.33)]
    [InlineData("63.5%", 0.635)]
    public void PercentagesComeBackAsFractions(string value, double expected)
        => OdfValue.ParsePercentage(value)!.Value.ShouldBe(expected, 1e-9);

    [Theory]
    [InlineData("true", true)]
    [InlineData("false", false)]
    // XSD booleans, so the numeric spellings are legal and do appear.
    [InlineData("1", true)]
    [InlineData("0", false)]
    public void BooleansAcceptBothSpellings(string value, bool expected)
        => OdfValue.ParseBoolean(value).ShouldBe(expected);

    [Theory]
    [InlineData("yes")]
    [InlineData("True")]
    public void AnUnrecognisedBooleanIsNull(string value)
        => OdfValue.ParseBoolean(value).ShouldBeNull();

    [Fact]
    public void ColoursParseAsSixDigitHexOrTheTransparentKeyword()
    {
        OdfValue.ParseColour("#729fcf").ShouldBe(new Colour(0x72, 0x9F, 0xCF));
        OdfValue.ParseColour("#000000").ShouldBe(Colour.Black);
        OdfValue.ParseColour("transparent").ShouldBe(Colour.Transparent);
        OdfValue.ParseColour("#abc").ShouldBeNull();
        OdfValue.ParseColour("red").ShouldBeNull();
    }

    [Fact]
    public void ATimestampWithoutAZoneIsNotShiftedByTheReadingMachinesZone()
    {
        // Shifting it would make the same file report different timestamps on different
        // machines, which is worse than reporting an unknown zone as UTC.
        DateTimeOffset parsed = OdfValue.ParseDateTime("2026-01-02T03:04:05")!.Value;
        parsed.Offset.ShouldBe(TimeSpan.Zero);
        parsed.Year.ShouldBe(2026);
        parsed.Hour.ShouldBe(3);
        parsed.Minute.ShouldBe(4);
        parsed.Second.ShouldBe(5);
    }

    [Fact]
    public void AnExplicitZoneIsHonoured()
        => OdfValue.ParseDateTime("2026-01-02T03:04:05+02:00")!.Value.Offset
                   .ShouldBe(TimeSpan.FromHours(2));

    [Fact]
    public void ADateWithoutATimeParses()
        => OdfValue.ParseDateTime("2026-07-30")!.Value.Date.ShouldBe(new DateTime(2026, 7, 30));

    [Theory]
    [InlineData("PT1H23M45S", 1, 23, 45)]
    [InlineData("PT0S", 0, 0, 0)]
    [InlineData("PT14H30M00S", 14, 30, 0)]
    [InlineData("PT90M", 1, 30, 0)]
    public void DurationsParse(string value, int hours, int minutes, int seconds)
        => OdfValue.ParseDuration(value)!.Value
                   .ShouldBe(new TimeSpan(hours, minutes, seconds));

    [Fact]
    public void ADayComponentContributesToADuration()
        => OdfValue.ParseDuration("P1DT2H")!.Value.ShouldBe(new TimeSpan(1, 2, 0, 0));

    [Theory]
    [InlineData("")]
    [InlineData("P")]
    [InlineData("1H")]
    // Months cannot become an exact TimeSpan, and an editing duration never uses them, so a
    // month component means this is not one.
    [InlineData("P2M")]
    [InlineData("PTXH")]
    public void AMalformedDurationIsNullRatherThanAnException(string value)
        => OdfValue.ParseDuration(value).ShouldBeNull();

    [Theory]
    [InlineData("Text_20_body", "Text body")]
    [InlineData("Heading_20_1", "Heading 1")]
    [InlineData("Standard", "Standard")]
    // An underscore that is not part of an escape survives: the pattern requires hex digits.
    [InlineData("My_Style", "My_Style")]
    [InlineData("List_20_Bullet", "List Bullet")]
    [InlineData("trailing_", "trailing_")]
    public void StyleNamesAreDecodedBackToWhatAUserSees(string encoded, string expected)
        => OdfNames.Decode(encoded).ShouldBe(expected);
}
