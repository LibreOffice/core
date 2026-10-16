using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;

namespace XRay.Office.Presentations.Ooxml;

/// <summary>
/// A slide shape whose body carries an <c>a:prstTxWarp</c>: WordArt, drawn as curves.
/// </summary>
/// <remarks>
/// <para>
/// <c>oox/source/drawingml/shape.cxx:2202-2211</c> puts such a shape into text-path mode and
/// <c>EnhancedCustomShapeEngine::render2</c> then replaces it outright with the object
/// <c>EnhancedCustomShapeFontWork::CreateFontWork</c> builds. So the box is not drawn, its own fill
/// and pen are not used, its shadow is dropped — <c>CreateSdrObjectFromParagraphOutlines</c> puts
/// <c>makeSdrShadowItem(false)</c> on the result, commented <em>"#i37011# NO shadow for FontWork
/// geometry"</em> — and the words leave the text layer.
/// </para>
/// <para>
/// <strong>The colour comes from the text, not from the shape.</strong>
/// <c>lcl_copyCharPropsToShape</c> (<c>shape.cxx:721-905</c>) copies the first non-empty run's fill
/// and outline onto the shape before any of that happens, because "MS Office has e.g. fill and
/// stroke of WordArt in the character properties, LibreOffice uses shape properties". It matters on
/// the one corpus deck that bends anything: every one of its four dial labels states
/// <c>&lt;a:noFill/&gt;</c> on the shape and a white <c>a:solidFill</c> on the run, so taking the
/// shape's fill would draw nothing at all.
/// </para>
/// <para>
/// This is <em>not</em> the Writer path. There the fill also comes from the run, but through
/// <c>w14:textFill</c> and with the shape restricted to a rectangle; see
/// <c>XRay.Office.WordProcessing.Ooxml.DocxFontwork</c>.
/// </para>
/// </remarks>
internal static class SlideFontwork
{
    /// <summary>What a warped body draws: its curves and the paint they take.</summary>
    /// <param name="Outline">The curves in the shape's own coordinates, or null when it is not warped.</param>
    /// <param name="Fill">What fills them.</param>
    internal readonly record struct Drawing(GraphicsPath? Outline, Paint? Fill);

    /// <summary>The warped outlines of a body, or nothing when it is ordinary text.</summary>
    /// <param name="body">The shape's text body.</param>
    /// <param name="size">The shape's own rectangle, which the warp is fitted into.</param>
    /// <param name="fonts">The resolver, for the face the first run names.</param>
    public static Drawing Read(SlideTextBody? body, DocSize size, SlideFonts fonts)
    {
        if (body is not { IsTextPath: true }) return default;
        if (size.IsEmpty) return default;

        List<string> lines = [];
        SlideParagraph? firstParagraph = null;
        SlideTextRun? firstRun = null;

        foreach (SlideParagraph paragraph in body.Paragraphs)
        {
            // A soft break inside a paragraph is a line of its own here, as it is in the
            // reference: `a:br` reaches EditEngine as a break and `InitializeFontWorkData`
            // (`EnhancedCustomShapeFontWork.cxx:127-146`) cuts the paragraph at every one.
            foreach (string line in paragraph.Text.Split(LineSeparator)) lines.Add(line);

            if (firstRun is not null) continue;

            foreach (SlideTextRun run in paragraph.Runs)
            {
                if (IsBlank(paragraph.Text, run)) continue;

                firstParagraph = paragraph;
                firstRun = run;
                break;
            }
        }

        if (firstRun is not { } stated || firstParagraph is not { } owner) return default;

        (OpenTypeFace? face, FontReference? _) =
            fonts.Resolve(stated.Typeface, stated.Weight, stated.IsItalic);

        if (face is null) return default;

        GraphicsPath? outline = Fontwork.Outline(new FontworkRequest
        {
            Preset = body.WarpPreset,

            // ODF names the Fontwork type and states its adjustments in WordArt units directly, so
            // neither the `prst` map nor the per-guide conversion applies to one. See
            // `SlideTextBody.WarpFontworkType`.
            FontworkType = body.WarpFontworkType,
            Adjustments = body.WarpAdjustments,
            AdjustmentValues = body.WarpAdjustmentValues,
            KeepsFontSize = body.WarpKeepsFontSize,

            // `PROP_FromWordArt` marks a shape that came from a binary WordArt object, which a
            // DrawingML shape tree never does; the arch family therefore keeps its stated size.
            FromWordArt = false,
            Box = size,
            Lines = lines,
            Face = face,
            FontSize = stated.Size,
            Alignment = Alignment(owner.Alignment),
        });

        return outline is null ? default : new Drawing(outline, Paint.Solid(stated.Colour));
    }

    /// <summary>The soft break <c>a:br</c> is read as.</summary>
    private const char LineSeparator = '\u2028';

    /// <summary>
    /// Whether a run is one the reference skips when it looks for the shape's character style.
    /// </summary>
    /// <remarks>
    /// Empty, a single space and a single no-break space are all passed over
    /// (<c>shape.cxx:748-756</c>), because a body often opens with a spacer run whose colour is
    /// not the colour anyone sees.
    /// </remarks>
    private static bool IsBlank(string text, SlideTextRun run)
    {
        if (run.Length <= 0 || run.Start < 0 || run.End > text.Length) return true;
        if (run.Length > 1) return false;

        char only = text[run.Start];
        return only is ' ' or '\u00A0';
    }

    /// <summary>The paragraph's alignment, as the shape's text anchor.</summary>
    /// <remarks>
    /// <c>shape.cxx:846-860</c>. A paragraph stating none is treated as left rather than as the
    /// body's default, which is the reference's own <c>ParagraphAdjust_LEFT</c> initialiser.
    /// </remarks>
    private static FontworkAlignment Alignment(TextAlignment alignment) => alignment switch
    {
        TextAlignment.Centre => FontworkAlignment.Centre,
        TextAlignment.End => FontworkAlignment.Right,
        _ => FontworkAlignment.Left,
    };
}
