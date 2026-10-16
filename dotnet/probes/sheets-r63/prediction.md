# Round 63 — sheets — prediction

Committed **before** anything is rendered post-change and before the change was measured against
a single corpus document. Environment: LibreOffice **26.2.4.2 620(Build:2)**,
`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`, worktree
`wt-sheets-r50`, branch `wt-sheets-r63`, base `43142b73ccf`.

**Baseline reproduced before any change: 280 of 307**, `score.py` over `MANIFEST.tsv`'s 307 sheets
paths (`TOTAL 363 MATCH 306 MISMATCH 57 REF-CANNOT-RENDER 0`; 280 match, 22 `words`,
5 `pages,words`). That is the brief's figure exactly.

## What was measured, and what is being shipped

Round 30 bracketed the axis wrap limit at **[0.990, 1.056] of the tick spacing** and shipped
1.000; round 62 argued that bracket was a measurement of `true ÷ 0.975` because the widths it was
fitted against were on an unquantised ruler, and shipped `IChartTextMeasurer.AdvanceScale` as a
stated stop-gap. **Both readings are wrong, and 328 decks say so.**

- Re-running round 30's own generator at six sizes and then sweeping the **chart frame width**
  continuously (which sweeps the tick spacing continuously, where a category count can only step)
  puts a **one-word** label's boundary at **[0.9974, 0.9988)** of the tick spacing on the
  quantised ruler, and the six per-size brackets on the **unquantised** ruler **do not intersect
  at all** — an independent confirmation of round 62's pixel-em law from a different observable.
- But that boundary is not the wrap limit. `lcl_hasWordBreak` does not turn the axis; it turns
  **line breaking off** and restarts, and the 45° follows only if the labels then collide as
  single lines. For a one-word label the two thresholds are 0.95 and 1.00 of the spacing and only
  the outer one leaves a trace, which is why round 30 could not see the inner one.
- Decks whose label carries a **space** separate them. `Middle Column` among twelve categories at
  10 pt gives **[0.9470, 0.9505)** and at 11 pt **[0.9486, 0.9524)** — both containing **0.95**,
  the source's own constant (`VCartesianAxis.cxx:753-759`), with nothing fitted. On the
  unquantised ruler the same two are [0.9713, 0.9748) and [0.9276, 0.9312) and are disjoint.
- A four-arm control varying the first word, the second word, and both, moves the boundary only
  where `0.95 × spacing` says it should.
- A deck whose only over-wide word is its **first** label stays upright, which is the C++'s
  `nTick > 0`.

**Shipped**: `Wraps` compares against `0.95 × spacing`, skips the first label, and stops counting
a trailing blank in a word's width. `IChartTextMeasurer.AdvanceScale`, `ChartText.AdvanceScale`
and the `SheetChart` override are **deleted** — the interface method and its default are gone.

## Predictions

### Sheets

| | predicted |
|---|---|
| verdicts | **278 to 281** — i.e. **at most 2 losses and at most 1 gain**, and losses are the likelier direction |
| documents whose rendering changes | **8 to 25 sheets documents**, of which I expect **more to improve than to worsen** on ink but I am not predicting a direction per document |
| `023_Waterfall_Chart_Template_for_Excel` | **unchanged, `match` 872/868.** Its widest tested word `Column` is 33.597 on chart2's device against a limit that goes 33.15 → 32.30; it wrapped before and wraps now |
| `058_Social_media_engagement_data` | **unchanged at 200/194.** Its labels are dates and `Words` splits them at `/`, so the widest run is `2023`; it exceeds both the old and the new limit. **`058` is not a `Wraps` document at all** — the reference draws ~24 where we draw 10 because of the *rhythm*, and this round does not touch that |
| the four pies (`003`/`011`/`019`/`027`) | **unchanged, word-exact.** A pie has no category axis |
| page counts on our side | **0 change** |
| direction of the change | **strictly more wrapping** everywhere: the limit falls from `spacing × PixelEmScale(size)` (0.975 at 10 pt, 1.023 at 11, 1.031 at 8) to a flat 0.95. So more axes turn 45°, and a turned axis raises our own token count because poppler splits rotated text |

### Cross track — this diff touches `XRay.Office.Core`

`ChartAxisLabels.Wraps` is shared. Slides and words keep their **unquantised** chart ruler
(round 62 left them there deliberately; it is item 6 on that round's list and is *not* done here),
so for them the limit moves from `1.000 × spacing` to `0.95 × spacing` against widths that are
2.5% too wide at 10 pt and 2.3% too narrow at 11. That is **closer to the truth than 1.000 at
every size**, and it is still wrong by the pixel em.

Census of what can move, `census-chartreach.py`'s shape re-run over all 946 manifest paths: **67
slides documents and 10 words documents** hold a text-bearing chart part. I predict **0 to 3
verdicts move on slides and 0 to 2 on words**, in either direction, and I will measure both
tracks in this worktree rather than argue about them. The parent's gate at HEAD is the authority.

### What this census cannot see

1. **Whether a chart is drawn at all.** `DrawingChartPlot.Read` returns null for several kinds, and
   a chart part that draws nothing cannot move a label.
2. **The rhythm.** Everything above is about `Wraps`. A document whose axis is thinned rather than
   turned is untouched, and `058` is exactly that case — so the brief's named witness is one I am
   predicting *will not move*, which is the prediction most likely to read as a failure.
3. **`Collides` still measures single-line boxes.** The reference wraps a label to two lines and
   collides the wrapped box; we collide the unwrapped one. Making `Wraps` fire more often reaches
   that defect more often, and I cannot predict which way it lands on a document I have not looked
   at. **This is the blind spot I expect to cost me a verdict.**
4. **BIFF and ODF charts.** The 0.95 applies to them too and no census here counts them.
5. The trailing-blank change to `Words` is supported by one deck series (`C`) and by the same
   observation that establishes the first-label skip; the two cannot be separated by that deck
   alone and I am shipping both together.

## Also measured this round and not shipped

`objectformatter.cxx:837-847` applies a **`D9D9D9` 0.75 pt chart-area border to every OOXML chart
except under the Impress filter**, and `ChartPlot.Border`'s remarks read the exception as the
rule. Census over all 946 manifest paths: **90 sheets documents / 138 parts and 10 words documents
/ 10 parts** gain a border; **0 slides**, because the exception is theirs. `pdf-ops.py` finds the
reference's own stroke on `023_Waterfall` at (68.17, 425.79)-(530.67, 755.77). Not implemented in
this round's diff, so that it cannot be confused with the wrap change; the census and the reading
are committed here so the next round starts from them.
