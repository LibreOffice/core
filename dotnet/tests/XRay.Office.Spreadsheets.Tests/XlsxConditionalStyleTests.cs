using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a <c>cfRule</c> naming a <c>dxf</c> paints, and which cells it reaches.
/// </summary>
/// <remarks>
/// <para>
/// The witness throughout is <c>TK-Syllabus-Comparison-Document-v2.xlsx</c>, the sheets track's
/// largest ink divergence at round 94 — 304.57 % summed unsigned ink over 1235 pages, and passing
/// the gate, because a colour and a strikethrough add no characters and no pages. It states
/// <strong>413</strong> conditional formats and every one of them is
/// <c>type="expression"</c> with a formula of the shape <c>&lt;reference&gt;="x"</c>. Its 413
/// <c>dxf</c> entries reduce to six distinct ones, and the six are taken here from 26.2.4.2's own
/// view of the file rather than from the specification: <c>soffice --convert-to fods</c> writes
/// each rule out as a <c>ConditionalStyle_N</c> cell style, and those styles say
/// <c>fo:color="#00b050"</c> with <c>style:text-line-through-style="none"</c> (287 of them),
/// <c>fo:color="#ff0000"</c> with a solid line-through (70), a
/// <c>fo:background-color="#afabab"</c> from theme <c>light2</c> at <c>lummod 7500</c> (30), a
/// bare green (21), a green-and-grey pair (3) and a bare red (2).
/// </para>
/// <para>
/// The two structural assertions are the ones a later round is most likely to break:
/// <see cref="OnlyOneRuleWinsACellAndTheirPropertiesAreNotMerged"/>, which is
/// <c>ScDocument::GetCondResult</c>'s first-match-wins, and
/// <see cref="AConditionalFormatIsInvisibleToThePrintAreaScan"/>, which keeps a rule declared over
/// a whole column from moving a page count.
/// </para>
/// </remarks>
public sealed class XlsxConditionalStyleTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>The commonest of the witness's six: green, and the strikethrough turned off.</summary>
    private const string Green = "<font><strike val=\"0\"/><color rgb=\"FF00B050\"/></font>";

    /// <summary>Its second: red with a line through.</summary>
    private const string RedStruck = "<font><strike/><color rgb=\"FFFF0000\"/></font>";

    /// <summary>Its third, whose colour is in <c>bgColor</c> and not in <c>fgColor</c>.</summary>
    private const string Grey =
        "<fill><patternFill><bgColor rgb=\"FFAFABAB\"/></patternFill></fill>";

    [Fact]
    public void ASheetWithNoRuleAtAllChangesNothing()
    {
        // The control, and it runs first for the reason the colour-scale suite's does: an
        // assertion that a colour appears is worth nothing until the same instrument has been
        // shown to answer "none" when there is none.
        (SheetFormatting formatting, SheetCellFormats formats) = Read(rules: null);

        formats.At(0, 18).Colour.ShouldBe(Colour.Black);
        formats.At(0, 18).IsStruckThrough.ShouldBeTrue();
        formatting.At(0, 18).Background.ShouldBeNull();
    }

    [Fact]
    public void TheWitnessRuleColoursOnlyTheRowsWhoseOwnMarkerCellSaysX()
    {
        // `H2="x"` stated on S1:S1048576 — the witness's own shape. The formula is written for
        // the top-left cell of the sqref and shifted for every other, so it tests a different H
        // on every row; the probe puts `x` in H2 and H4 and leaves H3 empty.
        (_, SheetCellFormats formats) = Read(Rule("S1:S1048576", "H2=\"x\"", 0));

        formats.At(0, 18).Colour.ToString().ShouldBe("#00B050");
        formats.At(2, 18).Colour.ToString().ShouldBe("#00B050");
        formats.At(1, 18).Colour.ShouldBe(Colour.Black);
    }

    [Fact]
    public void ARuleTurnsAStatedStrikethroughOff()
    {
        // 287 of the witness's 413 rules state `<strike val="0"/>`, and 26.2.4.2 writes them out
        // as `style:text-line-through-style="none"`. A reader treating a dxf toggle as two-valued
        // leaves the cell's own strikethrough on and draws the reference's plain text struck.
        (_, SheetCellFormats formats) = Read(Rule("S1:S1048576", "H2=\"x\"", 0));

        formats.At(0, 18).IsStruckThrough.ShouldBeFalse();
        formats.At(1, 18).IsStruckThrough.ShouldBeTrue();
    }

    [Fact]
    public void ADxfFillStatesItsColourInTheBackgroundAndNotTheForeground()
    {
        // The opposite of a cell's own `patternFill`, and the whole of `Fill::finalizeImport`'s
        // `if (mbDxf)` branch: a stated fill colour with no pattern or a solid one is moved into
        // the pattern colour and forced solid. 2734 of the corpus's dxf fills state only a
        // bgColor, so reading fgColor here finds nothing on any of them.
        (SheetFormatting formatting, _) = Read(Rule("S1:S1048576", "H2=\"x\"", 2));

        formatting.At(0, 18).Background.ShouldNotBeNull().ToString().ShouldBe("#AFABAB");
        formatting.At(1, 18).Background.ShouldBeNull();
    }

    [Fact]
    public void OnlyOneRuleWinsACellAndTheirPropertiesAreNotMerged()
    {
        // `ScDocument::GetCondResult` returns the first non-empty style's item set and
        // `ScConditionalFormat::GetCellStyle` the first matching entry's style, so a cell takes
        // one dxf or none. Two rules match row 1 here — the green font at priority 1 and the grey
        // fill at priority 2 — and the cell must take the green and no fill at all.
        (SheetFormatting formatting, SheetCellFormats formats) = Read(
            Rule("S1:S1048576", "H2=\"x\"", 0, priority: 1)
            + Rule("S1:S1048576", "H2=\"x\"", 2, priority: 2));

        formats.At(0, 18).Colour.ToString().ShouldBe("#00B050");
        formatting.At(0, 18).Background.ShouldBeNull();
    }

    [Fact]
    public void AConditionalFormatIsInvisibleToThePrintAreaScan()
    {
        // The regression guard, and the reason the fill goes through the conditional layer rather
        // than through an interned format. `SheetDecorationArea` decides how far a sheet prints
        // from these three, and every one of the witness's 413 rules is declared to row 1048576.
        (SheetFormatting formatting, _) = Read(Rule("S1:S1048576", "H2=\"x\"", 2));

        formatting.At(0, 18).Background.ShouldNotBeNull();
        formatting.Cells.ShouldBeEmpty();
        formatting.Rows.ShouldBeEmpty();
        formatting.ColumnRuns.ShouldBeEmpty();
    }

    [Fact]
    public void AnAbsoluteReferenceIsNotShiftedAndARelativeOneIs()
    {
        // Which halves carry the `$` is the whole of what a rule over a column means: `H2="x"`
        // tests a different H on every row and `$H$2="x"` tests one cell for all of them.
        (_, SheetCellFormats absolute) = Read(Rule("S1:S1048576", "$H$2=\"x\"", 0));

        absolute.At(0, 18).Colour.ToString().ShouldBe("#00B050");
        absolute.At(1, 18).Colour.ToString().ShouldBe("#00B050");
        absolute.At(2, 18).Colour.ToString().ShouldBe("#00B050");
    }

    [Fact]
    public void ACellIsRuleComparesTheCellsOwnValue()
    {
        // The corpus's second-commonest dxf-naming shape, 123 rules in 18 documents. Column S
        // holds 1, 2 and 3 in the probe's three rows.
        (_, SheetCellFormats formats) = Read(CellIs("S1:S3", "greaterThan", "1", 1));

        formats.At(0, 18).Colour.ShouldBe(Colour.Black);
        formats.At(1, 18).Colour.ToString().ShouldBe("#FF0000");
        formats.At(2, 18).Colour.ToString().ShouldBe("#FF0000");
    }

    [Fact]
    public void AFormulaOfMoreThanTwoOperandsIsEvaluated()
    {
        // This asserted the opposite until round 144, and the premise was the reader's rather
        // than the reference's: `Comparison` reads one operator between two operands, so every
        // longer formula was refused and painted nothing. `SheetFormula` evaluates them, and the
        // two-operand reader still runs first — so what reaches the evaluator is exactly the set
        // that used to paint nothing at all.
        (_, SheetCellFormats formats) = Read(Rule("S1:S1048576", "AND(H2=\"x\",H2&lt;&gt;\"y\")", 0));

        formats.At(0, 18).Colour.ToString().ShouldBe("#00B050");

        // Row 2's H3 is empty, so the first half is false and `AND` is zero. A rule that fired
        // everywhere would pass the assertion above without evaluating anything.
        formats.At(1, 18).Colour.ShouldBe(Colour.Black);
    }

    [Fact]
    public void AFormulaThisCannotEvaluatePaintsNothing()
    {
        // The floor the evaluator keeps: a function outside its vocabulary parses and answers
        // nothing rather than guessing, so the rule paints nothing rather than painting a cell
        // the reference leaves alone. `ISERROR` is the one the corpus states that this declines,
        // and it declines it because `ValueOf` stores an error cell as blank — so a reader that
        // answered it would answer it wrongly.
        (_, SheetCellFormats formats) = Read(Rule("S1:S1048576", "ISERROR(H2)", 0));

        formats.At(0, 18).Colour.ShouldBe(Colour.Black);
    }

    [Fact]
    public void AnOperatorInsideAQuotedStringIsNotTheComparison()
    {
        // `A1="<"` is legal and a naive IndexOf finds the wrong character in it. The probe's H5
        // holds a literal `<`, so a reader that split on the quoted one would compare `H5="` with
        // `"` and match nothing.
        (_, SheetCellFormats formats) = Read(Rule("S5:S5", "H5=\"&lt;\"", 0));

        formats.At(4, 18).Colour.ToString().ShouldBe("#00B050");
    }

    private static string Rule(string sqref, string formula, int dxf, int priority = 1)
        => $"<conditionalFormatting sqref=\"{sqref}\">"
           + $"<cfRule type=\"expression\" dxfId=\"{dxf}\" priority=\"{priority}\">"
           + $"<formula>{formula}</formula></cfRule></conditionalFormatting>";

    private static string CellIs(string sqref, string op, string formula, int dxf)
        => $"<conditionalFormatting sqref=\"{sqref}\">"
           + $"<cfRule type=\"cellIs\" operator=\"{op}\" dxfId=\"{dxf}\" priority=\"1\">"
           + $"<formula>{formula}</formula></cfRule></conditionalFormatting>";

    /// <summary>
    /// One sheet: column H holds the markers a rule tests and column S the cells it formats.
    /// </summary>
    /// <remarks>
    /// Every cell of column S states the workbook's one non-default <c>xf</c>, whose font is
    /// struck through — so a rule turning the strikethrough off has something to turn off, and a
    /// row no rule reaches is the control for it.
    /// </remarks>
    private static (SheetFormatting Formatting, SheetCellFormats Formats) Read(string? rules)
    {
        string[] markers = ["", "x", "", "x", "<"];
        string rows = string.Empty;
        for (int i = 0; i < markers.Length; i++)
        {
            string marker = markers[i] is { Length: > 0 } text
                ? $"<c r=\"H{i + 1}\" t=\"str\"><v>{System.Security.SecurityElement.Escape(text)}</v></c>"
                : string.Empty;
            rows += $"<row r=\"{i + 1}\">{marker}"
                    + $"<c r=\"S{i + 1}\" s=\"1\"><v>{i + 1}</v></c></row>";
        }

        XElement worksheet = XElement.Parse(
            $"<worksheet xmlns=\"{Namespace}\"><sheetData>{rows}</sheetData>{rules}</worksheet>");

        XElement styles = XElement.Parse(
            $"<styleSheet xmlns=\"{Namespace}\">"
            + "<fills count=\"1\"><fill><patternFill patternType=\"none\"/></fill></fills>"
            + "<borders count=\"1\"><border/></borders>"
            + "<cellXfs count=\"1\"><xf fillId=\"0\" borderId=\"0\"/></cellXfs>"
            + $"<dxfs count=\"3\"><dxf>{Green}</dxf><dxf>{RedStruck}</dxf><dxf>{Grey}</dxf></dxfs>"
            + "</styleSheet>");

        SheetFormatting formatting = XlsxCellDecoration.Read(
            styles, null, worksheet, XlsxSharedStrings.Empty,
            out Dictionary<(int Row, int Column), SheetConditionalText> conditional);

        // The text formats are built here rather than read, because what is under test is the
        // overlay and the rule that fills it: every cell of column S is struck through, so a rule
        // turning the strikethrough off has something to turn off and a row no rule reaches is
        // the control for it.
        SheetCellFormats.Builder builder = new();
        int plain = builder.Intern(SheetCellFormat.Default);
        int struck = builder.Intern(SheetCellFormat.Default with { IsStruckThrough = true });
        builder.SetSheetDefault(plain);
        for (int row = 0; row < markers.Length; row++) builder.SetCell(row, 18, struck);

        SheetCellFormats formats = builder.Build().WithConditionalText(conditional);

        return (formatting, formats);
    }
}
