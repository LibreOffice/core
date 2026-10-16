# r184 — the words track after the DOC continuous-section margin fix

```
TOTAL 337  MATCH 329  REF-CANNOT-RENDER 0
ABS-INK 434.55 (unsigned |ink|%, a track total)  SIGNED-INK 314.83  MAJOR PAGES 170  over 329
WORST PAGE 6.42 on words/done-014/doc/PK_FlugzeugeStricken.doc
```

Against r183's `435.75 / 315.10 / 171`, gate unmoved at 329 `match`. **Two rows of 337 differ**,
both improving: `PK_FlugzeugeStricken.doc` 8.32 → 7.13 with 3 major pages → 2, and
`150_5300_13_chg12.doc` 7.97 → 7.96.

## The head, ranked on the worst page

| worst | mean | sum | pages | major | document (`drift` 0) |
|---:|---:|---:|---:|---:|---|
| 6.42 | 1.02 | 7.13 | 7 | 2 | `PK_FlugzeugeStricken.doc` |
| **5.18** | 5.18 | 5.18 | 1 | 1 | `021_Unit_Circle_Chart_3D_Pie_Chart.docx` |
| **4.64** | 4.64 | 4.64 | 1 | 1 | `016_Project_Timeline_Template_Complete_Guide.docx` |
| **4.25** | 4.25 | 4.25 | 1 | 1 | `090_Business_Case_Template_Blue_Theme.docx` |
| 3.68 | 0.51 | 4.04 | 8 | 1 | `1_tpr_template__from_fy14_.docx` |
| **3.58** | 3.58 | 3.58 | 1 | 1 | `028_Unit_Circle_Chart_Optimized_Graph.docx` |
| 2.77 | 0.23 | 10.22 | 44 | 2 | `docs-quality-MA.IMS.00001-…-manual.docx` |
| 2.43 | 0.06 | 7.00 | 118 | 3 | `A_320.doc` |
| 2.07 | 0.69 | 3.45 | 5 | 1 | `644730BRI0mna000BOX361539B00public0.doc` |
| 2.04 | 0.86 | 2.57 | 3 | 2 | `Case-Study-Heathrow-Airport.docx` |

**The first row is spent.** Its 6.42 is one page, and that page is `TODO.word-parity.md`'s
*"A continuous section's own furniture arrives a page late"* — the reference's deferral, not our
defect. Its other six pages come to 0.71 between them. Row seven is spent for a different reason
(`probes/footerlink-r183/`: mean 0.23 is the raster floor, and the one real finding inside it is
the reference's too).

**So the live head of the words track is the one-page chart and template cluster** — rows 2, 3, 4
and 6, at 5.18, 4.64, 4.25 and 3.58, each a single page that is wholly wrong, and at least two of
them (`021` and `028`) are the same *Unit Circle Chart* template. Four rows that four documents
share is a class, not four documents, and that is where the next round should start.
