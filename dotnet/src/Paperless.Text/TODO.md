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
- [ ] Mid-run fallback when the primary face lacks a glyph. Coverage is queryable; choosing the
      fallback face and splitting the run is shaping's job and not written yet.

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
- [ ] Script and direction sub-runs. LibreOffice splits a bidi run into script runs and shapes each
      separately; Paperless shapes the whole run with the script guessed from its text, so a
      paragraph mixing Latin with a complex script may shape differently. Needs a script property
      table, which is the same shape of generated artefact as the line-break tables.
- [ ] Bidi resolution (UBA). `ShapingOptions.RightToLeft` shapes a run in one direction; resolving
      mixed direction into runs is not written.
- [ ] Letter spacing and justification adjustments — both adjust advances after shaping, so they
      belong here rather than in the measurer.
- [ ] Vertical text.
- [ ] Font fallback mid-run when the primary face lacks a glyph. Coverage is queryable and the
      resolver can choose a face; splitting the run and shaping each part is the missing piece.
- [ ] Cache shaped runs across calls. The harfbuzz face and font are cached per face, but a repeated
      word is reshaped; tables and lists repeat text constantly.

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
