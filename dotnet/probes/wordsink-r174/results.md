> **Superseded by `probes/wordsink-r181`, and `rank.py` should not be reused.** It duplicated
> `corpus-batches/scripts/track-ink-sweep.sh` with a measure biased by colour quantisation, no
> page-count gate and no alignment screen. The maintained tool ranks this track differently:
> `Annex-10` is ninth rather than first, and the two documents at the head here are excluded
> outright because their page counts differ. Kept as the record of what r174 ran.

# r174 — the words track ranked by ink at HEAD

`CLAUDE.md`'s standing instruction: the gate is page count, alphanumerics within max(2 %, 15) and
unembedded fonts, and it is blind to most real defects — a border, a fill, a bullet, a missing
arrowhead and a displaced block all add no character and no page. So rank the documents that
**pass** by ink and open the worst. That habit has produced a finding every time it has been run.

All 337 words-track documents rendered fresh through 26.2.4.2 and through this tree at
`717aefbf2`, scored on summed unsigned per-page ink at 36 dpi. `ranking.tsv` is the whole table,
sorted; `rank.py` is the scorer and prints the gate verdict beside the ink so the passing rows can
be read off.

**328 of 337 pass, and all nine failures are `pages` — no document on this track fails on
glyphs.**

## The head of the table

| sum \|ink\| | per page | MAJOR | pages | verdict | document |
|---:|---:|---:|---:|---|---|
| **881.67** | 2.826 | **214** | 312 | pages | `02_mcar_part-2_and_IS_v2.10.docx` |
| 321.20 | 2.072 | 48 | 155 | pages | `24-25_FAA_Holdover_Tables.docx` |
| 92.95 | 0.557 | 13 | 167 | pages | `FAA 2025-26 Holdover Tables.docx` |
| 71.95 | 0.922 | 16 | 78 | pages | `150_5300_13_chg10.doc` |
| 59.56 | 0.402 | 0 | 148 | **match** | `Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification` |
| 49.16 | 0.071 | 4 | 696 | **match** | `AC-150-5370-10G-updated-201604.docx` |
| 45.63 | 0.063 | 0 | 727 | **match** | `150-5370-10H.docx` |
| 35.14 | 0.132 | 0 | 266 | **match** | `SPA-02_mcar_part-2_and_IS_v2.9.docx` |
| **29.26** | **1.951** | **14 of 15** | 15 | **match** | `f445896eb008d14c1746fc37d412dc22.docx` |
| 9.81 | 1.963 | 2 | 5 | **match** | `HC-Bulletin-template.docx` |

**Read the per-page column, not the sum.** A percentage summed over pages is not comparable
between a 5-page document and a 727-page one: `150-5370-10H` is fifth by sum and **0.063 per page
with no MAJOR page at all**, which is a thin, even residual rather than a seat.

By that column the two passing documents worth opening are
`f445896eb008d14c1746fc37d412dc22.docx` — 1.951 per page with **14 of its 15 pages MAJOR** — and
`HC-Bulletin-template.docx` at 1.963 over five.

## The largest single seat on the track

`02_mcar_part-2_and_IS_v2.10.docx` is **881.67, nearly three times the next document, with 214 of
312 pages MAJOR** — and its alphanumerics are **414 531 against 414 481**, fifty characters apart
in four hundred thousand, on 313 pages against 312.

**That reading was wrong and `probes/tocwrap-r175` corrects it.** The pages are *misaligned*: ours
runs one PDF page behind the reference's from page 29 on, so comparing page *i* against page *i*
compares different pages and reports one defect two hundred times. The defect is one extra page of
table of contents, and it is closed — 881.67 → 25.57, 214 MAJOR pages → 0, `pages` → `match`.

## Read the `aligned` column before the ink

`rank.py` gained one after this round, and the reason is that the ink figure above misled three
readings before anyone checked. Ink is compared page against page, so a document that loses or
gains one page early reports every later page as different — one defect counted two hundred times.

- `02_mcar_part-2_and_IS_v2.10` at 881.67 was **one extra page of table of contents**
  (`probes/tocwrap-r175`); closed, and now 25.57 with `aligned` 0.99.
- `24-25_FAA_Holdover_Tables` at 321.27 and `FAA 2025-26 Holdover Tables` at 93.02 are **one
  deliberately divergent page each** — `TODO.word-parity.md`'s `TOC \t` entry. With the quirks
  switch on they are page-exact with **no MAJOR page between them** (`probes/holdover-r177`).

**37 of the 337 are in that state**, so the column is not optional reading. It is a screen rather
than a verdict: a page of dense numerals can score below the threshold while being the right page,
so a low `aligned` means *explain this before ranking it*.

`ranking.tsv` here is the table as re-scored at `608e635ce`, with the column.

## Track state

For comparison with the other two tracks, whose seats run into the hundreds per document: the
worst **passing** words document is 0.402 per page and the median is far below that. This track
is close to done on ink as well as on the gate; the remaining work is concentrated in about six
documents.
