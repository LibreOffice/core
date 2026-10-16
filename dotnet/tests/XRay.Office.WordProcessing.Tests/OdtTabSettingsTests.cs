using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.OpenDocument;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The document setting that decides where a tab stop is measured from.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwTextFormatter::NewTabPortion</c> takes the tab origin as
/// <c>m_pFrame-&gt;getFrameArea().Left() + (bTabsRelativeToIndent ? GetTabLeft() : 0)</c>
/// (<c>sw/source/core/text/txttab.cxx</c>:94-98, under the comment <em>"#i24363# tab stops relative to
/// indent"</em>), and <c>GetTabLeft()</c> is the paragraph's own text-left margin
/// (<c>SwTextNode::GetLeftMarginForTabCalculation</c>, <c>sw/source/core/txtnode/ndtxt.cxx</c>:3573-3593).
/// So the flag shifts every stop of an indented paragraph by that indent, and a tab that lands on the
/// first stop under one rule lands on the second under the other.
/// </para>
/// <para>
/// <b>Absent means true</b> — <c>mbTabRelativeToIndent(true)</c>,
/// <c>sw/source/core/doc/DocumentSettingManager.cxx</c>:80 — which is Writer's own answer for a document
/// it created. Every Word-family importer turns it off: <c>ww8par.cxx</c>:1951 for a <c>.doc</c>, and
/// <c>DomainMapper</c>'s constructor for DOCX and RTF alike
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:128-132). This tree's DOCX, RTF and WW8
/// readers each set <see cref="ParagraphFormat.TabsRelativeToIndent"/> false by hand for that reason;
/// the ODF reader is the one that has to <em>read</em> it, because LibreOffice's ODF export writes the
/// resulting flag into <c>settings.xml</c> and all 338 converted <c>.odt</c> of the corpus state false.
/// </para>
/// <para>
/// <b>Read out of the source rather than out of a rendering, for the reason
/// <see cref="OdtFlySettingsTests"/> records</b>: a minimal hand-authored flat ODF's
/// <c>office:settings</c> is ignored outright by 26.2.4.2, so a fixture built to isolate one config item
/// measures nothing at the reference. What the reference did settle is the consequence on a real file —
/// <c>A_320.odt</c>, whose 1041 tabbed indented paragraphs put it at 134 pages against 26.2.4.2's 118,
/// and which is page-exact and glyph-exact once the flag is read. Its own <c>.doc</c> spelling, through
/// the WW8 reader that already sets the flag, was 118 of 118 throughout.
/// <c>probes/odt-page-r87/results.md</c>.
/// </para>
/// </remarks>
public sealed class OdtTabSettingsTests
{
    /// <summary>The three arms of the setting, of which the absent one cannot be inferred from the file.</summary>
    [Theory]
    [InlineData("true", true)]
    [InlineData("false", false)]
    [InlineData(null, true)]
    public void TabsRelativeToIndentDefaultsToWritersOwnAnswer(string? stated, bool expected)
        => OdtLayoutSource.TabsRelativeToIndent(Settings("TabsRelativeToIndent", stated))
            .ShouldBe(expected);

    /// <summary>
    /// The setting reaches the paragraph, and moves the origin its stops are measured from.
    /// </summary>
    /// <remarks>
    /// The wiring is the half that breaks silently: the flag was readable from the settings for as long
    /// as the reader has had them and simply never reached <see cref="ParagraphFormat"/>, so every
    /// converted <c>.odt</c> laid its tabs out by Writer's native rule instead of by the Word one the
    /// file asks for.
    /// </remarks>
    [Theory]
    [InlineData(true, 43.2)]
    [InlineData(false, 0.0)]
    public void TheSettingDecidesTheParagraphsTabOrigin(bool relative, double originPoints)
    {
        ParagraphFormat format = OdfParagraphFormats.Resolve(
            Styles(), "Indented", tabsRelativeToIndent: relative);

        format.TabsRelativeToIndent.ShouldBe(relative);
        format.StartIndent.Points.ShouldBe(43.2, 0.01);
        format.TabOrigin.Points.ShouldBe(originPoints, 0.01);
    }

    /// <summary>
    /// A tab in a hanging-indented paragraph reaches a different stop under each rule.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>A_320.odt</c>'s own shape reduced to one paragraph: 0.6 inch of left margin, the same
    /// again of negative first-line indent, and stops at 0.3, 0.6 and 0.9 inch. The first line therefore
    /// begins at the text area's edge, and the tab that opens it is <em>inside the hanging indent</em>
    /// under both rules — but what that costs differs, because the left-margin overrule
    /// (<c>txttab.cxx</c>:222-285, <c>#i115705#</c>) is at <c>nLeftMarginTabPos</c>, which is the
    /// paragraph's indent under the Word rule and <b>zero</b> under Writer's.
    /// </para>
    /// <para>
    /// Under the Word rule the overrule does not fire — the nearest stop, 0.3 inch, is not beyond the
    /// 0.6 inch left margin — so the tab reaches 0.3 inch. Under Writer's the margin is at the origin,
    /// every stop is beyond it, and the tab is overruled back to the indent at 0.6 inch. Both answers are
    /// measured in 26.2.4.2's own PDF of that document: it draws <c>52</c> at x 53.50 against a text edge
    /// of 31.90, which is 0.3 inch, and this tree drew it at 75.00, which is 0.6.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(false, 21.6)]
    [InlineData(true, 43.2)]
    public void ATabInAHangingIndentReachesADifferentStopUnderEachRule(bool relative, double expected)
    {
        ParagraphFormat format = OdfParagraphFormats.Resolve(
            Styles(), "Indented", tabsRelativeToIndent: relative);

        // Where the first line starts, in the coordinates the stops are stated in: zero under the Word
        // rule, and minus the whole indent under Writer's, since the origin has moved to the indent.
        Length start = format.TabLineOffset(isFirstLine: true);

        Length reached = format.TabOrigin + format.NextTabStop(start).Position;

        reached.Points.ShouldBe(expected, 0.01);
    }

    /// <summary>One paragraph style with A_320's indents and stops.</summary>
    private static OdfStyles Styles()
    {
        XElement root = XElement.Parse($$"""
            <office:document-styles
                xmlns:office="{{OdfNamespaces.Office}}"
                xmlns:style="{{OdfNamespaces.Style}}"
                xmlns:fo="{{OdfNamespaces.FoCompatible}}">
              <office:styles>
                <style:style style:name="Indented" style:family="paragraph">
                  <style:paragraph-properties fo:margin-left="0.6in" fo:text-indent="-0.6in">
                    <style:tab-stops>
                      <style:tab-stop style:position="0.3in"/>
                      <style:tab-stop style:position="0.6in"/>
                      <style:tab-stop style:position="0.9in"/>
                    </style:tab-stops>
                  </style:paragraph-properties>
                </style:style>
              </office:styles>
            </office:document-styles>
            """);

        OdfStyles styles = new();
        styles.AddDocument(root, null);
        return styles;
    }

    /// <summary>A settings tree stating one item, in the shape LibreOffice's own export writes.</summary>
    /// <param name="name">The item's <c>config:name</c>.</param>
    /// <param name="value">Its value, or null to state no item at all.</param>
    private static XElement Settings(string name, string? value)
    {
        XNamespace config = "urn:oasis:names:tc:opendocument:xmlns:config:1.0";
        XNamespace office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";

        XElement set = new(
            config + "config-item-set",
            new XAttribute(config + "name", "ooo:configuration-settings"));

        if (value is not null)
        {
            set.Add(new XElement(
                config + "config-item",
                new XAttribute(config + "name", name),
                new XAttribute(config + "type", "boolean"),
                value));
        }

        return new XElement(office + "settings", set);
    }
}
