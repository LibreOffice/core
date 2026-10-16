# Words-E round 1 — prediction, committed before any measurement

Baseline `8a8194c517c` (words-d merged; `git merge-base --is-ancestor 5c34499a205 HEAD`
succeeds, so the sloppy-fit page geometry fix **is** in my baseline and is not re-openable
work). Worktree `/c/sandbox/workdir/wt-words-e`, branch `wt-words-e`. Reference
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`. `check-env.sh` green: LibreOffice
26.2.4.2 620(Build:2), Calibri→Carlito, Cambria→Caladea, DejaVu Sans→DejaVu Sans,
pdftoppm/pdftotext 26.01.0.

Written before the first render of this round, before any per-document figure of this
baseline was read, and before opening `word/document.xml` on any of the three landscape
documents.

---

## A. What my instruments cannot see — named first, not after

1. **Any XML census reads the 134 `.docx` and is blind to the 66 `.doc`.** All three
   documents in the landscape sub-shape are `.docx`, so a sub-shape census done in XML
   would be a census of two thirds of the corpus that *looks* complete. I will therefore
   run the orientation/shape census off the **PDF media boxes** on both sides, which is
   available for all 200 equally, and use XML only to explain a document already found.
2. **`first-divergence.py` cannot see an inserted page at all.** It has no kind for "this
   page is extra" (words-d §5.4), and its 512 px raster puts ~1.6 pt on one A4 pixel. An
   extra page presents to it as `glyphs` noise on everything after it. It is the wrong
   instrument for this subject and I will not lean on it for the landscape question.
3. **A blank-page probe on an authored DOCX proves what 26.2.4.2 does with *my* markup**,
   not what the three corpus documents contain. The bridge from probe to corpus is a
   separate measurement and I predict below which way it goes.
4. **Line counting via `pdftotext` is an extraction proxy for layout.** "37 lines vs 33"
   is a claim about laid-out lines; extracted text lines can merge or split runs. I will
   cross-check any line-fit claim against `pdftotext -bbox`/`-layout` y-coordinates rather
   than raw line counts alone.
5. **The C++ tree here is 27.2.0.0.alpha0+ and made none of the references.** Every rule
   below is to be measured on the installed 26.2.4.2 first and attributed to source after.
6. **Three documents is three documents.** Two of them (`template---tpr…`,
   `1_tpr_template…`) are two revisions of one TPR template, so the effective independent
   n for the sub-shape is **2**, not 3. Any rule fitted to it is fitted to 2 points.
7. **`template---tpr…docx` is one of the six documents whose *reference* page count moved
   with the 24.2.7.2 → 26.2.4.2 binary change** (`words-rebase-02` §5): it was 8/8 and is
   8/7 because the reference lost a page, not because we gained one. So on that document
   the "extra page" is partly a statement about the reference.

---

## B. Predictions

### The baseline

| # | prediction |
|---|---|
| P1 | The baseline reproduces to the digit: **154** matching of 200, absolute page error **117**, exact page counts **163**, **0** render failures. (Absolute *word* error will **not** be 7023 — the gate's word check changed — and I make no prediction on its value.) |
| P2 | The 37 check-1 failures are exactly `words-d-01/gate-after.tsv`'s, same page pairs, `diff` on page and verdict columns empty over all 200. |

### The landscape sub-shape

| # | prediction |
|---|---|
| P3 | The three page-shape sequences reproduce exactly: ours `A6BA` / `A3BA4B` / `A40B3A5`, reference `A5BA` / `A2BA4B` / `A39B3A5`. |
| P4 | **The extra portrait page is overflow, not an inserted break.** On all 3 of 3, the last portrait page before the first landscape run in our render carries real text that the reference has fitted on an earlier page — i.e. our portrait run is content-shifted by roughly one page's worth of lines, not padded with a blank or near-blank page. Point estimate **3 of 3 overflow, 0 of 3 blank**. Band: 2–3 overflow. |
| P5 | **Therefore the extra portrait page on `template---tpr…docx` and its page-2 "reference fits 37 lines, we fit 33" defect are ONE defect, not two.** Concretely: the accumulated line deficit over the portrait run before the first landscape section is ≥ one page's worth of lines. If P4 holds and P5 fails, I will say so. |
| P6 | **None of the three uses an `evenPage`/`oddPage` section break at the portrait→landscape boundary.** Point estimate 0 of 3. (If one did, LibreOffice's parity filler page would be the whole answer and this would be a two-hour round.) |
| P7 | The authored probe will find **26.2.4.2 emits no extra page for a `w:type=nextPage` section break to landscape**: N portrait pages of content followed by a landscape section gives exactly N portrait pages, for at least two different N (the slope is 1, intercept 0). |
| P8 | **26.2.4.2 promotes a `w:type=continuous` section break to a new page when the page size or orientation changes across it.** This is the classic Word behaviour and I expect LibreOffice to match it. |
| P9 | **We do not implement P8's promotion** — a `continuous` break with a changed orientation lays out on the same page in our renderer. Stated as the most likely *implementation gap*, at maybe 50 % confidence; if we already do it, P9 is refuted and P4 carries the round alone. |
| P10 | Whatever the landscape rule turns out to be, it is **not** the cause of the three documents' extra page. Point estimate: the rule is implemented correctly (or its gap is unreachable by these three), and the cause is P4's overflow. |

### Reach and movement

| # | prediction |
|---|---|
| P11 | **Verdict movement from anything shipped this round: point estimate 0, band 0–3.** Two consecutive rounds shipped correct fixes and moved 0 and +1; the prior on a page-flow fix moving a verdict is low, and a line-fitting fix that closes a 4-line-per-page deficit would have to be right on the *first* page to change a count. |
| P12 | If a `continuous`-with-changed-geometry fix ships, its corpus reach is **1–15 of 200 renderings changed** and 0–2 verdicts. |

### The page-1 divergence concentration (only if the above closes early)

| # | prediction |
|---|---|
| P13 | **Refuted again.** The 17 page-failing documents whose first divergence is page 1 will show **≥4 distinct proximate causes** and no property that separates them from the 43 *matching* documents that also diverge on page 1. Two rounds have claimed a shared cause here and both were refuted; the base rate says predict refutation. |
| P14 | Running any page-1 classifier over the 154 matching documents will show the same distribution shape as over the 37 failures, as it did for kind and for first-divergence page in words-d §5. |

### Scored honestly

I expect to refute at least 3 of P1–P14, and I expect at least one of them to be a
prediction of my own brief's framing rather than of my own reasoning. The last four words
rounds refuted 5 of 11, 7 of 12, 3 and 4 of 6.

---

## C. What would make this round a result even at zero verdicts

An **exclusion**: if the landscape rule is measured on 26.2.4.2, our implementation is
checked against it point for point, and it agrees, then section-break page emission joins
page geometry on the excluded list and the sub-shape is re-attributed to the flow — which
is where words-d said the cluster lives. That is the outcome I actually expect (P10), and
it is worth writing down *before* measuring so that it does not read afterwards as an
excuse for not having found a bug.
