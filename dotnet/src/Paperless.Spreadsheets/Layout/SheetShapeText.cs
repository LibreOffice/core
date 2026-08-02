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

/// <summary>A run of a shape's text: the characters, and the size they are set at.</summary>
/// <param name="Text">The characters.</param>
/// <param name="Size">The em size the run states, or the body's default where it states none.</param>
public readonly record struct SheetShapeRun(string Text, Length Size);

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
/// <strong>What this carries is what can be drawn, and no more.</strong> The furniture face is the
/// one face <see cref="SheetBandText"/> resolves, so a run's typeface, weight and slant are read
/// by nobody and are not modelled; the size is, because it decides both the line height and the
/// wrap and a body mixing 7 pt and 12 pt runs lays out visibly wrongly without it. That is a real
/// limitation rather than a design: it is recorded in the module's TODO beside the cell engine's
/// own single-face restriction, which is the same gap and will be closed by the same work.
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
