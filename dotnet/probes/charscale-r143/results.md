# Round 143 — O94: a character width stated by a style, lost by the uniform shortcut

**Measured against LibreOffice 26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**, with
the tarball's duplicate, Noto Latin, Narrow and Condensed faces moved aside. Both sweep legs are
pinned with `SOURCE_DATE_EPOCH=0`.

## 1. The seat's diagnosis was wrong, and the right one is three sites

O94 said `w:w` "is read from a run and not from a paragraph style's `w:rPr`". **The property is
resolved correctly through the style chain and always was** — a probe over the witness' own styles
answers `WidthPerCent=60` for a paragraph naming a style that states it, with no run properties at
all. What loses it is what happens *after*.

A paragraph whose runs all carry their style's formatting is **uniform**: the run splitter folds
the runs away and it reaches the layout with none. Three separate fallbacks then rebuild one run
from the paragraph's own face, size, shaping and tracking — and **none of the three carried a
width**:

1. `PageParagraph.Measure`, which builds the `FormattedRun` the measurement uses;
2. `PageDrawing.RunsIn`, which builds the `PageRun` the pen uses;
3. and the one that actually decided the line breaks — **the shortcut in `Paginator` and
   `FlowLayouter`**, which hands the breaker the paragraph's *text*, a face, a size and its
   shaping, and measures from those alone.

Tracking survives the third because `EffectiveShaping` carries it; a scale has nowhere to ride.
`PageParagraph` gained a `WidthPerCent`, the first two fallbacks pass it, and the third is
excluded by `IsHorizontallyScaled`.

**Fixing only the first two is worse than fixing none**, which is why the fixture has an arm that
wraps: with the pen scaled and the breaker not, every line is drawn at exactly `scale ×` its
unscaled width and broken in the unscaled places. That state passed every one-line measurement
this round started with, and it took a wrapping arm to see it.

## 2. The fixture, and the reference

`features/words-style-char-scale.docx`, built by `make-fixture.py`. Five paragraphs: the style
states `w:w="60"` and the runs state nothing; a run states it and the style does not; neither does;
then the same three-way contrast over eighteen words, long enough to wrap. 60 rather than the
corpus's commonest 99 because the question is whether the scale is applied at all.

`fixture-lines.tsv`, in points:

| arm | ours | 26.2.4.2 |
|---|---:|---:|
| A — style states it | 140.386 | 140.270 |
| B — run states it | 140.386 | 140.270 |
| C — neither | 233.976 | 233.832 |
| D — scaled, wrapping | **two** lines, 431.957 and 207.130 | **two**, 434.895 and 206.899 |
| E — unscaled, wrapping | three lines | three lines |

**Before the fix arm D broke into the same three lines as arm E**, each exactly 0.6 of it. After,
the break words agree with 26.2.4.2 exactly. The reference's spans include each line's trailing
space where this tree's do not, which is the whole of the residual on every line that has one.

## 3. The witness

`Regulations Governing the Status…docx`, whose title style `SL` states `<w:w w:val="96"/>` and
whose title runs state no `w:rPr` at all (`title-lines.tsv`):

| line | before | after | 26.2.4.2 |
|---|---:|---:|---:|
| `Regulations Governing the` | 316.58 | 303.40 | 309.77 |
| `Status, Basic Rights and Duties` | *broke after "and"* | 351.93 | 358.79 |
| `of Officials other than` | *began with "Duties"* | 248.22 | 254.57 |
| `Secretariat Officials, and` | 294.24 | 281.94 | 288.30 |
| `Experts on Mission` | 227.56 | **218.09** | **218.06** |

The title now breaks on the same words as the reference on all five lines, and the last line —
the only one with no trailing space for the reference's span to include — agrees to **0.03 pt**.
Over the whole document the mean |Δx| of 780 matched spans about their own median goes **0.365 →
0.205 pt**; pages 18 on both sides before and after; alphanumerics unchanged at 33 617 against
33 604.

## 4. Reach, and a census that had to be corrected against the sweep

Rendering the whole 337-document words track twice: **1 rendering moves and 336 are
byte-identical**, 0 failed either leg. The mover is the witness.

`census.py` predicts exactly that — **1 document, 166 paragraphs** — but only after its predicate
was corrected, and the correction is the useful part. The first cut counted a paragraph as uniform
when its runs merely agreed **with each other**, and reported 4 documents and 1138 paragraphs.
`RunsOf` compares each run against the **paragraph mark**, not against its neighbours, so a run
stating 99 inside a paragraph whose mark resolves to 100 makes the paragraph *vary* — the runs are
kept and that path was always right. **Census the predicate the code tests, not the one the prose
describes**; with the predicate corrected the census and the sweep agree exactly.

## 5. What this round did not do

**Three of the four readers do not read a character width at all.** `git grep` over
`src/XRay.Office.WordProcessing` finds `WidthPerCent` in the DOCX path and nowhere else: the WW8
reader does not read `sprmCCharScale`, the RTF reader does not read `\charscalex`, and the ODF one
does not read `style:text-scale`. Seated as **O101**, uncensused.

## 6. Pinned by mutation

Each of the three sites, mutated against the restored fix and rebuilt; each takes **2 of the 3**
tests with it, and a different two for the third than for the first two:

| mutation | of 3 |
|---|---|
| the measurement fallback drops the width | 2 fail |
| the drawing fallback drops the width | 2 fail |
| a scaled paragraph still takes the shortcut | 2 fail |

Restored with `cp` + `touch` and rebuilt; 3 of 3 pass and no `MUTATION` marker survives.

## 7. Suite

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 591 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 750 | 0 | 0 |
| Vector | 309 | 0 | 0 |
| Rendering | 164 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 194 | 0 | 0 |
| WordProcessing | 2013 | 0 | 0 |
| Spreadsheets | 1416 | 0 | 0 |
| Presentations | 1205 | 0 | 0 |
| Fidelity | 542 | **10** | 0 |

The ten fidelity failures are the documented set, unchanged. **0 skipped on Fidelity.**

## 8. Files

| file | what it is |
|---|---|
| `make-fixture.py` | writes `features/words-style-char-scale.docx`, including the wrapping arm |
| `census.py`, `census.tsv` | the corrected predicate, which agrees with the sweep exactly |
| `title-lines.tsv` | the witness' title, line for line, before/after/reference |
| `fixture-lines.tsv` | the fixture's eight drawn lines, ours against 26.2.4.2 |
| `witness.tsv` | the witness' pages, alphanumerics and span spread |
| `fingerprints-before.txt`, `fingerprints-after.txt` | md5 of all 337 words renderings, each leg |
| `par-sweep-words.sh` | renders the words track three at a time, one directory per document |
