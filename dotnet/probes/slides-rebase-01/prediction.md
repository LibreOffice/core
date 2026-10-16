# Slides rebase 01 — prediction, committed before any measurement

Written before running `ref-baseline.sh`, before opening a single chart source file, and
before grepping for a table-style id. Reference binary in this container is
**LibreOffice 26.2.4.2 620(Build:2)**; every stored slides figure was taken against
**24.2.7.2**.

The comparison baseline for "did the reference move" is `dotnet/probes/slides-r41/base-parity.tsv`
(163 rows, `ours/ref` per column, taken at `e5f54617c` against 24.2.7.2). Our column cannot be
re-measured this round — no package feed, no `XRay.Office.Cli` — so every verdict figure below is
**old ours vs new ref**, which is exactly the quantity that tells us how much of the stored
scoreboard survives the binary change.

## Gate, restated so a wrong number says which check moved

1. pages exact;
2. `|ours-ref| <= max(2% of ref, 3 words)`;
3. `unembedded == 0`.

## Predictions

| # | Claim | Prediction |
|---|---|---|
| P1 | reference **page** counts that differ from r41's ref column | **0 of 163** (I expect the predecessor to reproduce) |
| P2 | zip `ppt/slides/slideN.xml` count == new refpages, over the pptx half | **≥ 108 of 114 exact**; every mismatch a deck with `show="0"` hidden slides, since LO's PDF export omits hidden slides by default |
| P3 | reference **word** counts that differ *at all* from r41's ref column | **150–163** — I expect the predecessor's "160 of 163" to reproduce |
| P4 | reference word counts that move **beyond the gate band** (>2% and >3 words) | **3–12 of 163** — this is the refutation I expect: P3 is true and the sentence attached to it ("the word gate is unstable across the version change") is wrong, because the gate is a 2% band and a re-chunked text layer moves counts by ones |
| P5 | verdicts surviving: old-ours vs new-ref | **144–151 of 163** (from 151/163). Any drop is P4's documents, not new page failures |

If P1 fails, gate check 1 is *not* structurally stable and the whole "slides is the track the
version change hurts least" framing is wrong. If P3 reproduces but P4 lands at 3–12, the
predecessor's measurement is right and its implication is wrong — the dominant pattern.

### What this census cannot see

- Our half. No CLI, so no `ours` re-measure; if our renderer's output also moved between the
  two commits, P5 is an upper bound on agreement, not a measurement of it.
- `.ppt` decks (51 of 163) have no zip, so P2's control covers only the 114 pptx.
- Word counts come from `pdftotext`; a change in *its* version would masquerade as a renderer
  change. Same container, same `pdftotext`, so this is constant across P1–P5 but not across
  the 24.2.7.2 figures, which were taken on a different image.

## Chart cluster priors (guesses, to be scored)

| # | Item | Prior |
|---|---|---|
| C1 | `Demick_JetBlue` — `DrawingStyleMatrix` reads `a:lnStyleLst`/`a:fillStyleLst` and resolves them | **true**, and the missing piece is the automatic-colour rule plus a matrix handoff from `PptxSlideLayout` to the chart reader |
| C2 | `16 - UTM - (NASA)` — the chart text path cannot reach `FallbackShaper` | **true**, a sixth wiring hole of the sheets-cell shape |
| C3 | `8_P-Pavese_AIRBUS` — the deck's `a:tblPr` style id resolves against a ported 74-entry predefined table style table | **the table is ported but the lookup is never called from the pptx table reader** — i.e. the entries exist and nothing consults them. Second-most-likely: the table is ported partially (< 74) and this id is absent |
| C4 | `Fundamentals_Module_1_basics.ppt` / `W3_Case_Study` arrows | the shapes **do not reach** `PptShapeGeometry.PresetOf` at all — a `.ppt` binary Escher path that never consults the preset table, so adding entries would have moved nothing |
