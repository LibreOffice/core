# words-r53 — prediction

Committed **before anything was changed**. Environment: LibreOffice **26.2.4.2 620(Build:2)**,
`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, `fc-match Calibri` → `Carlito-Regular.ttf`,
worktree `wt-words-r50` on branch `wt-words-r53`, base `41445736a8c`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, reproduced first

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 335  MISMATCH 20`. Scored against
`MANIFEST.tsv`'s 337-path list rather than that total: **318 match, 19 open, 0 disagreements with
the manifest's status column, document for document.** The briefed baseline reproduces exactly.

## The change

The `w:br`-only paragraph. **The brief's observable holds and its stated reach does not.**

### Mechanism, established rather than assumed

`TextItemiser.AddWithoutControls` cuts every `IsFormatControl` character out of the item list, and
U+2028 — what all four word-processing readers and both DrawingML readers emit for a manual line
break — is in that set. `MeasuredParagraph.Measure` builds its runs by intersecting each
`FormattedRun` with those items, so **a paragraph whose whole text is breaks reaches
`MeasuredParagraph` with `_runs.Length == 0`**. `MeasureLine`'s three fallbacks then all miss: the
fold finds no run, the blanks-refold finds no run, and the last one — the empty-paragraph case,
`if (height == Length.Zero && _runs.Length > 0) Accumulate(_runs[0])` — is guarded on there being a
run. Every line of the paragraph comes out **0 pt tall**.

Measured directly, by laying the r52 probe documents out through `DocxLayoutSource` +
`ParagraphLayouter` and reading the boxes:

| document | paragraph text | measured runs | per-run path | single-face path |
|---|---|---:|---|---|
| `a-br` | `U+2028` | **0** | 2 lines, **0 pt** | 2 lines, 23.10 pt |
| `i-spacebr` | `SPACE U+2028` | 1 | 2 lines, 23.00 pt | 2 lines, 23.10 pt |

That is the whole of the brief's "one space is enough": a space is not a format control, so it
leaves a run behind and the fallback fires. `TextMeasurer` is **not** the seat — it returns the
right two lines in both cases, exactly as r52 said. The comment on `IsFormatControl` names this
defect in its own words ("a paragraph that is nothing but control characters then has no run and
no line") and moved the C0 range out for precisely this reason without moving U+2028.

### The fix

`MeasuredParagraph.Measure`: when every formatting run has been itemised away, keep one
zero-length `MeasuredRun` from the first formatted run so the paragraph still has a face and a size
to be as tall as. A zero-length run is invisible to `Fold` (`touches` and `contains` are both
false), invisible to `RunsBetween` (`to <= from`), and adds nothing to the prefix table, so it can
only be seen by the `_runs[0]` fallback — which is the empty-paragraph rule, and an all-break
paragraph is exactly as tall as the same number of empty ones in the reference.

## Reach — the brief's figure is refuted and the corrected one is 20× smaller

The brief and `words-r52/results.md` say **469 such paragraphs in 66 of 271 documents**, naming
`FAA 2025-26 Holdover Tables` (66), `24-25_FAA_Holdover_Tables` (58), `OM template …` (37) and
`EHEST-SMS` (35), and call that "the risk, and why this needs a whole-track sweep". **No census
committed with r52 produces those numbers and none I can construct reproduces them.** The script
beside this file counts, over `MANIFEST.tsv`'s own path list:

| reading | paragraphs | documents |
|---|---:|---:|
| `w:p` holding any non-page `w:br` | 3936 | 76 |
| …and no non-empty `w:t` | 23 | 14 |
| …and no tab, symbol, field, note or **as-character** drawing either — the defect | **22** | **13** |

The named witnesses do not survive: `FAA 2025-26 Holdover Tables` holds **1**, not 66;
`24-25_FAA_Holdover_Tables` **1**, not 58; `OM template` **1**, not 37 (it holds 37 `w:br`
*elements*, which is the likeliest source of the figure); and `EHEST-SMS` holds **0**, not 35 —
its 10 breaks all sit in paragraphs that also hold text. Every one of the 22 is a **single**
break, so each is worth two lines rather than more.

`097` holds **3**, which is the one figure of r52's that does reproduce.

Per document, with the manifest's status:

```
7  done  words/chartset-012/docx/096_Business_Case_Template_Editable_Layout_41fe0bc0.docx
3  open  words/chartset-006/docx/097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx
2  open  words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx
1  done  words/done-003/docx/Technical_Issue_Report_Form.docx
1  done  words/done-015/docx/Agile_Arc_SysDes.docx
1  done  words/done-015/docx/system_design__technical_architecture_template.docx
1  done  words/extra-001/docx/ABCD-SDE-23-00 - Avionic System Description - 17.02.16 - v1.docx
1  done  words/extra-001/docx/ABCD-WB-08-00 Weight and Balance Report - v1 08.03.16.docx
1  open  words/metrics-001/docx/OM template for non-complex NCC operators_August 2016.docx
1  done  words/pagination-001/docx/24-25_FAA_Holdover_Tables.docx
1  done  words/pagination-001/docx/ESPN-R - MCF - RA - Ed1.docx
1  done  words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx
1  done  words/pagination-001/docx/template---tpr-technical-progress-report-with-guidance.docx
```

**The whole-track sweep is still run**, because the change is in `XRay.Office.Text` and a census over
parts is exactly the instrument HANDOVER §7 says under-reaches.

## What the census cannot see, written down before the sweep

* **`.doc`, `.rtf` and `.odt`.** Their readers emit the same U+2028 (`Ww8DocumentReader.Layout.cs`
  :1187, `RtfDocumentReader.State.cs`:945, `OdtLayoutSource.cs`:51) and the census reads none of
  the three. The words figure is a **floor**, and 47 of the 337 words documents are binary or RTF.
* **Paragraphs of some *other* control character** — a lone U+200E, U+2060, U+FEFF. Same defect,
  not counted, and not measurable from a `w:br` census at all.
* **Whether the paragraph takes the per-run path.** The single-face path measures these correctly
  today, so a reader that emits no runs is unaffected. DOCX always emits runs; the other three
  readers were not checked.
* **Inheritance and defaults of every kind.** This is a shape census over parts.
* **A page-break cascade.** Each fix adds ~23 pt to one paragraph; on a document with section
  breaks that can move a page boundary a long way from the paragraph, which is the mechanism
  `CLAUDE.md` records for `AWR OPS-AOC 044`. Nothing in the census sees it.

## Verdict movement predicted

**318 → 319**, one gain, no regressions.

| document | now | expected | why |
|---|---|---|---|
| `097_Business_Case_Template_Elegant_Layout` | `pages` 1/2 | **`match`** | r52 localised it as 20 pt short of a second page; two of its three break-only paragraphs are on page 1 and each is worth ~23 pt |
| everything else | unchanged | unchanged | 20 of the 22 paragraphs are one per document |

Named risks, each of which would be a **regression** and is stated so it cannot be netted away:

* `096_Business_Case_Template_Editable_Layout` — 7 occurrences, currently `match` at 1 page.
  ~161 pt of new height is enough to open a second page. **This is the most likely regression.**
* `ABCD-FE-01-00 Flight Envelope` — currently `pages,words` 14/15; two occurrences, ~46 pt. Could
  close, could stay, could go 16/15.
* `OM template for non-complex NCC operators` — currently `pages` 166/165, one occurrence. One
  more line in a 166-page document should not move a page count, but it is the document r52's
  own line-height work already moved and it is one line from a boundary.
* `FAA 2025-26 Holdover Tables` and `24-25_FAA_Holdover_Tables` — one occurrence each, both
  `match` on large page counts. The census says they are *not* the heavy witnesses the brief
  named; if either moves a page, the brief's reach claim was right by some route this census
  cannot see, and that is the tell to look for.

## Cross-track reach — measured, not argued

`XRay.Office.Text/Layout/MeasuredParagraph.cs` is a **shared layer** and is reached by
`SlideTextLayout` and `SheetTextLayout` as well as `PageContent`. The same census over the other
two families:

* **slides: 17 paragraphs in 7 decks**, 6 of them passing —
  `ghgp-supply-chain-initiative_20100323_wri.pptx` (4), `10. Drawings, circuit diagrams and
  schematics.pptx` (4), `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` (3, open),
  `Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx` (3),
  `John_Broggio__RSS_Campion_event_John_Broggio.pptx`, `1_Country-Updates_DRC_English.pptx`,
  `B2B-Center-Readiness-and-Student-Retention.pptx` (1 each).
* **sheets: 0.** No `a:p` in any `xl/drawings` part is break-only.

A slide's text box is not paginated, so the expected slides movement is **ink, not verdicts**: a
break-only paragraph inside a body currently collapses to nothing and will start taking a line,
which moves everything below it in that box. `.ppt` and `.odp` are not read by the census.
**Predicted slides verdict movement: 0.** The five batches holding those seven decks are swept
both ways from here; the parent owns the full cross-track sweep.

## Tests

Every new test to be run through `verify-test.sh` by reintroduction — the mutation being the
restoration of the `_runs.Length > 0` guard with no run kept.

---

# Second prediction — item 2, the frame anchor's font size

Committed **before the second change and before anything was rendered against it**, and after the
first change's sweep, whose result is in `results.md`. The first change measured **+1 `097` and
−1 `096`, net 318**, which is what the first prediction named as its most likely regression.

## What `096` and `097` now have in common

With the break-only paragraphs fixed, `097`'s page-1 rows sit at a **uniform +34 pt** below the
reference's — the two 27 pt deficits are gone and one discrete excess is left, in the gap between
`Document Control` and `Document Information` where r52 localised it. `096`'s rows now carry the
reference's *pitch exactly* (59.2, 59.2, 59.3, 59.2, 47.6, 47.6, 59.3 — the reference's figures to
the tenth) and are pushed down by one 15 pt excess between `Email Address` and `Phone Number`.
Both are one place where we are too tall, not a drift.

## Mechanism, measured on ten authored variants

Cutting `097`'s block-1 paragraph five ways and rendering each both ways gave the first cut:

| variant | reference | ours | Δ |
|---|---:|---:|---:|
| as is (anchored drawing + `w:br`) | 136.4 | 173.6 | **+37.2** |
| the `w:br` removed | 122.7 | 141.3 | +18.6 |
| the drawing removed | 136.4 | 137.8 | +1.4 |
| **the run's `w:sz w:val="52"` removed, nothing else** | 136.4 | **137.8** | **+1.4** |

So the whole of `097`'s remaining error is one `w:sz` on a run whose only content is a drawing.

Pinned properly, ten variants of the same paragraph — anchored against as-character, 10 pt against
26 pt, alone against with text — reading the height the paragraph adds over an empty one:

| case | reference | ours |
|---|---:|---:|
| a run of text at 26 pt | 20.60 | 19.10 |
| **anchored** drawing, run at 10 pt | 0.00 | −1.10 |
| **anchored** drawing, run at 26 pt | **0.00** | **17.25** |
| anchored drawing at 10 pt, text beside it | 0.00 | 0.00 |
| anchored drawing at 26 pt, text beside it | **0.00** | **17.25** |
| **as-character** drawing, run at 10 pt | 7.00 | 6.95 |
| **as-character** drawing, run at 26 pt | **7.00** | **17.25** |
| as-character at 10 pt, text beside it | 9.70 | 9.70 |
| as-character at 26 pt, text beside it | **9.70** | **17.25** |

**The reference's answer does not depend on the run's size at all** — 0.00 at both sizes anchored,
7.00 at both sizes as-character, 9.70 at both with text. Ours is the run's size in every row where
it is large. Where the run's size happens to match the paragraph's we already agree, on all four of
those rows, which is why this has never shown up as a systematic error. It is Writer's model
exactly: a fly is a portion of its own and a run holding no text contributes no text portion, so
its font never reaches `SwLineLayout::Height`.

## The fix

`PageParagraph.Measure`: a `PageRun` whose whole range is anchor characters is measured in the
**paragraph's** face and size rather than its own. It keeps its own everything for drawing — the
rewrite is on the measurement half only — and an anchor sharing a run with real text is untouched.
That covers the as-character case too, where the picture's own height then decides, and the
comment mark, which is the third thing U+0001 stands for and is likewise not text.

Words-layer, not shared: `PageContent.cs` is `XRay.Office.WordProcessing`. The four
word-processing readers all use the same U+0001 convention so all four are served.

## Reach

`anchor-run-size-census.py`: **85 anchor-only runs stating a size, of 324 anchor-only runs, in 40
of the 337 words documents** — 38 of the 40 currently passing. What it cannot see is in the
script's own header: it does not resolve styles, so a stated size equal to the paragraph's counts
here and will not move (an **upper bound**), while a size arriving from a character style is
missed (a **floor**); `.doc`, `.rtf` and `.odt` are not read at all; and nothing here says whether
the run's line holds something else that is taller anyway.

## Verdict movement predicted

**318 → 320**: `097` and `096` both to `match`.

* `097` — the only error left on it is this one, worth ~34 pt of the ~34 pt it is out by.
* `096` — the regression the first change caused, worth ~19 pt of overshoot, and it holds two of
  these runs.

Named risks, stated so they cannot be netted away:

* **38 passing documents hold one of these runs.** A page-exact document whose picture run is
  oversized gets *shorter*, and shorter can lose a page as easily as taller gains one.
  `090_Business_Case_Template_Blue_Theme` (9 runs), `HC-Bulletin-template` (8),
  `t_TEMPforInvProgs` (5) and `ESPN-R - MCF - RA - Ed1` (5) are the four to watch.
* `EHEST-SMS` (open, 80/82 pages) holds one; it could move either way.
* The direction is always "we get shorter or stay the same", never taller, since the paragraph's
  size replaces a run size that is only ever *different* — usually larger, because that is what a
  logo or a signature block states.
