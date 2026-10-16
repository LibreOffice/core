using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Ooxml;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>What a WordprocessingML style may be applied to.</summary>
/// <remarks>
/// Part of a style's identity, not a description: a paragraph style and a character style may
/// share a <c>w:styleId</c>, and <c>w:basedOn</c> only resolves within one type.
/// </remarks>
public enum WordStyleType
{
    /// <summary>Not a type X-Ray.Office recognises.</summary>
    Unknown = 0,

    /// <summary>A paragraph style, referenced by <c>w:pStyle</c>.</summary>
    Paragraph,

    /// <summary>A character style, referenced by <c>w:rStyle</c>.</summary>
    Character,

    /// <summary>A table style, referenced by <c>w:tblStyle</c>.</summary>
    Table,

    /// <summary>A numbering style, referenced by <c>w:numStyleLink</c>.</summary>
    Numbering,
}

/// <summary>Where a resolved WordprocessingML property came from.</summary>
/// <remarks>
/// The same distinction <c>OdfPropertyOrigin</c> draws for ODF, and for the same reason: later
/// code has to be able to ask "was this set on the run itself, or merely inherited?" Collapsing
/// the cases into "has a value" loses that, and in DOCX it also makes the toggle-property rule
/// below impossible to implement, since that rule depends on <em>which layer</em> set a value.
/// </remarks>
public enum WordPropertyOrigin
{
    /// <summary>Nothing in the chain set the property.</summary>
    Unset = 0,

    /// <summary>Set as direct formatting, in the <c>w:rPr</c> or <c>w:pPr</c> of the object itself.</summary>
    Direct,

    /// <summary>Set on the style the object names.</summary>
    SetHere,

    /// <summary>Set on an ancestor reached through <c>w:basedOn</c>.</summary>
    Inherited,

    /// <summary>Came from <c>w:docDefaults</c>.</summary>
    Defaulted,
}

/// <summary>
/// One resolved WordprocessingML property.
/// </summary>
/// <param name="Element">
/// The property element itself — <c>w:b</c>, <c>w:sz</c>, <c>w:jc</c>. Kept whole rather than
/// reduced to its <c>w:val</c> because several properties carry more than one attribute
/// (<c>w:rFonts</c> names four scripts' fonts; <c>w:u</c> carries a style and a colour).
/// </param>
/// <param name="Origin">Which layer supplied it.</param>
/// <param name="SourceStyleId">The style that supplied it, for diagnostics.</param>
public readonly record struct WordProperty(
    XElement? Element,
    WordPropertyOrigin Origin,
    string? SourceStyleId = null)
{
    /// <summary>A property nothing set.</summary>
    public static WordProperty Unset => default;

    /// <summary>True when some layer supplied a value.</summary>
    public bool HasValue => Origin != WordPropertyOrigin.Unset;

    /// <summary>The <c>w:val</c> attribute, or null when the element carries none.</summary>
    public string? Value => Element?.Attribute(XName.Get("val", OoxmlNamespaces.WordprocessingML))?.Value;

    /// <summary>
    /// The property read as an on/off switch.
    /// </summary>
    /// <remarks>
    /// Presence means on. <c>w:val</c> may spell the state as <c>0</c>/<c>1</c>,
    /// <c>false</c>/<c>true</c> or <c>off</c>/<c>on</c> — all three appear in real files, and
    /// only accepting one of them turns bold off across a whole document.
    /// </remarks>
    public bool IsOn => Origin != WordPropertyOrigin.Unset && Value switch
    {
        null or "" => true,
        "0" or "false" or "off" => false,
        _ => true,
    };

    /// <summary>The <c>w:val</c> as an integer, or null.</summary>
    public int? IntegerValue
        => int.TryParse(Value, System.Globalization.NumberStyles.Integer,
                        System.Globalization.CultureInfo.InvariantCulture, out int parsed)
            ? parsed
            : null;
}

/// <summary>One <c>w:style</c> from <c>styles.xml</c>.</summary>
public sealed class WordStyle
{
    internal WordStyle(XElement element)
    {
        StyleId = Word.Attribute(element, "styleId") ?? string.Empty;
        Type = Word.Attribute(element, "type") switch
        {
            "paragraph" => WordStyleType.Paragraph,
            "character" => WordStyleType.Character,
            "table" => WordStyleType.Table,
            "numbering" => WordStyleType.Numbering,
            _ => WordStyleType.Unknown,
        };

        Name = Word.Value(element, "name");
        BasedOn = Word.Value(element, "basedOn");
        NextStyleId = Word.Value(element, "next");
        LinkedStyleId = Word.Value(element, "link");
        IsDefault = Word.Attribute(element, "default") is "1" or "true" or "on";
        IsCustom = Word.Attribute(element, "customStyle") is "1" or "true" or "on";

        ParagraphProperties = Word.Child(element, "pPr");
        RunProperties = Word.Child(element, "rPr");
        TableProperties = Word.Child(element, "tblPr");
        CellProperties = Word.Child(element, "tcPr");

        Dictionary<string, XElement> conditional = new(StringComparer.Ordinal);
        Dictionary<string, XElement> conditionalCells = new(StringComparer.Ordinal);
        foreach (XElement layer in Word.Children(element, "tblStylePr"))
        {
            if (Word.Attribute(layer, "type") is not { Length: > 0 } type) continue;

            // First wins, because a style stating the same layer twice is stating it once and then
            // contradicting itself, and Word keeps the first. Each half is indexed on its own: a layer
            // carrying only a `w:tcPr` is a real layer, and skipping the whole element when it has no
            // `w:rPr` — which is what stood here — is why a conditional cell shade was read by nothing.
            if (Word.Child(layer, "rPr") is { } runProperties) conditional.TryAdd(type, runProperties);
            if (Word.Child(layer, "tcPr") is { } cellProperties)
                conditionalCells.TryAdd(type, cellProperties);
        }

        ConditionalRunProperties = conditional;
        ConditionalCellProperties = conditionalCells;
    }

    /// <summary>The identifier content refers to. Not the user-visible name.</summary>
    public string StyleId { get; }

    /// <summary>What the style applies to.</summary>
    public WordStyleType Type { get; }

    /// <summary>
    /// The user-visible name from <c>w:name</c>, which is what a caller recognises: the style
    /// a user calls "Heading 1" has the id <c>Heading1</c>.
    /// </summary>
    public string? Name { get; }

    /// <summary>The parent style's id, or null at the top of the chain.</summary>
    public string? BasedOn { get; }

    /// <summary>The style to apply to the following paragraph.</summary>
    public string? NextStyleId { get; }

    /// <summary>The paired character or paragraph style, where the file links them.</summary>
    public string? LinkedStyleId { get; }

    /// <summary>Whether this is the default style for its type.</summary>
    public bool IsDefault { get; }

    /// <summary>
    /// Whether the file declares this a style of the author's rather than one of the application's.
    /// </summary>
    /// <remarks>
    /// <c>w:customStyle</c>. It separates a built-in style from one that merely wears a built-in
    /// <em>name</em>, and the two are not interchangeable: 26.2.4.2 voids the direct paragraph
    /// formatting of a paragraph in a built-in <c>heading N</c> named by a <c>TOC \t</c> switch and
    /// leaves a custom style of the same name alone. See <see cref="DocxTocStyles"/>.
    /// </remarks>
    public bool IsCustom { get; }

    /// <summary>The style's <c>w:pPr</c>, or null.</summary>
    public XElement? ParagraphProperties { get; private set; }

    /// <summary>The style's <c>w:rPr</c>, or null.</summary>
    public XElement? RunProperties { get; }

    /// <summary>
    /// A table style's conditional <c>w:rPr</c> layers, by the <c>w:tblStylePr w:type</c> that names
    /// each region. Empty on every other kind of style.
    /// </summary>
    /// <remarks>
    /// Indexed rather than kept as a list because a cell asks for named regions in a fixed order of
    /// specificity — see <see cref="WordTableStyleConditions.Names"/> — and never enumerates them.
    /// Layers carrying no <c>w:rPr</c> are absent from this dictionary and may still be present in
    /// <see cref="ConditionalCellProperties"/>.
    /// </remarks>
    public IReadOnlyDictionary<string, XElement> ConditionalRunProperties { get; }

    /// <summary>
    /// A table style's conditional <c>w:tcPr</c> layers, by the <c>w:tblStylePr w:type</c> that names
    /// each region. Empty on every other kind of style.
    /// </summary>
    /// <remarks>
    /// The other half of the same twelve layers, and the one a table's *shading* comes out of. Kept
    /// separately rather than storing the <c>w:tblStylePr</c> itself so that the two resolvers cannot
    /// accidentally read each other's element: a <c>w:shd</c> exists under both <c>w:rPr</c> and
    /// <c>w:tcPr</c> and means different things there — character shading against cell shading.
    /// </remarks>
    public IReadOnlyDictionary<string, XElement> ConditionalCellProperties { get; }

    /// <summary>
    /// A table style's <c>w:tblPr</c>, or null. Meaningless on any other kind of style.
    /// </summary>
    /// <remarks>
    /// Where <c>Table Grid</c> — the style Word puts on nearly every table — keeps the grid lines that
    /// make it a grid, and where a style states the cell margins that decide how tall a row is. Both are
    /// table-level properties that a table then overrides per element, which is why the chain is exposed
    /// rather than a resolved value.
    /// </remarks>
    public XElement? TableProperties { get; }

    /// <summary>
    /// A table style's unconditional <c>w:tcPr</c>, or null. Meaningless on any other kind of style.
    /// </summary>
    /// <remarks>
    /// <c>w:style/w:tcPr</c> — a sibling of <c>w:tblPr</c> and not a child of it, which is the reading
    /// a census of this got wrong first. It is the <c>wholeTable</c> layer's cell half by another
    /// spelling, and sixteen corpus cells take their fill from it.
    /// </remarks>
    public XElement? CellProperties { get; }

    /// <summary>
    /// Replaces the style's <c>w:pPr</c> with an equivalent that states one more attribute, for
    /// <see cref="WordStyles.CompleteOneSidedSpacing"/>.
    /// </summary>
    /// <remarks>
    /// A detached copy rather than an edit in place: the element belongs to the loaded part, and
    /// several readers walk that tree for their own purposes.
    /// </remarks>
    internal void ReplaceParagraphProperties(XElement replacement) => ParagraphProperties = replacement;
}

/// <summary>
/// The styles a DOCX declares, and the resolution rules over them.
/// </summary>
/// <remarks>
/// <para>
/// WordprocessingML layers formatting differently from ODF but needs the same discipline. The
/// layers for a run are, outermost first: <c>w:docDefaults</c>, the paragraph style's
/// <c>w:rPr</c> through its <c>w:basedOn</c> chain, the character style's <c>w:rPr</c> through
/// its chain, and finally the run's own <c>w:rPr</c> as direct formatting.
/// </para>
/// <para>
/// <strong>Toggle properties do not simply override.</strong> ECMA-376 §17.7.3 makes bold,
/// italic, caps, strike and a handful of others behave as toggles: when the paragraph-style
/// layer and the character-style layer both turn one on, the result is <em>off</em>. This is
/// why bold text inside a bold heading style comes out unbolded, and it is the single most
/// common way a DOCX reader gets emphasis wrong. Direct formatting is absolute and does not
/// participate in the toggle — which is why Word writes an explicit <c>w:b w:val="0"</c> when
/// a user unbolds text rather than relying on the XOR.
/// </para>
/// </remarks>
public sealed class WordStyles
{
    /// <summary>
    /// How far a <c>w:basedOn</c> chain is followed before it is treated as circular.
    /// </summary>
    /// <remarks>
    /// A cycle is illegal but does occur, and this is recursion over untrusted input.
    /// </remarks>
    public const int MaxBasedOnDepth = 64;

    /// <summary>
    /// The properties ECMA-376 §17.7.3 defines as toggles.
    /// </summary>
    /// <remarks>
    /// Enumerated rather than guessed: getting the membership wrong either makes ordinary
    /// properties cancel each other out or lets a genuine toggle stack.
    /// </remarks>
    public static readonly IReadOnlySet<string> ToggleProperties =
        new HashSet<string>(StringComparer.Ordinal)
        {
            "b", "bCs", "caps", "emboss", "i", "iCs", "imprint", "outline", "shadow",
            "smallCaps", "strike", "vanish",
        };

    private readonly Dictionary<(WordStyleType Type, string Id), WordStyle> _styles = [];
    private readonly Dictionary<WordStyleType, string> _defaults = [];

    /// <summary>Every style declared, in no guaranteed order.</summary>
    public IReadOnlyCollection<WordStyle> All => _styles.Values;

    /// <summary>The <c>w:rPr</c> from <c>w:docDefaults</c>, or null.</summary>
    public XElement? DefaultRunProperties { get; private set; }

    /// <summary>
    /// True when <c>w:docDefaults</c> declared a <c>w:rPrDefault</c> at all, however empty.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The run-side twin of <see cref="HasDefaultParagraphPropertiesElement"/>, and load-bearing for
    /// the same reason: presence rather than content decides. A DOCX import starts at
    /// <b>Calibri 11 pt</b> — <c>DomainMapper::DomainMapper</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:182-193, tdf#108350), <em>"In Word
    /// since version 2007, the default document font is Calibri 11 pt"</em> — and the moment a
    /// <c>w:rPrDefault</c> is seen at all, <c>StyleSheetTable::applyDefaults(false)</c> resets the
    /// document defaults to <b>Times New Roman 10 pt</b> and lays the file's own values over them
    /// (<c>StyleSheetTable.cxx</c>:2161-2180 for the family, and the 10 pt at :341-350 where
    /// <c>m_pDefaultCharProps</c> is constructed).
    /// </para>
    /// <para>
    /// So an empty <c>&lt;w:rPrDefault/&gt;</c> and a missing one give different documents, and the
    /// difference is a whole point of font size and a face whose line box is
    /// 2500/2048 of the em against 2355/2048.
    /// </para>
    /// </remarks>
    public bool HasDefaultRunPropertiesElement { get; private set; }

    /// <summary>The <c>w:pPr</c> from <c>w:docDefaults</c>, or null.</summary>
    public XElement? DefaultParagraphProperties { get; private set; }

    /// <summary>
    /// True when <c>w:docDefaults</c> declared a <c>w:pPrDefault</c> at all, however empty.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Distinct from <see cref="DefaultParagraphProperties"/> being non-null, and the difference is
    /// load-bearing: Word writes a bare <c>&lt;w:pPrDefault/&gt;</c> constantly, which carries no
    /// <c>w:pPr</c> and so leaves that property null while still being *present*.
    /// </para>
    /// <para>
    /// LibreOffice hangs a document-wide default on exactly that presence. <c>applyDefaults(true)</c>
    /// runs only from the <c>w:pPrDefault</c> arm of <c>StyleSheetTable::sprm</c>
    /// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:653-670), and it puts
    /// <c>ParaWidows</c> and <c>ParaOrphans</c> at 2 on the built-in style every other style
    /// inherits from — <em>"WARNING: these defaults only take effect IF there is a DocDefaults style
    /// section. Normally there is, but not always."</em> A package with no <c>w:pPrDefault</c> keeps
    /// Writer's own default of nought and gets no widow or orphan control whatever.
    /// </para>
    /// <para>
    /// The citation is a hypothesis and the probes are the evidence:
    /// <c>dotnet/probes/words-r46/widow-orphan-default.py</c> authors nine variants at five straddle
    /// positions, with a control variant that states <c>w:widowControl w:val="0"</c> on the straddling
    /// paragraph so the room at the foot of the page is measured rather than assumed.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, words-r62 — the same probe re-run unchanged against
    /// <b>26.2.4.2</b>, 45 renderings, and the answer is the one measured on 24.2.7.2. The
    /// discriminating pair is <c>no-pPrDefault</c> against <c>empty-pPrDefault</c>: at 14 and at 16
    /// fillers the first puts 3 and 1 target lines on page one — the same as <c>para-off</c>, which is
    /// what fits — and the second puts 2 and 0, the same as <c>para-on</c>. Presence alone, with no
    /// <c>w:pPr</c> inside it, turns the control on. Three further arms say it is a default and not an
    /// override: a <c>w:pPrDefault</c> that <em>states</em> <c>w:widowControl w:val="0"</c> is off, a
    /// paragraph stating <c>w:val="0"</c> under an empty <c>w:pPrDefault</c> is off, and
    /// <c>w:docDefaults</c> removed entirely is off. <c>settings-on</c> — a document-level
    /// <c>w:settings/w:widowControl</c> — is <b>inert at every filler count</b>, which reconfirms
    /// HANDOVER §7's refutation of it on the current binary.]
    /// </para>
    /// </remarks>
    public bool HasDefaultParagraphPropertiesElement { get; private set; }

    /// <summary>Reads a <c>styles.xml</c> root element.</summary>
    /// <param name="root">The <c>w:styles</c> element.</param>
    /// <param name="diagnostics">Receives malformed declarations, if given.</param>
    public void Add(XElement root, IList<Diagnostic>? diagnostics = null)
    {
        ArgumentNullException.ThrowIfNull(root);

        if (Word.Child(root, "docDefaults") is { } docDefaults)
        {
            if (Word.Child(docDefaults, "rPrDefault") is { } runDefault)
            {
                HasDefaultRunPropertiesElement = true;
                DefaultRunProperties = Word.Child(runDefault, "rPr");
            }

            if (Word.Child(docDefaults, "pPrDefault") is { } paragraphDefault)
            {
                HasDefaultParagraphPropertiesElement = true;
                DefaultParagraphProperties = Word.Child(paragraphDefault, "pPr");
            }
        }

        List<WordStyle> declared = [];

        foreach (XElement element in Word.Children(root, "style"))
        {
            WordStyle style = new(element);
            if (style.StyleId.Length == 0)
            {
                diagnostics?.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2101",
                    "A w:style has no w:styleId and cannot be referenced; ignoring it."));
                continue;
            }

            // A style with no w:name is dropped outright on a DOCX import, so nothing can reference
            // it — not by w:pStyle, w:rStyle, w:tblStyle or w:basedOn. StyleSheetTable::sprm
            // (sw/source/writerfilter/dmapper/StyleSheetTable.cxx:774) appends the finished entry to
            // neither the style table nor its identifier map unless
            // `!IsOOXMLImport() || !m_sStyleName.isEmpty()`, and this class reads OOXML only.
            //
            // The citation is the hypothesis; `dotnet/probes/words-r47/unnamed-style.py` is the
            // evidence. It authors a table, paragraph and character style stating w:sz 20 and
            // applies each one, with and without a w:name: LibreOffice draws 10 pt with the name and
            // 12 pt — the document default — without it, in all three families. The corpus half is a
            // causal mutation of `template---tpr-technical-progress-report-with-guidance.docx`,
            // sixteen of whose fifty-three styles carry a w:styleId and no w:name: adding
            // `<w:name w:val="Table1"/>` to the one its tables name changes the reference's own
            // output, and changing that style's own w:spacing changes nothing at all.
            //
            // Not a leniency question. A file stating a style it cannot name is malformed, and both
            // readings are defensible — this one is what the reference does, and matching it is the
            // whole point.
            if (style.Name is not { Length: > 0 })
            {
                diagnostics?.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2102",
                    $"w:style '{style.StyleId}' has no w:name; Word processors drop such a style "
                    + "entirely, so it is ignored here too."));
                continue;
            }

            _styles[(style.Type, style.StyleId)] = style;
            if (style.IsDefault) _defaults[style.Type] = style.StyleId;
            if (style.Type == WordStyleType.Paragraph) declared.Add(style);
        }

        CompleteOneSidedSpacing(declared);
        DropForwardInheritedContextualSpacing(declared);
    }

    /// <summary>
    /// Turns <c>w:contextualSpacing</c> off on a paragraph style that would only have inherited it
    /// from a parent declared <em>after</em> it, and that sets a margin of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same read-modify-write <see cref="CompleteOneSidedSpacing"/> models, seen from its other
    /// side. LibreOffice keeps both vertical margins <em>and</em> the contextual flag in one item —
    /// <c>SvxULSpaceItem</c>, whose <c>bContext</c> is what <c>SwFlowFrame::CalcUpperSpace</c> reads
    /// as <c>GetULSpace().GetContext()</c> (<c>sw/source/core/layout/flowfrm.cxx</c>:1533, 1620-1627)
    /// — while writerfilter sets the three through separate UNO properties. Setting a margin on a
    /// style therefore reads the item as it resolves <em>at that moment</em>, changes one field and
    /// puts the whole item back on the style, where it shadows the parent's. A parent declared later
    /// has contributed nothing yet, so its contextual flag never reaches the child.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 with one authored document rendered twice, differing only in where the
    /// parent style sits in <c>word/styles.xml</c> — three paragraphs per style, 10 pt, so a
    /// suppressed gap is a pitch of 11.50 pt and an honoured <c>w:after="240"</c> is 23.50
    /// (<c>probes/words-firstpage-r70/</c>, <c>ctx-lpfirst</c> and <c>ctx-lplast</c>):
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
    /// <c>w:basedOn</c> link itself is intact: the child takes the parent's 1440-twip indent under
    /// both orders, so this is one item's flag and not style inheritance failing.
    /// </para>
    /// <para>
    /// <strong>Reach: one corpus document.</strong> Censused over all 272 DOCX-family files for a
    /// paragraph style that sets a <c>w:spacing</c> margin, does not state
    /// <c>w:contextualSpacing</c>, and has an ancestor that does and is declared later —
    /// <c>PES-Technical-Report-Template_Jan_2019.docx</c> alone, whose <c>Heading9</c> (its bullet
    /// style, declared tenth) is based on <c>ListParagraph</c> (declared 114th). Suppressing the
    /// 12 pt between its bullets fits an extra bullet on several pages and costs the document the
    /// fourteenth page the reference has.
    /// </para>
    /// </remarks>
    /// <param name="declared">The paragraph styles of one <c>w:styles</c>, in declaration order.</param>
    private static void DropForwardInheritedContextualSpacing(List<WordStyle> declared)
    {
        Dictionary<string, int> position = new(StringComparer.Ordinal);
        Dictionary<string, WordStyle> byId = new(StringComparer.Ordinal);
        for (int i = 0; i < declared.Count; i++)
        {
            position.TryAdd(declared[i].StyleId, i);
            byId.TryAdd(declared[i].StyleId, declared[i]);
        }

        for (int i = 0; i < declared.Count; i++)
        {
            WordStyle style = declared[i];
            if (style.ParagraphProperties is not { } properties) continue;

            // A style stating the flag itself settles it, whichever way; nothing is inherited.
            if (Word.Child(properties, "contextualSpacing") is not null) continue;

            // Without a margin of its own the style has no item of its own, so it inherits the
            // parent's whole — flag included — however the two were ordered.
            if (Word.Child(properties, "spacing") is not { } spacing) continue;
            if (Word.Attribute(spacing, "before") is null
                && Word.Attribute(spacing, "beforeAutospacing") is null
                && Word.Attribute(spacing, "after") is null
                && Word.Attribute(spacing, "afterAutospacing") is null)
            {
                continue;
            }

            if (style.BasedOn is not { Length: > 0 } parentId) continue;

            // A parent already read is an ordinary inheritance: the read-modify-write picks up the
            // flag exactly as the layering would.
            if (position.TryGetValue(parentId, out int parentAt) && parentAt < i) continue;

            if (!InheritsContextualSpacingOn(byId, parentId)) continue;

            XElement replacement = new(properties);
            replacement.Add(new XElement(
                Word.Name("contextualSpacing"), new XAttribute(Word.Name("val"), "0")));
            style.ReplaceParagraphProperties(replacement);
        }
    }

    /// <summary>
    /// Whether the style chain starting at <paramref name="styleId"/> turns
    /// <c>w:contextualSpacing</c> on — the flag the style below it would otherwise have inherited.
    /// </summary>
    /// <param name="byId">The declared paragraph styles, by id.</param>
    /// <param name="styleId">Where to start the walk.</param>
    private static bool InheritsContextualSpacingOn(
        Dictionary<string, WordStyle> byId, string? styleId)
    {
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; depth < MaxBasedOnDepth; depth++)
        {
            if (styleId is not { Length: > 0 }) return false;
            if (!byId.TryGetValue(styleId, out WordStyle? style)) return false;
            if (!visited.Add(styleId)) return false;

            if (Word.Child(style.ParagraphProperties, "contextualSpacing") is { } stated)
            {
                return Word.IsOn(stated);
            }

            styleId = style.BasedOn;
        }

        return false;
    }

    /// <summary>
    /// Gives a paragraph style that states one of <c>w:spacing/@w:before</c> and
    /// <c>@w:after</c> a value for the other, when its parent has not been read yet.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice keeps a paragraph's two vertical margins in <em>one</em> item,
    /// <c>SvxULSpaceItem</c>, while writerfilter sets them through two separate UNO properties.
    /// Setting one is therefore a read-modify-write of the pair: the importer takes whatever the
    /// style resolves to at that moment, replaces the half the file states, and writes both back
    /// as <em>direct</em> values. So the unstated half stops being inherited and freezes at
    /// whatever the parent chain happened to hold — and styles are applied in the order
    /// <c>styles.xml</c> declares them, so "at that moment" means <em>before</em> a parent
    /// declared further down has had its own definition applied. What the parent still holds
    /// there is Writer's pool default for the built-in style its <c>w:name</c> maps onto.
    /// </para>
    /// <para>
    /// Measured on LibreOffice 24.2.7.2 rather than inferred, with the parent stating
    /// <c>w:before="480"</c> as a control: a child stating only <c>w:after</c> and based on a
    /// <c>heading 2</c> declared <em>after</em> it gets 12 pt above and never sees the 480; the
    /// same file with the parent declared <em>first</em> gets the 480. A custom parent gives
    /// zero, which is a suppression rather than a no-op for exactly the same reason.
    /// </para>
    /// <para>
    /// <strong>An unrecognised parent is not the end of it.</strong> "Nought when Writer knows no
    /// style of the parent's name" was fitted to a case that cannot test it — that probe's child
    /// was a <c>heading 1</c> and its parent a <c>heading 2</c>, so the parent always answered —
    /// and it is wrong wherever the child is itself one of Writer's headings. Then the style is
    /// <em>found</em> in Writer's pool rather than created, with Writer's own hierarchy still
    /// under it, and what it reads is its <c>Heading</c> base: 12 pt above, 6 pt below.
    /// </para>
    /// <para>
    /// So the order is parent first, the style's own name only as a fallback. Measured on the
    /// installed 26.2.4.2 by
    /// <c>dotnet/probes/words-pagination-01/one-sided-spacing-source.py</c>, over four name
    /// pairings times both declaration orders, reading <c>fo:margin-bottom</c> straight out of
    /// <c>--convert-to fodt</c>. The child states <c>w:before="480"</c>, so "mirror the stated
    /// value" would read 480 and is refuted outright:
    /// </para>
    /// <code>
    ///   child          parent         child declared first    parent declared first
    ///   heading 4      heading 2      120  (6 pt)             360  (the parent's own w:after)
    ///   heading 4      Custom Par     120                     360
    ///   Custom Kid     heading 2      120                     360
    ///   Custom Kid     Custom Par       0                     360
    /// </code>
    /// <para>
    /// The one cell that moves is row two, and it is the one four corpus documents sit in — both
    /// FAA Holdover Tables and <c>EHEST-SMS-Safety-Management-Manual-V2</c> have a
    /// <c>heading 4</c> based on a custom parent declared far later and stating only
    /// <c>w:before</c>, and <c>03_Technical_Report_(progress)_template</c> has a <c>heading 1</c>
    /// based on a <c>Body Text 2</c>, which Writer has no pool style for either. LibreOffice
    /// resolves all four to 6 pt below where we resolved nought. For the Holdover pair that 6 pt
    /// per NOTES heading, 214 of them, is the whole of their shared 13-page deficit.
    /// </para>
    /// <para>
    /// <strong>Which names answer from the style's own end is narrower than which answer from the
    /// parent's</strong>, and the same probe measures it over fifteen: only Heading 1-9, Title
    /// and Subtitle do. <c>Caption</c>, <c>List</c>, <c>Quote</c> and <c>Body Text</c> read
    /// nought from that end while still reading 120, 140 and 140 as a parent — see
    /// <see cref="WriterPoolSpacing.TryForOwnName"/>. That asymmetry is why the two ends need two
    /// tables rather than one.
    /// </para>
    /// <para>
    /// The whole change is therefore <em>additive</em>: it fires only where the old reading
    /// produced nought, so every document whose parent Writer does recognise —
    /// <c>Press release_EUREKA labels ITEA 3 Cluster</c> and that same
    /// <c>03_Technical_Report</c>'s <c>heading 2</c>, both over a <c>Body Text</c> — keeps the
    /// answer it had, and LibreOffice's own import agrees with both of those at nought above and
    /// 140 below respectively.
    /// </para>
    /// <para>
    /// What is <em>not</em> measured here is a chain longer than one link — a custom child based
    /// on a custom parent based on a <c>heading 2</c>. No corpus document takes that path and the
    /// probe does not cover it, so the walk stops at the immediate parent.
    /// </para>
    /// <para>
    /// This is what puts a 12 pt space above every <c>Heading1</c> of
    /// <c>final-technical-report-template.docx</c>, whose style states only <c>w:after="240"</c>
    /// and is based on its own <c>Heading2</c> — five headings' worth of page, and the sixth page
    /// the reference has and we did not.
    /// </para>
    /// </remarks>
    /// <param name="declared">The paragraph styles of one <c>w:styles</c>, in declaration order.</param>
    private static void CompleteOneSidedSpacing(List<WordStyle> declared)
    {
        Dictionary<string, int> position = new(StringComparer.Ordinal);
        Dictionary<string, WordStyle> byId = new(StringComparer.Ordinal);
        for (int i = 0; i < declared.Count; i++)
        {
            position.TryAdd(declared[i].StyleId, i);
            byId.TryAdd(declared[i].StyleId, declared[i]);
        }

        for (int i = 0; i < declared.Count; i++)
        {
            WordStyle style = declared[i];
            if (style.BasedOn is not { Length: > 0 } parentId) continue;
            if (Word.Child(style.ParagraphProperties, "spacing") is not { } spacing) continue;

            bool before = Word.Attribute(spacing, "before") is not null
                          || Word.Attribute(spacing, "beforeAutospacing") is not null;
            bool after = Word.Attribute(spacing, "after") is not null
                         || Word.Attribute(spacing, "afterAutospacing") is not null;
            if (before == after) continue;

            // A parent already read is an ordinary inheritance and needs nothing done to it: the
            // read-modify-write picks up the same value the layering would.
            if (position.TryGetValue(parentId, out int parentAt) && parentAt < i) continue;

            // An undeclared parent is a different case again — writerfilter never rewrites the
            // parent link at all, so the style keeps Writer's own parent for *its* built-in name.
            // Not handled here; no corpus document takes that path.
            if (!byId.TryGetValue(parentId, out WordStyle? parent)) continue;

            // The parent still answers whenever Writer has a style of the parent's name. Only
            // where it has none — where the old reading fell through to nought — does the style's
            // own Writer hierarchy show through, and then only for the heading family.
            if (!WriterPoolSpacing.TryFor(parent.Name, out (int Above, int Below) pool))
                WriterPoolSpacing.TryForOwnName(style.Name, out pool);
            (int above, int below) = pool;

            XElement replacementSpacing = new(spacing);
            replacementSpacing.SetAttributeValue(
                Word.Name(before ? "after" : "before"),
                (before ? below : above).ToString(System.Globalization.CultureInfo.InvariantCulture));

            // Which half this synthesised, so that the one path that must not see it can tell it
            // from a value the file states. See PoolCompletedSide.
            replacementSpacing.SetAttributeValue(PoolCompleted, before ? "after" : "before");

            XElement replacement = new(style.ParagraphProperties!);
            replacement.Element(Word.Name("spacing"))?.ReplaceWith(replacementSpacing);
            style.ReplaceParagraphProperties(replacement);
        }
    }

    /// <summary>
    /// Marks the half of a <c>w:spacing</c> that <see cref="CompleteOneSidedSpacing"/> supplied
    /// rather than the file stating.
    /// </summary>
    /// <remarks>
    /// A private namespace on a detached copy of the style's <c>w:pPr</c>, so nothing that reads
    /// <c>w:</c> attributes can see it and the loaded part is untouched. It exists because the
    /// completion is a property of the <em>style</em> and one caller — a paragraph that triggers
    /// tdf#118521 — has to resolve the same attribute as though the completion had never happened.
    /// See <see cref="PoolCompletedSide"/>.
    /// </remarks>
    private static readonly XName PoolCompleted =
        XName.Get("poolCompleted", "urn:xray-office:word-styles");

    /// <summary>
    /// Which half of a <c>w:spacing</c> layer is Writer's pool default rather than a stated value —
    /// <c>"before"</c>, <c>"after"</c>, or null when the layer states both halves itself.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A paragraph that sets one of its two margins directly does not see this
    /// completion at all.</strong> writerfilter's <c>tdf#118521</c> block
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:3110-3138) fires when the
    /// paragraph's own <c>w:pPr</c> sets an unequal subset of {top margin, bottom margin,
    /// contextual spacing}, and fills each unset margin <em>as direct formatting</em> from
    /// <c>GetPropertyFromParaStyleSheet</c> — which walks the DOCX <c>w:basedOn</c> chain and then
    /// <c>w:docDefaults</c> (<c>DomainMapper_Impl.cxx</c>:1556-1628) and never consults Writer's
    /// pool. The pool completion lives on the Writer style, so that walk cannot reach it.
    /// </para>
    /// <para>
    /// Measured on the installed 26.2.4.2 by
    /// <c>dotnet/probes/words-p1-01/direct-one-sided-spacing.py</c>, which authors the chain both
    /// declaration orders round because only one of them can answer the question — with the parent
    /// declared first the completion never fires and every variant reads the same number whatever
    /// the rule is. Child declared first, which is the arrangement the corpus document has:
    /// </para>
    /// <code>
    ///   paragraph states               top   bottom
    ///   nothing                        120      120     the completion
    ///   w:spacing w:line only          120      120     an element is not a setting
    ///   w:spacing w:before="80"         80       60     the DOCX chain, not the pool
    ///   w:spacing w:before="0"           0       60     stated, not non-zero, is the trigger
    ///   w:spacing before and after      80       40
    ///   w:contextualSpacing only       120       60     no w:spacing at all, and it still fires
    /// </code>
    /// <para>
    /// The bottom column is the whole of it: 120 is Writer's <c>Heading 4</c> pool row and 60 is
    /// the parent style's own <c>w:after</c>, three points apart. In
    /// <c>FAA 2025-26 Holdover Tables.docx</c> 31 of the 113 <c>NOTES</c> headings carry a direct
    /// <c>w:spacing w:before="80"</c> and the other 76 carry nothing, and LibreOffice puts the
    /// first note 3.00 pt closer on exactly those 31 — twenty pages' worth, because each of those
    /// pages is one line from full.
    /// </para>
    /// </remarks>
    /// <param name="spacing">A <c>w:spacing</c> layer.</param>
    internal static string? PoolCompletedSide(XElement spacing)
        => spacing?.Attribute(PoolCompleted)?.Value;

    /// <summary>The style with this id and type, or null.</summary>
    public WordStyle? Find(string? styleId, WordStyleType type)
        => styleId is not null && _styles.TryGetValue((type, styleId), out WordStyle? style) ? style : null;

    /// <summary>The default style's id for a type, or null when none is marked default.</summary>
    public string? DefaultStyleId(WordStyleType type)
        => _defaults.TryGetValue(type, out string? id) ? id : null;

    /// <summary>
    /// The user-visible name of a style, falling back to its id.
    /// </summary>
    /// <remarks>
    /// The id is what content references and the name is what a person recognises, and they
    /// differ by exactly the characters a name cannot contain — <c>Heading1</c> against
    /// "Heading 1". Callers want the latter.
    /// </remarks>
    public string? DisplayName(string? styleId, WordStyleType type)
    {
        if (styleId is null) return null;
        WordStyle? style = Find(styleId, type);
        return style?.Name is { Length: > 0 } name ? name : styleId;
    }

    /// <summary>
    /// Resolves a property through one style's <c>w:basedOn</c> chain, ignoring the document
    /// defaults.
    /// </summary>
    /// <param name="styleId">The style to start from.</param>
    /// <param name="type">The style's type.</param>
    /// <param name="runProperty">
    /// True to look in the style's <c>w:rPr</c>, false for its <c>w:pPr</c>.
    /// </param>
    /// <param name="localName">The property element's local name, e.g. <c>b</c> or <c>jc</c>.</param>
    public WordProperty ResolveInStyleChain(
        string? styleId, WordStyleType type, bool runProperty, string localName)
    {
        WordStyle? current = Find(styleId, type);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            XElement? properties = runProperty ? current.RunProperties : current.ParagraphProperties;
            if (Word.Child(properties, localName) is { } found)
            {
                return new WordProperty(
                    found,
                    depth == 0 ? WordPropertyOrigin.SetHere : WordPropertyOrigin.Inherited,
                    current.StyleId);
            }

            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, type);
        }
        return WordProperty.Unset;
    }

    /// <summary>
    /// Every layer that states one <c>w:pPr</c> child, innermost first: the paragraph's own, then its
    /// style chain, then the document defaults.
    /// </summary>
    /// <remarks>
    /// <para>
    /// For the property elements that are <em>bags of attributes</em> rather than single values —
    /// <c>w:spacing</c> and <c>w:ind</c> — the whole element is not the unit of inheritance. Word, and
    /// LibreOffice's importer with it, maps each attribute to its own property (<c>w:before</c> to
    /// <c>PARA_TOP_MARGIN</c>, <c>w:left</c> to <c>PARA_LEFT_MARGIN</c>, and so on), so a paragraph that
    /// states only <c>w:line</c> still inherits its style's <c>w:before</c>.
    /// </para>
    /// <para>
    /// Taking the innermost element whole instead silently zeroes every attribute that element omits,
    /// which loses paragraph spacing wherever a paragraph overrides one attribute of its style's
    /// <c>w:spacing</c> — the ordinary case, and one that shortens every page it appears on.
    /// </para>
    /// </remarks>
    /// <param name="localName">The property element's local name, e.g. <c>spacing</c>.</param>
    /// <param name="directParagraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    /// <param name="paragraphStyleId">The paragraph style in force, or null.</param>
    /// <param name="tableStyle">
    /// The <c>w:pPr</c> chain of the table style the paragraph sits inside, innermost first, or null for
    /// a paragraph that is not in a table. §17.7.2's hierarchy puts the table style <em>below</em> the
    /// paragraph styles and above the document defaults, which is where it goes here.
    /// </param>
    public List<XElement> ParagraphPropertyLayers(
        string localName,
        XElement? directParagraphProperties,
        string? paragraphStyleId,
        IReadOnlyList<XElement>? tableStyle = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(localName);

        List<XElement> layers = [];

        if (Word.Child(directParagraphProperties, localName) is { } direct) layers.Add(direct);

        WordStyle? current = Find(paragraphStyleId, WordStyleType.Paragraph);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            if (Word.Child(current.ParagraphProperties, localName) is { } found) layers.Add(found);
            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, WordStyleType.Paragraph);
        }

        if (tableStyle is not null)
        {
            foreach (XElement properties in tableStyle)
            {
                if (Word.Child(properties, localName) is { } fromTable) layers.Add(fromTable);
            }
        }

        if (Word.Child(DefaultParagraphProperties, localName) is { } fallback) layers.Add(fallback);

        return layers;
    }

    /// <summary>
    /// Every layer that states one <c>w:rPr</c> child, innermost first: the run's own, then the
    /// character style chain, then the paragraph style chain, then the document defaults.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The run-property counterpart of <see cref="ParagraphPropertyLayers"/>, and it exists for one
    /// element: <c>w:rFonts</c>, which names up to four families in four attributes and is inherited
    /// attribute by attribute rather than whole. A run carrying <c>&lt;w:rFonts w:cs="Arial"/&gt;</c> —
    /// which Word writes constantly, beside a <c>w:szCs</c>, to set only the complex-script face —
    /// still takes its Latin family from its style. Taking the innermost element whole instead loses
    /// that family and falls back to the complex-script one, so ordinary Latin text is laid out in the
    /// wrong face and every line it sets is the wrong height.
    /// </para>
    /// <para>
    /// The order is <see cref="ResolveRunProperty"/>'s: character style before paragraph style, since
    /// the character style is the inner of the two. No toggle rule applies — <c>w:rFonts</c> is not a
    /// toggle, and the elements this is used for never are.
    /// </para>
    /// </remarks>
    /// <param name="localName">The property element's local name, e.g. <c>rFonts</c>.</param>
    /// <param name="directRunProperties">The run's own <c>w:rPr</c>, or null.</param>
    /// <param name="paragraphStyleId">The paragraph style in force, or null.</param>
    /// <param name="characterStyleId">The character style the run names, or null.</param>
    /// <param name="tableStyleRunProperties">
    /// The table style's <c>w:rPr</c> layers for this cell, most specific first, or null outside a
    /// table. See <see cref="TableStyleRunProperties"/>.
    /// </param>
    public List<XElement> RunPropertyLayers(
        string localName,
        XElement? directRunProperties,
        string? paragraphStyleId,
        string? characterStyleId,
        IReadOnlyList<XElement>? tableStyleRunProperties = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(localName);

        List<XElement> layers = [];

        if (Word.Child(directRunProperties, localName) is { } direct) layers.Add(direct);

        AddChain(characterStyleId, WordStyleType.Character);
        AddChain(paragraphStyleId, WordStyleType.Paragraph);

        // Under both style chains and over the document defaults, which is where §17.7.2 puts a table
        // style: a paragraph style's font wins over the table's, and the table's over the default.
        if (tableStyleRunProperties is not null)
        {
            foreach (XElement layer in tableStyleRunProperties)
            {
                if (Word.Child(layer, localName) is { } found) layers.Add(found);
            }
        }

        if (Word.Child(DefaultRunProperties, localName) is { } fallback) layers.Add(fallback);

        return layers;

        void AddChain(string? styleId, WordStyleType type)
        {
            WordStyle? current = Find(styleId, type);
            HashSet<string> visited = new(StringComparer.Ordinal);

            for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
            {
                if (Word.Child(current.RunProperties, localName) is { } found) layers.Add(found);
                if (!visited.Add(current.StyleId)) break;
                current = Find(current.BasedOn, type);
            }
        }
    }

    /// <summary>
    /// A table style's <c>w:pPr</c> elements, its own first and then its <c>w:basedOn</c> chain.
    /// </summary>
    /// <remarks>
    /// A table style carries paragraph formatting for the paragraphs in its cells, and the one Word
    /// writes for nearly every table — <c>Table Grid</c> — sets <c>w:spacing w:after="0"
    /// w:line="240"</c>. That is what makes table text compact, so ignoring it leaves every cell
    /// paragraph carrying the document default's space-after and 1.08 line spacing instead, which makes
    /// each row a few points too tall and a long table pages too long.
    /// </remarks>
    /// <param name="tableStyleId">The <c>w:tblStyle</c> the table names, or null.</param>
    public List<XElement> TableStyleParagraphProperties(string? tableStyleId)
    {
        List<XElement> chain = [];

        WordStyle? current = Find(tableStyleId, WordStyleType.Table);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            if (current.ParagraphProperties is { } properties) chain.Add(properties);
            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, WordStyleType.Table);
        }

        return chain;
    }

    /// <summary>
    /// The <c>w:rPr</c> layers a table style applies to one cell, most specific first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The layer §17.7.2 puts between the document defaults and the paragraph style, and the one this
    /// reader had no notion of at all: <c>w:tblStylePr</c> was read by nothing, so a table style could
    /// make its heading row bold and the heading row came out plain. That is not a cosmetic loss —
    /// bold is wider, so the header wraps onto more lines, so the row is taller, so fewer body rows
    /// fit on the page. Measured on <c>airbus-pdf-information-package_v1-4.docx</c>, whose repeated
    /// header was three lines here against the reference's four, which moved one body row onto every
    /// page from the sixth and left the last page holding a fifth of what it should.
    /// </para>
    /// <para>
    /// A style's own layers come before its parent's, and within a style the order is
    /// <see cref="WordTableStyleConditions.Names"/>'s — so a <c>firstRow</c> layer on the style beats a
    /// <c>firstRow</c> layer on the style it is based on, and beats its own <c>wholeTable</c>.
    /// </para>
    /// </remarks>
    /// <param name="tableStyleId">The <c>w:tblStyle</c> the table names, or null.</param>
    /// <param name="conditions">Which regions the cell is in, and which the table asked for.</param>
    public List<XElement> TableStyleRunProperties(
        string? tableStyleId, WordTableStyleConditions conditions)
    {
        List<XElement> layers = [];

        WordStyle? current = Find(tableStyleId, WordStyleType.Table);
        if (current is null) return layers;

        IReadOnlyList<string> names = conditions.Names;
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            foreach (string name in names)
            {
                if (current.ConditionalRunProperties.TryGetValue(name, out XElement? found))
                    layers.Add(found);
            }

            // The unconditional half last, since every conditional layer refines it.
            if (current.RunProperties is { } own) layers.Add(own);

            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, WordStyleType.Table);
        }

        return layers;
    }

    /// <summary>
    /// The <c>w:tcPr</c> layers a table style applies to one cell, most specific first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The cell half of <see cref="TableStyleRunProperties"/>, resolved the same way and in the same
    /// order: a style's own layers before its parent's, and within a style the order of
    /// <see cref="WordTableStyleConditions.Names"/>. The unconditional <c>w:style/w:tcPr</c> goes last
    /// within each style, since every conditional layer refines it.
    /// </para>
    /// <para>
    /// This is what draws. Round 62 measured <c>012_Project_Timeline_Template_Black_and_Brown_Theme</c>
    /// page 1 at <b>75 fills on the reference against 19 of ours</b>, established that the missing 56
    /// are <em>not</em> <c>w:shd</c> — the document holds twelve and we draw twelve — and named this as
    /// the seat. The resolved reach over the corpus is 749 cells in 42 documents.
    /// </para>
    /// </remarks>
    /// <param name="tableStyleId">The <c>w:tblStyle</c> the table names, or null.</param>
    /// <param name="conditions">Which regions the cell is in, and which the table asked for.</param>
    public List<XElement> TableStyleCellProperties(
        string? tableStyleId, WordTableStyleConditions conditions)
    {
        List<XElement> layers = [];

        WordStyle? current = Find(tableStyleId, WordStyleType.Table);
        if (current is null) return layers;

        IReadOnlyList<string> names = conditions.Names;
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            foreach (string name in names)
            {
                if (current.ConditionalCellProperties.TryGetValue(name, out XElement? found))
                    layers.Add(found);
            }

            if (current.CellProperties is { } own) layers.Add(own);

            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, WordStyleType.Table);
        }

        return layers;
    }

    /// <summary>
    /// The band sizes a table style declares — <c>w:tblStyleRowBandSize</c> and
    /// <c>w:tblStyleColBandSize</c>, one each, defaulting to one.
    /// </summary>
    /// <remarks>
    /// Read off the <c>w:tblPr</c> chain rather than off the table, because they belong to the style:
    /// a table has no way to say how wide its own bands are. The first stated value in the chain wins,
    /// the style's own before its parent's, and a value below one is clamped away — a band size of
    /// nought would divide by zero and a negative one is not a thing the format can mean.
    /// </remarks>
    /// <param name="tableStyleId">The <c>w:tblStyle</c> the table names, or null.</param>
    public (int Rows, int Columns) TableStyleBandSizes(string? tableStyleId)
    {
        int rows = 0;
        int columns = 0;

        foreach (XElement properties in TableStyleTableProperties(tableStyleId))
        {
            if (rows == 0 && Size(properties, "tblStyleRowBandSize") is { } row) rows = row;
            if (columns == 0 && Size(properties, "tblStyleColBandSize") is { } column) columns = column;
        }

        return (rows > 0 ? rows : 1, columns > 0 ? columns : 1);

        static int? Size(XElement properties, string localName)
            => Word.Child(properties, localName) is { } element
               && int.TryParse(
                   Word.Attribute(element, "val"),
                   NumberStyles.Integer,
                   CultureInfo.InvariantCulture,
                   out int value)
               && value > 0
                ? value
                : null;
    }

    /// <summary>
    /// A table style's <c>w:tblPr</c> elements, its own first and then its <c>w:basedOn</c> chain.
    /// </summary>
    /// <remarks>
    /// The table-level half of <see cref="TableStyleParagraphProperties"/>, and the one that draws:
    /// <c>DomainMapperTableHandler::endTableGetTableStyle</c> merges the style's properties into the
    /// table's own — <c>m_aTableProperties-&gt;InsertProps(pMergedProperties)</c> then
    /// <c>InsertProps(pTableProps)</c>, so the table wins per property
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:438) — and
    /// <c>GetMergedInheritedProperties</c> walks <c>w:basedOn</c> parent-first for the same reason.
    /// <c>Table Grid</c> states its whole grid here and nothing else, so a reader that skipped this
    /// drew no line anywhere in the commonest table in Word.
    /// </remarks>
    /// <param name="tableStyleId">The <c>w:tblStyle</c> the table names, or null.</param>
    public List<XElement> TableStyleTableProperties(string? tableStyleId)
    {
        List<XElement> chain = [];

        WordStyle? current = Find(tableStyleId, WordStyleType.Table);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxBasedOnDepth; depth++)
        {
            if (current.TableProperties is { } properties) chain.Add(properties);
            if (!visited.Add(current.StyleId)) break;
            current = Find(current.BasedOn, WordStyleType.Table);
        }

        return chain;
    }

    /// <summary>Resolves a property from <c>w:docDefaults</c> alone.</summary>
    public WordProperty ResolveInDocumentDefaults(bool runProperty, string localName)
    {
        XElement? defaults = runProperty ? DefaultRunProperties : DefaultParagraphProperties;
        return Word.Child(defaults, localName) is { } found
            ? new WordProperty(found, WordPropertyOrigin.Defaulted)
            : WordProperty.Unset;
    }

    /// <summary>
    /// Resolves a run property across every layer, honouring the toggle rule.
    /// </summary>
    /// <param name="localName">The property element's local name.</param>
    /// <param name="directRunProperties">The run's own <c>w:rPr</c>, or null.</param>
    /// <param name="paragraphStyleId">The paragraph style in force, or null.</param>
    /// <param name="characterStyleId">
    /// The character style the run names through <c>w:rStyle</c>, or null.
    /// </param>
    /// <param name="tableStyleRunProperties">
    /// The table style's <c>w:rPr</c> layers for this cell, most specific first, or null outside a
    /// table. See <see cref="TableStyleRunProperties"/>.
    /// </param>
    public WordProperty ResolveRunProperty(
        string localName,
        XElement? directRunProperties,
        string? paragraphStyleId,
        string? characterStyleId,
        IReadOnlyList<XElement>? tableStyleRunProperties = null)
    {
        ArgumentException.ThrowIfNullOrEmpty(localName);

        // Direct formatting is absolute for every property, toggle or not.
        if (Word.Child(directRunProperties, localName) is { } direct)
            return new WordProperty(direct, WordPropertyOrigin.Direct);

        WordProperty fromParagraph =
            ResolveInStyleChain(paragraphStyleId, WordStyleType.Paragraph, runProperty: true, localName);
        WordProperty fromCharacter =
            ResolveInStyleChain(characterStyleId, WordStyleType.Character, runProperty: true, localName);

        if (!ToggleProperties.Contains(localName))
        {
            // Ordinary properties: the innermost layer that sets one wins.
            if (fromCharacter.HasValue) return fromCharacter;
            if (fromParagraph.HasValue) return fromParagraph;
            if (FromTableStyle(localName, tableStyleRunProperties) is { HasValue: true } table)
                return table;
            return ResolveInDocumentDefaults(runProperty: true, localName);
        }

        // A toggle set by both style layers cancels: §17.7.3. Only one layer setting it is the
        // ordinary case and behaves as an override.
        if (fromParagraph.HasValue && fromCharacter.HasValue)
        {
            bool toggled = fromParagraph.IsOn ^ fromCharacter.IsOn;
            return toggled
                ? fromCharacter
                : new WordProperty(OffElement(localName), fromCharacter.Origin, fromCharacter.SourceStyleId);
        }

        if (fromCharacter.HasValue) return fromCharacter;
        if (fromParagraph.HasValue) return fromParagraph;

        // The table style does *not* take part in the toggle cancellation above, and that is a reading
        // of §17.7.3 rather than a shortcut: the rule cancels a toggle set by a paragraph style and a
        // character style, which are the two chains a run is in at once. A table style is a third,
        // outer layer — the run is in it by where it sits rather than by naming it — and cancelling
        // against it would turn a heading row's bold *off* in any table whose cells use a bold
        // paragraph style, which is not what Word draws.
        if (FromTableStyle(localName, tableStyleRunProperties) is { HasValue: true } fromTable)
            return fromTable;

        return ResolveInDocumentDefaults(runProperty: true, localName);
    }

    /// <summary>The first table-style layer to state a property, or unset.</summary>
    private static WordProperty FromTableStyle(
        string localName, IReadOnlyList<XElement>? tableStyleRunProperties)
    {
        if (tableStyleRunProperties is null) return WordProperty.Unset;

        foreach (XElement layer in tableStyleRunProperties)
        {
            if (Word.Child(layer, localName) is { } found)
                return new WordProperty(found, WordPropertyOrigin.Inherited);
        }

        return WordProperty.Unset;
    }

    /// <summary>
    /// Resolves a paragraph property: direct <c>w:pPr</c>, then the style chain, then the
    /// document defaults.
    /// </summary>
    /// <remarks>
    /// Paragraph properties have no toggle rule — §17.7.3 applies to character properties — so
    /// this is a plain override chain.
    /// </remarks>
    public WordProperty ResolveParagraphProperty(
        string localName, XElement? directParagraphProperties, string? paragraphStyleId)
    {
        ArgumentException.ThrowIfNullOrEmpty(localName);

        if (Word.Child(directParagraphProperties, localName) is { } direct)
            return new WordProperty(direct, WordPropertyOrigin.Direct);

        WordProperty fromStyle = ResolveInStyleChain(
            paragraphStyleId, WordStyleType.Paragraph, runProperty: false, localName);
        return fromStyle.HasValue
            ? fromStyle
            : ResolveInDocumentDefaults(runProperty: false, localName);
    }

    /// <summary>
    /// A synthetic "off" element, for reporting the result of a cancelled toggle.
    /// </summary>
    /// <remarks>
    /// The XOR's result is a value no layer actually wrote, so there is no element to point at.
    /// Synthesising one keeps <see cref="WordProperty"/> uniform — every caller reads
    /// <c>IsOn</c> the same way — instead of adding a third state to it.
    /// </remarks>
    private static XElement OffElement(string localName)
        => new(XName.Get(localName, OoxmlNamespaces.WordprocessingML),
               new XAttribute(XName.Get("val", OoxmlNamespaces.WordprocessingML), "0"));
}
