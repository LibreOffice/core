using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// An <c>a:gradFill</c> resolved against the box it fills: stops, geometry and centre.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="DrawingFill.ReadGradient"/> stops at the file's own numbers because a gradient's
/// geometry needs the box, and <see cref="GradientGeometry"/> takes a box and knows nothing about
/// DrawingML. This is the join between them, and it is here rather than in either family's reader
/// because an <c>a:gradFill</c> means the same thing in a slide, a sheet and a Word document —
/// only what supplies the box differs.
/// </para>
/// <para>
/// It was in <c>PptxSlideLayout</c>, where a Word document could not reach it, and the cost was
/// visible rather than theoretical: 44 anchored shapes across 10 corpus <c>docx</c> state a
/// gradient, and every one of them drew as nothing. On
/// <c>020_Project_Timeline_Template_Modern_Theme</c> the unfilled shape is the page-wide
/// background rectangle, which is what makes that page's failure so much larger than one missing
/// fill: its title, its three milestone captions and their body text are all set in white, so on
/// the white paper we left behind them <em>none of it can be seen</em>. Four missing strings that
/// are drawn, correctly positioned, in the content stream all along.
/// </para>
/// </remarks>
public static class DrawingGradient
{
    /// <summary>
    /// The paint an <c>a:gradFill</c> draws over a box, or null when the element is not one or
    /// resolves to no colour at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>A path gradient's stop 0 is at the centre, and a linear one's is at the start of its
    /// ramp.</b> That is not obvious from the file and is the mapping most easily got backwards:
    /// LibreOffice <em>reverses</em> the stop list for a path gradient
    /// (<c>fillproperties.cxx:544</c>) before handing it to a model whose first stop paints the
    /// outer edge, so the two reversals cancel and DrawingML's own order is already
    /// centre-outwards. ODF says the opposite and needs the swap.
    /// </para>
    /// <para>
    /// <c>a:path path="shape"</c> — a gradient following a custom outline — is drawn as a
    /// rectangular one, which is what LibreOffice does with it too: its comment says
    /// "XML_rect or XML_shape, but the latter is not implemented".
    /// </para>
    /// <para>
    /// The returned paint carries <see cref="AffineTransform.Identity"/>. A caller drawing into a
    /// rotated space — which is the slide side and not the Word one, whose frames are upright —
    /// substitutes its own with a <c>with</c> expression, as it always did.
    /// </para>
    /// </remarks>
    /// <param name="element">The candidate <c>a:gradFill</c>.</param>
    /// <param name="theme">The theme its <c>a:schemeClr</c> stops resolve against, or null.</param>
    /// <param name="box">The box being filled, in the space the paint will be used in.</param>
    public static GradientPaint? Paint(XElement? element, DrawingTheme? theme, DocRect box)
        => Read(element, theme)?.Paint(box);

    /// <summary>
    /// An <c>a:gradFill</c> resolved as far as it can be without knowing the box, or null when
    /// the element is not one or resolves to no colour at all.
    /// </summary>
    /// <remarks>
    /// The half of <see cref="Paint(XElement?, DrawingTheme?, DocRect)"/> that a reader can do
    /// before layout has placed anything, which is what a word-processing frame needs: its fill
    /// is read when the drawing is, and where on the page it lands is settled much later.
    /// </remarks>
    /// <param name="element">The candidate <c>a:gradFill</c>.</param>
    /// <param name="theme">The theme its <c>a:schemeClr</c> stops resolve against, or null.</param>
    public static GradientDescription? Read(XElement? element, DrawingTheme? theme)
    {
        if (DrawingFill.ReadGradient(element) is not { Stops.Count: > 0 } gradient) return null;

        List<GradientStop> stops = [];
        foreach (DrawingGradientStop stop in gradient.Stops)
        {
            if (stop.Colour.Resolve(theme, placeholder: null) is not { } colour) continue;
            stops.Add(new GradientStop(stop.Position, colour));
        }

        if (stops.Count == 0) return null;

        if (gradient.Path is null)
        {
            return new GradientDescription(GradientKind.Linear, stops)
            {
                AngleDegrees = gradient.Angle ?? 0,
            };
        }

        // a:fillToRect states the inner rectangle the gradient converges on; its centre is what
        // LibreOffice keeps, as (MAX_PERCENT + l - r) / 2, truncated to whole per cent and
        // clamped into the box (fillproperties.cxx:531-537).
        int cx = FocusPerCent(gradient.FillToRect.Left, gradient.FillToRect.Right);
        int cy = FocusPerCent(gradient.FillToRect.Top, gradient.FillToRect.Bottom);

        GradientKind kind = gradient.Path == "circle"
            ? GradientKind.Radial
            : GradientKind.Rectangular;

        return new GradientDescription(kind, stops)
        {
            CentreX = cx / 100.0,
            CentreY = cy / 100.0,
        };
    }

    /// <summary>
    /// One axis of an <c>a:fillToRect</c>'s centre, as the whole number of per cent inside the
    /// filled box that LibreOffice keeps.
    /// </summary>
    /// <remarks>
    /// Both halves matter and both are observable. The <b>clamp</b> is what makes the stock
    /// Office theme's gradient a gradient at all: its <c>fillToRect</c> is
    /// <c>t="-80000" b="180000"</c>, a centre 80% of the box above its own top edge, and
    /// unclamped every point of the box sits past the ramp's last stop and the fill comes out
    /// flat. On the probe deck that is 56.94% of the page's pixels against 0.15%.
    /// <b>Its measured corpus reach is nought</b>, and the distinction is worth keeping: 79 of the
    /// 114 zip-container decks state that exact <c>fillToRect</c>, all of them in a theme's
    /// <c>a:fillStyleLst</c>, and not one of them changed a pixel when this landed — a theme's
    /// third fill style is almost never what a drawn shape resolves to. Correct, tested, and
    /// waiting for a document.
    ///
    /// [24.2.7-audit: VERIFIED 2026-08-21, slides-r59 — the truncation and the clamp both still
    /// hold on 26.2.4.2. Re-run of round 39's own four-arm fixture through the reference's
    /// flat-ODF export: <c>l="0" r="99000"</c> (0.5%) exports <c>draw:cx="0%"</c> and
    /// <c>l="0" r="98000"</c> (1%) exports <c>draw:cx="1%"</c>, so the truncation is intact;
    /// <c>t="-80000" b="180000"</c> exports <c>draw:cy="0%"</c>, so the clamp is intact. The
    /// corner branch the truncation used to feed is gone — a <c>path="circle"</c> whose focus
    /// lands on a corner is a radial gradient and not a diagonal linear one, which that round
    /// established on all four arms.]
    /// </remarks>
    private static int FocusPerCent(double nearInset, double farInset)
    {
        // Back to the file's own thousandths of a per cent before truncating: a stated 98000
        // must not arrive as 0.9999999 and fall on the wrong side of the rounding.
        long near = (long)Math.Round(nearInset * 100000);
        long far = (long)Math.Round(farInset * 100000);

        // Both divisions truncate towards zero, as the C++ integer arithmetic does.
        return (int)Math.Clamp((100000 + near - far) / 2 / 1000, 0, 100);
    }
}
