# Paperless.Text — TODO

Fonts, metrics, shaping, line breaking, paragraph layout. Shared by all three families.

**This library decides whether rendering can ever match LibreOffice.** Every measurement it
gets wrong propagates: wrong advance widths give wrong line breaks give wrong pagination,
and every page after the divergence differs for reasons unrelated to drawing.

Reference: `research/06-rendering.md` section B; `research/05-infrastructure.md` section F.

## Font resolution

- [ ] `IFontResolver` over the system font set (fontconfig on Linux, the platform store
      elsewhere)
- [ ] Reproduce LibreOffice's substitution order: the document's own font table, then
      LibreOffice's built-in substitution tables, then the platform's, then a last-resort
      default
- [ ] Metric-compatible pairs must resolve correctly — Calibri→Carlito, Cambria→Caladea,
      Arial→Liberation Sans, Times New Roman→Liberation Serif. These preserve advance
      widths and therefore line breaks; a non-metric-compatible substitution reflows
      everything.
- [ ] Embedded fonts win over anything installed: they are what the author saw
- [ ] **Report every substitution.** A silent one explains most otherwise-baffling reflow
      differences, so surface it rather than hiding it.
- [ ] Font caching keyed on `FaceKey`

## Metrics — hand-rolled OpenType reader

Hand-rolled deliberately: we need raw table access and our own precedence rules, not a
library's interpretation of them.

- [ ] sfnt/TTC parsing; `head`, `hhea`, `OS/2`, `post`, `name`, `cmap`, `hmtx`
- [ ] **Line height derivation.** The precedence between `hhea` ascent/descent, `OS/2`
      `usWinAscent`/`usWinDescent`, and `OS/2` typo metrics is specific, easy to get wrong,
      and shifts every baseline on the page when wrong. Follow
      `research/06-rendering.md` section B exactly, including its per-font override list.
- [ ] Underline and strikethrough position and thickness
- [ ] Glyph coverage queries for fallback
- [ ] Variable fonts: at minimum do not crash; ideally honour the named instance

## Shaping

- [ ] HarfBuzzSharp — the same engine LibreOffice uses, so advances agree by construction
- [ ] Script and direction runs; bidi resolution (UBA)
- [ ] Kerning, standard ligatures, letter spacing, justification adjustments
- [ ] Vertical text
- [ ] Font fallback mid-run when the primary face lacks a glyph

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

- [ ] Generate the `Line_Break` property table from Unicode's `LineBreak.txt` into a compact
      trie. This is the bulk of the data and it is mechanical — generate it, do not hand-write
      it, and check the generator in so it can be re-run on a Unicode update.
- [ ] Generate the `East_Asian_Width` table from `EastAsianWidth.txt` (needed by rule LB30
      and by kinsoku).
- [ ] Implement rules LB1–LB31 as a pair-table plus the handful of rules that need context.
      Around 400 lines of rule engine on top of the generated tables.
- [ ] Test against Unicode's own `LineBreakTest.txt` conformance suite — it is exhaustive and
      makes this verifiable independently of LibreOffice.
- [ ] Then diff against LibreOffice on real documents, since conformance to UAX #14 and
      agreement with ICU are not the same thing (below).

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

- [ ] Break a paragraph into lines within a given width, honouring indents
- [ ] Alignment: left, right, centre, justified, distributed
- [ ] Tab stops, including default intervals and decimal tabs
- [ ] Line spacing: proportional, at-least, exact, leading
- [ ] Drop caps; first-line indent; widow/orphan control
- [ ] Non-rectangular text areas, for text wrapping around floating objects

## Open questions

- [ ] Cache shaped runs? Shaping the same word repeatedly is common in tables and lists.
- [ ] How closely must justification match? LibreOffice distributes extra space by a
      specific rule; approximating it shifts every glyph on a justified line.
