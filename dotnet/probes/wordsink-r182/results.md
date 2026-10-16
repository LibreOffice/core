# r182 — the words track after the two HC-Bulletin fixes

Same instrument as `wordsink-r181`: `track-ink-sweep.sh` over all 337 `words` documents against
the banked 26.2.4.2 reference, `REF_SOFFICE=/opt/libreoffice26.2/program/soffice`, measuring a
checksummed snapshot of the CLI so the tree could be rebuilt underneath it.

```
TOTAL 337  MATCH 329  REF-CANNOT-RENDER 0
ABS-INK 435.75 (unsigned |ink|%, ranks)  SIGNED-INK 315.10  MAJOR PAGES 171  over 329 documents
```

Against r181's `ABS-INK 444.35 / SIGNED-INK 323.73 / MAJOR PAGES 173`, with the gate unmoved at
329 `match`.

## What moved

**One row of 337**, and it is the document the round was about.

| | abs_ink | signed | major | drift | verdict |
|---|---:|---:|---:|---:|---|
| r181 | 10.47 | 9.91 | 2 | 0 | match |
| after the `HasInk` fix | 3.89 | 3.30 | 1 | 0 | match |
| after the `spAutoFit` fix | **1.87** | 1.28 | **0** | 0 | match |

`HC-Bulletin-template.docx` was r181's honest number one — the worst document with no page
drift — and is now nowhere near the head. Nothing else in the corpus differs from r181 by a
hundredth of a point either way.

The intermediate sweep is worth recording because it is what found the WordArt exception. The
unqualified `a:spAutoFit` rule took `exhibit-06---technical-architecture-template.docx` from
**0.13 to 3.18** and from no major page to two; gating on the Fontwork case put it back to 0.13
exactly. It was the only other document the rule touched.

## The ranking now

Six of the top eight still carry drift, so the honest list is the one filtered on it.

| abs_ink | major | pages | the worst rows with `drift` 0 |
|---:|---:|---:|---|
| 10.22 | 2 | 44 | `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` |
| 8.32 | 3 | 7 | `PK_FlugzeugeStricken.doc` |
| 7.00 | 3 | 118 | `A_320.doc` |
| 6.77 | 2 | 85 | `SPA-06_mcar_part-6_and_IS_v2.9.docx` |
| 5.18 | 1 | 1 | `021_Unit_Circle_Chart_3D_Pie_Chart.docx` |
| 4.64 | 1 | 1 | `016_Project_Timeline_Template_Complete_Guide.docx` |
| 4.25 | 1 | 1 | `090_Business_Case_Template_Blue_Theme.docx` |
| 4.04 | 1 | 8 | `1_tpr_template__from_fy14_.docx` |

`docs-quality-MA.IMS.00001` is the new head. Note for whoever takes it: it holds **38**
`a:spAutoFit` shapes and none of them is fitted by the reference — its `--convert-to fodt` gives
`draw:auto-grow-height="false"` on all 71 of its shapes, against HC-Bulletin's one `true`. So
whatever costs it 10.22 is not this round's rule, and the fodt said so before a single page was
rendered.
