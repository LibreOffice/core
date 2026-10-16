# Round 56 — sheets — prediction

Committed **before** a line of the change was written and before anything was rendered
post-change. Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch
`wt-sheets-r56`, base `d968553554e`.

## Baseline, reproduced first

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 290 MISMATCH 35`. Scored against
`MANIFEST.tsv`'s 307 sheets paths (the raw total double-counts 18 case-alias entries):
**276 match / 31 mismatch, and zero disagreements with the manifest's status column in either
direction.** The brief's 276 of 307 reproduces to the document.

## What the round found before writing anything

**The `PAGE n OF 33` string is in an `oddHeader` after all**, and round 55's "the string is in no
cell, no `oddHeader` and no `oddFooter`" is wrong. `xl/worksheets/sheet10.xml` (the *ACC list*
sheet, which is pages 28–32) states

```
<oddHeader>&amp;R\n\n\n\n\n\n\n&amp;9PAGE \n&amp;P OF &amp;N</oddHeader>
```

— the right area, **seven empty leading lines**, then `PAGE ` and `28 OF 33` on two more. Four
tokens a page × five pages = the twenty. A grep for `PAGE.*OF` cannot see it because the string is
split by a newline and a `&9`.

And the reason the reference draws none of it is **not** a threshold. It is a rectangle:

* `PageSettingsConverter::convertHeaderFooterData` (`sc/source/filter/oox/pagesettings.cxx:1030-1041`)
  sets `mnBodyDist = statedBand − textHeight`; negative sets `mbDynamicHeight = false` and pins
  `mnHeight` at the stated band — here 200 mm100 = **5.67 pt** against a 98 pt header.
* `ScPrintFunc::UpdateHFHeight` (`printfun.cxx:789-793`) returns immediately for a pinned band, so
  5.67 pt survives to print time.
* `ScPrintFunc::PrintHF` (`printfun.cxx:1870`) sets a **clip region** of exactly
  `Rectangle(aStart, Size(nLineWidth, nHeight − nDistance))`.
* `ImpEditEngine::DrawText_ToPosition` (`editeng/source/editeng/impedit3.cxx:3367-3372`) takes the
  area's whole primitive range and **returns having emitted nothing at all** when it does not
  overlap that clip — not ink, and not PDF text.

`probe-bandclip.py`, nine authored fixtures with the control first, measures this on the binary:

| case | band | reference | our base |
|---|---:|---|---|
| A control, 11 pt, one line | 28.80 | drawn at y 25.20 | 25.25 |
| B 8 pt | 1.44 | **absent** | drawn |
| B 8 pt | 2.16 | drawn | drawn |
| C 20 pt | 4.32 | drawn | drawn |
| C 20 pt | 5.76 | drawn | drawn |
| D two 11 pt lines, band 14.4 | 14.40 | **both** at 21.6 / 33.9 | 21.65 / 32.85 |
| D two 11 pt lines, band 36 | 36.00 | both at 21.6 / 33.9 | 21.65 / 32.85 |
| E the FAA shape | 5.67 | **neither** | both, at 92.5 / 102.5 |
| E the same, roomy band | 101.03 | both at 100.3 / 110.3 | 92.5 / 102.5 |

Three things fall out and each is a claim this round is betting on:

1. **The threshold is `ascent − capHeight`** — how far below a line's top its ink starts, 0.217 em
   for Liberation Sans: 1.74 pt at 8 pt and 4.34 pt at 20 pt. That reproduces round 55's 8 pt
   bracket exactly with nothing fitted. **It refutes round 55's 20 pt bracket**: 4.32 pt draws.
2. **The clip is per *area*, not per line** (case D). A two-line area whose second line's ink is
   below the band keeps **both** lines in the PDF, at the positions a roomy band gives them. So
   "overflows downwards" is right; "is cropped line by line" is not.
3. **A 5.67 pt band is far above every threshold round 55 bracketed and still draws nothing**,
   because seven empty lines put the ink 90 pt down. A threshold could never have predicted case E.

### And a second, larger defect the same probe found

The reference draws band text in **the workbook's own default cell font**; we draw it in a
hard-coded ten-point Liberation Sans. Five sizes and five families, roomy band, one line each:

| workbook default | reference `ZZTOPZZ` box | ours |
|---|---|---|
| Liberation Sans 8 | 508.90 … 544.81 | 500.09 … 544.90 |
| Liberation Sans 10 | 499.95 … 544.89 | 500.09 … 544.90 |
| Liberation Sans 11 | 495.45 … 544.89 | 500.09 … 544.90 |
| Liberation Sans 14 | 481.95 … 544.84 | 500.09 … 544.90 |
| Liberation Sans 20 | 454.90 … 544.82 | 500.09 … 544.90 |
| Calibri 12 | `Carlito` | `LiberationSans` **plus** a stray `Carlito` from the body |
| Times New Roman 12 | `LiberationSerif` | `LiberationSans` |
| Courier New 12 | `LiberationMono` | `LiberationSans` |

Our column is **identical at every size and every family**. `SheetPageDecoration.SizeOf` falls back
to `SheetBandText.DefaultSize` (10 pt) and `SheetBandText.Shape` to `DefaultFamily`, while
`SheetBandHeight` — which sizes the band — already uses the workbook's default font. The two halves
of the same file disagree.

## The change

**A. The band clips.** `SheetPageDecoration.DrawBand` builds each area's runs, takes their union
ink range, and draws the area only if that range overlaps the band rectangle
`[left,right] × [top, top+height]` — which is `PrintHF`'s own `Rectangle(aStart, aPaperSize)`.
All-or-nothing per area, per case D. Ink top is modelled as `baseline − capHeight` from the face's
`OS/2`.

**B. The band's face is the workbook's default cell font** — family, size, weight and posture — and
the `&"Family,Style"` code `SheetHeaderFooter.ParseCodes` already reads and currently drops is
carried on the piece. A new `SheetPrintSetup.BandFont`, set by `XlsxPrintSetup`, `XlsbPrintSetup`
and `XlsPrintSetup` from the same `SheetDefaultFont` they already build; `OdsPrintSetup` is **not**
touched, so ODF bands are unchanged.

## Predictions

| | prediction |
|---|---|
| sheets verdicts | **276 → 276 of 307. Zero movement, in either direction.** |
| `FAA-2019-0995-0002_attachment_2` | 33/33 pages held; words **10015 → 9995**, exactly the reference's; `match` held before and after |
| any page count anywhere | **0 change** — `HeaderHeight`/`FooterHeight` come from `SheetBandHeight`, which already uses the right font and is not touched |
| any other document's word count | **0 change** |
| documents whose band *ink* moves (change B) | **81 xlsx/xlsm documents** have band content, and all but the few at Liberation-Sans-10 move; plus an unmeasured number of the 64 `.xls` |
| worksheets whose band is clipped away by change A | **exactly one** — `FAA-2019-0995-0002_attachment_2` `sheet10` `oddHeader`, right area |
| words track | **0** — no shared layer touched |
| slides track | **0** — no shared layer touched |
| tests | +12 to +18 in `XRay.Office.Spreadsheets`, no other project |

`census-bandclip.py` finds 313 worksheet bands with content, 26 of them pinned, and 7 with an area
whose ink falls entirely outside the band. **Six of the seven are already right**: five state a band
of exactly zero and one a negative band, and `SheetPageDecoration`'s existing `HeaderHeight >
Length.Zero` guard plus `XlsxPrintSetup`'s `Math.Max(0, …)` already draw nothing there. The seventh
is the FAA sheet. So change A's whole corpus reach is one worksheet, and that is why the verdict
prediction is zero.

## What these censuses cannot see — stated before the sweep

1. **`.xls` is invisible to both censuses.** 64 of the 307 sheets documents are BIFF; their bands
   come through `XclImpHFConverter`, which does the same arithmetic in a different reader that
   `census-bandclip.py` does not parse. Change B *will* move their band ink because
   `XlsPrintSetup` already builds a `SheetDefaultFont`. If a BIFF workbook has a pinned band with
   empty leading lines, change A will suppress it and no census here will have predicted it. The
   whole-track sweep is the only instrument that covers them, and any surprise will land there.
2. **Cap height is a proxy for the glyph bounding box.** LibreOffice takes the actual primitive
   range; I take `ascent − capHeight`, which is exact for capitals and too high for text that is
   all x-height, so right at the boundary I will draw where the reference clips. No corpus case is
   within 80 pt of the boundary, so this cannot bite today — but it will be the first thing to
   suspect if a band goes missing that should not.
3. **Case D is one geometry.** "Per area, all or nothing" is measured on a two-line right area at
   one band height. A band where one of the three areas is clipped and the others are not is not
   separately measured, and neither is an area that overlaps the clip only at its very last line.
4. **The `&"Family,Style"` arm is measured on fixtures and not on its 17 corpus documents.**
   119 worksheet bands in 17 documents state a face code. Implementing it changes those bands from
   "always Liberation Sans" to "the named family", which is right in general and could be wrong in
   a case where the named family resolves differently from the way Calc resolves it.
5. **The gate cannot see change B at all.** Its three checks are pages, extractable words and *our
   own* unembedded fonts — not a comparison of font sets. A change that moves band ink on 81
   documents is invisible to it in both directions, so "no verdict movement" is a weak control
   here and I am measuring band token positions directly instead (`band-agreement.py`).
6. **Bold and italic defaults are implemented from the same record and are not yet measured.** The
   probe varied family and size only.
7. **`evenHeader`/`evenFooter`** are folded into the pin decision but the round has not separated
   which area prints on an even page.

## What would refute this round

* Any `.xls` document losing a header or footer it should keep.
* A word count moving on a document other than `FAA-2019-0995-0002_attachment_2`.
* `band-agreement.py` getting *worse* after change B on documents whose default font is not
  Liberation Sans 10.
