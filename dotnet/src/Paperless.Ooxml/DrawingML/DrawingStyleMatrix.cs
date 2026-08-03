using System.Xml.Linq;
using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// A theme's <c>a:fmtScheme</c> — the three fill styles and three line styles a shape's
/// <c>p:style</c> names by index — resolved into the fill and line that shape actually draws.
/// </summary>
/// <remarks>
/// <para>
/// DrawingML lets a shape state no fill and no outline at all and still be painted, because
/// its <c>p:style</c> carries an <c>a:fillRef</c> and an <c>a:lnRef</c> into the theme's format
/// matrix. The reference is two halves and neither is enough alone: the index says
/// <em>which</em> of the theme's styles to take, and the colour inside the reference says what
/// to substitute for the <c>phClr</c> those styles are written in terms of. A shape drawn from
/// the index alone comes out in whatever colour the theme's placeholder happens to resolve to;
/// a shape drawn from the colour alone loses the theme's gradient, width and dash.
/// </para>
/// <para>
/// <strong>The substitution is done in XML rather than in a colour model</strong>, by cloning
/// the theme's style element and replacing each <c>a:schemeClr val="phClr"</c> with the
/// resolved colour. That keeps whatever the theme wrapped around the placeholder — a
/// <c>shade</c>/<c>satMod</c> pair, a gradient's stop list, a line's width and dash — and hands
/// the result to the same fill and line readers a shape stating its own uses, so a themed
/// gradient is read by the code that reads a stated one rather than by a second copy of it.
/// </para>
/// <para>
/// The indices are one-based and <c>0</c> means "none": <c>a:fillRef idx="0"</c> is how a shape
/// says it takes no fill from the theme, and LibreOffice's <c>getFillStyle</c>
/// (<c>oox/source/drawingml/theme.cxx</c>) returns nothing for it. Merging order is
/// <c>Theme::getFillStyle</c> then <c>assignUsed</c> of the shape's own properties —
/// <c>Shape::getActualFillProperties</c>, <c>oox/source/drawingml/shape.cxx</c> — which is to
/// say the theme is the base and anything the shape states wins over it.
/// </para>
/// </remarks>
public sealed class DrawingStyleMatrix
{
    private readonly List<XElement> _fills = [];
    private readonly List<XElement> _backgrounds = [];
    private readonly List<XElement> _lines = [];
    private readonly List<XElement> _effects = [];

    private DrawingStyleMatrix()
    {
    }

    /// <summary>True when the theme declared none of the lists, so nothing can be resolved.</summary>
    public bool IsEmpty =>
        _fills.Count == 0 && _backgrounds.Count == 0 && _lines.Count == 0 && _effects.Count == 0;

    /// <summary>
    /// Reads an <c>a:theme</c> root's format scheme, or null when it has none.
    /// </summary>
    /// <param name="theme">The <c>a:theme</c> element, or null.</param>
    public static DrawingStyleMatrix? Read(XElement? theme)
    {
        XElement? format = Drawing.Child(Drawing.Child(theme, "themeElements"), "fmtScheme");
        if (format is null) return null;

        DrawingStyleMatrix matrix = new();
        matrix._fills.AddRange(Drawing.Child(format, "fillStyleLst")?.Elements() ?? []);
        matrix._backgrounds.AddRange(Drawing.Child(format, "bgFillStyleLst")?.Elements() ?? []);
        matrix._lines.AddRange(Drawing.Child(format, "lnStyleLst")?.Elements() ?? []);
        matrix._effects.AddRange(Drawing.Child(format, "effectStyleLst")?.Elements() ?? []);

        return matrix.IsEmpty ? null : matrix;
    }

    /// <summary>
    /// The fill a shape's style reference names, wrapped so it reads like shape properties, or
    /// null when the style names none.
    /// </summary>
    /// <remarks>
    /// Wrapped rather than returned bare because the theme's entry <em>is</em> the
    /// <c>a:solidFill</c> or <c>a:gradFill</c>, and every caller looks for one of those among an
    /// element's children. Giving it the same shape as a <c>p:spPr</c> lets it join the chain of
    /// fill sources a shape already walks instead of needing a second code path.
    /// </remarks>
    /// <param name="style">The shape's <c>p:style</c> element, or null.</param>
    /// <param name="theme">The colour scheme the reference's colour resolves against.</param>
    public XElement? Fill(XElement? style, DrawingTheme? theme)
        => Resolve(Drawing.Child(style, "fillRef"), _fills, theme) is { } fill
            ? new XElement(Drawing.Name("spPr"), fill)
            : null;

    /// <summary>
    /// The fill a slide's <c>p:bgRef</c> names, wrapped so it reads like shape properties, or
    /// null when the reference names none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A slide background is stated one of two ways and only one of them is a fill: <c>p:bgPr</c>
    /// carries the fill itself, and <c>p:bgRef</c> carries an index into the theme's
    /// <em>fourth</em> style list, <c>a:bgFillStyleLst</c> — the one every Office theme fills with
    /// a flat, a gradient and a textured version of the same colour so that a deck can pick a
    /// weight without restating it.
    /// </para>
    /// <para>
    /// The index lives in the same numbering as <c>a:fillRef</c>'s and is separated from it by
    /// magnitude rather than by name: <c>Theme::getFillStyle</c>
    /// (<c>oox/source/drawingml/theme.cxx:49-54</c>) sends anything from 1000 up to the background
    /// list with 1000 subtracted, and everything below it to <c>a:fillStyleLst</c>. So
    /// <c>idx="1003"</c> is the third background style, not the thousand-and-third of anything,
    /// and an index past the end of the list is clamped to its last entry rather than dropped —
    /// <c>lclGetStyleElement</c>, the same file.
    /// </para>
    /// <para>
    /// Missing this leaves the slide white, which is invisible to a word count and to a page
    /// count alike, and catastrophic when the theme's colour map makes <c>bg1</c> a dark one:
    /// the deck's white title text is then drawn on white. 60 of the 112 corpus <c>pptx</c> decks
    /// state a <c>p:bgRef</c> somewhere.
    /// </para>
    /// </remarks>
    /// <param name="reference">The slide's, layout's or master's <c>p:bgRef</c>, or null.</param>
    /// <param name="theme">The colour scheme the reference's colour resolves against.</param>
    public XElement? Background(XElement? reference, DrawingTheme? theme)
    {
        if (reference is null) return null;
        if (Drawing.Number(reference, "idx") is not { } index) return null;

        List<XElement> styles = index >= 1000 ? _backgrounds : _fills;
        int position = index >= 1000 ? index - 1000 : index;
        if (styles.Count == 0 || position < 1) return null;

        // Clamped, not rejected: LibreOffice's `lclGetStyleElement` takes the last entry for an
        // index past the end, and themes written by other producers do state one.
        XElement style = styles[Math.Min(position, styles.Count) - 1];

        return Placeholder(reference, theme) is { } placeholder
            ? new XElement(Drawing.Name("spPr"), Substitute(style, placeholder))
            : null;
    }

    /// <summary>
    /// The <c>a:ln</c> a shape's style reference names, or null when the style names none.
    /// </summary>
    /// <param name="style">The shape's <c>p:style</c> element, or null.</param>
    /// <param name="theme">The colour scheme the reference's colour resolves against.</param>
    public XElement? Line(XElement? style, DrawingTheme? theme)
        => Resolve(Drawing.Child(style, "lnRef"), _lines, theme);

    /// <summary>
    /// The <c>a:effectLst</c> an <c>a:effectRef</c> names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Taken without substituting the reference's own colour for <c>phClr</c>, unlike
    /// <see cref="Fill"/> and <see cref="Line"/>. That is LibreOffice's behaviour and its source
    /// says why it is that way — *"TODO: use ph color when applying effect properties"*,
    /// <c>oox/source/drawingml/shape.cxx:1556</c> — and it costs nothing measurable, because the
    /// effect styles of every theme Office ships state a literal black with an <c>a:alpha</c>
    /// rather than a placeholder.
    /// </remarks>
    /// <param name="reference">The shape's <c>a:effectRef</c> element, or null.</param>
    public XElement? Effect(XElement? reference)
    {
        if (Drawing.Number(reference, "idx") is not { } index) return null;
        if (index <= 0 || index > _effects.Count) return null;

        return Drawing.Child(_effects[index - 1], "effectLst");
    }

    /// <summary>
    /// A themed element with a shape's own stated overrides laid over it.
    /// </summary>
    /// <remarks>
    /// This is what makes a shape that states <em>part</em> of a line work. A master connector
    /// reading <c>&lt;a:ln w="57150"/&gt;</c> under an <c>a:lnRef idx="1"</c> means "the theme's
    /// first line style, four and a half points wide": taking the shape's element alone loses
    /// the colour and draws nothing, and taking the theme's alone draws the rule at the theme's
    /// three quarters of a point. LibreOffice reaches the same result by property-set merging
    /// (<c>LineProperties::assignUsed</c>); here the merge is over the XML, one level deep,
    /// which is as deep as the format matrix's own entries go.
    /// </remarks>
    /// <param name="themed">The element resolved from the theme.</param>
    /// <param name="own">The element the shape states, whose every part wins.</param>
    public static XElement Overlay(XElement themed, XElement own)
    {
        XElement merged = new(themed.Name, themed.Attributes().Where(a => !a.IsNamespaceDeclaration));

        foreach (XAttribute attribute in own.Attributes())
        {
            if (attribute.IsNamespaceDeclaration) continue;
            merged.SetAttributeValue(attribute.Name, attribute.Value);
        }

        // A fill kind replaces a fill kind rather than joining it: a shape stating a:noFill over
        // a themed a:solidFill has no line, not two.
        bool ownStatesFill = own.Elements().Any(child => IsFill(child.Name.LocalName));

        foreach (XElement child in themed.Elements())
        {
            bool replaced = own.Elements().Any(theirs => theirs.Name == child.Name)
                            || (ownStatesFill && IsFill(child.Name.LocalName));
            if (!replaced) merged.Add(new XElement(child));
        }

        foreach (XElement child in own.Elements()) merged.Add(new XElement(child));

        return merged;
    }

    private static bool IsFill(string localName) => localName is
        "noFill" or "solidFill" or "gradFill" or "blipFill" or "pattFill" or "grpFill";

    /// <summary>The style at a reference's one-based index, with its <c>phClr</c> substituted.</summary>
    private static XElement? Resolve(XElement? reference, List<XElement> styles, DrawingTheme? theme)
    {
        if (reference is null) return null;
        if (Drawing.Number(reference, "idx") is not { } index) return null;
        if (index <= 0 || index > styles.Count) return null;

        return Placeholder(reference, theme) is { } placeholder
            ? Substitute(styles[index - 1], placeholder)
            : null;
    }

    /// <summary>
    /// The colour a style reference substitutes for the theme entry's <c>phClr</c>.
    /// </summary>
    /// <remarks>
    /// The reference's own child, transforms and all: an <c>a:lnRef</c> naming <c>accent1</c> with
    /// an <c>a:shade val="50000"</c> inside it is a darker accent 1, and that darkening is the
    /// reference's, not the theme entry's.
    /// </remarks>
    private static Colour? Placeholder(XElement reference, DrawingTheme? theme)
    {
        foreach (XElement child in reference.Elements())
        {
            if (DrawingColour.Read(child) is not { } colour) continue;
            if (colour.Resolve(theme) is { } resolved) return resolved;
        }

        return null;
    }

    /// <summary>
    /// Clones a theme style element with every <c>a:schemeClr val="phClr"</c> replaced by a
    /// literal colour.
    /// </summary>
    /// <remarks>
    /// The placeholder's own transforms are kept, because the theme states them: the first line
    /// style of every theme Office ships is <c>phClr</c> under a <c>shade</c> of 95% and a
    /// <c>satMod</c> of 105%, so dropping them draws every themed outline slightly too bright.
    /// Alpha survives too — it is stated on the colour rather than on the fill, and a diagram's
    /// overlapping circles are the same accent at half opacity.
    /// </remarks>
    /// <param name="element">The theme's style element.</param>
    /// <param name="placeholder">The colour to substitute.</param>
    public static XElement Substitute(XElement element, Colour placeholder)
        => (XElement)Replace(element, placeholder);

    private static XNode Replace(XNode node, Colour placeholder)
    {
        if (node is not XElement element) return node;

        if (element.Name == Drawing.Name("schemeClr")
            && Drawing.Attribute(element, "val") == "phClr")
        {
            return new XElement(
                Drawing.Name("srgbClr"),
                new XAttribute("val", $"{placeholder.R:X2}{placeholder.G:X2}{placeholder.B:X2}"),
                Alpha(placeholder),
                element.Elements().Select(child => Replace(child, placeholder)));
        }

        return new XElement(
            element.Name,
            element.Attributes().Where(a => !a.IsNamespaceDeclaration),
            element.Nodes().Select(child => Replace(child, placeholder)));
    }

    /// <summary>The <c>a:alpha</c> a resolved colour needs, or nothing when it is opaque.</summary>
    public static XElement? Alpha(Colour colour)
        => colour.IsOpaque
            ? null
            : new XElement(
                Drawing.Name("alpha"),
                new XAttribute(
                    "val",
                    ((int)Math.Round(colour.A / 255.0 * 100000))
                    .ToString(System.Globalization.CultureInfo.InvariantCulture)));
}
