# A cell's vertical alignment, and the object anchored in it

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`, against **both**
references: the distro's 24.2.7.2 and the TDF tarball's 26.2.4.2 with its duplicate fonts moved
aside. The two disagree here, which is the reason both are in the table.*

## Where this started

`1528039320.docx` draws its header logo 33.6 pt too low, over the row below it — the worst-looking
member of the catalogued *overlap and clipping* cause. Its logo is anchored in a header cell that
states `w:vAlign="bottom"`, so the question is what a cell's vertical alignment does to a frame
anchored inside it.

## The probe

`valign.py` writes one table, one row 200 pt tall, one cell whose `w:vAlign` is swept, holding a
paragraph with a text run reading `TEXT` and an anchored text box reading `MARK` at `positionV
relativeFrom="paragraph" posOffset="0"`. `TEXT`'s y is where the alignment put the paragraph;
`MARK`'s is where the frame went. Three dimensions are swept, because Writer's guard
(`#i43913#`, `sw/source/core/layout/tabfrm.cxx`:6270-6330) turns on all three: the alignment, the
object's wrap, and whether the object is in the cell at all.

## `TEXT`'s y — 72.53 is top of cell, 258.58 is bottom

| `w:vAlign` | wrap | where | 24.2.7.2 | 26.2.4.2 | ours |
|---|---|---|---:|---:|---:|
| bottom | *(no object)* | — | 258.58 | 258.58 | 260.89 |
| bottom | `wrapNone` | in the cell | **72.53** | **258.58** | 260.89 |
| bottom | `wrapSquare` | in the cell | 72.53 | **72.53** | **260.89** |
| bottom | `wrapThrough` | in the cell | 72.53 | **72.53** | **260.89** |
| bottom | `wrapTopAndBottom` | in the cell | 90.58 | **90.58** | **260.89** |
| bottom | `wrapNone` | below the table | 72.53 | 258.58 | 260.89 |

`center` behaves the same way throughout, and `top` is unaffected by any of it.

So, against the version the tree targets:

* **An object with a real wrap in the cell switches the alignment off.** `wrapSquare` and
  `wrapThrough` both put the text back at the cell's top. We keep aligning, and are 188 pt out.
* **`wrapTopAndBottom` is different again**: the object takes room at the top and the text starts
  below it, at 90.58.
* **`wrapNone` changes nothing** — the object is on the through layer and the cell aligns as if it
  were not there.

The guard's source says exactly this: alignment is dropped when an anchored object overlaps the cell
and `bForceTopVAlign || WrapTextMode_THROUGH != rSur.GetSurround()`, which `wrapNone` fails on both
counts.

## The two versions disagree, and 24.2 is the odd one

24.2.7.2 drops the alignment for `wrapNone` as well — including when the object is nowhere near the
cell, anchored 630 pt down the page. 26.2 keeps it in both. The difference is the newer half of the
guard, `FORCE_TOP_ALIGNMENT_IN_CELL_WITH_FLOATING_ANCHOR` (tdf#166710), which exists only in 26.2:

```sh
strings /usr/lib/libreoffice/program/libswlo.so | grep -c ForceTopAlignmentInCellWithFloatingAnchor  # 0
strings /opt/libreoffice26.2/program/libswlo.so | grep -c ForceTopAlignmentInCellWithFloatingAnchor  # 1
```

## Which is why `1528039320.docx` is not the defect it looked like

Its logo is `wrapNone`, so 26.2 keeps the cell's bottom alignment and puts the logo low — where we
put it. The three renderings of its first page, logo bounding box:

| | x0 | y0 | x1 | y1 |
|---|---:|---:|---:|---:|
| 24.2.7.2, the gate | 50.88 | **32.16** | 143.76 | 72.24 |
| 26.2.4.2, the target | 50.88 | **66.24** | 143.76 | 106.32 |
| ours | 49.92 | **65.76** | 143.76 | 106.32 |

**Half a point from the target and 33.6 pt from the gate.** See
`probes/words-version-screen/results.md`, which this document is the second worked example for.

## What is left, and it has no measured reach yet

The `wrapSquare` / `wrapThrough` / `wrapTopAndBottom` rows above are a real gap: 26.2 drops the
cell's vertical alignment and we keep it. No corpus document has been shown to hit it — the one that
sent this round looking was `wrapNone` — so it is recorded here rather than fixed, and the probe is
the thing to re-run when a document turns up that needs it.
