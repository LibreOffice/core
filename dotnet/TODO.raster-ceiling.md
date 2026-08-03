# Pages where the reference rasterises and the word gate cannot be won

**Check this list before working any word-count failure.** Several agents have each spent part
of a round re-deriving that some page belongs to this class.

LibreOffice sometimes **rasterises** an embedded object instead of playing it, so its PDF holds a
picture where ours holds real, searchable glyph runs. The rendered pages look alike —
`pdf-image-diff.py` scores them near-identical — but `pdftotext` reads our text and finds nothing
in theirs. **Our output is the better one**, and the word gate scores it as a failure. Driving
those numbers down would mean drawing less text, which is the wrong direction.

Regenerate with:

```sh
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /workspace/sample-files out
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /workspace/sample-files out --documents-only
```

Machine-readable copy: `dotnet/raster-ceiling-pages.tsv`.

## An embedded metafile is the commonest cause, not the only one

This file was first built by scanning for embedded EMF/WMF and comparing only the pages of
documents that carried one. **That was wrong, and it hid nearly half the list.**

| | |
|---|---|
| pages flagged | **53** across 31 documents |
| flagged pages whose document embeds a metafile | 29 |
| flagged pages whose document embeds **none** | **24** |
| excess words accounted for | **3687** |
| documents embedding a metafile at all | 100 of 534 |
| documents that cannot be judged yet | 82 |
| documents neither renderer could pair | 2 |

`W3_Case_Study…ppt` holds no metafile and its page 10 is squarely this class — the reference
draws there the same 845×572 object it draws on `Thailand17.ppt`'s page 8. Filtering by the
presumed cause made those pages invisible to the whole tool. **The flag now keys on the
observable signature and the metafile count rides along as an attribution.**

The scan also could not originally see a metafile in a binary document at all: a `.ppt` keeps its
pictures zlib-compressed inside Escher blip records, so a raw signature search finds nothing in a
file that plainly contains one. Inflating every plausible stream took the carrier count from 76
to 100.

## How a page earns its flag

Both conditions, on a document whose page count already agrees:

1. The **reference** draws a raster on that page (`pdfimages -list`).
2. We extract materially more words there than the reference does — at least 8 more and at least
   25% more, about two-thirds of a line of prose. Below that the difference is hyphenation and
   soft breaks rather than a rasterised object.

## Two boundaries worth stating

**A flagged page does not excuse its document, and the two can point opposite ways.**
`UG.CAO.00133 …Language.docx` over-draws on a flagged page while running **−225 words overall** —
it holds a genuine shortfall elsewhere. Subtract the flagged pages from the residue and work what
is left. Likewise `NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` passes its batch and still has a
flagged page.

**Eighty-two documents cannot be judged.** A per-page comparison is meaningless while the page
counts disagree, so those are an honest **unknown** rather than a pass — they may hold the
ceiling, a defect, or both. Fix their pagination first, then re-run. Two more (`2013_11.doc`,
`Sample_SQMS_Program.docx`) failed to produce a pair on this run.

## The flagged pages

| Document | Page | ours | ref | excess | metafile |
|---|---|---|---|---|---|
| `words/batch-016/…/AFS-050-004-F2_0i.docx` | 3 | 419 | 53 | +366 | — |
| `words/batch-016/…/JEMIT_Template.docx` | 1 | 745 | 465 | +280 | 2/0 |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 5 | 429 | 162 | +267 | 2/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 10 | 281 | 30 | +251 | 2/0 |
| `words/batch-015/…/hdss-bulletin-issue-285-25-june-2025.docx` | 9 | 406 | 182 | +224 | — |
| `slides/batch-014/…/N2_E_Maestroni_Swarm_COP.pptx` | 7 | 307 | 102 | +205 | — |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 18 | 418 | 224 | +194 | 6/0 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 43 | 396 | 229 | +167 | 6/0 |
| `words/batch-012/…/RMI_Document_Repository_Public-Reprts_GettingO` | 2 | 226 | 73 | +153 | — |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 29 | 109 | 1 | +108 | 2/0 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 7 | 261 | 158 | +103 | 2/0 |
| `slides/batch-010/…/W3_Case_Study_of_a_Tsunami_Warning_Simulation_` | 10 | 102 | 9 | +93 | — |
| `slides/batch-014/…/Thailand17.ppt` | 8 | 102 | 9 | +93 | 6/0 |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 2 | 254 | 187 | +67 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 21 | 78 | 23 | +55 | 0/1 |
| `words/batch-008/…/ECSS-E-ST-50-16C-Annex-A(30September2021).docx` | 3 | 164 | 113 | +51 | — |
| `slides/batch-010/…/Fundamentals_Module_1_basics.ppt` | 6 | 70 | 20 | +50 | 1/0 |
| `words/batch-011/…/f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 | 181 | +49 | 0/3 |
| `words/batch-018/…/airbus-pdf-information-package_v1-4.docx` | 6 | 173 | 125 | +48 | 1/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 28 | 53 | 5 | +48 | 0/1 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 76 | 97 | 51 | +46 | 6/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 9 | 96 | 50 | +46 | 2/0 |
| `words/batch-014/…/UG.CAO.00133 Foreign Part 145 approvals - Lang` | 7 | 77 | 32 | +45 | — |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 45 | 50 | 5 | +45 | 0/1 |
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 5 | 114 | 70 | +44 | 3/2 |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 5 | 93 | 54 | +39 | — |
| `words/batch-015/…/approvals-and-standardisation-organisation-app` | 6 | 161 | 123 | +38 | — |
| `sheets/batch-010/…/TOGAF9-Tool-ConfReqts-CSQ.xls` | 21 | 69 | 31 | +38 | — |
| `slides/batch-014/…/Structural Testing.pptx` | 19 | 37 | 5 | +32 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 44 | 34 | 4 | +30 | 0/1 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 46 | 35 | 5 | +30 | 0/1 |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 7 | 94 | 64 | +30 | — |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 4 | 108 | 79 | +29 | — |
| `slides/batch-016/…/FAAAIandtheArtandScienceofV&Vfinal.pptx` | 14 | 119 | 91 | +28 | 1/1 |
| `slides/batch-014/…/Intersil_Italy_CAN_Bus_Transceiver_Presentatio` | 30 | 130 | 103 | +27 | 6/0 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 6 | 85 | 59 | +26 | 31/0 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 7 | 106 | 82 | +24 | 31/0 |
| `slides/batch-017/…/Wildlife for REDAC September 11.pptx` | 29 | 49 | 26 | +23 | 4/1 |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 38 | 84 | 63 | +21 | — |
| `words/batch-014/…/UG.CAO.00133 Foreign Part 145 approvals - Lang` | 3 | 54 | 34 | +20 | — |
| `words/batch-014/…/UG.CAO.00133 Foreign Part 145 approvals - Lang` | 8 | 59 | 39 | +20 | — |
| `words/batch-014/…/UG.CAO.00133 Foreign Part 145 approvals - Lang` | 18 | 60 | 40 | +20 | — |
| `words/batch-017/…/system_design__technical_architecture_template` | 1 | 33 | 20 | +13 | — |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 5 | 53 | 41 | +12 | 31/0 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 8 | 52 | 40 | +12 | 31/0 |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 2 | 38 | 27 | +11 | — |
| `words/batch-017/…/Agile_Arc_SysDes.docx` | 1 | 30 | 20 | +10 | 1/2 |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 13 | 41 | 31 | +10 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 14 | 46 | 36 | +10 | — |
| `slides/batch-009/…/NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` | 5 | 15 | 5 | +10 | — |
| `slides/batch-004/…/ws_prod-g-doc-Events-industrymeeting18112004-E` | 9 | 38 | 29 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 26 | 30 | 21 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 39 | 38 | 30 | +8 | — |

## What is known about the mechanism, and what is not

**Established.** The rasterisation happens **upstream of PDF export**. The raster is not in the
file — two `.ppt`s were scanned through every inflated zlib stream, not just their raw bytes — and
it is not the PDF writer, since `implWriteBitmapEx` downsamples only under
`ReduceImageResolution` and the 300 dpi `FLOATTRANSPARENT` branch cannot yield the observed
66–265 dpi. `8_P-Pavese…pptx` slide 5 is a bare `p:pic` over an EMF with 791 `EXTTEXTOUTW`
records, no EMF+, no alpha, no raster-op and no bitmap, and the reference draws a 692×240 raster
with a soft mask.

**Not established.** Which LibreOffice path does it, and whether the metafile-carrying and
metafile-free cases share one. EMF+ is ruled out as the trigger by counter-example — `2014BSA`
slide 5's EMF *does* carry EMF+ and renders as text. `SELECTCLIPPATH` is the standout structural
difference between the two, but that is a correlation on two documents and is **unverified**.

Naming that path would let the flag become a rule rather than a list. Until then this is the
record.

## Sheets is nearly untouched by this

One flagged page on the whole track. The spreadsheet track's image problem is the opposite one:
**12 documents whose image *count* differs**, four badly — `apron-area.xls` draws 0 against the
reference's 1670, and that document is a *full match* on the word gate. That needs the image
check, not this list.
