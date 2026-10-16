# Screening the words queue against the version the tree targets

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`. **Two** references:
the distro's **24.2.7.2** at `/usr/bin/soffice`, which is what `batch-check.sh` and every ink figure
in this repository are measured against, and the TDF tarball's **26.2.4.2** at
`/opt/libreoffice26.2`, which is the version this tree is calibrated to.*

## Why this exists

Twice in one session a document at the top of the ink table turned out to be the gate binary rather
than a defect — the `Printable_Graph_Paper_Template` family's row pitch, and then the very next
document picked, `1528039320.docx`, whose header logo we place within **half a point** of 26.2.4.2
and 33.6 pt away from 24.2.7.2. Each cost an hour. The ink ranking is not a queue until the version
gap is taken out of it.

`screen.py` renders the worst N words documents with **both** binaries and scores ours against each.
`bucket.py` does the same for a named cause out of `pl-readings.json`.

**Before running either, move the tarball's duplicate metric-compatible fonts aside** — it ships its
own Carlito, Caladea, Liberation and DejaVu builds, which differ from the system's by md5 and shift
every advance width. 33 files here:

```sh
D=/opt/libreoffice26.2/share/fonts/truetype
mkdir -p $D/.duplicates-aside && mv $D/{Carlito,Caladea,Liberation,DejaVu}*.ttf $D/.duplicates-aside/
```

That leaves the tarball a *third* reference rather than the distro-packaged 26.2 the tree really
targets — see `CLAUDE.md`'s "Installing a specific LibreOffice" — so read it as a discriminator, not
as a new gate.

## The worst 30 by first-page ink, rescored

Eleven of the thirty are the version gap, and they are the whole top of the table:

| document | vs 24.2 | vs 26.2 |
|---|---:|---:|
| `088_Printable_Graph_Paper_Template_Quality_layout` | 32.40 | **0.16** |
| `Technical_Issue_Report_Form` | 28.44 | **1.39** |
| `4400-91_Proposal_To_Lease_Space_10-2024` | 20.70 | **2.89** |
| `1528039320` | 45.32 | **3.12** |
| `089_Printable_Graph_Paper_Template_Simlpe_Format` | 37.43 | **4.06** |
| `1528364855.doc` | 35.74 | **4.93** |
| `Form-SM-76A-…-Compliance-Statement` | 41.01 | **6.08** |
| `085_Printable_Graph_Paper_Template_Excellent_Format` | 36.98 | **8.86** |
| `080_Printable_Graph_Paper_Template_Black_Theme` | 51.29 | **11.26** |
| `018_Project_Timeline_Template_Editable_Format` | 40.66 | **12.09** |
| `016_Project_Timeline_Template_Complete_Guide` | 48.47 | **15.64** |

`Technical_Issue_Report_Form` is worth its own line: the border round scored it as a **regression**
(23.82 → 28.44) on evidence that its head rule and first table rules had become exact. Against the
version the tree targets it reads **1.39**, which settles that argument for good.

And what is genuinely ours, in order:

| document | vs 24.2 | vs 26.2 |
|---|---:|---:|
| `Case-Study-Heathrow-Airport` | 43.10 | **40.42** |
| `081_Printable_Graph_Paper_Template_Blue_Theme` | 37.25 | **39.47** |
| `RNW167A-150428.doc` | 37.51 | **37.50** |
| `084_Printable_Graph_Paper_Template_Editable_Layout` | 32.37 | **32.38** |
| `029_Unit_Circle_Chart_Pie_Theme` | 21.39 | **25.88** |
| `090_Business_Case_Template_Blue_Theme` | 28.89 | 24.99 |
| `096_Business_Case_Template_Editable_Layout` | 20.67 | 24.05 |
| `644730BRI0mna000BOX361539B00public0.doc` | 22.82 | 26.07 |
| `24-25_FAA_Holdover_Tables` | 23.67 | 23.67 |
| `JEMIT_Template` | 27.44 | 21.89 |
| `PES-Technical-Report-Template_Jan_2019` | 21.67 | 21.67 |

**Two of the seven graph-paper documents are real** — 081 and 084 — where the other five are the
version gap. A family sharing a name does not share a defect, and only the rescoring separates them.

## The catalogued *overlap and clipping* cause, all nine

| # | document | vs 24.2 | vs 26.2 | pages 24.2/26.2/ours |
|---:|---|---:|---:|---|
| 20 | `1528039320.docx` | 45.32 | **3.12** | 1/1/1 |
| 174 | `essd-16-3433-2024-t02.xlsx` | 9.52 | **0.00** | 4/4/4 |
| 44 | `2015-April-SWIM_Users_Forum-Q&A.docx` | 25.26 | 11.55 | 5/5/5 |
| 46 | `mde087077~283.docx` | 24.11 | **24.12** | 4/4/4 |
| 120 | `7-Zulkefli_Part147n66_IKMAS.pptx` | 22.10 | **16.10** | 18/18/18 |
| 82 | `TK-Syllabus-Comparison-Document-v2.xlsx` | 15.89 | **15.82** | 1235/1235/1235 |
| 180 | `FY2023-AIP-grants.xlsx` | 12.43 | **12.29** | 33/33/33 |
| 131 | `2017-04-27-Lease-Transition-Records-Checklist` | 10.18 | *53.15* | 5/**6**/5 |
| 135 | `2020-01-29-Lease-Transition-Records-Checklist` | 9.33 | *50.41* | 5/**6**/5 |

Three findings, none of which the ink ranking alone could give:

1. **#174 is not a defect at all.** It was carried as the bucket's most dramatic case — "renders
   completely blank" — and it matches 26.2.4.2 at **0.00 ink** on the catalogued page.
2. **The bucket's worst real member is #46**, at 24.12, not #20 at 45.32.
3. **#131 and #135 run the other way**: 26.2 paginates them to six pages where 24.2 and we both say
   five, and scoring against 26.2 makes them look five times worse. They are the counter-example to
   using 26.2 blindly, and the reason this is a screen rather than a replacement gate.

## The rule to carry

**Before working a words document, rescore it against 26.2.** Ink against 24.2 ranks the queue by
how far the two *binaries* have moved as much as by how far we have. Where the two references
disagree with each other, neither number means much on its own and the document needs reading
rather than scoring.
