using System.Xml.Linq;
using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// A theme's colour scheme: the twelve colours <c>a:clrScheme</c> declares.
/// </summary>
/// <remarks>
/// Only the colour scheme, not the font or format schemes — those are separate concerns with
/// separate consumers, and a reader that only needs to know what <c>accent1</c> is should not
/// have to parse a format scheme's three fill styles to find out.
/// </remarks>
public sealed class DrawingColourScheme
{
    private readonly Colour?[] _colours = new Colour?[12];

    /// <summary>The scheme's <c>name</c> attribute, for diagnostics.</summary>
    public string? Name { get; private init; }

    /// <summary>The colour in a slot, or null when the scheme declares none.</summary>
    public Colour? this[ThemeColourSlot slot]
        => (int)slot >= 0 && (int)slot < _colours.Length ? _colours[(int)slot] : null;

    /// <summary>
    /// Reads an <c>a:clrScheme</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each child is one slot and holds one colour reference, which is usually an
    /// <c>a:srgbClr</c> but is an <c>a:sysClr</c> for <c>dk1</c> and <c>lt1</c> in every theme
    /// Word ships — <c>&lt;a:sysClr val="windowText" lastClr="000000"/&gt;</c>. Reading only
    /// <c>a:srgbClr</c> therefore loses text1 and background1 in the common case, which is the
    /// two slots most documents reference.
    /// </para>
    /// <para>
    /// A scheme child may carry transforms of its own; they are applied here, so what the
    /// scheme yields is a literal colour.
    /// </para>
    /// </remarks>
    /// <param name="clrScheme">The <c>a:clrScheme</c> element, or null.</param>
    public static DrawingColourScheme? Read(XElement? clrScheme)
    {
        if (clrScheme is null) return null;

        DrawingColourScheme scheme = new() { Name = Drawing.Attribute(clrScheme, "name") };

        foreach (XElement child in clrScheme.Elements())
        {
            if (child.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;
            if (ThemeColourSlots.Parse(child.Name.LocalName) is not { } slot) continue;

            foreach (XElement reference in child.Elements())
            {
                if (DrawingColour.Read(reference) is not { } colour) continue;

                // No theme to resolve against: a scheme entry is a literal, and a schemeClr
                // inside one would be circular.
                scheme._colours[(int)slot] = colour.Resolve(theme: null);
                break;
            }
        }

        return scheme;
    }
}

/// <summary>
/// The mapping from a document's colour names onto the theme's slots — DrawingML's
/// <c>a:clrMap</c>, and WordprocessingML's <c>w:clrSchemeMapping</c>, which is the same idea
/// under different attribute names.
/// </summary>
/// <remarks>
/// <para>
/// This is why "background 1" is not simply the scheme's first colour. A slide master carrying
/// <c>&lt;a:clrMap bg1="dk1" tx1="lt1" …/&gt;</c> is a dark master: every shape on it that asks
/// for <c>bg1</c> gets the theme's <em>dark</em> colour, while a shape asking for <c>lt1</c>
/// still gets the light one. Skipping the map gives a light background and invisible text on
/// exactly the slides where it matters.
/// </para>
/// <para>
/// Only the twelve document-facing names are keys — see
/// <see cref="ThemeColourSlots.MapKey"/> for which those are and why <c>dk1</c> is not among
/// them. LibreOffice's equivalent is <c>ClrMap</c> in
/// <c>oox/source/drawingml/clrscheme.cxx</c>, applied at
/// <c>oox/source/ppt/pptimport.cxx</c>:155.
/// </para>
/// </remarks>
public sealed class DrawingColourMap
{
    private static readonly string[] Keys =
        ["bg1", "tx1", "bg2", "tx2",
         "accent1", "accent2", "accent3", "accent4", "accent5", "accent6",
         "hlink", "folHlink"];

    private readonly Dictionary<string, ThemeColourSlot> _map = new(StringComparer.Ordinal);

    /// <summary>The map that changes nothing, which is what an absent <c>a:clrMap</c> means.</summary>
    public static DrawingColourMap Identity { get; } = new();

    /// <summary>True when the map leaves every name where it found it.</summary>
    public bool IsIdentity => _map.Count == 0;

    /// <summary>
    /// Reads an <c>a:clrMap</c>, or a <c>w:clrSchemeMapping</c> in the WordprocessingML
    /// namespace — the attribute names differ (<c>w:t1</c> against <c>tx1</c>) and the values
    /// differ (<c>dark1</c> against <c>dk1</c>), but both normalise onto the same twelve keys.
    /// </summary>
    /// <param name="element">The mapping element, or null for the identity.</param>
    public static DrawingColourMap Read(XElement? element) => ReadLayered(element);

    /// <summary>
    /// Reads a chain of mapping elements, each one patching the one before it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// PresentationML states the base map once on the master, as <c>p:clrMap</c>, and lets a
    /// layout or a slide amend it with <c>p:clrMapOvr/a:overrideClrMapping</c>. The override
    /// is a <em>patch</em> rather than a replacement, which is the part that is not guessable
    /// from the schema: <c>SlideFragmentHandler</c> starts an <c>overrideClrMapping</c> from a
    /// copy of the map already in force and starts a <c>clrMap</c> from an empty one
    /// (<c>oox/source/ppt/slidefragmenthandler.cxx:194-203</c>). An empty
    /// <c>a:masterClrMapping</c> therefore means "keep what you have", and states no attributes
    /// to say so.
    /// </para>
    /// <para>
    /// <strong>An override that restates the master's map changes nothing, and most do.</strong>
    /// Of the ten corpus decks carrying one, three name exactly what the master already said.
    /// The seven that differ include a title layout sending <c>bg2</c> to <c>dk2</c> where the
    /// master sends it to <c>lt2</c> — its background gradient is built from a dark teal in the
    /// reference and from a near-white in ours, which is the whole page.
    /// </para>
    /// </remarks>
    /// <param name="layers">
    /// The mapping elements, outermost first. Nulls are skipped, so a caller can pass the
    /// master's map and a layout's and a slide's override without testing each one.
    /// </param>
    public static DrawingColourMap ReadLayered(params XElement?[] layers)
    {
        ArgumentNullException.ThrowIfNull(layers);

        DrawingColourMap map = new();

        foreach (XElement? element in layers)
        {
            if (element is null) continue;

            foreach (XAttribute attribute in element.Attributes())
            {
                // Unprefixed in DrawingML, w:-prefixed in WordprocessingML; the local name is
                // the key either way, once normalised.
                if (ThemeColourSlots.MapKey(attribute.Name.LocalName) is not { } key) continue;
                if (ThemeColourSlots.Parse(attribute.Value) is not { } slot) continue;

                map._map[key] = slot;
            }
        }

        if (map._map.Count == 0) return Identity;

        // A map that names every key as itself is the identity, and saying so lets callers skip
        // it — which matters because Word writes the identity into every settings.xml it saves.
        if (Keys.All(key => !map._map.TryGetValue(key, out ThemeColourSlot mapped)
                            || mapped == ThemeColourSlots.Parse(key)))
        {
            return Identity;
        }

        return map;
    }

    /// <summary>
    /// The slot a scheme-colour reference lands on, or null when the name is not a scheme
    /// colour at all.
    /// </summary>
    public ThemeColourSlot? Resolve(string? name)
    {
        if (ThemeColourSlots.MapKey(name) is { } key
            && _map.TryGetValue(key, out ThemeColourSlot mapped))
        {
            return mapped;
        }

        return ThemeColourSlots.Parse(name);
    }
}

/// <summary>
/// A DrawingML theme part, reduced to what colour resolution needs.
/// </summary>
/// <remarks>
/// The scheme and the map are held together because neither answers a question on its own: the
/// scheme says what <c>dk1</c> is and the map says whether <c>tx1</c> means <c>dk1</c>. Passing
/// only the scheme around is the mistake this type exists to prevent.
/// </remarks>
/// <param name="Colours">The colour scheme, or null when the part declared none.</param>
/// <param name="Map">The colour map in force; never null, defaulting to the identity.</param>
public sealed record DrawingTheme(DrawingColourScheme? Colours, DrawingColourMap Map)
{
    /// <summary>
    /// The font scheme, or null when the part declared none.
    /// </summary>
    /// <remarks>
    /// Not a positional member, so that a caller constructing a theme from a colour scheme alone
    /// — which is every caller that predates typeface resolution — keeps compiling.
    /// </remarks>
    public DrawingFontScheme? Fonts { get; init; }

    /// <summary>
    /// Reads an <c>a:theme</c> root.
    /// </summary>
    /// <remarks>
    /// The theme part itself carries no colour map — in PresentationML the map lives on the
    /// slide master and in WordprocessingML in <c>settings.xml</c> — so this yields the identity
    /// and the caller supplies the map with <see cref="WithMap"/>.
    /// </remarks>
    /// <param name="theme">The <c>a:theme</c> element, or null.</param>
    public static DrawingTheme? Read(XElement? theme)
    {
        if (theme is null) return null;

        XElement? elements = Drawing.Child(theme, "themeElements");
        return new DrawingTheme(
            DrawingColourScheme.Read(Drawing.Child(elements, "clrScheme")),
            DrawingColourMap.Identity)
        {
            Fonts = DrawingFontScheme.Read(Drawing.Child(elements, "fontScheme")),
        };
    }

    /// <summary>This theme seen through a colour map.</summary>
    public DrawingTheme WithMap(DrawingColourMap? map) => this with { Map = map ?? DrawingColourMap.Identity };

    /// <summary>
    /// The literal colour a scheme-colour name resolves to, before any transforms, or null when
    /// the theme has nothing for it.
    /// </summary>
    public Colour? Lookup(string? schemeColourName)
        => Colours is not null && Map.Resolve(schemeColourName) is { } slot ? Colours[slot] : null;

    /// <summary>The literal colour in a slot, bypassing the colour map.</summary>
    public Colour? Lookup(ThemeColourSlot slot) => Colours?[slot];
}
