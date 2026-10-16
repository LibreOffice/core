using System.Globalization;
using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Presentations.OpenDocument;

/// <summary>
/// The Fontwork a <c>draw:enhanced-geometry</c> states: ODF's own spelling of WordArt.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is the format the model is native to, and it shows in how little there is to
/// read.</strong> DrawingML names a warp <c>textArchUpCurve</c> and the OOXML filter maps that onto
/// a LibreOffice type through <c>presetgeometrynames.cxx</c>, converts each adjustment guide from a
/// per-mille or a 1/60000 degree into WordArt's units, and derives <c>ScaleX</c> from the preset's
/// name. ODF states all three outright: <c>draw:type="fontwork-arch-up-curve"</c> is the type
/// <see cref="FontworkPresets"/> is keyed by, <c>draw:modifiers</c> is already in the 21600 viewbox
/// the tables are written in, and <c>draw:text-path-scale</c> is <c>TextPathScaleX</c> itself.
/// </para>
/// <para>
/// <c>xmloff/source/draw/ximpcustomshape.cxx:1136-1150</c> is the reference's reader:
/// <c>draw:text-path</c>, <c>draw:text-path-mode</c>, <c>draw:text-path-scale</c> and
/// <c>draw:text-path-same-letter-heights</c> become the four members of the <c>TextPath</c>
/// property that <c>EnhancedCustomShapeFontWork.cxx</c> then reads.
/// </para>
/// <para>
/// <strong>Two of the four are read here and two are not, deliberately.</strong>
/// <c>draw:text-path-mode</c> decides between <c>NORMAL</c>, <c>PATH</c> and <c>SHAPE</c>, and
/// <c>EnhancedCustomShapeFontWork</c> never consults it — the fit is decided by the number of rails
/// the geometry makes and by <c>ScaleX</c>. <c>draw:text-path-same-letter-heights</c> is honoured,
/// at <c>EnhancedCustomShapeFontWork.cxx:488</c>, and is not implemented here; LibreOffice writes
/// it only when true, and it is true on nothing this project has measured — not in the corpus,
/// which holds no ODF file at all, and not on any of the five binary Escher WordArt shapes it does
/// hold, every one of which leaves bit 0x80 of <c>DFF_Prop_gtextFStrikethrough</c> clear.
/// </para>
/// </remarks>
internal static class OdfFontwork
{
    /// <summary>What an ODF shape says about its warp, or nothing when it states none.</summary>
    /// <param name="FontworkType">
    /// The LibreOffice Fontwork type, straight from <c>draw:type</c>. Null-checked against
    /// <see cref="FontworkPresets"/> before it is returned, so a <c>draw:type</c> of
    /// <c>non-primitive</c> — which is what LibreOffice writes for a shape a user drew rather than
    /// chose — reads as "not Fontwork" rather than as an unknown preset.
    /// </param>
    /// <param name="Adjustments">Its <c>draw:modifiers</c>, in WordArt units.</param>
    /// <param name="KeepsFontSize"><c>draw:text-path-scale="shape"</c>.</param>
    public readonly record struct Warp(
        string FontworkType, IReadOnlyList<double>? Adjustments, bool KeepsFontSize);

    /// <summary>The warp a <c>draw:enhanced-geometry</c> states, or null.</summary>
    public static Warp? Read(XElement? geometry)
    {
        if (geometry is null) return null;
        if (Attribute(geometry, "text-path") is not "true") return null;

        string? type = Attribute(geometry, "type");
        if (type is null || FontworkPresets.Find(type) is null) return null;

        return new Warp(type, Modifiers(geometry), Attribute(geometry, "text-path-scale") is "shape");
    }

    /// <summary>The <c>draw:modifiers</c> list, which is space-separated and may be absent.</summary>
    /// <remarks>
    /// Absent means "the preset's own defaults", which <see cref="FontworkGeometry"/> already
    /// applies to a null list. A value that will not parse is dropped rather than defaulted to
    /// zero: a partial list still fills the rest from the defaults, where a zero does not.
    /// </remarks>
    private static List<double>? Modifiers(XElement geometry)
    {
        if (Attribute(geometry, "modifiers") is not { Length: > 0 } stated) return null;

        List<double> values = [];
        foreach (string part in stated.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            if (!double.TryParse(part, NumberStyles.Float, CultureInfo.InvariantCulture, out double v))
            {
                break;
            }

            values.Add(v);
        }

        return values.Count > 0 ? values : null;
    }

    private static string? Attribute(XElement element, string name)
        => element.Attribute(XName.Get(name, OdfNamespaces.Draw))?.Value;
}
