# words-r56 — second prediction, the legacy `FORMCHECKBOX`

Committed before the corpus is re-rendered with the change. The first prediction
(`prediction.md`) covered the synthetic oblique only and said so.

## What the record said, and what is measurably wrong with it

`HANDOVER.md` §8 has said since round 38: *"249 legacy `FORMCHECKBOX` fields across 16 documents —
established, deliberately not implemented: the drawn square's size would not pin (9.0…15.9 pt, not
following `w:checkBox/w:size`; LibreOffice's own arithmetic gives 3.53 mm where the square measures
3.175 mm) and 12 of the 16 currently match."*

Two things are wrong with that and one is a warning.

1. **The census is smaller than the corpus.** `checkbox-census.py` over `MANIFEST.tsv`'s 337 paths,
   counting `w:checkBox` in **every** part of every package rather than in `document.xml` alone:
   **675 boxes in 12 documents**, all `.docx`. 566 state `w:sizeAuto`, 109 state a `w:size`, 10 are
   ticked.
2. **It pins exactly, and it was measured against the wrong candidate.**
   `SwFieldFormCheckboxPortion::Format` (`sw/source/core/text/portxt.cxx`:1492) sets the portion's
   width and height to `rInf.GetTextHeight()` and its ascent to `rInf.GetAscent()`;
   `SwTextPaintInfo::DrawCheckBox` (`inftxt.cxx`:1247) strokes that rectangle deflated by a hard
   `delta = 25` twips a side. So the square follows the **line's text height**, and 9.0…15.9 pt is a
   range of *font sizes* read as a failure to pin. `w:checkBox/w:size` is expected to be inert.

   Measured on 26.2.4.2, `formcheckbox.py`, nineteen authored packages, **control first**: the same
   input rendered twice agreed to the digit before anything else was read.

   | | text height | drawn square | difference |
   |---|---:|---:|---:|
   | Liberation Serif 8 pt | 184 tw | 134 tw (6.700 pt) | 50 tw |
   | 12 pt | 276 | 226 (11.300) | 50 |
   | 24 pt | 552 | 502 (25.100) | 50 |
   | 40 pt | 920 | 870 (43.500) | 50 |
   | Liberation Mono 12 pt | 272 | 222 (11.100) | 50 |
   | DejaVu Sans 12 pt | 280 | 230 (11.500) | 50 |
   | Carlito 12 pt | 293 | 243 (12.150) | 50 |

   Four fixtures stating `w:size` of 5, 10, 20 and 40 pt all draw the run's own 11.300.

3. **The warning stands and is the reason to predict carefully.** All 12 documents holding these
   **currently match**. This change can gain nothing on the gate and can lose up to twelve.

## What I predict

| quantity | instrument | predicted |
|---|---|---|
| documents whose rendering changes | byte diff, `/CreationDate` normalised | **12, exactly the census** |
| **verdict movement** | `batch-check.sh` vs `MANIFEST.tsv` | **0** |
| downside risk | same | **−1 to −4**, all within the 12 |
| page counts changed | `parity.tsv` | **0 to 3**, all within the 12 |
| extractable words changed | `parity.tsv` | **0** — the box has no text layer |
| font lists changed | `pdffonts` | **0** — no new face is asked for |
| documents outside the 12 that move | byte diff | **0** |

**Zero verdict movement is the prediction and the fix is still right**, because the width is the
half that matters: 675 positions were reserving *nothing* where the reference reserves a square of
the line's text height, so every line holding one was laid out narrower than the reference lays it
out. A line that is 13.8 pt narrower than it should be can hold a word it should not.

**If it costs a verdict I will report the trade rather than hide it, and if it costs more than two I
will revert and report the measurement**, which is the same judgement the sheets round made on
`013_Contextures_chart_sample`.

## What this census cannot see

1. **The `.doc` and `.rtf` arms are not censused and are not implemented.** WW8 spells this as a
   `PLCF` of field characters and RTF as `\*\formfield`; neither is counted above, so "675 in 12" is
   a floor for the corpus and an exact figure only for OOXML.
2. **It cannot see a box in a part the layout never reads.** The census counts `w:checkBox` in every
   `.xml` part of the package, including ones no page draws — a `w:altChunk` body, a header for a
   section the document never enters. So the census over-reaches where the round-55 font census
   under-reached, and 675 is an upper bound on what will actually be drawn.
3. **It cannot see the ascent.** The square's *height* is asserted against the reference; where it
   sits relative to the baseline is taken from `rInf.GetAscent()` and checked only at 12 pt.
4. **It cannot predict a reflow.** Whether 13.8 pt of new width on a line moves that line's break
   depends on where the line already ended, which nothing short of rendering answers. That is why
   the page-count band is 0–3 rather than 0.
