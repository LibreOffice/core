using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
using Paperless.Text.Shaping;

namespace Paperless.Text.Layout;

/// <summary>
/// One run of a paragraph: a range of its text with its own face and size.
/// </summary>
/// <remarks>
/// <para>
/// Runs partition a paragraph's text and never overlap, which is what lets a prefix width be the sum of
/// whole runs plus part of one. Formatting that <em>does</em> overlap — a bold range crossing a hyperlink
/// — is resolved into non-overlapping runs before it reaches here, because measurement needs one answer
/// per character and the document model's interval tags do not give one.
/// </para>
/// <para>
/// Only what changes a measurement is here. A run's colour and its underline do not move a line break, so
/// they travel with whatever draws it rather than with this.
/// </para>
/// </remarks>
/// <param name="Start">The run's first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Face">The face it is set in.</param>
/// <param name="EmSize">The em size it is set at.</param>
/// <param name="Shaping">How it is shaped.</param>
/// <param name="MetricEmSize">
/// The size the run's <em>line metrics</em> are taken at, or zero for <paramref name="EmSize"/> — which is
/// every run but one kind.
/// <para>
/// Small capitals are the kind. A lowercase letter inside a small-capitals run is drawn uppercase at four
/// fifths of the size (<c>SMALL_CAPS_PERCENTAGE</c>, <c>include/editeng/svxfont.hxx</c>) and yet does
/// <em>not</em> shorten the line it sits on: Writer builds the shrunken font locally inside
/// <c>SwSubFont::DoOnCapitals</c> and leaves <c>SwFont::GetHeight</c> — which is what
/// <c>SwLineLayout::CalcLine</c> asks — reporting the unshrunken one. Without this separation a heading
/// set entirely in lowercase small capitals would draw its line four fifths as tall as the reference and
/// repaginate the document under it.
/// </para>
/// </param>
/// <param name="Tracking">
/// A fixed distance added between the run's characters, zero for none.
/// <para>
/// <c>SvxKerningItem</c> / <c>EE_CHAR_KERNING</c>, which DrawingML spells <c>a:rPr/@spc</c> in
/// hundredths of a point and a word processor spells <c>w:spacing</c> in twentieths. It is
/// <em>not</em> pair kerning — that is the face's own <c>kern</c> feature and is applied by the
/// shaper — but a constant per character, commonly negative, that a designer uses to pull a
/// heading in. Ignoring it makes a tracked line measure wider than the reference by the whole
/// accumulated amount, which on a 50-character line at the corpus's commonest value of −0.2 pt is
/// ten points: enough to move the last word of every line onto the next one.
/// </para>
/// <para>
/// <strong>It is a distance between characters, so a run of <c>n</c> of them carries
/// <c>n − 1</c>.</strong> That is what the reference measures
/// (<c>SvxFont::QuickGetTextSize</c>, <c>editeng/source/items/svxfont.cxx:481-500</c>, which adds
/// one per distinct advance and then takes the trailing one back off), and it is exactly what a
/// prefix table can express for a whole paragraph and cannot express for an arbitrary range: the
/// count of gaps inside a range depends on the range. So the table charges the gap
/// <em>before</em> each character, which is right to the last unit for a paragraph measured whole
/// and one tracking unit generous for a line that starts part-way in.
/// </para>
/// </param>
public readonly record struct FormattedRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    ShapingOptions Shaping = default,
    Length MetricEmSize = default,
    Length Tracking = default)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>The size this run's line metrics are scaled by.</summary>
    public Core.Units.Length LineEmSize
        => MetricEmSize > Core.Units.Length.Zero ? MetricEmSize : EmSize;

    /// <summary>True when the run covers a character.</summary>
    public bool Covers(int index) => index >= Start && index < End;
}

/// <summary>
/// Something set <em>in</em> a line that is not text: an as-character picture or frame.
/// </summary>
/// <remarks>
/// <para>
/// Writer's <c>SwFlyCntPortion</c> — a portion of the line, so it takes room <em>on</em> the line rather
/// than room <em>from</em> it the way a floating frame does. The two are worth keeping apart because they
/// are opposite: a floating frame narrows the lines beside it and never moves them along, and this one
/// moves the text after it along and never narrows anything.
/// </para>
/// <para>
/// The offset is the boundary the object occupies, so it sits immediately before the character at that
/// index and the text from there on is pushed along by <see cref="Width"/>. That is why it is a boundary
/// and not a character: three of the four word-processing formats put no character in the text for an
/// inline picture at all, and inventing one would shift every offset a reader recorded — a note's
/// citation, a bookmark, a comment's anchor.
/// </para>
/// <para>
/// <see cref="Height"/> is the line's business rather than the object's: an as-character object usually
/// rests its bottom on the baseline, so the line's ascent has to grow to at least this. Measured against
/// LibreOffice's PDF of <c>picture-anchor.fodt</c>, whose 1 cm picture on a 12 pt line gives an ascent of
/// 28.35 pt where the text alone would give 10.69, and a line 31.46 pt tall where the text alone gives
/// 13.8.
/// </para>
/// <para>
/// <strong>Usually, but not always, which is what <see cref="Ascent"/> is for.</strong> Writer's
/// as-character fly is placed by <c>SwFlyCntPortion::SetBase</c> (<c>sw/source/core/text/porfly.cxx</c>),
/// which asks <c>SwAsCharAnchoredObjectPosition</c> for a position <em>relative to the baseline</em> and
/// then splits it: a negative one becomes the portion's ascent, and a position of nought or more leaves
/// the ascent at nought and lets the object hang below the line instead. Resting on the baseline is only
/// the case where that position is the object's own height. A shape whose vertical orientation is
/// <c>TEXT_LINE</c> with no offset — which is what WW8 writes for a picture set in a line — comes back
/// with nought, and treating it like a picture raises the baseline of the line it sits on by the whole
/// height of the shape. Measured on <c>word-features.doc</c>: LibreOffice keeps the anchor line at
/// 455.51 and draws the box's own text 11.2 pt <em>below</em> it, where resting the box on the baseline
/// put that line at 477.71.
/// </para>
/// </remarks>
/// <param name="Offset">The boundary it occupies, as an index into the paragraph's text.</param>
/// <param name="Width">How far it moves the text after it along the line.</param>
/// <param name="Height">How tall it is.</param>
/// <param name="Ascent">
/// How much of it sits above the baseline, or null for all of it — which is the ordinary inline picture
/// and the reason this is the default rather than a value every caller has to state. The rest hangs below
/// and grows the line's descent, so a line box is always tall enough to hold the whole object either way.
/// </param>
public readonly record struct InlineObject(
    int Offset, Length Width, Length Height, Length? Ascent = null)
{
    /// <summary>How much of the object sits above the baseline, with the default resolved.</summary>
    public Length AboveBaseline => Ascent ?? Height;

    /// <summary>How much of it hangs below, which is what the line's descent has to hold.</summary>
    public Length BelowBaseline => Length.Max(Length.Zero, Height - AboveBaseline);
}

/// <summary>
/// A shaped run, positioned within its paragraph.
/// </summary>
/// <param name="Run">The run's own range and formatting.</param>
/// <param name="Shaped">Its glyphs and prefix widths, in its own face's design units.</param>
/// <param name="Metrics">The line metrics its face resolves to.</param>
public readonly record struct MeasuredRun(
    FormattedRun Run,
    ShapedText Shaped,
    LineMetrics Metrics);

/// <summary>
/// A paragraph measured across its runs.
/// </summary>
/// <remarks>
/// <para>
/// The thing a mixed-formatting paragraph needs that a single-face one does not: widths summed in
/// <em>EMUs</em> rather than in design units. Two runs at different sizes, or in faces with different
/// design grids, have no common unit smaller than the document's — so each run is scaled once from its
/// own grid and the results added, which is the only order that is both exact per run and comparable
/// across runs.
/// </para>
/// <para>
/// The line height comes from the runs on the line, not from the paragraph: a 24 pt word in an 11 pt
/// paragraph makes its own line taller and leaves the others alone. That is why the metrics travel with
/// each run rather than being resolved once, and why <see cref="HeightOf"/> takes a range.
/// </para>
/// </remarks>
public sealed class MeasuredParagraph
{
    private readonly MeasuredRun[] _runs;
    private readonly long[] _prefixEmu;
    private readonly TextItem[] _items;
    private readonly InlineObject[] _objects;

    private readonly bool _blanksAreTransparentToHeight;

    private MeasuredParagraph(
        string text,
        MeasuredRun[] runs,
        long[] prefixEmu,
        TextItem[] items,
        byte paragraphLevel,
        InlineObject[] objects,
        bool blanksAreTransparentToHeight)
    {
        Text = text;
        _runs = runs;
        _prefixEmu = prefixEmu;
        _items = items;
        ParagraphLevel = paragraphLevel;
        _objects = objects;
        _blanksAreTransparentToHeight = blanksAreTransparentToHeight;
    }

    /// <summary>The paragraph's text.</summary>
    public string Text { get; }

    /// <summary>
    /// The runs, in logical order, one per formatting change and per direction, script or face change
    /// within it.
    /// </summary>
    /// <remarks>
    /// Logical rather than visual, which is what Writer's own line portions are: its PDF export emits
    /// them in the order the characters are stored and positions each with an absolute pen. Drawing
    /// right-to-left text needs the visual order, and <see cref="Items"/> carries the levels it is
    /// derived from.
    /// </remarks>
    public IReadOnlyList<MeasuredRun> Runs => _runs;

    /// <summary>
    /// The direction and script sub-runs the paragraph was cut into, in logical order.
    /// </summary>
    /// <remarks>
    /// One entry per stretch of one embedding level and one script — the same partition the runs
    /// were shaped against, minus the formatting changes. A caller drawing the paragraph needs these
    /// to put the runs in visual order; <see cref="TextItemiser.InVisualOrder"/> does that.
    /// </remarks>
    public IReadOnlyList<TextItem> Items => _items;

    /// <summary>The paragraph's own embedding level: even for left to right, odd for right to left.</summary>
    public byte ParagraphLevel { get; }

    /// <summary>
    /// Shapes a paragraph's runs and builds its prefix widths.
    /// </summary>
    /// <remarks>
    /// Runs are sorted and clamped to the text rather than trusted: they come from a document, and one
    /// that overlaps or runs past the end would otherwise corrupt every width after it. A gap between two
    /// runs is filled from the run before it, since a character with no formatting is set in whatever
    /// preceded it.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="runs">Its runs; may be empty, in which case the paragraph measures as nothing.</param>
    /// <param name="shaper">The shaper to use, or null for the default.</param>
    /// <param name="itemisation">
    /// How to cut the paragraph into sub-runs before shaping, or null for the neutral settings: left
    /// to right, and no glyph fallback. A paragraph of Latin prose is cut into one sub-run per
    /// formatting run either way, and shaped in exactly the calls it was shaped in before sub-runs
    /// existed.
    /// </param>
    /// <param name="objects">
    /// The as-character pictures and frames set in the text, or null for a paragraph with none — which is
    /// nearly every paragraph, and the path every one of them took before inline objects existed.
    /// </param>
    /// <param name="grid">
    /// The device grid each run's vertical metrics are rounded through, or null to scale them exactly.
    /// See <see cref="MetricGrid"/>.
    /// </param>
    /// <param name="blanksAreTransparentToHeight">
    /// True to leave tabs and all-blank runs out of a line's height, which is what the Word formats ask
    /// for; see <see cref="HeightOf"/>. False — the default — measures every run on the line, which is
    /// what ODF and RTF want.
    /// </param>
    public static MeasuredParagraph Measure(
        string text,
        IReadOnlyList<FormattedRun> runs,
        ITextShaper? shaper = null,
        ItemisationOptions? itemisation = null,
        IReadOnlyList<InlineObject>? objects = null,
        MetricGrid? grid = null,
        bool blanksAreTransparentToHeight = false)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(runs);

        ITextShaper engine = shaper ?? TextShaper.Default;
        InlineObject[] inline = Inline(text, objects);
        List<FormattedRun> formatted = Split(Normalise(text, runs), inline);
        ItemisationOptions options = itemisation ?? DefaultItemisation(formatted);

        // The bidi algorithm is resolved over the whole paragraph, never per formatting run: a run
        // boundary is a change of font, not a change of direction, and resolving each separately
        // would let a bold word inside a Hebrew sentence see the paragraph's direction instead of
        // the sentence's.
        BidiParagraph bidi = BidiParagraph.Resolve(text, options.BaseDirection);
        List<TextItem> items = TextItemiser.Itemise(text, bidi);

        List<MeasuredRun> measured = [];
        long[] prefix = new long[text.Length + 1];
        long[] tracking = new long[text.Length];
        bool tracked = false;
        long running = 0;

        foreach (FormattedRun run in formatted)
        {
            foreach (FormattedRun part in SubRuns(text, run, items, options))
            {
                ShapedText shaped = engine.Shape(
                    part.Face, text.AsSpan(part.Start, part.Length), part.Shaping);

                measured.Add(new MeasuredRun(part, shaped, LineSpacing.Resolve(part.Face, grid)));

                // Each sub-run's own prefix widths, scaled from its own grid into EMUs and added to
                // the running total. Summing in design units instead would add numbers from two
                // different grids; reading the running total off the table instead would break the
                // moment a control character left a gap between two sub-runs.
                for (int i = 1; i <= part.Length; i++)
                {
                    prefix[part.Start + i] = running + shaped.WidthUpTo(i, part.EmSize).Emu;
                }

                if (part.Tracking != Length.Zero)
                {
                    tracked = true;
                    for (int i = 0; i < part.Length; i++) tracking[part.Start + i] = part.Tracking.Emu;
                }

                running += shaped.Width(part.EmSize).Emu;
            }
        }

        // Any position no sub-run covered — a text with no runs at all, a trailing gap, or a format
        // control character that was cut out — carries the width of the last position that was
        // covered, so the table stays monotonic and the control measures as nothing.
        for (int i = 1; i <= text.Length; i++)
        {
            if (prefix[i] < prefix[i - 1]) prefix[i] = prefix[i - 1];
        }

        // Tracking is charged for the gap *before* each character, so the first pays nothing and a
        // paragraph of n characters carries n - 1 gaps, which is what the reference measures. See
        // FormattedRun.Tracking for why a prefix table cannot do better than that.
        if (tracked)
        {
            long carried = 0;

            for (int i = 1; i <= text.Length; i++)
            {
                prefix[i] += carried;
                carried += tracking[i - 1];

                // A tracking value more negative than a character is wide would walk the table
                // backwards, and every width read out of it is a difference of two entries.
                if (prefix[i] < prefix[i - 1]) prefix[i] = prefix[i - 1];
            }
        }

        // Each object widens every prefix *past* the boundary it occupies and none at or before it, which
        // is what makes it sit between two characters rather than replace one. The consequence worth
        // stating: a line that ends at the boundary does not pay for the object and a line that starts
        // there does, so a picture too wide for the room left on a line moves to the next line whole.
        foreach (InlineObject one in inline)
        {
            for (int i = one.Offset + 1; i <= text.Length; i++) prefix[i] += one.Width.Emu;
        }

        return new MeasuredParagraph(
            text, [.. measured], prefix, [.. items], bidi.ParagraphLevel, inline,
            blanksAreTransparentToHeight);
    }

    /// <summary>
    /// The inline objects, clamped into the text and put in order.
    /// </summary>
    /// <remarks>
    /// Sorted because the drawing pen walks them alongside the runs and expects to meet them in position
    /// order, and clamped because the offsets come from a document: a frame anchored past the end of the
    /// paragraph it claims to be in is a repair rather than a reason to throw.
    /// </remarks>
    private static InlineObject[] Inline(string text, IReadOnlyList<InlineObject>? objects)
    {
        if (objects is null || objects.Count == 0) return [];

        List<InlineObject> kept = [];

        foreach (InlineObject one in objects)
        {
            if (one.Width <= Length.Zero && one.Height <= Length.Zero) continue;

            kept.Add(one with { Offset = Math.Clamp(one.Offset, 0, text.Length) });
        }

        kept.Sort(static (left, right) => left.Offset.CompareTo(right.Offset));
        return [.. kept];
    }

    /// <summary>
    /// Cuts the runs at every inline object's boundary, so that none is shaped across one.
    /// </summary>
    /// <remarks>
    /// A picture between two words breaks the shaping context exactly as a run boundary does — Writer
    /// makes it a portion of its own for that reason — and cutting here rather than only where the line is
    /// drawn is what keeps the two agreeing. A run shaped whole and drawn in halves measures very slightly
    /// differently, which is enough to move a line break; see the remark on
    /// <c>PageDrawing.InVisualOrder</c>, which pays the same cost for the same reason.
    /// </remarks>
    private static List<FormattedRun> Split(List<FormattedRun> runs, InlineObject[] objects)
    {
        if (objects.Length == 0) return runs;

        List<FormattedRun> cut = [];

        foreach (FormattedRun run in runs)
        {
            int at = run.Start;

            foreach (InlineObject one in objects)
            {
                if (one.Offset <= at || one.Offset >= run.End) continue;

                cut.Add(run with { Start = at, Length = one.Offset - at });
                at = one.Offset;
            }

            if (at < run.End) cut.Add(run with { Start = at, Length = run.End - at });
        }

        return cut;
    }

    /// <summary>
    /// The itemisation to use when the caller stated none.
    /// </summary>
    /// <remarks>
    /// The base direction comes from the runs' own shaping options, so a caller that has been saying
    /// <c>RightToLeft</c> on every run of a Hebrew paragraph keeps getting a right-to-left paragraph.
    /// That was the only way to say it before this existed, and silently changing what it meant would
    /// re-align every such document.
    /// </remarks>
    private static ItemisationOptions DefaultItemisation(List<FormattedRun> runs)
        => runs.Count > 0 && runs[0].Shaping.RightToLeft
            ? new ItemisationOptions { BaseDirection = BidiDirection.RightToLeft }
            : ItemisationOptions.Default;

    /// <summary>
    /// Cuts one formatting run into the pieces a shaper can take.
    /// </summary>
    /// <remarks>
    /// The whole run unchanged is the common case and is returned untouched — not merely equivalent,
    /// but the same <see cref="ShapingOptions"/> the caller passed, so a paragraph of Latin prose
    /// reaches HarfBuzz in the identical call it did before any of this existed. Anything else is
    /// split at every change of direction, script or face, and each piece is told which it is.
    /// </remarks>
    private static List<FormattedRun> SubRuns(
        string text, FormattedRun run, List<TextItem> items, ItemisationOptions options)
    {
        List<FormattedRun> parts = [];

        foreach (TextItem item in items)
        {
            int start = Math.Max(item.Start, run.Start);
            int end = Math.Min(item.End, run.End);
            if (end <= start) continue;

            foreach (FaceRun face in FontItemiser.Split(
                         text, start, end - start, run.Face,
                         options.GlyphFallback, options.OnGlyphFallback))
            {
                bool wholeRun = face.Start == run.Start && face.End == run.End;
                bool plain = wholeRun && !item.IsRightToLeft && !face.IsFallback;

                parts.Add(run with
                {
                    Start = face.Start,
                    Length = face.Length,
                    Face = face.Face,
                    Shaping = plain
                        ? run.Shaping
                        : run.Shaping with
                        {
                            Script = item.Script,
                            RightToLeft = item.IsRightToLeft,
                        },
                });
            }
        }

        return parts;
    }

    /// <summary>The width of the characters between two indices.</summary>
    public Length WidthBetween(int start, int end)
        => Length.FromEmu(At(end) - At(start));

    /// <summary>The whole paragraph's width.</summary>
    public Length Width => Length.FromEmu(_prefixEmu[^1]);

    /// <summary>
    /// The natural line height and ascent for a range of the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The maxima across the runs the range touches, taken three ways rather than one: the tallest run's
    /// height, the largest ascent, and the largest descent, with the height then raised to hold the ascent
    /// and descent if together they exceed it. That is <c>SwLineLayout::CalcLine</c>
    /// (<c>sw/source/core/text/porlay.cxx</c>), which accumulates <c>Height()</c>, <c>mnAscent</c> and
    /// <c>nMaxDescent</c> independently and finishes with
    /// <c>if (nMaxDescent &gt; Height() - mnAscent) Height(nMaxDescent + mnAscent)</c>.
    /// </para>
    /// <para>
    /// Taking the tallest run's ascent instead would agree whenever one run is tallest in every direction,
    /// which is the case for two sizes of one family — and disagree the moment a line mixes families,
    /// since a face with a deep descent can own the descent while a different face owns the ascent.
    /// </para>
    /// <para>
    /// An empty range takes the run that would contain it, so an empty line is still as tall as the text
    /// that would go on it.
    /// </para>
    /// <para>
    /// <strong>Tabs and blanks are transparent when the paragraph was measured that way.</strong> Word
    /// does not let a tab or a run of spaces make a line taller, and Writer follows it behind the
    /// <c>IgnoreTabsAndBlanksForLineCalculation</c> setting, which its DOC and DOCX importers both turn on
    /// and its RTF and ODF ones leave off (measured: a flat-ODF export of the same prose reads
    /// <c>true</c> from <c>.doc</c> and <c>.docx</c>, <c>false</c> from <c>.rtf</c>, <c>.odt</c> and
    /// <c>.fodt</c>). <c>SwLineLayout::CalcLine</c> skips such a portion outright while any other portion
    /// is on the line (<c>porlay.cxx</c>:340) and falls back to it when none is (<c>porlay.cxx</c>:601),
    /// which is the two passes below.
    /// </para>
    /// <para>
    /// It matters far more than it sounds. A tab between two runs carries whatever size the character
    /// formatting left it — usually the document default, not the size of the text either side of it — so
    /// a tabbed table set in 8 pt with 12 pt tabs is laid out on 12 pt lines by anything that measures the
    /// tabs. Measured on <c>prison-population-bulletin-june.doc</c>: 16.7 pt of row pitch against the
    /// reference's 12.1, at identical glyph sizes, which cost that document two whole pages.
    /// </para>
    /// </remarks>
    public (Length Height, Length Ascent) HeightOf(int start, int end)
    {
        Length height = Length.Zero;
        Length ascent = Length.Zero;
        Length descent = Length.Zero;

        Fold(start, end, _blanksAreTransparentToHeight, ref height, ref ascent, ref descent);

        // A line holding nothing but tabs and blanks is as tall as those tabs and blanks: they are skipped
        // only while something else is there to be measured instead.
        if (height == Length.Zero && _blanksAreTransparentToHeight)
        {
            Fold(start, end, skipBlankRuns: false, ref height, ref ascent, ref descent);
        }

        // No run at all, which happens for an empty paragraph. The first run's metrics are the
        // paragraph's own, and a paragraph with no runs has nothing to be tall for.
        if (height == Length.Zero && _runs.Length > 0)
        {
            Accumulate(_runs[0], ref height, ref ascent, ref descent);
        }

        // An as-character object divides at the baseline: the part above raises the ascent and the part
        // below raises the descent, which for the ordinary inline picture is the whole of it above and
        // nothing below. The `Max(height, ascent + descent)` below then grows the box to hold it, which is
        // the same rule `SwLineLayout::CalcLine` applies to a run taller than the line and reaches the
        // measured answer exactly: a 1 cm picture on a 12 pt Liberation Serif line gives ascent 28.35 and
        // height 31.46, where LibreOffice's own PDF of `picture-anchor.fodt` puts the second line's
        // baseline 31.46 pt below the first's.
        foreach (InlineObject one in _objects)
        {
            bool within = start <= one.Offset && one.Offset < end;

            // A paragraph whose whole content is a picture is an empty paragraph with an object at nought
            // and a line running nought to nought — the most ordinary way for a document to carry a logo,
            // and an RTF one always looks like this because RTF appends no character for a picture.
            if (!within && !(start == end && one.Offset == start)) continue;

            ascent = Length.Max(ascent, one.AboveBaseline);
            descent = Length.Max(descent, one.BelowBaseline);
        }

        return (Length.Max(height, ascent + descent), ascent);
    }

    /// <summary>Folds every run touching a range into the maxima a line's height is built from.</summary>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the line's visible text ends.</param>
    /// <param name="skipBlankRuns">
    /// True to pass over a run whose share of the range is nothing but tabs and blanks, which is what
    /// <see cref="HeightOf"/> documents.
    /// </param>
    /// <param name="height">The tallest run so far.</param>
    /// <param name="ascent">The largest ascent so far.</param>
    /// <param name="descent">The largest descent so far.</param>
    private void Fold(
        int start, int end, bool skipBlankRuns,
        ref Length height, ref Length ascent, ref Length descent)
    {
        foreach (MeasuredRun run in _runs)
        {
            bool touches = run.Run.Start < end && start < run.Run.End;
            bool contains = start == end && run.Run.Covers(start);
            if (!touches && !contains) continue;

            if (skipBlankRuns
                && IsAllBlanks(Text, Math.Max(start, run.Run.Start), Math.Min(end, run.Run.End)))
            {
                continue;
            }

            Accumulate(run, ref height, ref ascent, ref descent);
        }
    }

    /// <summary>
    /// Whether a stretch of the text is nothing but the characters Word measures no line height from.
    /// </summary>
    /// <remarks>
    /// The tab first, which Writer answers separately because a tab is its own portion type
    /// (<c>SwLinePortion::InTabGrp</c>), then the four spaces <c>lcl_HasOnlyBlanks</c> lists
    /// (<c>porlay.cxx</c>:231) plus the en space it accepts outside a fieldmark. An empty stretch is
    /// <em>not</em> all blanks: that is the empty-line case, which takes its height from the run covering
    /// it rather than losing it.
    /// </remarks>
    private static bool IsAllBlanks(string text, int start, int end)
    {
        if (end <= start) return false;

        for (int i = start; i < end && i < text.Length; i++)
        {
            switch (text[i])
            {
                case '\t':
                case ' ': // SPACE
                case '\u2002': // EN SPACE
                case '\u2003': // EM SPACE
                case '\u2005': // FOUR-PER-EM SPACE
                case '\u3000': // IDEOGRAPHIC SPACE
                    continue;
                default:
                    return false;
            }
        }

        return true;
    }

    /// <summary>Folds one run into the running maxima a line's height is built from.</summary>
    private static void Accumulate(
        MeasuredRun run, ref Length height, ref Length ascent, ref Length descent)
    {
        // Twips throughout, because Writer lays out in whole twips and a fraction kept here would
        // eventually move a line onto a different page.
        Length runHeight = Length.FromTwips(run.Metrics.ScaledLineHeight(run.Run.LineEmSize).Twips);
        Length runAscent = Length.FromTwips(run.Metrics.ScaledAscent(run.Run.LineEmSize).Twips);

        height = Length.Max(height, runHeight);
        ascent = Length.Max(ascent, runAscent);
        descent = Length.Max(descent, runHeight - runAscent);
    }

    /// <summary>
    /// The runs overlapping a range, clipped to it.
    /// </summary>
    /// <remarks>
    /// What drawing a line needs: one glyph run per formatting change on the line, each with the
    /// characters that actually fall on it. A run spanning a line break is clipped at the break, which is
    /// why this returns ranges rather than the runs themselves.
    /// </remarks>
    public List<FormattedRun> RunsBetween(int start, int end)
    {
        List<FormattedRun> clipped = [];

        foreach (MeasuredRun run in _runs)
        {
            int from = Math.Max(run.Run.Start, start);
            int to = Math.Min(run.Run.End, end);
            if (to <= from) continue;

            clipped.Add(run.Run with { Start = from, Length = to - from });
        }

        return clipped;
    }

    private long At(int index) => _prefixEmu[Math.Clamp(index, 0, _prefixEmu.Length - 1)];

    /// <summary>
    /// Sorts the runs, clips them to the text, and fills the gaps between them.
    /// </summary>
    /// <remarks>
    /// A document can state runs out of order, overlapping, or past the end of the text it is describing.
    /// Each is repaired rather than rejected: an overlap is resolved in favour of the earlier run, since
    /// something has to win and the first is the one the document mentioned first, and a gap is filled
    /// from the run before it because a character with no formatting of its own inherits what preceded it.
    /// </remarks>
    private static List<FormattedRun> Normalise(string text, IReadOnlyList<FormattedRun> runs)
    {
        List<FormattedRun> sorted = [.. runs
            .Where(run => run.Length > 0 && run.Start < text.Length)
            .OrderBy(run => run.Start)];

        List<FormattedRun> result = [];
        int at = 0;

        foreach (FormattedRun run in sorted)
        {
            int start = Math.Max(run.Start, at);
            int end = Math.Min(run.End, text.Length);
            if (end <= start) continue;

            // A gap before this run belongs to whatever came before it, or to this run when nothing did.
            if (start > at)
            {
                FormattedRun filler = result.Count > 0 ? result[^1] : run;
                result.Add(filler with { Start = at, Length = start - at });
            }

            result.Add(run with { Start = start, Length = end - start });
            at = end;
        }

        if (at < text.Length && result.Count > 0)
        {
            result.Add(result[^1] with { Start = at, Length = text.Length - at });
        }

        return result;
    }
}
