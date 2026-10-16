using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The one colour a <c>patternFill</c> paints.
/// </summary>
/// <remarks>
/// <para>
/// Calc has no hatched cell background, so <c>Fill::finalizeImport</c>
/// (<c>sc/source/filter/oox/stylesbuffer.cxx</c>:1978-2056) does not draw the pattern: it
/// <strong>mixes</strong> the pattern colour into the fill colour by a weight the pattern name
/// alone decides, and stores the single blended colour. <c>lclGetMixedColor</c> (<c>:1846-1856</c>)
/// is <c>(pattern − fill) × alpha / 0x80 + fill</c> per component, in integer arithmetic, and
/// <c>alpha</c> comes from the seventeen-entry switch at <c>:2014-2035</c>. A solid fill is the
/// same arithmetic at <c>alpha = 0x80</c>, which returns the pattern colour exactly — so the
/// solid case is not a special case, it is the general one at full weight.
/// </para>
/// <para>
/// Both readers used to take one of the two colours whole. That is right for a solid fill and
/// wrong for every hatch: on <c>072_Gantt_project_planner</c> the three <c>lightUp</c> rules that
/// draw the planned, actual and overrun bands each state the <em>same</em> foreground — the
/// theme's accent — over three different backgrounds, so taking the foreground drew all three in
/// one dark colour indistinguishable from the solid rule above them. At <c>lightUp</c>'s
/// <c>alpha = 0x20</c> they come out <c>#DCD5DC</c>, <c>#D6BCA8</c> and <c>#B5A1B5</c>, which is
/// what 26.2.4.2 paints.
/// </para>
/// <para>
/// <strong>An automatic colour is not the absence of one.</strong> The pattern colour falls back
/// to the window <em>text</em> colour and the fill colour to the window background — black and
/// white — and a <c>&lt;bgColor auto="1"/&gt;</c> still counts as stated, which is what decides
/// the differential branch below.
/// </para>
/// </remarks>
internal static class XlsxPatternFill
{
    /// <summary>
    /// The colour a fill paints, or null when it paints nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <paramref name="differential"/> selects the <c>mbDxf</c> branch at
    /// <c>stylesbuffer.cxx</c>:1985-2002, which exists because a <c>dxf</c>'s three parts are
    /// each optional and a <c>PatternFillModel</c> built for one starts with all three marked
    /// <em>unused</em> (<c>:1753-1762</c>). It does three things in order: a stated background
    /// under no pattern or a solid one is <strong>moved onto the pattern colour</strong> and the
    /// pattern forced solid, which is why a <c>dxf</c> states its fill in <c>bgColor</c> where a
    /// cell states it in <c>fgColor</c>; a solid pattern stating neither colour is turned off
    /// outright; and everything else is left alone.
    /// </para>
    /// <para>
    /// For an ordinary cell all three parts are used from the start, so an absent
    /// <c>patternType</c> is <c>none</c> and paints nothing while an absent colour is automatic.
    /// </para>
    /// </remarks>
    /// <param name="fill">The <c>fill</c> element, or null.</param>
    /// <param name="palette">The workbook's palette, for the colours the fill names.</param>
    /// <param name="differential">True for a <c>dxf</c>'s fill, false for a <c>fills</c> entry.</param>
    public static Colour? Resolve(XElement? fill, XlsxPalette palette, bool differential)
    {
        ArgumentNullException.ThrowIfNull(palette);

        XElement? pattern = Xlsx.Child(fill, "patternFill");
        if (pattern is null) return null;

        XElement? foregroundElement = Xlsx.Child(pattern, "fgColor");
        XElement? backgroundElement = Xlsx.Child(pattern, "bgColor");

        // `mnPattern` starts at `XML_none`, so a `dxf` stating no `patternType` paints nothing
        // unless the swap below turns it solid. **An absent attribute and a stated `none` are not
        // the same thing here**: the swap's test is `!mbPatternUsed || mnPattern == XML_solid`
        // and `mbPatternUsed` is set by the attribute's mere presence, so `patternType="none"`
        // beside a `bgColor` does not swap and paints nothing. Folding the two together made this
        // tree paint a black cell for `<patternFill patternType="none"><bgColor auto="1"/>`, which
        // is what two corpus `dxf` state and what the round's confinement sweep caught.
        string? type = Xlsx.Attribute(pattern, "patternType");

        Colour? foreground = palette.Read(foregroundElement);
        Colour? background = palette.Read(backgroundElement);

        if (differential)
        {
            if (backgroundElement is not null && type is null or "solid")
            {
                foreground = background;
                background = null;
                type = "solid";
            }
            else if (backgroundElement is null && foregroundElement is null && type is "solid")
            {
                return null;
            }
        }

        if (type is null or "none") return null;

        return Mix(foreground ?? Colour.Black, background ?? Colour.White, AlphaOf(type));
    }

    /// <summary>How much of the pattern colour shows, out of <c>0x80</c>.</summary>
    /// <remarks>
    /// <c>stylesbuffer.cxx</c>:2014-2035. An unrecognised name keeps the initial <c>0x80</c>,
    /// which draws the pattern colour whole — the same answer the readers gave before this
    /// existed, so a pattern nobody has heard of does not regress.
    /// </remarks>
    private static int AlphaOf(string type) => type switch
    {
        "darkDown" or "darkGrid" or "darkHorizontal" or "darkUp" or "darkVertical"
            or "mediumGray" => 0x40,
        "darkGray" or "darkTrellis" => 0x60,
        "gray0625" => 0x08,
        "gray125" => 0x10,
        "lightDown" or "lightGray" or "lightHorizontal" or "lightUp" or "lightVertical" => 0x20,
        "lightGrid" => 0x38,
        "lightTrellis" => 0x30,
        _ => 0x80,
    };

    /// <summary>
    /// <c>lclGetMixedColor</c>: integer, per component, and truncating towards zero.
    /// </summary>
    /// <remarks>
    /// C++ integer division truncates towards zero rather than flooring, so the sign of
    /// <c>pattern − fill</c> decides which way a component rounds; <c>int</c> division in C#
    /// does the same, which is why this is written as the source writes it rather than through
    /// a double.
    /// </remarks>
    private static Colour Mix(Colour pattern, Colour fill, int alpha)
        => new(
            (byte)Component(pattern.R, fill.R, alpha),
            (byte)Component(pattern.G, fill.G, alpha),
            (byte)Component(pattern.B, fill.B, alpha));

    private static int Component(int pattern, int fill, int alpha)
        => (((pattern - fill) * alpha) / 0x80) + fill;
}
