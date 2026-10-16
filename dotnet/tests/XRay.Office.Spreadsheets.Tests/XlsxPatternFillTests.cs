using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A hatched fill is drawn as one blended colour, and the blend is arithmetic rather than a
/// choice between the two colours it names.
/// </summary>
/// <remarks>
/// <para>
/// Calc has no hatched cell background, so <c>Fill::finalizeImport</c> mixes the pattern colour
/// into the fill colour at a weight the pattern name decides and stores the single result
/// (<c>sc/source/filter/oox/stylesbuffer.cxx</c>:2014-2054). Both readers used to take one of the
/// two colours whole — the foreground for a <c>dxf</c>, the background for a cell — which is
/// right only at <c>solid</c>'s full weight.
/// </para>
/// <para>
/// The witness is <c>072_Gantt_project_planner_dde00e33.xlsx</c>, whose planned, actual and
/// overrun bands are three <c>lightUp</c> rules stating <strong>the same foreground</strong> —
/// the theme's accent, <c>#735773</c> — over three different backgrounds. Taking the foreground
/// drew all three in one dark colour indistinguishable from the solid rule above them, so the
/// chart came out a single slab. The expected values here are read out of 26.2.4.2's own
/// rendering of that workbook.
/// </para>
/// <para>
/// <strong>The rounding is measured and is not a detail.</strong> <c>lclGetMixedColor</c> is
/// integer arithmetic and C++ integer division truncates towards zero, so a component whose
/// pattern is darker than its fill rounds <em>up</em>: <c>#735773</c> over <c>#CAB9CA</c> at
/// <c>lightUp</c>'s weight is <c>#B5A1B5</c> and flooring would give <c>#B4A1B4</c>. The
/// reference draws <c>#B5A1B5</c>.
/// </para>
/// </remarks>
public sealed class XlsxPatternFillTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    private static Colour? Resolve(string pattern, bool differential)
    {
        XElement fill = XElement.Parse($"<fill xmlns=\"{Namespace}\">{pattern}</fill>");
        return XlsxPatternFill.Resolve(fill, XlsxPalette.Read(null, null), differential);
    }

    private static string Hatch(string type, string foreground, string background)
        => $"<patternFill patternType=\"{type}\">{foreground}{background}</patternFill>";

    [Theory]
    // `Plan`: the accent over an automatic background, which is the window colour — white.
    [InlineData("lightUp", "<fgColor rgb=\"FF735773\"/>", "<bgColor auto=\"1\"/>", "#DCD5DC")]
    // `Actual`: the same accent over the accent at a 60 % tint.
    [InlineData("lightUp", "<fgColor rgb=\"FF735773\"/>", "<bgColor rgb=\"FFCAB9CA\"/>", "#B5A1B5")]
    // `ActualBeyond`: the same accent again, over the second accent at a 60 % tint.
    [InlineData("lightUp", "<fgColor rgb=\"FF735773\"/>", "<bgColor rgb=\"FFF6DDB9\"/>", "#D6BCA8")]
    public void AHatchIsTheTwoColoursMixedAtTheWeightItsNameCarries(
        string type, string foreground, string background, string expected)
        => Resolve(Hatch(type, foreground, background), differential: true)
            .ShouldNotBeNull()
            .ToString()
            .ShouldBe(expected);

    [Theory]
    // The seventeen weights of `stylesbuffer.cxx`:2014-2035, drawn with both colours automatic —
    // the pattern's is the window text and the fill's the window background, so each reads as its
    // own weight out of 0x80: `255 + trunc(-255 * alpha / 128)`. The truncation is what puts each
    // of these one above the rounded figure.
    [InlineData("gray0625", "#F0F0F0")]
    [InlineData("gray125", "#E0E0E0")]
    [InlineData("lightUp", "#C0C0C0")]
    [InlineData("lightTrellis", "#A0A0A0")]
    [InlineData("lightGrid", "#909090")]
    [InlineData("darkUp", "#808080")]
    [InlineData("darkTrellis", "#404040")]
    [InlineData("solid", "#000000")]
    // A name this does not know keeps the initial 0x80, which draws the pattern colour whole —
    // the answer the readers gave before any of this existed, so an unknown pattern cannot
    // regress.
    [InlineData("notAPatternName", "#000000")]
    public void EachPatternNameCarriesItsOwnWeight(string type, string expected)
        => Resolve($"<patternFill patternType=\"{type}\"/>", differential: false)
            .ShouldNotBeNull()
            .ToString()
            .ShouldBe(expected);

    [Fact]
    public void ADifferentialFillStatesItsSolidColourInTheBackground()
    {
        // The `mbDxf` swap: a stated background under no pattern or a solid one is moved onto the
        // pattern colour. It is why a `dxf` names its fill in `bgColor` where a cell names it in
        // `fgColor`, and 2734 of the corpus's `dxf` fills state only a background.
        Resolve("<patternFill><bgColor rgb=\"FF00B050\"/></patternFill>", differential: true)
            .ShouldNotBeNull().ToString().ShouldBe("#00B050");

        Resolve("<patternFill patternType=\"solid\"><fgColor auto=\"1\"/>"
                + "<bgColor rgb=\"FF735773\"/></patternFill>", differential: true)
            .ShouldNotBeNull().ToString().ShouldBe("#735773");
    }

    [Fact]
    public void ACellFillStatesItsSolidColourInTheForeground()
        => Resolve("<patternFill patternType=\"solid\"><fgColor rgb=\"FF00B050\"/>"
                   + "<bgColor indexed=\"64\"/></patternFill>", differential: false)
            .ShouldNotBeNull().ToString().ShouldBe("#00B050");

    [Theory]
    // `patternType="none"` paints nothing, in both readers.
    [InlineData("<patternFill patternType=\"none\"/>", true)]
    [InlineData("<patternFill patternType=\"none\"/>", false)]
    // A solid `dxf` pattern stating neither colour is turned off outright — the second arm of the
    // `mbDxf` branch.
    [InlineData("<patternFill patternType=\"solid\"/>", true)]
    // A `dxf` stating no pattern at all keeps `mnPattern`'s initial `XML_none`, so a foreground
    // on its own paints nothing. A cell's fill is read the same way for the same reason.
    [InlineData("<patternFill><fgColor rgb=\"FF00B050\"/></patternFill>", true)]
    [InlineData("<patternFill><fgColor rgb=\"FF00B050\"/></patternFill>", false)]
    // **An absent `patternType` and a stated `none` are not the same thing**, although the swap
    // above treats a `dxf`'s background alike in every other respect: `mbPatternUsed` is set by
    // the attribute's mere presence, so a stated `none` beside a `bgColor` does not swap and
    // `mnPattern == XML_none` then turns the fill off. Folding the two together made this tree
    // paint a black cell for exactly this markup, which two corpus `dxf` state — a font-only rule
    // that writes its "no fill" out longhand — and it is what the round's confinement sweep
    // caught: two documents moved that had no hatch and no expression rule in them.
    [InlineData("<patternFill patternType=\"none\"><bgColor auto=\"1\"/></patternFill>", true)]
    [InlineData("<patternFill patternType=\"none\"><bgColor rgb=\"FF00B050\"/></patternFill>", true)]
    public void AFillThatPaintsNothingAnswersNothing(string pattern, bool differential)
        => Resolve(pattern, differential).ShouldBeNull();
}
