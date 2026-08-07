using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

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
public readonly record struct SheetShapeRun(string Text, Length Size, string? Family = null);

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
/// <strong>What this carries is what can be drawn, and no more.</strong> A run's size and typeface
/// are carried because both decide the line height and the wrap; its weight, slant and colour are
/// not, because nothing downstream would use them. The typeface arrives already resolved — a
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
    /// </remarks>
    public static Length DefaultSize { get; } = Length.FromPoints(12);

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
