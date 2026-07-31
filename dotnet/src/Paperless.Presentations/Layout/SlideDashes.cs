using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Expands DrawingML's ten preset dash patterns into the dash array a stroke carries.
/// </summary>
/// <remarks>
/// <para>
/// <c>a:prstDash</c> names a pattern; it does not state one. The lengths are a <em>percentage of
/// the line width</em>, which is why a table of literal point values would be wrong on every line
/// that is not one point wide: a <c>dash</c> on a 3 pt pen is four times 3 pt of ink and three
/// times 3 pt of gap.
/// </para>
/// <para>
/// Two steps, both ported. <c>lclConvertPresetDash</c>
/// (<c>oox/source/drawingml/lineproperties.cxx:60-83</c>) turns the name into a count of dots and
/// dashes with their lengths as percentages; <c>XDash::CreateDotDashArray</c>
/// (<c>svx/source/xoutdev/xattr.cxx:503-640</c>) then multiplies each by the line width and lays
/// them out <em>dots first, then dashes</em>, each followed by one gap. So <c>dashDot</c> comes
/// out as a four-width dash, a three-width gap, a one-width dash and a three-width gap — long
/// before short, which is the opposite order from the name.
/// </para>
/// <para>
/// The line cap changes the arithmetic, because Microsoft measures a round or square cap
/// <em>inside</em> the dash and ODF adds it on: <c>lineproperties.cxx:470-479</c> takes 99% off
/// each ink length and gives it to the gap, so a round-capped dashed line has the same period and
/// visibly shorter strokes. A butt cap leaves the numbers alone.
/// </para>
/// </remarks>
public static class SlideDashes
{
    /// <summary>
    /// The width a hairline is treated as when scaling a dash: 26.95 units of 1/100 mm.
    /// </summary>
    /// <remarks>
    /// <c>SMALLEST_DASH_WIDTH</c> (<c>svx/source/xoutdev/xattr.cxx:501</c>). A zero-width pen is a
    /// hairline rather than nothing, and multiplying a percentage by zero would collapse the whole
    /// pattern into a solid line — which is the failure this constant exists to prevent.
    /// </remarks>
    private static readonly Length HairlineDashWidth = Length.FromEmu(9702);

    /// <summary>
    /// The dash array a preset asks for at a given pen width, or null for a solid line.
    /// </summary>
    /// <param name="preset">The <c>a:prstDash/@val</c> value, or null.</param>
    /// <param name="width">The pen width; zero for a hairline.</param>
    /// <param name="capExtendsDash">
    /// True when the line's cap is round or square, in which case the cap is measured inside the
    /// ink and the ink shortens to compensate.
    /// </param>
    public static IReadOnlyList<Length>? Pattern(
        string? preset, Length width, bool capExtendsDash = false)
    {
        if (Preset(preset) is not { } pattern) return null;

        if (capExtendsDash)
        {
            // Not 100, because a zero length reads back as "unstated" and LibreOffice then treats
            // it as the full 100% again (lineproperties.cxx:472).
            if (pattern.DotLength >= 100 || pattern.DashLength >= 100) pattern.Distance += 99;
            if (pattern.DotLength >= 100) pattern.DotLength -= 99;
            if (pattern.DashLength >= 100) pattern.DashLength -= 99;
        }

        Length pen = width > Length.Zero ? width : HairlineDashWidth;
        Length gap = Scale(pattern.Distance, pen);

        List<Length> array = [];
        for (int i = 0; i < pattern.Dots; i++)
        {
            array.Add(Scale(pattern.DotLength, pen));
            array.Add(gap);
        }

        for (int i = 0; i < pattern.Dashes; i++)
        {
            array.Add(Scale(pattern.DashLength, pen));
            array.Add(gap);
        }

        return array.Count > 0 ? array : null;
    }

    /// <summary>A percentage of the pen width, or the pen width itself when the percentage is zero.</summary>
    private static Length Scale(int percent, Length pen)
        => percent > 0 ? Length.FromEmu((long)Math.Round(pen.Emu * percent / 100.0)) : pen;

    /// <summary>How many dots and dashes a preset is, and how long each is as a percentage.</summary>
    private record struct DashPattern(
        int Dots, int DotLength, int Dashes, int DashLength, int Distance);

    /// <summary>
    /// The ten preset patterns, transcribed with the ×100 that turns their units into percentages.
    /// </summary>
    /// <remarks>
    /// <c>solid</c> and an unrecognised name are both null rather than a fallback: LibreOffice
    /// substitutes <c>dash</c> for an unknown token, which would draw a dashed line where the file
    /// asked for something this does not know, and a solid line is the honest answer.
    /// </remarks>
    private static DashPattern? Preset(string? preset) => preset switch
    {
        "dot" => new DashPattern(1, 100, 0, 0, 300),
        "dash" => new DashPattern(1, 400, 0, 0, 300),
        "dashDot" => new DashPattern(1, 400, 1, 100, 300),
        "lgDash" => new DashPattern(1, 800, 0, 0, 300),
        "lgDashDot" => new DashPattern(1, 800, 1, 100, 300),
        "lgDashDotDot" => new DashPattern(1, 800, 2, 100, 300),
        "sysDot" => new DashPattern(1, 100, 0, 0, 100),
        "sysDash" => new DashPattern(1, 300, 0, 0, 100),
        "sysDashDot" => new DashPattern(1, 300, 1, 100, 100),
        "sysDashDotDot" => new DashPattern(1, 300, 2, 100, 100),
        _ => null,
    };
}
