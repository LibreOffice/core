using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Shaping;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Turns laid-out pages into drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// One line becomes one glyph run, positioned at its baseline. Positioned rather than measured again:
/// layout already committed to these advances when it decided where the lines broke, so re-deriving the
/// positions here would risk output that disagrees with the breaks around it — which is exactly what
/// <see cref="GlyphRun"/>'s own contract says a backend must not do.
/// </para>
/// <para>
/// The glyphs come from shaping the line's own characters, and shaping the line rather than the paragraph
/// is a deliberate small inaccuracy: a kern pair straddling a line break does not apply, which is right,
/// but a contextual form that depended on a following character now sees the line's end instead. For the
/// Latin text this can currently measure, the two are the same.
/// </para>
/// </remarks>
public static class PageDrawing
{
    /// <summary>
    /// Draws a page's body text into a sink.
    /// </summary>
    /// <remarks>
    /// The body only. Headers, footers and floating frames are furniture that pagination does not place
    /// yet, so a page drawn here is a page of body text — which is what it holds.
    /// </remarks>
    /// <param name="page">The page to draw.</param>
    /// <param name="paragraphs">The paragraphs the page's lines index into.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public static void Draw(
        LaidOutPage page, IReadOnlyList<PageParagraph> paragraphs, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(page);
        ArgumentNullException.ThrowIfNull(paragraphs);
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(page.Size);
        try
        {
            foreach (PlacedLine line in page.Lines)
            {
                if (line.ParagraphIndex < 0 || line.ParagraphIndex >= paragraphs.Count) continue;

                PageParagraph paragraph = paragraphs[line.ParagraphIndex];
                if (RunFor(page, line, paragraph) is not { } run) continue;

                sink.DrawGlyphRun(run, Paint.Solid(paragraph.Colour));
            }
        }
        finally
        {
            // Always closed, even if a sink throws part way through: a page left open would make the
            // next one nest inside it, turning one bad page into a broken document.
            sink.EndPage();
        }
    }

    /// <summary>
    /// The glyph run for one line, or null when the line has no visible text.
    /// </summary>
    /// <remarks>
    /// The origin is the start of the baseline, not the top-left of a box — which is what
    /// <see cref="GlyphRun.Origin"/> means and what every text API expects. Getting it from the line's
    /// box needs both of the page's own offsets and the line's baseline within its box, and the baseline
    /// is the part that is not derivable from the height: line spacing puts its extra space above the
    /// text, so a line's baseline is not a fixed fraction of its box.
    /// </remarks>
    public static GlyphRun? RunFor(LaidOutPage page, PlacedLine line, PageParagraph paragraph)
    {
        ArgumentNullException.ThrowIfNull(page);
        ArgumentNullException.ThrowIfNull(paragraph);

        string text = line.Box.Line.VisibleTextIn(paragraph.Text).ToString();
        if (text.Length == 0) return null;

        ShapedText shaped = TextShaper.Default.Shape(paragraph.Face, text, paragraph.Shaping);
        if (shaped.Glyphs.Count == 0) return null;

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length pen = Length.Zero;
        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, paragraph.EmSize);

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + shaped.Scale(glyph.OffsetX, paragraph.EmSize),

                    // Negated: a shaper's y offset is up-positive, and document space is down-positive.
                    // Getting this backwards puts every accent below the letter it belongs to.
                    -shaped.Scale(glyph.OffsetY, paragraph.EmSize)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = paragraph.Font ?? Reference(paragraph),
            FontSize = paragraph.EmSize,
            Origin = new DocPoint(
                page.BodyArea.X + line.Box.Left,
                page.BodyArea.Y + line.Baseline),
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    /// <summary>
    /// A reference for a paragraph whose font was not resolved through a resolver.
    /// </summary>
    /// <remarks>
    /// Hand-built input — a test, or a caller driving the paginator directly — has a face but no
    /// reference. Naming the face's own family is enough for a backend to group runs by font, and it
    /// records no substitution because none was made.
    /// </remarks>
    private static FontReference Reference(PageParagraph paragraph) => new()
    {
        FamilyName = paragraph.Face.FamilyName ?? string.Empty,
        Weight = paragraph.Face.Weight,
        IsItalic = paragraph.Face.IsItalic,
        FaceKey = paragraph.Face.FamilyName ?? string.Empty,
    };
}
