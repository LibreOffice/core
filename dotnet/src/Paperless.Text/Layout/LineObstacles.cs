using Paperless.Core.Units;

namespace Paperless.Text.Layout;

/// <summary>
/// The horizontal stretch a line may use, and how far down the page it had to move to get it.
/// </summary>
/// <remarks>
/// <para>
/// The answer to the only question a line filler has to ask an obstacle: "given that I wanted this
/// stretch at this height, what do I actually get?" All three fields can change — a frame at the start
/// edge pushes <see cref="Left"/> along, one at the end edge shortens <see cref="Width"/>, and one
/// spanning the whole measure leaves no stretch at all, so the line drops below it and
/// <see cref="Descent"/> says by how much.
/// </para>
/// <para>
/// A <see cref="Descent"/> rather than a new top, because the caller is accumulating tops and a delta
/// composes with what it has already added. Writer expresses the same thing as a dummy line of the
/// frame's own height (<c>SwFlyPortion</c> with <c>bFullLine</c>, <c>itrform2.cxx</c>), which is the
/// same arithmetic seen from the other side.
/// </para>
/// </remarks>
/// <param name="Left">Where the line's text may start, measured from the text area's start edge.</param>
/// <param name="Width">How wide it may be.</param>
/// <param name="Descent">How far the line had to move down to find that stretch; usually zero.</param>
public readonly record struct LineSpace(Length Left, Length Width, Length Descent = default)
{
    /// <summary>True when the line found no room at all and could not move down to any.</summary>
    public bool IsBlocked => Width <= Length.Zero;
}

/// <summary>
/// What narrows a paragraph's lines: the floating frames the text has to flow around.
/// </summary>
/// <remarks>
/// <para>
/// The interface between the layout engine's page model and the line filler, and it is deliberately
/// this thin. <see cref="ParagraphLayouter"/> knows how tall a line is and where it sits inside its
/// paragraph; it does not know where the paragraph sits on a page, which frames are anchored there, or
/// what wrap mode each one asks for. Passing a function of "top and height" keeps every one of those out
/// of <c>Paperless.Text</c>, so the same filler serves a header, a table cell and a page body without
/// any of them leaking in.
/// </para>
/// <para>
/// Writer's equivalent is <c>SwTextFly</c> (<c>sw/source/core/text/txtfly.cxx</c>), which the formatter
/// asks the same question of once per line through <c>SwTextFormatter::CalcFlyWidth</c>: it builds a
/// rectangle for the line, calls <c>GetFrame</c> to intersect it against the anchored objects, and
/// shortens the line's width or inserts a fly portion in front of it depending on which side the answer
/// came out. This is that call, with the page-dependent half moved to the caller.
/// </para>
/// </remarks>
public interface ILineObstacles
{
    /// <summary>
    /// The stretch a line may use, given the one it wanted.
    /// </summary>
    /// <param name="top">The top of the line's box, measured from the paragraph's own top.</param>
    /// <param name="height">How tall the box is.</param>
    /// <param name="wanted">The stretch the paragraph's indents and area would have given it.</param>
    LineSpace SpaceFor(Length top, Length height, LineSpace wanted);

    /// <summary>True when nothing here can narrow a line, so the caller can take its fast path.</summary>
    /// <remarks>
    /// Worth asking rather than calling and comparing: a paragraph with no frame near it must lay out
    /// through exactly the code it did before frames existed, or every document in a corpus is at risk
    /// from a rounding difference in a branch that should not have been taken.
    /// </remarks>
    bool IsEmpty { get; }
}
