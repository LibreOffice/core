using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An OOXML <c>indent</c> level is three spaces of the workbook's default font, measured to the
/// <em>nearest</em> whole twip.
/// </summary>
/// <remarks>
/// <para>
/// The three-spaces rule is <c>sc/source/filter/oox/stylesbuffer.cxx</c>:1263, and one space is
/// <c>xFont-&gt;getCharWidth(' ')</c> (<c>sc/source/filter/oox/unitconverter.cxx</c>:139). That
/// call is <c>OutputDevice::GetTextWidth</c> cast to <c>sal_Int16</c>
/// (<c>toolkit/source/awt/vclxfont.cxx</c>:77), so the space reaches the multiplication as a whole
/// number of twips whatever the face's design metric says, and the only open question is which way
/// the fraction goes.
/// </para>
/// <para>
/// It rounds. Ten-point Liberation Sans has a 55.566-twip space: floor gives 55 and a two-level
/// indent of 16.500 pt, round gives 56 and 16.800 pt, and 26.2.4.2 draws 16.781. Measured over the
/// six default sizes at which the face's 5.5566 twips per point separate the two rules — 10, 12,
/// 14, 16, 28 and 30 pt, one workbook each, an indented cell against an unindented one in the same
/// column so the pen difference is the indent and nothing else — 26.2.4.2 rounds at six of six and
/// 24.2.7.2 at four of six, while truncating is wrong at all six against the target.
/// <c>probes/advance-ppem/indent-twip-rounding.py</c> re-runs it.
/// </para>
/// <para>
/// The assertions below name the mechanism rather than any of those figures: each one first checks
/// that its size genuinely separates truncation from rounding, and only then that the reader chose
/// rounding. A size where the two agree would assert nothing, and it is the check that says so.
/// </para>
/// </remarks>
public sealed class SheetIndentUnitTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>EMU in one twip, so the test does its own division rather than borrowing one.</summary>
    private const double EmuPerTwip = 635.0;

    /// <summary>How many spaces one <c>indent</c> level is worth.</summary>
    private const int SpacesPerLevel = 3;

    [Theory]
    [InlineData(10.0, 2)]
    [InlineData(12.0, 2)]
    [InlineData(14.0, 1)]
    [InlineData(16.0, 3)]
    [InlineData(28.0, 1)]
    [InlineData(30.0, 2)]
    public void AnIndentLevelIsThreeSpacesMeasuredToTheNearestTwip(double points, int levels)
    {
        XElement styleSheet = StyleSheet(points, levels);
        SheetCellFormat format = XlsxCellFormats.Read(styleSheet, XlsxStyles.Read(styleSheet), null)
            .Formats[0];

        SheetFace face = SheetFonts.For(format).ShouldNotBeNull(
            $"{points} pt: the default font must resolve, or nothing here is measured");
        double spaceTwips = SheetText.Measure(" ", face, Length.FromPoints(points)).Emu / EmuPerTwip;

        long truncated = SpacesPerLevel * levels * (long)Math.Floor(spaceTwips);
        long rounded = SpacesPerLevel * levels * (long)Math.Round(spaceTwips);

        // The premise of the case, asserted rather than assumed: at a size where the space is
        // already a whole number of twips the two rules agree and the assertion below is empty.
        truncated.ShouldNotBe(
            rounded,
            $"{points} pt: this size must separate truncation from rounding for the case to test "
            + $"anything (the space measures {spaceTwips:F3} twips)");

        format.Indent.Twips.ShouldBe(
            rounded,
            $"{points} pt at indent {levels}: {SpacesPerLevel} x {levels} spaces of "
            + $"{spaceTwips:F3} twips, each to the nearest twip");
    }

    /// <summary>A <c>styleSheet</c> whose one cell format carries the indent given.</summary>
    private static XElement StyleSheet(double points, int levels) => new(
        XName.Get("styleSheet", Namespace),
        new XElement(
            XName.Get("fonts", Namespace),
            new XElement(
                XName.Get("font", Namespace),
                new XElement(XName.Get("name", Namespace), new XAttribute("val", "Arial")),
                new XElement(
                    XName.Get("sz", Namespace),
                    new XAttribute("val", points.ToString(System.Globalization.CultureInfo.InvariantCulture))))),
        new XElement(
            XName.Get("cellXfs", Namespace),
            new XElement(
                XName.Get("xf", Namespace),
                new XAttribute("fontId", "0"),
                new XAttribute("applyAlignment", "1"),
                new XElement(
                    XName.Get("alignment", Namespace),
                    new XAttribute("horizontal", "left"),
                    new XAttribute("indent", levels.ToString(System.Globalization.CultureInfo.InvariantCulture))))));
}
