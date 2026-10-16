# r183 — the words track re-ranked on the worst page rather than the sum

Same sweep, same CLI, same banked 26.2.4.2 reference as `wordsink-r182` — the totals are
identical to the hundredth — re-run only so `ink.tsv` carries the two columns round 183 added:
`worst`, the worst single page's unsigned `|ink|%`, and `mean`, the same column averaged.

```
TOTAL 337  MATCH 329  REF-CANNOT-RENDER 0
ABS-INK 435.75 (unsigned |ink|%, a track total)  SIGNED-INK 315.10  MAJOR PAGES 171  over 329
WORST PAGE 6.46 (unsigned |ink|%, RANKS) on words/done-014/doc/PK_FlugzeugeStricken.doc
```

## Why the columns exist

**`abs_ink` is a sum, so it ranks by length.** Round 182 handed round 183 a `drift`-free ranking
headed by `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` at 10.22, and most
of round 183 went into it. Its **mean is 0.23** over 44 pages, which is the raster floor for text
that dense; its worst page is 2.77. The three one-page chart templates it outranked are 5.18, 4.64
and 4.25 **on their only page**. The one real finding inside it — a 11.5 pt footer offset on 37 of
its 44 pages — measures at **0.29 of the 10.22**, and is the reference's artefact rather than ours
(`probes/footerlink-r183/`, and the entry now in `TODO.word-parity.md`).

So: rank on `worst`; read `mean` as a screen, where below about 0.3 says the document holds no
defect at all; keep `abs_ink` for a track total, where a sum is what you want.

## The ranking that replaces it

| worst | mean | sum | pages | major | document (`drift` 0) |
|---:|---:|---:|---:|---:|---|
| **6.46** | 1.19 | 8.32 | 7 | 3 | `PK_FlugzeugeStricken.doc` |
| **5.18** | 5.18 | 5.18 | 1 | 1 | `021_Unit_Circle_Chart_3D_Pie_Chart.docx` |
| **4.64** | 4.64 | 4.64 | 1 | 1 | `016_Project_Timeline_Template_Complete_Guide.docx` |
| **4.25** | 4.25 | 4.25 | 1 | 1 | `090_Business_Case_Template_Blue_Theme.docx` |
| 3.68 | 0.51 | 4.04 | 8 | 1 | `1_tpr_template__from_fy14_.docx` |
| 3.58 | 3.58 | 3.58 | 1 | 1 | `028_Unit_Circle_Chart_Optimized_Graph.docx` |
| 2.77 | **0.23** | 10.22 | 44 | 2 | `docs-quality-MA.IMS.00001-…-manual.docx` |
| 2.43 | **0.06** | 7.00 | 118 | 3 | `A_320.doc` |
| 2.07 | 0.69 | 3.45 | 5 | 1 | `644730BRI0mna000BOX361539B00public0.doc` |
| 2.04 | 0.86 | 2.57 | 3 | 2 | `Case-Study-Heathrow-Airport.docx` |
| 2.01 | 2.01 | 2.01 | 1 | 1 | `081_Printable_Graph_Paper_Template_Blue_Theme.docx` |
| 1.51 | **0.10** | 2.02 | 20 | 1 | `Agile_Arc_SysDes.docx` |

The two orders disagree about almost everything. `A_320.doc` was fourth on the old list at 7.00 and
its mean is **0.06** — 118 pages of nothing. `Agile_Arc_SysDes` likewise at 0.10. Meanwhile four of
the top six here are single-page chart or template documents whose whole content is one wrong page,
and three of them did not appear in the old list's head at all.

**Where to start**: `PK_FlugzeugeStricken.doc`, 7 pages, worst page 6.46, mean 1.19, three major
pages — the only row that is both badly wrong somewhere and badly wrong on average. Then the
one-page chart cluster, which four rows of this table say is one class rather than four documents.
