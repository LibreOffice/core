using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.Core.Threading;

namespace XRay.Office.WordProcessing.OpenDocument;

/// <summary>
/// Turns an ODF text document's body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>content.xml</c>, separate from the one that builds the extraction tree, and
/// deliberately so: extraction discards font sizes, indents and spacing because it must not pay for
/// them, and layout needs nothing else. Sharing one walk would mean either extraction carrying
/// formatting it never reads or layout re-deriving it from a tree that no longer has it.
/// </para>
/// <para>
/// The spans are walked as well as the paragraph, because ODF has no inline formatting: one bold word is
/// an automatic style and a <c>text:span</c> pointing at it, so a reader that resolved only the paragraph
/// style would measure a mixed paragraph wholly in its body font — short lines wherever the emphasis is
/// larger, and the wrong face wherever it is bolder. A paragraph whose spans resolve to the paragraph's
/// own formatting carries no runs at all, which keeps plain prose on the cheap single-face path.
/// </para>
/// </remarks>
public sealed partial class OdtLayoutSource
{
    /// <summary>
    /// How many paragraphs are read before the rest are ignored.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input rather than a real limit. A book is tens of thousands of paragraphs;
    /// a generated file can claim millions, and each one costs a shaping pass.
    /// </remarks>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character a <c>text:line-break</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, the Unicode line separator, because UAX #14 gives it a mandatory break and the break
    /// iterator therefore honours it without layout having to special-case anything. A newline would do
    /// the same but would also read as the end of a paragraph to anything that later scans the text,
    /// which a line break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>
    /// The character an anchor occupies.
    /// </summary>
    /// <remarks>
    /// The same one the document model uses, so an offset counted here means the same thing there. It
    /// has a width but no text: a footnote's citation occupies a position in the sentence that cites it
    /// while its body lives in another flow.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    private readonly OdfStyles _styles;
    private readonly IReadOnlyList<Length> _sectionMargins;
    private readonly OdfPictures? _pictures;
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>
    /// Text styles already resolved, keyed by the cascade that produced them.
    /// </summary>
    /// <remarks>
    /// A document has a handful of span styles and thousands of spans referencing them, and resolving one
    /// walks a parent chain per property. The key is the cascade's names in order, because two spans with
    /// the same style inside different paragraph styles resolve differently.
    /// </remarks>
    private readonly Dictionary<string, OdfTextStyle> _resolved = new(StringComparer.Ordinal);

    /// <summary>Creates a source over a document's styles.</summary>
    /// <param name="styles">The document's resolved styles.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    /// <param name="masterPages">
    /// Which section each master page is, by name. Empty for a document laid out on one section, which
    /// leaves every block in section zero.
    /// </param>
    /// <param name="stylesRoot">
    /// The root of the styles part, for the <c>text:notes-configuration</c> that says how each class of note
    /// is numbered. Null leaves both classes on LibreOffice's defaults.
    /// </param>
    /// <param name="pictures">
    /// How to reach the bytes behind a <c>draw:image</c>, or null to lay the document out with its
    /// picture frames empty — which is what a caller who wants only measurements should pay for.
    /// </param>
    /// <param name="settings">
    /// The document's <c>office:settings</c>, or null for one that states none. Only the compatibility
    /// flags are read from it; see <see cref="_blanksAreTransparentToHeight"/>.
    /// </param>
    /// <param name="sectionMargins">
    /// Each section's left margin, in the order the masters are in, or null for a caller that states no
    /// geometry. Read only by <see cref="SectionLeftMargin"/>, for a floating table whose fly measures its
    /// offset from the sheet's own edge.
    /// </param>
    /// <param name="sectionCount">
    /// How many sections the caller will lay out — one per master page. A <c>text:section</c> that changes
    /// the geometry is numbered from here upwards, so a caller that leaves this at nought gets the old
    /// behaviour, in which such a section is transparent. See <see cref="ColumnSections"/>.
    /// </param>
    public OdtLayoutSource(
        OdfStyles styles,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, int>? masterPages = null,
        XElement? stylesRoot = null,
        OdfPictures? pictures = null,
        XElement? settings = null,
        IReadOnlyList<Length>? sectionMargins = null,
        int sectionCount = 0)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _sectionMargins = sectionMargins ?? [];
        _sectionCount = sectionCount;
        _pictures = pictures;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _masterPages = masterPages ?? new Dictionary<string, int>(StringComparer.Ordinal);
        _footnotes = NumberingIn(stylesRoot, "footnote", NoteNumbering.Footnotes);
        _endnotes = NumberingIn(stylesRoot, "endnote", NoteNumbering.Endnotes);
        _blanksAreTransparentToHeight =
            Setting(settings, "IgnoreTabsAndBlanksForLineCalculation") == "true";
        _shrinksJustifiedBlanks = ShrinksJustifiedBlanks(settings);
        _breaksWrappedTables = BreaksWrappedTables(settings);
        _tabsRelativeToIndent = TabsRelativeToIndent(settings);
        _rowHeightsIncludeInsets = RowHeightsIncludeInsets(settings);
    }

    /// <summary>
    /// Whether tabs and blanks are left out of a line's height, as the document's settings say.
    /// </summary>
    /// <remarks>
    /// False unless the file asks for it, which is the ODF default — and the reason it can be asked for at
    /// all is that LibreOffice writes the flag out. A document exported from a <c>.doc</c> or a <c>.docx</c>
    /// carries <c>true</c>, so a flat-ODF round trip of a Word file lays out the way the Word file does
    /// instead of quietly diverging from it. See <see cref="PageParagraph.BlanksAreTransparentToHeight"/>.
    /// </remarks>
    private readonly bool _blanksAreTransparentToHeight;

    /// <summary>
    /// Whether a justified line may squeeze its blanks below their natural width, as the document's
    /// settings say.
    /// </summary>
    /// <remarks>
    /// See <see cref="ShrinksJustifiedBlanks(XElement?)"/>, which is where the rule is stated; the value
    /// is resolved once per document because the setting is document-wide.
    /// </remarks>
    private readonly bool _shrinksJustifiedBlanks;

    /// <summary>
    /// Whether a tab stop's position is measured from the paragraph's own indent, as the document's
    /// settings say.
    /// </summary>
    /// <remarks>
    /// See <see cref="TabsRelativeToIndent(XElement?)"/>; the value is resolved once per document because
    /// the setting is document-wide.
    /// </remarks>
    private readonly bool _tabsRelativeToIndent;

    /// <summary>
    /// Whether a row's declared height carries its insets and its own top rule.
    /// </summary>
    /// <remarks>See <see cref="RowHeightsIncludeInsets"/>, which has the measurement.</remarks>
    private readonly bool _rowHeightsIncludeInsets;

    /// <summary>
    /// Whether two consecutive paragraphs' spacings add rather than the larger one winning, as the
    /// document's settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>PARA_SPACE_MAX</c>, which ODF spells <c>AddParaTableSpacing</c>
    /// (<c>SwXDocumentSettings.cxx</c> names the property at :189 and sets the flag at :449). Set, the two
    /// spacings are summed; clear, the larger wins — the name says the opposite of what it does.
    /// </para>
    /// <para>
    /// True when the file states nothing, because the setting then keeps the application's configured
    /// <c>AddSpacing</c>, whose shipped default is <c>true</c>
    /// (<c>officecfg/registry/schema/org/openoffice/Office/Compatibility.xcs</c>). That is the ODF
    /// behaviour X-Ray.Office already had; what it did not have is the other branch, which a document
    /// converted from a Word file always carries.
    /// </para>
    /// <para>
    /// Measured on <c>paragraph-spacing-collapsed.odt</c> and its flat twin — eight paragraphs each with
    /// 12 pt of space-before and 8 pt of space-after on 12 pt exact lines, saved by LibreOffice from the
    /// collapsing DOCX: with <c>AddParaTableSpacing</c> false LibreOffice 24.2.7.2 puts every boundary at
    /// 24.00 pt and with it true at 32.00 pt. Both containers honour it, so this is not a packaged-only
    /// setting.
    /// </para>
    /// <para>
    /// Like the DOCX case it reaches nothing on the corpus — the 200-file words track holds no ODF
    /// document at all — so it is correctness rather than a measured win.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r61 — all three arms hold on 26.2.4.2, by
    /// `probes/words-r61/audit_odfspacing.py`. The flag **false** gives 24.00 pt boundaries, **true**
    /// gives 32.00, and the item **removed entirely** gives 32.00 — so absent is true, which is the
    /// arm no stated value can imply and the one every real document takes. Both sides agree on all
    /// three to 0.01 pt. Round 59 verified the DOCX twin
    /// (`WordCompatibility.AddsParagraphSpacing`); this is the ODF path, which reaches the flag
    /// through `SwXMLImport::SetConfigurationSettings` rather than through
    /// `DomainMapper_Impl::ApplySettingsTable`, so the two are separate claims.
    /// **A harness trap found on the way and worth more than the verification**: a minimal flat ODF
    /// authored for the probe — correct namespaces, `ooo:configuration-settings`, the item spelled
    /// as the fixture spells it — was read correctly by *this* reader and **ignored outright by
    /// 26.2.4.2**, which answered 32.00 in all six arms including the two stating `false`. Had the
    /// probe stopped there it would have reported the site WRONG. The arms are now one string
    /// substitution each into round 53's own `paragraph-spacing-collapsed.fodt`, which the reference
    /// demonstrably reads.]
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool AddsParagraphSpacing(XElement? settings)
        => Setting(settings, "AddParaTableSpacing") != "false";

    /// <summary>
    /// Whether a page-anchored fly may run past the body's bottom into the margin, as the settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The deadline a split fly is cut at is <c>GetFlyAnchorBottom</c>
    /// (<c>sw/source/core/layout/fly.cxx</c>:113-162), and it has two arms. The Word &gt;= 2013 one is the
    /// body frame's print bottom; the Word &lt;= 2010 one is the <em>page's</em> bottom, limited so the fly
    /// still fits the body's height. <c>isLegacyBehavior</c> (:101-110) picks between them, and its first
    /// test is <c>DocumentSettingId::TAB_OVER_MARGIN</c> — which ODF spells <c>TabOverMargin</c> and which
    /// the Word import filters set for a file below <c>compatibilityMode</c> 15.
    /// </para>
    /// <para>
    /// Absent means <em>false</em>: <c>mbTabOverMargin(false)</c>
    /// (<c>sw/source/core/doc/DocumentSettingManager.cxx</c>:92), and the item is in none of
    /// <c>SwXMLImport::SetConfigurationSettings</c>' "use the old behaviour when it is missing" cases.
    /// <b>177 of the 338 converted <c>.odt</c> state it true and 161 false</b>, so it is not a rarity —
    /// and it is what separates the three graph-paper templates, whose fly runs 9.75 pt past the body's
    /// bottom and is not split, from the six documents the reference does split.
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool FliesMayOverlapTheBottomMargin(XElement? settings)
        => Setting(settings, "TabOverMargin") == "true";

    /// <summary>
    /// Whether a row's declared height is raised by its insets and its own top rule, as the settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER</c>, which ODF spells
    /// <c>MinRowHeightInclBorder</c> (<c>sw/source/uibase/uno/SwXDocumentSettings.cxx</c>:288). It is read
    /// twice by the row's own formatting — <c>lcl_CalcMinRowHeight</c>
    /// (<c>sw/source/core/layout/tabfrm.cxx</c>:5061 for an <c>exact</c> row and :5074-5090 for an
    /// <c>atLeast</c> one) — and it is what <see cref="Layout.PageTable.MinHeightIncludesInsets"/> gates.
    /// </para>
    /// <para>
    /// <b>It is a document setting and not a property of the filter, which is what this reader had wrong.</b>
    /// The only filter that sets it is WW8's (<c>sw/source/filter/ww8/ww8par.cxx</c>:1966), and
    /// LibreOffice's ODF <em>export</em> then writes the resulting state into <c>settings.xml</c> — so every
    /// <c>.odt</c> it wrote from a Word-family document carries <c>true</c>, and **all 337 of the converted
    /// corpus state it**. Absent means <em>false</em>: <c>mbMinRowHeightInclBorder(false)</c>,
    /// <c>sw/source/core/doc/DocumentSettingManager.cxx</c>:113 — Writer's own answer, which is why an
    /// unread setting is a missing charge rather than a spurious one.
    /// </para>
    /// <para>
    /// [bin] Measured on 26.2.4.2 over the eighteen arms of <c>probes/vmergetop-r154/</c>'s fixture 7
    /// converted to <c>.odt</c> by the reference itself: <b>ten differed before this was read and two after</b>,
    /// and five of the ten held no merged cell at all. <c>probes/odtrowheight-r155/</c>.
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool RowHeightsIncludeInsets(XElement? settings)
        => Setting(settings, "MinRowHeightInclBorder") == "true";

    /// <summary>
    /// Whether a fly holding a table may be broken across pages at all, as the settings say.
    /// </summary>
    /// <remarks>
    /// <c>SwFlyFrame::IsFlySplitAllowed</c> refuses outright while
    /// <c>DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES</c> holds
    /// (<c>sw/source/core/layout/fly.cxx</c>:696-700), before it looks at the fly at all — so it is a
    /// document-level veto over every <c>loext:may-break-between-pages</c> the file states. Absent means
    /// the flag is off and splitting is allowed; <b>30 of the 338 converted <c>.odt</c> state it true</b>,
    /// and <c>SwXMLImport::SetConfigurationSettings</c> sets the property only when they do
    /// (<c>sw/source/filter/xml/xmlimp.cxx</c>:1627-1630).
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool BreaksWrappedTables(XElement? settings)
        => Setting(settings, "DoNotBreakWrappedTables") != "true";

    /// <summary>
    /// Whether a paragraph keeps its space-before at the top of the first page and after a page break,
    /// as the document's settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>PARA_SPACE_MAX_AT_PAGES</c>, which ODF spells <c>AddParaTableSpacingAtStart</c>, and
    /// the companion of <see cref="AddsParagraphSpacing"/> — <c>ww8par.cxx</c> sets the two on
    /// consecutive lines (:1946 and :1947). It decides whether the question is asked at all;
    /// <c>SwFlowFrame::HasParaSpaceAtPages</c> then decides where, granting the space on the first page
    /// and after an explicit break and taking it away at every automatic one.
    /// </para>
    /// <para>
    /// True when the file states nothing, because the application's shipped <c>AddSpacingAtPages</c>
    /// default is <c>true</c>
    /// (<c>officecfg/registry/schema/org/openoffice/Office/Compatibility.xcs</c>:47,
    /// <c>DocumentSettingManager.cxx</c>:131). X-Ray.Office previously assumed the opposite for ODF, which
    /// put the first line of a document whose first paragraph has space-before too high by exactly that
    /// spacing.
    /// </para>
    /// <para>
    /// Measured on the eight-paragraph fixture, whose paragraphs each carry 12 pt of space-before, in
    /// points down an A4 page from LibreOffice 24.2.7.2: with the setting true the first baseline is at
    /// 93.60, with it false at 81.60, and <b>with the item removed entirely at 93.60</b> — so absent
    /// means true, which is the part that could not be inferred from the DOCX side.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r61 — 93.60 / 81.60 / 93.60 reproduce exactly
    /// on 26.2.4.2, all three, and the corner where both flags are off gives 81.60 with 24.00 pt
    /// boundaries, which is the control that says the two settings are independent rather than one
    /// flag read twice. `probes/words-r61/audit_odfspacing.py`; see the marker on
    /// <see cref="AddsParagraphSpacing"/> for the authored-fixture trap that the derived arms avoid.]
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool KeepsParagraphSpacingAtPages(XElement? settings)
        => Setting(settings, "AddParaTableSpacingAtStart") != "false";

    /// <summary>
    /// Whether a justified line may squeeze its blanks below their natural width to fit another word, as
    /// the document's settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>JUSTIFY_LINES_WITH_SHRINKING</c>, which ODF spells <c>JustifyLinesWithShrinking</c> and
    /// which <c>SwXMLImport::SetConfigurationSettings</c> (<c>sw/source/filter/xml/xmlimp.cxx</c>:1238)
    /// sets straight through as a document property — it is in none of that function's exclusion lists and
    /// none of its "use the old behaviour when the item is missing" cases, so an absent item leaves the
    /// member at its own default. That default is <b>false</b>
    /// (<c>mbJustifyLinesWithShrinking = false</c>, <c>sw/source/core/inc/DocumentSettingManager.hxx</c>:180),
    /// which is why this is the one compatibility flag in this file whose absent case is <em>off</em>
    /// rather than on.
    /// </para>
    /// <para>
    /// It is the ODF spelling of what <c>compatibilityMode</c> 15 gives a DOCX
    /// (<see cref="ParagraphFormat.ShrinksJustifiedBlanks"/>), and LibreOffice's own export writes it into
    /// <c>settings.xml</c> for every Word file it converts — <b>161 of the 338 converted <c>.odt</c></b>
    /// state it true. Reading it wrongly does not move a line by a fraction of a point: a line that cannot
    /// squeeze holds one word fewer, so the paragraph sets in more lines and the document paginates longer.
    /// </para>
    /// <para>
    /// The <em>paragraph</em>-level spelling of the same thing, <c>loext:word-spacing-minimum</c>, is a
    /// second and independent way in: <c>SwTextPortion::Format_</c>
    /// (<c>sw/source/core/text/portxt.cxx</c>:546-562) shrinks when the document setting is on
    /// <em>and</em> the paragraph states no word-spacing bounds of its own — its <c>bOldInterop</c>, the
    /// "support old ODT documents" case — <em>or</em> when the paragraph states bounds that are not all
    /// 100%, whatever the document setting says. On the converted corpus the second clause adds nothing:
    /// all 51 documents stating <c>loext:word-spacing-minimum</c> state it as <c>75%</c>, which is the
    /// floor <see cref="JustificationShrink.MinimumBlankProportion"/> already applies, and all 51 also
    /// state the document setting. It is deliberately not read.
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool ShrinksJustifiedBlanks(XElement? settings)
        => Setting(settings, "JustifyLinesWithShrinking") == "true";

    /// <summary>
    /// Whether a tab stop's declared position is measured from the paragraph's own left indent or from
    /// the text area's edge, as the document's settings say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwTextFormatter::NewTabPortion</c> takes the tab origin as
    /// <c>m_pFrame-&gt;getFrameArea().Left() + (bTabsRelativeToIndent ? GetTabLeft() : 0)</c>
    /// (<c>sw/source/core/text/txttab.cxx</c>:94-98, under the comment <em>"#i24363# tab stops relative
    /// to indent"</em>), where <c>GetTabLeft()</c> is the paragraph's own text-left margin
    /// (<c>SwTextNode::GetLeftMarginForTabCalculation</c>, <c>ndtxt.cxx</c>:3573-3593). So the flag moves
    /// every stop of an indented paragraph by that indent, and a tab that lands on the first stop under
    /// one rule lands on the second under the other.
    /// </para>
    /// <para>
    /// <b>Absent means true</b>: <c>mbTabRelativeToIndent(true)</c>
    /// (<c>sw/source/core/doc/DocumentSettingManager.cxx</c>:80), which is Writer's own answer for a
    /// document it created. Every Word-family importer turns it off — <c>ww8par.cxx</c>:1951 for a
    /// <c>.doc</c> and <c>DomainMapper</c>'s constructor for DOCX and RTF
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:128-132) — and LibreOffice's ODF export
    /// writes the resulting flag into <c>settings.xml</c>, so a <c>.odt</c> converted from a Word file
    /// carries <c>false</c> and must be laid out the way the Word file is. <b>All 338 converted
    /// <c>.odt</c> state it false</b>, which is what the corpus's provenance predicts and not a rarity.
    /// </para>
    /// <para>
    /// The reach is narrower than that census, because the two rules differ only for a paragraph that
    /// both holds a tab and carries a non-zero left indent: <b>38 of the 338 hold one, 3728 paragraphs
    /// in all</b>. <c>probes/odt-page-r87/census-tabs.py</c>.
    /// </para>
    /// </remarks>
    /// <param name="settings">The document's <c>office:settings</c>, or null.</param>
    internal static bool TabsRelativeToIndent(XElement? settings)
        => Setting(settings, "TabsRelativeToIndent") != "false";

    /// <summary>One <c>config:config-item</c>'s value from a document's settings, or null when absent.</summary>
    /// <remarks>
    /// The items are nested in <c>config:config-item-set</c> elements, so this searches by descendant name
    /// rather than walking the sets: which set a flag lives in is a detail of how LibreOffice groups them,
    /// and a reader that hard-codes the grouping breaks on the next version that regroups.
    /// </remarks>
    private static string? Setting(XElement? settings, string name)
        => settings?
            .Descendants(XName.Get("config-item", OdfNamespaces.Config))
            .FirstOrDefault(item =>
                item.Attribute(XName.Get("name", OdfNamespaces.Config))?.Value == name)?
            .Value;

    /// <summary>How the document's footnotes are numbered.</summary>
    private readonly NoteNumbering _footnotes;

    /// <summary>How its endnotes are numbered, which is a separate sequence in a separate format.</summary>
    private readonly NoteNumbering _endnotes;

    /// <summary>
    /// The numbering one class of note declares, or the class's default when it declares none.
    /// </summary>
    /// <remarks>
    /// <c>text:notes-configuration</c>, which lives in <c>office:styles</c> rather than in the content and is
    /// written once per class. ODF states the format by <em>example</em> — <c>style:num-format</c> holds the
    /// literal "1", "i" or "A" — which is why parsing it belongs with OOXML's naming of the same set rather
    /// than here. <c>text:start-numbering-at</c> asks for a per-page or per-chapter restart, and is recorded
    /// rather than resolved: which page a note lands on is what filling the page decides, so the rule is
    /// carried and the paginator applies it.
    /// </remarks>
    private static NoteNumbering NumberingIn(
        XElement? stylesRoot, string noteClass, NoteNumbering fallback)
    {
        XElement? styles = stylesRoot?.Element(XName.Get("styles", OdfNamespaces.Office));

        XElement? configuration = styles?
            .Elements(XName.Get("notes-configuration", OdfNamespaces.Text))
            .FirstOrDefault(element =>
                element.Attribute(XName.Get("note-class", OdfNamespaces.Text))?.Value == noteClass);

        if (configuration is null) return fallback;

        NoteNumberFormat format =
            NoteNumbering.Parse(
                configuration.Attribute(XName.Get("num-format", OdfNamespaces.Style))?.Value)
            ?? fallback.Format;

        // `text:start-value` is an *offset*, not the first note's number, which is measured rather than read
        // off the specification: a document stating 7 renders VIII and IX. LibreOffice maps the attribute to
        // `SwFootnoteInfo::nFootnoteOffset` and adds one when it numbers, so a reader taking the value as the
        // first number is out by one on every citation in the document.
        int start = int.TryParse(
            configuration.Attribute(XName.Get("start-value", OdfNamespaces.Text))?.Value,
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int offset)
            ? offset + 1
            : fallback.StartAt;

        // Where the class collects. ODF states this only for footnotes, as `text:footnotes-position`, whose
        // "document" is the same thing an endnote does by default; endnotes have no such attribute at all, so
        // theirs is always the class default. Carrying the fallback's placement through is not optional: a
        // document that declares a configuration purely to set a start value would otherwise have its
        // endnotes silently moved to the foot of the page, because a fresh NoteNumbering defaults there.
        NotePlacement placement =
            configuration.Attribute(XName.Get("footnotes-position", OdfNamespaces.Text))?.Value switch
            {
                "document" => NotePlacement.DocumentEnd,
                "page" => NotePlacement.PageBottom,
                _ => fallback.Placement,
            };

        NoteRestart restart =
            NoteNumbering.ParseRestart(
                configuration.Attribute(XName.Get("start-numbering-at", OdfNamespaces.Text))?.Value)
            ?? fallback.Restart;

        return new NoteNumbering(format, start) { Placement = placement, Restart = restart };
    }

    /// <summary>Which section each master page is, by name.</summary>
    private readonly IReadOnlyDictionary<string, int> _masterPages;

    /// <summary>
    /// The section the walk is in, which a paragraph naming a master page changes.
    /// </summary>
    /// <remarks>
    /// ODF has no section delimiter at all: a paragraph reaches its page description through its paragraph
    /// style's <c>style:master-page-name</c>, and a paragraph naming one <em>starts a page</em> on that
    /// master. Everything after it stays there until another paragraph names a different one — which is why
    /// this is state carried along the walk rather than a property of the paragraph.
    /// </remarks>
    private int _sectionIndex;

    /// <summary>
    /// Which master page the walk is on, which is <see cref="_sectionIndex"/> except inside a
    /// <c>text:section</c> that changed the geometry — see <see cref="EnterColumnSection"/>.
    /// </summary>
    private int _masterIndex;

    /// <summary>The <c>text:section</c> currently open, or null when the walk is not inside one.</summary>
    private OdfSectionGeometry? _columnGeometry;

    /// <summary>How many sections the caller states, which is where the derived ones are numbered from.</summary>
    private readonly int _sectionCount;

    /// <summary>The sections the walk derived from a <c>text:section</c>, in the order allocated.</summary>
    private readonly List<OdtColumnSection> _columnSections = [];

    /// <summary>
    /// The sections a <c>text:section</c> asked for, numbered from <see cref="_sectionCount"/> upwards.
    /// </summary>
    /// <remarks>
    /// Empty for every document that holds no <c>text:section</c> changing the geometry, which is nearly
    /// all of them. The caller turns each into a section derived from the master's own, because only the
    /// caller has the master's geometry; see <see cref="OdtWordDocument.Layout"/>.
    /// </remarks>
    internal IReadOnlyList<OdtColumnSection> ColumnSections => _columnSections;

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    /// <remarks>
    /// Worth surfacing rather than swallowing: a substitution that is not metric-compatible changes
    /// every line break after the first paragraph that uses it, so it is the first thing to check when a
    /// comparison against a reference renderer disagrees.
    /// </remarks>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>
    /// Reads the body's blocks — its paragraphs and its tables — in document order.
    /// </summary>
    /// <param name="body">The <c>office:text</c> element.</param>
    public List<PageBlock> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        _sectionIndex = 0;
        _masterIndex = 0;
        _columnGeometry = null;
        _columnSections.Clear();
        return WalkBlocks(body, isBody: true);
    }

    /// <summary>
    /// Reads a table cell's blocks, tables included.
    /// </summary>
    /// <remarks>
    /// Unlike <see cref="ReadFlow"/>, which is for a header or a footer, a cell keeps its tables: a table
    /// inside a cell is how a nested table is written, and <see cref="FlowLayouter"/> lays one out. The two
    /// differ only in the list they fill, which is what the generic walk is for.
    /// </remarks>
    /// <param name="element">The cell element.</param>
    public List<PageBlock> ReadCell(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);
        return WalkBlocks(element);
    }

    /// <summary>
    /// Reads a flow's blocks: a header's or a footer's.
    /// </summary>
    /// <remarks>
    /// The same walk a cell takes, tables included, because a table is how a two-part running head is usually
    /// laid out — one cell hard left, another hard right — and <see cref="FlowLayouter"/> places one either
    /// way. Dropping the table instead is not the harmless simplification it looks like: its paragraphs would
    /// stack as loose lines, giving the header a height no table has and pushing the body text down by the
    /// difference on every page.
    /// </remarks>
    /// <param name="element">The element whose block-level children to read.</param>
    public List<PageBlock> ReadFlow(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);
        return WalkBlocks(element);
    }

    /// <summary>
    /// Walks one element's block-level children, and settles any frame it found outside a paragraph.
    /// </summary>
    /// <remarks>
    /// The one entry point all three public walks share, because the pending list must not cross
    /// between them: a text box's own flow is read from inside <see cref="FramesOf"/>, so a frame the
    /// body left pending would otherwise be handed to the first paragraph <em>inside</em> a frame.
    /// Saved and restored rather than merely cleared, for the same reason.
    /// </remarks>
    /// <param name="element">The element whose block-level children to walk.</param>
    /// <param name="isBody">
    /// True only for the document body. A floating table is lifted out of its frame there and nowhere
    /// else, because <c>SwFlyFrame::IsFlySplitAllowed</c> refuses a fly in a header, a footer or a
    /// footnote outright (<c>sw/source/core/layout/fly.cxx</c>:702-721) — and those are exactly the flows
    /// the other two walks read.
    /// </param>
    private List<PageBlock> WalkBlocks(XElement element, bool isBody = false)
    {
        List<XElement>? outer = _looseFrames;
        bool outerBody = _isBody;
        _looseFrames = null;
        _isBody = isBody;

        try
        {
            List<PageBlock> blocks = [];
            Walk(element, blocks, depth: 0);

            // A body whose last element is a frame — or whose only paragraph came before it — still
            // has to draw it, so whatever is left over goes onto the last paragraph read. A walk that
            // found no paragraph at all drops it, which is the one case ODF gives nothing to hang on.
            if (_looseFrames is { Count: > 0 })
            {
                for (int at = blocks.Count - 1; at >= 0; at--)
                {
                    if (blocks[at] is not PageParagraph last) continue;

                    blocks[at] = WithLooseFrames(last);
                    break;
                }
            }

            return blocks;
        }
        finally
        {
            _looseFrames = outer;
            _isBody = outerBody;
        }
    }

    /// <summary>True while the walk is reading the document body rather than a flow or a cell.</summary>
    private bool _isBody;

    /// <summary>
    /// Whether the document lets a fly holding a table be broken at all — see
    /// <see cref="BreaksWrappedTables"/>. False leaves every such fly on the frame path, whole.
    /// </summary>
    private readonly bool _breaksWrappedTables;

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Sections, lists and the change-tracking wrappers are transparent: their children are body-level
    /// content and a walk that stopped at them would lose whole chapters of a real document.
    /// </para>
    /// <para>
    /// Generic in what it fills, which is how one walk serves both the body and a flow. A body takes
    /// <see cref="PageBlock"/> and so keeps the tables; a header, a footer or a cell takes
    /// <see cref="PageParagraph"/>, and a table simply does not fit in the list — so it is dropped by the
    /// type rather than by a flag that could be passed the wrong way round.
    /// </para>
    /// <para>
    /// A <c>text:h</c> is a paragraph as far as layout is concerned. Its outline level changes which
    /// style it resolves through, which the style chain already handles.
    /// </para>
    /// </remarks>
    private void Walk<T>(XElement element, List<T> into, int depth)
        where T : PageBlock
    {
        // Deep nesting is legal — a list inside a section inside a change region — but a file can nest
        // indefinitely, and this recurses on untrusted input.
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            string name = child.Name.LocalName;
            string ns = child.Name.NamespaceName;

            if (ns == OdfNamespaces.Text && name is "p" or "h")
            {
                // Before the paragraph, because a fly anchored in it is drawn where the paragraph starts
                // and the flow carries on past it: the table has to be in the block list at the anchor's
                // own position for `Paginator.PlaceFloatedTable` to measure it against the right place.
                if (into is List<PageBlock> body) FloatOutTables(child, body);

                if (Paragraph(child) is { } paragraph && WithLooseFrames(paragraph) is T block)
                {
                    into.Add(block);
                }

                continue;
            }

            // A frame at block level rather than inside a paragraph. ODF allows it for a page-anchored
            // shape, and LibreOffice writes it that way whenever a picture was placed by Draw's rules
            // rather than typed into the text — `emf-picture.odt`'s whole body is one such frame and an
            // empty paragraph. A frame belongs to a paragraph in this model, so it is held and given to
            // the next one; the anchor it carries is what decides where it actually lands, and for a page
            // anchor that is the page, not the paragraph.
            if (OdfFrames.IsFrame(child))
            {
                // A floating table at block level is the same object as one inside a paragraph and goes
                // in at the place the file put it, rather than waiting for a paragraph to hang from.
                if (into is List<PageBlock> hoisted && Floated(child, hoisted)) continue;

                (_looseFrames ??= []).Add(child);
                continue;
            }

            if (ns == OdfNamespaces.Text && name == "list")
            {
                // Transparent to the block list and not to the walk: the nesting *is* the level, so the
                // depth has to be counted here or every list item reads as a level-one item.
                (int, OdfListStyle?) outer = EnterList(child);
                Walk(child, into, depth + 1);
                LeaveList(outer);
                continue;
            }

            if (ns == OdfNamespaces.Text && name is "list-item" or "list-header")
            {
                BeginListItem(child, numbered: name == "list-item");
                Walk(child, into, depth + 1);
                continue;
            }

            if (ns == OdfNamespaces.Text && name == "section" && EnterColumnSection(child))
            {
                Walk(child, into, depth + 1);
                LeaveColumnSection();
                continue;
            }

            if (_isBody
                && ns == OdfNamespaces.Text
                && (name == "section" || IsIndex(name))
                && _columnGeometry is { } enclosing)
            {
                // A section inside a section leaves the enclosing one's columns and splits it — see
                // OdfSectionGeometry.Nested. An ODF index is a section like any other, which is why
                // this arm names ODF's seven index elements beside `section`.
                EnterNestedSection(child, enclosing);
                Walk(child, into, depth + 1);
                LeaveNestedSection(enclosing);
                continue;
            }

            if (ns == OdfNamespaces.Text
                && name is "section" or "index-body"
                    or "table-of-content" or "alphabetical-index" or "illustration-index"
                    or "table-index" or "object-index" or "user-index" or "bibliography"
                    or "tracked-changes" or "deletion" or "insertion")
            {
                // A deletion's content is inside a change region and is not part of the text a reader
                // sees, so it is skipped rather than walked — the same decision every one of X-Ray.Office's
                // extraction readers makes.
                if (name is not ("tracked-changes" or "deletion")) Walk(child, into, depth + 1);
                continue;
            }

            if (OdfNamespaces.IsTable(ns) && name == "table")
            {
                // A table goes in whole, as a grid. It is dropped when the caller is filling a flow's
                // paragraph list, which cannot hold one — a table inside a header or inside another table
                // is legal and not laid out yet.
                if (Table(child) is { } table && table is T grid) into.Add(grid);
                continue;
            }
        }
    }

    /// <summary>
    /// Lifts every floating table a paragraph anchors out of its frame and into the block list.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>draw:frame</c> holding nothing but a table, marked <c>loext:may-break-between-pages</c>, is
    /// the same object a DOCX spells <c>w:tblpPr</c> — see <see cref="OdfFrames.FloatingTable"/> — and
    /// this engine models that as a positioned block table because that is the model that <em>splits</em>.
    /// Left as a frame the table is measured whole and its tail is drawn below the sheet, which reads as
    /// missing text and is not.
    /// </para>
    /// <para>
    /// The walk deliberately does not descend into a <c>draw:</c> subtree: a frame inside another frame's
    /// text box belongs to that frame's own flow, and hoisting it into the body would move it to a
    /// different page as well as to a different rectangle.
    /// </para>
    /// </remarks>
    /// <param name="paragraph">The <c>text:p</c> or <c>text:h</c> about to be read.</param>
    /// <param name="into">The block list the tables go into, in the paragraph's own place.</param>
    private void FloatOutTables(XElement paragraph, List<PageBlock> into)
    {
        if (!_isBody) return;

        // The block-level frames waiting for this paragraph first, since the file listed them first —
        // and in document order, which is why this rebuilds the pending list rather than removing from
        // it backwards.
        if (_looseFrames is { Count: > 0 } loose)
        {
            List<XElement> kept = [];
            foreach (XElement pending in loose)
            {
                if (!Floated(pending, into)) kept.Add(pending);
            }

            _looseFrames = kept.Count == 0 ? null : kept;
        }

        Descend(paragraph, 0);

        void Descend(XElement element, int depth)
        {
            if (depth > 32) return;

            foreach (XElement child in element.Elements())
            {
                if (child.Name.NamespaceName == OdfNamespaces.Draw)
                {
                    Floated(child, into);
                    continue;
                }

                Descend(child, depth + 1);
            }
        }
    }

    /// <summary>
    /// Reads one frame as a positioned table and adds it, answering whether it was one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The frame is recorded in <see cref="_floatedFrames"/> so that <see cref="FramesOf"/> declines it:
    /// the fly and the block table are one object, and drawing both would draw the table twice.
    /// </para>
    /// <para>
    /// The fly's placement beats the table's own. A framed table states <c>table:align</c> and
    /// <c>fo:margin-left</c> of its own — inside the fly, where they mean where it sits in the fly — and
    /// the fly is what is positioned on the page, so those are replaced rather than added to.
    /// </para>
    /// </remarks>
    /// <param name="frame">The candidate frame element.</param>
    /// <param name="into">The block list to add to.</param>
    private bool Floated(XElement frame, List<PageBlock> into)
    {
        if (!_isBody || !_breaksWrappedTables) return false;
        if (OdfFrames.FloatingTable(frame, _styles) is not { } floating) return false;

        // A fly whose offset from its own anchor is negative starts *above* the paragraph it hangs on,
        // which is a place the flow has already passed — and a positioned block table is placed at
        // `used + VerticalOffset`, so the model would put it behind the text rather than over it. This is
        // a limit of the model stated where it bites rather than a rule of Writer's, which simply draws
        // the fly there. Measured on `HC-Bulletin-template`, whose second fly states
        // `svg:y="-0.078in"`: hoisted it is cut 18.9 pt above the body's bottom and the document comes
        // to six pages, and left in its frame it is five against the reference's five. Two of the
        // converted corpus's 51 floating tables state a negative offset against their paragraph.
        if (floating.VerticalOrigin == FrameVerticalOrigin.Paragraph
            && floating.VerticalOffset < Length.Zero)
        {
            return false;
        }

        if (Table(floating.Table) is not { } table) return false;

        bool aligned = floating.Horizontal != FrameHorizontalAlignment.Offset;

        into.Add(table with
        {
            IsPositioned = true,
            HorizontalPosition = aligned ? floating.Horizontal : null,

            // `from-left` is the one value that uses `svg:x`, and the coordinate is measured from the
            // sheet's own left edge under `style:horizontal-rel="page"` — the same distinction OOXML
            // draws with `w:horzAnchor="page"`, and the same correction `PositionedLeftEdge` applies
            // there. `PageTable.LeftWithin` works in the text area's coordinates, so the margin comes
            // off before the offset joins them.
            LeftIndent = aligned
                ? Length.Zero
                : floating.HorizontalOffset
                  - (floating.HorizontalFromSheet ? SectionLeftMargin : Length.Zero),
            VerticalOrigin = floating.VerticalOrigin,
            VerticalOffset = floating.VerticalOffset,
            LowerSpacing = floating.LowerSpacing,
        });

        _floatedFrames.Add(frame);
        return true;
    }

    /// <summary>
    /// The left margin of the section the walk is in, or nought when the caller stated no geometry.
    /// </summary>
    /// <remarks>
    /// The one piece of page geometry this reader carries, and it is carried for one reason: a fly
    /// positioned <c>style:horizontal-rel="page"</c> states its offset from the sheet's edge while every
    /// other length on the way to <see cref="PageTable"/> is measured from the text area's. Eight of the
    /// converted corpus's 51 floating tables state it.
    /// </remarks>
    private Length SectionLeftMargin
        => _sectionMargins.Count == 0
            ? Length.Zero
            : _sectionMargins[Math.Clamp(_masterIndex, 0, _sectionMargins.Count - 1)];

    /// <summary>
    /// The frames already read as positioned tables, which must not be read as frames as well.
    /// </summary>
    /// <remarks>
    /// By identity, since two frames of equal value are still two frames — the same reason
    /// <c>FrameResolution</c> keys its anchors that way.
    /// </remarks>
    private readonly HashSet<XElement> _floatedFrames = new(ReferenceEqualityComparer.Instance);

    /// <summary>
    /// Frames seen at block level, waiting for a paragraph to hang from.
    /// </summary>
    /// <remarks>
    /// Null rather than an empty list in the overwhelmingly common case, which is a document that has
    /// none: every frame typed into text is inside its <c>text:p</c> and never comes near this.
    /// </remarks>
    private List<XElement>? _looseFrames;

    /// <summary>
    /// A paragraph with any block-level frames seen before it attached to it.
    /// </summary>
    /// <remarks>
    /// Before the paragraph's own frames rather than after, which is z-order: the file listed them
    /// first. The pending list is cleared whether or not any of them read as a frame, so a shape this
    /// reader declines cannot attach itself to every later paragraph in the document.
    /// </remarks>
    private PageParagraph WithLooseFrames(PageParagraph paragraph)
    {
        if (_looseFrames is not { Count: > 0 } loose) return paragraph;

        List<PageFrame> frames = FramesOf([.. loose.Select(frame => new FrameAnchor(0, frame))]);
        _looseFrames = null;

        return frames.Count == 0
            ? paragraph
            : paragraph with { Frames = [.. frames, .. paragraph.Frames] };
    }

    /// <summary>
    /// Opens a <c>text:section</c> that changes the geometry, answering the section index to restore.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Answers null — and the walk then treats the element as transparent, exactly as it always has —
    /// for a section that states one column and no indents, and for one outside the body. A section in a
    /// header, a footer or a table cell is not laid out as its own frame here: the flow it belongs to has
    /// no page geometry of its own to change.
    /// </para>
    /// <para>
    /// A section met while one is already open is a <em>nested</em> section and goes through
    /// <see cref="EnterNestedSection"/> instead, which is a different rule rather than a refusal: it
    /// leaves the enclosing section's columns and splits it. Of 139 <c>text:section</c> elements in 49 of
    /// the 338 converted <c>.odt</c>, none is inside another and none is inside a table — but an ODF
    /// <em>index</em> is a section too, and one of those is inside a columned section.
    /// </para>
    /// </remarks>
    /// <param name="section">The <c>text:section</c> element.</param>
    /// <returns>True when the section was opened, and the walk must close it again.</returns>
    private bool EnterColumnSection(XElement section)
    {
        if (!_isBody || _columnGeometry is not null || _sectionCount == 0) return false;

        string? styleName = section.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;
        if (OdfSectionGeometry.Read(_styles, styleName) is not { } geometry) return false;

        _columnGeometry = geometry;
        _sectionIndex = AllocateColumnSection(geometry);
        return true;
    }

    /// <summary>
    /// Whether an element name is one of ODF's seven indexes, each of which is a section in Writer.
    /// </summary>
    /// <remarks>
    /// A <c>text:table-of-content</c> and its six siblings each import as a <c>SwSectionNode</c> — a
    /// <c>SwTOXBaseSection</c> is a <c>SwSection</c> — so the nesting rule
    /// <see cref="OdfSectionGeometry.Nested"/> describes reaches them exactly as it reaches a
    /// <c>text:section</c>. Counting <c>text:section</c> alone is what answers "nothing in the corpus is
    /// nested"; counting the indexes too finds the one document that is.
    /// </remarks>
    /// <param name="name">The element's local name, in the text namespace.</param>
    private static bool IsIndex(string name)
        => name is "table-of-content" or "alphabetical-index" or "illustration-index"
            or "table-index" or "object-index" or "user-index" or "bibliography";

    /// <summary>
    /// Opens a section nested inside one that is already open.
    /// </summary>
    /// <remarks>
    /// The nested section is a frame of its own beside the parent rather than inside its columns, so it
    /// gets a section of its own here — one column unless it states more, at the parent's own indents
    /// unless it states its own. See <see cref="OdfSectionGeometry.Nested"/> for the three seats.
    /// </remarks>
    /// <param name="section">The nested <c>text:section</c> or index element.</param>
    /// <param name="enclosing">The geometry of the section it is nested in.</param>
    private void EnterNestedSection(XElement section, OdfSectionGeometry enclosing)
    {
        string? styleName = section.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;
        OdfSectionGeometry nested = OdfSectionGeometry.Nested(_styles, styleName, enclosing);

        _columnGeometry = nested;
        _sectionIndex = AllocateColumnSection(nested);
    }

    /// <summary>
    /// Closes a nested section, putting the flow back into a fresh frame of the enclosing one.
    /// </summary>
    /// <remarks>
    /// A fresh one rather than the frame the nested section interrupted, because that is what Writer
    /// does: <c>SwSectionFrame::SplitSect</c> moves everything after the nested section into a second
    /// frame of the parent's format (<c>sw/source/core/layout/frmtool.cxx</c>:1954-1960), which balances
    /// its own columns and starts where the nested section left off.
    /// </remarks>
    /// <param name="enclosing">The geometry of the section the nested one interrupted.</param>
    private void LeaveNestedSection(OdfSectionGeometry enclosing)
    {
        _columnGeometry = enclosing;
        _sectionIndex = AllocateColumnSection(enclosing);
    }

    /// <summary>
    /// Closes the section <see cref="EnterColumnSection"/> opened, returning the flow to the page.
    /// </summary>
    /// <remarks>
    /// Through a section of its own rather than by restoring the outer index, because the outer index
    /// names a section whose break is <c>NextPage</c> — returning to it would start a page where the
    /// file merely ends a columned stretch. The restoring section is the master's geometry with a
    /// continuous break, which is what leaving a <c>SwSectionFrame</c> does.
    /// </remarks>
    private void LeaveColumnSection()
    {
        _columnGeometry = null;
        _sectionIndex = AllocateColumnSection(geometry: null);
    }

    /// <summary>
    /// Allocates a section derived from the master the walk is on, and answers its index.
    /// </summary>
    /// <param name="geometry">
    /// The <c>text:section</c>'s own geometry, or null for the section that restores the master's.
    /// </param>
    private int AllocateColumnSection(OdfSectionGeometry? geometry)
    {
        _columnSections.Add(new OdtColumnSection(_masterIndex, geometry));
        return _sectionCount + _columnSections.Count - 1;
    }

    /// <summary>
    /// The section a paragraph style's master page names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Followed up the parent chain, because a document's own styles are usually derived from
    /// <c>Standard</c> and it is the derived style that names the master. An empty name is not the same as
    /// an absent one — ODF writes <c>style:master-page-name=""</c> to mean "explicitly no master page, so
    /// no page break here", which is what an automatic style derived from one that names a master uses to
    /// cancel it. Cycle-guarded, since a style pool read from a file can point at itself.
    /// </remarks>
    private int? MasterPageOf(string? styleName)
    {
        if (_masterPages.Count == 0) return null;

        HashSet<string> seen = new(StringComparer.Ordinal);
        string? name = styleName;

        for (int depth = 0; depth < 64 && name is not null && seen.Add(name); depth++)
        {
            OdfStyle? style = _styles.Find(name, OdfStyleFamily.Paragraph);
            if (style is null) return null;

            if (style.MasterPageName is { } master)
            {
                return master.Length == 0 ? null : _masterPages.GetValueOrDefault(master, 0);
            }

            name = style.ParentStyleName;
        }

        return null;
    }

    /// <summary>
    /// Reads one paragraph, with an optional prefix its own text does not contain.
    /// </summary>
    /// <param name="element">The <c>text:p</c> or <c>text:h</c>.</param>
    /// <param name="prefix">
    /// Text to place before the paragraph's own, or null for none. Used for a footnote's citation at the
    /// start of the note's first paragraph, which LibreOffice draws there and the file does not contain.
    /// </param>
    /// <param name="prefixStyle">The character style the prefix takes.</param>
    private PageParagraph? Paragraph(
        XElement element, string? prefix = null, OdfStyleReference? prefixStyle = null)
    {
        string? styleName = element
            .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

        // A paragraph whose style names a master page moves the document onto that master, and everything
        // after it follows until another paragraph says otherwise. Inside a text:section the master
        // decides the geometry the section's own columns and indents are applied *to*, so a change of
        // master there needs a section of its own rather than throwing the open one away.
        if (MasterPageOf(styleName) is { } master && master != _masterIndex)
        {
            _masterIndex = master;
            _sectionIndex = _columnGeometry is { } open ? AllocateColumnSection(open) : master;
        }

        OdfTextStyle text = OdfParagraphFormats.ResolveText(_styles, styleName);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        // Taken before anything recurses, because a list item's label belongs to its first paragraph and
        // reading a note body or a text box below re-enters this method.
        bool wantsLabel = _labelPending;
        _labelPending = false;

        RunWalker walker = new(styleName, CitationOf, _footnoteNumber, _endnoteNumber);
        walker.Walk(element, prefix, prefixStyle);

        // Notes are numbered across the document, so the counters advance by however many this paragraph
        // cited — and the bodies are read after the walk, since reading one recurses into this method and
        // would otherwise renumber from the middle of the paragraph that cites it.
        _footnoteNumber += walker.FootnotesSeen;
        _endnoteNumber += walker.EndnotesSeen;

        (PageLabel? label, ParagraphFormat format) =
            ListFormatting(
                OdfParagraphFormats.Resolve(
                    _styles, styleName, _shrinksJustifiedBlanks, _tabsRelativeToIndent),
                text,
                face,
                wantsLabel);

        // The nested flows are read with the list suspended: a paragraph inside this one's footnote or
        // text box is not an item of the list this paragraph is in, and would otherwise take its indents.
        (int, OdfListStyle?) outerList = (_listLevel, _listStyle);
        (_listLevel, _listStyle) = (0, null);
        IReadOnlyList<PageNote> notes = NotesOf(walker.Notes);
        IReadOnlyList<PageFrame> frames = FramesOf(walker.Frames);
        (_listLevel, _listStyle) = outerList;

        // The runs first, then the text they map: `Apply` rewrites both together, and the offsets it
        // preserves are the ones the notes and frames above were recorded against.
        List<PageRun> runs = RunsOf(walker.Ranges, text, face);

        return new PageParagraph
        {
            SectionIndex = _sectionIndex,
            Text = CaseMapping.Apply(walker.Text, runs),
            Face = face,
            Font = _references.GetValueOrDefault(text.FaceKey),
            Colour = text.Colour ?? Colour.Transparent,
            Format = format,
            Label = label,
            EmSize = text.Size,
            Language = text.Language,
            WidthPerCent = text.WidthPerCent,
            Shaping = new ShapingOptions(
                Language: text.Language, DisableKerning: !text.AutoKerning),
            BlanksAreTransparentToHeight = _blanksAreTransparentToHeight,
            Fallback = _fonts,
            Runs = runs,
            Fields = walker.Fields,
            Notes = notes,
            Frames = frames,
            Anchors = walker.Anchors,
            OutlineLevel = OutlineLevelOf(element),
            Source = element,
        };
    }

    /// <summary>
    /// The outline level of a paragraph, 1 and up, or 0 when it is body text.
    /// </summary>
    /// <remarks>
    /// A <c>text:h</c> is a heading and states its level in <c>text:outline-level</c>, which defaults to 1
    /// when absent. A <c>text:p</c> is not one, however it is styled.
    /// </remarks>
    private static int OutlineLevelOf(XElement paragraph)
    {
        if (paragraph.Name.LocalName != "h") return 0;

        string? stated = paragraph.Attribute(XName.Get("outline-level", OdfNamespaces.Text))?.Value;
        return int.TryParse(stated, System.Globalization.NumberStyles.None, System.Globalization.CultureInfo.InvariantCulture, out int level) && level > 0
            ? level
            : 1;
    }

    /// <summary>How many footnotes the walk has passed, counted across the document.</summary>
    private int _footnoteNumber;

    /// <summary>
    /// The number the next endnote is cited by, counted separately from the footnotes.
    /// </summary>
    /// <remarks>
    /// Its own counter because the two sequences are independent — a document with two footnotes and two
    /// endnotes cites 1, 2, i and ii, not 1, 2, iii and iv — and because they are formatted differently.
    /// </remarks>
    private int _endnoteNumber;

    /// <summary>
    /// Reads each anchored note's body.
    /// </summary>
    /// <remarks>
    /// <para>
    /// After the citing paragraph's own walk rather than during it, because reading a note's body recurses
    /// into <see cref="Paragraph"/> — and doing that mid-walk would advance the note counter from inside the
    /// paragraph that cites the note, numbering a note in the body of note one as note two.
    /// </para>
    /// <para>
    /// The citation is placed at the start of the note's first paragraph, which is where LibreOffice draws
    /// it and where the file does not have it. An endnote is read the same way and marked, so that a caller
    /// need not guess later which of its notes collect at the end of the document.
    /// </para>
    /// </remarks>
    private List<PageNote> NotesOf(List<NoteAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageNote> notes = new(anchors.Count);

        foreach (NoteAnchor anchor in anchors)
        {
            XElement? body = anchor.Element.Element(XName.Get("note-body", OdfNamespaces.Text));
            if (body is null) continue;

            List<PageBlock> blocks = ReadNoteBody(body, anchor.Citation);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = anchor.Offset,
                IsEndnote = IsEndnote(anchor.Element),
                Placement = (IsEndnote(anchor.Element) ? _endnotes : _footnotes).Placement,
                Restart = (IsEndnote(anchor.Element) ? _endnotes : _footnotes).Restart,
                Numbering = IsEndnote(anchor.Element) ? _endnotes : _footnotes,

                // ODF's note body does not carry its own number at all, so the reader prepends it and the
                // offset is nought by construction.
                Citation = anchor.Citation,
                BodyOffset = 0,
            });
        }

        return notes;
    }

    /// <summary>
    /// Reads a note's body, putting the citation at the start of its first paragraph.
    /// </summary>
    /// <remarks>
    /// Its own walk rather than <see cref="ReadCell"/>'s, only because the first paragraph takes the
    /// citation and the rest do not — everything else about it is the same, tables included.
    /// </remarks>
    private List<PageBlock> ReadNoteBody(XElement body, string citation)
    {
        List<PageBlock> blocks = [];
        bool first = true;

        foreach (XElement child in body.Elements())
        {
            OfficeOperation.Poll();
            if (child.Name.Namespace == OdfNamespaces.Text
                && child.Name.LocalName is "p" or "h")
            {
                PageParagraph? paragraph = first
                    ? Paragraph(child, citation)
                    : Paragraph(child);

                if (paragraph is not null)
                {
                    blocks.Add(paragraph);
                    first = false;
                }

                continue;
            }

            Walk(child, blocks, depth: 0);
        }

        return blocks;
    }

    /// <summary>
    /// The paragraph's runs, or nothing when every one of them is the paragraph's own formatting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Returning an empty list for a uniform paragraph is not just an optimisation: it puts plain prose
    /// back on the single-face measuring path, which shapes the whole paragraph in one call rather than
    /// once per run. A run's boundaries also break shaping context, so a paragraph split into runs it does
    /// not need would lose a kern pair at each boundary and measure very slightly wide.
    /// </para>
    /// <para>
    /// A range whose font cannot be loaded falls back to the paragraph's face rather than being dropped:
    /// its text is still part of the paragraph, and losing it would silently shorten the document.
    /// </para>
    /// </remarks>
    private List<PageRun> RunsOf(
        IReadOnlyList<StyledRange> ranges, OdfTextStyle paragraph, OpenTypeFace paragraphFace)
    {
        List<PageRun> runs = new(ranges.Count);
        bool varies = false;
        FontReference? paragraphFont = _references.GetValueOrDefault(paragraph.FaceKey);

        foreach (StyledRange range in ranges)
        {
            OdfTextStyle style = range.Cascade.Length <= 1 ? paragraph : Resolve(range.Cascade);
            if (range.IsCitation) style = AsCitation(style);

            OpenTypeFace face = Face(style) ?? paragraphFace;

            // Resolved before the predicate rather than inside the constructor call below, because the
            // predicate needs it: `Face` is what fills `_references`, so the lookup is only valid here.
            FontReference? font = _references.GetValueOrDefault(style.FaceKey);

            // The escapement is resolved here rather than where it was read, because its rise is a fraction
            // of the face's height and the face is only known now.
            Core.Units.Length size = style.Escapement.SizeOf(style.Size);
            Core.Units.Length rise = style.Escapement.RiseOf(face, style.Size);

            if (face != paragraphFace
                || size != paragraph.Size
                || style.Colour != paragraph.Colour
                || style.Language != paragraph.Language
                || rise != Core.Units.Length.Zero
                // A case map has to survive the uniform-paragraph shortcut: it is the one property here
                // that changes the *characters*, so dropping the runs would draw the text as stored.
                || style.CaseMap != PageCaseMap.None
                // So does a highlight: the paragraph carries none of its own, so a paragraph highlighted
                // end to end is uniform by every other test and would lose its band entirely.
                || style.Highlight is { A: not 0 }
                // And so do the two rules, for the same reason: neither changes a width, so a paragraph
                // underlined end to end is uniform by every measurement test and would be drawn plain.
                || style.Underline != TextUnderline.None
                || style.IsStruckThrough
                // Kerning, unlike the two rules, does change a measurement — so a run that kerns
                // inside a paragraph that does not has to survive the shortcut or its width is the
                // paragraph's answer rather than its own.
                || style.AutoKerning != paragraph.AutoKerning
                // And a character width, for the same reason and more strongly: it multiplies every
                // advance in the run, so a run scaled inside an unscaled paragraph that was folded away
                // would be measured and broken at the paragraph's own width. Round 143 established on
                // the DOCX side that the measurement fallbacks rebuild a run from the paragraph and
                // carry no scale, so the fold is where it is lost.
                || style.WidthPerCent != paragraph.WidthPerCent
                // And a synthetic oblique, which is drawing-only in the same way and was the one
                // missing from this list: an italic run whose family has no italic installed resolves to
                // the *same* face as its upright neighbour, so nothing above can see it and the fold
                // would draw it upright. See PageRun.LeansDifferently.
                || PageRun.LeansDifferently(font, paragraphFont)
                // A link keeps its own run even when formatted like its neighbours: the run is what the
                // clickable area is cut from.
                || range.Link is not null)
            {
                varies = true;
            }

            runs.Add(new PageRun(
                range.Start,
                range.Length,
                face,
                size,
                font,
                style.Colour ?? paragraph.Colour ?? Colour.Transparent,
                new ShapingOptions(Language: style.Language, DisableKerning: !style.AutoKerning),
                rise,
                style.CaseMap,
                Highlight: style.Highlight ?? default,
                Underline: style.Underline,
                IsStruckThrough: style.IsStruckThrough,
                WidthPerCent: style.WidthPerCent,
                Link: range.Link));
        }

        return varies ? runs : [];
    }

    /// <summary>
    /// An <em>anchor</em> citation's style, defaulted to superscript when the document names none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice draws the number in the sentence in its built-in <c>Footnote Anchor</c> character style,
    /// which carries an automatic superscript and which it does not write into the file — a saved document
    /// names no style on <c>text:note-citation</c> at all, and LibreOffice's own round trip of a document
    /// that <em>did</em> name one drops it. So a reader applying only what the file states draws the citation
    /// full size and on the baseline, where it fuses with the word before it and pushes the rest of the line
    /// along by the difference.
    /// </para>
    /// <para>
    /// The <em>note body's</em> number is deliberately not defaulted the same way, and that asymmetry is
    /// measured rather than assumed: Writer's other built-in style, <c>Footnote Characters</c>, states nothing
    /// at all (<c>DocumentStylePoolManager.cxx</c> falls straight through for it), and LibreOffice duly draws
    /// the number at the head of a note full size on the note's own baseline. Only the anchor is raised.
    /// </para>
    /// </remarks>
    private static OdfTextStyle AsCitation(OdfTextStyle style)
        => style.Escapement.IsNone ? style with { Escapement = Escapement.Superscript } : style;

    /// <summary>The text style a cascade resolves to, cached because spans repeat their styles.</summary>
    private OdfTextStyle Resolve(OdfStyleReference[] cascade)
    {
        string key = string.Join('\n', cascade.Select(reference => reference.Name));
        if (_resolved.TryGetValue(key, out OdfTextStyle cached)) return cached;

        OdfTextStyle resolved = OdfParagraphFormats.ResolveText(_styles, cascade);
        _resolved[key] = resolved;
        return resolved;
    }

    /// <summary>
    /// A stretch of a paragraph's text and the style cascade in force over it.
    /// </summary>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="Cascade">
    /// The styles in force, outermost first: the paragraph style, then each enclosing span's.
    /// </param>
    /// <param name="IsCitation">
    /// True for a note's citation, which is drawn superscript even when the document names no style for it.
    /// </param>
    /// <param name="Link">
    /// Where the text leads when it is clicked: an address, <c>#name</c> for a bookmark in this document,
    /// or null for text that is not a link. Two ranges that differ only in this do not merge.
    /// </param>
    private readonly record struct StyledRange(
        int Start, int Length, OdfStyleReference[] Cascade, bool IsCitation = false, string? Link = null);

    /// <summary>A note found while walking a paragraph, before its body has been read.</summary>
    /// <param name="Offset">Where its citation sits in the paragraph's text.</param>
    /// <param name="Element">The <c>text:note</c> element, whose body is read separately.</param>
    /// <param name="Citation">The number it is cited by, counted across the document and already formatted.</param>
    private readonly record struct NoteAnchor(int Offset, XElement Element, string Citation);

    /// <summary>One floating frame in a paragraph, with the character offset it is anchored at.</summary>
    private readonly record struct FrameAnchor(int Offset, XElement Element);

    /// <summary>
    /// How deeply a frame's own text may hold further frames before the innermost is dropped.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input rather than a limit anyone meets: a text frame holds paragraphs, a
    /// paragraph holds frames, and a file claiming a hundred levels would read the same walk a hundred
    /// deep. Real documents nest one.
    /// </remarks>
    private const int MaxFrameNesting = 8;

    /// <summary>How many frames enclose the paragraph currently being read.</summary>
    private int _frameDepth;

    /// <summary>
    /// Reads the frames a paragraph anchors, with their own text laid out inside them.
    /// </summary>
    /// <remarks>
    /// A frame's content goes through <see cref="ReadFlow"/> — the same walk a header takes — so a frame
    /// containing a table or a list needs nothing of its own. That does mean the reader re-enters itself,
    /// which is why the depth is counted: the recursion is bounded by the document rather than by the
    /// stack.
    /// </remarks>
    private List<PageFrame> FramesOf(List<FrameAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageFrame> frames = [];

        foreach (FrameAnchor anchor in anchors)
        {
            // A floating table has already gone into the block list as a positioned table, and the fly
            // and the table are one object — see `Floated`.
            if (_floatedFrames.Contains(anchor.Element)) continue;

            Func<XElement, IReadOnlyList<PageBlock>>? content = _frameDepth < MaxFrameNesting
                ? Content
                : null;

            frames.AddRange(
                OdfFrames.ReadAll(anchor.Element, _styles, content, anchor.Offset, _pictures));
        }

        return frames;

        IReadOnlyList<PageBlock> Content(XElement box)
        {
            _frameDepth++;
            try
            {
                return ReadFlow(box);
            }
            finally
            {
                _frameDepth--;
            }
        }
    }

    /// <summary>True when a <c>text:note</c> is an endnote rather than a footnote.</summary>
    /// <remarks>
    /// <c>text:note-class</c>, whose only other value is <c>footnote</c> — and which is worth reading rather
    /// than inferring, because the two are numbered by different sequences in different formats and placed on
    /// different pages.
    /// </remarks>
    private static bool IsEndnote(XElement note)
        => note.Attribute(XName.Get("note-class", OdfNamespaces.Text))?.Value == "endnote";

    /// <summary>
    /// How a note of each class is cited, which is not the same for the two.
    /// </summary>
    /// <remarks>
    /// Two sequences in two formats, from the document's <c>text:notes-configuration</c> where it has one and
    /// from LibreOffice's own defaults where it does not — footnotes 1, 2, 3 and endnotes i, ii, iii, which is
    /// measured rather than assumed.
    /// </remarks>
    /// <param name="isEndnote">True for an endnote.</param>
    /// <param name="index">How many notes of the class came before, counted from zero.</param>
    private string CitationOf(bool isEndnote, int index)
        => (isEndnote ? _endnotes : _footnotes).Citation(index);

    /// <summary>
    /// Walks a paragraph, building its text and the ranges its spans divide it into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One walk for both, because the ranges are offsets into the text and the text is not a
    /// concatenation of the element's text nodes: ODF writes runs of spaces, tabs and line breaks as
    /// elements, so every one of them shifts the offsets of everything after it. Building the text first
    /// and the ranges from the tree afterwards would mean re-deriving those shifts.
    /// </para>
    /// <para>
    /// Adjacent stretches with the same cascade merge, which is what makes a paragraph split by a
    /// bookmark or a soft page break still measure as one run.
    /// </para>
    /// </remarks>
    private sealed class RunWalker
    {
        /// <summary>How deep a paragraph's inline nesting is followed.</summary>
        /// <remarks>
        /// Spans nest legitimately — a bold word inside an italic phrase inside a hyperlink — but a
        /// generated file can nest indefinitely, and this recurses on untrusted input.
        /// </remarks>
        private const int MaxDepth = 64;

        private readonly StringBuilder _builder = new();
        private readonly List<OdfStyleReference> _cascade = [];
        private readonly List<StyledRange> _ranges = [];
        private readonly List<NoteAnchor> _notes = [];
        private readonly List<FrameAnchor> _frames = [];
        private readonly List<PageAnchor> _anchors = [];

        /// <summary>The link the text being emitted is inside, or null.</summary>
        private string? _link;

        /// <summary>The bookmarks the paragraph holds, with the offsets they sit at.</summary>
        internal IReadOnlyList<PageAnchor> Anchors => _anchors;

        /// <summary>Creates a walker over a paragraph with a given style.</summary>
        /// <param name="paragraphStyleName">The paragraph's own style name, which roots the cascade.</param>
        /// <param name="citation">How a note of a class and an index is cited.</param>
        /// <param name="footnote">
        /// How many footnotes came before this paragraph. Passed in because notes are numbered across the
        /// document rather than within a paragraph, so the counters belong to the source.
        /// </param>
        /// <param name="endnote">How many endnotes came before it, counted separately.</param>
        internal RunWalker(
            string? paragraphStyleName,
            Func<bool, int, string> citation,
            int footnote = 0,
            int endnote = 0)
        {
            _cascade.Add(new OdfStyleReference(paragraphStyleName, OdfStyleFamily.Paragraph));
            _citationOf = citation;
            _footnote = footnote;
            _endnote = endnote;
        }

        /// <summary>How a note of a class and an index is cited, which the source resolves.</summary>
        /// <remarks>
        /// A delegate because the walker is nested but not owned: the numbering comes from the document's
        /// <c>text:notes-configuration</c>, which the source read, and a walker is built per paragraph.
        /// </remarks>
        private readonly Func<bool, int, string> _citationOf;

        private int _footnote;
        private int _endnote;

        /// <summary>How many footnotes this paragraph cited, which is what advances the source's counter.</summary>
        internal int FootnotesSeen { get; private set; }

        /// <summary>How many endnotes it cited.</summary>
        internal int EndnotesSeen { get; private set; }

        /// <summary>The paragraph's text.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>The notes anchored in the paragraph, with the offsets their citations occupy.</summary>
        internal List<NoteAnchor> Notes => _notes;

        /// <summary>The page-sensitive fields this paragraph carries, with the spans their results own.</summary>
        internal List<PageFieldSpan> Fields { get; } = [];

        /// <summary>The floating frames anchored in the paragraph, with the offsets they sit at.</summary>
        internal List<FrameAnchor> Frames => _frames;

        /// <summary>Walks a <c>text:p</c> or <c>text:h</c>.</summary>
        /// <param name="paragraph">The paragraph element.</param>
        /// <param name="prefix">
        /// Text to emit before the paragraph's own, or null for none. What a footnote's citation is at the
        /// start of the note's first paragraph — emitted here rather than spliced in afterwards, because a
        /// prefix added later would shift every run's offset and every note anchor by its length.
        /// </param>
        /// <param name="prefixStyle">The character style the prefix is drawn in, if any.</param>
        internal void Walk(
            XElement paragraph, string? prefix = null, OdfStyleReference? prefixStyle = null)
        {
            if (!string.IsNullOrEmpty(prefix))
            {
                bool pushed = prefixStyle is { } style && !string.IsNullOrEmpty(style.Name);
                if (pushed) _cascade.Add(prefixStyle!.Value);

                Emit(prefix);

                if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
            }

            Append(paragraph, depth: 0);
        }

        private void Append(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            foreach (XNode node in element.Nodes())
            {
                switch (node)
                {
                    case XText textNode:
                        Emit(textNode.Value);
                        break;

                    case XElement child when child.Name.NamespaceName == OdfNamespaces.Text:
                        AppendTextElement(child, depth);
                        break;

                    // A comment's anchor occupies a position and draws nothing; its body is another flow,
                    // and LibreOffice's own PDF export leaves it out of the page entirely.
                    //
                    // Matched here rather than beside the other anchors because a comment is
                    // `office:annotation` and not `text:annotation` — the one paragraph-level element ODF
                    // puts outside the text namespace. A case for it under `AppendTextElement` never
                    // fires, so the walk below descended into the comment and spliced its author, its
                    // timestamp and its body into the sentence it is anchored in. Silent in a packaged
                    // `.odt`, where the elements abut and the words fuse into their neighbours; visible in
                    // the same document saved flat, where the pretty-printer's newlines separate them.
                    case XElement child when child.Name.NamespaceName == OdfNamespaces.Office
                                             && child.Name.LocalName is "annotation" or "annotation-end":
                        Emit(AnchorCharacter.ToString());
                        break;

                    // A floating frame is *in* the paragraph and is not part of it: its own text belongs
                    // to a rectangle of its own, and walking into it would splice a caption into the
                    // middle of the sentence the frame is anchored in. Recorded with the offset it sits
                    // at, which is what a character anchor is measured in.
                    case XElement child when OdfFrames.IsFrame(child):
                        _frames.Add(new FrameAnchor(_builder.Length, child));
                        break;

                    case XElement child:
                        Append(child, depth + 1);
                        break;

                    default:
                        break;
                }
            }
        }

        /// <summary>
        /// Appends one <c>text:</c> element, which is where ODF hides most of a paragraph's characters.
        /// </summary>
        /// <remarks>
        /// <para>
        /// A reader that took the descendant text nodes alone would lose every run of spaces, every tab
        /// and every line break — and <c>text:s</c> in particular, which is how any run of two or more
        /// spaces is written.
        /// </para>
        /// <para>
        /// A note's body is inside the paragraph in the file but is not part of its text, so it
        /// contributes only the citation it occupies. Walking into it would put a footnote's whole text
        /// into the middle of the sentence that cites it. A comment's body is the same case in the
        /// <c>office:</c> namespace, and is handled by the caller for that reason.
        /// </para>
        /// </remarks>
        private void AppendTextElement(XElement child, int depth)
        {
            switch (child.Name.LocalName)
            {
                case "s":
                    // text:c is the count, and its absence means one space rather than none.
                    int count = 1;
                    if (child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value is { } value
                        && int.TryParse(value, out int declared)
                        && declared is > 0 and <= 4096)
                    {
                        count = declared;
                    }
                    Emit(new string(' ', count));
                    break;

                case "tab":
                    Emit("\t");
                    break;

                case "line-break":
                    // A line break within a paragraph, which layout has to honour as a forced break —
                    // recorded as the character so the break iterator sees it.
                    Emit(LineSeparator.ToString());
                    break;

                case "note":
                    // A footnote's citation is drawn *in the sentence*, as a superscript, and again at the
                    // start of the note itself. So the anchor is not a bare placeholder the way a comment's
                    // is: it carries the citation's own text, under the citation's own character style,
                    // which is what makes it small and raised.
                {
                    bool isEndnote = IsEndnote(child);
                    string citation = _citationOf(isEndnote, isEndnote ? _endnote : _footnote);

                    _notes.Add(new NoteAnchor(_builder.Length, child, citation));
                    Citation(child, citation);

                    if (isEndnote)
                    {
                        _endnote++;
                        EndnotesSeen++;
                    }
                    else
                    {
                        _footnote++;
                        FootnotesSeen++;
                    }

                    break;
                }

                // The two fields whose cached value describes the document the producer had rather than
                // the one being laid out. ODF states them as typed elements holding their own last
                // result, so the walk emits that result and records where it landed; PageFields replaces
                // it once the page is known. `text:select-page` picks a neighbouring page and is not
                // modelled — that field keeps its cached value rather than being given this page's.
                case "page-number" or "page-count":
                {
                    int at = _builder.Length;
                    Append(child, depth + 1);

                    bool previousOrNext =
                        child.Attribute(XName.Get("select-page", OdfNamespaces.Text))?.Value
                            is "previous" or "next";

                    if (!previousOrNext)
                    {
                        Fields.Add(new PageFieldSpan(
                            at,
                            _builder.Length - at,
                            child.Name.LocalName == "page-number"
                                ? PageFieldKind.PageNumber
                                : PageFieldKind.PageCount));
                    }

                    break;
                }

                // A bookmark and a reference mark are named places: what a link to `#name` goes to. Nothing
                // is drawn for them; their offsets are kept so a PDF can declare a destination there.
                case "bookmark" or "bookmark-start" or "reference-mark" or "reference-mark-start":
                    if (child.Attribute(XName.Get("name", OdfNamespaces.Text))?.Value is { Length: > 0 } mark)
                    {
                        _anchors.Add(new PageAnchor(_builder.Length, mark));
                    }

                    break;

                case "soft-page-break" or "bookmark-end" or "reference-mark-end"
                    or "change" or "change-start" or "change-end":
                    break;

                // The three elements that carry character formatting of their own. A hyperlink's is a
                // character style like any other, which is why a link is usually blue and underlined
                // without anything inside it saying so.
                case "span" or "a" or "ruby-base":
                    string? styleName = child
                        .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

                    bool pushed = !string.IsNullOrEmpty(styleName);
                    if (pushed) _cascade.Add(new OdfStyleReference(styleName, OdfStyleFamily.Text));

                    // `text:a` is the one of the three that leads somewhere. Its address is a URL, or a
                    // fragment (`#Heading1`, `#bookmark|outline`) that names a place in the document; the
                    // `|outline`, `|table` and similar suffixes LibreOffice adds to say what kind of place
                    // it is are not part of the name.
                    string? outerLink = _link;
                    if (child.Name.LocalName == "a"
                        && child.Attribute(XName.Get("href", OdfNamespaces.XLink))?.Value is { Length: > 0 } href)
                    {
                        _link = LinkAddress(href);
                    }

                    Append(child, depth + 1);

                    _link = outerLink;
                    if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
                    break;

                default:
                    Append(child, depth + 1);
                    break;
            }
        }

        /// <summary>The address an <c>xlink:href</c> names, with a place in this document spelled <c>#name</c>.</summary>
        private static string? LinkAddress(string href)
        {
            if (href.Length == 0 || href[0] != '#') return href;

            int bar = href.IndexOf('|', StringComparison.Ordinal);
            string name = bar > 0 ? href[1..bar] : href[1..];
            return name.Length == 0 ? null : "#" + name;
        }

        /// <summary>
        /// Emits a note's citation at the anchor, under the citation's own character style.
        /// </summary>
        /// <remarks>
        /// The citation's <em>text</em> comes from the caller rather than from the file: LibreOffice ignores
        /// <c>text:note-citation</c>'s content and numbers the notes itself in document order, so a document
        /// stating 2 and 5 renders 1 and 2. The style does come from the file, since that is what makes the
        /// citation superscript — and a document that declares none falls back to an automatic superscript,
        /// which is what LibreOffice's built-in <c>Footnote Anchor</c> style carries.
        /// </remarks>
        /// <param name="note">The <c>text:note</c> element, for its citation's style.</param>
        /// <param name="text">The number the citation stands for, counted rather than read.</param>
        private void Citation(XElement note, string text)
        {
            XElement? citation = note.Element(XName.Get("note-citation", OdfNamespaces.Text));
            if (citation is null) return;

            string? styleName = citation
                .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

            bool pushed = !string.IsNullOrEmpty(styleName);
            if (pushed) _cascade.Add(new OdfStyleReference(styleName, OdfStyleFamily.Text));

            _inCitation = true;
            Emit(text);
            _inCitation = false;

            if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
        }

        /// <summary>True while the citation's own text is being emitted.</summary>
        private bool _inCitation;

        /// <summary>Appends text under the cascade currently in force.</summary>
        private void Emit(string text)
        {
            if (text.Length == 0) return;

            _builder.Append(text);

            if (_ranges.Count > 0
                && _ranges[^1].IsCitation == _inCitation
                && string.Equals(_ranges[^1].Link, _link, StringComparison.Ordinal)
                && SameCascade(_ranges[^1].Cascade))
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, [.. _cascade], _inCitation, _link));
        }

        private bool SameCascade(OdfStyleReference[] other)
        {
            if (other.Length != _cascade.Count) return false;

            for (int i = 0; i < other.Length; i++)
            {
                if (other[i] != _cascade[i]) return false;
            }

            return true;
        }
    }

    /// <summary>
    /// The face a text style resolves to, cached per distinct request.
    /// </summary>
    /// <remarks>
    /// Cached because a document has a handful of fonts and thousands of paragraphs, and resolving one
    /// means walking the substitution chain and reading a font file. Null only when nothing at all could
    /// be loaded, which means the machine has no usable fonts rather than that the document is bad.
    /// </remarks>
    private OpenTypeFace? Face(OdfTextStyle text) => Face(text.FamilyName, text.Weight, text.IsItalic);

    /// <summary>
    /// The face one family request resolves to, through the same cache.
    /// </summary>
    /// <remarks>
    /// Taken apart from <see cref="OdfTextStyle"/> because a list level names a family of its own — a
    /// symbol font, for a bullet — while taking its weight and slant from the item's text.
    /// </remarks>
    private OpenTypeFace? Face(string? family, int weight, bool italic)
    {
        (string? Family, int Weight, bool Italic) key = (family, weight, italic);
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        try
        {
            FontReference reference = _fonts.Resolve(new FontRequest(
                family ?? string.Empty, weight, italic));

            OpenTypeFace face = _fonts.LoadOpenType(reference);
            _faces[key] = face;
            _references[key] = reference;
            return face;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A font that cannot be read is not a reason to lose the paragraph, but there is nothing to
            // measure it with either — so it is dropped and the caller sees a shorter document rather
            // than an exception from the middle of a layout.
            _faces[key] = null!;
            return null;
        }
    }
}
