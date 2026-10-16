using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>Where a paragraph of shape text sits across its box.</summary>
public enum SheetShapeAlignment
{
    /// <summary>Against the left inset.</summary>
    Left,

    /// <summary>Centred between the insets.</summary>
    Centre,

    /// <summary>Against the right inset.</summary>
    Right,
}

/// <summary>Where a shape's text sits down its box.</summary>
public enum SheetShapeAnchor
{
    /// <summary>Against the top inset.</summary>
    Top,

    /// <summary>Centred between the insets.</summary>
    Middle,

    /// <summary>Against the bottom inset.</summary>
    Bottom,
}

/// <summary>A run of a shape's text: the characters, the size, and the face.</summary>
/// <param name="Text">The characters.</param>
/// <param name="Size">The em size the run states, or the body's default where it states none.</param>
/// <param name="Family">
/// The typeface the run states, with the theme's indirection already followed, or null where it
/// states none and the default face is right.
/// </param>
/// <param name="Bold">
/// Whether the run states <c>b="1"</c>.
/// </param>
/// <param name="Colour">
/// The ink the run states, or null where it states none and the drawing layer's black is right.
/// </param>
/// <remarks>
/// <para>
/// The weight is carried and the slant is not, and the asymmetry is measured rather than tidy: a
/// bold face is a <em>different file</em> with different advances, so a bold run measured in the
/// regular face wraps in the wrong place and is drawn in the wrong ink — <c>Air_Boss_Master_List
/// .xlsx</c>'s note box is one paragraph of <c>b="1"</c> that 26.2.4.2 draws in Carlito-Bold and
/// wraps two lines shorter than we did.
/// </para>
/// <para>
/// <strong>The slant stays uncarried because it has nil reach and that is measured</strong>: over
/// the 64 corpus <c>.xls</c>, 0 of 367 applied <c>TXO</c> formatting runs state an italic, and
/// 26.2.4.2's own resolved view of all nine shape-path workbooks holds 0 italic spans
/// (<c>probes/sheet-wrap-r99</c> §4, N23). The colour does not: the same pair of censuses puts it
/// at 48 stated runs and 119 drawn spans in two workbooks, so it is carried.
/// </para>
/// </remarks>
public readonly record struct SheetShapeRun(
    string Text, Length Size, string? Family = null, bool Bold = false, Colour? Colour = null);

/// <summary>One paragraph of a shape's text.</summary>
/// <remarks>
/// <strong>A paragraph holding no text still carries one run.</strong> A blank paragraph occupies
/// a line, and DrawingML says how tall in <c>a:endParaRPr</c> — the properties the next character
/// typed would take. Carrying them as an empty run rather than as a separate pair of properties
/// lets the painter read the size and face the same way whether or not there is ink.
/// </remarks>
public sealed record SheetShapeParagraph
{
    /// <summary>The runs, in order.</summary>
    public IReadOnlyList<SheetShapeRun> Runs { get; init; } = [];

    /// <summary>How the paragraph sits across the box.</summary>
    public SheetShapeAlignment Alignment { get; init; }

    /// <summary>The paragraph's text, with its runs joined.</summary>
    public string Text => Runs.Count switch
    {
        0 => string.Empty,
        1 => Runs[0].Text,
        _ => string.Concat(Runs.Select(run => run.Text)),
    };

}

/// <summary>
/// The text inside a shape anchored on a sheet.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A text box on a sheet is a drawing, not a cell</strong>, so nothing in the cell path
/// reaches it and it is invisible to every check that walks the grid. Calc reads one through the
/// same drawing layer as a picture — <c>GroupShapeContext::createShapeContext</c> takes
/// <c>sp</c> alongside <c>pic</c> and <c>graphicFrame</c>
/// (<c>sc/source/filter/oox/drawingfragment.cxx:198</c>) — and prints it with
/// <c>PrintDrawingLayer</c> like any other object.
/// </para>
/// <para>
/// <strong>What this carries is what can be drawn, and no more.</strong> A run's size, typeface and
/// weight are carried because all three decide the face, the line height and the wrap, and its
/// colour because the painter draws it; its slant is not, on the census
/// <see cref="SheetShapeRun"/> cites. The typeface arrives already resolved — a
/// <c>a:latin typeface="+mn-lt"</c> has been through the theme's font scheme before it gets here,
/// since taking that attribute literally asks the resolver for a family called <c>+mn-lt</c> and
/// gets whatever fontconfig offers for a name that exists nowhere.
/// </para>
/// </remarks>
public sealed record SheetShapeText
{
    /// <summary>The em size a run that states none, and inherits none, is set at.</summary>
    /// <remarks>
    /// <para>
    /// <strong>Twelve point, not the shape's own eighteen.</strong> A DrawingML shape carries a
    /// default character height of 18 pt (<c>Shape::setDefaults</c>,
    /// <c>oox/source/drawingml/shape.cxx:334</c>) and that is what the exported shape style
    /// states — but it is not what a run inherits. <c>TextBody::insertAt</c> reads the
    /// <em>text cursor's</em> <c>CharHeight</c> before any of the body is inserted
    /// (<c>oox/source/drawingml/textbody.cxx:62</c>) and hands it down as
    /// <c>nDefaultCharHeight</c>, which <c>TextRun::insertAt</c> puts on any run whose own
    /// <c>moHeight</c> is unset (<c>oox/source/drawingml/textrun.cxx:82-85</c>). On a fresh Calc
    /// drawing object that cursor reports the EditEngine pool's own default, 240 twips.
    /// </para>
    /// <para>
    /// Measured rather than derived, because the two candidates are both in the file. A probe
    /// workbook with three text boxes was round-tripped through LibreOffice 24.2.7.2's flat-ODS
    /// export: a box whose only run states no <c>sz</c> comes back as <c>fo:font-size="12pt"</c>,
    /// a box whose body states <c>sz="1100"</c> and whose trailing space states nothing comes back
    /// as 11 pt and 12 pt in two spans, and every one of the three shapes' default paragraph style
    /// states 18 pt while none of their runs does.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round 56 — 12 pt on 26.2.4.2, by two instruments.]
    /// <c>probes/sheets-r56/audit_shapetext.py</c> re-authors that workbook and runs it through
    /// the installed binary twice over: the flat-ODS export gives the bare run
    /// <c>fo:font-size="12pt"</c>, and the <em>rendering</em> — which does not depend on the
    /// exporter agreeing with the layout — gives it an ink-box height of 13.274 pt against
    /// 12.175 for a run stating <c>sz="1100"</c> and 19.926 for one stating <c>sz="1800"</c>.
    /// Twelve over eleven is 13.28 and twelve over eighteen is 13.28, so the rendered size is
    /// 12 pt to three figures on both ratios. <strong>The control ran first</strong>: the 1100
    /// box has to come back 11 pt or nothing else the probe says means anything, and it does. The
    /// 1800 box is there because 18 is the other candidate and a reader that always answered 12
    /// could not otherwise be told from one that read the shape's own default.
    /// </para>
    /// </remarks>
    public static Length DefaultSize { get; } = Length.FromPoints(12);

    /// <summary>The face a run that names none is set in.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The drawing layer's default, not the sheet's.</strong> A shape on a sheet is an
    /// <c>SdrObject</c> whose text lives in the drawing layer's item pool, and
    /// <c>SdrModel::SetTextDefaults</c> seeds that pool with
    /// <c>DefaultFontType::LATIN_TEXT</c> (<c>svx/source/svdraw/svdmodel.cxx</c>:668-669) — which
    /// <c>VCL.xcu</c> heads with <b>Liberation Serif</b>. A cell takes
    /// <c>DefaultFontType::LATIN_SPREADSHEET</c> instead
    /// (<c>sc/source/core/data/docpool.cxx</c>:201-202), which is Liberation <em>Sans</em>. The
    /// two defaults are different fonts and the same workbook uses both, so a shape cannot borrow
    /// the cell face the way a header band correctly does.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 over five corpus workbooks that all carry Excel's slicer-fallback
    /// shape — <c>Part_129_Operators.xlsx</c>, <c>Part_375_Operators.xlsx</c>,
    /// <c>TDA_Smoke-Detectors.xlsx</c>, <c>DynamicBubbleChart.xlsx</c> and
    /// <c>049_Expenses_calculator…xlsx</c>. Its runs name no typeface; the reference draws all
    /// 77 of their spans in <c>LiberationSerif</c> and we drew them in <c>LiberationSans</c>.
    /// </para>
    /// </remarks>
    public const string DefaultFamily = "Liberation Serif";

    /// <summary>The paragraphs, in order.</summary>
    public IReadOnlyList<SheetShapeParagraph> Paragraphs { get; init; } = [];

    /// <summary>The inset from the box's left edge.</summary>
    public Length LeftInset { get; init; } = Length.FromInches(0.1);

    /// <summary>The inset from the box's right edge.</summary>
    public Length RightInset { get; init; } = Length.FromInches(0.1);

    /// <summary>The inset from the box's top edge.</summary>
    public Length TopInset { get; init; } = Length.FromInches(0.05);

    /// <summary>The inset from the box's bottom edge.</summary>
    public Length BottomInset { get; init; } = Length.FromInches(0.05);

    /// <summary>True when a line too long for the box wraps rather than running on.</summary>
    public bool Wraps { get; init; } = true;

    /// <summary>Where the block of lines sits down the box.</summary>
    public SheetShapeAnchor Anchor { get; init; }

    /// <summary>
    /// True when text taller than the box is cut off at the box rather than drawn past it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// DrawingML's <c>a:bodyPr/@vertOverflow</c>, which <c>oox</c> turns into
    /// <c>TextClipVerticalOverflow</c> for both <c>clip</c> and <c>ellipsis</c>
    /// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:85-97</c>); the default, both there
    /// and here, is to let the text run on.
    /// </para>
    /// <para>
    /// <strong>It removes lines rather than masking them.</strong>
    /// <c>SdrTextObj::impDecomposeBlockTextPrimitive</c> builds a clip range of the box's height
    /// (<c>svx/source/svdraw/svdotextdecomposition.cxx:581-624</c>) and hands it to
    /// <c>TextHierarchyBreakupBlockText</c>, whose own comment states the rule: "only text portions
    /// completely inside are to be accepted, so this is different from geometric clipping (which
    /// would allow e.g. upper parts of portions to remain)" (<c>include/svx/svdoutl.hxx:56-59</c>).
    /// So an overflowing line is never drawn at all, which is why it is missing from the reference's
    /// text layer and not merely invisible in it.
    /// </para>
    /// </remarks>
    public bool ClipsVerticalOverflow { get; init; }

    /// <summary>
    /// The <c>a:prstGeom/@prst</c> of the shape the text sits in, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A preset shape states its own text rectangle and it is usually not the bounding
    /// box.</strong> Every preset in the DrawingML catalogue may carry an <c>a:rect</c> of four
    /// guide expressions, and a rounded rectangle's insets it by the corner radius on all four
    /// sides — <c>il = x1 * 29289/100000</c> where <c>x1 = min(w, h) * adj/100000</c>, so a
    /// stadium-shaped button at <c>adj="50000"</c> loses <c>0.1464 * min(w, h)</c> from each edge
    /// before the text insets are taken at all. LibreOffice reads the same table through
    /// <c>EnhancedCustomShape2d::GetTextRect</c> and lays the text out in that rectangle.
    /// </para>
    /// <para>
    /// It decides whether a body overflows, so on a shape that also states
    /// <see cref="ClipsVerticalOverflow"/> it decides whether the text is drawn at all. Measured on
    /// <c>076_Inventory_list_accessibility_guide…xlsx</c>, whose navigation buttons are 204 x 33 pt
    /// <c>roundRect</c>s holding one 16 pt line: the bounding box leaves 25.8 pt of room for an
    /// 18.6 pt line and nothing is clipped, while the preset's own rectangle leaves 16.1 pt and
    /// the reference draws two of the seven.
    /// </para>
    /// </remarks>
    public string? Preset { get; init; }

    /// <summary>
    /// The <c>a:avLst</c> values the shape states for its preset, by guide name.
    /// </summary>
    /// <remarks>
    /// Carried beside <see cref="Preset"/> because the text rectangle is a function of them: the
    /// same <c>roundRect</c> insets its text by nothing at <c>adj="0"</c> and by a seventh of its
    /// shorter side at the maximum.
    /// </remarks>
    public IReadOnlyDictionary<string, double>? Adjustments { get; init; }

    /// <summary>True when there is nothing to draw.</summary>
    public bool IsEmpty
    {
        get
        {
            foreach (SheetShapeParagraph paragraph in Paragraphs)
            {
                if (paragraph.Text.Length > 0) return false;
            }

            return true;
        }
    }
}
