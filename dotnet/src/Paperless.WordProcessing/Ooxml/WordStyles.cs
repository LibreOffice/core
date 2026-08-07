using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Ooxml;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>What a WordprocessingML style may be applied to.</summary>
/// <remarks>
/// Part of a style's identity, not a description: a paragraph style and a character style may
/// share a <c>w:styleId</c>, and <c>w:basedOn</c> only resolves within one type.
/// </remarks>
public enum WordStyleType
{
    /// <summary>Not a type Paperless recognises.</summary>
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

        ParagraphProperties = Word.Child(element, "pPr");
        RunProperties = Word.Child(element, "rPr");
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

    /// <summary>The style's <c>w:pPr</c>, or null.</summary>
    public XElement? ParagraphProperties { get; private set; }

    /// <summary>The style's <c>w:rPr</c>, or null.</summary>
    public XElement? RunProperties { get; }

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

    /// <summary>The <c>w:pPr</c> from <c>w:docDefaults</c>, or null.</summary>
    public XElement? DefaultParagraphProperties { get; private set; }

    /// <summary>Reads a <c>styles.xml</c> root element.</summary>
    /// <param name="root">The <c>w:styles</c> element.</param>
    /// <param name="diagnostics">Receives malformed declarations, if given.</param>
    public void Add(XElement root, IList<Diagnostic>? diagnostics = null)
    {
        ArgumentNullException.ThrowIfNull(root);

        if (Word.Child(root, "docDefaults") is { } docDefaults)
        {
            DefaultRunProperties = Word.Child(Word.Child(docDefaults, "rPrDefault"), "rPr");
            DefaultParagraphProperties = Word.Child(Word.Child(docDefaults, "pPrDefault"), "pPr");
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

            _styles[(style.Type, style.StyleId)] = style;
            if (style.IsDefault) _defaults[style.Type] = style.StyleId;
            if (style.Type == WordStyleType.Paragraph) declared.Add(style);
        }

        CompleteOneSidedSpacing(declared);
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

            (int above, int below) = WriterPoolSpacing.For(parent.Name);

            XElement replacementSpacing = new(spacing);
            replacementSpacing.SetAttributeValue(
                Word.Name(before ? "after" : "before"),
                (before ? below : above).ToString(System.Globalization.CultureInfo.InvariantCulture));

            XElement replacement = new(style.ParagraphProperties!);
            replacement.Element(Word.Name("spacing"))?.ReplaceWith(replacementSpacing);
            style.ReplaceParagraphProperties(replacement);
        }
    }

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
    public List<XElement> RunPropertyLayers(
        string localName,
        XElement? directRunProperties,
        string? paragraphStyleId,
        string? characterStyleId)
    {
        ArgumentException.ThrowIfNullOrEmpty(localName);

        List<XElement> layers = [];

        if (Word.Child(directRunProperties, localName) is { } direct) layers.Add(direct);

        AddChain(characterStyleId, WordStyleType.Character);
        AddChain(paragraphStyleId, WordStyleType.Paragraph);

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
    public WordProperty ResolveRunProperty(
        string localName,
        XElement? directRunProperties,
        string? paragraphStyleId,
        string? characterStyleId)
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
        return ResolveInDocumentDefaults(runProperty: true, localName);
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
