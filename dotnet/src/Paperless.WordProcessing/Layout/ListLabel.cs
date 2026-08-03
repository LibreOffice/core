using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// What closes the gap between a list label and the text it labels.
/// </summary>
/// <remarks>
/// ODF says it in <c>text:label-followed-by</c> and OOXML in <c>w:suff</c>; the two agree on all three
/// values, which is why one enumeration serves both. The older ODF spelling — <c>text:min-label-width</c>
/// with no follower at all — is <see cref="Nothing"/> with a minimum gap, since a fixed label width is a
/// tab stop expressed as a distance.
/// </remarks>
public enum LabelFollow
{
    /// <summary>A tab to the level's own stop: <c>listtab</c>, and <c>w:suff</c>'s default.</summary>
    ListTab = 0,

    /// <summary>A single space.</summary>
    Space,

    /// <summary>Nothing at all: the text abuts the label.</summary>
    Nothing,
}

/// <summary>
/// The label a numbered or bulleted paragraph draws in front of its first line.
/// </summary>
/// <remarks>
/// <para>
/// Writer builds one of these as a <em>portion</em> at the head of the first line
/// (<c>SwTextFormatter::NewNumberPortion</c>, <c>sw/source/core/text/txtfld.cxx:506</c>), so the label
/// takes part in the line's measurement and pushes the text along. Modelling it as a portion here would
/// mean splicing characters into the paragraph's text, and every offset the paragraph carries — its
/// notes, its frames, its bookmarks — is counted against that string. So the label sits beside the text
/// instead, and pays for itself by widening the first line's indent: see
/// <see cref="PageParagraph.Format"/>.
/// </para>
/// <para>
/// The label is <em>not</em> in <see cref="PageParagraph.Text"/> for the same reason, which is also why
/// it carries its own face and size. A bullet level names a symbol font that the item's own text does
/// not use, and a numbered level can name a character style; both are properties of the level rather
/// than of the paragraph.
/// </para>
/// </remarks>
public sealed record PageLabel
{
    /// <summary>The label as it is drawn: <c>1.</c>, <c>a)</c>, <c>•</c>.</summary>
    public required string Text { get; init; }

    /// <summary>The face it is set in, which is the level's rather than the paragraph's.</summary>
    public required OpenTypeFace Face { get; init; }

    /// <summary>The em size it is set at.</summary>
    public required Length EmSize { get; init; }

    /// <summary>Its advance width, measured once by <see cref="Measured"/>.</summary>
    /// <remarks>
    /// Stored rather than derived, because <see cref="PageParagraph.Format"/> consults it for every line
    /// of every paragraph the paginator fits and shaping two characters that often is measurable. It is
    /// the one field a caller must not set by hand.
    /// </remarks>
    public Length Width { get; init; }

    /// <summary>The resolved reference, for a backend that has to name the face it draws with.</summary>
    public FontReference? Font { get; init; }

    /// <summary>The colour it is drawn in.</summary>
    public Colour Colour { get; init; } = Colour.Black;

    /// <summary>How it is shaped.</summary>
    public ShapingOptions Shaping { get; init; }

    /// <summary>What closes the gap to the text.</summary>
    public LabelFollow Follow { get; init; }

    /// <summary>
    /// Where the text after a <see cref="LabelFollow.ListTab"/> lands, from the text area's start edge.
    /// </summary>
    /// <remarks>
    /// ODF's <c>text:list-tab-stop-position</c> and OOXML's <c>w:tab w:val="num"</c>. Writer reads it
    /// into the line's own tab information rather than the paragraph's stops
    /// (<c>SwTextNode::GetListTabStopPosition</c>, <c>sw/source/core/txtnode/ndtxt.cxx:4914</c>), which is
    /// what keeps a list tab from also catching the tabs in the item's prose.
    /// </remarks>
    public Length TabStop { get; init; }

    /// <summary>
    /// The least distance between the label's end and the text, whatever the mode.
    /// </summary>
    /// <remarks>
    /// Writer's <c>nMinDist</c>, which is <c>SvxNumberFormat::GetCharTextDistance</c> — ODF's
    /// <c>text:min-label-distance</c> — and is forced to zero in label-alignment mode
    /// (<c>txtfld.cxx:537</c>). Zero for nearly every document.
    /// </remarks>
    public Length MinimumGap { get; init; }

    /// <summary>
    /// Builds a label, measuring its width in the face it is drawn in.
    /// </summary>
    /// <remarks>
    /// The one way to make one, so that <see cref="Width"/> cannot disagree with <see cref="Text"/>. The
    /// readers call it because they are where the level's face and size are resolved; nothing below them
    /// shapes.
    /// </remarks>
    /// <param name="text">The label as it is drawn.</param>
    /// <param name="face">The face to set it in.</param>
    /// <param name="emSize">The size to set it at.</param>
    /// <param name="shaping">How to shape it.</param>
    public static PageLabel Measured(
        string text, OpenTypeFace face, Length emSize, ShapingOptions shaping = default)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(face);

        return new PageLabel
        {
            Text = text,
            Face = face,
            EmSize = emSize,
            Shaping = shaping,
            Width = text.Length == 0
                ? Length.Zero
                : TextShaper.Default.Shape(face, text, shaping).Width(emSize),
        };
    }

    /// <summary>
    /// How far the first line's text starts to the right of the label's own pen.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole of <c>SwNumberPortion::Format</c> (<c>sw/source/core/text/porfld.cxx:607</c>) reduced to
    /// its result: the number portion's width, which is what separates the label from the text. In
    /// label-width-and-position mode the portion stretches to the paragraph's left margin — Writer's
    /// <c>nDiff = rInf.Left() - rInf.First()</c>, which is the negated hanging indent — and in
    /// label-alignment mode it is the label's own width, with the tab that follows carrying the text on
    /// to the level's stop.
    /// </para>
    /// <para>
    /// The floor is <c>m_nFixWidth + m_nMinDist</c> in both modes, which is what stops a label wider than
    /// the space allowed for it from being written over by the text it labels.
    /// </para>
    /// </remarks>
    /// <param name="hangingIndent">
    /// How far the first line hangs back from the paragraph's left margin — the negated
    /// <see cref="ParagraphFormat.FirstLineIndent"/>, and where the label's pen therefore sits relative
    /// to where the text would otherwise start.
    /// </param>
    /// <param name="lineStart">
    /// Where the label's pen sits, from the text area's start edge, so that a
    /// <see cref="LabelFollow.ListTab"/> stop stated in the same frame of reference can be compared
    /// against it.
    /// </param>
    /// <param name="format">
    /// The paragraph's own stops and default interval, for the case where the label overruns both its
    /// level's stop and the hanging indent — see the remarks. Null keeps the label butted against the
    /// text, which is what a caller with no paragraph to consult can honestly say.
    /// </param>
    public Length Advance(Length hangingIndent, Length lineStart, ParagraphFormat? format = null)
    {
        // The label plus whatever the level insists on: the portion is never narrower than this, so a
        // long label pushes the text rather than colliding with it.
        Length floor = Width + MinimumGap;

        Length wanted = Follow switch
        {
            LabelFollow.ListTab => ListTabAdvance(floor, hangingIndent, lineStart, format),

            // No stop to aim at, so the space is the paragraph's own: the hanging indent is the room the
            // document set aside for the label, and Writer fills it.
            LabelFollow.Space or LabelFollow.Nothing => hangingIndent,

            _ => hangingIndent,
        };

        // A space is a real character rather than a distance, so it is added to the floor rather than
        // competing with it — "1. " is wider than "1." even where the level left no room.
        if (Follow == LabelFollow.Space) floor += SpaceWidth();

        return Max(Max(wanted, floor), Length.Zero);
    }

    /// <summary>
    /// Where the tab after a <see cref="LabelFollow.ListTab"/> label carries the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The follower is a real tab: Writer's number portion expands to the number <em>plus</em>
    /// <c>SvxNumberFormat::GetLabelFollowedByAsString</c>'s <c>"\t"</c>
    /// (<c>editeng/source/items/numitem.cxx:504</c>), and the tab that comes out of it goes through
    /// <c>SwTextFormatter::GetTabStop</c> like any other. That matters at exactly one point: when the
    /// label is wider than its level left room for. The level's own stop is then behind the pen and a
    /// stop behind the pen is not "no gap" — the search carries on through the paragraph's own stops
    /// and then along the default interval (<c>sw/source/core/text/txttab.cxx:189</c>). Stopping there
    /// instead is what put <c>1.0Executive Summary</c> on the page.
    /// </para>
    /// <para>
    /// Measured on <c>final-technical-report-template.docx</c> (words/batch-007): a level with
    /// <c>w:ind w:left="360" w:hanging="360"</c> puts its stop 18 pt along, the 18 pt label <c>2.0</c>
    /// is 23.0 pt wide, and LibreOffice starts the heading's text at 36.0 pt — the document's
    /// <c>w:defaultTabStop</c> — where we started it at 23.0.
    /// </para>
    /// <para>
    /// The order is the search's own. The level's stop first, when it is still ahead; then the
    /// paragraph's left margin, which is where Writer sends a tab that is still inside the hanging
    /// indent (<c>bNewTabPortionInsideHangingIndent</c>, <c>txttab.cxx:257</c>); and only then the
    /// ordinary stops.
    /// </para>
    /// </remarks>
    /// <param name="labelEnd">Where the label ends, measured from its own pen.</param>
    /// <param name="hangingIndent">Where the paragraph's left margin sits, from the same pen.</param>
    /// <param name="lineStart">The pen, from the text area's start edge.</param>
    /// <param name="format">The paragraph, or null when the caller has none.</param>
    private Length ListTabAdvance(
        Length labelEnd, Length hangingIndent, Length lineStart, ParagraphFormat? format)
    {
        Length listStop = TabStop - lineStart;

        if (listStop > labelEnd) return Max(listStop, hangingIndent);
        if (hangingIndent > labelEnd || format is null) return hangingIndent;

        // The stops are stated from the paragraph's own tab origin rather than from the text area, so the
        // pen has to be moved into their frame of reference and the answer moved back out of it.
        Length pen = lineStart + labelEnd - format.TabOrigin;
        Length next = format.NextTabStop(pen).Position + format.TabOrigin - lineStart;

        return Max(next, hangingIndent);
    }

    /// <summary>
    /// The line box the label alone would need: how tall, and how much of it is above the baseline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A label can be bigger than the paragraph it labels, and then it raises the line.</strong>
    /// Writer's label is an ordinary portion — <c>SwNumberPortion</c>, built by
    /// <c>SwTextFormatter::NewNumberPortion</c> (<c>sw/source/core/text/txtfld.cxx</c>:506) — so
    /// <c>SwLineLayout::CalcLine</c> (<c>sw/source/core/text/porlay.cxx</c>:340) folds its ascent and
    /// height into the line's maxima exactly as it does for a run of text. A level stating a size the
    /// item's own text does not use is ordinary rather than exotic: Word writes the level's character
    /// formatting into <c>w:lvl/w:rPr</c> and into the WW8 level's <c>grpprlChpx</c>, and both frequently
    /// name a size of their own.
    /// </para>
    /// <para>
    /// Measured through the same <see cref="LineSpacing"/> the runs use, and on the same device grid, so
    /// that a label and a run of the same face and size give the same box — otherwise a label equal to its
    /// text would still move the line.
    /// </para>
    /// </remarks>
    /// <param name="grid">The device grid the paragraph's metrics are rounded through, or null.</param>
    public (Length Height, Length Ascent) LineExtent(MetricGrid? grid = null)
    {
        LineMetrics metrics = LineSpacing.Resolve(Face, grid);

        // Whole twips, as MeasuredParagraph.Accumulate takes them: a fraction kept here would eventually
        // put a line on a different page from the one the runs beside it were measured onto.
        return (
            Length.FromTwips(metrics.ScaledLineHeight(EmSize).Twips),
            Length.FromTwips(metrics.ScaledAscent(EmSize).Twips));
    }

    /// <summary>The width of one space in the label's own face, for <see cref="LabelFollow.Space"/>.</summary>
    private Length SpaceWidth() => TextShaper.Default.Shape(Face, " ", Shaping).Width(EmSize);

    private static Length Max(Length left, Length right) => left > right ? left : right;
}
