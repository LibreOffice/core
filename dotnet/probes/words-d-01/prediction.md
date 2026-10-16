# Words-D round 1 — prediction, written before any render of this round

Baseline: `d82acd45832`. Reference `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`,
`fonts-dejavu-core` installed, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

Subject: the **37 documents failing check 1** (29 `pages`, 8 `pages,words`). Check-2-only
failures are another agent's and are not touched.

What I have read and am not re-deriving: `words-rebase-02/results.md`,
`words-b-01/results.md`, the merge note. The comparator is 159, not 158; the environment
moved on three axes (LibreOffice 26.2.4.2, missing then installed `fonts-dejavu-core`,
poppler 26.01.0); the three large outliers are re-taken; the 720 dpi law holds; the
per-side line composition shipped and moved 37 renderings and 0 page counts.

---

## Predictions

| # | prediction | confidence |
|---|---|---:|
| P1 | My own sweep at `d82acd45832` reproduces the briefed baseline **to the digit**: 154 matching, absolute page error 117, 163 exact page counts, absolute word error 7023, 0 render failures. If it does not, I stop and find out why before anything else. | 0.90 |
| P2 | The 37 check-1 failures are **exactly** the 37 rows of `words-rebase-02/gate.tsv` with `pages` in the verdict — same documents, same ours/ref page pairs. The per-side line composition moved 0 page counts, so nothing in that set can have moved. | 0.92 |
| P3 | Over all 200, `glyphs` is again the **dominant** first-divergence kind, and it is dominant on **more matching documents than failing ones**. Band: dominant on 40–70 of the 154 matching and 18–33 of the 46 failing. | 0.75 |
| P4 | `face` is dominant on **≥3 matching and ≤2 failing** documents — i.e. the round-47-era finding that `face` is a *pass* signature survives. | 0.6 |
| P5 | Between 70 and 115 of the 154 **matching** documents have a materially divergent page at all (`ink% > 0.35` somewhere in the common prefix). The point of the control is that this number is large; if it is under 40 the "existence, not kind" finding is wrong and the kind column becomes worth reading. | 0.6 |
| P6 | **The ±1 page cluster (23 of the 37) is not one cause.** Its first-divergence pages will span ≥10 distinct page numbers and ≥3 distinct dominant kinds. Predecessor already refuted this once; I expect it to stay refuted. | 0.8 |
| P7 | The largest sub-cluster with a **shared** cause is a **page-geometry** one: documents whose first divergent page is one `pdf-image-diff.py` refuses to compare (`page size differs`), i.e. our page N and the reference's page N have different sizes or orientations. Band **4–12** of the 37. At least 3 of them will be the shape already named in `first-divergence.py`'s own docstring for `1_tpr_template__from_fy14_.docx` — **we emit an extra page immediately before a section break that changes page orientation**. | 0.5 |
| P8 | I find a cause I can pin to a **file and line** and ship a fix for it. | 0.35 |
| P9 | Verdicts moved by this round: point estimate **0**, band **0–4**. A page-geometry fix that lands would be worth 1–4; anything larger would surprise me. | — |
| P10 | **≥3 and ≤12** of the 37 page-failing documents show **no** divergent page in the common prefix at all — the extra or missing page is at the tail, so every common page agrees. These are invisible to the classifier by construction and I will count them explicitly rather than let them vanish into a "no divergence" bucket. | 0.6 |
| P11 | The 37 split 24 `docx` / 13 `doc` in `gate.tsv`; I predict the **doc** half is over-represented among the documents the classifier cannot explain (no divergent page, or a divergence the notes do not name), because no XML census reads WW8 and the two readers meet late. Specifically: ≥4 of the ≤12 in P10 are `.doc`. | 0.45 |

## What my instruments cannot see — written down in advance

1. **`first-divergence.py` compares only the common prefix.** For `A_320.doc` (141/118) that
   is 118 pages and the 23 surplus pages are never looked at. For every `+N` document the
   reference's tail is never looked at either. A defect that lives only past `min(ours, ref)`
   is structurally invisible.
2. **The `ink% > 0.35` floor.** A one-line vertical shift on a dense page can score under it,
   so the reported "first" divergent page is an upper bound on where the fault is, never a
   lower one. A document whose true fault is a sub-threshold reflow that only accumulates
   into a page break twenty pages later will be classified at page twenty.
3. **The dominant kind is a majority vote over notes on one page.** The script's own header
   records `150-5370-10H.docx` where two real `face` notes were outvoted by 700 box notes.
   Any conclusion of the form "kind K is the cause" is therefore weaker than the same
   conclusion drawn from reading one page's operators by hand, and I will not promote a
   vote to a cause without a hand reading.
4. **The kind vocabulary has no entry for "this page is missing" or "this page is extra".**
   `page size differs` is the only structural signal in it. A page break in the wrong place
   between two same-sized pages presents as `glyphs`/`size`/`box` noise on every page after
   it and names nothing.
5. **66 of 200 are `.doc`.** Anything I census over `word/document.xml` or `styles.xml` sees
   at most 134 documents and will report a reach that is a lower bound. I will say which
   half any census covers.
6. **The reference PDFs are fixed on disk.** I can measure what LibreOffice 26.2.4.2 *did*,
   and I can author new probes and render them, but I cannot re-interrogate the reference on
   the corpus documents at any other setting. Any claim about *why* the reference paginated
   as it did on a corpus document is inference unless an authored probe reproduces it.
7. **A low prediction that comes true reads as well-calibrated.** P7's band 4–12 and P10's
   3–12 are wide on purpose; if the answer lands at the bottom of a band I will say so and
   treat it as an under-reaching census rather than a hit.

## What would make me stop

- P1 failing. A baseline that does not reproduce means the environment moved again and every
  number below it is a mixture.
- Any classification run that does not also cover the 154 matching documents. The control is
  the instrument; without it the kind column is a list of what a PDF looks like, not of what
  separates a failure from a pass.
