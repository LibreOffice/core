# words-b010-01 — prediction, committed before the fix was written or measured

Round brief: `words/batch-010/docx/5709.16 ch.40_mgfinal.docx`, 31 pages against 32, words
9790/9821 (inside the 2%+3 band), verdict `pages`.

Everything below was written after the diagnosis and **before** any code was changed, any sweep
run, or any page looked at.

## What kind of round this is

Neither of the two ways the brief warned the measurement might lie:

* **Not a non-deterministic reference.** The same `soffice` 26.2.4.2 converted the document five
  times in the same configuration: 32 pages and a byte-identical text layer on all five
  (`md5(pdftotext) = e2e27c575116` ×5). The banked reference agrees at 32.
* **Not the `1447.doc` line-height residue.** That residue is a per-line deficit that accumulates.
  This one does not accumulate at all: every body line on every page sits **exactly 11.00 pt**
  higher in ours than in the reference — 123.81 against 134.81 on pages 3 and 4, 123.69 against
  134.69 on page 2 — and the line *pitch* inside the body is identical on both sides. One step, at
  the boundary between the running head and the body, repeated unchanged on all 31 pages.

It is a real pagination defect, and the cause is the header's height.

## The measurement the diagnosis rests on

The header of this document is a **positioned (floating) table** —
`<w:tblpPr w:leftFromText="187" w:rightFromText="187" w:bottomFromText="403" w:vertAnchor="text"
w:tblpXSpec="center" w:tblpY="1"/>` in `word/header2.xml` — followed by one empty
`FSMHeader` paragraph (Arial 8 pt, so a 9.20 pt line).

LibreOffice makes that table a **frame**: `soffice --convert-to fodt` writes the header as a single
`text:p` holding a `draw:frame` whose `fr1` style carries `fo:margin-bottom="0.2799in"` — 403 twips,
i.e. `w:bottomFromText` — around a `draw:text-box` holding the table.

Five perturbations of that flat XML, each re-rendered by the installed 26.2.4.2, give the law
(the figure is the `yMin` of "Table of Contents", the first body line of page 2):

| variant | body top | pages |
|---|---:|---:|
| unchanged | 134.69 | 32 |
| frame `fo:margin-bottom` 0.2799in → **0** | **114.54** | **31** |
| frame `fo:margin-bottom` 0.2799in → 1in | 186.54 | 35 |
| frame `fo:margin-top` 0 → 1in | 134.69 | 32 |
| anchor paragraph 8 pt → 20 pt | 134.69 | 32 |
| anchor paragraph 8 pt → 60 pt (2 lines, 138 pt) | 174.39 | 34 |
| table row min height 0.691in → 1.5in (+47.46) | 182.09 | 35 |

Read off: the body top moves **one for one with the frame's lower spacing** (0 → 114.54,
0.2799in → 134.69, 1in → 186.54: exactly +20.15 and +72.00), does not move with the frame's *upper*
spacing, and does not move with the anchor paragraph until the paragraph is taller than the frame,
at which point it takes over. The anchor paragraph's own text is drawn at `yMin` 36.26 — the very
top of the header, **overlapping** the frame rather than being pushed below it.

> **A positioned table in a running head is a frame, and the head's height is
> `max(in-flow content height, frame bottom + the frame's lower spacing)`.**

Ours stacks it: header height = table height (78.10) + empty paragraph (9.20) = 87.30, against
LibreOffice's `max(9.20, 78.10 + 20.15)` = 98.25. The difference is **10.95 pt**, which is the
11.00 pt step measured on the page. `w:bottomFromText` is read nowhere in the tree
(`grep bottomFromText src/` is empty), and `DocxLayoutSource.Tables.cs` says outright that only the
horizontal half of `w:tblpPr` is honoured.

## What will be changed

1. `w:bottomFromText` read into `PageTable` as the positioned table's lower spacing.
2. In `FlowLayouter` — which lays out headers, footers and cells, never the body — a positioned
   table does not advance the flow. It is placed where the flow has reached, and the flow's
   `Advance` becomes `max(stacked height, that table's bottom + its lower spacing)`.

The body (`Paginator`) is deliberately **not** touched: in the body Writer's fly does wrap its
anchor text, which is what our in-flow stacking already approximates, and no measurement was taken
there. 21 corpus documents have a positioned table in the body and 4 in a header or footer.

## Predictions

| # | prediction | confidence |
|---|---|---|
| P1 | `5709.16 ch.40_mgfinal.docx` goes 31 → **32** pages and its verdict `pages` → `match` | high |
| P2 | Exactly **4** documents' renderings change — the four with a positioned table in a header or footer: `5709.16 ch.40_mgfinal`, `PAT-047 - Architecture and Detailed Design Assessment`, `HC-Bulletin-template`, `CRIF - Spécification technique - Socle applicatif`. No other document moves a byte. | high |
| P3 | Whole words track: match **158 → 159** | medium-high |
| P4 | Page-exact **166 → 167**, total absolute page error **113 → 112** | medium |
| P5 | `PAT-047` (4/4, `match`) stays `match`. Its header table is `vertAnchor="page"` with no `w:bottomFromText`, so its head loses the empty paragraph's height and gains nothing; the page count is expected to survive it. | medium |
| P6 | `HC-Bulletin-template` (5/5, `match`) stays `match`. Same shape, in a footer. | medium |
| P7 | `CRIF …` stays a mismatch. It is 33/29 today and its reference header is ~68 pt taller than ours — this fix does not reach that. | high |
| P8 | `XRay.Office.Fidelity.Tests` is **30 failed of 550** before and **30 of 550** after; no other project changes | high |
| P9 | `words/batch-010` goes 8/9 → **9/9**; `words/batch-001`…`010` goes 97/99 → **98/99**, the remaining failure being `1447.doc` at 3/4, which is the line-height residue and not this round's | high |
| P10 | The blind page-vision reading of the worst page will report the two renderings as differing in **where the page breaks / what content the page holds**, not in the running head itself — the head is pixel-close on both sides, only the body's start moves | medium |

The way P2 could be wrong that would matter most: a positioned table inside a *table cell* also
goes through `FlowLayouter`, so a cell holding one would change too. None was found in the corpus
scan, but the scan counted `w:tblpPr` per part rather than per nesting level.
