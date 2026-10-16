# The authored variants

Eleven variants of `words/chartset-006/docx/088_Printable_Graph_Paper_Template_Quality_layout_33051f6e.docx`
and four of `087`/`080`, each changing one thing, rendered through both stacks with
`SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` against LibreOffice 26.2.4.2 620(Build:2).

The documents themselves are not stored — they are a few hundred kilobytes each and are
reproducible from the corpus with the edits below. What is stored is the result, because the
result is the evidence.

## 088 — which of a drawing's properties costs the page (fix B)

| variant | edit to `word/document.xml` | ours before | ours after | ref |
|---|---|---:|---:|---:|
| A | none (original) | 2 | 1 | 1 |
| B | delete the `w:r` holding the `w:drawing` | 1 | 1 | 1 |
| C | mark `<w:sz w:val="4"/>` → `22` | 2 | 2 | 2 |
| D | B and C together | 2 | 2 | 2 |
| E | `<wp:posOffset>266065</wp:posOffset>` → `0` | 2 | 1 | 1 |
| F | `wp:posOffset` → `-900000` | 2 | 1 | 1 |
| G | `wp:posOffset` → `-266065` | 2 | 1 | 1 |
| K | `<wp:extent cx="2143125" cy="409575"/>` → `cy="9525"` | 2 | 1 | 1 |
| L | `behindDoc="0"` → `"1"` | 2 | 1 | 1 |
| M | `<wp:wrapNone/>` → `<wp:wrapSquare wrapText="bothSides"/>` | 2 | 1 | 1 |
| N | `positionV relativeFrom="paragraph"` → `"page"`, offset 9000000 | 2 | 1 | 1 |

Read it as: **C and D refute "the mark's size is ignored"** — it was always honoured, on both
stacks. **A and B isolate the cause to the drawing run existing.** **E through N refute every
property of the frame**: none of offset, extent, paint order, wrap mode or anchor origin moves
the answer. After the fix all eleven agree with the reference and C and D still take two pages,
which is the control that the rule did not simply shorten every paragraph.

## 087 and 080 — does a fly displace the flow (fix A's guard)

| variant | edit | ours before | ours after | ref |
|---|---|---:|---:|---:|
| P0 | `087` original — fly anchored above the top margin, flow is an empty paragraph then a `Title:` line | 2 | 2 | 2 |
| P1 | `087` with both `w:t` runs of that line emptied | 1 | 1 | **1** |
| Q0 | `080` original — fly below the flow position, flow is one empty 2 pt paragraph | 2 | 1 | 1 |
| Q1 | `080` with a `Title: ___` run appended to that paragraph | — | 1 | 1 |

P0 against P1 is the whole guard: **one variable — whether the paragraph under the fly has ink —
and the reference's page count follows it.** Q1 is the control that shows the rule is about the
fly covering the flow rather than about text as such: 080's fly starts *below* the flow, the
added line sits above it at y=760.34, and both stacks stay on one page.
