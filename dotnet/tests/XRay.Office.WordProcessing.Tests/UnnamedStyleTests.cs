using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>w:style</c> with no <c>w:name</c> cannot be referenced, so it is dropped when the styles are
/// read rather than skipped at each place a reference might reach it.
/// </summary>
/// <remarks>
/// <para>
/// <c>StyleSheetTable::sprm</c> (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:774)
/// appends a finished entry to neither <c>m_aStyleSheetEntries</c> nor
/// <c>m_aStyleSheetEntriesMap</c> unless <c>!IsOOXMLImport() || !m_sStyleName.isEmpty()</c>. The
/// identifier map is what every <c>w:pStyle</c>, <c>w:rStyle</c>, <c>w:tblStyle</c> and
/// <c>w:basedOn</c> is resolved through, so an unnamed style is invisible to all four.
/// </para>
/// <para>
/// The citation is the hypothesis and <c>dotnet/probes/words-r47/unnamed-style.py</c> is the
/// evidence: a table, paragraph and character style each stating <c>w:sz w:val="20"</c>, applied to
/// a run whose document defaults say 24, authored twice apiece with and without a <c>w:name</c>.
/// LibreOffice 24.2.7.2 draws 10 pt with the name and 12 pt without it in all three families.
/// </para>
/// <para>
/// The corpus half is a causal mutation rather than an argument. Sixteen of the fifty-three styles
/// in <c>template---tpr-technical-progress-report-with-guidance.docx</c> carry a <c>w:styleId</c>
/// and no <c>w:name</c>, four of them the table styles its tables name. Changing one of those
/// styles' own <c>w:spacing</c> from 240 to 480 moves nothing in the reference's output; changing
/// <c>w:docDefaults</c> from 276 to 480 moves the cell text; adding <c>&lt;w:name&gt;</c> makes the
/// style's own 10 pt appear.
/// </para>
/// <para>
/// <strong>The refuted alternative, pinned here.</strong> The first reading of that document was
/// that LibreOffice ranks <c>w:docDefaults</c> <em>above</em> a table style, against §17.7.2.
/// <c>dotnet/probes/words-r47/table-style-vs-docdefaults.py</c> says it does not: with a
/// <em>named</em> table style stating <c>w:line="240"</c> and defaults stating 276, LibreOffice
/// takes the table style's single spacing. The ranking was never the question; the name was.
/// </para>
/// </remarks>
public sealed class UnnamedStyleTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    [Fact]
    public void ANamedParagraphStyleIsApplied()
        => Resolve(Styles(name: true), "<w:pStyle w:val=\"Quiet\"/>")
            .SpaceBefore.ShouldBe(Length.FromTwips(480));

    [Fact]
    public void AnUnnamedParagraphStyleIsNotApplied()
        => Resolve(Styles(name: false), "<w:pStyle w:val=\"Quiet\"/>")
            .SpaceBefore.ShouldBe(Length.Zero);

    [Fact]
    public void AStyleWhoseNameIsEmptyIsNotAppliedEither()
        => Resolve(Styles(name: true, value: ""), "<w:pStyle w:val=\"Quiet\"/>")
            .SpaceBefore.ShouldBe(Length.Zero);

    [Fact]
    public void ANamedStyleBasedOnAnUnnamedOneDoesNotInheritThroughIt()
    {
        string styles = $"""
            <w:styles xmlns:w="{W}">
              <w:docDefaults><w:rPrDefault><w:rPr><w:sz w:val="24"/></w:rPr></w:rPrDefault></w:docDefaults>
              <w:style w:type="paragraph" w:styleId="Quiet">
                <w:pPr><w:spacing w:before="480"/></w:pPr>
              </w:style>
              <w:style w:type="paragraph" w:styleId="Louder">
                <w:name w:val="Louder"/><w:basedOn w:val="Quiet"/>
              </w:style>
            </w:styles>
            """;

        Resolve(styles, "<w:pStyle w:val=\"Louder\"/>").SpaceBefore.ShouldBe(Length.Zero);
    }

    [Fact]
    public void AnUnnamedTableStyleIsNotFound()
    {
        WordStyles styles = Read(Styles(name: false, type: "table"));

        styles.TableStyleParagraphProperties("Quiet").ShouldBeEmpty();
    }

    [Fact]
    public void ANamedTableStyleIsStillFound()
    {
        WordStyles styles = Read(Styles(name: true, type: "table"));

        styles.TableStyleParagraphProperties("Quiet").Count.ShouldBe(1);
    }

    /// <summary>
    /// A drift guard rather than a rule: an unnamed style must not become the document's default
    /// paragraph style either, since it was never added to the table it would be found in.
    /// </summary>
    [Fact]
    public void AnUnnamedStyleCannotBecomeTheDefault()
    {
        string styles = $"""
            <w:styles xmlns:w="{W}">
              <w:style w:type="paragraph" w:default="1" w:styleId="Quiet">
                <w:pPr><w:spacing w:before="480"/></w:pPr>
              </w:style>
            </w:styles>
            """;

        Read(styles).DefaultStyleId(WordStyleType.Paragraph).ShouldBeNull();
    }

    private static string Styles(bool name, string value = "Quiet", string type = "paragraph")
    {
        string named = name ? $"<w:name w:val=\"{value}\"/>" : "";
        return $"""
            <w:styles xmlns:w="{W}">
              <w:docDefaults><w:rPrDefault><w:rPr><w:sz w:val="24"/></w:rPr></w:rPrDefault></w:docDefaults>
              <w:style w:type="{type}" w:styleId="Quiet">
                {named}<w:pPr><w:spacing w:before="480"/></w:pPr>
              </w:style>
            </w:styles>
            """;
    }

    private static WordStyles Read(string stylesXml)
    {
        WordStyles styles = new();
        styles.Add(XElement.Parse(stylesXml));
        return styles;
    }

    private static ParagraphFormat Resolve(string stylesXml, string paragraphProperties)
        => WordParagraphFormats.Resolve(
            Read(stylesXml),
            XElement.Parse($"<w:pPr xmlns:w=\"{W}\">{paragraphProperties}</w:pPr>"),
            Length.FromTwips(720));
}
