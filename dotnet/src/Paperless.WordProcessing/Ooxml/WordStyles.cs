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
    public XElement? ParagraphProperties { get; }

    /// <summary>The style's <c>w:rPr</c>, or null.</summary>
    public XElement? RunProperties { get; }
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
