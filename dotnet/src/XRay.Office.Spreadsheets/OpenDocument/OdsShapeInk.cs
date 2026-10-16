using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// What an ODF sheet shape's graphic style declares for its fill and its outline.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The graphic style is already being read for the box properties, and this is four more
/// attributes off the same element.</strong> <see cref="OdsShapeText"/> resolves the insets, the
/// wrap, both text-area alignments and <c>style:overflow-behavior</c> through
/// <c>draw:style-name</c>; <c>draw:fill</c>, <c>draw:fill-color</c>, <c>draw:stroke</c>,
/// <c>svg:stroke-color</c> and <c>svg:stroke-width</c> sit beside them.
/// </para>
/// <para>
/// <strong>What it is worth is not the shapes that carry text.</strong> Censused over the 307
/// converted <c>.ods</c> (<c>probes/sheet-fill-r84/census-ods.py</c>), resolving each shape's
/// style through its parent chain: 589 shapes state <c>draw:fill="solid"</c> and 490 state
/// <c>draw:stroke="solid"</c>, and <b>444 custom shapes in 26 documents carry ink and no text at
/// all</b>. Those were answered null by the reader and therefore drawn nowhere <em>and</em> left
/// out of the printed block — see <see cref="OdsDrawings"/>.
/// </para>
/// <para>
/// <strong>Three of ODF's five fill kinds are left, and the reach of each is measured rather than
/// assumed.</strong> <c>gradient</c> reaches 32 shapes and <c>bitmap</c> 14, and neither can be
/// resolved from here: the <c>draw:gradient</c> and <c>draw:fill-image</c> tables are read by
/// <c>OdpFills</c>, which lives in <c>XRay.Office.Presentations</c> and is a sibling of this library
/// rather than below it. <c>hatch</c> appears not at all. Each leaves the shape unfilled rather
/// than painting a wrong flat colour, and all three are in the module's TODO.
/// </para>
/// <para>
/// <strong>An absent <c>draw:fill</c> is not a fill.</strong> The drawing layer's own pool default
/// is a solid blue, which no ODF file ever means — LibreOffice's exporter states the property on
/// every shape it writes, so the 78 shapes here that state neither are <c>draw:control</c>
/// elements whose style is the form's rather than a shape's. Answering nothing for them is the
/// same choice <c>OdpSlideLayout.Fill</c> makes.
/// </para>
/// </remarks>
internal static class OdsShapeInk
{
    /// <summary>One shape's resolved interior and outline.</summary>
    /// <param name="Fill">The flat interior colour, or null.</param>
    /// <param name="Stroke">The outline colour, or null.</param>
    /// <param name="StrokeWidth">How wide the outline is; zero for a hairline.</param>
    /// <param name="Preset">The preset the ink is painted through, or null for the box.</param>
    public readonly record struct Ink(
        Colour? Fill, Colour? Stroke, Length StrokeWidth, string? Preset)
    {
        /// <summary>True when there is something to paint.</summary>
        public bool HasInk => Fill is not null || Stroke is not null;
    }

    /// <summary>Reads one shape's ink from its graphic style.</summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="shape">The <c>draw:</c> element.</param>
    public static Ink Read(OdfStyles styles, XElement shape)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(shape);

        string? graphicStyle = shape.Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value;

        Colour? fill = Graphic(styles, graphicStyle, OdfNamespaces.Draw, "fill").Is("solid")
            ? Graphic(styles, graphicStyle, OdfNamespaces.Draw, "fill-color").AsColour()
            : null;

        // `draw:stroke` tells none from solid from dashed. A dash names a `draw:stroke-dash`
        // element whose pattern is not read yet, so it is drawn solid rather than not at all —
        // two shapes in the whole converted column state one.
        OdfProperty stroke = Graphic(styles, graphicStyle, OdfNamespaces.Draw, "stroke");
        bool outlined = stroke.HasValue && !stroke.Is("none");

        Colour? line = outlined
            ? Graphic(styles, graphicStyle, OdfNamespaces.SvgCompatible, "stroke-color").AsColour()
              ?? Colour.Black
            : null;

        Length width = outlined
            ? Graphic(styles, graphicStyle, OdfNamespaces.SvgCompatible, "stroke-width").AsLength()
              ?? Length.Zero
            : Length.Zero;

        string? preset = Preset(shape);

        return new Ink(fill, line, width, preset == "rect" ? null : preset);
    }

    /// <summary>
    /// The DrawingML preset an ODF enhanced geometry names, or null when it names none this
    /// tree's geometry table knows.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A custom shape LibreOffice imported from OOXML keeps the preset's name with an
    /// <c>ooxml-</c> prefix — <c>ooxml-roundRect</c>, <c>ooxml-star5</c> — so stripping the prefix
    /// names the same entry <see cref="XRay.Office.Ooxml.DrawingML.CustomShapeGeometry"/> holds. A
    /// shape drawn in LibreOffice itself carries a native name instead (<c>mso-spt202</c> is the
    /// text box, 33 of them in the converted column), and an unknown name falls back to the box.
    /// </para>
    /// <para>
    /// The adjustment values are deliberately not carried across. ODF states them as
    /// <c>draw:modifiers</c> in the shape's own coordinate space, where DrawingML's guides are
    /// hundred-thousandths, so the preset is resolved at its default adjustment.
    /// </para>
    /// </remarks>
    /// <param name="shape">The <c>draw:</c> element.</param>
    public static string? Preset(XElement shape)
    {
        ArgumentNullException.ThrowIfNull(shape);

        XElement? geometry = shape.Element(XName.Get("enhanced-geometry", OdfNamespaces.Draw));
        if (geometry is null) return null;
        if (geometry.Attribute(XName.Get("type", OdfNamespaces.Draw))?.Value
            is not { Length: > 0 } type)
        {
            return null;
        }

        const string prefix = "ooxml-";
        return type.StartsWith(prefix, StringComparison.Ordinal) ? type[prefix.Length..] : null;
    }

    private static OdfProperty Graphic(
        OdfStyles styles, string? graphicStyle, string propertyNamespace, string name)
        => styles.ResolveProperty(
            graphicStyle, OdfStyleFamily.Graphic, OdfPropertyKind.Graphic, propertyNamespace, name);
}
