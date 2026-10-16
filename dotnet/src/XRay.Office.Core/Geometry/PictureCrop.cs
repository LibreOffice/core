using XRay.Office.Core.Units;

namespace XRay.Office.Core.Geometry;

/// <summary>
/// Turns a picture's crop and fill insets into the rectangle to draw the picture into.
/// </summary>
/// <remarks>
/// <para>
/// <b>A crop has no representation in the drawing IR and needs none.</b> Cropping is drawing the
/// picture larger than its frame and clipping it, and every backend already clips — so the whole
/// of the feature, in every format that has it, is the arithmetic here plus a clip the renderer
/// was doing anyway.
/// </para>
/// <para>
/// <b>Why this is in Core.</b> It began in <c>XRay.Office.Presentations</c> because a slide was
/// what wanted it, and it is stated by all three families under three different spellings:
/// DrawingML's <c>a:srcRect</c>/<c>a:fillRect</c>, ODF's <c>fo:clip</c>, and Escher's
/// <c>cropFromTop</c> and its three siblings, which DOC, XLS and PPT all share because they all
/// delegate their drawings to MS-ODRAW. A reader in <c>XRay.Office.WordProcessing</c> cannot reach
/// a sibling of its own library, so leaving the arithmetic in the presentation layer meant
/// porting it once per family. It depends on nothing but <see cref="DocRect"/> and
/// <see cref="Length"/>, so it passes the rule Core is kept by: a thing belongs here when it
/// depends on nothing above Core, whatever it was written for.
/// </para>
/// </remarks>
public static class PictureCrop
{
    /// <summary>
    /// Where the <em>whole</em> picture goes, given where the visible part of it must land.
    /// </summary>
    /// <remarks>
    /// <para>
    /// If the source rectangle throws away a fraction <c>l</c> of the left edge, then the
    /// surviving <c>1 - l - r</c> of the image is what fills the destination, so the whole of it
    /// is that much wider — which is the same arithmetic <c>CropQuotientsFromSrcRect</c> does
    /// from the other end (<c>oox/source/drawingml/fillproperties.cxx:106</c>).
    /// </para>
    /// <para>
    /// <b>The fractions are exact and are not the <c>+ 1</c> in <c>lcl_ApplyCropping</c>.</b>
    /// LibreOffice's Escher reader computes <c>(size + 1) × factor + 0.5</c>
    /// (<c>filter/source/msfilter/msdffimp.cxx:3805-3826</c>), but that runs in the *pixel* space
    /// of a bitmap it is about to crop, not in the shape's placed rectangle; the rounding rule
    /// belongs to the coordinate space it was written in and porting it here would be a
    /// half-pixel error scaled up by the shape. Measured on <c>Thailand17.ppt</c> page 22, plain
    /// fractions reconcile the reference's destination to 0.03 pt in three independent
    /// coordinates.
    /// </para>
    /// </remarks>
    /// <param name="destination">Where the visible part of the picture goes.</param>
    /// <param name="left">Fraction cropped from the source's left edge.</param>
    /// <param name="top">Fraction cropped from its top edge.</param>
    /// <param name="right">Fraction cropped from its right edge.</param>
    /// <param name="bottom">Fraction cropped from its bottom edge.</param>
    /// <returns>
    /// The rectangle to draw the undisturbed picture into, or null when the crop keeps
    /// nothing — which a file can state and which would otherwise divide by zero.
    /// </returns>
    public static DocRect? Uncropped(
        DocRect destination, double left, double top, double right, double bottom)
    {
        double horizontal = 1 - left - right;
        double vertical = 1 - top - bottom;
        if (horizontal <= 0 || vertical <= 0) return null;

        double width = destination.Width.Emu / horizontal;
        double height = destination.Height.Emu / vertical;

        return new DocRect(
            Length.FromEmu(destination.Left.Emu - (long)Math.Round(left * width)),
            Length.FromEmu(destination.Top.Emu - (long)Math.Round(top * height)),
            Length.FromEmu((long)Math.Round(width)),
            Length.FromEmu((long)Math.Round(height)));
    }

    /// <summary>
    /// The rectangle a stretched fill draws its picture into, inset by <c>a:fillRect</c>.
    /// </summary>
    /// <remarks>
    /// The mirror image of <see cref="Uncropped"/> and the reason both exist: a positive
    /// <c>a:srcRect</c> edge throws away part of the picture, a positive <c>a:fillRect</c> edge
    /// leaves part of the <em>shape</em> empty. A negative one on either grows rather than
    /// shrinks, which is legal and is how a file states an overhanging fill.
    /// </remarks>
    /// <param name="area">The area being filled.</param>
    /// <param name="left">Fraction of the area's width to leave at the left.</param>
    /// <param name="top">Fraction of its height to leave at the top.</param>
    /// <param name="right">Fraction of its width to leave at the right.</param>
    /// <param name="bottom">Fraction of its height to leave at the bottom.</param>
    public static DocRect Inset(
        DocRect area, double left, double top, double right, double bottom)
    {
        double width = area.Width.Emu;
        double height = area.Height.Emu;

        return new DocRect(
            Length.FromEmu(area.Left.Emu + (long)Math.Round(left * width)),
            Length.FromEmu(area.Top.Emu + (long)Math.Round(top * height)),
            Length.FromEmu((long)Math.Round(width * (1 - left - right))),
            Length.FromEmu((long)Math.Round(height * (1 - top - bottom))));
    }
}
