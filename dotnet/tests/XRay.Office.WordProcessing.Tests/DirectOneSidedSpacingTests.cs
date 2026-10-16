using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A paragraph that sets one of its two vertical margins directly does not see the pool completion
/// its style carries — writerfilter's <c>tdf#118521</c>.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="OneSidedStyleSpacingBuiltInChildTests"/> settled what a one-sided <c>w:spacing</c> on a
/// <em>style</em> resolves to: when the parent is declared after the child and Writer knows the
/// child's own name, the unstated half comes from Writer's pool row for that name. That completion
/// lives on the Writer style. A paragraph carrying its own <c>w:spacing</c> takes a different path
/// entirely and never reaches it.
/// </para>
/// <para>
/// <c>DomainMapper_Impl.cxx</c>:3110-3138 — <em>"set paragraph top or bottom margin based on the
/// paragraph style if we already set the other margin with direct formatting"</em> — fires when the
/// paragraph's own <c>w:pPr</c> sets an unequal subset of {top margin, bottom margin, contextual
/// spacing}, and fills each unset margin <b>as direct formatting</b> from
/// <c>GetPropertyFromParaStyleSheet</c>. That walks the DOCX <c>w:basedOn</c> chain and then
/// <c>w:docDefaults</c> (<c>DomainMapper_Impl.cxx</c>:1556-1628); it never consults Writer's pool,
/// because the pool is a property of the Writer style rather than of the DOCX style sheet.
/// </para>
/// <para>
/// Every expectation is a measurement of the installed 26.2.4.2, from
/// <c>dotnet/probes/words-p1-01/direct-one-sided-spacing.py</c>, which authors each variant and reads
/// <c>fo:margin-top</c> / <c>fo:margin-bottom</c> straight out of <c>--convert-to fodt</c> — the
/// importer's own answer, before any layout, font or rounding beyond the hundredth of a millimetre
/// the format stores. The same script writes
/// <c>tests/corpus/features/direct-one-sided-spacing.docx</c>, so the fixture and these numbers come
/// from one place.
/// </para>
/// <para>
/// <b>That probe runs both declaration orders and the first version of it ran only one, which made it
/// unable to answer its own question.</b> With the parent declared first the completion never fires,
/// so both readings are the parent's 60 twips and every variant agrees whatever the rule is. The
/// fixture is child-first, which is the arrangement <c>FAA 2025-26 Holdover Tables.docx</c> has —
/// <c>Heading4</c> is its style 4 and <c>Notes/Cautions Heading</c> its style 186.
/// </para>
/// <para>
/// The document is why this matters: 31 of that file's 113 <c>NOTES</c> headings carry a direct
/// <c>&lt;w:spacing w:before="80"/&gt;</c> and the other 76 carry nothing, and LibreOffice puts the
/// first note <b>3.00 pt</b> — the inherited 60 twips exactly — closer on those 31. Each of those
/// pages is one line short of full, so the three points spilled a page apiece: it rendered at 185
/// pages against a reference of 167, and at 165 with this.
/// </para>
/// </remarks>
public sealed class DirectOneSidedSpacingTests
{
    /// <summary>Writer's pool row for <c>Heading 4</c>, which the style resolves to.</summary>
    private static readonly Length Pool = Length.FromTwips(120);

    /// <summary>What <c>Notes/Cautions Heading</c> states, which the DOCX chain resolves to.</summary>
    private static readonly Length Inherited = Length.FromTwips(60);

    /// <summary>
    /// The control: a paragraph stating no spacing of its own keeps the style's pool completion.
    /// </summary>
    /// <remarks>
    /// This is <see cref="OneSidedStyleSpacingBuiltInChildTests"/>'s rule seen from here, and it must
    /// not move. If it does, the change below has been applied to the style rather than to the
    /// paragraph and the two rules have been collapsed into one.
    /// </remarks>
    [Fact]
    public void AParagraphStatingNothingKeepsTheStylesPoolCompletion()
    {
        ParagraphFormat format = Resolve(null);

        format.SpaceBefore.ShouldBe(Length.FromTwips(120));
        format.SpaceAfter.ShouldBe(Pool);
    }

    /// <summary>
    /// A direct <c>w:before</c> alone re-resolves the <em>bottom</em> margin through the DOCX chain.
    /// </summary>
    [Fact]
    public void ADirectSpaceBeforeTakesTheBottomMarginFromTheDocxChain()
    {
        ParagraphFormat format = Resolve("""<w:spacing w:before="80"/>""");

        format.SpaceBefore.ShouldBe(Length.FromTwips(80));
        format.SpaceAfter.ShouldBe(Inherited);
    }

    /// <summary>
    /// It is the attribute being present that fires it, not its value being non-zero.
    /// </summary>
    /// <remarks>
    /// The C++ tests <c>pParaContext->isSet(PROP_PARA_TOP_MARGIN)</c>, and a stated zero sets it. A
    /// reading that looked at the value instead would agree with the row above and disagree here.
    /// </remarks>
    [Fact]
    public void AStatedZeroFiresItToo()
    {
        ParagraphFormat format = Resolve("""<w:spacing w:before="0"/>""");

        format.SpaceBefore.ShouldBe(Length.Zero);
        format.SpaceAfter.ShouldBe(Inherited);
    }

    /// <summary>
    /// A <c>w:spacing</c> that states only <c>w:line</c> sets neither margin, so nothing fires.
    /// </summary>
    /// <remarks>
    /// The element is not the setting. This is the row that separates "the paragraph has a
    /// <c>w:spacing</c>" from "the paragraph sets a margin", and only the second is the trigger.
    /// </remarks>
    [Fact]
    public void ALineRuleAloneIsNotASetting()
        => Resolve("""<w:spacing w:line="240" w:lineRule="auto"/>""").SpaceAfter.ShouldBe(Pool);

    /// <summary>
    /// <c>w:contextualSpacing</c> alone fires it, with no <c>w:spacing</c> in the paragraph at all.
    /// </summary>
    /// <remarks>
    /// The condition is three-way — <c>bTopSet != bBottomSet || bBottomSet != bContextSet</c> — so a
    /// paragraph that sets only the third of them has an unequal subset and loses the completion on
    /// both margins. Measured: this variant reads 120 above and 60 below where the control reads 120
    /// and 120. Implementing only the two-way condition passes every other case in this file.
    /// </remarks>
    [Fact]
    public void ContextualSpacingAloneFiresItWithNoSpacingElementAtAll()
    {
        ParagraphFormat format = Resolve("<w:contextualSpacing/>");

        format.SpaceBefore.ShouldBe(Length.FromTwips(120));
        format.SpaceAfter.ShouldBe(Inherited);
    }

    /// <summary>
    /// A paragraph stating both margins has nothing to fill, and the completion is irrelevant.
    /// </summary>
    [Fact]
    public void StatingBothLeavesNothingToFill()
    {
        ParagraphFormat format = Resolve("""<w:spacing w:before="80" w:after="40"/>""");

        format.SpaceBefore.ShouldBe(Length.FromTwips(80));
        format.SpaceAfter.ShouldBe(Length.FromTwips(40));
    }

    /// <summary>
    /// On a style with no built-in name there is no completion to lose, so every variant agrees.
    /// </summary>
    /// <remarks>
    /// <c>Plain Base</c> states both margins itself and Writer has no pool row for its name, so the
    /// DOCX chain and the Writer style are the same 60 twips. Kept because it is the shape the
    /// probe's parent-first half had by accident — a sample on which the rule cannot be wrong — and
    /// naming it here stops it being mistaken for evidence.
    /// </remarks>
    [Theory]
    [InlineData(null)]
    [InlineData("""<w:spacing w:before="80"/>""")]
    [InlineData("<w:contextualSpacing/>")]
    public void ACustomStyleCannotShowTheDifference(string? direct)
        => Resolve(direct, "Plain").SpaceAfter.ShouldBe(Inherited);

    private const string Ns = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static ParagraphFormat Resolve(string? direct, string styleId = "Heading4")
    {
        XElement properties = XElement.Parse(
            $"""<w:pPr xmlns:w="{Ns}"><w:pStyle w:val="{styleId}"/>{direct}</w:pPr>""");

        return WordParagraphFormats.Resolve(LoadStyles(), properties, Length.FromTwips(720));
    }

    private static WordStyles LoadStyles()
    {
        using ZipArchive archive = ZipFile.OpenRead(
            Corpus.Require("direct-one-sided-spacing.docx"));
        using Stream part = archive.GetEntry("word/styles.xml")!.Open();

        WordStyles styles = new();
        styles.Add(XDocument.Load(part).Root!);
        return styles;
    }
}
