using Paperless.Core.Units;

namespace Paperless.Text.Layout;

/// <summary>
/// The horizontal room one line has, when something beside the text has taken part of it.
/// </summary>
/// <remarks>
/// Both edges rather than a width, because an obstruction on the <em>left</em> moves a line's start as well
/// as shortening it — which is the ordinary case, since a picture at the left margin is what most documents
/// wrap text around. The two are in the text area's own coordinates; the paragraph's indents apply within
/// them, so an indented paragraph beside a frame is indented from the frame rather than from the margin.
/// </remarks>
/// <param name="Left">How far into the text area the line may start.</param>
/// <param name="Width">How much room it has from there.</param>
public readonly record struct LineSpace(Length Left, Length Width)
{
    /// <summary>The whole of a text area: what every line gets when nothing is in the way.</summary>
    public static LineSpace Of(Length width) => new(Length.Zero, width);
}

/// <summary>
/// How much room a line has, asked once per line as the paragraph is laid out.
/// </summary>
/// <remarks>
/// <para>
/// Takes the line's position rather than its index, because what decides the answer is <em>where the line
/// is</em>: an obstruction occupies a rectangle, and whether a line runs into it depends on the line's top
/// and its height. An index would make the caller reconstruct the vertical arithmetic the layouter has
/// already done.
/// </para>
/// <para>
/// Deliberately not a type that knows what the obstruction is. <c>Paperless.Text</c> lays text out and has
/// no notion of a floating frame, an anchor or a wrap mode; it asks how much room there is and is told.
/// </para>
/// </remarks>
/// <param name="top">Where the line's box starts, measured from the paragraph's top.</param>
/// <param name="height">How tall the box is, since an obstruction the line merely touches still counts.</param>
public delegate LineSpace LineRoom(Length top, Length height);
