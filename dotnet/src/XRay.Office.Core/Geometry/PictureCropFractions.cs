namespace XRay.Office.Core.Geometry;

/// <summary>
/// How much of a picture each of its four edges throws away, as fractions of the picture.
/// </summary>
/// <remarks>
/// <para>
/// <b>The crop has to travel, because the rectangle does not exist yet.</b>
/// <see cref="PictureCrop.Uncropped"/> needs the rectangle the visible part of the picture
/// lands in, and on two of the three tracks that rectangle is not known when the picture is
/// read: a sheet's drawing is anchored to <em>cells</em> and has no size until the page's
/// column widths are resolved, and a word processor's floating frame is placed by the layout
/// engine. So the four fractions are carried from the reader to the painter and the
/// arithmetic is done where the rectangle is. Only the slide path can do both at once,
/// because a slide shape's rectangle is stated in the file.
/// </para>
/// <para>
/// Stated once rather than as four <c>double</c> properties on each model that carries it:
/// they are meaningless apart, and a model that let one layer set three of them would lose
/// the fourth silently.
/// </para>
/// </remarks>
/// <param name="Left">Fraction of the picture thrown away at its left edge.</param>
/// <param name="Top">Fraction thrown away at its top edge.</param>
/// <param name="Right">Fraction thrown away at its right edge.</param>
/// <param name="Bottom">Fraction thrown away at its bottom edge.</param>
public readonly record struct PictureCropFractions(
    double Left, double Top, double Right, double Bottom)
{
    /// <summary>The whole picture, which is what almost every picture in a corpus states.</summary>
    public static PictureCropFractions None => default;

    /// <summary>True when no edge is cropped, so the picture fills its rectangle.</summary>
    public bool IsNone => Left == 0 && Top == 0 && Right == 0 && Bottom == 0;

    /// <summary>
    /// Where the <em>whole</em> picture goes, given where its visible part must land.
    /// </summary>
    /// <remarks>
    /// Returns <paramref name="destination"/> unchanged both when there is no crop and when the
    /// crop keeps nothing — a file can state the second, and drawing the picture uncropped in
    /// the right place beats leaving a hole. A caller wanting to know whether a clip is needed
    /// can compare the result against what it passed in.
    /// </remarks>
    /// <param name="destination">Where the visible part of the picture goes.</param>
    public DocRect Apply(DocRect destination)
        => IsNone
            ? destination
            : PictureCrop.Uncropped(destination, Left, Top, Right, Bottom) ?? destination;
}
