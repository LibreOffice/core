# words-r61 — prediction 1: a table takes the leading of the paragraph above it

Committed before the change it covers. Base `3f079cea621`, branch `wt-words-r61`.
Baseline reproduced first: `batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`,
scored against `MANIFEST.tsv`'s own 337-path list → **319 of 337, zero disagreements with the
manifest's status column, document for document**.

## What was measured, before any code was touched

Round 59 left the empty-paragraph item as *"11.50 against 12.65 per empty paragraph, worth 1.15 pt
on every empty paragraph in the corpus"*, with the direction confirmed by a second instrument at
−2.06 pt on `097` page 1 and the monotonic shape refuted. **Both halves of that sentence are wrong
and the refutation is measured twice**, once on the corpus document and once on authored packages:

* On `097_Business_Case_Template` the whole 3.36 pt by which the reference's last table rule sits
  below ours is spent at **four body paragraphs that sit above a table**: +0.95, +1.00, +1.05,
  +1.00, against −0.65 at the one body paragraph holding an image. Two of the four are empty and
  two hold a `<w:br/>` — **two lines** — and the deficit is the same 1.00 on both. A per-empty-
  paragraph line-height deficit would be twice as large on the two-line ones. It is not.
* `emptypara.py`, 44 authored packages over eleven families: with `k` paragraphs between two
  single-row tables, the **slope** in `k` is 21.65 pt on both sides and the **absolute** marker
  agrees to 0.01 pt once `k ≥ 2`. Under `par-…` (a paragraph in front instead of a table) the two
  sides agree at every `k`. Our per-line and per-paragraph costs are exact; only the *first*
  transition is wrong, and only when a table is on one side of it.
* The authored `k=1` gap reproduces the corpus figure to the digit: reference 22.10, ours 21.15,
  the same two numbers `097` gives.
* `tbl-text-097` shows *where*: the middle paragraph's own baseline is 104.55 on the reference and
  104.54 here, and the table below it starts 1.01 pt lower on the reference. **The extra is below
  the paragraph, not above it.**

## The law, pinned rather than argued — `tableleading.py`, 12 packages

`SwFlowFrame::CalcUpperSpace` adds `nPrevLineSpacing` to the upper space of whatever follows;
`pOwn->IsTextFrame()` guards only the *own* term (`sw/source/core/layout/flowfrm.cxx`:1648-1740),
so a `SwTabFrame` takes it too. `GetSpacingValuesOfFrame` reads it from
`SwTextFrame::GetLineSpace()` = `GetHeightOfLastLine() × p / 100 − GetHeightOfLastLine()`.

Measured on the table's own top rule, against the reference:

| arm | measured | law |
|---|---|---|
| proportion 100 / 107.9 / 120 / 150 / 200 % | +0.00, +1.00, +2.50, +6.30, **+12.65** | `floor(H·p/100) − H` in twips, `H` = 253 |
| paragraph's own size 11 pt → 22 pt at 150 % | +6.30 → **+12.65** | scales with `H` |
| two-line paragraph, big line **last** vs **first** | 12.65 vs 6.30 | `H` is the **last** line |
| `atLeast 400` / `exact 400` | **0** handed down in both | not `SvxInterLineSpaceRule::Prop` |
| control: a 100 % paragraph between the 150 % one and the table | **0.00 pt divergence** | the leading goes to the paragraph, not past it |

The reference's own first line takes none of it (`prop-150`: its first baseline is 81.90 and ours
81.89), which is `if( !IsParaLine() )` at `itrform2.cxx`:2425 and is the half we already implement.

## Reach, from what a paragraph *resolves* to and not from a grep

`tableleading-census.py` resolves `w:spacing/@w:line` and `@w:lineRule` through the paragraph's own
`w:pPr`, then its `w:pStyle` chain following `w:basedOn`, then `w:docDefaults/w:pPrDefault`.

```
paragraph-then-table boundaries        :  1478 in 147 documents
  ... of them proportional over 100%   :   275 in  85 documents   <- the sites
sites in documents the gate calls open :    67 in   4 documents
```

**What the census cannot see, written down in advance:**

* the **66 `.doc` paths** — the WW8 reader resolves its own line spacing and none of it is counted.
  The fix is below both readers, in `PageBlock` layout, so it reaches `.doc` and `.odt` too and the
  census is an under-count by an unknown amount. This is the shape that has concealed itself twice
  in this project, and it is named here rather than after the sweep.
* whether a site is *load-bearing*: a boundary in the middle of a page moves everything below it and
  moves no verdict at all. 275 is a count of sites, not of consequences.
* whether a paragraph whose last line holds an **inline object** hands down the object's height or
  the font's. `i#47162` says `MaxAscentDescent(…, bNoFlyCnt=true)` suppresses fly portions, and
  `ParagraphFormat.Apply`'s `baseHeight` already excludes them here — so the two should agree, but
  `097`'s image boundary is the one boundary where we are currently 0.65 pt **too tall**, and this
  change adds 1.00 pt to it in the wrong direction.
* header and footer flows, which `FlowLayouter` reaches and which no gate column can see.

## The prediction

| quantity | baseline | predicted |
|---|---:|---|
| **words verdict** | 319 of 337 | **320**, band 318–320 |
| `097_Business_Case_Template` pages | 1 against 2 | **2 against 2 — closes** |
| `012_Project_Timeline` / `015_Project_Timeline` pages | 1 against 2 | **unchanged at 1** |
| words renderings whose bytes change | — | 85 – 150 |
| words renderings whose **page count** changes | — | 1 – 5 |
| extractable words changed | — | 0 – 2 documents |
| font lists changed | — | 0 |
| slides / sheets | 200 / 279 | **unchanged by construction** — the diff is confined to `XRay.Office.WordProcessing/Layout`, which neither track compiles against a code path of |

`012` and `015` are predicted **not** to move, and that is a deliberate break with the brief, which
put all three in one class. Their reference page 2 is not a trailing empty paragraph: `012` draws a
white cell fill and a grey rule there and `015` draws five white rules, i.e. **a positioned table's
last row broken onto a second page**. Their tables carry `w:tblpPr` and are 767.25 pt wide on a
648 pt text area; `PlaceFloatedTable` refuses to split a floated table at all. That is a different
defect and this change cannot touch it.

## Downside and what would falsify this

The band's lower end is −1, not 0: 81 of the 85 documents with sites currently **pass**, and a
document with 48 sites (`OM template for non-complex NCC operators`) accumulates 48 pt — most of a
line and a half — which can cross a page boundary in either direction. `EHEST` and the three
`metrics-001` documents are the ones to watch.

The instrument that would refute this round's story: if `097` closes but the **four boundary
deltas** do not go to zero, the 1.00 pt was fitted rather than derived, and the round's law is
wrong even though its verdict moved.

# words-r61 — prediction 2 (conditional): an `atLeast` line loses its raise on a first line

Written with prediction 1 and committed with it, and it will only be implemented if prediction 1's
sweep is clean, because the two changes must be separable in the measurement.

`tableleading.py` arm 4 measured a second, independent defect. `SvxLineSpaceRule::Min` — OOXML's
`atLeast` — is applied at `itrform2.cxx`:2397, **outside** the `if( !IsParaLine() )` guard at :2425,
so it raises every line including a paragraph's first. `ParagraphFormat.Apply` puts the whole raise
into the line box's `SpaceAbove`, and `ParagraphLeading.AsDrawn` strips `SpaceAbove` from a
paragraph's first line and a frame's first line because that is where *proportional* leading lives.

Measured: `w:line="400" w:lineRule="atLeast"` on 11 pt Cambria — the reference draws a **20.00 pt**
line with its baseline at 89.25 (7.35 pt of raise above the text) and we draw **12.65 pt** with its
baseline at 81.89. `exact 400` agrees on both sides (88.00 against 87.99), which is the control that
says the defect is `Min` and not stated line heights in general.

`atleast-census.py`: **1 569 paragraphs in 29 documents** resolve to `atLeast`, of which **675 in 16
documents** state more than 253 twips — the natural line of 11 pt Cambria/Caladea, a crude threshold
and an over-count for any paragraph in a smaller face. `EHEST-SMS-Safety-Management-Manual-V2.docx`
is **295 of those and is open at 80 pages against 82**, i.e. short in the direction this defect
predicts. Blind spots: the threshold is not the resolved natural height, `.doc` is not counted, and
a cell paragraph counts the same as a body one.

Predicted, if implemented: **words 320–322**, with `EHEST` the candidate and `hdss-bulletin`,
`easa-regulations-update-20`, `easa-form-1` the regression risks.
