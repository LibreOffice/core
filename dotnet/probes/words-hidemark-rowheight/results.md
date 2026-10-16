# `w:hideMark` makes a row's `w:trHeight` a height rather than a floor

Measured 2026-09-05 in the container described at the top of `dotnet/CLAUDE.md`:
`/usr/bin/soffice` is **LibreOffice 24.2.7.2**, `/opt/libreoffice26.2/program/soffice` is the TDF
tarball **26.2.4.2** with its bundled font duplicates left in place. Carlito, Caladea, Liberation
and DejaVu are all installed; `fc-match "DejaVu Sans"` answers `DejaVuSans.ttf`.
XRay.Office at `agent/draw`, one commit before the fix for the "before" column.

## The question

`084_Printable_Graph_Paper_Template_Editable_Layout_d66c6820.docx` draws its graph-paper grid at a
**9.00 pt** row pitch in both references and at **14.40 pt** in ours — a grid 60% too tall, which
pushes the sheet's `Title:` and `Date:` rules off the page entirely. Its 48 rows declare
`w:trHeight w:val="180"` with no `w:hRule`, which is a *floor*, and an 11 pt Calibri line wants
13.7 pt. So the reference is treating the declared height as exact, and the question is what tells
it to.

## The rule, and the second rule inside it

`sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx:1157-1162`

> We have CellHideMark on all cells, and also all cells are empty: force the row height to be
> exactly as specified, and not just as the minimum suggestion.

with `lcl_hideMarks` at `:1027` (every cell carries `w:hideMark`, and none is vertically merged —
"if anything is vertically merged, the row must not be set to fixed as Writer's layout doesn't
handle that well") and `lcl_emptyRow` at `:1085` (every cell's text range is empty).

That alone does not explain `084`, because **every one of its 1728 cells holds a no-break space**.
The second rule is `sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:3032-3045`, tdf#77417:

```cpp
// tdf#77417 trim right white spaces in table cells in 2010 compatibility mode
sal_Int32 nMode = GetSettingsTable()->GetWordCompatibilityMode();
if (0 < m_StreamStateStack.top().nTableDepth && 0 < nMode && nMode <= 14)
```

so in a file below compatibility mode 15 a table cell's trailing spaces, tabs and no-break spaces
are trimmed away, and a cell holding nothing else becomes genuinely empty before the row is
measured. `084` declares `compatibilityMode 14`.

## The matrix

`make.py` builds a ten-row three-column table, `w:trHeight w:val="180"` and no `w:hRule`, 11 pt
Calibri; `measure.py` renders it through both binaries and our CLI and reports the median row
pitch in points off the 300 dpi raster. `out7/` varies three things: the declared compatibility
mode, whether every cell carries `w:hideMark`, and what the cell holds.

| mode | `hideMark` | cell holds | 24.2.7.2 | 26.2.4.2 | ours before | ours after |
|---|---|---|---:|---:|---:|---:|
| 12 | yes | nothing | 9.12 | 9.12 | 13.92 | **9.12** |
| 12 | yes | `U+00A0` | 9.12 | 9.12 | 13.92 | **9.12** |
| 12 | yes | `x` | 13.92 | 13.92 | 13.92 | 13.92 |
| 12 | no | any | 13.92 | 13.92 | 13.92 | 13.92 |
| 14 | yes | nothing | 9.12 | 9.12 | 13.92 | **9.12** |
| 14 | yes | `U+00A0` | 9.12 | 9.12 | 13.92 | **9.12** |
| 14 | yes | `x` | 13.92 | 13.92 | 13.92 | 13.92 |
| 15 | yes | nothing | 9.12 | 9.12 | 13.92 | **9.12** |
| 15 | yes | `U+00A0` | 13.92 | 13.92 | 13.92 | 13.92 |
| 15 | yes | `x` | 13.92 | 13.92 | 13.92 | 13.92 |
| absent | yes | nothing | 9.12 | 9.12 | 13.92 | **9.12** |
| absent | yes | `U+00A0` | 13.92 | 13.92 | 13.92 | 13.92 |

**24 of 24 exact after the fix, and the two binaries agree on every one of the 24** — this is not a
version gap. The `absent` rows are why the guard is `0 < mode && mode <= 14` and not `mode < 15`:
a file that states no mode leaves ours at −1 and both references give it the modern behaviour.

## The same three questions asked of the real document

`out3/` takes `084` itself and changes exactly one thing:

| variant | 24.2.7.2 | 26.2.4.2 | ours after |
|---|---:|---:|---:|
| `w:hideMark` deleted everywhere | 14.40 | 14.40 | 14.40 |
| the no-break space replaced by `x` | 14.40 | 14.40 | 14.40 |
| the whole run deleted | 8.88 | 8.88 | 8.88 |

3 of 3 exact, and between them they pin both halves of the rule on the document the defect was
found on.

## Effect on the corpus

Rendering the 158 DOCX documents that carry a `w:hideMark` or a VML `z-index`, before and after,
and scoring the mean per-page ink against 26.2.4.2:

| document | before | after |
|---|---:|---:|
| `084_Printable_Graph_Paper_Template_Editable_Layout` | 30.90 | **1.02** |
| `081_Printable_Graph_Paper_Template_Blue_Theme` | 37.53 | **32.55** |

No other document's row heights moved, and no page count moved anywhere.

## What this probe does not settle

The trailing-blank trim is implemented **only inside the emptiness test**, not in the layout.
LibreOffice really does trim those characters off the paragraph, which also changes a cell's
measured width; reproducing that reaches 1 043 cell paragraphs across 61 corpus documents below
mode 15 and belongs to a round of its own.
