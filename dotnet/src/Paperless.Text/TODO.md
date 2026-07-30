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
library's interpretation of them. (Also, `SixLabors.Fonts` is unusable — it fails the build
without a paid licence.)

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

- [ ] UAX #14. LibreOffice delegates to ICU, so matching its breaks means matching ICU's
      rules — hand-rolled approximations disagree on CJK, on hyphens and dashes, and around
      punctuation.
- [ ] Decide: `ICU4N` (prerelease only today), hand-roll, or bind native ICU. **Open
      question in the master TODO; resolve early** — everything downstream depends on it.
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
