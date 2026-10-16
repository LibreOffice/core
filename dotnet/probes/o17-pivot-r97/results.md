# O17 sharpened: `alle einzeln.xlsx` is not a pivot-table reading defect

## What the seat said

> **`alle einzeln.xlsx` 225.44** — states no conditional formatting at all; it holds a pivot
> table. Uncharacterised. — `probes/sheet-ink-r94`

The natural reading of "it holds a pivot table" is that we do not render the pivot output and
the reference does. **That reading is wrong**, and two cheap measurements rule it out before any
work is spent on a pivot reader.

## What the reference resolves

`--convert-to fods` at 26.2.4.2, eight seconds:

| | |
|---|---:|
| sheets | 2 (`Pivot`, `alle einzeln`) — one hidden |
| `table:data-pilot-table` | 2 |
| `calcext:conditional-format` | **0** |
| `chart:chart` | **0** |
| `draw:frame` / `draw:custom-shape` | **0** / **0** |
| `style:page-layout` | 3 |
| `Pivot` sheet | 1013 rows, 10092 cells, **8060** `text:p` |
| `alle einzeln` sheet | 4242 rows, 55102 cells, 50682 `text:p` |

So the reference does materialise the pivot output — 8060 paragraphs of it.

## What we already render, from the banked gate

`gate-orig-r83/rows.tsv`, which has been on disk since round 83 and cost nothing to read:

```
sheets/done-013/xlsx/alle einzeln.xlsx  xlsx  186/186  …  match  64184/64187  278872/278868
```

**186 pages of 186, and 278872 alphanumeric characters against the reference's 278868** — a
difference of **four characters over 186 pages**, one part in seventy thousand. The `.ods`
spelling is 278868/278868, exact.

A renderer that did not read the pivot table could not produce 186 correct pages carrying
essentially every character the reference carries. Whatever moves this document's ink does so
with the right text, in the right amount, on the right number of pages.

## What that leaves

Not the pivot reader. Not conditional formatting (there is none). Not charts or drawings
(there are none). So the 225.44 is **cell presentation or grid geometry** — fills, borders,
font selection, alignment, number formats, or column widths and row heights that displace
content without changing what or how much of it there is.

The gate cannot see any of that, which is exactly why this document passes its verdict and sits
near the head of the ink ranking at the same time. It is a clean example of the rule this
project keeps relearning: **the gate is blind to colour, position and drawing, so a `match`
verdict is not evidence of a correct page.**

The next instrument is therefore a per-operator diff of one page rather than another census:
take a page from the middle of the `Pivot` sheet, dump both sides' fill and stroke operators,
and see whether the divergence is area (fills and borders) or displacement (positions). Do not
start by rendering all 186 pages.

## Method note

Both measurements here are re-used rather than produced: the `fods` is one eight-second
conversion, and the gate row was already banked. Neither required a render, which matters
because a gate run under contention undercounts on the reference side.
