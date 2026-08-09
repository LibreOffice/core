using Paperless.MsBinary.Escher;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Maps a shape's <c>msofbtChildAnchor</c> rectangle out of its group's own coordinate space and
/// into twips measured from the anchored group's top-left corner.
/// </summary>
/// <remarks>
/// <para>
/// A group states two rectangles that look alike and are not. <c>msofbtSpgr</c> — <see
/// cref="EscherShape.GroupBounds"/> — is the arbitrary coordinate space its children's rectangles are
/// expressed in; the <c>FSPA</c>, or the group's own child anchor when it is nested, says where the
/// group actually lands on the page. Placing a member by its raw child anchor therefore puts it at a
/// coordinate that has nothing to do with the page, which is why every member of a Word 97 masthead
/// lands off the sheet if the space is not divided out.
/// </para>
/// <para>
/// Nesting composes: an inner group's child anchor is in the outer group's space, so the inner
/// transform is built from the rectangle the outer one already mapped it to. That is the same
/// composition the DOCX reader does for <c>a:chOff</c>/<c>a:chExt</c>
/// (<see cref="Ooxml.DocxFrames"/>), stated in integers here because Word's units already are.
/// </para>
/// <para>
/// A group whose <c>msofbtSpgr</c> is absent or degenerate — zero width or height, which a file may
/// state and which no scale can divide by — is read one-to-one, so a member keeps the coordinates the
/// file gave it rather than being dropped or scaled by an invented factor.
/// </para>
/// </remarks>
/// <param name="OriginX">The child space's left edge.</param>
/// <param name="OriginY">The child space's top edge.</param>
/// <param name="ScaleX">Twips of page per unit of child space, horizontally.</param>
/// <param name="ScaleY">The same vertically.</param>
/// <param name="OffsetX">Where the group's left edge sits, in twips from the outermost group's.</param>
/// <param name="OffsetY">The same vertically.</param>
public readonly record struct Ww8GroupTransform(
    int OriginX,
    int OriginY,
    double ScaleX,
    double ScaleY,
    int OffsetX,
    int OffsetY)
{
    /// <summary>
    /// The transform a group's children are placed by, given the rectangle the group itself occupies.
    /// </summary>
    /// <param name="group">The group shape, whose <c>msofbtSpgr</c> names the child space.</param>
    /// <param name="width">How wide the group is on the page, in twips.</param>
    /// <param name="height">How tall it is, in twips.</param>
    /// <param name="left">Where its left edge sits, in twips from the outermost group's.</param>
    /// <param name="top">Where its top edge sits.</param>
    public static Ww8GroupTransform Of(
        EscherShape group, int width, int height, int left = 0, int top = 0)
    {
        EscherRectangle bounds = group.GroupBounds ?? default;

        double scaleX = bounds.Width != 0 ? (double)width / bounds.Width : 1.0;
        double scaleY = bounds.Height != 0 ? (double)height / bounds.Height : 1.0;

        return new Ww8GroupTransform(bounds.Left, bounds.Top, scaleX, scaleY, left, top);
    }

    /// <summary>Maps one child rectangle into twips from the outermost group's top-left corner.</summary>
    public (int X, int Y, int Width, int Height) Map(EscherRectangle child)
    {
        int x = OffsetX + (int)Math.Round((child.Left - OriginX) * ScaleX);
        int y = OffsetY + (int)Math.Round((child.Top - OriginY) * ScaleY);
        int right = OffsetX + (int)Math.Round((child.Right - OriginX) * ScaleX);
        int bottom = OffsetY + (int)Math.Round((child.Bottom - OriginY) * ScaleY);

        return (x, y, right - x, bottom - y);
    }
}
