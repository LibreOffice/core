# Pages where an embedded metafile puts the word gate out of reach

**Check this list before working any word-count failure.** Four separate agents have each spent
part of a round re-deriving that some page belongs to this class.

LibreOffice sometimes **rasterises** an embedded EMF or WMF instead of playing it, so its PDF
holds a picture where ours holds real, searchable glyph runs. The rendered pages look alike —
`pdf-image-diff.py` scores them near-identical — but `pdftotext` reads our text and finds nothing
in theirs. **Our output is the better one**, and the word gate scores it as a failure. Driving
those numbers down would mean drawing less text, which is the wrong direction.

Regenerate with:

```sh
.claude/skills/corpus-batches/scripts/metafile-pages.py /workspace/sample-files out
.claude/skills/corpus-batches/scripts/metafile-pages.py /workspace/sample-files out --documents-only
```

Machine-readable copy: `dotnet/metafile-pages.tsv`. Measured at `717c45724`.

## What the numbers are

| | |
|---|---|
| documents embedding a metafile | **76** of 534 — 42 slides, 30 words, 4 sheets |
| documents where a page actually shows the ceiling | **15** |
| pages flagged | **27** — 18 slides, 9 words, 0 sheets |
| excess words those pages account for | **2072** |
| documents that cannot be judged yet | **13** |

**Embedding a metafile is not evidence of anything by itself.** Of the 76 documents carrying
one, 48 show no ceiling at all — they play correctly on both sides, and several of those with
the largest metafile counts pass their batch outright. `redac-sas-201403-ppt-portfolio-rev-sim.pptx`
carries six and matches. A list built from "does it contain an EMF" would excuse dozens of real
defects, which is worse than no list: the point is to stop chasing what cannot be won, not to
stop looking.

## How a page earns its flag

Both conditions, on a document whose page count already agrees:

1. The **reference** draws a raster on that page (`pdfimages -list`).
2. We extract materially more words there than the reference does — at least 8 more and at
   least 25% more, which is about two-thirds of a line of prose. Below that the difference is
   hyphenation and soft breaks rather than a rasterised metafile.

## The flagged pages

| Document | Page | ours | ref | excess |
|---|---|---|---|---|
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.ppt` | 10 | 281 | 30 | +251 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.ppt` | 9 | 96 | 50 | +46 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 29 | 109 | 1 | +108 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 7 | 261 | 158 | +103 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität.pptx` | 21 | 78 | 23 | +55 |
| `slides/batch-014/…/WiGr_2021W_1_…` | 28 | 53 | 5 | +48 |
| `slides/batch-014/…/WiGr_2021W_1_…` | 45 | 50 | 5 | +45 |
| `slides/batch-014/…/WiGr_2021W_1_…` | 44 | 34 | 4 | +30 |
| `slides/batch-014/…/WiGr_2021W_1_…` | 46 | 35 | 5 | +30 |
| `slides/batch-014/…/Structural Testing.pptx` | 19 | 37 | 5 | +32 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 6 | 85 | 59 | +26 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 7 | 106 | 82 | +24 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 5 | 53 | 41 | +12 |
| `slides/batch-014/…/2014BSA_Sunday_Killion.pptx` | 8 | 52 | 40 | +12 |
| `slides/batch-014/…/Intersil_Italy_CAN_Bus_Transceiver_Presentation.pptx` | 30 | 130 | 103 | +27 |
| `slides/batch-016/…/FAAAIandtheArtandScienceofV&Vfinal.pptx` | 14 | 119 | 91 | +28 |
| `slides/batch-017/…/Wildlife for REDAC September 11.pptx` | 29 | 49 | 26 | +23 |
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 5 | 114 | 70 | +44 |
| `words/batch-016/…/JEMIT_Template.docx` | 1 | 745 | 465 | +280 |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities…docx` | 5 | 429 | 162 | +267 |
| `words/batch-013/…/FO.FCTOA_.000129 …` | 2 | 254 | 187 | +67 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 18 | 418 | 224 | +194 |
| `words/batch-020/…/EHEST-SMS-…-V2.docx` | 43 | 396 | 229 | +167 |
| `words/batch-020/…/EHEST-SMS-…-V2.docx` | 76 | 97 | 51 | +46 |
| `words/batch-011/…/f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 | 181 | +49 |
| `words/batch-018/…/airbus-pdf-information-package_v1-4.docx` | 6 | 173 | 125 | +48 |
| `words/batch-017/…/Agile_Arc_SysDes.docx` | 1 | 30 | 20 | +10 |

**A flagged page does not excuse its document.** `2014BSA_Sunday_Killion.pptx` has four flagged
pages and thirty-odd others where any difference is a real defect. Subtract the flagged pages
from the residue and work what is left.

## Thirteen documents cannot be judged yet

A per-page comparison is meaningless while the page counts disagree, so these are an honest
**unknown** rather than a pass — they may hold the ceiling, a defect, or both. **Fix their
pagination first, then re-run the scan.**

```
words/batch-010/docx/195584360.docx
words/batch-010/docx/5709.16 ch.40_mgfinal.docx
words/batch-013/docx/A1. EASA Form 2.docx
words/batch-013/docx/bulletin.docx
words/batch-017/docx/UG.CAO.00006 Foreign Part 145 approvals - User Guide….docx
words/batch-018/docx/HC-Bulletin-template.docx
words/batch-019/docx/CRIF - Spécification technique - Socle applicatif.docx
words/batch-020/docx/150-5370-10H.docx
words/batch-020/docx/AC-150-5370-10G-updated-201604.docx
words/batch-021/docx/ABCD-SDE-23-00 - Avionic System Description….docx
words/batch-021/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx
sheets/batch-012/xlsx/FAA-2019-0995-0002_attachment_2.xlsx
sheets/batch-014/xls/TICAPCapability_Final.xls
```

## What is known about the mechanism, and what is not

**Established.** The rasterisation happens **upstream of PDF export**, and it is a metafile path
rather than an OLE path. The raster is not in the file — two `.ppt`s were scanned through every
inflated zlib stream, not just their raw bytes — and it is not the PDF writer, since
`implWriteBitmapEx` downsamples only under `ReduceImageResolution` and the 300 dpi
`FLOATTRANSPARENT` branch cannot yield the observed 66–265 dpi. `8_P-Pavese…pptx` slide 5 is a
bare `p:pic` over an EMF with 791 `EXTTEXTOUTW` records, no EMF+, no alpha, no raster-op and no
bitmap, and the reference draws a 692×240 raster with a soft mask.

**Not established.** Which LibreOffice path does it. EMF+ is ruled out as the trigger by a
counter-example — `2014BSA` slide 5's EMF *does* carry EMF+ and renders as text.
`SELECTCLIPPATH` is the standout structural difference between the two, but that is a
correlation on two documents and is **unverified**.

Naming that path would let the flag become a rule rather than a list. Until then this file is
the record.

## Sheets has none

No sheets page is flagged, and only four sheets documents embed a metafile at all. The
spreadsheet track's image problem is a different one: **12 documents whose image *count*
differs**, four badly — `apron-area.xls` draws 0 against the reference's 1670, and that document
is a *full match* on the word gate. That is the opposite blind spot, and it needs the image
check rather than this list.
