# sheets-clip-01 — prediction, written before the reach measurement

Branch `wt-sheet-clip`, 2026-08-14. Reference binary **26.2.4.2**; banked references at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, reused. `SOURCE_DATE_EPOCH=1700000000`.

## Honesty about the order of work

This is not a pre-registration of everything below. Before writing it I had already read the two
divergent documents' PDF operators, so §1 is **established, not predicted** and is written down
here so the predictions in §2–§4 can be scored against something fixed. Nothing about reach,
regressions, verdicts or the corpus has been measured yet.

## 1. Established before this file was written (from the reference's own operators)

- `fse_identification_form.xlsx` p1: the reference emits `q 199.389 249.109 734.315 61.483 re W* n`
  around the wrapped description cell — its right edge is **933.70**. Ours emits
  `199.4457 249.0803 782.3622 61.4835 re W n` — right edge **981.81**, one column further.
  933.68 is `mnScrX + mnScrW`, the printed column block: the page's fills and rules run
  `50.343 … 883.332` wide, ending at 933.68.
- `Infotabelle_WLAN im Flugzeug.xlsx` p2: seven `kein WLAN` runs at exactly `308.75` in **both**
  renderings — the placement agrees to 0.01 pt. The reference wraps them in
  `q 50.4 188.05 285.08 597.146 re W* n`, which is the block in **both** dimensions. We emit no
  clip at all. Ink: ours reaches 361.2 pt, the reference stops at 334.8 pt.
- Those cells are `D:E` merges (`<mergeCell ref="D8:E8"/>` …) centred; page 2 prints column D
  alone, so the merged align rect sticks out past the block's right edge.

## 2. The rule I predict, and what should confirm it

> `ScOutputData::AdjustAreaParamClipRect` (`output2.cxx:2928-2954`) clamps the clip rectangle to
> `[mnScrX, mnScrX+mnScrW]` **and sets `mbLeftClip`/`mbRightClip` when it has to**. `LayoutStrings`
> computes `bHClip` *after* calling it (`:2038-2039`), so **a cell whose output area sticks out of
> the printed column block is clipped to that block even when its text fitted the room it was
> given.** The dimension that needed no clipping is then widened to the whole block
> (`:2114-2123`), which is why the emitted rectangle is never narrower than the block there.

1. An authored probe with a wide **merged** cell straddling a page's right-hand column boundary
   will show the reference clipping its ink at the block edge while its text layer keeps every
   character. *(predicted)*
2. The previous round's "it is **not** a clipping rule — 617.63 pt on a 612 pt page" rider was
   measured on the **text layer**, which a clip does not touch. I predict the same probe's *ink*
   is clipped at the block edge, and that the rider is therefore wrong about ink and right about
   text. *(predicted — this contradicts the brief's own background)*
3. A plain, unmerged left-aligned string overflowing rightwards through **empty** neighbours will
   still be clipped at the block edge, because the walk stops as soon as the missing width is
   absorbed and the absorbing column's right edge lies past the block. *(predicted)*

## 3. The change

Clip a cell's text to `[BlockLeft, BlockRight]` as well as to its output area, and turn the clip
*on* when the area sticks out of the block. No change to placement, wrapping, shortening or `###`.

## 4. Reach, predicted before measuring

| | prediction |
|---|---|
| page counts moved, of 171 | **0** — a clip is downstream of pagination |
| word counts moved, of 171 | **0** — a clip does not touch the text layer |
| gate verdicts moved | **0** |
| Fidelity, from the 30/550 baseline | 30, possibly 29 |
| documents whose page-1 ink extent moves | 15–40 of 171 |
| regressions in `sheets/batch-001…006` | 0 |

The point of the round is that **the gate cannot see it**. If a word count moves at all I have
changed something I did not mean to.
