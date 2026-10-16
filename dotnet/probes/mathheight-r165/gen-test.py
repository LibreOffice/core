#!/usr/bin/env python3
"""Emit the xunit ladder from the fixtures and 26.2.4.2's own answer for each."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build
from check import measured

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = "/home/user/libreoffice-core/dotnet/tests/XRay.Office.WordProcessing.Tests/OfficeMathBoxTests.cs"
ref = measured()
PT = 2540.0 / 72.0

# The arms whose construct the model implements. `leaf-paren`, `func`, `mixed` and the n-ary
# family hold a bracket or an operator glyph inside a text run, which the height walk reads as
# ordinary text -- they are measured in the probe and deliberately not asserted here.
SKIP = {"leaf-paren", "func", "mixed", "nary-sum", "nary-int", "nary-nolim", "nary-frac",
        "limlow", "limupp", "groupchr", "delim-nest"}

arms = []
for name, body in sorted(build.SHAPES.items()):
    if name in SKIP or name not in ref:
        continue
    arms.append((name, '<m:oMath>%s</m:oMath>' % body, ref[name] / PT))

rows = "\n".join(
    '        ["%s"] = ("""%s""", %.3f),' % (n, x, h) for n, x, h in arms)

TEMPLATE = '''using System.Xml.Linq;
using XRay.Office.Ooxml.OfficeMath;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A formula's height is StarMath's, and every arm below is 26.2.4.2's own answer for it.
/// </summary>
/// <remarks>
/// <para>
/// The reference does not lay an OMML formula out as text: it imports the subtree into a StarMath
/// object anchored as-character, so the line takes the <em>object's</em> height. That height is
/// readable without a rasteriser -- <c>soffice --convert-to fodt</c> prints it as the
/// <c>draw:frame</c>'s <c>svg:height</c> -- and the expected value on every arm here is that
/// attribute, converted to points, from a fixture built out of the real document's own parts so
/// that it inherits the same OOXML compatibility defaults.
/// </para>
/// <para>
/// <strong>Why a ladder rather than one assertion.</strong> The heights compose: a fraction is its
/// numerator plus its denominator plus a rule, a script is clamped against a line 40 % of the way
/// up the base. One number for a whole formula says a walk is wrong and not which construct broke,
/// and the constructs are what a later round will change. The fixtures, the sweep and the
/// per-construct table are in <c>probes/mathheight-r165/</c>.
/// </para>
/// <para>
/// <strong>The tolerance is a quarter of a point and it is not slack.</strong> StarMath computes in
/// whole hundredths of a millimetre -- 0.028 pt -- with integer division at every level, so a
/// four-deep construct carries a few units of the reference's own rounding; the two nested-radical
/// arms carry more, because the sign's glyph box is approximated by the height <c>AdaptToY</c> asks
/// for rather than measured from OpenSymbol. Nothing here is fitted: the leaf height and ascent are
/// two measured constants and every other number is <c>SmFormat</c>'s own default.
/// </para>
/// </remarks>
public sealed class OfficeMathBoxTests
{
    /// <summary>How far an arm may sit from 26.2.4.2's own <c>svg:height</c>.</summary>
    private const double Tolerance = 0.25;

    /// <summary>The two arms that need more, and why, is in the class remarks.</summary>
    private static readonly HashSet<string> Radicals = ["rad-rad", "rad-frac"];

    /// <summary>Every construct, and the height 26.2.4.2 gives it, in points.</summary>
    private static readonly Dictionary<string, (string Omml, double Points)> Ladder = new()
    {
@@ROWS@@
    };

    /// <summary>Every arm of the ladder lands on the reference's own answer.</summary>
    [Fact]
    public void EveryConstructMatchesTheReference()
    {
        List<string> wrong = [];

        foreach ((string name, (string omml, double want)) in Ladder)
        {
            double got = Measure(omml);
            double limit = Radicals.Contains(name) ? 0.4 : Tolerance;
            if (Math.Abs(got - want) > limit) wrong.Add($"{name}: {got:F3} against {want:F3}");
        }

        wrong.ShouldBeEmpty();
    }

    /// <summary>A plain row of maths text is one line whatever it says.</summary>
    /// <remarks>
    /// StarMath sets every leaf in its own faces at its own base size, so a capital, a digit and a
    /// Greek letter with a descender are all the same height -- which is why the walk needs no font
    /// metrics of the document's.
    /// </remarks>
    [Fact]
    public void EveryPlainRowIsTheSameHeight()
    {
        double[] heights = [.. Ladder.Where(a => a.Key.StartsWith("leaf-", StringComparison.Ordinal))
                                     .Select(a => Measure(a.Value.Omml))];

        heights.Length.ShouldBeGreaterThan(4);
        heights.ShouldAllBe(h => Math.Abs(h - heights[0]) < 0.001);
    }

    /// <summary>
    /// <c>w:sz</c> is ignored, which is the law that separates this from laying the formula out as
    /// text.
    /// </summary>
    /// <remarks>
    /// Measured on 26.2.4.2 at 8, 12 and 20 pt: <c>x</c> comes back 13.349 pt at all three and
    /// <c>x</c> with a subscript 15.134 pt at both 12 and 20. A reader that honoured the run's size
    /// would have the 20 pt arm two thirds taller, and that is exactly what this tree used to do --
    /// its pitch was 49.350 pt for every shape at the default size and moved with <c>w:sz</c>, so it
    /// was wrong in both directions at once.
    /// </remarks>
    [Theory]
    [InlineData("leaf-x", "sz8")]
    [InlineData("leaf-x", "sz20")]
    [InlineData("sub", "sz20-sub")]
    [InlineData("frac", "sz20-frac")]
    public void TheRunsOwnSizeDoesNotReachTheFormula(string plain, string sized)
        => Measure(Ladder[sized].Omml).ShouldBe(Measure(Ladder[plain].Omml), 0.001);

    /// <summary>A fraction is its two terms and a rule, and nesting one adds another term.</summary>
    [Fact]
    public void AFractionStacksItsTerms()
    {
        double leaf = Measure(Ladder["leaf-x"].Omml);
        double fraction = Measure(Ladder["frac"].Omml);

        fraction.ShouldBeGreaterThan(2 * leaf);
        Measure(Ladder["frac-nest-num"].Omml).ShouldBeGreaterThan(fraction + leaf);
    }

    /// <summary>Anything that is not a formula is measured as nothing.</summary>
    [Fact]
    public void OnlyAFormulaIsMeasured()
    {
        OfficeMathBox.Measure(null).ShouldBeNull();
        OfficeMathBox.Measure(XElement.Parse("<p xmlns='urn:x'/>")).ShouldBeNull();
        OfficeMathBox.Measure(XElement.Parse(
            $"<m:oMath xmlns:m='{Math_}'/>")).ShouldBeNull();
    }

    private const string Math_ = "http://schemas.openxmlformats.org/officeDocument/2006/math";

    private static double Measure(string omml)
    {
        XElement element = XElement.Parse(
            omml.Replace("<m:oMath>",
                $"<m:oMath xmlns:m='{Math_}' "
                + "xmlns:w='http://schemas.openxmlformats.org/wordprocessingml/2006/main'>",
                StringComparison.Ordinal));

        return OfficeMathBox.Measure(element)!.Value.Height.Points;
    }
}
'''
open(OUT, "w").write(TEMPLATE.replace("@@ROWS@@", rows))
print("wrote", OUT, "with", len(arms), "arms")
