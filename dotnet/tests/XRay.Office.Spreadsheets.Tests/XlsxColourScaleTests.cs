using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a <c>cfRule type="colorScale"</c> paints.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is a colour LibreOffice 26.2.4.2 actually drew, not a colour derived
/// from the specification. The twelve in <see cref="TheTwelveColoursOfTheCorpusWitness"/> are read
/// straight off <c>003_advanced_excel_pie.xlsx</c>'s reference PDF, whose sheet 2 states one
/// three-stop scale over <c>B2:B13</c> holding 93 to 170 in steps of seven; the rest reproduce the
/// authored cases in <c>probes/sheets-r58/probe-colorscale.py</c>, which were rendered through the
/// installed binary with two controls ahead of them — the same sheet with no rule at all, which
/// draws nothing, and the same sheet with a stated solid fill, which draws that.
/// </para>
/// <para>
/// The one that is not a colour is <see cref="AScaleIsInvisibleToThePrintAreaScan"/>, and it is
/// the regression guard: a conditional fill that reached <see cref="SheetFormatting"/>'s stated
/// cells would extend how far the sheet prints, and one corpus rule is declared over
/// <c>N18:Q1048576</c>.
/// </para>
/// </remarks>
public sealed class XlsxColourScaleTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>The stops of the scale every `advanced_excel` workbook states.</summary>
    private const string ThreeStop =
        "<cfvo type=\"min\"/><cfvo type=\"percentile\" val=\"50\"/><cfvo type=\"max\"/>"
        + "<color rgb=\"FFF8696B\"/><color rgb=\"FFFFEB84\"/><color rgb=\"FF63BE7B\"/>";

    private const string TwoStop =
        "<cfvo type=\"min\"/><cfvo type=\"max\"/>"
        + "<color rgb=\"FFF8696B\"/><color rgb=\"FF63BE7B\"/>";

    private static readonly double?[] Witness =
        [93, 100, 107, 114, 121, 128, 135, 142, 149, 156, 163, 170];

    [Fact]
    public void ASheetWithNoRuleAtAllPaintsNothing()
    {
        // The control, and it runs first for the same reason the probe's does: an assertion that
        // a colour appears is worth nothing until the same instrument has been shown to answer
        // "none" when there is none.
        SheetFormatting formatting = Read(Witness, rule: null);

        for (int row = 1; row <= 12; row++)
        {
            formatting.At(row, 1).Background.ShouldBeNull();
        }
    }

    [Fact]
    public void TheTwelveColoursOfTheCorpusWitness()
    {
        // Read off LibreOffice 26.2.4.2's own rendering of 003_advanced_excel_pie.xlsx page 3.
        string[] expected =
        [
            "#F8696B", "#F9806F", "#FA9874", "#FBAF78", "#FDC77D", "#FEDF81",
            "#F1E784", "#D5DF82", "#B9D780", "#9CCF7F", "#80C77D", "#63BE7B",
        ];

        SheetFormatting formatting = Read(Witness, Scale("B2:B13", ThreeStop));

        for (int i = 0; i < expected.Length; i++)
        {
            formatting.At(1 + i, 1).Background.ShouldNotBeNull().ToString().ShouldBe(expected[i]);
        }
    }

    [Fact]
    public void TheInterpolationTruncatesTheDeltaRatherThanTheResult()
    {
        // The seventh cell is the discriminator, and it is the whole of `GetColorValue`. Its red
        // channel is 255 + (int)(-156 × 0.0909) = 255 − 14 = 241 = 0xF1. Rounding the *sum*
        // instead — 240.82 → 241 here but 128.64 → 129 on the second cell's green — cannot
        // satisfy both, and flooring the sum gives 0xF0 here.
        SheetFormatting formatting = Read(Witness, Scale("B2:B13", ThreeStop));

        formatting.At(7, 1).Background.ShouldNotBeNull().R.ShouldBe((byte)0xF1);
        formatting.At(2, 1).Background.ShouldNotBeNull().G.ShouldBe((byte)0x80);
    }

    [Fact]
    public void ACellThatIsNotNumericTakesNoColourAndDoesNotSetTheRange()
    {
        // `if(!rCell.hasNumeric()) return {}`, and the authored `08-text-in-range` draws 0 of 11.
        // The second half matters more: the text cell must not participate in the minimum either,
        // so the two numbers left take the two end colours outright.
        SheetFormatting formatting = Read(
            [1.0, null, 3.0], Scale("B2:B4", TwoStop),
            texts: new Dictionary<int, string> { [3] = "middle" });

        formatting.At(1, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#F8696B");
        formatting.At(2, 1).Background.ShouldBeNull();
        formatting.At(3, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#63BE7B");
    }

    [Fact]
    public void AScaleReplacesTheFillTheCellStates()
    {
        // `07-with-own-fill`: eleven cells stating a solid green draw the scale on all eleven and
        // the green nowhere. Calc rebuilds ATTR_BACKGROUND from the scale after the stated fill.
        SheetFormatting formatting = Read([0.0, 5.0, 10.0], Scale("B2:B4", TwoStop), statedFill: true);

        formatting.At(1, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#F8696B");
        formatting.At(3, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#63BE7B");
    }

    [Fact]
    public void AScaleIsInvisibleToThePrintAreaScan()
    {
        // The regression guard. `SheetDecorationArea` decides how far a sheet prints from these
        // three, and a rule declared past the data would otherwise move a page count. 26.2.4.2
        // does not extend it either: an authored scale over B2:B40 on data reaching B12 still
        // prints one page.
        SheetFormatting formatting = Read([0.0, 5.0, 10.0], Scale("B2:B40", TwoStop));

        formatting.At(1, 1).Background.ShouldNotBeNull();
        formatting.Cells.ShouldBeEmpty();
        formatting.Rows.ShouldBeEmpty();
        formatting.ColumnRuns.ShouldBeEmpty();
        formatting.IsEmpty.ShouldBeFalse();
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void ThePriorityAttributeDecidesBetweenTwoOverlappingScales(bool winnerFirst)
    {
        // The discriminating pair: case 14 alone cannot separate "highest priority wins" from
        // "last in document order wins", because its winner was both. Rendered both ways on
        // 26.2.4.2 the red-to-green scale paints in each, and the loser's blue-to-magenta ramp
        // appears in neither.
        string red = "<conditionalFormatting sqref=\"B2:B4\">"
            + $"<cfRule type=\"colorScale\" priority=\"1\"><colorScale>{TwoStop}</colorScale>"
            + "</cfRule></conditionalFormatting>";
        string blue = "<conditionalFormatting sqref=\"B2:B4\">"
            + "<cfRule type=\"colorScale\" priority=\"9\"><colorScale>"
            + "<cfvo type=\"min\"/><cfvo type=\"max\"/>"
            + "<color rgb=\"FF0000FF\"/><color rgb=\"FFFF00FF\"/>"
            + "</colorScale></cfRule></conditionalFormatting>";

        SheetFormatting formatting = Read(
            [0.0, 5.0, 10.0], winnerFirst ? red + blue : blue + red);

        formatting.At(1, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#F8696B");
        formatting.At(3, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#63BE7B");
    }

    [Fact]
    public void PercentStopsAreTakenFromTheRangesOwnSpanAndClampOutsideThem()
    {
        // `05-percent-25-75` over 0…10: the stops land on 2.5 and 7.5, so the first three cells
        // and the last three are flat end colours and only the middle five interpolate. The two
        // asserted here are the clamp and the first interpolated step — 3 gives
        // 248 + (int)(-149 × 0.1) = 234 = 0xEA.
        SheetFormatting formatting = Read(
            [0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0],
            Scale("B2:B12", "<cfvo type=\"percent\" val=\"25\"/><cfvo type=\"percent\" val=\"75\"/>"
                            + "<color rgb=\"FFF8696B\"/><color rgb=\"FF63BE7B\"/>"));

        formatting.At(2, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#F8696B");
        formatting.At(4, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#EA716C");
        formatting.At(9, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#63BE7B");
    }

    [Fact]
    public void APercentileStopOverAnEvenCountFallsBetweenTheMiddlePair()
    {
        // `GetPercentile` indexes at p × (n − 1) and interpolates the remainder, so the twelve
        // witness values put the middle stop at 131.5 rather than on either of 128 and 135. That
        // is what makes the seventh cell the first of the yellow-to-green leg rather than the
        // last of the red-to-yellow one, and the sixth and seventh colours straddle the stop.
        SheetFormatting formatting = Read(Witness, Scale("B2:B13", ThreeStop));

        formatting.At(6, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#FEDF81");
        formatting.At(7, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#F1E784");
    }

    [Fact]
    public void AStopNamedByThemeSlotResolvesThroughTheWorkbooksTheme()
    {
        // `075_Idea_planner_tasks` takes all three of its stops from the theme; the reference
        // draws #D74F0C, #FFE366 and #8FAE2A on its four cells. Asserted untinted here, because
        // XlsxTint owns the tinted transform and has its own tests.
        const string themed =
            "<cfvo type=\"num\" val=\"-1\"/><cfvo type=\"num\" val=\"0\"/><cfvo type=\"num\" val=\"1\"/>"
            + "<color theme=\"4\"/><color theme=\"5\"/><color theme=\"6\"/>";

        SheetFormatting formatting = Read([-1.0, 0.0, 1.0], Scale("B2:B4", themed), theme: true);

        formatting.At(1, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#112233");
        formatting.At(2, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#445566");
        formatting.At(3, 1).Background.ShouldNotBeNull().ToString().ShouldBe("#778899");
    }

    private static string Scale(string sqref, string body)
        => $"<conditionalFormatting sqref=\"{sqref}\">"
           + $"<cfRule type=\"colorScale\" priority=\"1\"><colorScale>{body}</colorScale></cfRule>"
           + "</conditionalFormatting>";

    /// <summary>One column of numbers at B, from row 2 down, plus whatever rules are given.</summary>
    private static SheetFormatting Read(
        double?[] values,
        string? rule,
        bool statedFill = false,
        bool theme = false,
        Dictionary<int, string>? texts = null)
    {
        string rows = string.Empty;
        for (int i = 0; i < values.Length; i++)
        {
            int r = 2 + i;
            string s = statedFill ? " s=\"1\"" : string.Empty;
            string cell = values[i] is { } number
                ? $"<c r=\"B{r}\"{s}><v>{number.ToString(CultureInfo.InvariantCulture)}</v></c>"
                : texts is not null && texts.TryGetValue(r, out string? text)
                    ? $"<c r=\"B{r}\"{s} t=\"inlineStr\"><is><t>{text}</t></is></c>"
                    : string.Empty;
            rows += $"<row r=\"{r}\">{cell}</row>";
        }

        XElement worksheet = XElement.Parse(
            $"<worksheet xmlns=\"{Namespace}\"><sheetData>{rows}</sheetData>{rule}</worksheet>");

        XElement styles = XElement.Parse(
            $"<styleSheet xmlns=\"{Namespace}\">"
            + "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill>"
            + "<fill><patternFill patternType=\"gray125\"/></fill>"
            + "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF00FF00\"/></patternFill></fill>"
            + "</fills><borders count=\"1\"><border/></borders>"
            + "<cellXfs count=\"2\"><xf fillId=\"0\" borderId=\"0\"/>"
            + "<xf fillId=\"2\" borderId=\"0\" applyFill=\"1\"/></cellXfs></styleSheet>");

        XElement? themeRoot = theme
            ? XElement.Parse(
                "<theme xmlns=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
                + "<themeElements><clrScheme name=\"probe\">"
                + "<dk1><srgbClr val=\"000000\"/></dk1><lt1><srgbClr val=\"FFFFFF\"/></lt1>"
                + "<dk2><srgbClr val=\"101010\"/></dk2><lt2><srgbClr val=\"F0F0F0\"/></lt2>"
                + "<accent1><srgbClr val=\"112233\"/></accent1>"
                + "<accent2><srgbClr val=\"445566\"/></accent2>"
                + "<accent3><srgbClr val=\"778899\"/></accent3>"
                + "<accent4><srgbClr val=\"AABBCC\"/></accent4>"
                + "<accent5><srgbClr val=\"DDEEFF\"/></accent5>"
                + "<accent6><srgbClr val=\"010203\"/></accent6>"
                + "<hlink><srgbClr val=\"0000FF\"/></hlink>"
                + "<folHlink><srgbClr val=\"800080\"/></folHlink>"
                + "</clrScheme></themeElements></theme>")
            : null;

        return XlsxCellDecoration.Read(styles, themeRoot, worksheet);
    }
}
