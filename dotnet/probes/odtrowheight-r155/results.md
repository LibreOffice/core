# Round 155 — three rules an `.odt`'s tables were not reading

**Written incrementally.** Each section was written as its measurement finished.

## 0. What this round is

Round 154 measured its four fixtures in all four word-processing formats by converting them with
26.2.4.2 itself, and left the `.odt` column at **13 of 34 arms differing** with three named seats:

| seat | what |
|---|---|
| **O103** | `MinRowHeightInclBorder` is not read, so no `.odt` row charges its insets or its own top rule to a declared height |
| **O104** | the ODF twin of round 154's covered-cell seat |
| **O106** | *(opened by this round)* `fo:break-before="page"` on a table style is not read |

All three are closed here, and the third was found because it blocked the fixture.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, base `77304c267` *(round 154)* |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`** |
| C++ read | this checkout, `configure.ac` **27.2.0.0.alpha0+** — *not* the reference binary's source |
| date | 2026-09-19 |

---

## 1. [src] `MinRowHeightInclBorder` is a document setting, and this reader thought it was a filter's

`PageTable.MinHeightIncludesInsets` gates two terms of a row's height — its insets and its own stated
top rule — and its remark said *"`MIN_ROW_HEIGHT_INCL_BORDER` is a per-document setting and the three
Word filters set it while the ODF one does not"*. The first half is right and the second is not.

`git grep MIN_ROW_HEIGHT_INCL_BORDER -- sw/source` finds exactly one filter that sets it:
**`sw/source/filter/ww8/ww8par.cxx`:1966**, the WW8 one. It is reachable from anywhere else only as a
**UNO document setting**, `MinRowHeightInclBorder`
(`sw/source/uibase/uno/SwXDocumentSettings.cxx`:288) — which is how an ODF file states it, and
LibreOffice's ODF *export* writes the state the import left behind. So every `.odt` it wrote from a
Word-family document carries `true`, and **all 337 of the converted corpus state it.**

Its default is **false** (`mbMinRowHeightInclBorder(false)`,
`sw/source/core/doc/DocumentSettingManager.cxx`:113), which is the direction that matters: an unread
setting is a *missing* charge on every such file rather than a spurious one on a few.

## 2. [bin] What each rule is worth, on 26.2.4.2's own conversions of round 154's fixtures

| fixture, converted to `.odt` | arms | differed before | after |
|---|--:|--:|--:|
| 7 — a `w:trHeight` floor | 18 | **10** | 0 |
| 8 — the band above the row | 8 | 2 | 0 |
| 9 — the band below it | 4 | 1 | 0 |
| 10 — the band below the last row | 4 | 0 | 0 |
| **all four** | **34** | **13** | **0** |

Reading the setting alone takes fixture 7 from **10 to 2**, and the two that remain are `VBN` and
`VBA` — the covered-cell arms, which is O104 exactly. **Five of the ten hold no merged cell at all**,
which is what says the larger half of that column's row-height error was never the covered cell.

## 3. [bin] O106: `fo:break-before="page"` on a table style, found by a fixture that would not lay out

The fixture for this round is 26.2.4.2's own `--convert-to fodt` of round 154's
`words-vmerge-covered-rule.docx`, whose eight arms are one per page. **The reference draws eight
pages of it and this tree drew one.**

A DOCX states a break before a table on the empty paragraph in front of it; LibreOffice's ODF export
writes it onto the **table's own automatic style**, as `fo:break-before="page"` on a
`style:table-properties`. `PageTable.StartsNewPage` has existed since the round that closed the DOCX
side and its remark has cited *this attribute* the whole time — as the evidence that LibreOffice
carries the break — while nothing in the ODF reader ever set it.

**12 of the 337 converted `.odt` state it, over 40 table styles.**

## 4. The change, the tests and the pin

`OdtLayoutSource.RowHeightsIncludeInsets` reads the setting, beside the five settings this reader
already reads; `OdtLayoutSource.Tables.Row` accumulates a `table:covered-table-cell`'s stated rules
into `PageTableRow.CoveredTopRule`/`CoveredBottomRule` exactly as the three Word readers do; and
`StartsNewPage(styleName)` reads `fo:break-before` off the table's own style.

`odt-table-row-height.fodt` is the converted fixture, so **two readers are now measured against one
set of arms** and `OdtTableRowHeightTests` asserts the same numbers as
`TableCoveredCellRuleTests` does for the `.docx`. 8 tests. Mutation-pinned (`mutate.sh`) over all
four carriers — the setting, the covered top, the covered bottom and the break — 2, 2, 2 and 5 of 8
red respectively, and 8 of 8 green at the base.

## 5. [bin] Confinement: 117 of 1620 renderings move and every one of them is an `.odt`

Our half of the whole corpus, of the 337 converted `.odt` and of the 337 converted `.rtf`, rendered
once at the round's base and once with the fix under `SOURCE_DATE_EPOCH=0`, one output directory per
document. `sweep.sh`, `diff-legs.py`, `confinement.txt`.

| family | moved | of |
|---|--:|--:|
| **odt column** | **117** | 337 |
| rtf column | 0 | 337 |
| words `.docx` / `.doc` | 0 | 337 |
| slides + sheets | 0 | 609 |

**117 moved and 1503 are byte-identical.** A change confined to `OpenDocument/OdtLayoutSource*`
reaching exactly the ODF column is what the layering predicts, and the other 1283 renderings are the
measurement of it rather than the assumption.

## 6. [bin] What it is worth: the gate gains four verdicts and loses none

The 117 movers, each scored against its own 26.2.4.2 rendering with the gate's own rule — equal page
counts, and an alphanumeric difference that fails only when it exceeds **both** 2 % and a floor of 15.
`gatescore.py`, `gatescore.txt`.

| | before | after |
|---|--:|--:|
| gate verdict `match`, of the 117 movers | 92 | **96** |
| verdicts gained / lost | — | **4 / 0** |
| page count closer to 26.2.4.2 / further | — | **6 / 1** |
| mean \|Δy\| per span, median over the documents | 7.2343 pt | **5.7512 pt** |
| documents closer / further on that | — | **87 / 15** |

**No document that matched now fails.** The one page-count regression,
`03_Technical_Report_(progress)_template`, was already failing on glyphs and now fails on pages
instead: 11 against the reference's 10, where the base drew 10 and still failed the character band.

**Fifteen documents are further and the largest is worth naming.**
`gpp-pr-top-7-office-markets-4q-2023.odt` goes 9.33 → 23.45 pt of mean baseline distance; the rest
move by a point or so on documents already 16 to 86 pt out. That is the shape of a pagination change
on a document whose pages do not agree in the first place, and it is not evidence against the rule —
the rule itself is measured at the reference on 34 arms with no free parameter.

## 7. The suite

| project | result |
|---|---|
| `XRay.Office.Core` | 591 passed |
| `XRay.Office.Text` | 750 passed |
| `XRay.Office.Vector` | 309 passed |
| `XRay.Office.Containers` | 109 passed |
| `XRay.Office.WordProcessing` | **2084** passed — 2076 at the round's base plus this round's 8 |
| `XRay.Office.Spreadsheets` | 1484 passed |
| `XRay.Office.Presentations` | 1205 passed |
| `XRay.Office.Markup` | 259 passed |
| `XRay.Office.Fidelity` | **10 failed of 552**, 0 skipped — the round's base figure exactly, and the known set |

Build: 0 warnings, 0 errors.
