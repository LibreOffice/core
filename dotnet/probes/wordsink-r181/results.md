# r181 — the words track ink ranking, re-run with the fixed tools

`wordsink-r174` ranked this track with a hand-rolled scorer. Round 180 fixed the shared tools;
this re-runs the ranking with `track-ink-sweep.sh` instead, against
`REF_SOFFICE=/opt/libreoffice26.2/program/soffice`, reusing the banked 26.2.4.2 reference
(verified first: 12 documents re-rendered fresh reproduce the bank **12 of 12** on pages and
words).

The run announces what it measured, which is the point of round 180:

```
measuring …/XRay.Office.Cli
reference /opt/libreoffice26.2/program/soffice -- LibreOffice 26.2.4.2 0229ac93…
```

## The gate

**337 documents — 329 `match`, 7 `pages`, 1 `pages,words`.** No `unembedded`, no failed render.

## A third defect, found by running it: the two sweeps were two different gates

The first run of this sweep reported **`MATCH 322`**, seven short of an independently computed
figure. The cause is in the tool, not the tree: `batch-check.sh` moved its text check from
**tokens, floor 3** to **alphanumeric characters, floor 15** on 2026-09-05, and
`track-ink-sweep.sh` carried its own older copy of `words_of` and of the verdict block. Its
`rows.tsv` had no `glyphs` column at all, so nothing showed it — two gates under one column
name, in one skill.

`words_of`, the verdict block and the `glyphs` column are now verbatim from `batch-check.sh`.
Re-run: **`MATCH 329`**, which is the independent count exactly, and the ink columns are
byte-identical at `ABS-INK 444.35` because ink never depended on the rule.

## The ranking

`abs_ink` is the summed per-page unsigned `|ink|%`; **`drift` is the number of pages holding
content the reference puts elsewhere, and a non-zero value means that row's ink is partly
measuring an offset.**

| abs_ink | signed | MAJOR | pages | drift | document |
|---:|---:|---:|---:|---:|---|
| 32.59 | 23.14 | 2 | 266 | **37** | `SPA-02_mcar_part-2_and_IS_v2.9.docx` |
| 31.64 | 27.38 | 14 | 696 | **17** | `AC-150-5370-10G-updated-201604.docx` |
| 17.21 | 13.23 | 5 | 64 | **4** | `150_5335_5a.doc` |
| 17.10 | 13.03 | 9 | 727 | **16** | `150-5370-10H.docx` |
| 16.32 | 13.82 | 6 | 82 | **3** | `EHEST-SMS-Safety-Management-Manual-V2.docx` |
| 12.91 | 9.18 | 4 | 24 | **4** | `AirbusCallouts.doc` |
| 10.47 | 9.91 | 2 | 5 | 0 | `HC-Bulletin-template.docx` |
| 10.22 | 6.81 | 2 | 44 | 0 | `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` |

**Six of the top eight carry drift**, so the honest ranking is the one filtered on it:

| abs_ink | MAJOR | pages | the worst rows with `drift` 0 |
|---:|---:|---:|---|
| 10.47 | 2 | 5 | `HC-Bulletin-template.docx` |
| 10.22 | 2 | 44 | `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` |
| 8.32 | 3 | 7 | `PK_FlugzeugeStricken.doc` |
| 7.00 | 3 | 118 | `A_320.doc` |
| 6.77 | 2 | 85 | `SPA-06_mcar_part-6_and_IS_v2.9.docx` |
| 5.18 | 1 | 1 | `021_Unit_Circle_Chart_3D_Pie_Chart.docx` |
| 4.64 | 1 | 1 | `016_Project_Timeline_Template_Complete_Guide.docx` |
| 4.25 | 1 | 1 | `090_Business_Case_Template_Blue_Theme.docx` |

## What the old scorer got wrong, measured

| document | hand-rolled scorer | `track-ink-sweep.sh` |
|---|---|---|
| `Annex-10-…-GCAA` | **worst passing document**, 59.56 | **3.97**, ninth among `drift` 0 |
| `02_mcar_part-2_and_IS_v2.10` | first overall, 881.67 | excluded — fails on pages, so no ink is computed |
| `24-25_FAA_Holdover_Tables` | first overall, 321.27 | excluded, same reason |
| gate | 329 `match` (character rule, PyMuPDF) | 329 `match` (character rule, `pdftotext`) |

The two extractors agree exactly where both were checked, so the gate figure was never the
problem — the *ink* figure was, twice over: once for colour quantisation and once for pages
compared against the wrong pages. `pdf-image-diff.py` refuses to score a document whose page
counts differ at all, which is why the two documents that dominated the old table are simply
absent from this one.

**`probes/wordsink-r174/rank.py` is superseded and should not be reused.** It duplicated this
tool with a biased measure, no page-count gate and no alignment screen.
