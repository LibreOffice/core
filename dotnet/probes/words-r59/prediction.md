# words-r59 — prediction 1: the list label's slant

Committed before any post-change rendering. Base `e4296ee8520`, branch `wt-words-r59`.
Baseline reproduced first: `batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`,
scored against `MANIFEST.tsv`'s own 337-path list → **319 match, 18 open, zero disagreements with
the manifest's status column, document for document**.

## What the probe settled before the change (`label-slant.py`, 16 packages × 4 formats)

Round 58's five-row table said "the level's `w:rPr` leans the bullet, and the paragraph mark's
leans it; a run's does not". That is true and **incomplete in two ways that decide the code**:

1. **The level wins outright when it states anything.** `<w:i w:val="0"/>` on the level over an
   italic paragraph mark draws the label **upright** (`leveloff-markon`), and `<w:i/>` on the
   level over `<w:i w:val="0"/>` on the mark draws it **leaning** (`levelon-markoff`). So the
   rule is *level-if-stated, else the mark* — not *level OR mark*. This matters on a real corpus
   document: 13 of the 271 `.docx` state `w:i w:val="0"` on a level and also carry an italic list
   paragraph mark.
2. **A bullet and a number do not have the same base.** The paragraph *style*'s `w:i` leans a
   **number** label and does **not** lean a **bullet** (`style`: number 10 of 10 italic, bullet 0
   sheared). That is `#i53199` in `SwTextFormatter::NewNumberPortion` — the bullet branch resets
   posture and weight on the base font before the level's format is applied, and the number branch
   resets only underline and overline.

So the rule implemented is:

    bullet: level-stated ?? paragraph-mark-DIRECT-stated ?? false
    number: level-stated ?? the paragraph mark's resolved posture

## What I expect to change

| quantity | baseline | predicted |
|---|---:|---|
| words verdict | 319 of 337 | **0 movement — 319**, downside risk −1 |
| slides verdict | 200 of 302 | 0 movement (no shared layer is touched) |
| sheets verdict | 276 of 307 | 0 movement (ditto) |
| OpenSymbol glyphs the reference shears and we do not | 112 in 10 | **0 – 25** |
| — of which the `.docx` arm (7 documents) | 32 | **0** |
| — of which the `.doc` arm (3 documents) | 80 | **0 – 25** |
| words renderings whose bytes change | — | **12 – 45** |
| words page counts changed | — | **0** |
| words extractable-word counts changed | — | **0** |
| words font lists changed | — | **0 – 4**, and see the risk below |

The label-slant probe itself, after the change: `docx` and `doc` columns agreeing with the
reference on all 16 packages; `odt` agreeing on the level arm; `rtf` unchanged and still wrong.

## The reach census, and it partitions rather than sums

`label-italic-census.py` counts three **arms**, not one total, because they are different code
paths and a document can be in more than one; the union is printed as a union.

- **A** — a level states `w:i` on: **8** `.docx`, 369 levels.
- **B** — a list paragraph's own `w:pPr/w:rPr` states it: **18** `.docx`, 118 paragraphs.
  The style chain is deliberately *not* walked, because #i53199 means a style's `w:i` cannot
  reach a bullet.
- **C** — a level states it *off* over an italic mark: **13** `.docx`. This arm takes a lean
  **away**, and two of the corpus's number labels are drawn italic by us today where the
  reference draws them upright.
- union of A and B: **26** `.docx`.

## What the census cannot see, written down before the measurement

1. **The 66 `.doc` documents are not examined at all.** A WW8 level's `sprmCFItalic` is in a
   binary `grpprlChpx` this script does not parse, so they are reported as *not examined* rather
   than as zero. The three known `.doc` witnesses (`A320SimNotes` 75 glyphs, `SFSP_2013-02` 3,
   `AirbusCallouts` 2) are 80 of the 112, so **the larger half of the target is in the arm the
   census is blind to**, and its predicted range is correspondingly wide.
2. **A and C are upper bounds**: Word writes nine levels per `abstractNum` and a document may
   reference one of them. A level stating `w:i` that no paragraph uses changes nothing.
3. **The census cannot see whether the label is a bullet or a number**, and the two have
   different bases. A document in arm B whose levels are all numbered moves nothing, because a
   number label already takes the mark's posture today.
4. **It cannot see `w:iCs`.** The complex-script slot is not on the predicate here; round 56 and
   round 58 between them established that `w:iCs` leans complex-script text and not Latin, and a
   recoded OpenSymbol bullet is Latin. Unmeasured, and named as unmeasured.
5. **The `.rtf` arm has no witness at all** — the words corpus is 271 `.docx` and 66 `.doc`.

## The risk that could move a verdict, stated in advance

`batch-check.sh`'s third check is font embedding. A **number** label that starts resolving to
`LiberationSerif-Italic` in a document that names that face nowhere else adds a row to our font
list; a number label that stops resolving to it removes one. Either can flip the third check in
**either** direction. I predict zero net movement and name **−1 to +1** as the honest band, and
the per-document comparison will be reported unnetted whichever way it goes.

## Deferred, with the reason and the measurement

- **`.rtf`.** Our RTF reader draws the bullet in the paragraph's own Liberation Serif and not in
  OpenSymbol at all — `01-bullet-level.rtf` is `LiberationSerif 0 lean / 9 flat` against the
  reference's `LiberationSerif 8 flat + OpenSymbol 1 lean`. The label's face and its slant are
  both in the `{\listtext}` destination's own character formatting, which the reader discards.
  That is a larger defect than the slant and it has **zero witnesses in this corpus**.
- **The extra ODT glyph.** Our `.odt` bullet renderings draw **two** OpenSymbol glyphs where the
  reference draws one, in every one of the eight bullet packages including the control. Separate
  from the slant, pinned here, not chased.
