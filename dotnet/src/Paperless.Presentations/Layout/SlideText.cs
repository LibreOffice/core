using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;

namespace Paperless.Presentations.Layout;

/// <summary>Where a text body sits inside its shape, vertically.</summary>
/// <remarks>
/// DrawingML's <c>a:bodyPr/@anchor</c> and ODF's <c>draw:textarea-vertical-align</c>, which
/// spell the same three positions differently. Justified anchoring — spreading the paragraphs to
/// fill the shape — is a fourth value both formats have and neither corpus deck uses; it is read
/// as <see cref="Top"/> rather than silently as <see cref="Middle"/>, which is what LibreOffice
/// falls back to for a single paragraph anyway.
/// </remarks>
public enum TextAnchor
{
    /// <summary>The text block starts at the top of the text rectangle.</summary>
    Top = 0,

    /// <summary>It is centred vertically.</summary>
    Middle,

    /// <summary>It ends at the bottom.</summary>
    Bottom,
}

/// <summary>
/// A shape's text body before layout: its paragraphs, its insets, and how it is anchored.
/// </summary>
/// <remarks>
/// The presentation family's equivalent of the word processor's <c>PageParagraph</c> list, and
/// deliberately its own type rather than a reuse: a slide's text is bounded by the shape rather
/// than flowed down a page, so what layout needs to know is the rectangle and the anchor, and
/// none of the pagination properties — widows, keep-with-next, page breaks — mean anything.
/// </remarks>
public sealed record SlideTextBody
{
    /// <summary>The paragraphs, in order.</summary>
    public IReadOnlyList<SlideParagraph> Paragraphs { get; init; } = [];

    /// <summary>
    /// The insets between the shape's text rectangle and the text.
    /// </summary>
    /// <remarks>
    /// Defaulted to DrawingML's own defaults — 0.1 inch left and right, 0.05 inch top and bottom
    /// (<c>a:bodyPr</c>'s <c>lIns</c>, <c>tIns</c>, <c>rIns</c>, <c>bIns</c>) — because a body
    /// that states none gets exactly those, and a reader defaulting them to zero puts every line
    /// of every unstated text box 7.2 pt too far left.
    /// </remarks>
    public Margins Insets { get; init; } = DefaultInsets;

    /// <summary>DrawingML's default text insets: 91440 EMU across, 45720 EMU down.</summary>
    public static Margins DefaultInsets { get; } = new(
        Length.FromEmu(91440), Length.FromEmu(45720),
        Length.FromEmu(91440), Length.FromEmu(45720));

    /// <summary>Where the block sits vertically.</summary>
    public TextAnchor Anchor { get; init; }

    /// <summary>
    /// The multiplier <c>a:normAutofit/@fontScale</c> asks for, or one when it states none.
    /// </summary>
    /// <remarks>
    /// Applied to every run's size rather than recomputed. The value in the file is what the
    /// authoring application arrived at when it last shrank the text to fit, and LibreOffice
    /// honours it as stated (<c>oox/source/drawingml/textbodypropertiescontext.cxx:240</c>)
    /// rather than solving the fit again — so a reader that recomputed it would disagree with the
    /// reference on every autofitted shape.
    /// </remarks>
    public double FontScale { get; init; } = 1.0;

    /// <summary>The fraction <c>a:normAutofit/@lnSpcReduction</c> takes off the line spacing.</summary>
    public double LineSpaceReduction { get; init; }

    /// <summary>
    /// Whether the text wraps at the shape's width.
    /// </summary>
    /// <remarks>
    /// <c>a:bodyPr/@wrap="none"</c> means it does not: the line runs on past the shape and the
    /// shape grows around it. Modelled as an unbounded width rather than as clipping, which is
    /// what makes a `wrap="none"` label come out on one line as its author saw it.
    /// </remarks>
    public bool Wraps { get; init; } = true;
}

/// <summary>One paragraph of a shape's text.</summary>
/// <param name="Text">Its text, without a terminating mark.</param>
/// <param name="Runs">
/// Its runs, partitioning the text. Never empty for non-empty text: a paragraph with no stated
/// formatting still carries one run, so that the size an empty line is as tall as is known.
/// </param>
/// <param name="Alignment">How its lines are placed across the text rectangle.</param>
/// <param name="SpaceBefore">The space above it.</param>
/// <param name="SpaceAfter">The space below it.</param>
/// <param name="LineSpacing">Its line-spacing rule.</param>
/// <param name="StartIndent">Its indent from the start edge.</param>
/// <param name="FirstLineIndent">The extra indent on its first line, negative for a hanging one.</param>
/// <param name="Language">A BCP 47 tag, for the language-specific break rules.</param>
public sealed record SlideParagraph(
    string Text,
    IReadOnlyList<SlideTextRun> Runs,
    TextAlignment Alignment = TextAlignment.Start,
    Length SpaceBefore = default,
    Length SpaceAfter = default,
    LineSpacingRule LineSpacing = default,
    Length StartIndent = default,
    Length FirstLineIndent = default,
    string? Language = null);

/// <summary>One run of a paragraph: a range of its text with its own face, size and colour.</summary>
/// <param name="Start">The run's first character.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Typeface">The family it asks for, or null for the deck's default.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1–1000 scale.</param>
/// <param name="IsItalic">Whether it is italic.</param>
/// <param name="Colour">The colour it is drawn in.</param>
public readonly record struct SlideTextRun(
    int Start,
    int Length,
    string? Typeface,
    Length Size,
    int Weight,
    bool IsItalic,
    Colour Colour)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// Resolves the faces a slide's text needs, once per distinct request.
/// </summary>
/// <remarks>
/// The same shape as the word processor's cache and for the same reason: a deck has a handful of
/// typefaces and hundreds of runs, and resolving one means walking a substitution chain and
/// reading a font file. Its own type rather than a shared one because the two libraries sit at
/// the same layer and neither may depend on the other.
/// </remarks>
public sealed class SlideFonts
{
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string?, int, bool), (OpenTypeFace? Face, FontReference? Reference)>
        _resolved = [];

    /// <summary>Creates a cache over a resolver, or over the installed fonts.</summary>
    /// <param name="fonts">The resolver to use, or null to build one over the installed fonts.</param>
    public SlideFonts(SystemFontResolver? fonts = null)
        => _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());

    /// <summary>The substitutions made so far, which is the first thing a comparison checks.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>The face and reference a request resolves to, both null when nothing could be read.</summary>
    public (OpenTypeFace? Face, FontReference? Reference) Resolve(
        string? family, int weight, bool isItalic)
    {
        (string?, int, bool) key = (family, weight, isItalic);
        if (_resolved.TryGetValue(key, out (OpenTypeFace?, FontReference?) cached)) return cached;

        (OpenTypeFace? Face, FontReference? Reference) resolved = default;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(family ?? string.Empty, weight, isItalic));
            resolved = (_fonts.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A face that cannot be read costs the shape its text, not the deck its layout.
        }

        _resolved[key] = resolved;
        return resolved;
    }
}
