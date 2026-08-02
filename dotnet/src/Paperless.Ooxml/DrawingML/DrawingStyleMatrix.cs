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
    private readonly List<XElement> _lines = [];

    private DrawingStyleMatrix()
    {
    }

    /// <summary>True when the theme declared neither list, so nothing can be resolved.</summary>
    public bool IsEmpty => _fills.Count == 0 && _lines.Count == 0;

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
        matrix._lines.AddRange(Drawing.Child(format, "lnStyleLst")?.Elements() ?? []);

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
    /// The <c>a:ln</c> a shape's style reference names, or null when the style names none.
    /// </summary>
    /// <param name="style">The shape's <c>p:style</c> element, or null.</param>
    /// <param name="theme">The colour scheme the reference's colour resolves against.</param>
    public XElement? Line(XElement? style, DrawingTheme? theme)
        => Resolve(Drawing.Child(style, "lnRef"), _lines, theme);

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

        // The placeholder colour is the reference's own child, transforms and all: an
        // `a:lnRef` naming `accent1` with a `a:shade val="50000"` inside it is a darker accent 1,
        // and that darkening is the reference's, not the theme entry's.
        Colour? placeholder = null;
        foreach (XElement child in reference.Elements())
        {
            if (DrawingColour.Read(child) is not { } colour) continue;
            if (colour.Resolve(theme) is { } resolved) { placeholder = resolved; break; }
        }

        return placeholder is null ? null : Substitute(styles[index - 1], placeholder.Value);
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
