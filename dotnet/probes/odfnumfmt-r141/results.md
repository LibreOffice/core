# Round 141 — O96: an ODF number style's subformats were not assembled

**Measured against LibreOffice 26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**
(`/opt/libreoffice26.2/program/soffice`), with the tarball's duplicate, Noto Latin, Narrow and
Condensed faces moved aside. Every sweep is pinned with `SOURCE_DATE_EPOCH=0`.

## 1. ODF does not write a multi-section format as one element

It writes **one `number:*-style` per section**, links them from the one the cell names with
`style:map`, and makes the named style's own body the **last** section. `OdfNumberFormat.Code`
walked one element's `number:` children and stopped, so one section was compiled and applied to
every value — and the colour, which ODF states as a `style:text-properties` child rather than as a
keyword in the code, was skipped by the same loop.

The rule is `SvXMLNumFormatContext::CreateAndInsert` and `AddCondition`
(`xmloff/source/style/xmlnumfi.cxx`:1588-1602 and :2130-2186). Four details decide it and three
are not in the specification's prose:

* **The condition must begin `value()`** and only what follows is the comparison (`:2137-2140`).
  One that does not, or that names a style nothing resolves, contributes **nothing at all** — not
  an empty section, not even a semicolon.
* **A single `value()>=0` map writes no condition into the code** (`:2149-2150`): it is the
  ordinary positive/negative pair, and bracketing it would make both sections conditional and
  leave nothing to fall through to.
* **In a `number:text-style` the last map is unconditional too** (`:2152-2156`, whose comment says
  the last condition "can only be all other numbers"). That is how an accounting format arrives:
  the owner is the *text* style holding `@` and its three maps are the numeric sections, so the
  assembled code is the familiar four in the familiar order with the third bare.
* **`!=` is rewritten to `<>`** (`:2161-2166`), once. The decimal separator is localised in the
  same place; this compiles as en-US throughout, so that half is a no-op.

The colour is `AddColor` (`:2196-2216`), which turns the RGB back into a keyword and **inserts it
at the front of the code**, wherever in the element the property sat — and matches it against
`aNumFmtStdColors` (`:212-226`), the same ten in the same order as
`ImpSvNumberformatScan::StandardColor`, which `NumberFormatSection` already reads the other way.
**A colour that is not one of the ten is dropped by the reference as well.**

## 2. The instrument is the reference's own assembled code, not a rendered page

`soffice --convert-to html` writes each cell's format as `sdnum="<language>;<system>;<code>"`, and
that third field is the string `SvNumberFormatter` holds **after** the conditions have been
prepended. So the reference states the answer rather than showing a consequence of it.

`reference-sdnum.tsv` is 26.2.4.2's own six for the fixture. Against this tree's compiler:

| arm | 26.2.4.2's own `sdnum` | ours |
|---|---|---|
| `0.0;[Red]-0.0;[Blue]0.0` | `[>0]0.0;[<0][RED]-0.0;[BLUE]0.0` | **identical** |
| `#,##0 ;[Red](#,##0)` | `#,##0" ";[RED](#,##0)` | **identical** |
| `[>=100]"big" 0;[<0]"neg" 0;0.0` | `[>=100]"big "0;[<0]"neg "0;0.0` | **identical** |
| `[COLOR10]0.0` | `0.0` | **identical** |
| `0.00` | `0.00` | **identical** |
| `_(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)` | `[>0]_(* #,##0_);[<0]_(* (#,##0);_(* "-"_);_(@_)` | same **shape**, `_` and `*` missing |

Five of six character for character. The sixth agrees in structure — three conditions with the
last bare, the text-style owner's body last — and differs only in the padding directives:
`number:fill-character` reaches no branch of `Append` and `loext:blank-width-char` is unread, so
each becomes the literal space the element carries. Seated as **O99** rather than folded in.

## 3. The fixture

`features/sheet-odf-numfmt-sections.ods` is **26.2.4.2's own ODS export** of a workbook stating
those six codes (`make-fixture.py` writes the `.xlsx` and converts it), because the thing under
test is how LibreOffice's exporter writes a multi-section format and no hand-authored file can be
trusted to reproduce it. Six columns, one per format; four rows — 150, −100, 0 and the string
`text` — so a cell's address states which arm it is.

Rendered both ways, the negative row and the text row are **identical span for span and colour for
colour**, and the positive and zero rows differ only in that the reference emits the accounting
column's fill run as one span where this tree emits two — the same padding gap as above. Before
the fix, this tree drew one section on every cell of every column.

The round-139 fixture converted to `.ods` is the second witness and is exact:

| | `1.5` | `-2.5` | `0.0` | `-2.50` |
|---|---|---|---|---|
| 26.2.4.2 | #000000 | **#ff0000** | **#0000ff** | #000000 |
| ours, before | #000000 | #000000 | #000000 | #000000 |
| ours, after | #000000 | **#ff0000** | **#0000ff** | #000000 |

## 4. Reach, censused by what a style PAINTS

`/home/user/corpus-odf` did not survive the container rebuild; `convert-ods.sh` re-made the `.ods`
column with 26.2.4.2 — **307 of 307, none failed**. `census.py` over it:

| | documents | cells |
|---|---:|---:|
| hold a `number:*-style` with a `style:map` | **307** | — |
| have a **cell** naming one | 92 | 360 426 |
| hold a cell whose **value selects a mapped section** | **64** | 128 964 |
| hold a cell taking one of the ten keyword colours | **8** | 110 |

**The first row is the trap and it is worth more than the others.** Every file LibreOffice exports
carries its built-in format list — `N129`…`N147`, the currency and accounting formats — whether or
not a cell uses one, so "how many documents state a `style:map`" answers *307 of 307* and means
nothing. The figure to quote is the third: a cell exists whose own value selects a section the old
reader could not reach. That is C9 in this file's own words, met head-on.

## 4a. Reach, measured: 21 renderings move and two gate verdicts are gained

Both legs rendered the whole 307-document `.ods` column under `SOURCE_DATE_EPOCH=0`, three at a
time, one output directory per document, with no rebuild in flight:

**21 renderings move, 286 byte-identical, 0 failed on either leg, and no page count moves
anywhere** — before, after and reference agree on every page count of all 21.

**Two gate verdicts are gained and none lost**: `044_Cash_flow_forecast` and
`006_Contextures_chart_sample` go `glyphs` → `match`, the second landing within **1** alphanumeric
character of the reference (796 against 795, from 771).

Colour, counted as spans drawn in anything but black, summed |ours − reference| over the 21:
**1158 → 1127**. Five documents improve — `041_Business_budget` 34 → 37 of 49,
`018_Weight_Loss_Chart` 389 → 403 of 465, `060_Monthly_company_budget` 32 → 45 of 48,
`069_Blue_modern_balance_sheet` 86 → 87 of 89 — and **none worsens**.

Alphanumeric characters, same measure: **1659 → 1880**, which is worse, and the reason is worth
more than the number. Five improve, thirteen are unchanged, and **three worsen** — and all three
are documents where this tree already draws chart content the reference does not:

* **`057_Simple_balance_sheet`** page 3: 414 → 492 against the reference's 112. This is the
  **raster ceiling** this project already has on file for this exact document — 26.2.4.2 draws
  that page's turned chart labels as filled outlines, so they score **zero** in the text layer,
  and its path count there is **361 against our 84**. Our output is the better one and the gate
  scores it worse.
* **`019_Free_Blood_Sugar_Chart`** and **`018_Weight_Loss_Chart`**: the pages that get *closer*
  are the chart pages (019 pages 1, 4 and 7; 018 pages 4 and 8, which become **exact**), and the
  pages that get worse are ones where we draw a chart the reference does not put there at all.
  **The vector path counts are identical before and after on both documents** and already two to
  four times the reference's on every page, so the placement gap predates this round; what changed
  is that its labels now carry text. On 019 page 2 the added spans are `11/1`…`11/6`, `0:00` and
  `12:00` — correctly formatted dates and times, drawn where the reference draws none.

So the alphanumeric column moves the wrong way on documents whose *chart geometry* is already
wrong, and the column that can see this change cleanly — the colour one — moves the right way on
five and the wrong way on none.


## 5. Pinned by mutation

Three mutations, each against the restored fix and each rebuilt:

| mutation | OpenDocument (21) | Spreadsheets (4) |
|---|---|---|
| `maps` forced empty — `style:map` not followed | **13 fail** | **3 fail** |
| the `fo:color` prefix never inserted | **11 fail** | **2 fail** |
| `bare` forced false — every condition bracketed | **3 fail** | — |

The third is the one worth having: it fails exactly the three arms that depend on the two
unconditional-section rules — the single `value()>=0` map, the reference's own `sdnum` for it, and
the accounting format's bare third section — and leaves the other eighteen green, which is what
says those tests are pinning that rule rather than passing by association.

Restored with `cp` + `touch` (never `mv`: the up-to-date check skips a project whose source looks
older than its assembly) and rebuilt: 21 and 4 of 4 pass, and no `MUTATION` marker survives in the
source.


## 6. Suite

Each project run on its own and totalled by hand.

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 591 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 750 | 0 | 0 |
| Vector | 309 | 0 | 0 |
| Rendering | 164 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 190 | 0 | 0 |
| WordProcessing | 2010 | 0 | 0 |
| Spreadsheets | 1416 | 0 | 0 |
| Presentations | 1205 | 0 | 0 |
| Fidelity | 542 | **10** | 0 |

OpenDocument 169 → 190 and Spreadsheets 1412 → 1416 are this round's own tests. The ten fidelity
failures are the documented set and are unchanged — four `PageDrawingComparisonTests`, four
`TabStopComparisonTests`, one `JustificationShrinkComparisonTests` and one
`SheetDrawingComparisonTests`. **0 skipped on Fidelity**, which is what says the reference binary
was reachable.


## 7. What this round did not do

* **`FormatKind` still reads the owner element's name, not the assembled code — and that turned
  out not to matter.** It was seated as **O98** on the reasoning that an accounting format's owner
  is a `number:text-style`, so an ODF numeric cell under one answers `NumberFormatKind.Text` where
  `XlsxCellFormats` would answer `Number`, and that this decides the `###` rule. **Both halves of
  that were wrong and the probe took eight minutes.** The `###` rule does not consult the kind at
  all; its only consumer is `SheetTextLayout.Breaks`, which decides whether a *wrapping* numeric
  cell line-breaks. Three one-attribute variants of the fixture — four-character columns, six
  with `wrapText` everywhere, and the same with the values widened to 1 234 567.89 — agree with
  26.2.4.2 **cell for cell and row height for row height on all three**, because `###` fires
  before the wrap can. The corpus case exists in quantity and still shows nothing: 31 documents
  and 102 690 numeric cells under a text-style owner, 7 documents and 75 402 of them wrapping.
  **O98 is now a nil-reach entry, refuted the day it was seated.** The probe builders are
  `o98-narrow-fixture.py`, `o98-wrap-fixture.py` and `o98-wrapbig-fixture.py`, and the census is
  `o98-wrap-census.tsv`.

  *The general point is worth more than the entry.* The seat was written from reading the source
  and not from running it, and it named a property — `ShowsHashesWhenTooNarrow` — that **does not
  exist in this tree**. A seat is a claim, and a claim that cites an identifier should have been
  grepped for.
* **`number:fill-character` and `loext:blank-width-char`** — **O99**, above.

## 8. Files

| file | what it is |
|---|---|
| `make-fixture.py` | writes the `.xlsx` and converts it with 26.2.4.2; the `.ods` is the committed fixture |
| `census.py`, `census.tsv` | the three-level reach census over the converted `.ods` column |
| `reference-sdnum.tsv` | 26.2.4.2's own assembled code for each of the fixture's six styles |
| `convert-ods.sh` | re-makes `/home/user/corpus-odf/ods` from the corpus with 26.2.4.2 |
| `par-sweep.sh` | renders the column three at a time, one output directory per document |
| `fingerprints-before.txt`, `fingerprints-after.txt` | md5 of all 307 renderings, each leg |
| `movers.tsv` | the renderings that move, with pages, alphanumerics and coloured spans |
| `o98-*-fixture.py`, `o98-wrap-census.tsv` | the three variants and the census that refuted O98 |
