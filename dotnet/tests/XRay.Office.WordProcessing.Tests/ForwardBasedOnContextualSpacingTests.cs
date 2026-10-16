using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A paragraph style inherits <c>w:contextualSpacing</c> from its <c>w:basedOn</c> parent only when
/// the parent is declared <em>before</em> it — and only matters when the style sets a margin of its
/// own.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="OneSidedStyleSpacingTests"/>' mechanism, seen from its other side. LibreOffice keeps
/// both vertical margins <em>and</em> the contextual flag in one item, <c>SvxULSpaceItem</c>, whose
/// <c>bContext</c> is what <c>SwFlowFrame::CalcUpperSpace</c> reads as <c>GetULSpace().GetContext()</c>
/// (<c>sw/source/core/layout/flowfrm.cxx</c>:1533 and 1620-1627), while writerfilter sets the three
/// through separate UNO properties. Setting a margin on a style is therefore a read-modify-write of
/// the whole item as it resolves at that moment, and the result shadows the parent's — so a parent
/// declared later, which has contributed nothing yet, never gets its flag through.
/// </para>
/// <para>
/// Measured on 26.2.4.2 with one authored document rendered twice, differing only in where the parent
/// sits in <c>word/styles.xml</c>; three paragraphs per style at 10 pt, so a suppressed gap is a
/// baseline pitch of 11.50 pt and an honoured <c>w:after="240"</c> is 23.50
/// (<c>probes/words-firstpage-r70/</c>, <c>ctx-lpfirst.docx</c> and <c>ctx-lplast.docx</c>):
/// </para>
/// <code>
///   child style                        parent first   parent last
///   sets w:ind only                       11.50          11.50     no item of its own
///   sets w:spacing w:before="0"           11.50          11.50     item replaced, after 0 either way
///   sets w:spacing w:after="240"          11.50        * 23.50     item replaced, flag lost
///   the parent itself                     11.50          11.50
/// </code>
/// <para>
/// Only the starred row moves, and the <c>w:ind</c> row is the control that says the
/// <c>w:basedOn</c> link is intact: the child takes the parent's 1440-twip indent under both orders,
/// so this is one item's flag rather than style inheritance failing.
/// </para>
/// <para>
/// <b>Reach: one corpus document.</b> <c>PES-Technical-Report-Template_Jan_2019.docx</c>, whose
/// bullet style <c>Heading9</c> is declared tenth and based on <c>ListParagraph</c>, declared 114th.
/// Suppressing the 12 pt between its bullets fits an extra bullet on several pages and costs the
/// document the fourteenth page the reference has.
/// </para>
/// </remarks>
public sealed class ForwardBasedOnContextualSpacingTests
{
    /// <summary>The parent declared first: the flag is inherited as one would expect.</summary>
    [Fact]
    public void AParentDeclaredFirstPassesTheFlagDown()
    {
        Resolve(parentFirst: true, "Aft").HasContextualSpacing.ShouldBeTrue();
    }

    /// <summary>The parent declared last: setting <c>w:after</c> replaces the item and loses it.</summary>
    [Fact]
    public void AParentDeclaredLastLosesTheFlagToAMarginOfItsOwn()
    {
        Resolve(parentFirst: false, "Aft").HasContextualSpacing.ShouldBeFalse();
    }

    /// <summary>A style with no margin of its own has no item to shadow the parent's with.</summary>
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void AStyleSettingNoMarginKeepsTheFlagEitherWay(bool parentFirst)
    {
        Resolve(parentFirst, "NoSp").HasContextualSpacing.ShouldBeTrue();
    }

    /// <summary>The parent itself is unaffected by where it sits.</summary>
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TheParentKeepsItsOwnFlag(bool parentFirst)
    {
        Resolve(parentFirst, "LP").HasContextualSpacing.ShouldBeTrue();
    }

    /// <summary>A style stating the flag itself is never touched.</summary>
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void AStyleStatingTheFlagItselfKeepsIt(bool parentFirst)
    {
        Resolve(parentFirst, "Own").HasContextualSpacing.ShouldBeTrue();
    }

    /// <summary>
    /// A style whose chain never turns the flag on does not gain one.
    /// </summary>
    /// <remarks>
    /// The guard that keeps the rule inert on the other 271 corpus DOCX: the synthesised
    /// <c>w:contextualSpacing w:val="0"</c> is only written where there was a flag to lose.
    /// </remarks>
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void AStyleWithNoContextualAncestorIsUntouched(bool parentFirst)
    {
        Resolve(parentFirst, "Plain").HasContextualSpacing.ShouldBeFalse();
    }

    private const string Ns = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>The parent, which is where the flag comes from.</summary>
    private const string Parent = """
        <w:style w:type="paragraph" w:styleId="LP"><w:name w:val="List Paragraph"/>
          <w:basedOn w:val="Normal"/>
          <w:pPr><w:spacing w:after="240"/><w:contextualSpacing/></w:pPr>
        </w:style>
        """;

    /// <summary>The children, which differ only in what each sets for itself.</summary>
    private const string Children = """
        <w:style w:type="paragraph" w:styleId="NoSp"><w:name w:val="No Spacing Set"/>
          <w:basedOn w:val="LP"/><w:pPr><w:ind w:left="0"/></w:pPr>
        </w:style>
        <w:style w:type="paragraph" w:styleId="Aft"><w:name w:val="After Set"/>
          <w:basedOn w:val="LP"/><w:pPr><w:spacing w:after="240"/></w:pPr>
        </w:style>
        <w:style w:type="paragraph" w:styleId="Own"><w:name w:val="Own Flag"/>
          <w:basedOn w:val="LP"/>
          <w:pPr><w:spacing w:after="240"/><w:contextualSpacing/></w:pPr>
        </w:style>
        <w:style w:type="paragraph" w:styleId="Plain"><w:name w:val="Plain"/>
          <w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="240"/></w:pPr>
        </w:style>
        """;

    private static ParagraphFormat Resolve(bool parentFirst, string styleId)
    {
        string body = parentFirst ? Parent + Children : Children + Parent;
        XElement root = XElement.Parse(
            $"""
            <w:styles xmlns:w="{Ns}">
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
              {body}
            </w:styles>
            """);

        WordStyles styles = new();
        styles.Add(root);

        XElement properties = XElement.Parse(
            $"""<w:pPr xmlns:w="{Ns}"><w:pStyle w:val="{styleId}"/></w:pPr>""");

        return WordParagraphFormats.Resolve(styles, properties, Length.FromTwips(720));
    }
}
