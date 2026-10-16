using System.Xml.Linq;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A numbering level with no <c>w:start</c> begins at zero, not at one.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured rather than read off the schema.</strong> Four one-level decimal lists differing
/// only in this element were rendered by LibreOffice 26.2.4.2: omitting <c>w:start</c> numbers them
/// <c>0. 1. 2.</c>, and <c>w:val</c> of 0, 1 and 3 number them <c>0. 1. 2.</c>, <c>1. 2. 3.</c> and
/// <c>3. 4. 5.</c>. We agreed on all three explicit values and disagreed only on the omission, which is
/// what makes this a default and not a parsing bug — see <c>probes/numbering-start-default/</c>.
/// </para>
/// <para>
/// The corpus document it decides is
/// <c>ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx</c>, whose heading list — <c>numId</c> 21,
/// <c>abstractNum</c> 9, level 0 — carries no <c>w:start</c>. LibreOffice numbers its sections from zero
/// ("0. Introduction", "1. References", "2. List of Abbreviations") and the document's own stored table
/// of contents, written by Word, agrees. We numbered every section one higher.
/// </para>
/// </remarks>
public sealed class NumberingStartDefaultTests
{
    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>An explicit value is taken as written, including zero.</summary>
    [Theory]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(3)]
    [InlineData(17)]
    public void AnExplicitStartIsTakenAsWritten(int start)
    {
        Level(new XElement(W + "start", new XAttribute(W + "val", start))).Start.ShouldBe(start);
    }

    /// <summary>And an absent one is zero — the case the corpus turns on.</summary>
    [Fact]
    public void AnAbsentStartIsZero()
    {
        Level(null).Start.ShouldBe(0);
    }

    /// <summary>
    /// A <c>w:start</c> whose value is not a number is treated as absent rather than as one.
    /// </summary>
    /// <remarks>
    /// The leniency rule applied to this element: a malformed attribute falls back to the same default
    /// the missing element gets, so the two cannot disagree.
    /// </remarks>
    [Fact]
    public void AMalformedStartFallsBackToTheSameDefault()
    {
        Level(new XElement(W + "start", new XAttribute(W + "val", "one"))).Start.ShouldBe(0);
    }

    private static WordNumberingLevel Level(XElement? start)
    {
        XElement level = new(
            W + "lvl",
            new XAttribute(W + "ilvl", 0),
            new XElement(W + "numFmt", new XAttribute(W + "val", "decimal")),
            new XElement(W + "lvlText", new XAttribute(W + "val", "%1.")));

        if (start is not null) level.Add(start);

        return new WordNumberingLevel(level);
    }
}
