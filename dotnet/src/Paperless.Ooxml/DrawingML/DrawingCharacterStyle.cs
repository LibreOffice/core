using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// The character properties a DrawingML text run ends up with that extraction does not report:
/// its colour, its size and its typefaces.
/// </summary>
/// <remarks>
/// <para>
/// Kept apart from the emphasis flags <c>DrawingTextBody</c> resolves, because these three are
/// the properties that need the <em>theme</em> to answer — a colour is usually an
/// <c>a:schemeClr</c> and a typeface is usually the indirect <c>+mn-lt</c> — and because they
/// are what a renderer wants rather than what a content tree carries.
/// </para>
/// <para>
/// Every member is nullable and null means "no layer stated one", not "black" or "the default
/// face". That distinction is the whole of the inheritance: a run that states nothing has to
/// fall through to its paragraph, and a paragraph that states nothing to its shape.
/// </para>
/// </remarks>
public sealed record DrawingCharacterStyle
{
    /// <summary>Nothing stated.</summary>
    public static readonly DrawingCharacterStyle None = new();

    /// <summary>The text colour, or null when no layer states one.</summary>
    public Colour? Colour { get; init; }

    /// <summary>The em size, or null when no layer states one.</summary>
    public Length? Size { get; init; }

    /// <summary>The Latin-script typeface, with the theme's indirection already followed.</summary>
    public string? LatinTypeface { get; init; }

    /// <summary>The East Asian typeface.</summary>
    public string? AsianTypeface { get; init; }

    /// <summary>The complex-script typeface.</summary>
    public string? ComplexTypeface { get; init; }

    /// <summary>True when no member is set.</summary>
    public bool IsEmpty
        => Colour is null && Size is null && LatinTypeface is null
           && AsianTypeface is null && ComplexTypeface is null;

    /// <summary>
    /// This style with anything it does not state taken from <paramref name="weaker"/>.
    /// </summary>
    /// <remarks>
    /// Property by property, which is the rule the whole chain runs on: a source stating only a
    /// colour has not cancelled the typeface a weaker source gives it. LibreOffice's
    /// <c>TextCharacterProperties::assignUsed</c> merges the same way, in the opposite
    /// direction — it applies the weaker source first and lets the stronger overwrite.
    /// </remarks>
    /// <param name="weaker">The source further down the chain.</param>
    public DrawingCharacterStyle Over(DrawingCharacterStyle? weaker)
    {
        if (weaker is null || weaker.IsEmpty) return this;

        return new DrawingCharacterStyle
        {
            Colour = Colour ?? weaker.Colour,
            Size = Size ?? weaker.Size,
            LatinTypeface = LatinTypeface ?? weaker.LatinTypeface,
            AsianTypeface = AsianTypeface ?? weaker.AsianTypeface,
            ComplexTypeface = ComplexTypeface ?? weaker.ComplexTypeface,
        };
    }

    /// <summary>
    /// Reads what one <c>a:rPr</c> or <c>a:defRPr</c> states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only <c>a:solidFill</c> is read as a colour. A gradient or picture text fill is a real
    /// thing DrawingML can express and not one a single colour can stand for, so it reads as
    /// unstated and falls through rather than being flattened to its first stop — which would be
    /// a colour the file never asked for.
    /// </para>
    /// <para>
    /// <c>sz</c> is in hundredths of a point, not the half-points WordprocessingML's <c>w:sz</c>
    /// uses: <c>sz="1800"</c> is 18 pt. The two vocabularies sit in the same file, so reading one
    /// with the other's unit is a plausible mistake with a factor of fifty in it.
    /// </para>
    /// </remarks>
    /// <param name="properties">The <c>a:rPr</c> or <c>a:defRPr</c>, or null.</param>
    /// <param name="theme">The theme, for a scheme colour and for typeface indirection.</param>
    public static DrawingCharacterStyle Read(XElement? properties, DrawingTheme? theme)
    {
        if (properties is null) return None;

        return new DrawingCharacterStyle
        {
            Colour = DrawingColour.Read(Drawing.Child(properties, "solidFill")?.Elements().FirstOrDefault())
                ?.Resolve(theme),
            Size = Drawing.Attribute(properties, "sz") is { } size
                   && int.TryParse(size, NumberStyles.Integer, CultureInfo.InvariantCulture, out int hundredths)
                   && hundredths > 0
                ? Length.FromPoints(hundredths / 100.0)
                : null,
            LatinTypeface = Typeface(properties, "latin", theme),
            AsianTypeface = Typeface(properties, "ea", theme),
            ComplexTypeface = Typeface(properties, "cs", theme),
        };
    }

    /// <summary>
    /// Reads a shape's own text style: the <c>a:fontRef</c> of its <c>p:style</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the rung of the inheritance chain that has no element of its own in the text
    /// body. A shape style states four references — line, fill, effect and font — and the font
    /// one carries both a typeface, by index into the theme's font scheme, and a colour, as an
    /// ordinary colour reference inside the element. LibreOffice builds exactly this and nothing
    /// else at <c>oox/source/drawingml/shape.cxx</c>:2239-2253, where the theme's font style
    /// supplies the faces and <c>maPhClr</c> becomes the character fill.
    /// </para>
    /// <para>
    /// <c>idx="none"</c>, and an index the theme cannot answer, contribute no typeface — but the
    /// colour beside it still counts, so the two are read independently.
    /// </para>
    /// </remarks>
    /// <param name="shapeStyle">The <c>p:style</c> (or <c>xdr:style</c>) element, or null.</param>
    /// <param name="theme">The theme, whose colour scheme and font scheme both answer here.</param>
    public static DrawingCharacterStyle FromShapeStyle(XElement? shapeStyle, DrawingTheme? theme)
    {
        XElement? fontReference = Drawing.Child(shapeStyle, "fontRef");
        if (fontReference is null) return None;

        string? index = Drawing.Attribute(fontReference, "idx");

        return new DrawingCharacterStyle
        {
            Colour = DrawingColour.Read(fontReference.Elements().FirstOrDefault())?.Resolve(theme),
            LatinTypeface = theme?.Fonts?.ForReference(index, "latin"),
            AsianTypeface = theme?.Fonts?.ForReference(index, "ea"),
            ComplexTypeface = theme?.Fonts?.ForReference(index, "cs"),
        };
    }

    /// <summary>
    /// The style a run resolves to, over the whole inheritance chain.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The sources, strongest first: the run's own <c>a:rPr</c>; the paragraph's
    /// <c>a:pPr/a:defRPr</c>; the body's <c>a:lstStyle</c> entry for this level; <b>the shape's
    /// own text style</b>; then each inherited level source's <c>a:defRPr</c> — the layout
    /// placeholder, the master placeholder, the master's <c>p:txStyles</c>, the presentation's
    /// <c>p:defaultTextStyle</c>.
    /// </para>
    /// <para>
    /// <b>Where the shape's text style goes is the whole point of this method.</b> It is neither
    /// first nor last: it beats everything the shape inherits and loses to everything the body
    /// states. LibreOffice's <c>TextParagraph::getCharacterStyle</c>
    /// (<c>oox/source/drawingml/textparagraph.cxx</c>:52-67) assigns the master list style, then
    /// the shape's text style, then the body's list style, then the paragraph's <c>defRPr</c>,
    /// each overwriting only what it sets — which is this order read from the other end.
    /// Putting the shape style at either extreme gives the right answer on every shape that
    /// states nothing else, which is most of them, and the wrong one exactly where a deck
    /// bothered to state two.
    /// </para>
    /// </remarks>
    /// <param name="runProperties">The run's <c>a:rPr</c>, or null.</param>
    /// <param name="paragraphProperties">The paragraph's <c>a:pPr</c>, or null.</param>
    /// <param name="bodyListStyle">The text body's own <c>a:lstStyle</c>, or null.</param>
    /// <param name="level">The paragraph's zero-based outline level.</param>
    /// <param name="options">The shape's text style, the theme, and the inherited sources.</param>
    public static DrawingCharacterStyle Resolve(
        XElement? runProperties,
        XElement? paragraphProperties,
        XElement? bodyListStyle,
        int level,
        DrawingTextOptions? options)
    {
        options ??= DrawingTextOptions.Default;
        DrawingTheme? theme = options.Theme;

        DrawingCharacterStyle resolved = Read(runProperties, theme)
            .Over(Read(Drawing.Child(paragraphProperties, "defRPr"), theme))
            .Over(Read(DefaultRunProperties(bodyListStyle, level), theme))
            .Over(options.ShapeTextStyle);

        if (options.InheritedLevelProperties is null) return resolved;

        foreach (XElement inherited in options.InheritedLevelProperties(level))
        {
            resolved = resolved.Over(Read(Drawing.Child(inherited, "defRPr"), theme));
        }

        return resolved;
    }

    /// <summary>The <c>a:defRPr</c> of a list style's entry for a level, or null.</summary>
    private static XElement? DefaultRunProperties(XElement? listStyle, int level)
        => Drawing.Child(DrawingTextBody.LevelProperties(listStyle, level), "defRPr");

    private static string? Typeface(XElement properties, string localName, DrawingTheme? theme)
    {
        string? stated = Drawing.Attribute(Drawing.Child(properties, localName), "typeface");
        if (string.IsNullOrEmpty(stated)) return null;

        // Without a theme an indirect name cannot be followed, and reporting "+mn-lt" as a font
        // family would end the search on a face that does not exist.
        return theme?.Fonts is { } fonts ? fonts.Resolve(stated) : stated[0] == '+' ? null : stated;
    }
}
