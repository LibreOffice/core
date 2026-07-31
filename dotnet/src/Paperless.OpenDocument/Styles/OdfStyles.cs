using System.Xml.Linq;
using Paperless.Core.Diagnostics;

namespace Paperless.OpenDocument.Styles;

/// <summary>A reference to a style by name and family.</summary>
/// <remarks>
/// Both halves are needed: a name alone is ambiguous across families, and ODF documents do
/// reuse names (a paragraph style and a cell style may both be called <c>Default</c>).
/// </remarks>
/// <param name="Name">The style name, or null when the object names no style.</param>
/// <param name="Family">The family the name resolves in.</param>
public readonly record struct OdfStyleReference(string? Name, OdfStyleFamily Family);

/// <summary>
/// Every style declared by a document, and the resolution rules over them.
/// </summary>
/// <remarks>
/// <para>
/// ODF splits styles three ways, and all three participate in resolution:
/// </para>
/// <list type="bullet">
///   <item><description>
///     <c>office:styles</c> — named, user-visible styles: the paragraph, character, list
///     and page styles a user picks by name.
///   </description></item>
///   <item><description>
///     <c>office:automatic-styles</c> — generated single-use styles. ODF has no inline
///     style attribute, so <em>every</em> piece of direct formatting becomes a generated
///     style: one bold word produces a <c>&lt;style:style style:name="T1"&gt;</c> and a
///     <c>&lt;text:span text:style-name="T1"&gt;</c> pointing at it. This is why
///     <see cref="OdfPropertyOrigin.SetHere"/> is the ODF evidence for "hard formatting".
///   </description></item>
///   <item><description>
///     <c>office:master-styles</c> — page and slide masters, pairing a geometry
///     (<c>style:page-layout</c>) with header and footer content.
///   </description></item>
/// </list>
/// <para>
/// Resolution walks <c>style:parent-style-name</c> upwards and then falls back to the
/// family's <c>style:default-style</c>, recording which of the three cases produced the
/// value. That is the same resolve-through-a-chain semantics LibreOffice implements with
/// <c>SfxItemSet</c>/<c>SfxItemState</c> — see
/// <c>dotnet/research/05-infrastructure.md</c> section E and
/// <c>dotnet/research/02-writer.md</c> sections A.4 and D.
/// </para>
/// <para>
/// A document's styles come from two files. <c>styles.xml</c> holds all three containers;
/// <c>content.xml</c> holds a second <c>office:automatic-styles</c> for the styles its body
/// references. Both are added to one collection, so lookups do not need to know which file
/// a style came from.
/// </para>
/// </remarks>
public sealed class OdfStyles : IOdfStyleResolver
{
    /// <summary>
    /// How far a parent chain is followed before it is treated as circular.
    /// </summary>
    /// <remarks>
    /// A cycle in <c>style:parent-style-name</c> is illegal but does occur in files written
    /// by buggy converters, and it would otherwise be an infinite loop over untrusted input.
    /// Sixty-four is far deeper than any real style hierarchy.
    /// </remarks>
    public const int MaxParentChainDepth = 64;

    private readonly Dictionary<(OdfStyleFamily Family, string Name), OdfStyle> _named = [];
    private readonly Dictionary<(OdfStyleFamily Family, string Name), OdfStyle> _automatic = [];
    private readonly Dictionary<OdfStyleFamily, OdfStyle> _defaults = [];
    private readonly Dictionary<string, OdfMasterPage> _masterPages = new(StringComparer.Ordinal);
    private readonly Dictionary<string, OdfStyle> _pageLayouts = new(StringComparer.Ordinal);
    private readonly Dictionary<string, OdfListStyle> _listStyles = new(StringComparer.Ordinal);
    private readonly Dictionary<string, OdfDataStyle> _dataStyles = new(StringComparer.Ordinal);
    private readonly Dictionary<string, OdfFontFace> _fontFaces = new(StringComparer.Ordinal);

    /// <summary>Named styles from <c>office:styles</c>.</summary>
    public IReadOnlyCollection<OdfStyle> NamedStyles => _named.Values;

    /// <summary>Generated styles from either file's <c>office:automatic-styles</c>.</summary>
    public IReadOnlyCollection<OdfStyle> AutomaticStyles => _automatic.Values;

    /// <summary>Per-family defaults from <c>style:default-style</c>.</summary>
    public IReadOnlyDictionary<OdfStyleFamily, OdfStyle> DefaultStyles => _defaults;

    /// <summary>Page and slide masters from <c>office:master-styles</c>, keyed by name.</summary>
    public IReadOnlyDictionary<string, OdfMasterPage> MasterPages => _masterPages;

    /// <summary>Page geometries from <c>style:page-layout</c>, keyed by name.</summary>
    public IReadOnlyDictionary<string, OdfStyle> PageLayouts => _pageLayouts;

    /// <summary>List styles, keyed by name.</summary>
    public IReadOnlyDictionary<string, OdfListStyle> ListStyles => _listStyles;

    /// <summary>
    /// The document's single <c>text:outline-style</c>, which numbers headings.
    /// </summary>
    /// <remarks>
    /// Unlike a list style this is never referenced by name — a heading picks it up purely
    /// by having an outline level — so it is exposed separately rather than sitting in
    /// <see cref="ListStyles"/> waiting for a lookup that never comes.
    /// </remarks>
    public OdfListStyle? OutlineStyle { get; private set; }

    /// <summary>Number formats, keyed by name.</summary>
    public IReadOnlyDictionary<string, OdfDataStyle> DataStyles => _dataStyles;

    /// <summary>Font declarations from <c>office:font-face-decls</c>, keyed by name.</summary>
    public IReadOnlyDictionary<string, OdfFontFace> FontFaces => _fontFaces;

    /// <summary>
    /// Adds every style container found under an <c>office:document-content</c>,
    /// <c>office:document-styles</c> or flat <c>office:document</c> root.
    /// </summary>
    /// <param name="documentRoot">The root element of one of the document's XML parts.</param>
    /// <param name="diagnostics">Receives collisions and malformed declarations, if given.</param>
    public void AddDocument(XElement documentRoot, IList<Diagnostic>? diagnostics = null)
    {
        ArgumentNullException.ThrowIfNull(documentRoot);

        // Order matters only for font faces and defaults, both of which are last-wins;
        // styles are keyed by (family, name) so container order is irrelevant.
        foreach (string container in
                 (string[])["font-face-decls", "styles", "automatic-styles", "master-styles"])
        {
            XElement? element = documentRoot.Element(XName.Get(container, OdfNamespaces.Office));
            if (element is not null) AddContainer(element, diagnostics);
        }
    }

    /// <summary>
    /// Adds the declarations in one style container: <c>office:styles</c>,
    /// <c>office:automatic-styles</c>, <c>office:master-styles</c> or
    /// <c>office:font-face-decls</c>.
    /// </summary>
    public void AddContainer(XElement container, IList<Diagnostic>? diagnostics = null)
    {
        ArgumentNullException.ThrowIfNull(container);

        OdfStyleSource source = container.Name.LocalName == "automatic-styles"
            ? OdfStyleSource.Automatic
            : OdfStyleSource.Named;

        foreach (XElement child in container.Elements())
        {
            string ns = child.Name.NamespaceName;
            switch (child.Name.LocalName)
            {
                case "style" when ns == OdfNamespaces.Style:
                    AddStyle(child, source, diagnostics);
                    break;

                case "default-style" when ns == OdfNamespaces.Style:
                    AddDefaultStyle(child, diagnostics);
                    break;

                case "page-layout" when ns == OdfNamespaces.Style:
                    AddPageLayout(child, diagnostics);
                    break;

                case "master-page" when ns == OdfNamespaces.Style:
                    AddMasterPage(child, diagnostics);
                    break;

                case "list-style" when ns == OdfNamespaces.Text:
                    AddListStyle(child, diagnostics);
                    break;

                case "outline-style" when ns == OdfNamespaces.Text:
                    OutlineStyle = new OdfListStyle(child);
                    break;

                case "font-face" when ns == OdfNamespaces.Style:
                    OdfFontFace face = new(child);
                    if (face.Name.Length > 0) _fontFaces[face.Name] = face;
                    break;

                default:
                    // Data styles are the number: family; everything else in these
                    // containers (notes configuration, line numbering, themes, gradients,
                    // presentation page layouts) is not part of style resolution.
                    if (ns == OdfNamespaces.Number) AddDataStyle(child, diagnostics);
                    break;
            }
        }
    }

    /// <summary>
    /// Finds a style by name and family, preferring an automatic style over a named one.
    /// </summary>
    /// <remarks>
    /// Content references automatic styles by the same attribute it uses for named ones, and
    /// an automatic style is the more specific declaration, so it wins. In practice the two
    /// name spaces do not overlap — LibreOffice generates <c>P1</c>, <c>T1</c>, <c>ce1</c>
    /// and so on — but a converter that reuses a named style's name should not silently
    /// shadow the direct formatting.
    /// </remarks>
    public OdfStyle? Find(string? name, OdfStyleFamily family)
    {
        if (name is null) return null;

        // Page layouts live in their own container because they are written as
        // style:page-layout rather than style:style, but they resolve like any other style —
        // so the lookup has to reach them or a page's geometry would be unresolvable.
        if (family == OdfStyleFamily.PageLayout)
            return _pageLayouts.TryGetValue(name, out OdfStyle? layout) ? layout : null;

        if (_automatic.TryGetValue((family, name), out OdfStyle? automatic)) return automatic;
        return _named.TryGetValue((family, name), out OdfStyle? named) ? named : null;
    }

    /// <summary>The named style with this name and family, ignoring automatic styles.</summary>
    public OdfStyle? FindNamed(string? name, OdfStyleFamily family)
        => name is not null && _named.TryGetValue((family, name), out OdfStyle? style) ? style : null;

    /// <summary>The family's <c>style:default-style</c>, or null when it declares none.</summary>
    public OdfStyle? GetDefault(OdfStyleFamily family)
        => _defaults.TryGetValue(family, out OdfStyle? style) ? style : null;

    /// <summary>The master page with this name, or null.</summary>
    public OdfMasterPage? FindMasterPage(string? name)
        => name is not null && _masterPages.TryGetValue(name, out OdfMasterPage? page) ? page : null;

    /// <summary>The page layout with this name, or null.</summary>
    public OdfStyle? FindPageLayout(string? name)
        => name is not null && _pageLayouts.TryGetValue(name, out OdfStyle? layout) ? layout : null;

    /// <summary>The list style with this name, or null.</summary>
    public OdfListStyle? FindListStyle(string? name)
        => name is not null && _listStyles.TryGetValue(name, out OdfListStyle? style) ? style : null;

    /// <summary>The data style with this name, or null.</summary>
    public OdfDataStyle? FindDataStyle(string? name)
        => name is not null && _dataStyles.TryGetValue(name, out OdfDataStyle? style) ? style : null;

    /// <inheritdoc/>
    public OdfProperty ResolveProperty(
        string? styleName,
        OdfStyleFamily family,
        OdfPropertyKind kind,
        string propertyNamespace,
        string propertyName)
    {
        OdfProperty set = ResolveWithoutDefaults(styleName, family, kind, propertyNamespace, propertyName);
        return set.HasValue ? set : ResolveFromDefaults(family, kind, propertyNamespace, propertyName);
    }

    /// <summary>
    /// Resolves a property through a cascade of styles, outermost first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is what a run of text actually needs. Character formatting on a
    /// <c>&lt;text:span&gt;</c> overrides the enclosing paragraph style's text properties,
    /// which in turn override the family defaults — so the effective value comes from the
    /// innermost style that sets one, and only if none does at all do the defaults apply.
    /// </para>
    /// <para>
    /// Resolving each reference independently and taking the first with a value would be
    /// wrong: the innermost reference nearly always has a <em>defaulted</em> value, which
    /// would beat an outer style's explicitly set one. Hence the two passes.
    /// </para>
    /// </remarks>
    /// <param name="cascade">
    /// The style references from outermost to innermost — for a span inside a paragraph,
    /// the paragraph style then the span style.
    /// </param>
    /// <param name="kind">Which property set to look in.</param>
    /// <param name="propertyNamespace">The property attribute's namespace URI.</param>
    /// <param name="propertyName">The property attribute's local name.</param>
    public OdfProperty ResolveProperty(
        IReadOnlyList<OdfStyleReference> cascade,
        OdfPropertyKind kind,
        string propertyNamespace,
        string propertyName)
    {
        ArgumentNullException.ThrowIfNull(cascade);

        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfProperty found = ResolveWithoutDefaults(
                cascade[i].Name, cascade[i].Family, kind, propertyNamespace, propertyName);
            if (found.HasValue) return found;
        }

        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfProperty defaulted = ResolveFromDefaults(
                cascade[i].Family, kind, propertyNamespace, propertyName);
            if (defaulted.HasValue) return defaulted;
        }

        return OdfProperty.Unset;
    }

    /// <summary>
    /// Resolves a property through the parent chain only, ignoring the family defaults.
    /// </summary>
    /// <remarks>
    /// Exposed because "does anything actually set this, or is it only a default?" is a
    /// question callers ask — a presentation placeholder's inherited formatting must beat a
    /// defaulted value but lose to an explicitly set one.
    /// </remarks>
    public OdfProperty ResolveWithoutDefaults(
        string? styleName,
        OdfStyleFamily family,
        OdfPropertyKind kind,
        string propertyNamespace,
        string propertyName)
    {
        OdfStyle? current = Find(styleName, family);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < MaxParentChainDepth; depth++)
        {
            if (current.GetOwnProperty(kind, propertyNamespace, propertyName) is { } value)
            {
                return new OdfProperty(
                    value,
                    depth == 0 ? OdfPropertyOrigin.SetHere : OdfPropertyOrigin.Inherited,
                    current.Name);
            }

            if (!visited.Add(current.Name)) break;
            current = Find(current.ParentStyleName, family);
        }

        return OdfProperty.Unset;
    }

    /// <summary>
    /// Resolves a property from the family defaults alone.
    /// </summary>
    /// <remarks>
    /// The fallback families are not arbitrary. ODF declares no
    /// <c>style:default-style style:family="text"</c>: default character formatting lives in
    /// the <em>paragraph</em> default's <c>style:text-properties</c>, so a character style
    /// that sets nothing must fall through to it or every run loses its default font.
    /// Presentation styles fall back to the graphic defaults for the same reason — that is
    /// where Impress records the defaults its placeholder styles build on.
    /// </remarks>
    public OdfProperty ResolveFromDefaults(
        OdfStyleFamily family,
        OdfPropertyKind kind,
        string propertyNamespace,
        string propertyName)
    {
        foreach (OdfStyleFamily candidate in DefaultFamilyChain(family))
        {
            OdfStyle? defaults = GetDefault(candidate);
            if (defaults?.GetOwnProperty(kind, propertyNamespace, propertyName) is { } value)
                return new OdfProperty(value, OdfPropertyOrigin.Defaulted, defaults.Name);
        }
        return OdfProperty.Unset;
    }

    private static OdfStyleFamily[] DefaultFamilyChain(OdfStyleFamily family) => family switch
    {
        OdfStyleFamily.Text => [OdfStyleFamily.Text, OdfStyleFamily.Paragraph],
        OdfStyleFamily.Presentation => [OdfStyleFamily.Presentation, OdfStyleFamily.Graphic],
        _ => [family],
    };

    private void AddStyle(XElement element, OdfStyleSource source, IList<Diagnostic>? diagnostics)
    {
        string? name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value;
        if (string.IsNullOrEmpty(name))
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2001",
                   "A style:style has no style:name and cannot be referenced; ignoring it.");
            return;
        }

        OdfStyleFamily family = OdfStyleFamilies.Parse(
            element.Attribute(XName.Get("family", OdfNamespaces.Style))?.Value);

        OdfStyle style = new(name, family, source, element);
        Dictionary<(OdfStyleFamily, string), OdfStyle> target =
            source == OdfStyleSource.Automatic ? _automatic : _named;

        if (target.ContainsKey((family, name)))
        {
            Report(diagnostics, DiagnosticSeverity.Information, "PL2002",
                   $"Style '{name}' ({family.ToAttributeValue()}) is declared more than once; "
                   + "the later declaration wins.");
        }
        target[(family, name)] = style;
    }

    private void AddDefaultStyle(XElement element, IList<Diagnostic>? diagnostics)
    {
        OdfStyleFamily family = OdfStyleFamilies.Parse(
            element.Attribute(XName.Get("family", OdfNamespaces.Style))?.Value);
        if (family == OdfStyleFamily.Unknown)
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2003",
                   "A style:default-style names no recognised style:family; ignoring it.");
            return;
        }

        // The name is synthetic: defaults have none, but OdfProperty.SourceStyleName reads
        // far better in a diagnostic as "<default paragraph>" than as null.
        _defaults[family] = new OdfStyle(
            $"<default {family.ToAttributeValue()}>", family, OdfStyleSource.Default, element);
    }

    private void AddPageLayout(XElement element, IList<Diagnostic>? diagnostics)
    {
        string? name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value;
        if (string.IsNullOrEmpty(name))
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2004",
                   "A style:page-layout has no style:name; ignoring it.");
            return;
        }
        _pageLayouts[name] = new OdfStyle(name, OdfStyleFamily.PageLayout, OdfStyleSource.Automatic, element);
    }

    private void AddMasterPage(XElement element, IList<Diagnostic>? diagnostics)
    {
        OdfMasterPage master = new(element);
        if (master.Name.Length == 0)
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2005",
                   "A style:master-page has no style:name; ignoring it.");
            return;
        }
        _masterPages[master.Name] = master;
    }

    private void AddListStyle(XElement element, IList<Diagnostic>? diagnostics)
    {
        OdfListStyle style = new(element);
        if (style.Name.Length == 0)
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2006",
                   "A text:list-style has no style:name and cannot be referenced; ignoring it.");
            return;
        }
        _listStyles[style.Name] = style;
    }

    private void AddDataStyle(XElement element, IList<Diagnostic>? diagnostics)
    {
        OdfDataStyle style = new(element);
        if (style.Kind == OdfDataStyleKind.Unknown) return;
        if (style.Name.Length == 0)
        {
            Report(diagnostics, DiagnosticSeverity.Warning, "PL2007",
                   $"A {element.Name.LocalName} has no style:name; ignoring it.");
            return;
        }
        _dataStyles[style.Name] = style;
    }

    private static void Report(
        IList<Diagnostic>? diagnostics, DiagnosticSeverity severity, string code, string message)
        => diagnostics?.Add(new Diagnostic(severity, code, message));
}
