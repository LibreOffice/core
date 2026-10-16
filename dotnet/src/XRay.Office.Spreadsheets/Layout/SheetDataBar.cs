using XRay.Office.Core.Graphics;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The bar a <c>dataBar</c> conditional format draws over one cell, already resolved to
/// percentages of that cell's own width.
/// </summary>
/// <remarks>
/// <para>
/// The reference splits the same job in the same place. <c>ScDataBarFormat::GetDataBarInfo</c>
/// (<c>sc/source/core/data/colorscale.cxx</c>:968-1094) turns the rule, the cell's value and the
/// numbers in the rule's range into an <c>ScDataBarInfo</c> holding a length and a zero position
/// as percentages and a colour; <c>drawDataBars</c> (<c>sc/source/ui/view/output.cxx</c>:883-953)
/// is the only code that knows where the cell is. Everything here is the first half's output, so
/// the drawing side needs the cell rectangle and nothing else.
/// </para>
/// <para>
/// A conditional <em>bar</em> and not a conditional fill: it covers part of the cell, it is drawn
/// over whatever the cell's own background is rather than replacing it, and — like the colour
/// scale beside it — it must never reach <see cref="SheetDecorationArea"/>, because a rule stated
/// over a million rows would otherwise decide how far the sheet prints.
/// </para>
/// </remarks>
public readonly record struct SheetDataBar
{
    /// <summary>The colour the bar is painted in.</summary>
    /// <remarks>
    /// The rule's positive colour, or — for a value below zero — its <c>x14:negativeFillColor</c>
    /// if it states one and <c>COL_LIGHTRED</c> if it does not.
    /// <para>
    /// <c>mbNeg</c> does <em>not</em> mean "a negative colour was stated". It is
    /// <c>ScDataBarFormatData</c>'s own default of <strong>true</strong>
    /// (<c>sc/inc/colorscale.hxx</c>:107), the OOXML importer only ever sets it true again
    /// (<c>condformatbuffer.cxx</c>:1658-1664), and the single place in <c>sc/</c> that clears it
    /// is the <em>ODF</em> importer (<c>sc/source/filter/xml/xmlcondformat.cxx</c>:483). So the
    /// fallback at <c>GetDataBarInfo</c>'s <c>colorscale.cxx</c>:1073-1085 — the colour at
    /// <c>:1082</c> — is not merely reachable from SpreadsheetML, it is the usual answer there.
    /// Measured at 26.2.4.2 on <c>sheet-cf-data-bar-default-negative.xlsx</c>.
    /// </para>
    /// </remarks>
    public Colour Colour { get; init; }

    /// <summary>
    /// How far the bar reaches from the axis, as a percentage of the cell's usable width.
    /// </summary>
    /// <remarks>
    /// Signed: negative reaches left of <see cref="Zero"/> and positive right of it, and zero
    /// draws nothing at all — <c>drawDataBars</c> returns before painting anything, which is why
    /// a cell sitting exactly on an automatic minimum shows no bar.
    /// </remarks>
    public double Length { get; init; }

    /// <summary>Where the axis sits, as a percentage of the cell's usable width from its left.</summary>
    public double Zero { get; init; }

    /// <summary>The colour of the dashed axis line, drawn only when <see cref="HasAxis"/>.</summary>
    public Colour AxisColour { get; init; }

    /// <summary>Whether the cell's own value is still drawn over the bar.</summary>
    /// <remarks>
    /// <c>showValue="0"</c> on the rule. <c>ScOutputData::DrawStrings</c> clears <c>bDoCell</c>
    /// for such a cell (<c>sc/source/ui/view/output2.cxx</c>:1691-1698), so the number is not
    /// drawn at all — not merely hidden behind the bar.
    /// </remarks>
    public bool ShowValue { get; init; }

    /// <summary>Whether a gradient was asked for, which this tree paints as a solid bar.</summary>
    /// <remarks>
    /// <para>
    /// True is <c>ScDataBarFormatData</c>'s own default and false is what every corpus rule
    /// states, through <c>x14:dataBar/@gradient="0"</c>. The reference's true branch is
    /// <c>DrawGradient</c> with a linear gradient from the bar's colour to <c>COL_TRANSPARENT</c>
    /// and 255 steps; its PDF of <c>sheet-cf-data-bar-lengths.xlsx</c> comes back as 209 slices
    /// fading from <c>#2e75b6</c> to white, a count VCL derives from the device's own pixel width
    /// and which nothing here can reproduce. **0 of the 9 corpus <c>dataBar</c> rules state it**,
    /// so it is recorded rather than modelled and such a bar is painted solid.
    /// </para>
    /// </remarks>
    public bool Gradient { get; init; }

    /// <summary>
    /// Whether the dashed vertical axis is drawn, which needs a zero strictly inside the cell.
    /// </summary>
    /// <remarks>
    /// <c>if(!(mnZero &amp;&amp; mnZero != 100)) return;</c> — a bar whose range holds no negative
    /// value has its zero at the left edge and draws none, and one entirely below zero has it at
    /// the right edge and draws none either.
    /// </remarks>
    public bool HasAxis => Zero != 0 && Zero != 100;
}
