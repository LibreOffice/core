# Round 57 — sheets — prediction

Committed **before** a line of the change is written and before anything is rendered
post-change. Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch
`wt-sheets-r57`, base `a45eb8e5391`.

## Baseline, reproduced

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 291 MISMATCH 34`. Scored
against `MANIFEST.tsv`'s 307 sheets paths: **277 match / 30 mismatch**, one document above the
briefed 276.

**The one is `sheets/unstable-001/xlsx/fse_identification_form.xlsx`, and it is the date
volatility, demonstrated inside a single sweep.** The corpus mount gives that file two names, so
`batch-check.sh` rendered it twice minutes apart:

```
fse_identification_form.XLSX   3/3   440/427   3/3   words
fse_identification_form.xlsx   3/3   440/440   3/3   match
```

Our half is pinned by `SOURCE_DATE_EPOCH` at 440 both times; the reference's moved 427 → 440
with the wall clock. So the reproducible baseline is **276 of 307**, exactly as briefed, plus one
document whose verdict is not a property of the code. Every other one of the 31 manifest-`open`
documents mismatches on both spellings and no manifest-`done` document mismatches at all.

## What round 56 left, and what this round found before changing anything

Round 56 measured an **18.46 pt uniform downward translation of the body** on
`fm-provider-service-measures` p36 and **18.49 pt** on `FY2023-AIP-grants` p1, on pages whose
*band* agrees with the reference to 0.0005 pt. Its brief said to "test whether the header height
is being counted twice".

**It is not counted twice. It is not scaled.** Both witnesses are *scaled* worksheets —
`fitToHeight="17"` on `fm-provider` sheet 7 and `scale="43"` on `FY2023` sheet 1 — and

```
ScPrintFunc::GetDocPageSize   (sc/source/ui/view/printfun.cxx:3002)
    aPageRect.SetTop( ( aPageRect.Top() + nTopMargin ) * 100 / nZoom + aHdr.nHeight );
```

builds the page rectangle in **document twips**, in which the margin is divided by the zoom and
the band height is not. A document twip renders at `zoom/100` of a physical twip (`aTwipMode`
carries the zoom as its map-mode fraction, `InitModes`, `printfun.cxx:2645`), so the margin comes
back out at full size and **the band comes out at `nHeight × zoom/100`**.

Arithmetic on both witnesses, done before the probe was run:

| | stated band | nominal | printed band `H` | zoom | `H × (1 − zoom)` | round 56 measured |
|---|---:|---:|---:|---:|---:|---:|
| `fm-provider` sheet 7 | 32.4 pt | 14 (one `&14` line) | ≈ 35.45 | ≈ 0.479 | **18.5** | 18.46 |
| `FY2023-AIP-grants` sheet 1 | 32.4 pt | 33 (three 11 pt lines → **pinned**) | 32.4 | 0.43 | **18.47** | 18.49 |

`probes/sheets-r57/probe-bandscale.py`, five scales, the 100 % control first:

| scale | band text size ref / ours | body token y ref / ours | ours − ref |
|---:|---|---|---:|
| 100 | 14.0 / 14.0 | 56.18 / 56.21 | **0.03** |
| 80 | 11.2 / 11.2 | 49.18 / 56.04 | 6.86 |
| 60 | 8.4 / 8.4 | 42.33 / 55.98 | 13.65 |
| 40 | 5.6 / 5.6 | 35.46 / 55.90 | 20.44 |
| 25 | 3.5 / 3.5 | 30.26 / 55.83 | 25.57 |

Two things that reading gets right and "counted twice" does not: **the band's *text* is already
drawn at the print scale on our side and agrees exactly at all five** (`SheetPageDecoration.
DrawBand` has taken the zoom since it was written), and the residual is
`HeaderHeight × (1 − zoom)` to within 1.5 %, not a constant 18 pt.

`SheetPagination.DocPageSize` already ports the same arithmetic exactly — which is why page
counts match — but its comment says the bands "are printed at full size whatever the sheet's
scale: they are page furniture rather than content", and
**`SheetPrintSetup.PrintableArea`, which is what *places* what a page holds, implements that
sentence instead of the arithmetic.**

## The change

`SheetPrintSetup.PrintableArea` becomes `PrintableAreaAt(double scale)`: the two band terms are
multiplied by the page's own print scale and the four margins are not. One call site changes
behaviour — `SpreadsheetPages`'s `BodyOrigin`, which passes the page's `_scale`. The two
`SheetNotes` call sites pass **1.0 explicitly**, which is what they get today; a note page's own
scale is a separate claim and this round does not measure it (see blind spots).

## Reach, from a census rather than a grep

`probes/sheets-r57/census-bandscale.py` over the 243 xlsx-family sheets documents: **80 have a
banded worksheet, 53 have a worksheet that is both banded and scaled.** Those 53 are the
documents whose body origin can move, and the census lists them.

## Predictions

| | prediction |
|---|---|
| sheets verdicts | **276 → 276 of 307. Zero movement, in either direction.** |
| documents whose body ink moves | **53 xlsx-family, plus an unmeasured number of `.xls`** |
| direction of the movement | **upwards, always** — the body currently starts `H × (1 − zoom)` too low |
| page counts anywhere | **0 change.** `SheetPagination` is not touched and its arithmetic is already the reference's |
| word counts | 0 change on all but at most **two** documents. The only mechanism that can move one is a token that currently falls off the bottom of the paper coming back onto it, and the movement is upwards, so **any change is an increase**. If a count moves down, the reading above is wrong |
| band ink (header/footer text) | **0 change.** The probe shows it already scales and already agrees |
| the 100 % control | **byte-identical.** Every unscaled sheet, and every sheet with no band, must be untouched. A moved 100 % case means the change is measuring something else |
| words / slides tracks | **0.** No shared layer: every file is in `XRay.Office.Spreadsheets` |
| tests | **+6 to +12**, all in `XRay.Office.Spreadsheets` |
| `MANIFEST.tsv` | no row changes status |

## What this census and this probe cannot see — written before the sweep

1. **`.xls` and `.xlsb` are invisible to the census**, which reads OOXML parts only. 64 of the
   307 sheets documents are `.xls`. Round 56's blind spot 1 fired exactly there, twice. `.xls`
   states its scale in a `SETUP` record and its bands in `HEADER`/`FOOTER`, so the arm is real
   and unmeasured — a surprise will land in the sweep, not in the census.
2. **A `fitToPage` sheet's real zoom is not in the file**; it is the bisection's answer. The
   census counts such a sheet as scaled whenever `fitToWidth`/`fitToHeight` is set, so **53 is
   an over-count** of the documents that actually move — every sheet that happens to fit at 100 %
   is in it and moves nothing. A prediction that over-counts hides nothing; one that under-counts
   does, which is why it is written this way round.
3. **The gate cannot see this change at all** on a document whose tokens all stay on the paper,
   so "zero verdict movement" is a weak control and the strong one is
   `probes/sheets-r56/band-agreement.py`'s instrument re-pointed at the *body*.
4. **Whether an off-paper token is extractable is not established.** The prediction that word
   counts can only increase assumes poppler drops text drawn below the MediaBox. If it does not,
   the word counts were never sensitive to this and the prediction is trivially true for the
   wrong reason.
5. **The note-page path is left at scale 1.0 and that is knowingly not the reference's rule.**
   `ScPrintFunc::PrintNotes` uses the same `aPageRect` and the same map mode as `PrintPage`
   (`printfun.cxx:2004-2066`), so a note page's geometry *is* scaled; and separately
   `SheetPage`'s note constructor sets `Placement = default`, which gives
   `SheetPageDecoration` a zoom of 0 and therefore `Math.Max(1, 0) / 100.0` — a **1 % scale** for
   a note page's band. Both are pre-existing. **Zero of the 243 xlsx-family documents set
   `cellComments="atEnd"`**, so the xlsx arm cannot reach it; the `.xls` `SETUP` `fNotes` bit is
   *not* censused and could.
6. **Vertical centring changes too**, because `PrintableArea.Height` grows by
   `(HeaderHeight + FooterHeight) × (1 − zoom)`. That is a second observable on the same change
   and the census does not separate the sheets that centre from the ones that do not.
7. `.ods` is untouched and there is no `.ods` in the sheets corpus, so that arm is unmeasured
   either way — the same standing gap round 56 recorded for `OdsPrintSetup`.
