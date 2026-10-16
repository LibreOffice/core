using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Several <c>w:num</c> over one <c>w:abstractNumId</c> are one list, and a <c>w:startOverride</c>
/// restarts that shared list once.
/// </summary>
/// <remarks>
/// <para>
/// <c>AbstractListDef::MapListId</c> (<c>dmapper/NumberingManager.cxx</c>:405-412) records the first
/// paragraph's Writer list id on the <em>abstract</em> definition and hands the same id back for
/// every later one, and <c>DomainMapper_Impl::finishParagraph</c>
/// (<c>DomainMapper_Impl.cxx</c>:2962-2976) writes it onto the paragraph — so a document that
/// changes <c>w:numId</c> half way through a list carries on counting. The same function applies a
/// level's <c>w:startOverride</c> as <c>ParaIsNumberingRestart</c> + <c>NumberingStartValue</c>
/// (<c>:2986-2997</c>) on the first paragraph of that instance and never again, because
/// <c>m_aListOverrideApplied</c> is keyed on the <c>w:numId</c> alone.
/// </para>
/// <para>
/// Every number below was measured against 26.2.4.2 before it was written, from its own rendering of
/// the committed fixture <c>tests/corpus/features/words-list-instance-share.docx</c> — seven arms,
/// 32 labels, all 32 reproduced. <c>probes/listshare-r159/</c>.
/// </para>
/// <para>
/// Its witness is <c>FAA 2025-26 Holdover Tables.docx</c>, whose table notes open with paragraphs
/// naming a restarting instance and continue with paragraphs that name none and take the
/// <c>ListNotes</c> style's. The reference numbers each table's notes <b>1 … 12</b>; this tree
/// numbered them 1 … 5 and then <b>516 … 522</b>, the document-wide count of everything that ever
/// used that one instance — and the wider label moved the text with it. Alphanumeric distance from
/// the reference <b>0.37 % → 0.01 %</b> over 167 pages.
/// </para>
/// </remarks>
public sealed class DocxListInstanceShareTests
{
    private const string Fixture = "words-list-instance-share.docx";

    /// <summary>
    /// Two instances over one abstract definition count on from one another.
    /// </summary>
    /// <remarks>Arm A, and arm G is the same rule reached through a paragraph style.</remarks>
    [Fact]
    public void TwoInstancesOverOneAbstractShareACounter() => Arm("A").ShouldBe(["1.", "2.", "3.", "4."]);

    /// <summary>
    /// The FAA shape: the first instance restarts and the second inherits the running count.
    /// </summary>
    /// <remarks>Arm B — the arm this round exists for.</remarks>
    [Fact]
    public void AnInstanceWithNoOverrideInheritsTheRunningCount()
        => Arm("B").ShouldBe(["1.", "2.", "3.", "4."]);

    /// <summary>
    /// An override on the second instance restarts the shared counter rather than being ignored.
    /// </summary>
    /// <remarks>
    /// Arm C. This is the half a "share the counter" fix gets wrong on its own: the counter already
    /// exists when the second instance is first used, so a rule that only consults the override
    /// where no counter exists never fires it.
    /// </remarks>
    [Fact]
    public void AnOverrideRestartsTheSharedCounter() => Arm("C").ShouldBe(["1.", "2.", "1.", "2."]);

    /// <summary>And it restarts at the value it states, which need not be one.</summary>
    /// <remarks>Arm D, so a restart cannot be confused with a level's own <c>w:start</c>.</remarks>
    [Fact]
    public void AnOverrideRestartsAtItsOwnValue() => Arm("D").ShouldBe(["1.", "2.", "5.", "6."]);

    /// <summary>
    /// An override fires once for its instance, however many times the instance is used.
    /// </summary>
    /// <remarks>
    /// Arm E, which is <c>m_aListOverrideApplied</c> exactly: the second run of the overriding
    /// instance continues the shared count instead of restarting it again.
    /// </remarks>
    [Fact]
    public void AnOverrideFiresOnceOnly()
        => Arm("E").ShouldBe(["1.", "2.", "1.", "2.", "3.", "4.", "5.", "6."]);

    /// <summary>
    /// Two instances over two <em>different</em> abstracts share nothing, which is the control.
    /// </summary>
    /// <remarks>
    /// Arm F. Without it the whole rule could be satisfied by one counter for the document, and
    /// every unrelated list in it would run on into the next.
    /// </remarks>
    [Fact]
    public void TwoAbstractsAreTwoLists() => Arm("F").ShouldBe(["1.", "2.", "1.", "2."]);

    /// <summary>
    /// A paragraph with no <c>w:numPr</c> takes its style's instance and shares that counter too.
    /// </summary>
    /// <remarks>
    /// Arm G, and the shape the witness is in: the style names one instance and the paragraphs
    /// before it name another over the same abstract.
    /// </remarks>
    [Fact]
    public void TheStyleRouteSharesTheSameCounter() => Arm("G").ShouldBe(["1.", "2.", "3.", "4."]);

    /// <summary>The labels of the paragraphs in one arm, in document order.</summary>
    private static List<string> Arm(string arm)
    {
        List<(string Text, string? Label)> paragraphs = Paragraphs();
        int at = paragraphs.FindIndex(p => p.Text == "ARM " + arm);
        at.ShouldBeGreaterThanOrEqualTo(0, $"the fixture states no arm '{arm}'");

        List<string> labels = [];
        for (int i = at + 1; i < paragraphs.Count && !paragraphs[i].Text.StartsWith("ARM ", StringComparison.Ordinal); i++)
        {
            labels.Add(paragraphs[i].Label ?? "");
        }

        return labels;
    }

    private static List<(string Text, string? Label)> Paragraphs()
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture));
        using IDocument document = new WordProcessingReader().Read(source);

        var pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Paragraphs.Select(p => (p.Text, p.Label?.Text))];
    }
}
