using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A DOCX paragraph has widow and orphan control when the package declared a
/// <c>w:docDefaults/w:pPrDefault</c>, and none at all when it did not.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="DocWidowControlTests"/>, and the two readers used to disagree: the
/// WW8 one defaults the flag on, while this one read an absent <c>w:widowControl</c> as off, so no
/// ordinary DOCX had widow or orphan control at all.
/// </para>
/// <para>
/// The rule is not "on by default". It is hung on the *presence* of a <c>w:pPrDefault</c> element,
/// which Word writes empty and which LibreOffice treats as the trigger for a document-wide default:
/// <c>StyleSheetTable::applyDefaults</c> puts <c>ParaWidows</c> and <c>ParaOrphans</c> at 2 on the
/// built-in style every other style inherits from, and it is called only from the
/// <c>w:pPrDefault</c> arm of <c>StyleSheetTable::sprm</c>
/// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:653-670, 2115-2160) — <em>"WARNING:
/// these defaults only take effect IF there is a DocDefaults style section."</em>
/// </para>
/// <para>
/// The citation is a hypothesis and <c>dotnet/probes/words-r46/widow-orphan-default.py</c> is the
/// evidence: nine authored variants at five straddle positions of a four-line paragraph, measured
/// against the installed 24.2.7.2, with a control variant stating <c>w:widowControl w:val="0"</c> so
/// the room at the foot of the page is measured rather than assumed. A package with no
/// <c>w:pPrDefault</c> splits the paragraph one-and-three; one with an empty <c>w:pPrDefault</c>
/// moves the whole paragraph, exactly as an explicit <c>&lt;w:widowControl/&gt;</c> does.
/// </para>
/// <para>
/// The refuted alternative, pinned here because it is the reading a later round would reach for
/// first: the document-level <c>w:settings/w:widowControl</c> does <em>not</em> turn it on. That
/// variant behaves identically to the control in every one of the five straddle positions.
/// </para>
/// </remarks>
public sealed class DocxWidowControlTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>An ordinary Word package: a bare <c>w:pPrDefault</c>, nothing else stated.</summary>
    [Fact]
    public void AnEmptyParagraphDefaultTurnsWidowAndOrphanControlOn()
    {
        ParagraphFormat format = Resolve(Styles(paragraphDefault: "<w:pPrDefault/>"));

        format.WidowLines.ShouldBe(2);
        format.OrphanLines.ShouldBe(2);
    }

    /// <summary>A <c>w:pPrDefault</c> carrying unrelated properties is still the trigger.</summary>
    [Fact]
    public void AParagraphDefaultWithContentIsStillTheTrigger()
    {
        ParagraphFormat format = Resolve(Styles(
            paragraphDefault: "<w:pPrDefault><w:pPr><w:spacing w:after=\"0\"/></w:pPr></w:pPrDefault>"));

        format.WidowLines.ShouldBe(2);
        format.OrphanLines.ShouldBe(2);
    }

    /// <summary>
    /// No <c>w:pPrDefault</c>, so Writer keeps its own default of nought and nothing is controlled.
    /// </summary>
    [Fact]
    public void WithNoParagraphDefaultThereIsNoControlAtAll()
    {
        ParagraphFormat format = Resolve(Styles(paragraphDefault: ""));

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }

    /// <summary>No <c>w:docDefaults</c> at all is the same case.</summary>
    [Fact]
    public void WithNoDocumentDefaultsThereIsNoControlAtAll()
    {
        ParagraphFormat format = Resolve($"<w:styles xmlns:w=\"{W}\"/>");

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }

    /// <summary>
    /// The default is applied without overwriting, so a <c>w:widowControl</c> inside the
    /// <c>w:pPrDefault</c> itself wins — writerfilter's <c>bOverwrite=false</c>, and the authored
    /// probe agrees.
    /// </summary>
    [Fact]
    public void TheParagraphDefaultsOwnFlagBeatsTheDefault()
    {
        ParagraphFormat format = Resolve(Styles(
            paragraphDefault:
            "<w:pPrDefault><w:pPr><w:widowControl w:val=\"0\"/></w:pPr></w:pPrDefault>"));

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }

    /// <summary>A paragraph turning the flag off keeps it off, trigger or no trigger.</summary>
    [Fact]
    public void AParagraphTurningItOffKeepsItOff()
    {
        ParagraphFormat format = Resolve(
            Styles(paragraphDefault: "<w:pPrDefault/>"),
            "<w:widowControl w:val=\"0\"/>");

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }

    /// <summary>And a style in the chain turning it off does too.</summary>
    [Fact]
    public void AStyleTurningItOffKeepsItOff()
    {
        string styles = $"""
            <w:styles xmlns:w="{W}">
              <w:docDefaults><w:pPrDefault/></w:docDefaults>
              <w:style w:type="paragraph" w:styleId="Quiet"><w:name w:val="Quiet"/>
                <w:pPr><w:widowControl w:val="0"/></w:pPr>
              </w:style>
            </w:styles>
            """;

        ParagraphFormat format = Resolve(styles, "<w:pStyle w:val=\"Quiet\"/>");

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }

    private static string Styles(string paragraphDefault) => $"""
        <w:styles xmlns:w="{W}">
          <w:docDefaults>
            <w:rPrDefault><w:rPr><w:sz w:val="24"/></w:rPr></w:rPrDefault>
            {paragraphDefault}
          </w:docDefaults>
        </w:styles>
        """;

    private static ParagraphFormat Resolve(string stylesXml, string paragraphProperties = "")
    {
        WordStyles styles = new();
        styles.Add(XElement.Parse(stylesXml));

        XElement? properties = paragraphProperties.Length == 0
            ? null
            : XElement.Parse($"<w:pPr xmlns:w=\"{W}\">{paragraphProperties}</w:pPr>");

        return WordParagraphFormats.Resolve(styles, properties, Length.FromTwips(720));
    }
}
