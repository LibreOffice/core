using System.Xml.Linq;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A level shown inside a deeper item's number has been used, and the next item at that level counts
/// on from it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured on LibreOffice 26.2.4.2</strong> — <c>probes/skipped-level-counter/</c>, three
/// documents over one four-level <c>multilevel</c> list with no <c>w:start</c> anywhere, differing
/// only in which levels their paragraphs sit at. Levels 0, 2, 1, 1 number <c>0</c>, <c>0.0.0</c>,
/// <c>0.1</c>, <c>0.2</c>. We gave the third item <c>0.0</c>.
/// </para>
/// <para>
/// <c>FormatLabel</c> rendered the skipped level's component from its start value and threw the value
/// away, so the level kept no counter and its first real item took the start a second time. Both
/// engines draw the deeper item identically — the disagreement is entirely about what that drawing
/// does to the counter.
/// </para>
/// <para>
/// The corpus document it decides is
/// <c>OM template for non-complex NCC operators_August 2016.docx</c>, whose <c>0.2</c> section opens
/// with a <c>Heading4</c> before any <c>Heading3</c>: the reference numbers what follows
/// <c>0.2.2</c>, <c>0.2.3</c>, <c>0.2.4</c> and its own Word-written table of contents agrees, while
/// we numbered them one lower throughout. Its numbered headings went from 590 to 601 of 613 agreeing.
/// </para>
/// <para>
/// The numbers begin at nought here rather than one because a level with no <c>w:start</c> starts at
/// zero — separately measured, see <see cref="NumberingStartDefaultTests"/>.
/// </para>
/// </remarks>
public sealed class SkippedLevelCounterTests
{
    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>The control: a run that skips nothing is untouched by any of this.</summary>
    [Fact]
    public void ARunThatSkipsNoLevelIsUnchanged()
    {
        WordNumbering numbering = Numbering();

        numbering.Advance("1", 0).ShouldBe("0");
        numbering.Advance("1", 1).ShouldBe("0.0");
        numbering.Advance("1", 1).ShouldBe("0.1");
        numbering.Advance("1", 2).ShouldBe("0.1.0");
    }

    /// <summary>Skipping one level: the level the deeper item drew is then counted on from.</summary>
    [Fact]
    public void ASkippedLevelIsCountedOnFrom()
    {
        WordNumbering numbering = Numbering();

        numbering.Advance("1", 0).ShouldBe("0");
        numbering.Advance("1", 2).ShouldBe("0.0.0");
        numbering.Advance("1", 1).ShouldBe("0.1", "the level the deeper item drew has been used");
        numbering.Advance("1", 1).ShouldBe("0.2");
    }

    /// <summary>And two skipped levels behave the same way as one.</summary>
    [Fact]
    public void TwoSkippedLevelsBehaveTheSameWay()
    {
        WordNumbering numbering = Numbering();

        numbering.Advance("1", 0).ShouldBe("0");
        numbering.Advance("1", 3).ShouldBe("0.0.0.0");
        numbering.Advance("1", 1).ShouldBe("0.1");
        numbering.Advance("1", 2).ShouldBe("0.1.0");
    }

    /// <summary>
    /// A level the deeper item does not <em>show</em> is not seeded, because nothing rendered it.
    /// </summary>
    /// <remarks>
    /// The narrow reading of what was measured. The corpus holds no document that separates "shown"
    /// from "passed over", so seeding every shallower level regardless would be a guess dressed as a
    /// rule; this states which of the two the code implements.
    /// </remarks>
    [Fact]
    public void ALevelTheDeeperItemDoesNotShowIsNotSeeded()
    {
        WordNumbering numbering = Numbering(deepestShowsOnlyItself: true);

        numbering.Advance("1", 0).ShouldBe("0");
        numbering.Advance("1", 2).ShouldBe("0");
        numbering.Advance("1", 1).ShouldBe("0.0", "level one was never drawn, so it never started");
    }

    /// <summary>Builds a four-level decimal list, each level showing its ancestors.</summary>
    private static WordNumbering Numbering(bool deepestShowsOnlyItself = false)
    {
        XElement abstractNum = new(
            W + "abstractNum",
            new XAttribute(W + "abstractNumId", 0),
            new XElement(W + "multiLevelType", new XAttribute(W + "val", "multilevel")));

        for (int level = 0; level < 4; level++)
        {
            string text = deepestShowsOnlyItself && level == 2
                ? "%3"
                : string.Join('.', Enumerable.Range(1, level + 1).Select(at => $"%{at}"));

            abstractNum.Add(new XElement(
                W + "lvl",
                new XAttribute(W + "ilvl", level),
                new XElement(W + "numFmt", new XAttribute(W + "val", "decimal")),
                new XElement(W + "lvlText", new XAttribute(W + "val", text))));
        }

        XElement root = new(
            W + "numbering",
            abstractNum,
            new XElement(
                W + "num",
                new XAttribute(W + "numId", 1),
                new XElement(W + "abstractNumId", new XAttribute(W + "val", 0))));

        WordNumbering numbering = new();
        numbering.Add(root);
        return numbering;
    }
}
