using System.Globalization;
using System.Xml.Linq;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// The compatibility options in <c>settings.xml</c>, and which of them Paperless acts on.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:compat</c> holds around eighty flags, each naming a version of Word whose layout the
/// document wants reproduced. Almost all of them are inert in practice, and which ones are not
/// is a question of measurement rather than of reading the schema — so every flag listed here
/// was checked by rendering the same document with and without it and comparing where
/// LibreOffice put the text. What that showed, on LibreOffice 24.2 with Carlito and Liberation
/// Serif:
/// </para>
/// <list type="table">
///   <listheader><term>Flag</term><description>Measured effect</description></listheader>
///   <item>
///     <term><c>w:doNotExpandShiftReturn</c></term>
///     <description>
///     <b>Moves text, and is wired.</b> A justified line ended by a <c>w:br</c> stops being
///     stretched: the first line's last run goes from x = 538.75 pt, hard against the right
///     margin, to x = 154.0 pt. See <see cref="Layout.ManualBreakJustification"/>.
///     </description>
///   </item>
///   <item>
///     <term><c>w:doNotUseHTMLParagraphAutoSpacing</c></term>
///     <description>
///     <b>Moves text, and was already wired</b> as <c>PaginationOptions.CollapsesSpacing</c>:
///     it decides whether two paragraphs' spacings add or the larger wins, which is worth one
///     paragraph space per boundary and a page break within five pages.
///     </description>
///   </item>
///   <item>
///     <term><c>w:compatSetting name="compatibilityMode"</c></term>
///     <description>
///     <b>Moves text, and was already wired</b> in <c>DocxLayoutSource</c>: at 15 and above a
///     non-nested table's indent is measured to the cell's text rather than to its edge.
///     Measured as <em>inert</em> for the other thing LibreOffice hangs on it,
///     <c>JustifyLinesWithShrinking</c> — mode 14 and mode 15 lay four justified paragraphs out
///     identically, to the hundredth of a point.
///     </description>
///   </item>
///   <item>
///     <term><c>w:noLeading</c></term>
///     <description>
///     <b>Moves text and is not wired.</b> It turns off external leading, so every line loses
///     the face's line gap: measured with Liberation Serif at 11 pt, the first baseline goes
///     from 66.95 pt to 66.50 pt and every line after it moves by the same amount again. It is
///     unwired because the line gap enters the layout in <c>Paperless.Text</c>'s line-metric
///     derivation, which is shared by all four formats and by the other two families; suppressing
///     it needs a metric option there rather than anything a DOCX reader can do. Note that it is
///     invisible with Carlito, whose declared line gap is zero — which is why the first
///     measurement of it said "inert".
///     </description>
///   </item>
///   <item>
///     <term><c>w:suppressTopSpacing</c>, <c>w:suppressSpacingAtTopOfPage</c></term>
///     <description>
///     <b>Identified and inert.</b> They would drive
///     <c>PaginationOptions.KeepsSpacingAtTopOfPage</c>, but LibreOffice does not read either —
///     neither appears in <c>SettingsTable::lcl_sprm</c> — and measured on a twenty-paragraph
///     document with 20 pt of space-before, adding either changes nothing at all. Wiring them
///     would move Paperless away from the only reference there is.
///     </description>
///   </item>
///   <item>
///     <term><c>w:noColumnBalance</c></term>
///     <description>
///     <b>Identified and inert here.</b> LibreOffice reads it
///     (<c>PropertyMap.cxx</c>:903) but measured on a two-column section it renders the same
///     either way, because it already leaves the last section group unbalanced. This engine does
///     not balance columns at all, so there is nothing yet for the flag to turn off.
///     </description>
///   </item>
///   <item>
///     <term><c>w:usePrinterMetrics</c></term>
///     <description>
///     <b>Read and applied.</b> It was recorded here as inert on the grounds that headless
///     LibreOffice ignores it, and that is refuted by the importer:
///     <c>DomainMapper_Impl::ApplySettingsTable</c>
///     (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:10173) sets
///     <c>PrinterIndependentLayout::DISABLED</c> from it, which is the same state
///     <c>WW8Dop::fUsePrinterMetrics</c> puts a DOC into. So the metrics are rounded onto a
///     300 dpi grid, exactly as <c>DocReader</c> already does for the binary format.
///     </description>
///   </item>
///   <item>
///     <term><c>w:splitPgBreakAndParaMark</c>, <c>w:ulTrailSpace</c>,
///     <c>w:balanceSingleByteDoubleByteWidth</c>, <c>w:doNotVertAlignCellWithSp</c>,
///     <c>w:adjustLineHeightInTable</c>, <c>w:doNotBreakWrappedTables</c></term>
///     <description>
///     <b>Identified, unread.</b> Each needs a feature this engine does not have yet — a
///     paragraph mark that survives a page break, underline under a trailing blank, ideographic
///     space balancing, floating anchors in cells, a text grid, and floating tables. Reading
///     them now would mean storing a flag nothing can act on.
///     </description>
///   </item>
/// </list>
/// <para>
/// The document-level <c>w:widowControl</c> is deliberately not here: it is a child of
/// <c>w:settings</c> rather than of <c>w:compat</c>, and the per-paragraph <c>w:widowControl</c>
/// that carries the same meaning is already resolved through the style chain. Measured over ten
/// straddle positions of a six-line paragraph across a page break, adding the document-level flag
/// moved nothing in LibreOffice's output.
/// </para>
/// </remarks>
/// <param name="CompatibilityMode">
/// The Word version the file targets, from the <c>compatibilityMode</c> compatibility setting, or
/// <c>-1</c> when it states none. 15 is Word 2013 and after.
/// </param>
/// <param name="DoNotUseHtmlParagraphAutoSpacing">
/// True when consecutive paragraphs' spacings add rather than the larger winning.
/// </param>
/// <param name="DoNotExpandShiftReturn">
/// True when a justified line ended by a manual break is left ragged.
/// </param>
/// <param name="NoLeading">
/// True when external leading is suppressed. Read and reported; see the remarks for why nothing
/// consumes it.
/// </param>
/// <param name="UsesPrinterMetrics">
/// True when the document asks to be measured on a printer's pixel grid rather than
/// printer-independently, so every font metric is rounded to a 300 dpi step.
/// </param>
/// <param name="HasSettingsPart">
/// True when the package actually carried a <c>word/settings.xml</c>. Distinct from every flag
/// being off; <see cref="AddsParagraphSpacing"/> is the one place the difference shows.
/// </param>
public sealed record WordCompatibility(
    int CompatibilityMode,
    bool DoNotUseHtmlParagraphAutoSpacing,
    bool DoNotExpandShiftReturn,
    bool NoLeading,
    bool UsesPrinterMetrics,
    bool HasSettingsPart)
{
    /// <summary>What a package with no settings part gets: every flag off.</summary>
    public static WordCompatibility None { get; } = new(-1, false, false, false, false, false);

    /// <summary>
    /// True when two consecutive paragraphs' spacings add rather than the larger one winning.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>PARA_SPACE_MAX</c>, whose name says the opposite of what it does: set, the two
    /// spacings are summed (<c>SwFlowFrame::CalcUpperSpace</c>,
    /// <c>sw/source/core/layout/flowfrm.cxx</c>:1652); clear, the larger wins.
    /// </para>
    /// <para>
    /// The DOCX import writes it from <c>w:doNotUseHTMLParagraphAutoSpacing</c> — as
    /// <c>AddParaTableSpacing</c>, which <c>SwXDocumentSettings.cxx</c>:449 turns back into
    /// <c>PARA_SPACE_MAX</c> — so an ordinary Word document takes the larger, since Word's own
    /// exporter writes that element only when the flag is on.
    /// </para>
    /// <para>
    /// <b>Unless there is no settings part at all.</b> That write lives in
    /// <c>DomainMapper_Impl::ApplySettingsTable</c>, which returns at its first line when the
    /// document has no settings table (<c>DomainMapper_Impl.cxx</c>:10124), leaving the setting at
    /// the <em>application's</em> configured <c>AddSpacing</c>, whose shipped default is
    /// <c>true</c> (<c>officecfg/registry/schema/org/openoffice/Office/Compatibility.xcs</c>). So a
    /// package with no <c>word/settings.xml</c> adds where every other DOCX collapses.
    /// </para>
    /// <para>
    /// Measured on eight paragraphs each carrying 12 pt of space-before and 8 pt of space-after on
    /// 12 pt exact lines, so a boundary is 24 pt collapsed and 32 pt added. With a settings part —
    /// present but empty, or naming only <c>compatibilityMode</c> — LibreOffice 24.2.7.2 puts every
    /// boundary at 24.00 pt; with the part removed entirely, at 32.00 pt; and with the flag added
    /// back to a document that has the part, at 32.00 pt again. Those are
    /// <c>paragraph-spacing-settings.docx</c> and <c>paragraph-spacing-no-settings.docx</c>.
    /// </para>
    /// <para>
    /// Narrow, and worth saying so: <b>no document on the 200-file words corpus lacks a settings
    /// part</b>, so this changes nothing there. It is here because the case is cheap to get right
    /// and expensive to rediscover — a synthetic fixture written without the part is the standard
    /// way to measure this rule backwards, and doing so is what produced the claim that DOCX adds
    /// in general.
    /// </para>
    /// </remarks>
    public bool AddsParagraphSpacing => !HasSettingsPart || DoNotUseHtmlParagraphAutoSpacing;

    /// <summary>Reads the compatibility block out of a <c>w:settings</c> root.</summary>
    /// <param name="settings">The <c>w:settings</c> element, or null.</param>
    public static WordCompatibility Read(XElement? settings)
    {
        if (settings is null) return None;

        XElement? compat = Word.Child(settings, "compat");

        return new WordCompatibility(
            Mode(compat),
            Word.IsOn(Word.Child(compat, "doNotUseHTMLParagraphAutoSpacing")),
            Word.IsOn(Word.Child(compat, "doNotExpandShiftReturn")),
            Word.IsOn(Word.Child(compat, "noLeading")),
            Word.IsOn(Word.Child(compat, "usePrinterMetrics")),
            HasSettingsPart: true);
    }

    /// <summary>
    /// The <c>compatibilityMode</c> setting, or <c>-1</c>.
    /// </summary>
    /// <remarks>
    /// Not a flag but a numbered setting, and the only one in the block that is: Word writes it
    /// as a <c>w:compatSetting</c> whose <c>w:name</c> is <c>compatibilityMode</c> and whose
    /// <c>w:uri</c> is Word's own, alongside settings named for other producers. The name has to
    /// be matched rather than the position, since the order is not fixed.
    /// </remarks>
    private static int Mode(XElement? compat)
    {
        foreach (XElement setting in Word.Children(compat, "compatSetting"))
        {
            if (Word.Attribute(setting, "name") != "compatibilityMode") continue;

            if (int.TryParse(
                    Word.Attribute(setting, "val"), NumberStyles.Integer,
                    CultureInfo.InvariantCulture, out int mode))
            {
                return mode;
            }
        }

        return -1;
    }
}
