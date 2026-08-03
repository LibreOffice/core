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

    /// <summary>The largest size any run states, which is what sets the line's height.</summary>
    public Length Size
    {
        get
        {
            Length largest = Length.Zero;
            foreach (SheetShapeRun run in Runs)
            {
                if (run.Size > largest) largest = run.Size;
            }

            return largest;
        }
    }

    /// <summary>The face the paragraph is set in, or null where no run names one.</summary>
    /// <remarks>
    /// The <em>first</em> face any run states, where <see cref="Size"/> takes the largest. The two
    /// rules differ because the quantities differ: a line's height is decided by its tallest run,
    /// and its face is not a maximum of anything. A paragraph mixing faces is drawn wholly in the
    /// first, which is the same single-face approximation the rest of this path makes and is
    /// exact for every text box in the corpus — each states one face for the whole body.
    /// </remarks>
    public string? Family
    {
        get
        {
            foreach (SheetShapeRun run in Runs)
            {
                if (!string.IsNullOrWhiteSpace(run.Family)) return run.Family;
            }

            return null;
        }
    }
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
    /// <summary>The default em size a run that states none is set at.</summary>
    /// <remarks>
    /// DrawingML's own default, <c>1800</c> in hundredths of a point
    /// (<c>oox/source/drawingml/textcharacterproperties.cxx</c> leaves the size unset and the
    /// default character properties supply 18 pt).
    /// </remarks>
    public static Length DefaultSize { get; } = Length.FromPoints(18);

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
