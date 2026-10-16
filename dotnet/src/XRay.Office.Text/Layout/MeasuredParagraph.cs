using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Shaping;

namespace XRay.Office.Text.Layout;

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
/// <param name="WidthPerCent">
/// How wide the glyphs are drawn against their own design, 100 for a face drawn as it is drawn.
/// <para>
/// <c>w:rPr/w:w</c> and RTF's <c>\charscalex</c>, which VCL applies by setting the font's width away
/// from its height (<c>Font::SetAverageFontWidth</c>). It is not tracking — that puts a gap between
/// glyphs and leaves them their own shape — and it is not a size, which would change the line's height.
/// </para>
/// <para>
/// <strong>The width it lands on is an integer number of twips, and that is measurable rather than
/// pedantic.</strong> VCL's font width is a <c>tools::Long</c> in the map mode's own unit, which for
/// Writer is twips, so a 12 pt run at 99 per cent is set at <c>trunc(240 × 99 / 100) = 237</c> twips and
/// scales by 237/240 = <b>0.9875</b>, not 0.99. Measured against 24.2.7.2 on
/// <c>dotnet/probes/words-character-scale/</c>: <c>Hamburgefonstiv 12345</c> at 12 pt comes to 83.928 pt
/// unscaled and 82.879 at <c>w:w="99"</c>, and 82.879/83.928 is 0.98750 to five places. Every other
/// value in the corpus divides 240 exactly, so this shows on the commonest one alone — which is
/// 1226 of the corpus's 1440 scaled runs.
/// </para>
/// </param>
/// <param name="Item">
/// The font item this run is set from, or default when the reader distinguishes none. It carries
/// the family class and the language the glyph-fallback pattern is built with; see
/// <see cref="Fonts.FontItem"/>.
/// </param>
public readonly record struct FormattedRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    ShapingOptions Shaping = default,
    Length MetricEmSize = default,
    Length Tracking = default,
    int WidthPerCent = 100,
    Fonts.FontItem Item = default)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>
    /// <see cref="WidthPerCent"/> as the factor the run's advances are actually multiplied by.
    /// </summary>
    /// <remarks>
    /// One for an unscaled run, so the ordinary path multiplies by nothing. See the parameter for why
    /// this is not simply the percentage.
    /// </remarks>
    public double WidthScale => TextWidthScale.Of(EmSize, WidthPerCent);

    /// <summary>
    /// The shaping this run is actually shaped with, once its tracking has had its say.
    /// </summary>
    /// <remarks>
    /// Every path that shapes a run has to use this rather than <see cref="Shaping"/>, because
    /// measurement and drawing disagreeing about a ligature is worse than either answer: the line
    /// would be laid out at one width and painted at another. See
    /// <see cref="ShapingOptions.WithTracking"/> for the rule and for what it cost when it was
    /// missing.
    /// </remarks>
    public ShapingOptions EffectiveShaping => Shaping.WithTracking(Tracking);

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
/// <param name="RaisesTextHeight">
/// <para>
/// Whether this object takes part in the height proportional line spacing multiplies. False for a real
/// as-character object, which is the whole point of round 45's rule; <strong>true for a list label</strong>,
/// which is not an object at all in Writer but a <c>SwNumberPortion</c> standing in the line.
/// </para>
/// <para>
/// Writer draws the line exactly here. <c>SwTextFrame::CalcHeightOfLastLine</c>
/// (<c>sw/source/core/text/txtfrm.cxx</c>:3952-3957) asks the line for
/// <c>MaxAscentDescent(…, bNoFlyCnt = true)</c> — <em>"i#47162 — suppress consideration of fly content
/// portions and the line portion"</em> — and <c>SwTextFrame::GetLineSpace</c> takes
/// <c>(prop − 100)%</c> of that. A fly-in-content is suppressed; a number portion is not.
/// </para>
/// <para>
/// Measured, because the citation is only the hypothesis:
/// <c>dotnet/probes/words-r47/list-label-line-height.py</c> puts a 14, 20 and 28 pt level over 12 pt
/// text at 100, 150 and 200% and LibreOffice's extension is <c>(prop − 100)%</c> of the <em>label's</em>
/// box every time, against the item's text height before this flag existed.
/// <c>label-and-picture.py</c> then puts a 100 pt picture on the same line and the extension stays at
/// the label's 32.20 pt rather than the line's 114 — so the base is the tallest portion that is not a
/// fly, and the two rules are one rule.
/// </para>
/// </param>
public readonly record struct InlineObject(
    int Offset, Length Width, Length Height, Length? Ascent = null, bool RaisesTextHeight = false)
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

    /// <summary>
    /// The width of the as-character objects standing at the very end of the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// An object is a <em>boundary</em> in the prefix table: it widens every prefix past the position it
    /// occupies and none at or before it, which is what makes it sit between two characters rather than
    /// replace one, and what lets a picture too wide for the room left on a line move to the next line
    /// whole. The last boundary has no prefix past it, so an object there is worth nothing to every
    /// width the table answers — and that is the commonest inline picture there is: a paragraph whose
    /// entire content is a logo.
    /// </para>
    /// <para>
    /// It is invisible in a DOCX and unmissable in every other format, because the readers disagree
    /// about whether a picture is a character. OOXML puts a <c>U+0001</c> where one stands, so the
    /// object sits at boundary 0 of a one-character paragraph and the table carries it; ODF, RTF and —
    /// for a paragraph holding nothing else — WW8 put nothing there, so the object sits at boundary 0
    /// of an empty paragraph, which <em>is</em> the last boundary. Measured on one right-aligned
    /// picture-only paragraph written five ways by LibreOffice 26.2.4.2, the picture's left edge:
    /// 394.55 in all five references, and ours 394.50 from the <c>.docx</c> against <b>540.00</b> from
    /// the <c>.odt</c>, the <c>.doc</c>, the <c>.rtf</c> and the <c>.fodt</c> — the whole picture's
    /// width out, drawn from the right margin rightwards. See <c>probes/words-aschar-band/</c>.
    /// </para>
    /// <para>
    /// Writer has no such gap because it has no prefix table: an as-character fly is a
    /// <c>SwFlyCntPortion</c> in the line and its width is the line's like any other portion's
    /// (<c>sw/source/core/text/txtfly.cxx</c>). This is the one place the boundary model needs telling
    /// that there is no next line.
    /// </para>
    /// </remarks>
    private readonly long _trailingObjectsEmu;

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

        long trailing = 0;
        foreach (InlineObject one in objects)
        {
            if (one.Offset >= text.Length) trailing += one.Width.Emu;
        }

        _trailingObjectsEmu = trailing;
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
    /// <param name="leadingAboveText">
    /// Whether each run's external leading is charged to its ascent. True only for Writer's own text;
    /// see <see cref="LineMetrics.ScaledAscent"/>.
    /// </param>
    /// <param name="addsScriptSpace">
    /// Whether a script change with East Asian text on one side widens the text by a fifth of the em.
    /// True for a document that came from Word; see <see cref="ScriptSpacing"/>.
    /// </param>
    public static MeasuredParagraph Measure(
        string text,
        IReadOnlyList<FormattedRun> runs,
        ITextShaper? shaper = null,
        ItemisationOptions? itemisation = null,
        IReadOnlyList<InlineObject>? objects = null,
        MetricGrid? grid = null,
        bool blanksAreTransparentToHeight = false,
        bool leadingAboveText = false,
        bool addsScriptSpace = false)
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
            bool itemised = false;

            foreach (FormattedRun part in SubRuns(text, run, items, options))
            {
                itemised = true;

                ShapedText shaped = engine.Shape(
                    part.Face, text.AsSpan(part.Start, part.Length), part.EffectiveShaping);

                measured.Add(new MeasuredRun(
                    part, shaped, LineSpacing.Resolve(part.Face, grid, leadingAboveText)));

                // Each sub-run's own prefix widths, scaled from its own grid into EMUs and added to
                // the running total. Summing in design units instead would add numbers from two
                // different grids; reading the running total off the table instead would break the
                // moment a control character left a gap between two sub-runs.
                double squeeze = part.WidthScale;

                for (int i = 1; i <= part.Length; i++)
                {
                    prefix[part.Start + i] = running
                        + Scaled(Advance(shaped, shaped.AdvanceUpTo(i), part.EmSize, grid), squeeze);
                }

                if (part.Tracking != Length.Zero)
                {
                    tracked = true;
                    for (int i = 0; i < part.Length; i++) tracking[part.Start + i] = part.Tracking.Emu;
                }

                running += Scaled(Advance(shaped, shaped.AdvanceInDesignUnits, part.EmSize, grid), squeeze);
            }

            // A run every one of whose characters is a format control makes no sub-run at all, and
            // until round 151 it then vanished from the measurement entirely. The commonest such run
            // is a MANUAL LINE BREAK on its own: U+2028 is what all four word-processing readers and
            // both DrawingML readers emit for one, and `TextItemiser.IsFormatControl` cuts it out of
            // every sub-run.
            //
            // The paragraph whose *whole* text is breaks already had a repair -- the `measured.Count
            // == 0` fallback below -- but a break run beside ordinary text did not, and that is the
            // ordinary case: `<w:r><w:br/></w:r>` at one size ahead of a run at another. `MeasureLine`
            // then found no run covering the break-only line, fell through to its last resort, and
            // took `_runs[0]`'s metrics -- which is the *neighbouring* run. So the line came out the
            // height of whatever text was beside it instead of the height of the break's own face.
            //
            // [bin] Measured on `probes/brline-r149/fixtures3`, thirteen arms varying only the break
            // run's `w:sz` from 1 pt to 40 pt: 26.2.4.2 draws 1.150 .. 46.550 pt on a 1.164-em slope
            // and this tree drew a flat 12.800 at every one of them, agreeing only at `b22`, which is
            // 11 pt -- the paragraph's own size. `probes/brline-r151/discriminate.txt`.
            //
            // Kept with its own range rather than as a zero-length marker, because `Fold` matches on
            // `touches` or `contains` and a zero-length run satisfies neither. It carries
            // `ShapedText.Empty`, so it adds no advance, draws nothing, and the prefix table's
            // monotonic fix-up below covers the positions it leaves uncovered -- exactly as it did
            // when the run was absent.
            if (!itemised && run.Length > 0)
            {
                measured.Add(new MeasuredRun(
                    run, ShapedText.Empty, LineSpacing.Resolve(run.Face, grid, leadingAboveText)));
            }
        }

        // Every run itemised away, which happens when the paragraph's whole text is format control
        // characters — a paragraph whose entire content is manual line breaks, since U+2028 is what all
        // four word-processing readers and both DrawingML readers emit for one and
        // `TextItemiser.IsFormatControl` cuts it out of every sub-run. The comment on that predicate
        // states the consequence ("a paragraph that is nothing but control characters then has no run
        // and no line") and moved the C0 range out to avoid it; this is the other half of the same
        // repair, for the characters that must stay cut.
        //
        // Without it `MeasureLine` has nothing to be tall for: its fold finds no run, its
        // blanks-refold finds no run, and its last resort — the empty-paragraph rule, which takes the
        // first run's metrics — is guarded on there being one. Every line comes out nought points, so
        // a paragraph holding one break contributes **nothing** where the reference gives it two
        // lines. Measured on `probes/words-r52/br-paragraph-probe.py` against 26.2.4.2: a paragraph
        // whose whole content is one `w:br` adds 25.30 pt there and added 0.00 here, while the same
        // paragraph with a single space in front of the break added 23.00 — the space is not a
        // control, so it left a run behind and the fallback fired.
        //
        // Zero length, so it is the fallback and nothing else that can see it: `Fold` matches on
        // `touches` or `contains` and both are false for an empty range, `RunsBetween` clips it away,
        // and the prefix table is not touched. The face and size are the first formatted run's, which
        // for a paragraph the reader found no formatting for is the paragraph's own — see
        // `PageParagraph.Measure`, which supplies exactly that.
        if (measured.Count == 0 && formatted.Count > 0)
        {
            measured.Add(new MeasuredRun(
                formatted[0] with { Start = 0, Length = 0 },
                ShapedText.Empty,
                LineSpacing.Resolve(formatted[0].Face, grid, leadingAboveText)));
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

        // Writer's fifth of an em where the script changes with East Asian text on one side. Added to
        // the prefix table the way an object is, and for the same reason: it is a width between two
        // characters rather than a property of either, and every reader of the table — the line
        // filler, the tab ruler, the as-character anchor — has to see it.
        //
        // The size is the run's that *ends* at the boundary, which is the font
        // `SwTextFormatter::BuildPortions` has in hand when it closes the portion.
        if (addsScriptSpace)
        {
            foreach (int boundary in ScriptSpacing.Boundaries(text))
            {
                Length gap = ScriptSpacing.GapFor(SizeAt(formatted, boundary - 1));
                if (gap <= Length.Zero) continue;

                for (int i = boundary; i <= text.Length; i++) prefix[i] += gap.Emu;
            }
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
    /// <para>
    /// <strong>The faces are chosen over the whole run and the items are applied to the result, not
    /// the other way about.</strong> Glyph fallback asks for one face covering <em>all</em> of a
    /// run's missing characters at once — see <see cref="FontItemiser.Split"/> — so the answer
    /// depends on which characters were in the request, and the drawing pass
    /// (<c>PageDrawing.ByFace</c>) asks per run. Splitting by item first would ask a different
    /// question here and let measurement and drawing pick different faces for the same character,
    /// which is a line laid out at one width and painted at another.
    /// </para>
    /// </remarks>
    private static List<FormattedRun> SubRuns(
        string text, FormattedRun run, List<TextItem> items, ItemisationOptions options)
    {
        List<FormattedRun> parts = [];

        List<FaceRun> faces = FontItemiser.Split(
            text, run.Start, run.Length, run.Face,
            options.GlyphFallback, options.OnGlyphFallback, run.Item);

        foreach (TextItem item in items)
        {
            int from = Math.Max(item.Start, run.Start);
            int to = Math.Min(item.End, run.End);
            if (to <= from) continue;

            foreach (FaceRun face in faces)
            {
                int start = Math.Max(face.Start, from);
                int end = Math.Min(face.End, to);
                if (end <= start) continue;

                bool wholeRun = start == run.Start && end == run.End;
                bool plain = wholeRun && !item.IsRightToLeft && !face.IsFallback;

                parts.Add(run with
                {
                    Start = start,
                    Length = end - start,
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

    /// <summary>The em size in force at a character position, or the first run's.</summary>
    /// <remarks>
    /// A linear walk because a paragraph has a handful of runs and this is asked once per script
    /// boundary. Falls back to the first run rather than to zero, so a boundary in a gap no run
    /// covers — which a control character cut out of the runs can leave — still opens its gap.
    /// </remarks>
    private static Length SizeAt(List<FormattedRun> runs, int index)
    {
        foreach (FormattedRun run in runs)
        {
            if (run.Covers(index)) return run.EmSize;
        }

        return runs.Count > 0 ? runs[0].EmSize : Length.Zero;
    }

    /// <summary>
    /// A sub-run's advance width, through the device grid when the document asks for one.
    /// </summary>
    /// <remarks>
    /// Applied to each *cumulative* width rather than to each glyph, because that is what the device
    /// does — see <see cref="MetricGrid.ToAdvance"/>. A sub-run is the nearest thing this measurement
    /// has to Writer's text portion, which is the unit the reference device's width is truncated over.
    /// The truncation is monotonic, so the prefix table stays monotonic, which every reader of it
    /// depends on.
    /// </remarks>
    /// <summary>An advance squeezed by the run's character width, or left alone when it states none.</summary>
    /// <remarks>
    /// The multiply is guarded on the ordinary case rather than applied always, because every run in
    /// the corpus but 1440 of them is unscaled and a double round trip on each of their advances would
    /// move the last twip of some of them for nothing. See <see cref="TextWidthScale"/>.
    /// </remarks>
    private static long Scaled(long advance, double squeeze)
        => squeeze == 1.0 ? advance : (long)Math.Round(advance * squeeze);

    private static long Advance(ShapedText shaped, long designUnits, Length emSize, MetricGrid? grid)
        => grid is { QuantisesAdvances: true } device
            ? device.ToAdvance(designUnits, shaped.UnitsPerEm, emSize).Emu
            : shaped.Scale(designUnits, emSize).Emu;

    /// <summary>The width of the characters between two indices.</summary>
    /// <remarks>
    /// A range reaching the end of the text also carries the objects standing at that last boundary —
    /// see <see cref="_trailingObjectsEmu"/>. A range that <em>starts</em> there does not, so the empty
    /// line a trailing manual break opens does not pay for the picture on the line above it.
    /// </remarks>
    public Length WidthBetween(int start, int end)
        => Length.FromEmu(At(end) - At(start) + (HoldsTrailingObjects(start, end) ? _trailingObjectsEmu : 0));

    /// <summary>Whether a range carries the objects standing at the text's last boundary.</summary>
    private bool HoldsTrailingObjects(int start, int end)
        => _trailingObjectsEmu != 0 && end >= Text.Length && (start < Text.Length || Text.Length == 0);

    /// <summary>The whole paragraph's width.</summary>
    public Length Width => Length.FromEmu(_prefixEmu[^1] + _trailingObjectsEmu);

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
        (Length height, Length ascent, _) = MeasureLine(start, end);
        return (height, ascent);
    }

    /// <summary>
    /// The same measurement, with the height the line's <em>text</em> alone wants beside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two differ only where an as-character object is taller than the text around it, and the
    /// difference is what proportional line spacing multiplies. Writer keeps them apart deliberately:
    /// <c>SwLineLayout::Height(nNew, bText)</c> (<c>sw/source/core/text/porlay.cxx</c>:110) records
    /// <c>m_nLineSpacingBaseHeight</c> only when the portion that raised the line
    /// <c>IsUsedToCalcLineSpacingHeight</c>, which for a legacy document is true for text portions and
    /// false for a fly-in-content — so an inline picture raises <c>Height()</c> and leaves the base
    /// alone. <c>SwTextFormatter::CalcRealHeight</c> then adds
    /// <c>(prop − 100)% × GetLineSpacingBaseHeight()</c> to the full line height rather than scaling it.
    /// </para>
    /// <para>
    /// Zero when the range holds no run at all — a paragraph whose whole content is a picture. Writer
    /// falls back to the paragraph's own font there (<c>porlay.cxx</c>:645,
    /// <c>SetLineSpacingBaseHeight(rInf.GetTextHeight())</c>), which the caller supplies because it is
    /// the one that knows the paragraph's face and size.
    /// </para>
    /// </remarks>
    public (Length Height, Length Ascent, Length TextHeight) MeasureLine(int start, int end)
    {
        Length height = Length.Zero;
        Length ascent = Length.Zero;
        Length descent = Length.Zero;

        Fold(start, end, skipTransparentRuns: true, ref height, ref ascent, ref descent);

        // A line holding nothing but tabs and blanks is as tall as those tabs and blanks: they are skipped
        // only while something else is there to be measured instead.
        if (height == Length.Zero && _blanksAreTransparentToHeight)
        {
            Fold(start, end, skipTransparentRuns: false, ref height, ref ascent, ref descent);
        }

        // No run at all, which happens for an empty paragraph. The first run's metrics are the
        // paragraph's own, and a paragraph with no runs has nothing to be tall for.
        if (height == Length.Zero && _runs.Length > 0)
        {
            Accumulate(_runs[0], ref height, ref ascent, ref descent);
        }

        // Held before the objects have their say: this is what proportional line spacing scales.
        Length textHeight = Length.Max(height, ascent + descent);

        // The same two maxima again, kept apart from the line's own so that a fly-in-content can raise
        // the line without raising this. A label joins these; a picture does not. See the fold below.
        Length textAscent = ascent;
        Length textDescent = descent;

        // A blank or tab run is transparent to the line's *height* and opaque to the height
        // proportional line spacing takes its percentage of. The two exclusions are separate rules in
        // Writer and they do not have the same membership: the blanks-and-tabs rule decides
        // `SwLineLayout::Height`, and `m_nLineSpacingBaseHeight` is a second maximum with its own test.
        //
        // Measured against the installed 26.2.4.2 by `probes/words-w-pitch/mk.py`, six paragraphs per
        // group of a 10 pt Arial style at `w:line="288" w:lineRule="auto"` with `w:contextualSpacing`,
        // so the pitch is the line height plus the proportional gap and nothing else. A run holding one
        // tab, or one space, in a face and size of its own:
        //
        //   | the blank run       | line height | LibreOffice's pitch | base it implies |
        //   |---------------------|------------:|--------------------:|----------------:|
        //   | none                |       11.50 |               13.80 |           11.50 |
        //   | Calibri 11 pt tab   |       11.50 |               14.15 |           13.43 |
        //   | Calibri 11 pt space |       11.50 |               14.15 |           13.43 |
        //   | Calibri 22 pt tab   |       11.50 |               16.85 |           26.86 |
        //   | Arial 20 pt tab     |       11.50 |               16.10 |           23.00 |
        //
        // Every implied base is the blank run's own line height to the twip, and the line height never
        // moves — so the blank run is in one maximum and out of the other. Scaling the line instead, or
        // leaving the blank run out of both, gives 13.80 for all five.
        //
        // This is what a contents entry costs: `OM template for non-complex NCC operators` sets each
        // `TOC4` line with a `minorHAnsi` 11 pt run holding the tab between the number and the title,
        // and drawing those at 13.80 rather than 14.15 fitted 83 entries where the reference fits 79.
        if (_blanksAreTransparentToHeight)
        {
            Length blankHeight = Length.Zero;
            Length blankAscent = Length.Zero;
            Length blankDescent = Length.Zero;

            Fold(start, end, skipTransparentRuns: false, ref blankHeight, ref blankAscent, ref blankDescent);

            textHeight = Length.Max(
                textHeight, Length.Max(blankHeight, blankAscent + blankDescent));
        }

        // A line holding no text has nothing to hang below its baseline. Writer builds a line's descent
        // out of the portions that carry text and a fly-in-content is not one of them, so a picture
        // alone on its line makes a line exactly as tall as the picture — floored at the paragraph
        // font's own line height, which is why the run's *height* above is still wanted and only its
        // descent is dropped.
        //
        // Measured against the installed 26.2.4.2 by `dotnet/probes/words-r46/picture-alone-descent.py`
        // — **re-checked 2026-08-21 under `TODO.24-2-7-audit.md` and found unchanged**. All eight
        // DOCX rows and all four `fodt alone` rows come back exact, and the reference's own figures
        // [24.2.7-audit: VERIFIED 2026-08-21, round words-r53 — unchanged on 26.2.4.2.]
        // are identical to the ones round 46 read off 24.2.7.2, to the tenth of a point, at 20, 50
        // and 150 pt. The four `fodt with-text` rows still come back 27.60 at every picture height:
        // that is round 46's own measured-and-deliberately-unfixed ODF defect (an as-character
        // object at the very end of a flat-ODF paragraph contributes no height), not a movement in
        // the reference. As round 46 originally read it off 24.2.7.2:
        // sixteen rows over two formats, two shapes and four picture heights. A DOCX picture alone on
        // its line at 20, 50 and 150 pt gives a baseline-to-baseline gap of h + 13.8 with a 12 pt
        // Liberation Serif paragraph, and h + 13.8 + 2.6 with any text beside the picture — 2.6 pt
        // being that font's descent to the tenth. At 5 pt both come back 13.8 + 13.8, the floor.
        //
        // This closes round 45's open item, which could not act because <see cref="HeightOf"/> cites an
        // ODF fixture where LibreOffice *does* add that descent. It does, and it is not a format
        // difference: `picture-anchor.fodt` has text on the picture's line — "An inline picture
        // follows: … and that was it" — so it is the with-text row. The probe authors both shapes in
        // both formats to settle exactly that, and LibreOffice's fodt and docx agree to 0.00 pt in all
        // eight pairs. Nothing here is conditioned on the format.
        //
        // Keyed on the line's *characters*, not on the run being empty, because the readers disagree
        // about whether an inline picture puts a character in the text at all: DOCX and DOC emit an
        // anchor character for one and RTF and ODF emit nothing, so the same paragraph reaches here as
        // `start == end` from one reader and as a one-character line from another. Both are lines whose
        // whole content is the object. A U+0001 standing for something that is *not* an as-character
        // object — a field result, a note reference — is deliberately not covered: that mark is drawn,
        // and its descent is real.
        //
        // And the ascent goes with it, for the same reason and by the same test. The anchor character
        // *is* the object in Writer — `SwTextFormatter::NewFlyCntPortion` replaces it with a
        // `SwFlyCntPortion` rather than shaping it beside one — so a line holding nothing but anchors
        // has no text portion to be tall for, and `SwLineLayout::CalcLine` builds its ascent out of the
        // fly portion alone.
        //
        // It shows only where an object's own ascent is *below* the paragraph font's, and that is one
        // class: a WW8 SHAPE-field frame, whose ascent is nought because it hangs below the baseline
        // (`Ww8Frames.InlineAscent`, the only place anything sets an ascent smaller than the object).
        // Measured on `picture-furniture.doc`, whose running head is one such 79.2 pt logo: taking the
        // font's 11.2 pt ascent as well makes the head 90.4 pt, pushes the body down by that much, and
        // costs page 1 two of its lines and 21 of its 476 words against the reference.
        if (_objects.Length > 0 && HoldsNoText(start, end))
        {
            descent = Length.Zero;
            ascent = Length.Zero;
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

            // A list label is a portion rather than an object, so it counts towards the height
            // proportional line spacing takes its percentage of — see <see cref="InlineObject"/>.
            //
            // Per side, not as a box. Writer keeps a running maximum ascent and a running maximum
            // descent and makes the base out of the two, so a label that is deeper than the text but no
            // taller overall still widens the base by its extra descent. Folding its whole box in
            // instead gives the same answer for a label of the paragraph's own face at a larger size —
            // which is every row round 47 measured, and why it could not see the difference — and a
            // short answer for a label that differs by face.
            //
            // Measured against the installed 26.2.4.2 by `dotnet/probes/words-b-01/labelshape.py`, a
            // 12 pt level over a 12 pt Liberation Serif item at 200%: a Liberation Mono label (ascent
            // 9.99, descent 3.60) gives LibreOffice an extension of 14.80 = 11.20 + 3.60, and a Caladea
            // one (10.80, 3.00) gives 14.20 = 11.20 + 3.00. The box rule gives 13.59 and 13.80.
            if (one.RaisesTextHeight)
            {
                textAscent = Length.Max(textAscent, one.AboveBaseline);
                textDescent = Length.Max(textDescent, one.BelowBaseline);
            }
        }

        textHeight = Length.Max(textHeight, textAscent + textDescent);

        return (Length.Max(height, ascent + descent), ascent, textHeight);
    }

    /// <summary>True when a line carries an as-character object and no text at all.</summary>
    /// <remarks>
    /// <para>
    /// Two shapes, because the readers disagree about whether an inline picture is a character. RTF and
    /// ODF put nothing in the text for one, so the line is an empty range with an object on it; DOCX and
    /// DOC emit an anchor character, so it is a one-character line. Neither line has any text.
    /// </para>
    /// <para>
    /// The test is on the characters rather than on the object offsets, which is the distinction that
    /// matters: an <see cref="InlineObject"/>'s offset is a <em>boundary</em>, so an object at nought in
    /// the paragraph "x" sits before a real letter and that line does have text. A control character is
    /// never drawn and never has a descent, whoever put it there. Tab is excluded because it is a
    /// control by Unicode's reckoning and a measured portion by Writer's — see the remarks on
    /// <see cref="HeightOf"/>.
    /// </para>
    /// </remarks>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where its visible text ends.</param>
    private bool HoldsNoText(int start, int end)
    {
        int last = Math.Min(end, Text.Length);

        for (int at = Math.Max(start, 0); at < last; at++)
        {
            if (Text[at] >= ' ' || Text[at] == '\t') return false;
        }

        return Array.Exists(_objects, one => start <= one.Offset && one.Offset <= end);
    }

    /// <summary>Folds every run touching a range into the maxima a line's height is built from.</summary>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the line's visible text ends.</param>
    /// <param name="skipTransparentRuns">
    /// True to pass over a run that contributes no height: one whose share of the range is nothing but
    /// tabs and blanks, which is what <see cref="HeightOf"/> documents, or one whose share is nothing
    /// but frame anchors, which is <see cref="IsAllAnchors"/>.
    /// </param>
    /// <param name="height">The tallest run so far.</param>
    /// <param name="ascent">The largest ascent so far.</param>
    /// <param name="descent">The largest descent so far.</param>
    private void Fold(
        int start, int end, bool skipTransparentRuns,
        ref Length height, ref Length ascent, ref Length descent)
    {
        foreach (MeasuredRun run in _runs)
        {
            bool touches = run.Run.Start < end && start < run.Run.End;
            bool contains = start == end && run.Run.Covers(start);
            if (!touches && !contains) continue;

            int from = Math.Max(start, run.Run.Start);
            int to = Math.Min(end, run.Run.End);

            // The anchor rule is not gated on the caller's flag the way the blanks rule is: #i3952 is a
            // Word compatibility setting that the RTF and ODF filters leave off, while a run holding no
            // text making no text portion is Writer's line model itself and is the same in every format.
            if (skipTransparentRuns
                && (IsAllAnchors(Text, from, to)
                    || (_blanksAreTransparentToHeight && IsAllBlanks(Text, from, to))))
            {
                continue;
            }

            Accumulate(run, ref height, ref ascent, ref descent);
        }
    }

    /// <summary>
    /// Whether a stretch of the text is nothing but the anchors of things that are not text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// U+0001 is what every word-processing reader puts where a floating frame, an as-character picture
    /// or a comment mark sits. Writer builds a line out of portions and neither of the three is a text
    /// portion — an as-character fly is a <c>SwFlyCntPortion</c> carrying its own height, an
    /// at-character fly is not in the line at all, and a comment mark is a mark. So the run's font
    /// never reaches <c>SwLineLayout::Height</c>, and a document that sets its logo run at 26 pt
    /// because the heading beside it was does not thereby make the line 26 pt tall.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2 on ten authored variants of one real paragraph
    /// (<c>dotnet/probes/words-r53/</c>): the height a paragraph holding one drawing adds is
    /// <b>the same at 10 pt and at 26 pt</b> — 0.00 anchored, 7.00 as-character, 9.70 as-character with
    /// text beside it — while ours was the run's own size in every row where that size was large. It is
    /// worth 34 pt on <c>097_Business_Case_Template_Elegant_Layout</c> and nothing at all on a document
    /// whose picture run happens to be set in the size of the text around it, which is most of them.
    /// </para>
    /// <para>
    /// Skipped only while something else is on the line, exactly as a blank run is: a line holding
    /// nothing but an anchor falls back through the same refold, where the run is met again — by then
    /// carrying the paragraph's own face and size, which the word-processing layer substitutes for a
    /// run that holds nothing but anchors. That is the reference's answer for the alone case, 0.00 over
    /// an empty paragraph at both sizes.
    /// </para>
    /// <para>
    /// An empty stretch is not all anchors, for the reason <see cref="IsAllBlanks"/> gives: that is the
    /// empty-line case and it must keep the run covering it.
    /// </para>
    /// </remarks>
    private static bool IsAllAnchors(string text, int start, int end)
    {
        if (end <= start) return false;

        for (int at = start; at < end && at < text.Length; at++)
        {
            if (text[at] != '\u0001') return false;
        }

        return true;
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
