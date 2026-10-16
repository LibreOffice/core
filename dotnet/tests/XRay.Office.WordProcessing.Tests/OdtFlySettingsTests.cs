using System.Xml.Linq;
using XRay.Office.WordProcessing.OpenDocument;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The two document settings that decide what happens to a fly holding a table.
/// </summary>
/// <remarks>
/// <para>
/// <c>DoNotBreakWrappedTables</c> is a veto: <c>SwFlyFrame::IsFlySplitAllowed</c> refuses while it holds
/// (<c>sw/source/core/layout/fly.cxx</c>:696-700), before it looks at the fly at all, so a document
/// stating it splits nothing however many of its frames say <c>loext:may-break-between-pages</c>.
/// <c>TabOverMargin</c> chooses the <em>deadline</em> a fly that may split is cut at: it is
/// <c>isLegacyBehavior</c>'s first test (<c>fly.cxx</c>:101-110) and it swaps the body frame's print
/// bottom for the page's, limited so the fly still fits the body's height
/// (<c>GetFlyAnchorBottom</c>, :113-162).
/// </para>
/// <para>
/// Both default to <em>false</em> when the file states nothing —
/// <c>sw/source/core/doc/DocumentSettingManager.cxx</c>:92 for the first, and
/// <c>SwXMLImport::SetConfigurationSettings</c> setting the second only when the item says true
/// (<c>sw/source/filter/xml/xmlimp.cxx</c>:1627-1630). Reach on the converted corpus: <b>177 of the 338
/// <c>.odt</c> state <c>TabOverMargin</c> true and 161 false; 30 state
/// <c>DoNotBreakWrappedTables</c></b>.
/// </para>
/// <para>
/// <b>Read out of the source and not out of a rendering, deliberately.</b> A minimal hand-authored flat
/// ODF's <c>office:settings</c> is ignored outright by 26.2.4.2 — demonstrated over forty probes at twenty
/// page widths, byte-identical either way — so a fixture built to isolate one config item measures
/// nothing at the reference. What the reference <em>did</em> settle is the consequence, on real files:
/// the two graph-paper templates that state <c>TabOverMargin</c> draw their fly 9.75 pt past the body's
/// bottom on one page, and reading the flag as absent split them onto two.
/// </para>
/// </remarks>
public sealed class OdtFlySettingsTests
{
    /// <summary><c>TabOverMargin</c> is off unless the file says otherwise.</summary>
    [Theory]
    [InlineData("true", true)]
    [InlineData("false", false)]
    [InlineData(null, false)]
    public void TabOverMarginDecidesWhetherAFlyMayRunPastTheBodysBottom(string? stated, bool expected)
        => OdtLayoutSource.FliesMayOverlapTheBottomMargin(Settings("TabOverMargin", stated))
            .ShouldBe(expected);

    /// <summary>
    /// <c>DoNotBreakWrappedTables</c> is the veto, and its absent case is the permissive one.
    /// </summary>
    /// <remarks>
    /// Stated the other way up from the flag it reads, because what the layout wants to know is whether
    /// it <em>may</em> break — the same inversion <c>fo:keep-together</c> gets on a table row.
    /// </remarks>
    [Theory]
    [InlineData("true", false)]
    [InlineData("false", true)]
    [InlineData(null, true)]
    public void DoNotBreakWrappedTablesVetoesEverySplit(string? stated, bool expected)
        => OdtLayoutSource.BreaksWrappedTables(Settings("DoNotBreakWrappedTables", stated))
            .ShouldBe(expected);

    /// <summary>
    /// A settings tree stating one item, in the shape LibreOffice's own export writes.
    /// </summary>
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
