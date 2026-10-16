using System.Xml.Linq;
using XRay.Office.Ooxml.OfficeMath;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A formula's height is StarMath's, and every arm below is 26.2.4.2's own answer for it.
/// </summary>
/// <remarks>
/// <para>
/// The reference does not lay an OMML formula out as text: it imports the subtree into a StarMath
/// object anchored as-character, so the line takes the <em>object's</em> height. That height is
/// readable without a rasteriser -- <c>soffice --convert-to fodt</c> prints it as the
/// <c>draw:frame</c>'s <c>svg:height</c> -- and the expected value on every arm here is that
/// attribute, converted to points, from a fixture built out of the real document's own parts so
/// that it inherits the same OOXML compatibility defaults.
/// </para>
/// <para>
/// <strong>Why a ladder rather than one assertion.</strong> The heights compose: a fraction is its
/// numerator plus its denominator plus a rule, a script is clamped against a line 40 % of the way
/// up the base. One number for a whole formula says a walk is wrong and not which construct broke,
/// and the constructs are what a later round will change. The fixtures, the sweep and the
/// per-construct table are in <c>probes/mathheight-r165/</c>.
/// </para>
/// <para>
/// <strong>The tolerance is a quarter of a point and it is not slack.</strong> StarMath computes in
/// whole hundredths of a millimetre -- 0.028 pt -- with integer division at every level, so a
/// four-deep construct carries a few units of the reference's own rounding; the two nested-radical
/// arms carry more, because the sign's glyph box is approximated by the height <c>AdaptToY</c> asks
/// for rather than measured from OpenSymbol. Nothing here is fitted: the leaf height and ascent are
/// two measured constants and every other number is <c>SmFormat</c>'s own default.
/// </para>
/// </remarks>
public sealed class OfficeMathBoxTests
{
    /// <summary>How far an arm may sit from 26.2.4.2's own <c>svg:height</c>.</summary>
    private const double Tolerance = 0.25;

    /// <summary>The two arms that need more, and why, is in the class remarks.</summary>
    private static readonly HashSet<string> Radicals = ["rad-rad", "rad-frac"];

    /// <summary>Every construct, and the height 26.2.4.2 gives it, in points.</summary>
    private static readonly Dictionary<string, (string Omml, double Points)> Ladder = new()
    {
        ["acc"] = ("""<m:oMath><m:acc><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:acc></m:oMath>""", 13.351),
        ["acc-frac"] = ("""<m:oMath><m:acc><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e></m:acc></m:oMath>""", 28.460),
        ["acc-tilde"] = ("""<m:oMath><m:acc><m:accPr><m:chr m:val="̃"/></m:accPr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:acc></m:oMath>""", 13.351),
        ["bar-bot"] = ("""<m:oMath><m:bar><m:barPr><m:pos m:val="bot"/></m:barPr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:bar></m:oMath>""", 13.351),
        ["bar-top"] = ("""<m:oMath><m:bar><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:bar></m:oMath>""", 13.351),
        ["box"] = ("""<m:oMath><m:box><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:box></m:oMath>""", 13.351),
        ["delim"] = ("""<m:oMath><m:d><m:dPr></m:dPr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:d></m:oMath>""", 14.797),
        ["delim-frac"] = ("""<m:oMath><m:d><m:dPr></m:dPr><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e></m:d></m:oMath>""", 31.408),
        ["delim-sq"] = ("""<m:oMath><m:d><m:dPr><m:begChr m:val="["/><m:endChr m:val="]"/></m:dPr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:d></m:oMath>""", 14.740),
        ["eqarr-2"] = ("""<m:oMath><m:eqArr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e></m:eqArr></m:oMath>""", 27.298),
        ["eqarr-3"] = ("""<m:oMath><m:eqArr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:e></m:eqArr></m:oMath>""", 41.187),
        ["frac"] = ("""<m:oMath><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 28.460),
        ["frac-lin"] = ("""<m:oMath><m:f><m:fPr><m:type m:val="lin"/></m:fPr><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 13.351),
        ["frac-nest-both"] = ("""<m:oMath><m:f><m:num><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:num><m:den><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:den></m:f></m:den></m:f></m:oMath>""", 58.649),
        ["frac-nest-den"] = ("""<m:oMath><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:den></m:f></m:den></m:f></m:oMath>""", 43.540),
        ["frac-nest-num"] = ("""<m:oMath><m:f><m:num><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:den></m:f></m:oMath>""", 43.540),
        ["frac-nest3"] = ("""<m:oMath><m:f><m:num><m:f><m:num><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:den></m:f></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:den></m:f></m:oMath>""", 58.649),
        ["frac-nobar"] = ("""<m:oMath><m:f><m:fPr><m:type m:val="noBar"/></m:fPr><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 27.298),
        ["frac-scripts"] = ("""<m:oMath><m:f><m:num><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub></m:sSub></m:num><m:den><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sub></m:sSub></m:den></m:f></m:oMath>""", 32.060),
        ["frac-skw"] = ("""<m:oMath><m:f><m:fPr><m:type m:val="skw"/></m:fPr><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 28.460),
        ["leaf-abc"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>abc</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-beta"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>β</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-caps"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>WMTOM</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-digit"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-plus"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a+b</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-rho"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>ρ</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-two-runs"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>y</m:t></m:r></m:oMath>""", 13.351),
        ["leaf-x"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:oMath>""", 13.351),
        ["mat-1x2"] = ("""<m:oMath><m:m><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e></m:mr></m:m></m:oMath>""", 13.351),
        ["mat-2x2"] = ("""<m:oMath><m:m><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e></m:mr><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:mr></m:m></m:oMath>""", 27.694),
        ["mat-2x2-frac"] = ("""<m:oMath><m:m><m:mr><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e></m:mr><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:mr></m:m></m:oMath>""", 42.803),
        ["mat-3x2"] = ("""<m:oMath><m:m><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:e></m:mr><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>c</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:mr><m:mr><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>y</m:t></m:r></m:e><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:e></m:mr></m:m></m:oMath>""", 42.038),
        ["phant"] = ("""<m:oMath><m:phant><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:phant></m:oMath>""", 13.351),
        ["pre"] = ("""<m:oMath><m:sPre><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub><m:sup><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sup><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:sPre></m:oMath>""", 16.838),
        ["rad"] = ("""<m:oMath><m:rad><m:radPr><m:degHide m:val="1"/></m:radPr><m:deg/><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:rad></m:oMath>""", 14.400),
        ["rad-deg"] = ("""<m:oMath><m:rad><m:deg><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>3</m:t></m:r></m:deg><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:rad></m:oMath>""", 15.591),
        ["rad-frac"] = ("""<m:oMath><m:rad><m:radPr><m:degHide m:val="1"/></m:radPr><m:deg/><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e></m:rad></m:oMath>""", 30.898),
        ["rad-rad"] = ("""<m:oMath><m:rad><m:radPr><m:degHide m:val="1"/></m:radPr><m:deg/><m:e><m:rad><m:radPr><m:degHide m:val="1"/></m:radPr><m:deg/><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e></m:rad></m:e></m:rad></m:oMath>""", 15.562),
        ["seq-frac-then-x"] = ("""<m:oMath><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:oMath>""", 28.460),
        ["seq-x-then-frac"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 28.460),
        ["sub"] = ("""<m:oMath><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub></m:sSub></m:oMath>""", 15.137),
        ["sub-deep"] = ("""<m:oMath><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sub><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>y</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub></m:sSub></m:sub></m:sSub></m:oMath>""", 16.157),
        ["sub-of-frac"] = ("""<m:oMath><m:sSub><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub></m:sSub></m:oMath>""", 29.849),
        ["sub-tall-script"] = ("""<m:oMath><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sub><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:sub></m:sSub></m:oMath>""", 24.208),
        ["subsup"] = ("""<m:oMath><m:sSubSup><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>1</m:t></m:r></m:sub><m:sup><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sup></m:sSubSup></m:oMath>""", 16.838),
        ["sup"] = ("""<m:oMath><m:sSup><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sup><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sup></m:sSup></m:oMath>""", 15.052),
        ["sup-deep"] = ("""<m:oMath><m:sSup><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sup><m:sSup><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>y</m:t></m:r></m:e><m:sup><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sup></m:sSup></m:sup></m:sSup></m:oMath>""", 16.101),
        ["sup-tall-base"] = ("""<m:oMath><m:sSup><m:e><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:e><m:sup><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr><m:t>2</m:t></m:r></m:sup></m:sSup></m:oMath>""", 30.161),
        ["sz20"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="40"/></w:rPr><m:t>x</m:t></m:r></m:oMath>""", 13.351),
        ["sz20-frac"] = ("""<m:oMath><m:f><m:num><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="40"/></w:rPr><m:t>a</m:t></m:r></m:num><m:den><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="40"/></w:rPr><m:t>b</m:t></m:r></m:den></m:f></m:oMath>""", 28.460),
        ["sz20-sub"] = ("""<m:oMath><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="40"/></w:rPr><m:t>x</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="40"/></w:rPr><m:t>1</m:t></m:r></m:sub></m:sSub></m:oMath>""", 15.137),
        ["sz8"] = ("""<m:oMath><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/><w:sz w:val="16"/></w:rPr><m:t>x</m:t></m:r></m:oMath>""", 13.351),
    };

    /// <summary>Every arm of the ladder lands on the reference's own answer.</summary>
    [Fact]
    public void EveryConstructMatchesTheReference()
    {
        List<string> wrong = [];

        foreach ((string name, (string omml, double want)) in Ladder)
        {
            double got = Measure(omml);
            double limit = Radicals.Contains(name) ? 0.4 : Tolerance;
            if (Math.Abs(got - want) > limit) wrong.Add($"{name}: {got:F3} against {want:F3}");
        }

        wrong.ShouldBeEmpty();
    }

    /// <summary>A plain row of maths text is one line whatever it says.</summary>
    /// <remarks>
    /// StarMath sets every leaf in its own faces at its own base size, so a capital, a digit and a
    /// Greek letter with a descender are all the same height -- which is why the walk needs no font
    /// metrics of the document's.
    /// </remarks>
    [Fact]
    public void EveryPlainRowIsTheSameHeight()
    {
        double[] heights = [.. Ladder.Where(a => a.Key.StartsWith("leaf-", StringComparison.Ordinal))
                                     .Select(a => Measure(a.Value.Omml))];

        heights.Length.ShouldBeGreaterThan(4);
        heights.ShouldAllBe(h => Math.Abs(h - heights[0]) < 0.001);
    }

    /// <summary>
    /// <c>w:sz</c> is ignored, which is the law that separates this from laying the formula out as
    /// text.
    /// </summary>
    /// <remarks>
    /// Measured on 26.2.4.2 at 8, 12 and 20 pt: <c>x</c> comes back 13.349 pt at all three and
    /// <c>x</c> with a subscript 15.134 pt at both 12 and 20. A reader that honoured the run's size
    /// would have the 20 pt arm two thirds taller, and that is exactly what this tree used to do --
    /// its pitch was 49.350 pt for every shape at the default size and moved with <c>w:sz</c>, so it
    /// was wrong in both directions at once.
    /// </remarks>
    [Theory]
    [InlineData("leaf-x", "sz8")]
    [InlineData("leaf-x", "sz20")]
    [InlineData("sub", "sz20-sub")]
    [InlineData("frac", "sz20-frac")]
    public void TheRunsOwnSizeDoesNotReachTheFormula(string plain, string sized)
        => Measure(Ladder[sized].Omml).ShouldBe(Measure(Ladder[plain].Omml), 0.001);

    /// <summary>A fraction is its two terms and a rule, and nesting one adds another term.</summary>
    [Fact]
    public void AFractionStacksItsTerms()
    {
        double leaf = Measure(Ladder["leaf-x"].Omml);
        double fraction = Measure(Ladder["frac"].Omml);

        fraction.ShouldBeGreaterThan(2 * leaf);
        Measure(Ladder["frac-nest-num"].Omml).ShouldBeGreaterThan(fraction + leaf);
    }

    /// <summary>Anything that is not a formula is measured as nothing.</summary>
    [Fact]
    public void OnlyAFormulaIsMeasured()
    {
        OfficeMathBox.Measure(null).ShouldBeNull();
        OfficeMathBox.Measure(XElement.Parse("<p xmlns='urn:x'/>")).ShouldBeNull();
        OfficeMathBox.Measure(XElement.Parse(
            $"<m:oMath xmlns:m='{Math_}'/>")).ShouldBeNull();
    }

    private const string Math_ = "http://schemas.openxmlformats.org/officeDocument/2006/math";

    private static double Measure(string omml)
    {
        XElement element = XElement.Parse(
            omml.Replace("<m:oMath>",
                $"<m:oMath xmlns:m='{Math_}' "
                + "xmlns:w='http://schemas.openxmlformats.org/wordprocessingml/2006/main'>",
                StringComparison.Ordinal));

        return OfficeMathBox.Measure(element)!.Value.Height.Points;
    }
}
