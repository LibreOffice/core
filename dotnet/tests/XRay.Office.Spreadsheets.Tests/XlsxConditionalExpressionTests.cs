using System.Globalization;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A Gantt chart drawn entirely out of conditional formats, which is what an
/// <c>expression</c> rule of more than two operands is actually for.
/// </summary>
/// <remarks>
/// <para>
/// The witness is <c>072_Gantt_project_planner_dde00e33.xlsx</c>. Its whole chart is eight
/// <c>cfRule type="expression"</c> over a single <c>sqref</c>, and every one of them names a
/// <strong>defined name</strong> rather than a formula — <c>PercentComplete</c>, <c>Actual</c>,
/// <c>Plan</c> and so on, each of which refers in turn to further names. The reader before round
/// 144 read one operator between two operands, so all eight were refused and the chart came out
/// blank: <b>7 filled paths against 26.2.4.2's 1453</b>.
/// </para>
/// <para>
/// This fixture is that workbook's shape reduced to the five band rules and its first four
/// activities, with the theme colours written out as literals. The expected colours are
/// 26.2.4.2's own, read out of its rendering of the real workbook at the same cells; the two
/// cells the real file paints <c>#F6DDB9</c> and the striping it paints under everything are the
/// three rules this fixture leaves out, so they are unpainted here.
/// </para>
/// <para>
/// <strong>What each column of the expectation is evidence for.</strong> The dark cell is
/// <c>PercentComplete</c>, which needs <c>MEDIAN</c>, <c>INT</c>, a nested name, and operator
/// precedence over five operands. The bands either side of it are <c>Actual</c> and <c>Plan</c>,
/// which are the same names one level up. And that the three are <em>different colours at all</em>
/// is the hatch mix of <see cref="XlsxPatternFillTests"/> — all three <c>dxf</c> state the same
/// foreground.
/// </para>
/// </remarks>
public sealed class XlsxConditionalExpressionTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Sheet = "Project Planner";

    /// <summary>Start, duration, actual start, actual duration and percent complete, per row.</summary>
    private static readonly double[][] Activities =
    [
        [1, 5, 1, 4, 0.25],
        [1, 6, 1, 6, 1],
        [2, 4, 2, 5, 0.35],
        [4, 8, 4, 6, 0.1],
    ];

    [Fact]
    public void EachActivityIsDrawnAsThePlannedActualAndCompletedBandsTheReferenceDraws()
    {
        SheetFormatting formatting = Read();

        // 26.2.4.2's own colours at these cells. A dash is a cell the three omitted rules paint
        // and these five do not.
        string[] expected =
        [
            "P A A A L - - - - - - -",
            "P P P P P P - - - - - -",
            "- P A A A B - - - - - -",
            "- - - P A A A A A L L -",
        ];

        StringBuilder drawn = new();
        for (int activity = 0; activity < Activities.Length; activity++)
        {
            if (activity > 0) drawn.Append('\n');
            for (int period = 1; period <= 12; period++)
            {
                if (period > 1) drawn.Append(' ');
                drawn.Append(Letter(formatting.At(4 + activity, 6 + period).Background));
            }
        }

        drawn.ToString().ShouldBe(string.Join('\n', expected));
    }

    /// <summary>One letter per band, so a failure names which rule won rather than a colour.</summary>
    private static string Letter(Colour? fill) => fill?.ToString() switch
    {
        null => "-",
        "#735773" => "P",       // PercentComplete, solid
        "#E9AB51" => "O",       // PercentCompleteBeyond, solid
        "#B5A1B5" => "A",       // Actual, a lightUp hatch
        "#D6BCA8" => "B",       // ActualBeyond, a lightUp hatch
        "#DCD5DC" => "L",       // Plan, a lightUp hatch
        { } other => other,
    };

    private static SheetFormatting Read()
    {
        StringBuilder rows = new();

        // Row 4 carries the period numbers the rules compare against, across H onwards.
        rows.Append("<row r=\"4\">");
        for (int period = 1; period <= 12; period++)
        {
            rows.Append(CultureInfo.InvariantCulture,
                $"<c r=\"{Column(6 + period)}4\"><v>{period}</v></c>");
        }

        rows.Append("</row>");

        for (int activity = 0; activity < Activities.Length; activity++)
        {
            rows.Append(CultureInfo.InvariantCulture, $"<row r=\"{5 + activity}\">");
            for (int field = 0; field < 5; field++)
            {
                rows.Append(CultureInfo.InvariantCulture,
                    $"<c r=\"{Column(2 + field)}{5 + activity}\">"
                    + $"<v>{Activities[activity][field].ToString(CultureInfo.InvariantCulture)}</v></c>");
            }

            rows.Append("</row>");
        }

        XElement worksheet = XElement.Parse(
            $"<worksheet xmlns=\"{Namespace}\"><sheetData>{rows}</sheetData>"
            + "<conditionalFormatting sqref=\"H5:BO30\">"
            + Rule(0, 1, "PercentComplete")
            + Rule(1, 3, "PercentCompleteBeyond")
            + Rule(2, 4, "Actual")
            + Rule(3, 5, "ActualBeyond")
            + Rule(4, 6, "Plan")
            + "</conditionalFormatting></worksheet>");

        XElement workbook = XElement.Parse(
            $"<workbook xmlns=\"{Namespace}\"><definedNames>"
            + Name("Actual", "(PeriodInActual*('Project Planner'!$E1&gt;0))*PeriodInPlan")
            + Name("ActualBeyond", "PeriodInActual*('Project Planner'!$E1&gt;0)")
            + Name("PercentComplete", "PercentCompleteBeyond*PeriodInPlan")
            + Name("PercentCompleteBeyond",
                   "('Project Planner'!A$4=MEDIAN('Project Planner'!A$4,'Project Planner'!$E1,"
                   + "'Project Planner'!$E1+'Project Planner'!$F1)*('Project Planner'!$E1&gt;0))"
                   + "*(('Project Planner'!A$4&lt;(INT('Project Planner'!$E1+'Project Planner'!$F1"
                   + "*'Project Planner'!$G1)))+('Project Planner'!A$4='Project Planner'!$E1))"
                   + "*('Project Planner'!$G1&gt;0)")
            + Name("PeriodInActual",
                   "'Project Planner'!A$4=MEDIAN('Project Planner'!A$4,'Project Planner'!$E1,"
                   + "'Project Planner'!$E1+'Project Planner'!$F1-1)")
            + Name("PeriodInPlan",
                   "'Project Planner'!A$4=MEDIAN('Project Planner'!A$4,'Project Planner'!$C1,"
                   + "'Project Planner'!$C1+'Project Planner'!$D1-1)")
            + Name("Plan", "PeriodInPlan*('Project Planner'!$C1&gt;0)")
            + "</definedNames></workbook>");

        XElement styles = XElement.Parse(
            $"<styleSheet xmlns=\"{Namespace}\">"
            + "<fills count=\"1\"><fill><patternFill patternType=\"none\"/></fill></fills>"
            + "<borders count=\"1\"><border/></borders>"
            + "<cellXfs count=\"1\"><xf fillId=\"0\" borderId=\"0\"/></cellXfs>"
            + "<dxfs count=\"5\">"
            + Solid("FF735773")
            + Solid("FFE9AB51")
            + Hatch("<bgColor rgb=\"FFCAB9CA\"/>")
            + Hatch("<bgColor rgb=\"FFF6DDB9\"/>")
            + Hatch("<bgColor auto=\"1\"/>")
            + "</dxfs></styleSheet>");

        return XlsxCellDecoration.Read(
            styles, null, worksheet, XlsxSharedStrings.Empty,
            out Dictionary<(int Row, int Column), SheetConditionalText> _, Sheet, workbook);
    }

    private static string Rule(int dxf, int priority, string formula)
        => $"<cfRule type=\"expression\" dxfId=\"{dxf}\" priority=\"{priority}\">"
           + $"<formula>{formula}</formula></cfRule>";

    private static string Name(string name, string body)
        => $"<definedName name=\"{name}\">{body}</definedName>";

    /// <summary>A <c>dxf</c> states its solid colour in <c>bgColor</c>, not in <c>fgColor</c>.</summary>
    private static string Solid(string rgb)
        => "<dxf><fill><patternFill patternType=\"solid\"><fgColor auto=\"1\"/>"
           + $"<bgColor rgb=\"{rgb}\"/></patternFill></fill></dxf>";

    /// <summary>The three bands, which state one foreground between them.</summary>
    private static string Hatch(string background)
        => "<dxf><fill><patternFill patternType=\"lightUp\"><fgColor rgb=\"FF735773\"/>"
           + $"{background}</patternFill></fill></dxf>";

    private static string Column(int index)
    {
        StringBuilder name = new();
        for (int at = index + 1; at > 0; at /= 26)
        {
            name.Insert(0, (char)('A' + ((at - 1) % 26)));
            at -= (at - 1) % 26;
        }

        return name.ToString();
    }
}
