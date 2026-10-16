# slides-r53 — prediction

Committed **before** anything is rendered post-change. Base `41445736a8c`, branch
`wt-slides-r53`, reference **LibreOffice 26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## The baseline, reproduced first

Whole-track sweep at the base commit, `ours` re-rendered and the reference PDFs reused from
`scratch-r52-slides/ink-after/ref` (the reference cannot move while nothing touches `soffice`):

- sweep's own `TOTAL` 311 rows, **MATCH 201** — the case-insensitive mount's 9 alias spellings.
- scored over `MANIFEST.tsv`'s 302 paths: **199 of 302 passing, 0 disagreements** with the
  manifest's `status` column, document by document, case-folded.
- `abs_ink` **1238.56**, signed **913.58**, major pages **434** over 302 documents.

Round 52 reported 1238.64 / 913.80 / 434. The 0.08 of `abs_ink` between them is the words
track's `DrawingChartPlot.cs` change, merged after r52's slides leg and named in the r52 merge
note as moving exactly one slides row (`done-011/171128IPAP.pptx`).

## What this round changes

**One reader defect, in `PptTextBody.Runs`, found by measurement rather than by reading.**

`Runs` walks the character-property runs accumulating `position`, records `atStart` when
`start >= position && start < runEnd`, and breaks when `position >= end`. For an **empty**
paragraph `start == end`, so the loop breaks at the end of the run that *ends* at `start` — one
run before the run that *contains* `start`. `atStart` is therefore never found for any empty
paragraph except one at text position 0, and the blank line falls back to the master level's
character height.

### The measurement that found it, on `ITE106-Chapter 4.ppt` p7

Round 52 left this page with "our paragraph space is 1.9× the reference's". Reading the two
content streams' baselines:

| | ours | reference |
|---|---:|---:|
| body `/Tf` | 21.9969 | **24.009** |
| baseline pitch inside a paragraph | 23.754 | **28.800** |
| baseline gap across a paragraph boundary | **68.769** | **52.214** |

The gap decomposes as `h(last line) + marginTop(empty) + h(empty) + marginTop(next)` — the
ascents cancel. The reference's is `28.800 + 3.004 + 14.402 + 6.008 = 52.212`, and **14.402 is
1.2 × 12.0**: its blank line is 12 pt. Ours needs `h(empty) + marginTop(empty) = 39.615`, which
is `1.2 × round(32 × 0.925) × 0.9 + 32 × 0.25 × 0.9 = 39.60` — **a blank line of 32 pt.**

Independently, LibreOffice's own flat-ODF export of the deck gives the four blank paragraphs
`fo:margin-top="0.106cm"` (= 3.004 pt = `12 × 20/80`) against the text paragraphs'
`"0.212cm"` (= 6.008 pt = `24 × 20/80`), so the reference resolves those blank lines at 12 pt
by a second, unrelated route.

And the record says 12 directly (`ppt-style-dump.py`, a Python parser that shares nothing with
the C# reader). Slide 7's body text is 537 characters with `\r` at 116, 117, 237, 238, 392, 393
and 536 — **paired** returns, so every bullet is followed by a genuinely empty paragraph — and
the character-run array is `117@24, 1@12, 120@24, 1@12, 154@24, 1@12, 143@24`. The empty
paragraphs start at 117, 238 and 393: exactly the three one-character runs stating **12**.

### Reach — a census, and what it cannot see

`empty-paragraph-census.py` over all 51 `.ppt` documents in the slides corpus:

| | |
|---|---:|
| empty paragraphs | 872 |
| … at text position 0 (`atStart` already found today) | 104 |
| … **`atStart` never found** | **768** |
| … of those, covering run **states a font height** | **673** |
| … covering run states none (no change) | 84 |
| … no covering run at all | 11 |
| **documents holding at least one** | **26 of 51** |

**What the census cannot see, stated before the measurement:**

1. **It does not know the master level's default.** A blank paragraph whose covering run states
   the same height the level already gave it changes nothing. So 673 is an upper bound on
   *changes* and the true number could be much smaller. The stated heights run 2…48 and cluster
   at 12/14/16/18/20/24/27/28, which is a wide enough spread that many must differ, but that is
   an inference and not a measurement.
2. **37 style atoms in the corpus did not parse** in the Python probe (an optional field runs
   off the end of the buffer). Those shapes are invisible to the census, so the document count
   is a **lower** bound.
3. It cannot see whether a height change flips the autofit table's row. That is the amplifier
   round 52 named — a twelve-row table quantises a height error into a whole step — and it is
   the reason a small reader fix can move a whole `/Tf` size.
4. It says nothing about the `.pptx` and `.odp` readers, which build their paragraphs from
   markup and do not go through this loop. They are expected to be untouched, and the sweep is
   what will say so.
5. It cannot see documents where the blank line's height is *already* right for a different
   reason — e.g. where the level default happens to equal the run's height.

## The predictions

1. **Verdict movement: 0.** 199 → 199. The gate reads page count, extractable words and font
   embedding; a blank paragraph carries no words and no slide is added or removed. I will be
   surprised by anything outside **−2 … +2**, and 0 is the expected answer.
2. **Page counts: 0 of 302 move.**
3. **Known-answer check, `ITE106-Chapter 4.ppt` p7, stated to the digit before the change:**
   body `/Tf` **21.9969 → 24.009**, intra-paragraph pitch **23.754 → 28.800**, inter-paragraph
   baseline gap **68.769 → 52.212 ± 0.05**. If the fit does not come off its scaling row this is
   falsified outright, and the fix is at best partial.
4. **Renderings that move: between 20 and 26 documents**, all `.ppt`, drawn from the census list.
   Nothing outside `.ppt` should move at all; a `.pptx` or `.odp` that moves refutes the claim
   that the change is confined to `PptTextBody`.
5. **`abs_ink`: down.** `ITE106` alone is 19.22 today. Point estimate for the track **−15 to
   −45**; anything positive is a regression to be named rather than netted.
6. **`tf-agreement.py` mean rises**, and specifically **at least 3 of round 52's 8 regressed
   documents improve** — `Lepore.ppt`, `gfopportunitiesforlinkagespres_2010_en.ppt`,
   `FAA_Form_337.ppt`, `joint_user_outcomes…ppt` and `WC_Update-Aug03.ppt` all appear in the
   census with 13, 19, 3, 2 and 10 affected blank paragraphs. `ws_prod-g-doc-Events-r-6.-ESM`,
   `010605Vul` and `EG1_dsrc tech` do **not** appear, so those three are predicted **unmoved**
   by this change — which is the control that separates "the fix works" from "everything moved".
7. **Word counts may move in both directions.** A `/Tf` change re-wraps, and `pdftotext`
   re-infers word boundaries from geometry. Round 52 moved 8 and 5 of them were worse.

## Also in this round, and predicted separately

- **`8_P-Pavese_AIRBUS…pptx`'s table fills — predicted already drawn, i.e. the brief's item 3 is
  stale.** Recorded here before the sweep so that the refutation is dated. Its `abs_ink` is
  47.76 over **26 pages with only 1 major page**, which is not the shape of 55 missing fills.
- **`SlideChart`'s `\n`-separated data label.** Predicted to be a *placement* defect rather than
  the words track's run-fusing one, because `SlideChart.Text` routes the label through
  `SlideTextLayout`, which breaks on `\n` — but measures the label's width as the sum of *all*
  lines' advances and lays every line out `TextAlignment.Start`.
- **The 24.2.7.2 audit, `SlideTextLayout.cs`'s six sites.** No prediction of the outcome is made;
  the point of a re-check is that its answer is not known. What *is* predicted is that none of
  the six is item 1's answer — item 1's answer is in `PptTextBody`, and it was found before the
  sites were re-checked.
