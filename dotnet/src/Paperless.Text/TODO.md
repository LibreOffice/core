# Paperless.Text — TODO

Fonts, metrics, shaping, line breaking, paragraph layout. Shared by all three families.

**This library decides whether rendering can ever match LibreOffice.** Every measurement it
gets wrong propagates: wrong advance widths give wrong line breaks give wrong pagination,
and every page after the divergence differs for reasons unrelated to drawing.

Reference: `research/06-rendering.md` section B; `research/05-infrastructure.md` section F.

## Font resolution

- [x] `SystemFontIndex` over the platform's font directories, built by reading each file's `name`
      table. Deliberately not fontconfig: since the substitution chain comes from LibreOffice's own
      table rather than from the platform, going through fontconfig would add a second source of
      truth rather than the missing one — at the cost of any rules an administrator configured.
- [x] LibreOffice's substitution order: the document's embedded face, the requested family, then
      LibreOffice's chain, then a fallback by shape, then whatever is installed. Never null, because
      a document naming a font nobody has still has to render.
- [x] The substitution table itself, **generated from
      `officecfg/registry/data/org/openoffice/VCL.xcu`** rather than reimplemented — the research
      notes call that file the single most valuable portable artefact for a C# port, because it
      encodes what LibreOffice actually falls back to independently of any platform font API.
- [x] The metric-compatible pairs resolve: Calibri→Carlito, Cambria→Caladea, Arial→Liberation Sans,
      Times New Roman→Liberation Serif, Courier New→Liberation Mono. Compatibility is *derived* from
      the table's own Microsoft-equivalent declarations rather than hardcoded, so a new pair needs no
      code.
- [x] Embedded fonts win over anything installed: they are what the author saw, and the only face
      guaranteed to have the metrics the document was laid out against.
- [x] **Every substitution is reported**, with whether it was metric-compatible — the difference
      between a page that looks slightly different and a document whose every later page is wrong.
- [x] Faces cached on `FaceKey`, so disposing one view of a face does not invalidate another.
- [ ] Per-locale substitution. Only the neutral `en` table is generated; the per-locale ones differ
      mainly in CJK preference order, and using one locale's answers for another changes which font a
      document renders in. Wants the locale plumbed through the resolver first.
- [x] Mid-run fallback when the primary face lacks a glyph. The candidate order is **ported from
      LibreOffice's own generic list** — `ImplInitGenericGlyphFallback`,
      `vcl/source/font/PhysicalFontCollection.cxx:113` — rather than invented, because the face that
      draws a missing character decides its advance width and therefore where the line holding it
      breaks. Anything installed that covers the character is tried after that list, ordered by name
      so two runs of the same document agree. **Every fallback is reported** on
      `SystemFontResolver.GlyphFallbacks`, including the characters nothing could draw: a fallback
      face is chosen for its coverage rather than its metrics, so it is almost never
      metric-compatible, and without the list there is nothing to tell that apart from a layout bug.
- [ ] The platform half of the fallback chain. LibreOffice asks fontconfig first, with the missing
      characters as a charset (`vcl/unx/generic/fontmanager/fontconfig.cxx`), and only falls back to
      the generic list when that fails. Paperless has only the second half, deliberately — see the
      fontconfig note above — but it means a machine whose administrator configured a fallback gets
      Paperless's answer rather than LibreOffice's. Worth revisiting only with a way to read
      fontconfig's *configuration* rather than to call its matcher.

## Metrics — hand-rolled OpenType reader

Hand-rolled deliberately: we need raw table access and our own precedence rules, not a
library's interpretation of them.

- [x] sfnt and TTC parsing; `head`, `hhea`, `OS/2`, `post`, `name`, `cmap`, `hmtx`. A collection
      holds several faces, so a face is addressed by file *and* index — a reader that assumes one
      face per file reads the first face of every CJK font on the system.
- [x] **Line height derivation**, in the documented precedence: `hhea` first but only if its signs
      are right, then `OS/2`'s `usWin*` because that is what Windows used and what a generation of
      documents was authored against, unless `fsSelection` bit 7 asks for the typographic metrics
      instead. Which set was believed is *reported*, because a line-height difference is the most
      visible way two renderers diverge and knowing the source turns a half-page offset into a
      one-line answer.
- [x] Verified end to end against LibreOffice: `LineHeightComparisonTests` has LibreOffice render
      three single-spaced lines to PDF in each of five fonts, measures the baselines out of the PDF,
      and compares the pitch with what the reader derived from the same font file. All five agree to
      within a fortieth of a point, which is the unit noise between hundredths of a millimetre and
      points.
- [x] Underline and strikethrough position and thickness, each falling back to a fraction of the em
      rather than to zero — a zero-thickness line draws nothing at all.
- [x] Glyph coverage queries for fallback, from `cmap` formats 0, 4, 6 and 12. Format 12 is
      preferred where a face has both, because preferring format 4 silently loses every character
      above U+FFFF — which is every emoji.
- [x] Advance widths, including the repeat past the end of `hmtx`: a CJK font states one width for
      most of its glyphs, so a reader that returns zero past the table measures a whole Japanese
      paragraph as empty.
- [ ] The per-font override lists LibreOffice ships (`FontsUseWinMetrics`,
      `FontsDontUseUnderlineMetrics`) for the specific fonts known to lie about their own metrics
- [ ] Variable fonts: at minimum do not crash; ideally honour the named instance. Today the
      static metrics are read, which is the default instance rather than the requested one.
- [x] `kern` and `GPOS` are not read here on purpose — HarfBuzz reads them, and it is handed the
      font file whole for exactly that reason. Two readers of the same positioning tables would be
      two chances to disagree with LibreOffice about an advance width.

## Shaping

- [x] HarfBuzzSharp, configured the way LibreOffice configures it — which is what makes the advances
      agree by construction rather than by approximation. Four details do the work: the font is
      scaled to the face's own units per em (`hb_font_set_scale(font, upem, upem)` in
      `LogicalFontInstance::InitHbFont`) so advances come back on the design grid unrounded; metrics
      come from the OpenType tables rather than a rasteriser; **no features are passed unless
      something is being switched off**, which leaves HarfBuzz's defaults in force the way an empty
      feature list does in `CommonSalLayout.cxx`; and clusters are counted per character
      (`HB_BUFFER_CLUSTER_LEVEL_CHARACTERS`) so a cluster index is an index into the text.
- [x] `ShapingOptions` is named after what it switches **off**, so `default` means what LibreOffice
      means by default. LibreOffice's own flags are the same way round (`DisableKerning`,
      `DisableLigatures`), and getting it backwards would silently give every caller who said nothing
      an unkerned approximation.
- [x] Kerning and the standard ligatures, therefore. Not cosmetic: a line of English prose at 12 pt
      in Carlito accumulates 244 thousandths of an em of kerning — just under 3 pt — which is enough
      to decide whether its last word fits. Measured without it, one of the fidelity test's
      paragraphs came out 0.8 pt over a 481.9 pt text width and lost its last word, and every line
      after that was wrong too.
- [x] `ShapedText` answers for **every prefix** of the run, not just the whole. Filling lines asks
      for hundreds of prefix widths per paragraph, and shaping each separately would be quadratic
      *and* wrong, since a prefix shaped alone is not always a prefix of the shaped whole. A glyph's
      width is credited to its cluster, which is what `GenericSalLayout::GetPartialTextWidth` does.
- [x] Graceful fallback to `MetricsShaper` when the native harfbuzz library is absent, with
      `TextShaper.IsShapingAvailable` saying which was used — an unshaped document has slightly wrong
      line breaks, where a `DllNotFoundException` is no document at all. The two shapers agree
      exactly for text with nothing to kern, which is what makes the fallback a fallback rather than
      a different answer that happens to be close.
- [x] Verified end to end: `LineBreakPositionTests` has LibreOffice lay six paragraphs out to PDF at
      a known text width, reads the word boxes back with `pdftotext -bbox`, groups them into lines,
      and compares line by line — at three em sizes, and with kerning both on and off on both sides.
      Two of the paragraphs are kerning-heavy on purpose, and the test asserts that they really do
      break differently with kerning than without, so the comparison cannot pass by accident.
- [ ] Letter spacing and justification adjustments — both adjust advances after shaping, so they
      belong here rather than in the measurer.
- [ ] Vertical text.
- [ ] Cache shaped runs across calls. The harfbuzz face and font are cached per face, but a repeated
      word is reshaped; tables and lists repeat text constantly.

## Itemisation — direction, script and face sub-runs

One pass, not three: a paragraph's text becomes a sequence of sub-runs, each with a direction, a
script and a face, and each shaped separately. `MeasuredParagraph.Measure` does it, resolving the
bidi algorithm over the **whole paragraph** and only then intersecting with the formatting runs —
a run boundary is a change of font, not a change of direction, so resolving per run would let a
bold word inside a Hebrew sentence see the paragraph's direction instead of the sentence's.

**Decided: read the property tables from ICU, not from the UCD.** ICU is the library LibreOffice
itself resolves bidi and script with, so its tables are the authority for agreeing with Writer
rather than a defensible second opinion — and it also makes a *differential* possible that the line
breaker could not have: 7,944 generated cases checked against ICU's own answers. The scripts are
`generate-itemisation-tables.py` and `generate-bidi-cases.py`; see `scripts/README.md`.

- [x] The `Bidi_Class`, `Bidi_Paired_Bracket` and `Script` tables, generated into
      `Itemisation/BidiProperties.Tables.cs` and `Itemisation/ScriptProperties.Tables.cs` from ICU 74
      (Unicode 15.1) through `ctypes`. The Unicode version is written into each file, so a
      regeneration that moves it shows up in the diff.
- [x] UAX #9 in full, in the standard's order: P2–P3, X1–X8, X9's removals, X10's isolating run
      sequences, W1–W7, N0 with the bracket pairs, N1–N2, I1–I2, L1 and L2. Rule by rule rather than
      as a table, for the reason the line breaker gives: when a level comes out wrong the only useful
      question is which rule decided it.
- [x] Bracket pairs (N0). Not optional — without them "‏שלום (abc) עולם" puts the parentheses the
      wrong way round, which is the most visible bidi bug a reader will meet, because parenthesised
      Latin inside right-to-left prose is everywhere in technical writing.
- [x] Script itemisation (UAX #24), ported from `vcl/source/gdi/scrptrun.cxx` — ICU's `ScriptRun`
      sample as LibreOffice vendored it, with LibreOffice's two adjustments baked into the generated
      table so the itemiser cannot forget them: a non-spacing mark reports `Zinh` whatever its own
      script is (tdf#154549), and Katakana reports `Hira` because the three Japanese script codes
      share one OpenType tag. The bracket stack is the part that looks like an accident and is not: a
      closing bracket has to take its *opener's* script, or "(Ελληνικά) English" shapes its two
      halves in different faces.
- [x] Format control characters are cut out rather than shaped, which is `ImplLayoutArgs::AddRun`
      splitting on `IsControlChar` (`vcl/source/text/ImplLayoutArgs.cxx`). One departure, recorded
      as a decision: LibreOffice also removes U+0001–U+001F and Paperless keeps them, because the tab
      is in that range and its width is resolved by the line filler rather than by the shaper.
- [x] **The no-op case is a genuine no-op, measured.** A Latin paragraph produces exactly one
      sub-run and reaches HarfBuzz in the identical call — same `ShapingOptions` instance, no script
      or direction override — and `ItemisationTests` asserts the glyphs come back glyph for glyph
      equal to shaping the whole run. That is not pedantry: a paragraph split into runs it does not
      need loses the shaping context at each boundary and measures very slightly wide, which is
      enough to move a line break.
- [x] Verified against LibreOffice, in `Paperless.Fidelity.Tests/BidiItemisationComparisonTests`.
      LibreOffice emits one `BT … ET` block per portion in *logical* order with an absolute pen, so
      one reference PDF gives both halves: the glyph counts read cumulatively are the portion
      boundaries as character offsets, and the pens are the visual order the reordering produced.
      Every level-run boundary is one of LibreOffice's, and the leftmost pen of each sub-run rises
      strictly along the visual order — nine paragraphs, six left-to-right and three right-to-left.
- [x] Verified against ICU differentially: every ordered pair and triple of bidi classes, brackets
      with each kind of content and context, and prose in Hebrew, Arabic and Latin, at both
      paragraph directions. Two behaviours were **measured out of ICU rather than read out of the
      standard**, and both change where a portion boundary falls: the levels of the characters X9
      removes propagate *backwards* (ICU reports "a RLE b PDF c" as 0 2 2 0 0, giving each control
      the level of the text it opens or returns to, where the UAX reference propagates forwards and
      would say 0 0 2 2 0); and a paragraph with nothing right-to-left in it is reported flat rather
      than having its Arabic numbers raised two levels by I1, which is the difference between one
      portion and two. The second has a subtle clause found only by measurement — ICU resolves "٠٠"
      flat but "٠ ٠" as 2 1 2, because a space between two Arabic numbers can itself become
      right-to-left under N1.
- [ ] 72 of the 7,944 differential cases hold an *unterminated or leading* embedding, override or
      isolate, and on those ICU short-circuits to a flat level in a way that could not be
      characterised without its source — "‫ا" at paragraph level 1 is 1 1 to ICU and 3 3 by the
      rules, and "‭ا" at level 0 is 0 0 to ICU and 2 2 by the rules. Their *reordering* agrees, which
      is what renders, so the test holds them to that and not to the levels. It matters little in
      practice: no format Paperless reads emits an embedding or an isolate, so the characters only
      arrive when an author typed one. Closing it wants ICU's `ubidi.cpp`, which is not in this tree
      (`external/icu` is a tarball recipe, not the source).
- [ ] Writer's *font* script runs, which are a different and coarser partition from the shaper's.
      `SwScriptInfo` splits a paragraph by `GetScriptClass` — Latin, Asian or Complex, from a block
      table in `i18nutil/source/utility/scriptclass.cxx:56` — because those choose between a
      paragraph's Western, Asian and CTL *font attributes*. Measured: a space is `WEAK` there by an
      explicit special case (bug 102975) and an opening parenthesis is `LATIN` because it is in the
      Basic Latin block, so "‏שלום (abc) עולם" gets a Writer portion boundary at the bracket that no
      UAX #24 itemisation has. Paperless's readers resolve one face per run and never look at the
      CJK or CTL font attributes, so there is nothing yet for this to choose between; it belongs
      with whichever reader starts honouring `w:rFonts/@w:cs` and `style:font-name-complex`.
- [ ] Drawing right-to-left text. `MeasuredParagraph.Items` carries the levels and
      `TextItemiser.InVisualOrder` orders them, but nothing in `Paperless.WordProcessing.Layout`
      consumes either — a mixed-direction paragraph still measures correctly and draws its runs left
      to right in logical order. `SwTextPainter` is the shape to follow: portions are stored
      logically and each is given an absolute pen, which is exactly what the PDF export shows.
- [ ] Aligning a right-to-left paragraph. `TextAlignment.Start` is resolved against the paragraph's
      writing mode by the layouter, not by this, and `MeasuredParagraph.ParagraphLevel` is the value
      it would need.

## Line breaking

**Decided: hand-roll UAX #14.** The runtime offers nothing equivalent, so there is no
cheaper option to weigh it against.

What the BCL actually provides (verified by reflecting over the shipped assemblies on .NET
10, not assumed):

| Available | Not available |
|---|---|
| `TextElementEnumerator` / `StringInfo` — grapheme clusters (UAX #29) | Any line-break iterator |
| `Rune` — code point enumeration | The Unicode `Line_Break` property |
| `CharUnicodeInfo.GetUnicodeCategory` — general category only | `East_Asian_Width` |
| `string.Normalize` — NFC/NFD | Word or sentence segmentation |

Note the trap: `StringInfo` looks like segmentation but returns **grapheme clusters**, which
are per-character and unrelated to where a line may break. And although .NET's globalization
is ICU-backed on Linux, the BCL surfaces only collation, casing, normalisation and calendars
— ICU's `BreakIterator` is not exposed, so being "on ICU" buys us nothing here.

So `ICU4N` (prerelease-only) or a native ICU binding were the alternatives, and hand-rolling
avoids both a prerelease dependency and a native one.

- [x] The `Line_Break`, `East_Asian_Width` and `Extended_Pictographic` tables, generated into
      `Layout/LineBreakProperties.Tables.cs` — see `scripts/README.md` for their provenance and
      what would improve it.
- [x] Rules LB1 to LB31, written **in the standard's order** rather than as a pair table. A pair
      table is faster and opaque: when a break comes out wrong, the only way to find out why is to
      work out which cell decided it. A paragraph is a few hundred characters, so the speed is
      irrelevant and the legibility is not.
- [x] The four places LibreOffice's rule set differs from current UAX #14, since matching
      LibreOffice's line breaks is the whole point: LB15 disabled (i#83649), a number range that
      breaks after its hyphen (i#83229), CJ folded into NS for strict breaking, and LB21a in the
      Unicode 15.0 form without 15.1's `[^HL]`.
- [x] Verified differentially against an independent implementation of the same specification over
      4,065 generated cases — every ordered pair of line-break classes, the same pairs across a
      space, and real prose in eight scripts. The four deliberate differences are excluded from that
      set and asserted directly instead, because agreeing there would be wrong.
- [ ] Unicode's own `LineBreakTest.txt` as a third source. Stronger evidence than agreement with
      another implementation, and not reachable from this environment: the egress policy denies
      `unicode.org`.
- [ ] Language-specific tailoring. `FindBreakOpportunities` takes a language and ignores it; the
      cases that need it are few but real — Finnish and Slovak hyphenation-adjacent rules, and the
      CJ handling that a document could ask to be loose rather than strict.
- [ ] Dictionary-based breaking for Thai, Lao, Khmer and Burmese. Those scripts resolve to
      `Alphabetic` today, so their text gets no intra-word breaks at all — which is what every
      implementation without a dictionary produces, and better than breaking in the wrong places.

### Where this will diverge from LibreOffice, and what to do about it

LibreOffice's breaks are **ICU's** breaks, and ICU is UAX #14 *plus tailorings*. Expect two
gaps:

- **South East Asian scripts (`SA`: Thai, Lao, Khmer, Burmese) need dictionary-based
  breaking.** These scripts do not delimit words with spaces, so UAX #14 defers to a
  dictionary and ICU ships one. A pure rule implementation cannot break them correctly.
  **Decided: ship a dictionary as an embedded resource.**
  - [ ] Build the dictionary from ICU's own source word lists so breaks agree with
        LibreOffice's rather than merely being defensible.
  - [ ] Store it as a compact trie (or DAWG) embedded in the assembly — these lists are
        large in text form and must not become a loose file to deploy.
  - [ ] Longest-match segmentation over `SA` runs, with a sensible fallback for
        out-of-dictionary text rather than refusing to break at all.
  - [ ] Keep it in a separate assembly or an opt-in resource if size becomes a problem;
        most documents never touch these scripts, so paying for it unconditionally is worth
        measuring before accepting.
- **ICU's minor tailorings** elsewhere. Find these empirically by diffing against reference
  output rather than trying to predict them, and record each as a known deviation.

- [ ] Hyphenation (optional; only when a document enables it). LibreOffice uses Hunspell
      dictionaries, so matching its breaks needs the same dictionaries.
- [ ] East Asian line-break rules (kinsoku).

## Paragraph layout

- [x] Break a paragraph into lines within a given width, honouring a first-line indent that differs
      from the rest. **Greedy, deliberately not Knuth-Plass**: total-fit produces better-looking
      paragraphs and *different* line breaks, and different line breaks are precisely what must not
      happen when the point is to agree with Writer.
- [x] Two details that decide more breaks than the measurement does. A line's trailing spaces do not
      count towards its width, so a space that would overflow the margin does not push its word to
      the next line — which is why a paragraph of short words does not break after every one of them.
      And a single word too long for the line takes the line alone and is allowed to exceed it,
      because the alternative is an empty line followed by the same problem.
- [x] `ParagraphFormat`: the resolved layout properties, one type for what the four formats spell four
      ways. It lives here rather than beside the word-processing model because a spreadsheet cell and a
      slide's text box lay their paragraphs out with the same rules — LibreOffice's EditEngine plays
      exactly this part for the same reason.
- [x] **Line spacing**, all four modes, verified against LibreOffice's rendered baselines: proportional
      (a multiple), at-least (whichever of the declared and natural heights is larger), exact (honoured
      even when it clips), and leading (natural plus a gap). They agree at single spacing and are four
      separate calculations everywhere else, so all four are tested.
- [x] Where the extra height goes, which is asymmetrical and easy to get backwards: proportional
      spacing puts it **above** the text, so a double-spaced paragraph pushes its first baseline down
      rather than leaving a gap under its last line. Exact spacing shorter than the ascent clamps the
      baseline to the box and clips from below, because text climbing into the paragraph above is worse
      than text with its descenders cut.
- [x] Indents: start, end, and a first-line indent that may be **negative** — the hanging indent every
      numbered list is built from, where the number sits out to the left of the text. A reader that
      clamps it at zero starts every list in the wrong place.
- [x] Alignment: start, end and centre, verified against LibreOffice. Centring is computed from the
      line's own width, so it is also a check on the measurement: a line a point too wide is centred
      half a point too far left.
- [x] Contextual spacing, which needs **both** paragraphs to ask for it — that is what keeps a list
      tight while still leaving a gap before it.
- [ ] Justification. `TextAlignment.Justify` and `Distribute` are recorded and the last line is left
      start-aligned, but the stretching itself is not done: it adjusts advances inside the line rather
      than where the line starts, so it belongs with the shaped run.
- [ ] Tab stops. `ParagraphFormat` carries them and the default interval; nothing advances to them yet,
      and doing it properly means the line filler has to be tab-aware, since a tab's width depends on
      where in the line it falls.
- [ ] Whether space-before collapses against the previous paragraph's space-after. Word takes the
      larger, Writer adds them, and which applies is a compatibility flag — so it belongs to whatever
      assembles paragraphs into a page and knows the flag, not to the paragraph.
- [ ] Drop caps; widow/orphan control (the counts are carried, nothing enforces them)
- [ ] Non-rectangular text areas, for text wrapping around floating objects

## Open questions

- [ ] How closely must justification match? LibreOffice distributes extra space by a
      specific rule; approximating it shifts every glyph on a justified line.
