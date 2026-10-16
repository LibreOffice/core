using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbering;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.Presentations.Layout;
using XRay.Office.Vector;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// Turns a binary PowerPoint text run into the text body the slide layouter lays out.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <c>PptxTextBody</c> and <c>OdfTextBody</c>, and the place the master style
/// sheet finally earns its keep: a PPT paragraph states only what differs from its outline level,
/// so its size, colour, typeface, indents, alignment and bullet are all resolved here, property by
/// property, against <see cref="PptStyleSheet"/>.
/// </para>
/// <para>
/// <strong>A property mask of zero is not a property of zero.</strong> Every field below is
/// carried on the run beside the mask bit that says whether the run stated it, because a
/// paragraph that says nothing about its alignment is left-aligned only if its master is —
/// reading the field regardless would left-align every inherited paragraph in the deck and,
/// worse, would move the text of every outline paragraph to the shape's edge by zeroing an
/// indent the master states.
/// </para>
/// </remarks>
internal static class PptTextBody
{
    /// <summary>The mask bit a paragraph sets when it states its own alignment.</summary>
    private const uint StatesAlignment = 0x0000_0800;

    /// <summary>The mask bit for the line feed.</summary>
    private const uint StatesLineFeed = 0x0000_1000;

    /// <summary>The mask bits for the space above and below.</summary>
    private const uint StatesSpaceBefore = 0x0000_2000;
    private const uint StatesSpaceAfter = 0x0000_4000;

    /// <summary>The mask bits for the two indents.</summary>
    private const uint StatesTextOffset = 0x0000_0100;
    private const uint StatesBulletOffset = 0x0000_0400;

    /// <summary>The mask bit for the bullet's presence, <c>PPT_ParaAttr_BulletOn</c>.</summary>
    private const uint StatesBulletOn = 0x0000_0001;

    /// <summary>The mask bit for the bullet's character, <c>PPT_ParaAttr_BulletChar</c>.</summary>
    private const uint StatesBulletChar = 0x0000_0080;

    /// <summary>The mask bits for the bullet's own face, size and colour.</summary>
    private const uint StatesBulletFont = 0x0000_0010;
    private const uint StatesBulletHeight = 0x0000_0040;
    private const uint StatesBulletColour = 0x0000_0020;

    /// <summary>
    /// The mask bit for <c>PPT_ParaAttr_BuHardColor</c>, which says whether the paragraph itself
    /// decided that its bullet's colour is stated rather than inherited from its text.
    /// </summary>
    private const uint StatesBulletHardColour = 0x0000_0004;

    /// <summary>
    /// The mask bit for <c>PPT_ParaAttr_BuHardFont</c>, which says whether the paragraph itself
    /// decided that its bullet's face is stated rather than inherited from its text.
    /// </summary>
    private const uint StatesBulletHardFont = 0x0000_0002;

    /// <summary>
    /// <c>PPT_ParaAttr_BuHardColor</c>'s bit within the bullet-flags word — the second, counting
    /// from <c>PPT_ParaAttr_BulletOn</c> at bit zero.
    /// </summary>
    private const ushort BulletHardColourFlag = 0x0004;

    /// <summary><c>PPT_ParaAttr_BuHardFont</c>'s bit within the same word — the first.</summary>
    private const ushort BulletHardFontFlag = 0x0002;

    /// <summary>
    /// The mask bit for <c>PPT_ParaAttr_BuHardHeight</c>, and its bit in the bullet-flags word.
    /// </summary>
    /// <remarks>
    /// A stated <c>buSize</c> only counts as the paragraph's own when <em>both</em> are set:
    /// <c>ReadParaProps</c> clears <c>mnAttrSet</c>'s <c>PPT_ParaAttr_BulletHeight</c> again unless
    /// <c>(nMask &amp; (1 &lt;&lt; PPT_ParaAttr_BuHardHeight)) &amp;&amp;
    /// (nBulFlg &amp; (1 &lt;&lt; PPT_ParaAttr_BuHardHeight))</c>
    /// (<c>svdfppt.cxx</c>:4903-4912), which is the only one of the three hard-flags that gates
    /// the mask rather than the value.
    /// </remarks>
    private const uint StatesBulletHardHeight = 0x0000_0008;

    /// <summary><c>PPT_ParaAttr_BuHardHeight</c>'s bit within the bullet-flags word.</summary>
    private const ushort BulletHardHeightFlag = 0x0008;

    /// <summary>The mask bits a character run sets for its face, size and colour.</summary>
    private const uint StatesFontIndex = 0x0001_0000;
    private const uint StatesFontHeight = 0x0002_0000;
    private const uint StatesColour = 0x0004_0000;

    /// <summary>The mask bit for a raised or lowered baseline, <c>PPT_CharAttr_Escapement</c>.</summary>
    private const uint StatesEscapement = 0x0008_0000;

    /// <summary>
    /// Builds a body, or returns null when the run holds nothing to draw.
    /// </summary>
    /// <param name="run">The text run, as the reader produced it.</param>
    /// <param name="styles">The style sheet of the master the run's page belongs to.</param>
    /// <param name="scheme">The page's colour scheme, which every colour here resolves through.</param>
    /// <param name="fonts">The document's font table, which a typeface index refers to.</param>
    /// <param name="insets">The shape's text insets.</param>
    /// <param name="anchor">Where the block sits vertically.</param>
    /// <param name="wraps">Whether lines break at the shape's width.</param>
    /// <param name="autofits">
    /// Whether the text shrinks until it fits the shape. Decided by the shape rather than by the
    /// text — see <c>PptSlideLayout.Autofits</c> for what the binary format makes that mean.
    /// </param>
    /// <param name="bulletPictures">
    /// The document's picture-bullet store, or null when nothing will draw one. A level's
    /// <c>buBlip</c> makes the bullet a bitmap only when a graphic answers for it
    /// (<c>svdfppt.cxx:3448</c>), so the store is what decides the bullet's <em>kind</em> and not
    /// only its picture.
    /// </param>
    public static SlideTextBody? Build(
        PptTextRun run,
        PptStyleSheet? styles,
        PptColourScheme scheme,
        PptFontTable fonts,
        Margins insets,
        TextAnchor anchor,
        bool wraps,
        bool autofits = false,
        PptBulletPictures? bulletPictures = null)
    {
        ArgumentNullException.ThrowIfNull(run);

        List<SlideParagraph> paragraphs = [];
        int start = 0;

        // One counter and one "inside a numbered run" flag per outline level, owned by the body
        // rather than by a paragraph -- an automatic number runs across the paragraphs of one
        // text body and restarts wherever the run is broken, which is the same contract
        // `DrawingTextBody.AutoNumber` takes for the OOXML side and the reason both readers can
        // share its arithmetic instead of numbering a nested list two different ways.
        int[] counters = new int[PptNumbering.Levels];
        bool[] counting = new bool[PptNumbering.Levels];

        while (start <= run.Text.Length)
        {
            int stop = run.Text.IndexOf(PptTextReader.ParagraphSeparator, start);
            int length = (stop < 0 ? run.Text.Length : stop) - start;

            // Where an empty paragraph looks for the character properties its blank line is
            // measured in. That is its own position for every paragraph but the phantom one a
            // trailing return leaves behind, which takes the *preceding* portion's — see the
            // remarks on Runs' `inherit`.
            int inherit = stop < 0 && start > 0 && length == 0 ? start - 1 : start;

            paragraphs.Add(
                Paragraph(
                    run, styles, scheme, fonts, start, length, inherit, counters, counting,
                    bulletPictures));

            if (stop < 0) break;
            start = stop + 1;
        }

        // The empty paragraph a trailing return leaves behind is kept, because the reference keeps
        // it. `PPTStyleTextPropReader::Init` appends one more portion, in one more paragraph, once
        // the loop over the text is done and the last portion it made belongs to the paragraph
        // before the counter (`filter/source/msfilter/svdfppt.cxx`:5403-5409) — which is exactly
        // the case where the string ended on a newline marker. Read out of 26.2.4.2's own resolved
        // view as well: `soffice --convert-to fodp` on `pres_ioc_phuket.ppt` gives the banner of
        // page 26, whose whole text is a single `\r`, **two** `<text:p>` elements and a shape
        // 3.647 cm tall — two 40 pt lines. Dropping the second drew it 38.5 pt short. A title is
        // the exception and it is handled where the text is read, not here: `PptTextReader.Broken`
        // turns a `PageTitle`'s returns into line breaks, so such a run never splits at all.
        if (paragraphs.Count == 0) return null;

        // EditEngine adds a paragraph's space above only when it is not the first, and its space
        // below only when it is not the last (ImpEditEngine::CalcHeight,
        // editeng/source/editeng/impedit2.cxx:4791-4802). Worth 0.125 pt on the corpus deck,
        // exactly one master unit — small, and the difference between "agrees with the reference"
        // and "nearly agrees".
        //
        // SlideTextLayout now applies the same rule for all three families, so this is redundant
        // rather than load-bearing. It stays because it makes the body this reader hands over say
        // what it means: the outer two spacings are not part of the text's height.
        paragraphs[0] = paragraphs[0] with { SpaceBefore = Length.Zero };
        paragraphs[^1] = paragraphs[^1] with { SpaceAfter = Length.Zero };

        return new SlideTextBody
        {
            Paragraphs = paragraphs,
            Insets = insets,
            Anchor = anchor,
            Wraps = wraps,
            AutoFit = autofits,
        };
    }

    private static SlideParagraph Paragraph(
        PptTextRun run,
        PptStyleSheet? styles,
        PptColourScheme scheme,
        PptFontTable fonts,
        int start,
        int length,
        int inherit,
        int[] counters,
        bool[] counting,
        PptBulletPictures? bulletPictures)
    {
        PptParagraphRun properties = PropertiesAt(run.Paragraphs, start);
        int depth = properties.Depth;

        PptParagraphLevel level = styles?.Paragraph(run.Kind, depth)
                                 ?? new PptParagraphLevel(0, 0x2022);
        PptCharacterLevel characters = styles?.Character(run.Kind, depth)
                                       ?? new PptCharacterLevel(0, 0, 0xFFFF, 18, 0x08000001, 0);

        // U+2028, which is what the PPTX and ODF readers already produce for a manual break, so
        // one layout rule serves all three. A newline breaks the same way — the break set accepts
        // both — but it is also what a reader may leave on the end of a paragraph's text to mean
        // the paragraph ends there, and the rule that gives a trailing break its own empty line
        // has to be able to tell those apart. Reading `\n` here left this deck's bullets a line
        // short each: `2015-Civil-Rights-Website-training.ppt` ends every paragraph in a `\x0B`.
        string text = run.Text.Substring(start, length).Replace(
            PptTextReader.LineBreak, '\u2028');

        List<SlideTextRun> runs =
            Runs(run, scheme, fonts, characters, start, length, inherit, text.Length);

        // A bullet whose colour is not hard takes the first portion's -- and a hyperlink portion
        // hands it the colour it had *before* the link recoloured it, rather than the scheme's
        // hyperlink slot: `PPTParagraphObj::GetAttrib` keeps `mnHylinkOrigColor` for exactly this
        // (`filter/source/msfilter/svdfppt.cxx:6037-6042`), falling back to the master character
        // level's own colour when the portion stated none of its own. Without it every bulleted
        // paragraph opening on a link would take a blue bullet the reference does not draw.
        Colour? linkedBullet = run.IsLinked(start)
            ? PptColour.ResolveText(UnlinkedColour(run, characters, start), scheme)
            : null;

        ushort alignment = properties.States(StatesAlignment) ? properties.Alignment : level.Alignment;
        short lineFeed = properties.States(StatesLineFeed) ? properties.LineFeed : level.LineFeed;
        short before = properties.States(StatesSpaceBefore) ? properties.SpaceBefore : level.SpaceBefore;
        short after = properties.States(StatesSpaceAfter) ? properties.SpaceAfter : level.SpaceAfter;
        // The shape's own ruler outranks the master, and is consulted only where the paragraph
        // itself said nothing. That is exactly `ReadParaProps`' tail
        // (`filter/source/msfilter/svdfppt.cxx:5062-5068`): the ruler's value is written into the
        // property set *and the mask bit is set with it*, so a level the ruler speaks for never
        // reaches the master's level at all. See `PptTextRuler` for the measurement.
        ushort textOffset = properties.States(StatesTextOffset)
            ? properties.TextOffset
            : run.Ruler?.TextOffset(depth) ?? level.TextOffset;
        ushort bulletOffset = properties.States(StatesBulletOffset)
            ? properties.BulletOffset
            : run.Ruler?.BulletOffset(depth) ?? level.BulletOffset;

        // A percentage paragraph space is resolved against the size of the paragraph's LAST
        // portion, not its first and not its largest. `PPTParagraphObj::ApplyTo` reads the height
        // off `m_PortionList.back()` before it converts the percentage to master units
        // (`filter/source/msfilter/svdfppt.cxx`:6296-6306, this tree) --
        //
        //     m_PortionList.back()->GetAttrib(PPT_CharAttr_FontHeight, nFontHeight, ...);
        //     if (static_cast<sal_Int16>(nUpperDist) > 0)
        //         nUpperDist = -static_cast<sal_Int16>((nFontHeight * nUpperDist * 100) / 1000);
        //
        // -- so a bullet whose last words are set smaller than its first gets a proportionally
        // smaller gap above it. Measured at 26.2.4.2 rather than only read: its own flat ODP of
        // `gillikin_online_user_mtg_2010.ppt` gives page 2's five bullets `fo:margin-top` of
        // 0.3 cm, 0.3, 0.318, 0.3 and 0.212 -- 20/80 of 34 pt, 34, **36** and **24**, which are
        // the last portion of each paragraph, where the first portion of the third and fifth is
        // 34 pt in both. Over the whole `.ppt` column this is the last portion 205 times out of
        // 208 multi-size paragraphs against 25.5 % for the first and 39.4 % for the largest
        // (`probes/slides-r108/ulpct2.py`).
        Length size = runs.Count > 0 ? runs[^1].Size : Length.FromPoints(characters.FontHeight);

        return new SlideParagraph(
            text,
            runs,
            Alignment(alignment),
            Distance(before, size),
            Distance(after, size),
            Spacing(lineFeed),
            MasterUnits(textOffset),
            MasterUnits(bulletOffset) - MasterUnits(textOffset),
            Language: null,
            Marker: Marker(
                properties,
                level,
                scheme,
                fonts,
                runs,
                linkedBullet,
                TextFontAt(run, characters, start),
                run.ExtensionAt(ExtendedIndexAt(run, start)),
                styles?.Extended.Level(run.Kind, depth) ?? default,
                bulletPictures,
                FontHeightAt(run, characters, start),
                characters.FontHeight,
                NumberingIsOwn(properties, run.Ruler, run.ExtensionAt(ExtendedIndexAt(run, start)), depth),
                depth,
                text.Length > 0,
                counters,
                counting))
        {
            // Every binary paragraph, unconditionally -- and that is a measurement rather than a
            // reading. `PPTParagraphObj::ApplyTo` puts the SvxLineSpacingItem (and with it
            // SvxInterLineSpaceRule::Prop, which at a proportion of exactly 100 makes the fit's
            // spacing reduction unreachable) behind `if (bIsHardAttribute)`, and that flag is set
            // when the paragraph states a line feed or its first portion states a typeface index
            // (svdfppt.cxx:6266-6288). Implementing exactly that disjunction was measured over the
            // whole track and it is NOT what the reference draws: 13 documents moved for -13.06
            // abs_ink, against -85.96 and 30 improvements for treating every .ppt paragraph as
            // stating its spacing. `slides/done-006/ppt/Lepore.ppt` is the case that identifies
            // it -- its body's paragraph mask is 0 and its character run states only a font
            // HEIGHT, so the record makes it soft by both terms, and the reference still draws it
            // at a pitch of 1.2 x em with the fit's 0.850 font scale applied.
            //
            // `GetAttrib` has two further hardness terms neither this reader nor the census can
            // evaluate -- a destination instance of TSS_Type::Unknown, and a comparison of the
            // source instance's master level against the destination instance's -- and the honest
            // statement is that one of those fires far more often than the record suggests. What
            // is measured is the outcome: on 26.2.4.2 no .ppt paragraph takes the ::Off arm.
            //
            // See probes/slides-r54/results.md for the A/B and for the authored known-answer deck.
            LineSpacingStated = true,

            // `rOutliner.Insert(OUString(), nParaIndex, mnDepth)` gives every paragraph its depth
            // before anything looks at its text (`svdfppt.cxx:2309`), and the empty-paragraph
            // suppression at `:2363-2366` clears only EE_PARA_BULLETSTATE. So a binary
            // paragraph's numbering level survives having no characters, and its bullet's box
            // still floors the line. See SlideParagraph.EmptyKeepsMarkerLevel.
            //
            // <strong>A PICTURE bullet is a hole in this, and it is measured rather than
            // guessed.</strong> `nBuBlip != 0xffff` with a graphic behind it makes the level
            // SVX_NUM_BITMAP (`svdfppt.cxx:3448-3465`) and `Outliner::ImplGetBulletSize` then
            // answers `pFmt->GetGraphicSize()` rather than a font's ascent and descent
            // (`outliner.cxx:1345-1350`), so the box is the picture's. This reader does not read
            // the picture: it falls back to the level's bullet CHARACTER, and on
            // `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt` -- the one corpus
            // deck whose outline masters carry one, `text:list-level-style-image` on
            // `Default-outline1` and `Default-outline2` in 26.2.4.2's own flat ODP of it -- that
            // character is a Wingdings 2 slot recoding to a Private Use code point no installed
            // face holds, drawn at 31 pt beside 20 pt text. Flooring an empty line at that
            // fiction's box costs pages 8 and 15 of that deck, which the same tree reading the
            // reference's own ODP of the file gets right. Gating on the paragraph's own
            // `BulletBlip` does NOT reach it: this deck's blip is inherited from the master's
            // `aExtParaSheet[instance].aExtParaLevel[level]` (`svdfppt.cxx:3425-3446`), which
            // nothing here reads -- tried, measured inert on all four pages, and removed rather
            // than left in as dead code. See probes/slides-size2-r110.
            EmptyKeepsMarkerLevel = true,

            // The master's own value, which PowerPoint writes as 0x240 — one inch — and which the
            // record's default already is. Reading it matters for the deck that states something
            // else, and stating nothing must not fall back to a word processor's half inch.
            //
            // The ruler's own default tab wins over it outright — `GetDefaultTab` is asked
            // unconditionally rather than only when the paragraph said nothing
            // (`svdfppt.cxx:5069-5070`), because a default tab is a property of the shape and a
            // paragraph has no way to state one.
            DefaultTabInterval = DefaultTab(run.Ruler, level),
        };
    }

    /// <summary>The tab interval in force, which the shape's ruler may state for itself.</summary>
    private static Length DefaultTab(PptTextRuler? ruler, PptParagraphLevel level)
    {
        if (ruler?.DefaultTab is { } stated && stated > 0) return MasterUnits(stated);

        return level.DefaultTab > 0
            ? MasterUnits(level.DefaultTab)
            : SlideParagraph.DefaultTabDistance;
    }

    /// <summary>
    /// The bullet the paragraph draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The character, its face, its size and its colour each fall through to the master's level
    /// independently, which is what lets a deck state a per-level bullet once and every slide use
    /// it. The size is a percentage of the text's, so it becomes
    /// <see cref="SlideMarker.Scale"/> rather than a length.
    /// </para>
    /// <para>
    /// <strong>The colour word is only the bullet's colour when a separate flag says so.</strong>
    /// PowerPoint writes a <c>bulletColor</c> into the record whether or not the bullet has one of
    /// its own, and gates it behind <c>PPT_ParaAttr_BuHardColor</c> — bit two of the bullet-flags
    /// word, which is stated by the paragraph when the mask names it and inherited from the
    /// master's level when it does not. With the flag clear the bullet takes the colour of the
    /// paragraph's <em>first character run</em> instead
    /// (<c>PPTParagraphObj::GetAttrib</c>, <c>filter/source/msfilter/svdfppt.cxx:5891-5916</c> for
    /// the paragraph's own set and <c>:6019-6055</c> for the fall-through to the level).
    /// </para>
    /// <para>
    /// Reading the word unconditionally is not a subtle error: measured on
    /// <c>slides/batch-007/ppt/architecture6.ppt</c>, every one of its eighty bullets came out
    /// <c>#000000</c> against a reference that draws them in the run's own <c>#46424D</c>, and the
    /// two whose paragraph opens on a red run are drawn red by LibreOffice and were black here.
    /// A null colour is what <see cref="SlideMarker"/> already spells "the first run's".
    /// </para>
    /// </remarks>
    /// <summary>
    /// Whether the paragraph gets a numbering rule of its own, or keeps its master's —
    /// <c>nHardCount</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PPTNumberFormatCreator::GetNumberFormat</c> (<c>svdfppt.cxx</c>:3690-3712) sums eight
    /// hardness answers, and <c>PPTParagraphObj::ApplyTo</c> (<c>:6151-6199</c>) then builds the
    /// paragraph's rule <em>only</em> when the sum is non-zero: it copies the style sheet's own
    /// <c>SvxNumBulletItem</c> for the instance and replaces the single level the paragraph sits
    /// at. A paragraph summing zero has no item put on it at all and keeps the master's rule at
    /// every level.
    /// </para>
    /// <para>
    /// <strong>That is what decides which font height sizes a picture bullet</strong>, because the
    /// two rules are built from different heights. The master's is
    /// <c>GetNumberFormat(…, rParaLevel, rCharLevel, …)</c> (<c>:3642-3661</c>), which passes
    /// <c>rCharLevel.mnFontHeight</c> — the master's character height at that depth. The
    /// paragraph's own is <c>pParaObj->First()->GetAttrib(PPT_CharAttr_FontHeight, …)</c>
    /// (<c>:3707-3708</c>), which is the first portion's stated height, or that same master
    /// character height where the portion states none.
    /// </para>
    /// <para>
    /// <strong>Measured on the two decks that disagree, and the eighth term is not needed to
    /// separate them.</strong> On
    /// <c>ws_prod-…-M.017-(French)-France.ppt</c> the page-8 body paragraphs state
    /// <c>0x1801</c> — <em>BulletOn</em>, alignment and line feed — so they sum one, take their
    /// own portion's 16 pt, and 26.2.4.2 draws their bullets at 17.86 pt, which is
    /// <c>round(16 × 0.254 × 155)</c>. On
    /// <c>ws_prod-g-doc-Events-Part-M-presentation.ppt</c> page 21 the paragraphs behind the six
    /// drawn pictures state <c>0x0000</c> and <c>0x1000</c> — neither of them one of the seven —
    /// so they sum zero, keep the master's rule, and the reference draws 31.24 and 19.02 pt, which
    /// are the master character levels' 28 and 24 pt at the same two percentages. Round 111
    /// measured this pair as unseparable and read <c>0x1801</c> as stating none of the seven;
    /// <c>PPT_ParaAttr_BulletOn</c> is bit zero and is the first of them.
    /// </para>
    /// <para>
    /// <strong>Confirmed against 26.2.4.2's own flat ODP of the Part-M deck.</strong> Its
    /// twenty-five <c>text:list-level-style-image</c> entries take four heights at level 1 —
    /// 1.102 cm, 0.945, 0.787, 0.709, which are 28, 24, 20 and 18 pt at 155 % — and exactly one at
    /// level 2, 0.671 cm, which is 24 pt at 110 % on every one of the thirteen list styles that
    /// has one. Its depth-1 paragraphs state a hard 18 pt portion throughout, so if the portion
    /// sized the box unconditionally, 0.709 cm would appear at level 2 somewhere and it never
    /// does. The two list styles on page 21 that override a level at all — <c>L18</c> at level 2
    /// and <c>L7</c> at level 1 — replace it with a <em>number</em>, not a picture, which is the
    /// same rule seen from the other side.
    /// </para>
    /// <para>
    /// The seven are <c>BulletOn</c>, <c>BulletChar</c>, <c>BulletFont</c>, <c>BulletHeight</c>,
    /// <c>BulletColor</c>, <c>TextOfs</c> and <c>BulletOfs</c>, and two of them are not simply
    /// mask bits. <c>BulletHeight</c> is hard only when the paragraph also sets
    /// <c>BuHardHeight</c> in both its mask and its flags word — see
    /// <see cref="StatesBulletHardHeight"/>. <c>TextOfs</c> and <c>BulletOfs</c> are hard when the
    /// shape's own <em>ruler</em> speaks for them as well as when the paragraph does, because
    /// <c>ReadParaProps</c>' tail writes the ruler's value into the property set and sets the mask
    /// bit with it (<c>:5062-5068</c>).
    /// </para>
    /// <para>
    /// The eighth term is <c>ImplGetExtNumberFormat</c>'s own return, added only for a bulleted
    /// paragraph (<c>:3709-3711</c>). It is <c>true</c> when the destination instance is
    /// <c>TSS_Type::Unknown</c> — which no shape path reaches, <c>ProcessObj</c> always resolving
    /// one at <c>:999-1028</c> — or when the paragraph carries an extended entry stating anything
    /// at all (<c>:3409-3419</c>).
    /// </para>
    /// <para>
    /// <strong>What is still not evaluated here is the same pair round 111 named</strong>: where
    /// the text's own instance differs from the destination the seven are also hard whenever the
    /// source instance's master level differs from the destination's (<c>:5954-5958</c>). A Body
    /// text resolves to the Body destination and never reaches it; a <em>HalfBody</em> or
    /// <em>QuarterBody</em> does, since both map to Body (<c>:1021-1024</c>). No page in this
    /// corpus's picture-bullet set is one, so the arm is left unimplemented rather than guessed.
    /// </para>
    /// </remarks>
    internal static bool NumberingIsOwn(
        PptParagraphRun properties, PptTextRuler? ruler, PptExtendedParagraph? extended, int depth)
    {
        if (properties.States(StatesBulletOn)
            || properties.States(StatesBulletChar)
            || properties.States(StatesBulletFont)
            || properties.States(StatesBulletColour))
        {
            return true;
        }

        if (properties.States(StatesBulletHardHeight)
            && (properties.BulletFlags & BulletHardHeightFlag) != 0
            && properties.States(StatesBulletHeight))
        {
            return true;
        }

        if (properties.States(StatesTextOffset) || ruler?.TextOffset(depth) is not null) return true;
        if (properties.States(StatesBulletOffset) || ruler?.BulletOffset(depth) is not null) return true;

        return extended is { Mask: not 0 };
    }

    private static SlideMarker? Marker(
        PptParagraphRun properties,
        PptParagraphLevel level,
        PptColourScheme scheme,
        PptFontTable fonts,
        List<SlideTextRun> runs,
        Colour? linkedBullet,
        ushort textFont,
        PptExtendedParagraph? extended,
        PptExtendedParagraphLevel masterExtended,
        PptBulletPictures? bulletPictures,
        ushort fontHeight,
        ushort masterFontHeight,
        bool ownNumbering,
        int depth,
        bool hasText,
        int[] counters,
        bool[] counting)
    {
        bool bulleted = properties.HasBullet ?? level.HasBullet;

        // `ImplGetExtNumberFormat` is reached only for a paragraph that draws a bullet
        // (`svdfppt.cxx:3709-3711`), and the picture branch outranks the automatic number:
        // `if (nBuBlip != 0xffff) … else if (nHasAnm)` (`:3448-3466`).
        PptExtendedParagraph merged = MergedExtension(extended, masterExtended);

        if (bulleted
            && runs.Count > 0
            && BulletPicture(
                   merged.BulletBlip,
                   bulletPictures,
                   ownNumbering ? fontHeight : masterFontHeight,
                   ownNumbering && properties.States(StatesBulletHeight)
                       ? properties.BulletHeight
                       : level.BulletHeight) is { } bitmap)
        {
            return new SlideMarker(
                string.Empty,
                null,
                MarkerScale(properties, level),
                MarkerColour(properties, level, scheme, linkedBullet))
            {
                Picture = bitmap,
            };
        }

        // The number outranks the bullet the paragraph states, and the paragraph goes on stating
        // one: a numbered PowerPoint list still carries its master's round dot in its property
        // run, so reading the two in the other order draws a bullet on every numbered item.
        // `ImplGetExtNumberFormat` reaches the same place by overwriting the SVX_NUM_CHAR_SPECIAL
        // the bullet put there (`svdfppt.cxx:3466-3630`).
        if (bulleted
            && runs.Count > 0
            && merged is { HasAutoNumber: true } numbering
            && PptNumbering.Next(numbering, depth, hasText, counters, counting) is { } number)
        {
            return new SlideMarker(
                number,
                runs[0].Typeface,
                MarkerScale(properties, level),
                MarkerColour(properties, level, scheme, linkedBullet),

                // A generated number sits on the text's own baseline rather than centred on it,
                // and is drawn in the paragraph's own face -- see `SlideMarker.IsSymbol`.
                IsSymbol: false);
        }

        PptNumbering.Break(depth, counting);

        if (!bulleted || runs.Count == 0) return null;

        char character = properties.BulletCharacter
                         ?? (level.BulletCharacter != 0 ? (char)level.BulletCharacter : '•');

        // The face word is only the bullet's face when a separate flag says so, and the flag is
        // PPT_ParaAttr_BuHardFont -- the exact counterpart of the BuHardColor rule below, and read
        // the same way: the paragraph's own flag when its mask names it, otherwise the master
        // level's. With the flag clear the word means nothing and the bullet is drawn in
        // <em>the face of the first character of the text it labels</em>
        // (<c>PPTParagraphObj::GetAttrib</c>, <c>svdfppt.cxx:5918-5942</c>, both the hard and the
        // inherited branch: "it is the font used which assigned to the first character of the
        // following text").
        //
        // Measured on <c>slides/done-014/ppt/Aerospace_Journey_of_Flight_Chapter_*.ppt</c> page 5,
        // whose Body master level states <c>buFlags=1</c> -- BulletOn and nothing else -- beside a
        // <c>buFont</c> of 0, Times New Roman. The reference draws every bullet on the page from
        // <b>Liberation Sans</b>, the face of the Arial text beside it; we took the level's word at
        // face value and drew them from Liberation Serif.
        bool hardFont = properties.States(StatesBulletHardFont)
            ? (properties.BulletFlags & BulletHardFontFlag) != 0
            : (level.BulletFlags & BulletHardFontFlag) != 0;

        ushort font = hardFont
            ? properties.States(StatesBulletFont) ? properties.BulletFont : level.BulletFont
            : textFont;

        ushort height = properties.States(StatesBulletHeight)
            ? properties.BulletHeight
            : level.BulletHeight;
        uint colour = properties.States(StatesBulletColour)
            ? properties.BulletColour
            : level.BulletColour;

        // The paragraph states the flag only when its mask names it; otherwise the master's level
        // holds it, exactly as the character and the face do.
        bool hardColour = properties.States(StatesBulletHardColour)
            ? (properties.BulletFlags & BulletHardColourFlag) != 0
            : (level.BulletFlags & BulletHardColourFlag) != 0;

        char symbol = PptTextReader.Symbolised(character, fonts, font);
        string? face = fonts[font];

        // A face whose slots LibreOffice has a recode table for keeps both its code point and its
        // own name, because the two only mean anything together: `SlideTextLayout` turns the slot
        // into the OpenSymbol glyph holding the same picture, and it needs the face to know which
        // table to use. Anything else keeps the old answer — U+2022, drawn in the paragraph's own
        // face, since a symbol face's name with a non-symbol code point resolves to nothing.
        bool recodeable = fonts.IsSymbol(font) && SymbolFontRecode.IsRecodeable(face);

        string text = recodeable
            ? symbol.ToString()
            : OutlineNumbers.NormaliseBullet(symbol.ToString());
        if (text.Length == 0) return null;

        string? typeface = recodeable ? face : fonts.IsSymbol(font) ? null : face;

        return new SlideMarker(
            text,
            typeface,
            MarkerScale(properties, level),
            MarkerColour(properties, level, scheme, linkedBullet));
    }

    /// <summary>
    /// The paragraph's own extended entry merged with the master's level for that outline depth.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PPTNumberFormatCreator::ImplGetExtNumberFormat</c>, <c>svdfppt.cxx:3407-3446</c>. The
    /// merge is skipped outright when the paragraph's own mask names all three fields, and each
    /// field is otherwise taken from the level only where the paragraph's mask does not name it.
    /// </para>
    /// <para>
    /// <strong>The picture bullet carries an extra condition and it is not symmetrical with the
    /// others.</strong> A blip is inherited only where the paragraph also says nothing about
    /// automatic numbering — <c>if (!(nBuFlags &amp; 0x02000000)) nBuBlip = rLev.mnBuBlip;</c>,
    /// under the comment "if there is a BuStart without BuInstance, then there is no graphical
    /// Bullet possible". So a paragraph that states <c>hasAnm</c> and nothing else suppresses its
    /// level's picture rather than merging it.
    /// </para>
    /// <para>
    /// <strong>Why this is where a picture bullet is found at all.</strong> Measured over the
    /// corpus's 51 <c>.ppt</c>: <b>8 documents carry a master level stating a blip</b>, 120 levels
    /// in all, and <c>ws_prod-…-M.017-(French)-France.ppt</c> — whose four O15 pages this closes —
    /// holds exactly <b>three</b> blip-bearing entries in any shape's or the document's own
    /// <c>ExtendedParagraphAtom</c>. Reading only the paragraph's entry therefore finds a picture
    /// bullet essentially nowhere. The same census counts <b>zero</b> master levels stating
    /// <c>hasAnm</c> anywhere in the column, so merging the numbering fields as well is the
    /// reference's rule and is inert on this corpus.
    /// </para>
    /// </remarks>
    internal static PptExtendedParagraph MergedExtension(
        PptExtendedParagraph? paragraph, PptExtendedParagraphLevel level)
    {
        PptExtendedParagraph own = paragraph ?? default;

        // `nBuFlags` is zero unless the paragraph carries an entry that states something, and
        // every field below is read out of the entry only under that guard (`:3409-3419`).
        uint mask = own.Mask;
        ushort blip = (mask & PptExtendedParagraphLevel.StatesBulletBlip) != 0
            ? own.BulletBlip
            : PptExtendedParagraph.NoBulletBlip;
        bool numbered = (mask & PptExtendedParagraphLevel.StatesAutoNumber) != 0 && own.HasAutoNumber;
        uint scheme = (mask & PptExtendedParagraphLevel.StatesScheme) != 0 ? own.Scheme : 0;

        if ((mask & PptExtendedParagraphLevel.StatesEverything)
                == PptExtendedParagraphLevel.StatesEverything
            || !level.IsSet)
        {
            return new PptExtendedParagraph(mask, blip, numbered, scheme);
        }

        uint master = level.Mask;

        if ((mask & PptExtendedParagraphLevel.StatesBulletBlip) == 0
            && (master & PptExtendedParagraphLevel.StatesBulletBlip) != 0
            && (mask & PptExtendedParagraphLevel.StatesAutoNumber) == 0)
        {
            blip = level.BulletBlip;
        }

        if ((mask & PptExtendedParagraphLevel.StatesScheme) == 0
            && (master & PptExtendedParagraphLevel.StatesScheme) != 0)
        {
            scheme = level.Scheme;
        }

        if ((mask & PptExtendedParagraphLevel.StatesAutoNumber) == 0
            && (master & PptExtendedParagraphLevel.StatesAutoNumber) != 0)
        {
            numbered = level.HasAutoNumber;
        }

        return new PptExtendedParagraph(mask | master, blip, numbered, scheme);
    }

    /// <summary>
    /// The picture bullet a resolved <c>buBlip</c> names, at the size the format states for it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>svdfppt.cxx:3448-3465</c>. The height is
    /// <c>round(fontHeight × 0.2540 × bulletHeight)</c> in hundredths of a millimetre — the
    /// paragraph's first portion's size in <em>points</em> against the level's bullet percentage —
    /// and the width follows the graphic's own aspect ratio. It is an absolute size fixed at
    /// import, so unlike a character bullet it does not move with the autofit's font scale.
    /// </para>
    /// <para>
    /// <strong>Confirmed against 26.2.4.2's own flat ODP</strong> of
    /// <c>ws_prod-…-M.017-(French)-France.ppt</c>, whose <c>text:list-level-style-image</c>
    /// elements state its resolved answer for every level it draws: 0.787 cm at 20 pt, 0.630 at
    /// 16, 0.709 at 18 and 1.102 at 28 with the level's 155 %, and 0.671 at 24, 0.503 at 18,
    /// 0.447 at 16 and 0.559 at 20 with the second level's 110 % — that expression to the
    /// hundredth of a millimetre at eight of eight.
    /// </para>
    /// <para>
    /// <strong>The bullet height is <em>not</em> put through the negative-value conversion
    /// first.</strong> <c>GetNumberFormat</c> turns a <c>nBulletHeight</c> above <c>0x7fff</c>
    /// into a percentage of the character height only when the format is not a bitmap
    /// (<c>:3659-3660</c>), and the graphic's size is computed before that in any case. So this
    /// takes the raw stored word where <see cref="MarkerScale"/> takes the clamped one.
    /// </para>
    /// </remarks>
    internal static SlideMarkerPicture? BulletPicture(
        ushort blip,
        PptBulletPictures? pictures,
        ushort fontHeight,
        ushort bulletHeight)
    {
        if (pictures is null) return null;

        long height = (long)Math.Round(fontHeight * 0.2540 * bulletHeight, MidpointRounding.AwayFromZero);
        if (height <= 0) return null;

        // Decoded once per blip and shared by every paragraph that names it — see
        // `PptBulletPictures.Decoded`, whose remarks carry what a picture per paragraph costs.
        if (pictures.Decoded(blip, BulletGraphic) is not { } graphic) return null;

        long width = graphic.Extent.Height > Length.Zero
            ? height * graphic.Extent.Width.Emu / graphic.Extent.Height.Emu
            : 0;

        return new SlideMarkerPicture(
            graphic.Raster, graphic.Vector, Length.FromMm100(width), Length.FromMm100(height));
    }

    /// <summary>One bullet graphic, decoded, with the extent its aspect ratio comes from.</summary>
    private sealed record BulletGraphicEntry(
        RasterImage? Raster, Lazy<VectorImage>? Vector, DocSize Extent);

    /// <summary>
    /// Decodes one bullet graphic and reads back the size its aspect ratio comes from.
    /// </summary>
    /// <remarks>
    /// <c>Graphic::GetPrefSize</c> is only ever divided into itself here — <c>nWidth = (nHeight ×
    /// aPrefSize.Width()) / aPrefSize.Height()</c> — so the unit does not matter and a raster's
    /// pixel extent serves. A graphic with no readable extent takes a zero width, exactly as the
    /// reference's own <c>aPrefSize.Height()</c> guard does.
    /// </remarks>
    private static BulletGraphicEntry? BulletGraphic(EscherBlip graphic)
    {
        ReadOnlyMemory<byte> bytes = graphic.Bytes;
        if (bytes.IsEmpty) return null;

        // A metafile bullet is decoded here rather than on first paint, which is the one place
        // this reader pays for a picture it may not draw. It has to: the aspect ratio comes from
        // the graphic itself, and a bullet has no stated extent of its own to fall back on. Rare
        // enough not to matter -- all eight of the corpus decks carrying a picture bullet store
        // theirs as PNG or JPEG.
        if (VectorImages.For(bytes.Span) is not null)
        {
            Lazy<VectorImage> vector = new(() => VectorImages.Decode(bytes));
            VectorImage decoded = vector.Value;
            DocSize extent = decoded.IntrinsicSize.IsEmpty
                ? decoded.ViewBox.Size
                : decoded.IntrinsicSize;

            return new BulletGraphicEntry(null, vector, extent);
        }

        return new BulletGraphicEntry(
            RasterImage.Encoded(bytes, BulletMediaType(graphic.RecordType)),
            null,
            SlideImages.NaturalSize(bytes.Span) ?? default);
    }

    /// <summary>What a bullet blip's record type says its bytes are.</summary>
    private static string? BulletMediaType(ushort recordType) => recordType switch
    {
        0xF01D or 0xF02A => "image/jpeg",
        0xF01E => "image/png",
        0xF01F => "image/bmp",
        0xF029 => "image/tiff",
        _ => null,
    };

    /// <summary>The marker's size as a fraction of its text's.</summary>
    private static double MarkerScale(PptParagraphRun properties, PptParagraphLevel level)
    {
        ushort height = properties.States(StatesBulletHeight)
            ? properties.BulletHeight
            : level.BulletHeight;

        return height is > 0 and <= 400 ? height / 100.0 : 1.0;
    }

    /// <summary>The marker's own colour, or null for the first run's.</summary>
    /// <remarks>
    /// The paragraph states <c>PPT_ParaAttr_BuHardColor</c> only when its mask names it; otherwise
    /// the master's level holds it, exactly as the character and the face do.
    /// </remarks>
    /// <param name="properties">The paragraph's own property run.</param>
    /// <param name="level">The master's level, for everything the paragraph does not state.</param>
    /// <param name="scheme">The page's colour scheme.</param>
    /// <param name="linkedBullet">
    /// The colour the paragraph's first portion had before a text-range hyperlink recoloured it,
    /// or null when that portion is not linked. A soft bullet takes the first portion's colour,
    /// and <c>GetAttrib</c> reaches past the link for it
    /// (<c>filter/source/msfilter/svdfppt.cxx:6037-6042</c>).
    /// </param>
    private static Colour? MarkerColour(
        PptParagraphRun properties,
        PptParagraphLevel level,
        PptColourScheme scheme,
        Colour? linkedBullet = null)
    {
        uint colour = properties.States(StatesBulletColour)
            ? properties.BulletColour
            : level.BulletColour;

        bool hardColour = properties.States(StatesBulletHardColour)
            ? (properties.BulletFlags & BulletHardColourFlag) != 0
            : (level.BulletFlags & BulletHardColourFlag) != 0;

        if (hardColour) return PptColour.ResolveText(colour, scheme);

        return linkedBullet;
    }

    /// <summary>
    /// Which <c>ExtendedParagraphAtom</c> entry is in force at a paragraph's first character.
    /// </summary>
    /// <remarks>
    /// The selection is made by the <em>character</em> run rather than by the paragraph, which is
    /// how one atom entry covers three paragraphs and a second covers the fourth
    /// (<c>PPTStyleTextPropReader::ReadCharProps</c>, <c>svdfppt.cxx:5171-5182</c>).
    /// </remarks>
    private static int ExtendedIndexAt(PptTextRun run, int start)
    {
        int position = 0;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            if (start >= position && start < runEnd) return character.ExtendedIndex;

            position = runEnd;
        }

        return 0;
    }

    /// <summary>
    /// The typeface index in force at a paragraph's first character.
    /// </summary>
    /// <remarks>
    /// What a bullet with no hard face of its own is drawn in. The character run covering the
    /// paragraph's first character when it states a face, and the level's otherwise — which is
    /// <c>PPTParagraphObj::GetAttrib</c>'s own pair of alternatives for
    /// <c>PPT_ParaAttr_BulletFont</c> (<c>svdfppt.cxx:5929-5941</c>).
    /// </remarks>
    private static ushort TextFontAt(PptTextRun run, PptCharacterLevel level, int start)
    {
        int position = 0;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            if (start >= position && start < runEnd)
            {
                return character.States(StatesFontIndex) ? character.FontIndex : level.FontIndex;
            }

            position = runEnd;
        }

        return level.FontIndex;
    }

    /// <summary>
    /// The point size in force at a paragraph's first character, which sizes a picture bullet.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PPTNumberFormatCreator::GetNumberFormat</c> initialises it to <b>24</b> and then asks the
    /// paragraph's <em>first portion</em> for <c>PPT_CharAttr_FontHeight</c>
    /// (<c>svdfppt.cxx:3705-3708</c>), so a paragraph whose first portion states nothing and whose
    /// master level states nothing takes 24 rather than zero. It is the raw stored word, in whole
    /// points, and not the size the fit will draw the text at.
    /// </para>
    /// <para>
    /// <strong>It is not always the height the reference sizes the picture from, and which one it
    /// is was not settled.</strong> A paragraph gets a numbering rule of its own only when
    /// <c>nHardCount</c> is non-zero (<c>PPTParagraphObj::ApplyTo</c>, <c>:6151</c>); otherwise it
    /// keeps the <em>master's</em> rule, built once per instance at <c>:4409-4419</c> from
    /// <c>rCharLevel.mnFontHeight</c>. Measured on two decks with the same master structure:
    /// on <c>ws_prod-…-M.017-(French)-France.ppt</c> page 8 the reference's six bullets are
    /// 17.86 pt, which is this portion's 16 pt at the level's 155 %, and on
    /// <c>ws_prod-g-doc-Events-Part-M-presentation.ppt</c> page 21 they are 31.24 and 19.02 pt,
    /// which are the <em>master's</em> 28 pt at 155 % and 24 pt at 110 %. Implementing the
    /// <c>nHardCount</c> test over the seven paragraph attributes it sums does <em>not</em>
    /// separate them — France's paragraph states none of the seven and still takes its portion's
    /// height — so the discriminator is one of <c>GetAttrib</c>'s two terms this reader cannot
    /// evaluate, the destination instance and the source-against-destination master comparison,
    /// exactly as <c>LineSpacingStated</c> above records for a different question. The portion is
    /// used because it is what the paragraph overload literally reads and because it is exact on
    /// the deck this seat is about; the cost is one page, and
    /// <c>probes/slides-final-r111/results.md</c> §2.3 measures it.
    /// </para>
    /// </remarks>
    private static ushort FontHeightAt(PptTextRun run, PptCharacterLevel level, int start)
    {
        int position = 0;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            if (start >= position && start < runEnd)
            {
                return character.States(StatesFontHeight) ? character.FontHeight : level.FontHeight;
            }

            position = runEnd;
        }

        return level.FontHeight;
    }

    /// <summary>
    /// The runs covering a paragraph, each resolved against the master's level for what it does
    /// not state.
    /// </summary>
    private static List<SlideTextRun> Runs(
        PptTextRun run,
        PptColourScheme scheme,
        PptFontTable fonts,
        PptCharacterLevel level,
        int start,
        int length,
        int inherit,
        int textLength)
    {
        List<SlideTextRun> runs = [];
        int end = start + length;
        int position = 0;

        // The properties in force at the paragraph's first character, kept for the empty-paragraph
        // case below: an empty paragraph covers no characters, so the loop places nothing from it,
        // and the run it sits inside is the only thing that says how tall its blank line is.
        //
        // The loop below must therefore be allowed to run PAST `end` until it has found that run.
        // For an empty paragraph `start == end`, and the character runs are contiguous from zero,
        // so the run that *ends* at `start` is the last one the old `position >= end` break saw --
        // one short of the run that *contains* `start`. `atStart` was consequently never found for
        // any empty paragraph other than one at text position 0, and every blank line in the
        // corpus fell back to the master level's character height.
        //
        // Measured on `slides/done-005/ppt/ITE106-Chapter 4.ppt` p7, whose bullets are separated
        // by paired carriage returns: its blank paragraphs sit on one-character runs stating
        // 12 pt, the level default is 32, and the reference draws the blank line at 12 -- both in
        // its own flat-ODF export, which gives those paragraphs `fo:margin-top="0.106cm"`
        // (= 12 x 20/80 pt) against the text paragraphs' `"0.212cm"` (= 24 x 20/80), and in the
        // rendered page, whose inter-bullet baseline gap decomposes as
        // 28.800 + 3.004 + 1.2x12 + 6.008 = 52.212 pt against the 52.214 it draws.
        //
        // Not a fraction of a line: 32 against 12 is 24 pt of surplus height per blank paragraph,
        // which on this page pushed the shrink-to-fit walk two rows down `constScaleLevels` and
        // cost the whole body 2 pt of em.
        // `inherit` is that position, and it is `start` for every paragraph but one. The phantom
        // paragraph a trailing return leaves behind takes the run one character EARLIER, because
        // the reference does not read the character run covering the text's own length at all:
        // `PPTStyleTextPropReader::Init`'s loop is bounded by `nCharReadCnt < nStringLen`, so the
        // portion it appends afterwards is a copy of `aCharPropList.back()` — the portion of the
        // paragraph before it (`filter/source/msfilter/svdfppt.cxx`:5403-5409). A `.ppt` states one
        // more character run than it has characters and PowerPoint writes something different in
        // it: `pres_ioc_phuket.ppt` page 26's banner is a single `\r` with runs of 40 pt and 8 pt,
        // and 26.2.4.2 measures both of its lines at 40 — its own flat-ODP export gives the shape
        // 3.647 cm, which is 2 x fround(1411 x 1.2) plus the two 127-unit insets.
        PptCharacterRun atStart = default;
        bool found = false;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            int from = Math.Max(position, start);
            int to = Math.Min(runEnd, end);

            if (!found && inherit >= position && inherit < runEnd) { atStart = character; found = true; }

            // A text-range hyperlink splits the portion it lands in, and each piece it covers is
            // its own field: `svdfppt.cxx:7080-7091` clones the PPTCharPropSet at the range's end
            // and gives the head its own SvxURLField, so a run half inside a link is two portions.
            for (int piece = from; piece < to;)
            {
                bool linked = run.IsLinked(piece);
                int stop = piece + 1;
                while (stop < to && run.IsLinked(stop) == linked) stop++;

                runs.Add(Run(character, scheme, fonts, level, piece - start, stop - piece, linked));
                piece = stop;
            }

            position = runEnd;

            // `found`, not just `position >= end`: see the note above `atStart`. A paragraph that
            // covers characters finds its run on the first overlapping iteration, so this stops
            // exactly where it used to for every non-empty paragraph.
            if (position >= end && found) break;
        }

        // Text past the last stated run, and a run that states none at all, both take the level's
        // defaults. A writer that under-counts is commoner than one that over-counts, and dropping
        // the tail would lose the text rather than its formatting.
        int covered = 0;
        foreach (SlideTextRun placed in runs) covered += placed.Length;

        if (covered < textLength)
        {
            runs.Add(Run(default, scheme, fonts, level, covered, textLength - covered));
        }

        // An empty paragraph still gets one run, of no characters, carrying the level's size.
        //
        // It is a blank *line*, not nothing: layout drops a paragraph that resolves no face at
        // all, so without this an empty paragraph contributes no height and everything below it
        // moves up by a line. PowerPoint decks use them as spacing constantly — the fourth page of
        // WC_Update-Aug03.ppt separates all eleven of its bullets that way, and LibreOffice's own
        // flat-ODF export of it writes each as a list header holding one empty paragraph.
        //
        // The PPTX reader has always done this; the comment on SlideParagraph.Runs says why.
        if (runs.Count == 0)
        {
            runs.Add(Run(atStart, scheme, fonts, level, 0, 0));
        }

        return runs;
    }

    /// <param name="character">The character property run the piece falls in.</param>
    /// <param name="scheme">The page's colour scheme.</param>
    /// <param name="fonts">The document's font collection.</param>
    /// <param name="level">The master's character level, behind everything the run leaves unsaid.</param>
    /// <param name="start">Where the piece begins in the paragraph's own text.</param>
    /// <param name="length">How many characters it covers.</param>
    /// <param name="linked">
    /// Whether a text-range hyperlink covers it, which makes it a field and redecorates it —
    /// see the remarks.
    /// </param>
    private static SlideTextRun Run(
        PptCharacterRun character,
        PptColourScheme scheme,
        PptFontTable fonts,
        PptCharacterLevel level,
        int start,
        int length,
        bool linked = false)
    {
        ushort fontIndex = character.States(StatesFontIndex) ? character.FontIndex : level.FontIndex;
        ushort height = character.States(StatesFontHeight) ? character.FontHeight : level.FontHeight;
        uint colour = character.States(StatesColour) ? character.Colour : level.Colour;

        RunEmphasis emphasis = (level.Emphasis & ~character.Stated)
                               | (character.Emphasis & character.Stated);

        if (linked)
        {
            // The scheme's "accent and hyperlink" slot, imposed on the portion whatever it stated
            // (`SetColor(PPT_COLSCHEME_A_UND_HYPERLINK)`, `svdfppt.cxx:7060` and `:7094`, the
            // constant at `:145`).
            colour = HyperlinkSchemeColour;

            // And the emphasis is *replaced* rather than added to: `svdfppt.cxx:7054-7056` sets
            // the underline bit in the attribute mask and then assigns -- not ors --
            // `mnFlags = 1 << PPT_CharAttr_Underline`, so every emphasis the portion stated for
            // itself is turned off and stated as off, while the ones it left to the master still
            // come from there. A bold linked run is therefore drawn upright and light.
            RunEmphasis stated = character.Stated | RunEmphasis.Underline;
            emphasis = (level.Emphasis & ~stated) | RunEmphasis.Underline;
        }

        // Already a percentage in the file, so it goes straight through; the size that goes with
        // it does not, and LibreOffice supplies DFLT_ESC_PROP whenever the value is non-zero
        // (filter/source/msfilter/svdfppt.cxx:5764-5775).
        short escapement =
            character.States(StatesEscapement) ? character.Escapement : level.Escapement;

        return new SlideTextRun(
            start,
            length,
            fonts[fontIndex],
            Length.FromPoints(height > 0 ? height : level.FontHeight),
            emphasis.HasFlag(RunEmphasis.Bold) ? 700 : 400,
            emphasis.HasFlag(RunEmphasis.Italic),
            PptColour.ResolveText(colour, scheme) ?? Colour.Black,
            IsUnderlined: emphasis.HasFlag(RunEmphasis.Underline),
            IsStruckThrough: emphasis.HasFlag(RunEmphasis.Strikethrough),
            IsShadowed: emphasis.HasFlag(RunEmphasis.Shadow),
            Escapement: escapement == 0
                ? SlideEscapement.None
                : new SlideEscapement(escapement, SlideEscapement.AutomaticProportion),
            // A hyperlinked portion is an EditEngine field, which changes where the line breaks
            // and how the lines it spills onto are stacked -- see `SlideTextRun.IsField`,
            // `PptHyperlinkRange` and `PptHyperlinks`.
            IsField: linked);
    }

    /// <summary>
    /// The packed colour word for the scheme's "accent and hyperlink" slot.
    /// </summary>
    /// <remarks>
    /// <c>PPT_COLSCHEME_A_UND_HYPERLINK</c>, <c>filter/source/msfilter/svdfppt.cxx:145</c> — the
    /// <c>0x08</c> family with the scheme index in the low word, which
    /// <see cref="PptColour.ResolveText"/> already resolves.
    /// </remarks>
    private const uint HyperlinkSchemeColour = 0x0800_0006;

    /// <summary>
    /// The colour the character run at a position states, before any hyperlink recolours it.
    /// </summary>
    /// <remarks>
    /// The portion's own word when it states one and the master character level's when it does
    /// not, which is the pair <c>GetAttrib</c> chooses between for a soft bullet whose paragraph
    /// opens on a link (<c>filter/source/msfilter/svdfppt.cxx:6039-6042</c>).
    /// </remarks>
    private static uint UnlinkedColour(PptTextRun run, PptCharacterLevel level, int start)
    {
        int position = 0;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            if (start >= position && start < runEnd)
            {
                return character.States(StatesColour) ? character.Colour : level.Colour;
            }

            position = runEnd;
        }

        return level.Colour;
    }

    /// <summary>
    /// The paragraph properties covering the character at <paramref name="start"/>.
    /// </summary>
    /// <remarks>
    /// A paragraph property run is <em>not</em> one paragraph: its count is a character count and
    /// a writer may cover several paragraphs with one run
    /// (<c>filter/source/msfilter/svdfppt.cxx:5081-5090</c>).
    /// </remarks>
    private static PptParagraphRun PropertiesAt(IReadOnlyList<PptParagraphRun> runs, int start)
    {
        int position = 0;

        foreach (PptParagraphRun run in runs)
        {
            position += Math.Max(run.Length, 1);
            if (start < position) return run;
        }

        return runs.Count > 0 ? runs[^1] : default;
    }

    private static TextAlignment Alignment(ushort adjust) => adjust switch
    {
        1 => TextAlignment.Centre,
        2 => TextAlignment.End,
        3 => TextAlignment.Justify,
        _ => TextAlignment.Start,
    };

    /// <summary>
    /// A line feed as a spacing rule.
    /// </summary>
    /// <remarks>
    /// One field, two units: a positive value is a percentage of the natural line height and a
    /// negative one is a fixed height in eighths of a point
    /// (<c>PPTParagraphObj::ApplyTo</c>, <c>svdfppt.cxx:6273-6289</c>). Reading the sign the wrong
    /// way turns a 92% paragraph into one set at eleven and a half points.
    /// </remarks>
    private static LineSpacingRule Spacing(short lineFeed)
        => lineFeed switch
        {
            0 => LineSpacingRule.SingleSpaced,
            > 0 => LineSpacingRule.Multiple(lineFeed / 100.0),
            _ => LineSpacingRule.Exactly(Length.FromPoints(-lineFeed / 8.0)),
        };

    /// <summary>
    /// The space above or below a paragraph, in the same two units as the line feed.
    /// </summary>
    /// <remarks>
    /// Negative is a distance in master units; positive is a proportion of the font's height, and
    /// LibreOffice's conversion of it is <c>fontHeight × value / 10</c> master units, with the
    /// height in points (<c>svdfppt.cxx:6300-6305</c>). Eighty master units make a point, so the
    /// whole of it reduces to a division by eighty — and the body style's default of 20 is a
    /// quarter of the font's size rather than a fifth of it.
    /// </remarks>
    private static Length Distance(short value, Length fontSize)
        => value <= 0
            ? MasterUnits((ushort)Math.Min(-value, ushort.MaxValue))
            : Length.FromEmu(fontSize.Emu * value / 80);

    private static Length MasterUnits(ushort units)
        => Length.FromEmu((long)units * Length.EmuPerInch / PptSlideLayout.MasterUnitsPerInch);
}
