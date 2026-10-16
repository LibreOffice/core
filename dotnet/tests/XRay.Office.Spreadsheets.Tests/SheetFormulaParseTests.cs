using XRay.Office.Spreadsheets.Layout;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What the conditional-format expression grammar reads, and — as much as it matters — what it
/// refuses.
/// </summary>
/// <remarks>
/// <para>
/// These are about the parse and not about the value: a rule this cannot read must paint nothing,
/// which is what it did before the parser existed, so <strong>refusing correctly is half of what
/// is under test</strong>. A grammar that quietly accepted a construct it could not evaluate would
/// turn an absent answer into a confident wrong one.
/// </para>
/// <para>
/// The shapes here are the corpus's. <c>072_Gantt_project_planner</c> states its whole chart as
/// ten expression rules over defined names, and those names refer to other names — so a name is
/// expanded as <em>text</em> and parsed in the place it was used, which is what keeps the
/// <c>$</c> flags that decide how each half shifts.
/// </para>
/// </remarks>
public class SheetFormulaParseTests
{
    private const string Sheet = "Project Planner";

    private static SheetFormula.Node? Parse(string formula, params (string Name, string Body)[] names)
    {
        Dictionary<string, string> defined =
            names.ToDictionary(one => one.Name, one => one.Body, StringComparer.OrdinalIgnoreCase);

        return SheetFormula.Parse(formula, Sheet, name => defined.GetValueOrDefault(name));
    }

    [Fact]
    public void AComparisonOfTwoCallsIsATreeAndNotAString()
    {
        SheetFormula.Node? node = Parse("MOD(COLUMN(),2)=0");

        SheetFormula.BinaryNode root = node.ShouldBeOfType<SheetFormula.BinaryNode>();
        root.Operator.ShouldBe("=");
        root.Right.ShouldBeOfType<SheetFormula.NumberNode>().Value.ShouldBe(0);

        SheetFormula.CallNode mod = root.Left.ShouldBeOfType<SheetFormula.CallNode>();
        mod.Name.ShouldBe("MOD");
        mod.Arguments.Count.ShouldBe(2);
        mod.Arguments[0].ShouldBeOfType<SheetFormula.CallNode>().Name.ShouldBe("COLUMN");
        mod.Arguments[0].ShouldBeOfType<SheetFormula.CallNode>().Arguments.Count.ShouldBe(0);
    }

    [Theory]
    // Excel's own precedence, loosest first: comparison, concatenation, +/-, * and /, then ^.
    [InlineData("1+2*3", "+")]
    [InlineData("1*2+3", "+")]
    [InlineData("1&2=3", "=")]
    [InlineData("1+2&3", "&")]
    [InlineData("2^3*4", "*")]
    public void TheLoosestOperatorIsTheRoot(string formula, string expected)
        => Parse(formula).ShouldBeOfType<SheetFormula.BinaryNode>().Operator.ShouldBe(expected);

    [Fact]
    public void ExponentiationAssociatesToTheRightAndTheRestToTheLeft()
    {
        SheetFormula.BinaryNode power = Parse("2^3^4").ShouldBeOfType<SheetFormula.BinaryNode>();
        power.Right.ShouldBeOfType<SheetFormula.BinaryNode>().Operator.ShouldBe("^");

        SheetFormula.BinaryNode minus = Parse("1-2-3").ShouldBeOfType<SheetFormula.BinaryNode>();
        minus.Left.ShouldBeOfType<SheetFormula.BinaryNode>().Operator.ShouldBe("-");
    }

    [Theory]
    // The two `$` flags are the whole of what a rule over a range means, so they survive the parse
    // separately for the row and the column.
    [InlineData("A1", 0, 0, false, false)]
    [InlineData("$A1", 0, 0, false, true)]
    [InlineData("A$4", 3, 0, true, false)]
    [InlineData("$H$2", 1, 7, true, true)]
    [InlineData("BO30", 29, 66, false, false)]
    public void AReferenceKeepsBothDollarsApart(
        string text, int row, int column, bool absoluteRow, bool absoluteColumn)
    {
        SheetFormula.ReferenceNode node =
            Parse(text).ShouldBeOfType<SheetFormula.ReferenceNode>();

        node.Row.ShouldBe(row);
        node.Column.ShouldBe(column);
        node.AbsoluteRow.ShouldBe(absoluteRow);
        node.AbsoluteColumn.ShouldBe(absoluteColumn);
    }

    [Theory]
    // The rule's own sheet, quoted or not, is the only one this holds values for.
    [InlineData("'Project Planner'!A$4")]
    [InlineData("'project planner'!A$4")]
    public void AReferenceToTheRulesOwnSheetIsThatSheetsCell(string text)
        => Parse(text).ShouldBeOfType<SheetFormula.ReferenceNode>().Row.ShouldBe(3);

    [Fact]
    public void AReferenceToAnotherSheetIsRefusedRatherThanReadAsBlank()
        // This evaluator holds one sheet's values. Reading `Summary!A1` as an empty cell would
        // make a rule fire where the reference's does not, which is worse than not reading it.
        => Parse("Summary!A1=1").ShouldBeNull();

    [Fact]
    public void ADefinedNameIsExpandedWhereItIsUsedAndKeepsItsDollars()
    {
        SheetFormula.Node? node = Parse(
            "Plan",
            ("Plan", "PeriodInPlan*('Project Planner'!$C1>0)"),
            ("PeriodInPlan", "'Project Planner'!A$4=MEDIAN('Project Planner'!A$4,'Project Planner'!$C1,1)"));

        SheetFormula.BinaryNode root = node.ShouldBeOfType<SheetFormula.BinaryNode>();
        root.Operator.ShouldBe("*");

        // The left half is the nested name, expanded in place: a comparison, not an opaque value.
        SheetFormula.BinaryNode inner = root.Left.ShouldBeOfType<SheetFormula.BinaryNode>();
        inner.Operator.ShouldBe("=");
        inner.Left.ShouldBeOfType<SheetFormula.ReferenceNode>().AbsoluteRow.ShouldBeTrue();
        inner.Left.ShouldBeOfType<SheetFormula.ReferenceNode>().AbsoluteColumn.ShouldBeFalse();
    }

    [Fact]
    public void ANameThatExpandsToItselfIsRefusedRatherThanFollowedForever()
        => Parse("Loop", ("Loop", "Loop+1")).ShouldBeNull();

    [Theory]
    [InlineData("TRUE", 1)]
    [InlineData("false", 0)]
    public void ABooleanIsTheNumberItIsWorth(string text, double expected)
        => Parse(text).ShouldBeOfType<SheetFormula.NumberNode>().Value.ShouldBe(expected);

    [Theory]
    // A doubled quote inside a string is one quote, and a bare one ends it.
    [InlineData("\"a\"\"b\"", "a\"b")]
    [InlineData("\"\"", "")]
    public void AStringUndoesItsDoubledQuotes(string text, string expected)
        => Parse(text).ShouldBeOfType<SheetFormula.TextNode>().Value.ShouldBe(expected);

    [Theory]
    // Everything this does not read has to come back null so the rule paints nothing.
    [InlineData("A1:B2")]
    [InlineData("MOD(COLUMN(),2")]
    [InlineData("1+")]
    [InlineData("")]
    [InlineData("NotADefinedName")]
    [InlineData("SUM(A1:A9)>0")]
    public void AnythingOutsideTheGrammarIsRefused(string formula)
        => Parse(formula).ShouldBeNull();
}
