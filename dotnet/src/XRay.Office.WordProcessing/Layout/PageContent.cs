using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// One piece of body-level content waiting to be paginated: a paragraph or a table.
/// </summary>
/// <remarks>
/// <para>
/// The distinction Writer's own layout draws, and for the same reason: a body frame holds text frames and
/// table frames side by side, and the two flow differently. A paragraph is a run of lines that can be cut
/// anywhere a line ends; a table is a grid whose rows are sized by their tallest cell, and whose cells are
/// each a flow of their own. Neither reduces to the other — flattening a table into its cells' paragraphs
/// would give the page a height no table has.
/// </para>
/// <para>
/// A closed hierarchy of exactly two cases: sections and floating frames will be pages' business rather
/// than blocks', because a section changes the page and a floating frame is anchored rather than flowed.
/// </para>
/// </remarks>
public abstract record PageBlock
{
    /// <summary>The caller's own reference to whatever this came from.</summary>
    /// <remarks>
    /// Pagination reorders nothing but it does split and drop things, so a caller needs to get back from a
    /// laid-out line to the node it belongs to; carrying an opaque reference is cheaper than making the
    /// engine know about the document model.
    /// </remarks>
    public object? Source { get; init; }

    /// <summary>
    /// Which of the document's sections this block belongs to.
    /// </summary>
    /// <remarks>
    /// On the block rather than worked out by the paginator, because only the reader can know it: three of
    /// the four formats delimit sections by position in a stream the layout engine never sees, and ODF does
    /// not delimit them at all — a paragraph reaches its page description through its style's master page.
    /// Zero for a document with one section, which is most of them.
    /// </remarks>
    public int SectionIndex { get; init; }
}

/// <summary>
/// A paragraph waiting to be paginated: its text, its resolved formatting, and the face it is set in.
/// </summary>
/// <remarks>
/// <para>
/// The paginator's input, deliberately not the document model. Pagination needs a flat sequence of
/// things with heights, and a paragraph's height depends only on its text, its format, its face and the
/// width it is given — so taking exactly that keeps the engine testable against hand-built input rather
/// than only against a whole document, and keeps it usable by whichever pass eventually builds it.
/// </para>
/// </remarks>
public sealed record PageParagraph : PageBlock
{
    /// <summary>The paragraph's text, without its terminating mark.</summary>
    public required string Text { get; init; }

    /// <summary>The face the text is set in.</summary>
    public required OpenTypeFace Face { get; init; }

    /// <summary>
    /// The resolved font reference, for a renderer that has to name the face it is drawing with.
    /// </summary>
    /// <remarks>
    /// Kept beside the face rather than derived from it, because the two answer different questions: the
    /// face has the metrics that decided the layout, and the reference records <em>which</em> face that
    /// was and what was asked for before substitution. A PDF backend deduplicates embedded fonts on the
    /// reference's key, and a comparison against a reference renderer needs the requested family to
    /// explain a difference.
    /// </remarks>
    public FontReference? Font { get; init; }

    /// <summary>The colour the text is drawn in.</summary>
    /// <remarks>
    /// Black by default rather than nothing, since a run with no colour is drawn in the document's text
    /// colour and every format's default for that is black.
    /// </remarks>
    public Colour Colour { get; init; } = Colour.Black;

    /// <summary>
    /// Its resolved layout properties, with room made on the first line for a list label.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The only formatting anything downstream should measure against.</strong> A list label is
    /// drawn beside the paragraph's text rather than spliced into it — see <see cref="PageLabel"/> — so
    /// something has to hold the first line's text back far enough to leave room, and it is this: the
    /// declared first-line indent, which for a list is negative, widened by the label's own advance.
    /// Writer arrives at the same first line by making the label a portion within it
    /// (<c>SwNumberPortion::Format</c>, <c>sw/source/core/text/porfld.cxx:607</c>).
    /// </para>
    /// <para>
    /// Adjusted here rather than at each of the five places that lay a paragraph out, because a paragraph
    /// measured against one first-line indent and drawn against another puts its own words in two
    /// different places. <see cref="DeclaredFormat"/> is what the reader actually said, and the label
    /// hangs at <em>its</em> <see cref="ParagraphFormat.LineStart"/>.
    /// </para>
    /// </remarks>
    public ParagraphFormat Format
    {
        get => Label is null
            ? _format
            : _format with { FirstLineIndent = _format.FirstLineIndent + LabelAdvance };
        init => _format = value;
    }

    private readonly ParagraphFormat _format = ParagraphFormat.Default;

    /// <summary>The formatting as the reader stated it, before the label was allowed for.</summary>
    /// <remarks>
    /// Where the label's own pen sits, and what a test asserting a reader's work should compare against
    /// rather than the widened <see cref="Format"/>.
    /// </remarks>
    public ParagraphFormat DeclaredFormat => _format;

    /// <summary>
    /// The label this paragraph draws in front of its first line, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// Null for the overwhelming majority of paragraphs, and for the continuation paragraphs of a
    /// multi-paragraph list item as well: ODF gives the label to the first <c>text:p</c> of a
    /// <c>text:list-item</c> only, and the other three formats say the same thing by putting no list
    /// instance on the paragraph. Such a paragraph keeps the level's indents and draws nothing.
    /// </remarks>
    public PageLabel? Label { get; init; }

    /// <summary>How far the label pushes the first line's text along, or zero when there is none.</summary>
    internal Length LabelAdvance
        => Label?.Advance(
               -_format.FirstLineIndent, _format.StartIndent + _format.FirstLineIndent, _format)
           ?? Length.Zero;

    /// <summary>
    /// The colour filled behind the whole paragraph, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A paragraph's own background — <c>w:pPr/w:shd</c>, ODF's <c>fo:background-color</c>, RTF's
    /// <c>\cbpat</c> — which is a different thing from a run's highlight and from a table cell's shade: it
    /// covers the paragraph's <em>whole</em> text area rather than the width of its words, which is what
    /// makes a shaded heading read as a bar across the page.
    /// </para>
    /// <para>
    /// Kept beside <see cref="Colour"/> rather than inside <see cref="ParagraphFormat"/> because it is a
    /// painting attribute and nothing about it changes a measurement: a shaded paragraph breaks its lines
    /// exactly where an unshaded one would. See <see cref="PageDrawing"/> for the rectangle it fills, which
    /// is the paragraph's print area and not its frame.
    /// </para>
    /// </remarks>
    public Colour? Shading { get; init; }

    /// <summary>
    /// The rules drawn round the paragraph, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Unlike <see cref="Shading"/> this is not only a painting attribute: a top or bottom border keeps a
    /// measured distance from the text, so it lengthens the paragraph and can decide a page break. The
    /// left and right rules draw without measuring — LibreOffice grows the box outward past the page
    /// margin rather than narrowing the text — so only <see cref="BorderAbove"/> and
    /// <see cref="BorderBelow"/> reach the paginator.
    /// </para>
    /// <para>
    /// Already joined by the reader where two consecutive paragraphs are bordered alike, because the join
    /// changes both the picture and the height and the two must agree: see
    /// <see cref="ParagraphBorderSet.Join"/>.
    /// </para>
    /// </remarks>
    public ParagraphBorderSet? Borders { get; init; }

    /// <summary>The room the paragraph's top border takes above its first line.</summary>
    public Length BorderAbove => Borders?.Above ?? Length.Zero;

    /// <summary>The room the paragraph's bottom border takes below its last line.</summary>
    public Length BorderBelow => Borders?.Below ?? Length.Zero;

    /// <summary>The em size the text is set at.</summary>
    public Length EmSize { get; init; } = Length.FromPoints(12);

    /// <summary>A BCP 47 tag, for the language-specific break rules.</summary>
    public string? Language { get; init; }

    /// <summary>How the text is shaped; the default is what Writer does.</summary>
    public ShapingOptions Shaping { get; init; }

    /// <summary>
    /// The distance put between the paragraph's characters where its runs say nothing else.
    /// </summary>
    /// <remarks>
    /// The paragraph mark's own tracking, which is what a paragraph set end to end in one tracked style
    /// carries — and which nothing else would supply, because such a paragraph is uniform by every test
    /// <see cref="Runs"/> makes and reaches <see cref="Measure"/> with no runs at all.
    /// </remarks>
    public Length Tracking { get; init; }

    /// <summary>
    /// The character width scaling applied where the paragraph's runs say nothing else, as a
    /// percentage.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="Tracking"/>'s twin, and it was missing for exactly the reason the remark above
    /// gives for tracking: a paragraph set end to end in one scaled style is <strong>uniform by
    /// every test <see cref="Runs"/> makes</strong>, so it arrives at <see cref="Measure"/> with
    /// no runs at all and was measured — and drawn — at 100 per cent. The property was resolved
    /// correctly through the style chain the whole time; it had nowhere to go.
    /// </para>
    /// <para>
    /// That is also why a run stating the <em>same</em> percentage as its style came out
    /// unscaled, which is a trap for anyone probing this: setting the run to its style's own
    /// value makes the paragraph uniform and so reproduces the defect rather than controlling for
    /// it. A seat was nearly filed as "<c>w:w</c> is not applied at all" on that reading.
    /// </para>
    /// <para>
    /// Measured on <c>Regulations Governing the Status…docx</c>, whose title style <c>SL</c>
    /// states <c>&lt;w:w w:val="96"/&gt;</c> and whose title runs state no <c>w:rPr</c> at all:
    /// its <c>Experts on Mission</c> was drawn <b>227.56 pt</b> against 26.2.4.2's <b>218.06</b>,
    /// same face and same start, and the title then wrapped a word later on every line.
    /// </para>
    /// </remarks>
    public int WidthPerCent { get; init; } = 100;

    /// <summary>
    /// Whether the paragraph's own character width is anything but natural, which is what puts it
    /// on the measured path rather than the shortcut.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both layouters take a shortcut for a paragraph with no runs: they hand the line breaker the
    /// <em>text</em>, a face, a size and <see cref="EffectiveShaping"/>, and that overload measures
    /// from those alone. Tracking survives it because <see cref="EffectiveShaping"/> carries it;
    /// a character width has nowhere to ride, so the lines were broken at the <b>unscaled</b>
    /// widths and then drawn at the scaled ones.
    /// </para>
    /// <para>
    /// That is why the defect was invisible to a one-line probe and to a width measurement: the
    /// drawn line is exactly <c>scale ×</c> the unscaled one, so it looks right until the text is
    /// long enough to wrap and then it breaks in the unscaled places. Measured on
    /// <c>features/words-style-char-scale.docx</c>, whose fourth paragraph is a scaled style over
    /// eighteen words: 26.2.4.2 breaks it into <b>two</b> lines and this tree broke it into
    /// <b>three</b>, each exactly 0.6 of the unscaled control's — the same three the control
    /// breaks into.
    /// </para>
    /// </remarks>
    public bool IsHorizontallyScaled => WidthPerCent != TextWidthScale.Natural;

    /// <summary>
    /// How the paragraph's text is shaped where its runs say nothing else, once
    /// <see cref="Tracking"/> has had its say.
    /// </summary>
    /// <remarks>
    /// The uniform paragraph's own <see cref="PageRun.EffectiveShaping"/>. A paragraph set end to
    /// end in one tracked style carries no runs at all — it is uniform by every test
    /// <see cref="Runs"/> makes — so the rule has to be stated here as well or such a paragraph is
    /// the one kind that escapes it.
    /// </remarks>
    public ShapingOptions EffectiveShaping => Shaping.WithTracking(Tracking);

    /// <summary>
    /// The paragraph's runs, when its formatting is not uniform.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Empty means uniform: the whole paragraph is measured and drawn in <see cref="Face"/> at
    /// <see cref="EmSize"/>, which is what a paragraph of plain text is and by far the common case. When
    /// runs are present they partition the text and each carries its own face, size and colour, and the
    /// line height becomes the tallest run's on that line rather than the paragraph's.
    /// </para>
    /// <para>
    /// <see cref="Face"/> and <see cref="EmSize"/> stay required even so, because they are the
    /// paragraph's own — what its mark carries, and what an empty paragraph is as tall as.
    /// </para>
    /// </remarks>
    public IReadOnlyList<PageRun> Runs { get; init; } = [];

    /// <summary>
    /// Named places in the paragraph — bookmarks — and the character offset each sits at.
    /// </summary>
    /// <remarks>
    /// Declared to a PDF as named destinations, so that an internal link (<see cref="PageRun.Link"/>, a
    /// cross-reference, a table-of-contents entry) has somewhere to go. Layout does not use them.
    /// </remarks>
    public IReadOnlyList<PageAnchor> Anchors { get; init; } = [];

    /// <summary>
    /// The paragraph's outline level, 1 to 9, or 0 when it is body text.
    /// </summary>
    /// <remarks>
    /// Resolved through the style chain, so a heading style's level reaches every paragraph set in it.
    /// A PDF's bookmarks panel is built from the paragraphs that have one. Layout does not use it.
    /// </remarks>
    public int OutlineLevel { get; init; }

    /// <summary>True when some run of the paragraph is a link.</summary>
    internal bool HasLinks => Runs.Any(run => run.Link is not null);

    /// <summary>True when the paragraph's formatting varies across its text.</summary>
    public bool HasRuns => Runs.Count > 0;

    private bool? _needsGlyphFallback;

    /// <summary>
    /// True when the paragraph's own face has no glyph for something in its own text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The uniform-paragraph shortcut — no runs, so measure the whole text in one face — is only
    /// equivalent while that face can draw the text. The drawing pass cuts every paragraph by face
    /// through <see cref="FontItemiser.Split"/> whether it has runs or not, so a uniform paragraph
    /// holding a script its face lacks was <em>drawn</em> from a fallback face and <em>measured</em>
    /// from the missing-glyph box of the face it asked for. The two disagree by whatever the two
    /// faces' advances differ by, and the line breaks on the measurement.
    /// </para>
    /// <para>
    /// Answering true sends the paragraph down the per-run path, which is the one that itemises by
    /// face, so both sides make the same cut. Cached because pagination measures a paragraph once per
    /// attempt at placing it and this walks the text.
    /// </para>
    /// </remarks>
    public bool NeedsGlyphFallback
        => _needsGlyphFallback ??=
            Fallback is not null && FontItemiser.NeedsFallback(Text, Face);

    private bool? _hasScriptSpace;

    /// <summary>True when the paragraph holds a script change that opens a gap.</summary>
    /// <remarks>
    /// The second reason a uniform paragraph cannot take the single-face shortcut: that path measures
    /// the text straight off one shaped run and has no prefix table to add a gap to, so a paragraph
    /// mixing scripts has to be measured per run whether its formatting varies or not.
    /// </remarks>
    public bool HasScriptSpace
        => _hasScriptSpace ??= AddsScriptSpace && ScriptSpacing.Boundaries(Text).Count > 0;

    /// <summary>
    /// The device grid the paragraph's fonts are measured through.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Defaults to <see cref="MetricGrid.Reference"/> rather than to nothing</b>, because Writer has no
    /// unquantised path: it formats against a virtual reference device at 8640 dpi in twips, and every
    /// vertical metric is rounded onto that grid and back. The few documents that ask to be laid out
    /// against a printer instead get <see cref="MetricGrid.Printer"/>. Null is reserved for a caller that
    /// genuinely wants exact scaling, which no Writer reader is.
    /// </para>
    /// <para>
    /// Carried on the paragraph rather than passed down the layout call chain because a header, a table
    /// cell and a text box all need the same answer and all reach the layouter by different routes; the
    /// reader that knows the document's answer sets it once.
    /// </para>
    /// </remarks>
    public MetricGrid? Metrics { get; init; } = MetricGrid.Reference;

    /// <summary>
    /// Where to look for a face when the paragraph's own has no glyph for a character, or null to not look.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Beside <see cref="Metrics"/> and set by the same readers for the same reason: a header, a table
    /// cell and a text box all need the same answer and all reach the layouter by different routes.
    /// </para>
    /// <para>
    /// Without it a character the run's face cannot draw is shaped to <c>.notdef</c> and drawn as that
    /// face's missing-glyph box, at that face's <c>.notdef</c> width — so the text is invisible
    /// <em>and</em> the line breaks in the wrong place. Measured on <c>手机免提系统TSB.doc</c>, whose
    /// every Chinese character came out a box while LibreOffice drew all of them from WenQuanYi Zen
    /// Hei. The mechanism was complete on both sides of this property —
    /// <see cref="FontItemiser"/> splits the run and <see cref="SystemFontResolver"/> answers the
    /// query — and nothing in the tree ever connected them.
    /// </para>
    /// </remarks>
    public IGlyphFallbackResolver? Fallback { get; init; }

    /// <summary>
    /// The font item the paragraph's own face was resolved from, for a paragraph with no runs.
    /// </summary>
    /// <remarks>
    /// Beside <see cref="Fallback"/> and set by the same readers, because it is half of the same
    /// question: the resolver answers <em>which face draws this character</em> and the item is what
    /// the pattern it asks with is built from. A paragraph that has runs takes each run's own; see
    /// <see cref="FontItem"/> for why it cannot be recovered from the face.
    /// </remarks>
    public FontItem Item { get; init; }

    /// <summary>
    /// Whether a script change with East Asian text on one side opens Writer's extra gap.
    /// </summary>
    /// <remarks>
    /// Writer's <c>SvxScriptSpaceItem</c> — "add space between Asian and Western text" — which the
    /// Word filters turn on and ODF carries its own value for. Beside <see cref="Metrics"/> and
    /// <see cref="Fallback"/>, set by the same readers for the same reason. See
    /// <see cref="ScriptSpacing"/> for the rule and its two exclusions.
    /// </remarks>
    public bool AddsScriptSpace { get; init; }

    /// <summary>
    /// True when a tab or a run of spaces must not make a line taller, which is what Word does.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>IgnoreTabsAndBlanksForLineCalculation</c> (#i3952). Its DOC importer sets it outright
    /// (<c>sw/source/filter/ww8/ww8par.cxx</c>:2041) and its DOCX import ends up with it too, while RTF and
    /// ODF leave it off unless the file says otherwise — measured by exporting the same prose from each
    /// format to flat ODF and reading the setting back: <c>true</c> from <c>.doc</c> and <c>.docx</c>,
    /// <c>false</c> from <c>.rtf</c>, <c>.odt</c> and <c>.fodt</c>.
    /// </para>
    /// <para>
    /// It only ever matters on a paragraph whose formatting varies, since a tab takes the size of whatever
    /// character formatting covers it and that is frequently the document's default rather than the size of
    /// the text around it. Beside <see cref="Metrics"/> and for the same reason: the reader knows the
    /// answer, and four different routes into the layouter need it.
    /// </para>
    /// </remarks>
    public bool BlanksAreTransparentToHeight { get; init; }

    /// <summary>
    /// True when this paragraph's lines carry no margin line number and do not advance the count.
    /// </summary>
    /// <remarks>
    /// <c>w:pPr/w:suppressLineNumbers</c>, and Writer's <c>SwFormatLineNumber::IsCount</c>. Both halves
    /// matter and only one of them is obvious: a suppressed paragraph is skipped by the counter as well as
    /// by the pen, so the line after it takes the number the line before it would have led to. On the
    /// paragraph rather than in <see cref="LineNumbering"/> because it is stated per paragraph, and false
    /// on all but a handful — see <see cref="LineNumbering"/> for the rest of the rule.
    /// </remarks>
    public bool SuppressesLineNumbers { get; init; }

    /// <summary>
    /// The direction its bidi resolution takes as its base.
    /// </summary>
    /// <remarks>
    /// The declared writing mode first and the runs' shaping options after it, which is the rule
    /// <see cref="MeasuredParagraph"/> applies when it is handed no itemisation of its own. One
    /// rule rather than two, because measuring a paragraph at one base level and drawing it at
    /// another puts its sub-runs in an order its own widths do not describe.
    /// </remarks>
    public BidiDirection BaseDirection
        => Format.IsRightToLeft || (HasRuns ? Runs[0].Shaping : Shaping).RightToLeft
            ? BidiDirection.RightToLeft
            : BidiDirection.LeftToRight;

    /// <summary>
    /// How to cut it into sub-runs, or null for the neutral settings.
    /// </summary>
    /// <remarks>
    /// Null rather than a left-to-right instance for the paragraph that needs nothing, so a
    /// document that says nothing about direction is measured through exactly the path it took
    /// before writing modes existed — including a caller that says right-to-left on its runs and
    /// nothing on the paragraph, which is how it had to be said before.
    /// </remarks>
    internal ItemisationOptions? Itemisation
        => Fallback is null && !Format.IsRightToLeft
            ? null
            : new ItemisationOptions
            {
                // BaseDirection rather than Format.IsRightToLeft, because a paragraph that says
                // right-to-left only on its runs used to reach the same answer through the null
                // branch's default and has to keep reaching it now the fallback opens this one.
                BaseDirection = BaseDirection,
                GlyphFallback = Fallback,
            };

    /// <summary>
    /// The notes anchored in the paragraph's text, in order.
    /// </summary>
    /// <remarks>
    /// Carried on the paragraph because that is where the anchor is: a footnote occupies a character
    /// position in the sentence that cites it, and its body lives at the foot of whichever page that
    /// position lands on. Which page that is cannot be known until the paragraph is placed, which is what
    /// makes notes a pagination matter rather than a reading one.
    /// </remarks>
    public IReadOnlyList<PageNote> Notes { get; init; } = [];

    /// <summary>
    /// The floating frames anchored in this paragraph, in document order.
    /// </summary>
    /// <remarks>
    /// On the paragraph because that is where every format puts the anchor, a page-anchored frame
    /// included: even <c>text:anchor-type="page"</c> is written at a position in the text, and Word has
    /// no page anchor at all — its page-relative positions are still anchored to a paragraph. So which
    /// page a frame lands on is a pagination result rather than a property, which is what makes frames a
    /// two-pass affair; see <see cref="Paginator"/>.
    /// </remarks>
    public IReadOnlyList<PageFrame> Frames { get; init; } = [];

    /// <summary>
    /// The spans of this paragraph's text that a field computed, for the two fields pagination decides.
    /// </summary>
    /// <remarks>
    /// Empty for almost every paragraph, and for every paragraph of a document with no page-number field
    /// anywhere. What the reader put here is the producer's cached result and where it sits; see
    /// <see cref="PageFields"/> for why that is not what gets drawn.
    /// </remarks>
    public IReadOnlyList<PageFieldSpan> Fields { get; init; } = [];

    /// <summary>
    /// The as-character frames among <see cref="Frames"/>: room <em>on</em> a line rather than beside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Derived rather than stored, because a frame states one thing and layout needs it twice — an
    /// as-character frame is placed by hanging it on its line, and the <em>same</em> frame is what makes
    /// that line wider and taller. Deriving keeps the two from disagreeing, which they would the first
    /// time a reader set one and forgot the other.
    /// </para>
    /// <para>
    /// Empty for a paragraph with no inline frame, which is nearly all of them, and that is what lets the
    /// paginator keep taking the cheaper single-face measurement for those.
    /// </para>
    /// </remarks>
    public IReadOnlyList<InlineObject> InlineObjects =>
        HasInlineObjects
            ? [.. Frames
                .Where(frame => frame.Anchor == FrameAnchor.AsCharacter)
                .Select(frame => new InlineObject(
                    frame.AnchorOffset,
                    frame.InlineExtent.Width,
                    frame.InlineExtent.Height + ShapeLineTwip(frame),
                    frame.InlineAscent ?? frame.InlineExtent.Height))]
            : [];

    /// <summary>
    /// The extra twip a <em>drawn shape</em> adds to the line it hangs on, below the baseline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// An as-character picture and an as-character shape do not reach Writer's line the same way. A
    /// picture is a fly holding a <c>SwNoTextFrame</c> and its size is its size; a shape is a draw
    /// object, and a draw object's rectangle is inclusive — <c>SwRect::Bottom() == Top() + Height() - 1</c>
    /// — so the line it hangs on comes out one twip taller than the shape.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2 on two fixtures differing only in what their <c>a:graphicData</c> holds,
    /// ten identical paragraphs of one inline object each, the pitch between the objects being the line
    /// height (<c>probes/words-catalogue-bias/</c>):
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// a <c>wps:wsp</c> shape of stated height <c>cy</c>: the reference's line is <c>cy + 1</c> twips and
    /// ours was <c>cy</c>, at every one of twelve heights from 560 to 571 twips.
    /// </description></item>
    /// <item><description>
    /// a <c>pic:pic</c> picture, the same sweep: <b>no difference at all</b>, six of six. So the twip is
    /// the shape's, not every inline object's, and the picture is the control that says so.
    /// </description></item>
    /// </list>
    /// <para>
    /// Below the baseline rather than above it, which the same fixture settles: the <em>first</em> shape
    /// on the page is drawn at the same place in both engines, 785.19 pt against 785.20, so the extra
    /// twip is behind the object and not in front of it. That is why the ascent is resolved here to the
    /// unpadded height rather than left to <c>InlineObject</c>'s default, which would otherwise take the
    /// padded one and move the drawing down with it.
    /// </para>
    /// <para>
    /// Its reach on the corpus is the catalogue this was found on,
    /// <c>WordArt_Shapes_Arrows_Catalog1.docx</c>, whose rows are paragraphs of inline shapes: the twip
    /// accumulates down each page, to 14.6 twips by the fifteenth row of page 48. That residual was
    /// recorded by an earlier round as sub-point placement noise on the twip grid, and it is not noise:
    /// over 466 paired drawing records the vertical offset runs <b>+418 / −11 / 0 37</b> with a mean of
    /// +0.2059 pt, twenty-eight standard errors from zero, while the horizontal offset's mean is
    /// +0.0009 pt.
    /// </para>
    /// </remarks>
    private static Length ShapeLineTwip(PageFrame frame)
        => frame.Image is not null || frame.Vector is not null || frame.Chart is not null
            || frame.IsTextPortion
            ? Length.Zero
            : Length.FromTwips(1);

    /// <summary>True when an as-character frame is set in the paragraph's text.</summary>
    public bool HasInlineObjects
        => Frames.Count > 0 && Frames.Any(frame => frame.Anchor == FrameAnchor.AsCharacter);

    /// <summary>
    /// True when the label is taller than the paragraph's own text, so it raises the first line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A list level states its own character formatting — <c>w:lvl/w:rPr</c> in OOXML, the level's
    /// <c>grpprlChpx</c> in WW8 — and it is regularly a different size from the item's text. Writer's
    /// label is a portion in the line (<c>SwNumberPortion</c>), so
    /// <c>SwLineLayout::CalcLine</c> folds it into the line's maxima and a 12 pt label over 11 pt text
    /// gives a 12 pt line. Measured on <c>loi_format_letter_of_intent-a-320-214-a330.doc</c>, whose
    /// bulleted items are 11 pt under a 12 pt level: LibreOffice's own pitch through the list is
    /// 13.80 pt where the item's text alone would give 12.65.
    /// </para>
    /// <para>
    /// Asked as a predicate rather than always folded in, because it decides which of the two layout
    /// paths the paragraph takes. A label no taller than its text changes nothing, and a paragraph that
    /// changes nothing must keep measuring through exactly the path it measured through before.
    /// </para>
    /// </remarks>
    public bool LabelRaisesFirstLine => LabelExtent is not null;

    /// <summary>
    /// The label's line box, when it reaches past the paragraph's own on either side of the baseline,
    /// and null otherwise.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A line is composed side by side, so the test has to be as well.</strong> Writer's
    /// <c>SwLineLayout::CalcLine</c> keeps a running maximum ascent and a running maximum descent and
    /// makes the line out of the two, so a portion that is <em>shorter overall</em> than the text beside
    /// it still deepens the line when its descent alone is deeper. Asking only whether the label's whole
    /// box or its ascent is bigger misses exactly that case, and it is not exotic: it is what a level
    /// that names a different <em>face</em> at the <em>same</em> size does, which round 47 recorded as a
    /// blind spot it could not see and could not act on.
    /// </para>
    /// <para>
    /// Measured against the installed 26.2.4.2 by <c>dotnet/probes/words-b-01/labelshape.py</c>, a 12 pt
    /// level over a 12 pt Liberation Serif item, reading the baseline-to-baseline gap to the paragraph
    /// below:
    /// </para>
    /// <list type="table">
    /// <item><description>
    /// Liberation Mono label — ascent 9.99, descent 3.60 against the item's 11.20 and 2.60, so its
    /// <em>box</em> is 13.59 against 13.80 and neither old term fires. LibreOffice's gap is 14.80 and
    /// ours was 13.80: the whole of the label's extra 1.00 pt of descent was lost.
    /// </description></item>
    /// <item><description>
    /// Caladea label — ascent 10.80, descent 3.00, box 13.80 exactly equal to the item's. LibreOffice
    /// 14.20, ours 13.80.
    /// </description></item>
    /// <item><description>
    /// Carlito (box 14.65, taller outright) and Liberation Sans (ascent 11.26, taller above) are the
    /// controls this must not move, because the old gate already fired for both — and both matched
    /// LibreOffice to the hundredth before and still do.
    /// </description></item>
    /// </list>
    /// <para>
    /// Nothing below this changes: once the extent is returned, <c>MeasuredParagraph.MeasureLine</c>
    /// already folds the object into the line's ascent and descent separately. The defect was only ever
    /// that a whole class of label never got that far.
    /// </para>
    /// </remarks>
    private (Length Height, Length Ascent)? LabelExtent
    {
        get
        {
            if (Label is not { Text.Length: > 0 } label) return null;

            (Length height, Length ascent) = label.LineExtent(Metrics);
            (Length own, Length ownAscent, Length ownDescent) = OwnExtent();

            return height > own || ascent > ownAscent || height - ascent > ownDescent
                ? (height, ascent)
                : null;
        }
    }

    /// <summary>The line box the paragraph's own face and size give, for the label to be compared against.</summary>
    /// <remarks>
    /// <para>
    /// The paragraph's rather than the first line's runs', because this only has to decide whether the
    /// label can matter; <see cref="MeasuredParagraph.HeightOf"/> takes the maximum over whatever is
    /// really on the line, so a run taller than both still wins.
    /// </para>
    /// <para>
    /// The descent is accumulated per face rather than taken as <c>height - ascent</c> at the end. Those
    /// are different numbers whenever the tallest run and the highest-ascent run are not the same run,
    /// and the difference would show up as a label that raises the line in a paragraph mixing two faces
    /// and not in either face alone.
    /// </para>
    /// </remarks>
    private (Length Height, Length Ascent, Length Descent) OwnExtent()
    {
        Length height = Length.Zero;
        Length ascent = Length.Zero;
        Length descent = Length.Zero;

        void Fold(OpenTypeFace face, Length size)
        {
            LineMetrics metrics = LineSpacing.Resolve(face, Metrics, WriterLineBox.LeadingAboveText);
            Length box = Length.FromTwips(metrics.ScaledLineHeight(size).Twips);
            Length above = Length.FromTwips(metrics.ScaledAscent(size).Twips);

            height = Length.Max(height, box);
            ascent = Length.Max(ascent, above);
            descent = Length.Max(descent, box - above);
        }

        foreach (PageRun run in Runs)
        {
            Fold(run.Face, run.MetricEmSize > Length.Zero ? run.MetricEmSize : run.EmSize);
        }

        Fold(Face, EmSize);
        return (height, ascent, descent);
    }

    /// <summary>
    /// Shapes the paragraph's runs, ready for measuring across them.
    /// </summary>
    /// <remarks>
    /// Here rather than in the paginator because the body, a header, a table cell and a text box all need
    /// the same answer, and they used to arrive at it separately — the flow layouter's copy passed
    /// <see cref="Runs"/> straight through, so a uniform paragraph reaching the run path measured as
    /// nothing at all. The paragraph's own face and size close any gap the runs leave, so a document that
    /// formats its text and leaves its paragraph mark unmentioned is normal rather than malformed.
    /// </remarks>
    internal MeasuredParagraph Measure()
    {
        List<FormattedRun> runs = Coalesce(AtParagraphSizeWhereOnlyAnAnchor(Runs));

        if (runs.Count == 0)
        {
            runs.Add(new FormattedRun(
                0, Text.Length, Face, EmSize, Shaping,
                Tracking: Tracking, WidthPerCent: WidthPerCent, Item: Item));
        }

        return MeasuredParagraph.Measure(
            Text, runs, shaper: null, Itemisation, MeasurementObjects(), Metrics,
            BlanksAreTransparentToHeight, WriterLineBox.LeadingAboveText, AddsScriptSpace);
    }

    /// <summary>
    /// The runs, with any that holds nothing but frame anchors measured at the paragraph's own size.
    /// </summary>
    /// <remarks>
    /// <para>
    /// U+0001 stands for a thing that takes a position and is <em>not</em> text — a floating frame, an
    /// as-character picture, a comment mark. The run around it still carries a font size, and a document
    /// routinely states a large one there: a logo run set at 26 pt because that is what the heading beside
    /// it was. **The reference does not let that size reach the line's height**, because Writer builds the
    /// line out of portions and a run with no text makes no text portion — a fly is a
    /// <c>SwFlyCntPortion</c> of its own height, and an at-character fly is not a portion at all.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2 on ten authored variants of one real paragraph
    /// (<c>probes/words-r53/</c>), reading the height the paragraph adds over an empty one:
    /// </para>
    /// <code>
    ///   case                                       reference   before   after
    ///   a run of text at 26 pt                         20.60    19.10   19.10
    ///   anchored drawing, run at 10 pt                  0.00    -1.10   -1.10
    ///   anchored drawing, run at 26 pt                  0.00    17.25   -1.10
    ///   anchored drawing at 26 pt, text beside it       0.00    17.25    0.00
    ///   as-character drawing, run at 10 pt              7.00     6.95    6.95
    ///   as-character drawing, run at 26 pt              7.00    17.25    6.95
    ///   as-character at 26 pt, text beside it           9.70    17.25    9.70
    /// </code>
    /// <para>
    /// The reference's answer is the same at both sizes on every row, and the rows where the run's size
    /// already matched the paragraph's are the rows we were already right on — which is why this never
    /// showed as a systematic error and why it is worth 34 pt on one document and nothing on most.
    /// </para>
    /// <para>
    /// The measurement half only. A <see cref="PageRun"/> also says what to <em>draw</em>, and an anchor
    /// draws nothing, so nothing downstream of the drawing pass can see this. A run holding an anchor
    /// <em>and</em> text keeps its own size, because then it really does make a text portion.
    /// </para>
    /// </remarks>
    private IReadOnlyList<PageRun> AtParagraphSizeWhereOnlyAnAnchor(IReadOnlyList<PageRun> runs)
    {
        List<PageRun>? rewritten = null;

        for (int i = 0; i < runs.Count; i++)
        {
            if (!HoldsNothingButAnchors(Text, runs[i])) continue;

            rewritten ??= [.. runs];
            rewritten[i] = runs[i] with { Face = Face, EmSize = EmSize, MetricEmSize = default };
        }

        return rewritten ?? runs;
    }

    /// <summary>The character a frame, a picture or a comment mark occupies.</summary>
    /// <remarks>
    /// The same one every word-processing reader emits — see <c>DocxLayoutSource.AnchorCharacter</c>,
    /// <c>OdtLayoutSource</c>, <c>Ww8DocumentReader</c> and <c>RtfDocumentReader</c> — so the rule
    /// serves all four formats rather than the one it was found on.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    /// <summary>True when a run's whole range is anchor characters.</summary>
    /// <remarks>
    /// An empty range is not: a zero-length run is dropped before it can be measured, and answering
    /// true for one would rewrite a run that stands for nothing at all.
    /// </remarks>
    private static bool HoldsNothingButAnchors(string text, PageRun run)
    {
        int end = Math.Min(run.End, text.Length);
        if (end <= run.Start) return false;

        for (int at = Math.Max(run.Start, 0); at < end; at++)
        {
            if (text[at] != AnchorCharacter) return false;
        }

        return true;
    }

    /// <summary>
    /// The inline objects the measurement sees, which is the drawn ones plus the list label.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Not <see cref="InlineObjects"/>, and the difference is deliberate.</strong> That property
    /// is what the drawing pass walks alongside the paragraph's as-character frames, one for one; a
    /// phantom in it would hang the wrong frame on the wrong line. This list is measurement's alone.
    /// </para>
    /// <para>
    /// The label enters as a zero-width object at offset nought — the room for it is already made by
    /// <see cref="Format"/>'s widened first-line indent, so all that is left to say is how tall it is and
    /// where its baseline sits. An object at nought touches the first line only, which is where the label
    /// is drawn, and a zero-width one at the head of the text neither cuts a run nor moves a break: see
    /// <c>MeasuredParagraph.Split</c>, which skips a boundary at a run's own start.
    /// </para>
    /// </remarks>
    private List<InlineObject>? MeasurementObjects()
    {
        if (LabelExtent is not (Length height, Length ascent)) return HasInlineObjects ? [.. InlineObjects] : null;

        List<InlineObject> objects =
            [new InlineObject(0, Length.Zero, height, ascent, RaisesTextHeight: true)];
        objects.AddRange(InlineObjects);
        return objects;
    }

    /// <summary>
    /// The runs' measurement halves, with adjacent identical ones joined.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>What makes a drawing-only property free.</strong> A <see cref="PageRun"/> carries both
    /// what changes a width and what only changes a mark — a colour, a highlight, an underline — and the
    /// readers split a paragraph into runs whenever any of them varies, because a property dropped by
    /// the uniform-paragraph shortcut is a property never drawn. Without this, that split reaches
    /// measurement: a shaper called twice across a boundary loses the kern pair that straddles it, so
    /// underlining a sentence would make it fractionally wider and could move a line break. Joining the
    /// runs whose <see cref="FormattedRun"/>s are equal restores exactly the shaping the paragraph would
    /// have had, which is the invariant worth stating outright — <em>a property that decides only what a
    /// mark looks like cannot decide where it lands.</em>
    /// </para>
    /// <para>
    /// Only <em>adjacent</em> and only <em>identical</em>: a run boundary that is a real change of face
    /// or size still breaks the shaping context, and it should — those are different fonts, and there is
    /// no kern pair across them to lose.
    /// </para>
    /// </remarks>
    private static List<FormattedRun> Coalesce(IReadOnlyList<PageRun> runs)
    {
        List<FormattedRun> formatted = new(runs.Count);

        foreach (PageRun run in runs)
        {
            FormattedRun next = run.ToFormattedRun();

            // Equal in every field but the range, and butting up against the one before it. The record
            // struct's own equality is what decides "identical", so a field added to the measurement
            // half is accounted for here without this method being touched.
            if (formatted.Count > 0
                && formatted[^1].End == next.Start
                && formatted[^1] with { Start = next.Start, Length = next.Length } == next)
            {
                formatted[^1] = formatted[^1] with
                {
                    Length = formatted[^1].Length + next.Length,
                };
                continue;
            }

            formatted.Add(next);
        }

        return formatted;
    }
}

/// <summary>
/// One note anchored in a paragraph: a footnote or an endnote.
/// </summary>
/// <remarks>
/// The body is blocks rather than paragraphs for the same reason a cell's is — a note can contain a table,
/// and it is laid out by <see cref="FlowLayouter"/> either way.
/// </remarks>
public sealed record PageNote
{
    /// <summary>The note's body.</summary>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>
    /// Where its anchor sits in the citing paragraph's text.
    /// </summary>
    /// <remarks>
    /// A character offset, which the readers already mark with U+0001 — the anchor occupies a position and
    /// has a width but is not text. The offset is what decides which page the note lands on: the page
    /// holding the <em>line</em> that contains this offset.
    /// </remarks>
    public int Offset { get; init; }

    /// <summary>True for an endnote, which is a class rather than a position — see <see cref="Placement"/>.</summary>
    /// <remarks>
    /// Kept apart from the placement because the two really are different questions: an endnote numbered in
    /// roman and collected at the end of a section is still an endnote, and a reader wanting to list a
    /// document's endnotes should not have to know where they were put.
    /// </remarks>
    public bool IsEndnote { get; init; }

    /// <summary>Where the note collects.</summary>
    /// <remarks>
    /// Defaults to the foot of the page, which is what a footnote is and what an endnote becomes when the
    /// document asks for its endnotes at the end of each section.
    /// </remarks>
    public NotePlacement Placement { get; init; }

    /// <summary>Where this class of note begins counting again.</summary>
    /// <remarks>
    /// Carried on the note beside <see cref="Placement"/>, and for the same reason: both are properties of the
    /// note's <em>class</em> that only pagination can act on, and the paginator is handed notes rather than the
    /// document. This is the one numbering rule a reader cannot resolve — a note's number under a restart is
    /// its position within its page, and which page it is on is what filling the page decides.
    /// </remarks>
    public NoteRestart Restart { get; init; }

    /// <summary>
    /// How this note's class is numbered, for a pagination pass that has to number it again.
    /// </summary>
    /// <remarks>
    /// The sequence and the start value, which <see cref="Restart"/> alone cannot supply: a per-page restart
    /// says the count begins again and this says what the count is written in. Defaults to the footnote
    /// sequence, which is what a note whose reader states nothing renders as.
    /// </remarks>
    public NoteNumbering Numbering { get; init; } = NoteNumbering.Footnotes;

    /// <summary>
    /// The citation this note carries as it was read, in document order.
    /// </summary>
    /// <remarks>
    /// Kept so that a renumbering pass can find it again. It sits in the citing paragraph's text at
    /// <see cref="Offset"/> and in the note body's first paragraph at <see cref="BodyOffset"/>, in both cases
    /// exactly this many characters long — LibreOffice draws a note's number twice and the readers emit it
    /// twice, so both have to be rewritten or the sentence and the note disagree about which note it is.
    /// </remarks>
    public string Citation { get; init; } = "";

    /// <summary>
    /// Where the citation sits in the first block of <see cref="Blocks"/>.
    /// </summary>
    /// <remarks>
    /// Zero in three of the four formats, which prepend it, and not in DOCX: the note body marks where its own
    /// number goes with a <c>w:footnoteRef</c>, and a note beginning with a tab puts it at one rather than at
    /// nought. Recorded rather than searched for, because searching a note's text for the string "1" finds
    /// whatever the note happens to say first.
    /// </remarks>
    public int BodyOffset { get; init; }
}

/// <summary>
/// One run of a paragraph: a range of its text with its own formatting.
/// </summary>
/// <remarks>
/// The measurement half and the drawing half of a run travel together here, unlike in
/// <see cref="FormattedRun"/>, which carries only what changes a width. A colour does not move a line
/// break but it does decide what a backend is handed, and splitting the two would mean matching them up
/// again by range.
/// </remarks>
/// <param name="Start">The run's first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Face">The face it is set in.</param>
/// <param name="EmSize">The em size it is set at.</param>
/// <param name="Font">The resolved reference, for a backend that has to name the face.</param>
/// <param name="Colour">The colour it is drawn in.</param>
/// <param name="Shaping">How it is shaped.</param>
/// <param name="Rise">
/// How far the run is raised above the baseline; negative lowers it. What a superscript is, together with
/// the smaller <paramref name="EmSize"/> that goes with it — the two are independent, and a document can
/// raise text without shrinking it.
/// </param>
/// <param name="CaseMap">
/// The case the run's text is drawn in, which is not the case it is stored in — <c>w:caps</c>,
/// <c>w:smallCaps</c> and their counterparts in the other three formats. Resolved away by
/// <see cref="CaseMapping.Apply"/> before the paragraph is measured, so nothing downstream of a reader
/// ever sees a value other than <see cref="PageCaseMap.None"/>.
/// </param>
/// <param name="MetricEmSize">
/// The size the run's line metrics are taken at, or zero for <paramref name="EmSize"/>. Set only by the
/// small-capitals split; see <see cref="FormattedRun.MetricEmSize"/> for why the two sizes differ.
/// </param>
/// <param name="Highlight">
/// The band drawn behind the run — Word's highlighter and ODF's character background — or transparent
/// when it has none. It changes no measurement: the band takes the room the glyphs already had, so a
/// document gains and loses highlighting without a line moving.
/// </param>
/// <param name="Underline">
/// How the run is underlined — <c>w:u</c>, <c>sprmCKul</c>, <c>\ul</c> and
/// <c>style:text-underline-style</c>. Like <paramref name="Highlight"/> it changes no measurement: the
/// rule is drawn across the advance the glyphs already had, so nothing reflows when it appears.
/// <para>
/// A tri-state and not a flag, because <c>w:u w:val="double"</c>, <c>\uldb</c>, ODF's
/// <c>style:text-underline-type="double"</c> and <c>sprmCKul</c> operand 3 all ask for
/// <em>two thinner lines at their own offsets</em> rather than one. See <see cref="TextUnderline"/>.
/// </para>
/// </param>
/// <param name="IsStruckThrough">
/// True when a rule is drawn through the run — <c>w:strike</c> and <c>w:dstrike</c>,
/// <c>sprmCFStrike</c> and <c>sprmCFDStrike</c>, <c>\strike</c> and
/// <c>style:text-line-through-style</c>. The doubled forms are folded onto the single one, which is
/// what the extraction side does with the same four properties.
/// </param>
/// <param name="Tracking">
/// A fixed distance put between the run's characters, zero for none — the <c>w:spacing</c> of a
/// <c>w:rPr</c>, <c>sprmCDxaSpace</c>, <c>\expndtw</c> and <c>fo:letter-spacing</c>. Unlike the two rules
/// above it <em>does</em> change a measurement, so a run carrying it must survive the uniform-paragraph
/// shortcut or the paragraph is measured without it. See <see cref="FormattedRun.Tracking"/> for how the
/// distance is charged.
/// </param>
/// <param name="WidthPerCent">
/// How wide the run's glyphs are drawn against their own design, 100 for none — <c>w:rPr/w:w</c> and
/// <c>\charscalex</c>. It changes a measurement like <paramref name="Tracking"/> does and so has the same
/// duty to survive the uniform-paragraph shortcut, and it changes the glyphs' shape as well, which
/// tracking does not. See <see cref="FormattedRun.WidthPerCent"/> for why the factor is not the
/// percentage.
/// </param>
/// <param name="Item">
/// The font item this run is set from, or default when the reader distinguishes none. It carries
/// the family class and the language the glyph-fallback pattern is built with; see
/// <see cref="XRay.Office.Text.Fonts.FontItem"/>.
/// </param>
/// <param name="Link">
/// Where the run leads when it is clicked, or null when it is not a link. An address is an external
/// link; an address beginning <c>#</c> names a bookmark in this document. It changes no measurement
/// and draws nothing: a PDF turns it into a link annotation over the run, a raster ignores it.
/// </param>
public readonly record struct PageRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    FontReference? Font = null,
    Colour Colour = default,
    ShapingOptions Shaping = default,
    Length Rise = default,
    PageCaseMap CaseMap = PageCaseMap.None,
    Length MetricEmSize = default,
    Colour Highlight = default,
    TextUnderline Underline = TextUnderline.None,
    bool IsStruckThrough = false,
    Length Tracking = default,
    int WidthPerCent = 100,
    FontItem Item = default,
    string? Link = null)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>
    /// <see cref="WidthPerCent"/> as the factor this run's advances and glyphs are drawn at.
    /// </summary>
    public double WidthScale => TextWidthScale.Of(EmSize, WidthPerCent);

    /// <summary>
    /// The shaping this run is drawn with, once its tracking has had its say.
    /// </summary>
    /// <remarks>
    /// The drawing half of <see cref="FormattedRun.EffectiveShaping"/>, and it has to be the same
    /// answer: the measurement decided where the line broke on the strength of it.
    /// </remarks>
    public ShapingOptions EffectiveShaping => Shaping.WithTracking(Tracking);

    /// <summary>True when a rule of any kind is drawn under the run.</summary>
    public bool IsUnderlined => Underline != TextUnderline.None;

    /// <summary>True when the run carries a rule under it, through it, or both.</summary>
    public bool IsDecorated => IsUnderlined || IsStruckThrough;

    /// <summary>The colour to draw with, black when the run states none.</summary>
    /// <remarks>
    /// A <c>default</c> colour is fully transparent black, which would draw nothing — so an unstated
    /// colour has to mean the document's text colour rather than the struct's default.
    /// </remarks>
    public Colour EffectiveColour => Colour.A == 0 ? Core.Graphics.Colour.Black : Colour;

    /// <summary>
    /// The colour this run is drawn in over a given background.
    /// </summary>
    /// <remarks>
    /// <see cref="EffectiveColour"/> answers the same question with no background to answer it
    /// against, which is the same answer wherever the background is not dark. Transparent
    /// <see cref="Colour"/> is OOXML's <c>auto</c> and ODF's absent colour alike — the state
    /// LibreOffice calls <c>COL_AUTO</c> — and it is resolved against the frame's brush at the
    /// drawing pass rather than at the reader, because the reader does not know which cell the
    /// paragraph will land in.
    /// </remarks>
    /// <param name="background">The brush behind the run, or transparent for none.</param>
    public Colour ColourOn(Colour background)
        => Colour.A != 0 ? Colour : AutomaticColour.Over(background);

    /// <summary>True when the run is drawn on a coloured band rather than on the page.</summary>
    /// <remarks>
    /// Transparent means no band, as it does for <see cref="Colour"/>: a highlight is an addition to the
    /// page rather than something every run has, and the struct's default has to mean its absence.
    /// </remarks>
    public bool IsHighlighted => Highlight.A != 0;

    /// <summary>The measurement half of this run.</summary>
    /// <remarks>
    /// <see cref="Item"/> is carried, and it has to be: it decides which face a character this one
    /// cannot draw is looked for in, and measurement and drawing choosing different faces for the
    /// same character is a line laid out at one width and painted at another.
    /// </remarks>
    public FormattedRun ToFormattedRun()
        => new(Start, Length, Face, EmSize, Shaping, MetricEmSize, Tracking, WidthPerCent, Item);

    /// <summary>
    /// True when two resolved fonts disagree about whether their glyphs are drawn leaning.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The four readers each decide whether a paragraph's formatting <em>varies</em>, and fold it into
    /// a single run when it does not. That fold drops everything the runs disagreed about, so every
    /// property that has to reach the page has to be on the predicate — which is why highlight,
    /// underline and strike-through are each on it with a sentence saying why. This is the same
    /// sentence for the one that was missed.
    /// </para>
    /// <para>
    /// <strong>It is invisible on nearly every family, which is why it survived.</strong> An italic run
    /// of <c>Arial</c> resolves to <c>LiberationSans-Italic</c> — a different
    /// <see cref="OpenTypeFace"/>, so <c>face != paragraphFace</c> already fires and the run survives
    /// the fold. The families with <em>no</em> italic installed are exactly the fallback faces: DejaVu
    /// Sans and DejaVu Serif ship Book and Bold and nothing else here. An italic run that falls back to
    /// one of those resolves to the <em>same</em> face as its upright neighbour, passes every other
    /// test, and loses its lean at the fold.
    /// </para>
    /// <para>
    /// Measured, `probes/words-r56/oblique-uniform.py`, ten authored packages of one paragraph and two
    /// runs. A run stating only <c>w:i</c> in a fallback family: reference 23 sheared glyphs, ours
    /// <b>0</b>. The same run with a <c>w:sz</c> added — a property the predicate already tests —
    /// reference 23, ours 22. The two differ by one thing and no reading of <c>w:i</c> predicts that.
    /// </para>
    /// <para>
    /// Adding it costs no measurement. <see cref="ToFormattedRun"/> does not carry
    /// <see cref="Font"/>, so a paragraph split only by this is rejoined by
    /// <c>PageContent.Coalesce</c> into exactly the shaping it would have had — and the slant itself
    /// moves no advance, since the reference hands it to HarfBuzz as a synthetic slant, which moves
    /// outlines and leaves widths alone.
    /// </para>
    /// </remarks>
    public static bool LeansDifferently(FontReference? run, FontReference? paragraph)
        => (run?.SyntheticOblique ?? false) != (paragraph?.SyntheticOblique ?? false);
}

/// <summary>
/// One line, placed on a page.
/// </summary>
/// <param name="ParagraphIndex">Which paragraph of the input it belongs to.</param>
/// <param name="LineIndex">Which line of that paragraph it is, counted from the paragraph's first.</param>
/// <param name="Box">The line as its paragraph laid it out, relative to the paragraph's top.</param>
/// <param name="Top">
/// Where the line's box sits on this page, measured from the top of the page's body area — so unlike
/// <see cref="LineBox.Top"/> this is a position on a page rather than within a paragraph.
/// </param>
/// <param name="Column">
/// Which column of the page it is in, counted from zero. Zero for the single-column text that most
/// documents are, so the field costs nothing to ignore — but it is what a caller has to consult to know
/// <em>which</em> rectangle <see cref="Top"/> is measured from, since a second column's lines start again
/// at the top of the page.
/// </param>
/// <param name="UpperSpace">
/// How much of the gap above this line is the paragraph's <em>own</em> upper spacing, as collapsing and
/// the top-of-frame rule left it. Zero on every line but a paragraph's first.
/// </param>
/// <param name="Columns">
/// How many columns the rectangle <see cref="Column"/> indexes was divided into, and
/// <paramref name="ColumnGap"/> the gap between them.
/// </param>
/// <param name="ColumnGap"><inheritdoc cref="Columns" path="/summary"/></param>
/// <param name="ColumnRuler">
/// The stated widths of those columns, already fitted to the body's measure, for a section that does not
/// space them evenly; null for the ordinary case.
/// </param>
/// <param name="FlyDisplacement">
/// How much of <paramref name="Top"/> is a floated table having pushed this paragraph clear of itself.
/// Zero on every line of every paragraph but the first one a fly displaced — see
/// <see cref="ParagraphTop"/> for why it has to come back off again.
/// </param>
/// <param name="BodyLeft">
/// The left edge of the text area this line was laid out in, when that is not the page's own — see
/// <see cref="BodyWidth"/>, which is what says whether either is stated.
/// </param>
/// <param name="BodyWidth">
/// The width of that text area, or zero when the line takes the page's. Zero rather than a nullable
/// pair because a text area of no width is not a thing a section can state, so the degenerate value is
/// free to mean <em>unstated</em> — and because every line of every document that has no text section in
/// it then leaves the two at their default.
/// </param>
/// <remarks>
/// <see cref="UpperSpace"/> is carried because a frame anchored to the paragraph is positioned from a
/// point above the line: Writer's <c>SwAnchoredObjectPosition::GetTopForObjPos</c>
/// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx:225</c>) takes the anchor frame's own
/// top and adds back only <c>GetUpperSpaceAmountConsideredForPrevFrame</c> — the previous paragraph's
/// lower space and line spacing — so the paragraph's own space-before is <em>not</em> in the origin. That
/// difference is not recoverable from <see cref="Top"/> alone, because collapsing, contextual spacing and
/// the top-of-page rule each change how much of the gap the paragraph contributed.
///
/// <para>
/// <see cref="Columns"/> is carried for a reason the single-column case hides: one page can hold sections
/// of different column counts. A continuous section break in the middle of a page opens a two-column
/// stretch and the next one closes it, so a page can be one column at the top, two in the middle and one
/// again below — and the page as a whole then has no single answer. Reading the count off the page put
/// the *last* section's answer on every line of it, which drew a full-width paragraph into half a column.
/// </para>
/// <para>
/// <see cref="BodyWidth"/> is carried for the other half of the same argument. A page carries one text
/// area and it is whichever section's was current when the page was emitted, but an ODF
/// <c>text:section</c> is a frame inside the body and takes its own indents
/// (<c>SwSectionFrame::Init</c>, <c>sw/source/core/layout/sectfrm.cxx</c>:129-166), so a page can hold a
/// stretch measured from one left edge above a stretch measured from another. Line breaking already used
/// the indented measure; the drawing used the page's, which put an indented two-column section's first
/// column at 72 pt where 26.2.4.2 draws it at 108.
/// </para>
/// </remarks>
public readonly record struct PlacedLine(
    int ParagraphIndex,
    int LineIndex,
    LineBox Box,
    Length Top,
    int Column = 0,
    Length UpperSpace = default,
    int Columns = 1,
    Length ColumnGap = default,
    ColumnRuler? ColumnRuler = null,
    Length FlyDisplacement = default,
    Length BodyLeft = default,
    Length BodyWidth = default)
{
    /// <summary>True when the line was laid out in a text area of its own rather than the page's.</summary>
    /// <remarks>
    /// <see cref="BodyWidth"/> is the discriminator and <see cref="BodyLeft"/> is meaningless without it.
    /// </remarks>
    public bool HasOwnBody => BodyWidth > Length.Zero;

    /// <summary>Where a frame anchored to this line's paragraph measures its offset from.</summary>
    /// <remarks>
    /// The paragraph's top for object positioning — see <see cref="UpperSpace"/>. Equal to
    /// <see cref="Top"/> for every line that is not a paragraph's first, and for a paragraph whose space
    /// above was collapsed away or dropped at the top of a page.
    ///
    /// <para>
    /// <see cref="FlyDisplacement"/> comes off for a reason the field's own summary cannot give: Writer
    /// positions a paragraph's anchored objects when the paragraph is <em>first</em> formatted, which is
    /// before a floated table above it has pushed the paragraph clear of itself, and it does not position
    /// them again afterwards. So the object stays where the flow was and the text moves out from under
    /// it. Measured on 21 authored documents in <c>probes/words-fly-clearance/</c>: with a fly filling
    /// the column and a text box anchored to the paragraph directly after it, 24.2.7.2 draws the box at
    /// the top of the body at every fly height, and draws the paragraph's own text a line below the fly's
    /// bottom. Put an empty paragraph between the two and the box moves down with its paragraph — that
    /// paragraph is no longer the one being displaced, which is exactly what makes this a property of the
    /// displacement rather than of the anchor.
    /// </para>
    /// </remarks>
    public Length ParagraphTop => Top - UpperSpace - FlyDisplacement;

    /// <summary>The baseline's distance from the top of the body area.</summary>
    public Length Baseline => Top + Box.Baseline;

    /// <summary>True when this is the first line of its paragraph.</summary>
    public bool StartsParagraph => LineIndex == 0;
}

/// <summary>
/// A flow of paragraphs laid out into a rectangle of its own: a header, a footer, or a table cell.
/// </summary>
/// <remarks>
/// <para>
/// One type for the three because they are the same thing seen three times — a list of paragraphs, the
/// lines they broke into, and the rectangle those lines are measured from. What differs is only where the
/// rectangle is and who decided its width, which is the caller's business rather than the flow's. Sharing
/// it means one drawing path serves all three, so tabs and per-run formatting cannot drift between a
/// header and a cell.
/// </para>
/// <para>
/// Its own block list rather than an index into the body's, because each of the three <em>is</em> a
/// separate flow: a header's paragraphs are not the document's body text, and a
/// <see cref="PlacedLine.ParagraphIndex"/> pointing into the body would name the wrong paragraph. Two
/// pages sharing one header share this whole object.
/// </para>
/// <para>
/// A flow holds tables as well as lines, because all three of the things it models can contain one: a
/// table inside a cell is how every format writes a nested table, and a table inside a header is how a
/// two-part running head is usually laid out. What a flow does <em>not</em> do is paginate — a nested table
/// that outgrows its cell overflows rather than splitting, since a cell belongs to its row.
/// </para>
/// </remarks>
public sealed record PlacedFlow
{
    /// <summary>The blocks the lines index into.</summary>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>The lines, in order, positioned relative to the area's top.</summary>
    public required IReadOnlyList<PlacedLine> Lines { get; init; }

    /// <summary>The tables inside the flow, with page-coordinate rectangles.</summary>
    public IReadOnlyList<PlacedTable> Tables { get; init; } = [];

    /// <summary>Where the flow sits on the page.</summary>
    public required DocRect Area { get; init; }

    /// <summary>
    /// How far the flow advanced in all — the <em>last</em> block's own lower spacing included.
    /// </summary>
    /// <remarks>
    /// Different from where the ink stops, which is what <see cref="FlowLayouter.Extent"/> reports, and
    /// the difference is a table cell's whole point. Writer's
    /// <c>SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell</c> adds the last frame's lower spacing to the
    /// cell under the <c>AddParaSpacingToTableCells</c> setting, which both the DOC and the DOCX
    /// importers switch on — so in a Word document every cell is as tall as its content plus the space
    /// after its final paragraph. Sizing rows from the ink instead makes each one short by that spacing,
    /// which on a long table is many pages.
    /// </remarks>
    public Length Advance { get; init; }

    /// <summary>
    /// The proportional line spacing the flow's last paragraph would have handed to a paragraph after
    /// it, which nothing collected because there is no paragraph after it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Held apart from <see cref="Advance"/> rather than folded into it because only some callers want
    /// it. Inside a flow the gap is unambiguous — <see cref="ParagraphLeading"/> gives it to the line
    /// below, Writer's newer builds keep it under the line above, and both put the same distance in the
    /// same place — but at the flow's <em>end</em> the two answers differ by the whole gap, and which is
    /// right depends on the frame: a running head grows by it, and a body's last paragraph on a page
    /// does not, since the space belongs to the page break.
    /// </para>
    /// <para>
    /// Measured against the installed 26.2.4.2 by <c>probes/words-w-pitch/mkhdr.py</c>, a two-paragraph
    /// header whose second paragraph is empty, at <c>w:top</c> 720 and <c>w:header</c> 709, with the
    /// body's first baseline reporting the header band:
    /// </para>
    /// <code>
    ///   mark rPr   w:line       reference   this engine before
    ///   10 pt      240 (100%)      774.04   774.05
    ///   10 pt      480 (200%)      762.54   774.05
    ///   12 pt      240 (100%)      771.74   771.75
    ///   12 pt      360 (150%)      764.89   771.75
    ///   12 pt      480 (200%)      757.99   771.75
    ///   20 pt      480 (200%)      739.54   762.55
    /// </code>
    /// <para>
    /// Every reference row is the header plus the last paragraph's own proportional gap; every "before"
    /// row is the header without it, and moves with the size but not with the spacing. It is 13.75 pt on
    /// <c>OM template for non-complex NCC operators</c>, whose running head ends with an empty 12 pt
    /// paragraph at <c>w:line="480"</c> — enough to take one contents entry per page.
    /// </para>
    /// </remarks>
    public Length TrailingLineSpacing { get; init; }

    /// <summary>True when nothing was laid out.</summary>
    public bool IsEmpty => Lines.Count == 0 && Tables.Count == 0;
}

/// <summary>
/// A page after pagination: how big it is, where its body sits, and which lines landed on it.
/// </summary>
/// <remarks>
/// Lines only, not paragraphs, because a paragraph can span pages and a page is defined by what fits on
/// it. A caller wanting the paragraphs asks which <see cref="PlacedLine.ParagraphIndex"/> values appear;
/// a caller wanting to know whether a paragraph was split compares that across pages.
/// </remarks>
public sealed record LaidOutPage
{
    /// <summary>The page's zero-based position in the document.</summary>
    public required int Index { get; init; }

    /// <summary>
    /// The blocks this page's lines index into, or null for the document's own.
    /// </summary>
    /// <remarks>
    /// Null on almost every page, and the exception is the reason it exists: the endnote pages at the end of a
    /// document are laid out from a flow assembled out of the notes' bodies rather than from the body's blocks,
    /// so their <see cref="PlacedLine.ParagraphIndex"/> counts in a different list. A page carrying its own
    /// list is how that stays correct without every page paying for a copy — and a null here is not "no
    /// blocks" but "the ones the sequence holds".
    /// </remarks>
    public IReadOnlyList<PageBlock>? Blocks { get; init; }

    /// <summary>
    /// The number printed on the page, which is not the index.
    /// </summary>
    /// <remarks>
    /// A section can restart numbering, and a title page numbered zero so that the following page is
    /// one is a real thing people do — so the two are kept apart rather than one derived from the other.
    /// </remarks>
    public required int Number { get; init; }

    /// <summary>The sheet's size.</summary>
    public required DocSize Size { get; init; }

    /// <summary>The border drawn round this page, or null when it carries none.</summary>
    /// <remarks>
    /// Placed by the paginator, which is the only layer that knows both the sheet and the text area
    /// and the only one that knows whether a page is the first of its section — the two things
    /// <c>w:offsetFrom</c> and <c>w:display</c> respectively need. Page furniture rather than
    /// content: a page border never moves <see cref="BodyArea"/>, in either <c>offsetFrom</c> mode.
    /// </remarks>
    public PlacedPageBorder? Borders { get; init; }

    /// <summary>Where body text goes, in page coordinates.</summary>
    /// <remarks>
    /// The whole text area, columns and the gaps between them included. A line's own coordinates are
    /// relative to <em>its column's</em> rectangle rather than to this — see
    /// <see cref="ColumnArea(int)"/> — which for the single-column case are the same thing.
    /// </remarks>
    public required DocRect BodyArea { get; init; }

    /// <summary>How many columns the page's text area is divided into; one for ordinary text.</summary>
    public int ColumnCount { get; init; } = 1;

    /// <summary>The gap between two columns.</summary>
    public Length ColumnGap { get; init; }

    /// <summary>
    /// The columns' own widths for a section that stated them one by one, already fitted to
    /// <see cref="BodyArea"/>; null when the columns are even.
    /// </summary>
    /// <remarks>
    /// Carried on the page for the reason <see cref="ColumnArea(int)"/> is: a renderer is handed a page,
    /// and recomputing the ruler would mean giving it the section too.
    /// </remarks>
    public ColumnRuler? ColumnRuler { get; init; }

    /// <summary>
    /// True when the page's section reads right to left, so that its first column is the rightmost.
    /// </summary>
    /// <remarks>
    /// Carried on the page rather than looked up from the section for the same reason
    /// <see cref="ColumnArea(int)"/> is: a renderer is handed a page, and a page that had to consult the
    /// section could disagree with the one that laid the lines out.
    /// </remarks>
    public bool IsRightToLeft { get; init; }

    /// <summary>
    /// One column's rectangle, which is what a line's own coordinates are relative to.
    /// </summary>
    /// <remarks>
    /// Carried on the page rather than looked up from the section, because a page is what a renderer is
    /// handed: recomputing this would mean giving the renderer the section too, and the two could then
    /// disagree about a page laid out before a geometry change.
    /// </remarks>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public DocRect ColumnArea(int column)
        => Area(BodyArea, ColumnCount, ColumnGap, ColumnRuler, column);

    /// <summary>
    /// The rectangle one line's own coordinates are relative to.
    /// </summary>
    /// <remarks>
    /// A line's own column count rather than the page's, because a page can hold sections that disagree
    /// about it — see <see cref="PlacedLine.Columns"/>.
    /// <para>
    /// A line that states one column takes the whole body, <em>whatever the page states</em>. It used to
    /// take the page's column at the line's own index instead, on the reading that a line stating one
    /// column had stated nothing — and that put a full-measure index inside a two-column page's second
    /// column on `absrc-pac-01-info-note-en.odt`, because the page is written with whichever section is
    /// current when it is emitted. Every line the paginator places records the count in force where it
    /// was laid out, so there is no such thing as a body line that states nothing.
    /// </para>
    /// </remarks>
    /// <param name="line">The line whose rectangle is wanted.</param>
    public DocRect ColumnArea(PlacedLine line)
    {
        DocRect body = BodyAreaOf(line);

        if (line.Columns <= 1) return body;

        return Area(body, line.Columns, line.ColumnGap, line.ColumnRuler, line.Column);
    }

    /// <summary>The text area one line was laid out in, which is the page's unless it states its own.</summary>
    /// <remarks>
    /// Horizontal only: a text section is inset from the body's sides and begins wherever the flow had
    /// reached, so its top and its height are the page's — <see cref="PlacedLine.Top"/> is measured from
    /// the page's body area and stays so. See <see cref="PlacedLine.BodyWidth"/>.
    /// </remarks>
    /// <param name="line">The line whose text area is wanted.</param>
    public DocRect BodyAreaOf(PlacedLine line)
        => line.HasOwnBody
            ? new DocRect(line.BodyLeft, BodyArea.Y, line.BodyWidth, BodyArea.Height)
            : BodyArea;

    /// <summary>
    /// One column's rectangle inside a text area, from either description of the columns.
    /// </summary>
    /// <remarks>
    /// The page and a line each state their own count, gap and ruler, and a line its own text area — a
    /// page can hold sections that disagree about all four — and the arithmetic below is the same for
    /// both, so it lives here rather than twice. A ruler whose count does not match is ignored, which is
    /// the lenient reading a section that states widths for columns it does not have needs.
    /// </remarks>
    /// <param name="body">The text area being divided: the page's, or the line's own.</param>
    /// <param name="count">How many columns it is divided into.</param>
    /// <param name="gap">The gap between two of them.</param>
    /// <param name="ruler">Their stated widths, or null when they are even.</param>
    /// <param name="column">The column wanted, counted from zero at the leading edge.</param>
    private DocRect Area(DocRect body, int count, Length gap, ColumnRuler? ruler, int column)
    {
        int columns = Math.Max(1, count);
        int at = Math.Clamp(column, 0, columns - 1);

        // The leading edge is the right one in a right-to-left section, so its first column is the
        // rightmost — see PageGeometry.IsRightToLeft, where it is measured.
        if (IsRightToLeft) at = columns - 1 - at;

        if (ruler is { } stated && stated.Count == columns)
        {
            return new DocRect(
                body.X + stated.OffsetOf(at), body.Y, stated.WidthAt(at), body.Height);
        }

        Length gaps = gap * (columns - 1);
        Length width = body.Width - gaps;
        width = width > Length.Zero ? width / columns : body.Width;

        return new DocRect(body.X + ((width + gap) * at), body.Y, width, body.Height);
    }

    /// <summary>The lines on the page, in order.</summary>
    public required IReadOnlyList<PlacedLine> Lines { get; init; }

    /// <summary>
    /// The tables on the page, or the parts of them that landed here.
    /// </summary>
    /// <remarks>
    /// Beside the lines rather than among them, because a table is not a run of lines: its cells sit side
    /// by side and each carries its own rectangle. A table crossing a page break appears once per page it
    /// touches, each time with the rows that fit and its headings repeated.
    /// </remarks>
    public IReadOnlyList<PlacedTable> Tables { get; init; } = [];

    /// <summary>
    /// The margin line numbers beside the body's lines, empty for a document that asks for none.
    /// </summary>
    /// <remarks>
    /// Filled by a pass over the finished pages rather than during the fill, because a margin number is
    /// drawn outside the text area and so cannot move a line — see <see cref="LineNumbering"/>.
    /// </remarks>
    public IReadOnlyList<PageLineNumber> LineNumbers { get; init; } = [];

    /// <summary>
    /// The rule <see cref="LineNumbers"/> were produced by, or null when the page carries none.
    /// </summary>
    /// <remarks>
    /// Beside them rather than folded into each, because the face, the size and the shaping are the same
    /// for every number on every page of the document and a backend needs them once — a page carrying
    /// forty-five numbers would otherwise carry forty-five copies of one font reference.
    /// </remarks>
    public LineNumbering? Numbering { get; init; }

    /// <summary>Which section's geometry the page was laid on.</summary>
    public int SectionIndex { get; init; }

    /// <summary>The page's header, or null when it has none.</summary>
    /// <remarks>
    /// Per page rather than per section, because a section's first and even pages can each take a different
    /// one — and because a page number in a header makes even two pages sharing a slot differ once fields
    /// are resolved.
    /// </remarks>
    public PlacedFlow? Header { get; init; }

    /// <summary>The page's footer, or null when it has none.</summary>
    public PlacedFlow? Footer { get; init; }

    /// <summary>
    /// The footnotes at the foot of the page, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Bottom-aligned inside <see cref="BodyArea"/> rather than below it, which is measured rather than
    /// assumed: the last note line's box bottom coincides with the body area's bottom. So the notes take
    /// their room out of the body's, which is why a page with notes holds less text — and why adding one can
    /// push the line that cites it onto the next page.
    /// </remarks>
    public PlacedFlow? Notes { get; init; }

    /// <summary>
    /// The rule above the notes, or null when the page has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A rectangle rather than a line, because that is what it is: Writer's <c>Footnote Separator</c> is a
    /// frame style with a width, a thickness and an alignment, and LibreOffice's PDF export writes it as a
    /// filled path rather than a stroke. Measured from that path — 56.7 to 177.15 pt on an A4 page with 2 cm
    /// margins, half a point thick — which makes it a quarter of the text width, left aligned, 0.5 pt.
    /// </para>
    /// <para>
    /// Carried on the page rather than derived by a backend, because its position depends on where the notes
    /// ended up and only pagination knows that.
    /// </para>
    /// <para>
    /// Both measurements above are Writer's, and a document a Word filter opened gets neither: a fixed two
    /// inches, and a position 60 % of the way down a reservation taken from the default paragraph style.
    /// See <see cref="PaginationOptions.UsesWordNoteSeparator"/>, which is where that switch lives and why
    /// it is not simply part of <see cref="PaginationOptions.Word"/>.
    /// </para>
    /// </remarks>
    public DocRect? NoteSeparator { get; init; }

    /// <summary>
    /// The floating frames that landed on this page, with the rectangles they were given.
    /// </summary>
    /// <remarks>
    /// Beside the lines rather than among them, for the same reason a table is: a frame is placed at a
    /// resolved position rather than stacked, and the lines around it have already been shortened to make
    /// room. A renderer draws these after the body text, which is what puts an opaque frame over the text
    /// it displaced rather than under it.
    /// </remarks>
    public IReadOnlyList<PlacedFrame> Frames { get; init; } = [];

    /// <summary>How much of the body area the lines used.</summary>
    public Length UsedHeight =>
        Lines.Count == 0 ? Length.Zero : Lines[^1].Top + Lines[^1].Box.Height;

    /// <summary>True when nothing landed on the page.</summary>
    public bool IsEmpty => Lines.Count == 0 && Tables.Count == 0;
}

/// <summary>A named place in a paragraph.</summary>
/// <param name="Offset">The character offset the name sits at.</param>
/// <param name="Name">The name a link addresses it by.</param>
public readonly record struct PageAnchor(int Offset, string Name);
