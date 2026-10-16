# words/table-001 — prediction, written before measuring the fix

Round: rotated table-cell text (`w:textDirection w:val="btLr"`) drawn upright, one glyph per line.
Written 2026-08-14, **before** building or measuring anything with the change in it.

## The seat, as located

`grep -rn "textDirection" dotnet/src` returns **nothing**. `w:textDirection` is not read by any
reader in the tree, and no cell model, layouter or painter carries a text direction. So this is
not a mis-mapping to correct; it is an unimplemented property. The three documents in the group
all state it, and all three state only `btLr`:

| document | `w:textDirection` occurrences |
|---|---|
| `A1. EASA Form 2.docx` | 6 × `btLr` |
| `B11. TE.CAO.00129  Experience  logbook.docx` | 12 × `btLr` |
| `approvals-and-standardisation-…-Experience--logbook.docx` | 11 × `btLr` |

Because nothing reads it, the label paragraph is laid out upright at the *column* width — a few
points wide — so every line holds one glyph and the cell becomes as tall as the label has
characters. That is one mechanism producing both reported symptoms: on a logbook the row was
already tall enough to absorb it (page-exact, text layer shattered into single-character tokens),
on the EASA form it was not (label column hundreds of points tall, two extra pages).

## What LibreOffice 26.2.4.2 actually does — measured, not predicted

Established on 45 generated probes rendered through the installed binary and read out of the
PDF operators (not a raster), before writing any code:

1. `btLr` is drawn with the text matrix `0 1 -1 0 x y` — a 90° turn, glyphs advancing **up** the
   page, one `Tm`+`Tj` per glyph.
2. Successive lines stack **rightwards**, one line height apart.
3. **A `btLr` cell contributes nothing at all to its row's height.** A row holding only rotated
   cells collapses to zero and draws nothing — no text and no borders (`q1-solo`, `q6-two`).
4. The line-break length is the cell's ordinary inner height: frame height − borders − *vertical*
   padding. Pinned by a 5-twip sweep: the 4→5 glyph boundary sits at exactly 500 twips = 25.00 pt
   in all three of {borders on, borders off, 10 pt top+bottom margin}, whose frame heights are
   25.5, 25.0 and 45.5 pt. Horizontal padding does **not** shorten the line.
5. `w:vAlign` places the line *stack* horizontally; paragraph `w:jc` runs along the vertical.
6. A line whose stack offset falls outside the cell's inner width is **not emitted at all** —
   dropped, not clipped. There is no `Tm` for it.
7. `tbRl` and `tbRlV` turn the other way (`0 -1 1 0`); `lrTbV` and `tbLrV` are upright. Matches
   `DomainMapperTableManager.cxx:325-350`. None of the three documents uses them.

## Predictions

Scored honestly afterwards in `results.md`.

**P1 — one fix closes all three.** All three documents state only `btLr`, and the two symptoms
are one mechanism. I expect the single change to move all three to `match`. *Confidence: high.*

**P2 — the EASA form loses its two extra pages.** 9 → 7 pages, matching the reference.
*Confidence: medium-high.* The label column is the stated cause but it is not proven to be the
only thing wrong with that document; there may be a second defect underneath.

**P3 — the logbooks' single-character token counts collapse.** 121-124 single-character tokens
→ the reference's 24-25, because a token is only single-character while a line is.
*Confidence: high.*

**P4 — words counts.** EASA 2399 → within the 2%+3 band of 2205; the two logbooks 1329 → 1247
and 1168 → 1098. *Confidence: medium.* The counts overshoot today; if we draw lines LibreOffice
drops (point 6 above) we would still overshoot, which is why the drop rule is being implemented.

**P5 — reach.** `w:textDirection` is rare. I expect **3 to 6** of the 200 words documents to
change their rendering at all, and **no verdict to move except the three**. *Confidence: medium.*
A grep would give a bigger number than this; the estimate is deliberately of what *resolves*.

**P6 — fidelity.** 30 failed of 550 before; I predict **30 of 550 after**, i.e. no fidelity test
moves either way, because none of them holds a rotated cell. *Confidence: medium-high.*

**P7 — regression.** `words/done-*` stays exactly where it is: zero documents change verdict.
*Confidence: high.* The new code path is entered only by a cell that states a rotating
`w:textDirection`, and everything else takes the branch it takes today.

**P8 — the raster-ceiling false positive.** `TODO.raster-ceiling.md`'s page-6 row for
`approvals-and-standardisation-…` will stop being reachable once the shattering is gone, i.e.
that document will pass without any raster concession. *Confidence: high* — the brief says both
sides draw the same six 47×90 images and the streams are byte-identical, so there is nothing
rasterised to concede.

## What would falsify the diagnosis

- The EASA form staying at 9 pages after the label column is fixed. That would mean the extra
  pages come from something else and the rotated cell merely made it louder.
- Any `words/done-*` document changing. Nothing without `w:textDirection` should be reachable.
- Row heights moving on documents whose rotated cells sit in rows that are already tall — a
  rotated cell contributes zero height, so making it contribute zero where it previously
  contributed a very large number must *shrink* rows and never grow them.
