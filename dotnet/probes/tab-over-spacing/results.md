# Where a right tab stop actually lands, and what the right indent does to it

Measured 2026-08-15 in the container described in `dotnet/CLAUDE.md` — **LibreOffice 26.2.4.2**,
Carlito/Caladea/Liberation/DejaVu present. Reference rendered with `soffice --convert-to pdf`;
positions read out of the PDF's own text geometry with `pdftotext -bbox-layout`.

## The question

`TabRuler` clamped a right, centred or decimal stop at the **line's** right edge — the frame's
width with the paragraph's indents taken out. `SwTabPortion::PostFormat`
(`sw/source/core/text/txttab.cxx`:503) clamps at the **frame's** right edge when `TabOverSpacing`
is on, and `WriterFilter.cxx`:325 turns that on for every writerfilter document. The two differ by
the paragraph's right indent, and the difference is what this probe measures.

## The fixture

`make.py` writes a DOCX with 30 paragraphs: one right dotted stop each, at ten positions crossing
the text area's right edge, at three right indents. US Letter, 1440-twip margins, so the text area
is **9360 twips** wide and starts 1440 twips from the page's left edge — a stop honoured where it
was declared therefore ends at `1440 + pos`. Built twice, at `compatibilityMode` 15
(`TabOverSpacing` alone) and 14 (`TabOverMargin` as well), to see whether the two behave alike.

## The answer, in twips of the last glyph's right edge

`compat15.txt` and `compat14.txt` hold all 30 rows of each. The measurements carry a constant
+2 twip offset, which is the reference's own text origin.

| declared | expected if honoured | `w:right=0` | `w:right=360` | `w:right=1134` |
|---:|---:|---:|---:|---:|
| 8000 | 9440 | 9442 | 9442 | 9442 |
| 9000 | 10440 | 10442 | 10442 | 10442 |
| 9360 (the area's edge) | 10800 | 10802 | 10802 | 10802 |
| 9500 | 10940 | 10942 | 10942 | 10942 |
| 9800 | 11240 | 11242 | 11242 | 11242 |
| 10500 | 11940 | 11942 | 11942 | 11942 |
| 10799 | 12239 | 12241 | 12241 | 12241 |
| 11000 | — clamped | 12277 | 12277 | 12269 |
| 12000 | — clamped | 12284 | 12284 | 12277 |
| 13000 | — clamped | 12284 | 12284 | 12277 |

Three things follow, and the first is the one the round turned on:

1. **The right indent moves nothing.** All three columns agree to the twip for every stop that is
   honoured. A clamp at the line's edge would have moved the `360` column 360 twips and the
   `1134` column 1134.
2. **A stop past the text area is still honoured**, out into the page's right margin, until it
   reaches the page's own right edge at 12240. That is Writer comparing a line-relative
   `GetTabPos()` against an absolute `getFrameArea().Right()`, which is a coordinate confusion in
   the reference rather than a rule — reproducing it needs the text area's absolute position, so
   `ClampsTabsAtLineEdge` clamps at the area's edge instead and says so.
3. **`TabOverMargin` makes no difference here.** Both files measure identically, all 30 rows, so
   the `compatibilityMode` ≤ 14 branch does not need separating from this one for right stops.

## What it cost on the corpus

The right-aligned stop on the dot-leader documents, ours minus the reference, at the right edge of
the page number, before and after the clamp was moved to the frame's edge:

| document | before | after |
|---|---:|---:|
| `words/pagination-002/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | −28.450 pt on 22 of 24 lines, −21.350 on 2 | **−0.100 on all 24** |
| `words/metrics-001/…/SPA-02_mcar_part-2_and_IS_v2.9.docx` | −18.10 median | **−0.094 median** |
| `words/metrics-001/…/02_mcar_part-2_and_IS_v2.10.docx` | −18.09 median | **−0.087 median** |
| `words/metrics-001/…/OM template for non-complex NCC operators_August 2016.docx` | −0.110 median | −0.110 median |

EHEST's two values are its two contents styles: `toc 1` declares `w:right="992"` and `toc 2`
`w:right="1134"`, and the shortfall is the indent less the 566 twips by which the stop already sat
inside the frame. The `mcar` figures are `toc 4`'s `w:right="360"` exactly, because its stop is
declared at the text area's edge. `OM template` never had the defect — its contents paragraphs
carry no right indent — which is why it is the control.
