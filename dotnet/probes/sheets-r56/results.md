# Round 56 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r56`, base
`d968553554e`. Read `prediction.md` (`015ecc19c73`) beside this file first: it was committed
before a line of the change was written and before anything was rendered post-change.

## 1. Baseline, reproduced to the document

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 290 MISMATCH 35`. Scored against
`MANIFEST.tsv`'s 307 sheets paths (the raw total double-counts 18 case-alias entries): **276 match
/ 31 mismatch, zero disagreements with the manifest in either direction.**

## 2. Result

**276 → 276 of 307. Zero verdict movement, which is what the prediction file said.** No document
regressed and none closed. What moved is ink, and a great deal of it.

| | base | after |
|---|---|---|
| `FAA-2019-0995-0002_attachment_2` words | 10015 / ref 9995 | **9995 / ref 9995** |
| rows where our font count moved | — | **46, every one towards the reference; 0 away** |
| documents whose band agreement moved | — | **41 of 81; 39 improved, 2 worsened** |
| median per-document band `\|dx\|+\|dy\|` | 1.515 pt | **0.220 pt** |
| tests | 4795 / 0 failed / 1 skipped | **4811 / 0 / 1** |

## 3. The `PAGE n OF 33` blocks: the string is in an `oddHeader`, and round 55 said it was not

`xl/worksheets/sheet10.xml` — the *ACC list* sheet, which is pages 28 to 32 — states

```
<oddHeader>&amp;R\n\n\n\n\n\n\n&amp;9PAGE \n&amp;P OF &amp;N</oddHeader>
```

the right area, **seven empty leading lines**, then `PAGE ` and `28 OF 33`. Four tokens a page ×
five pages = the twenty. Round 55's "the string is in no cell, no `oddHeader` and no `oddFooter`"
is **wrong**, and it is wrong for an instructive reason: a grep for `PAGE.*OF` cannot see it,
because the string is split by a newline and a `&9`. *The interesting part of that sentence was
the part that was an artefact of the search.*

### 3.1 There is no threshold. There is a clip rectangle.

Round 55 bracketed a "text-fit threshold … about 0.27× the point size" and reported it as an
unexplained law. The mechanism is four calls long and every one of them is in the reference's own
source:

| | |
|---|---|
| `PageSettingsConverter::convertHeaderFooterData` (`sc/source/filter/oox/pagesettings.cxx:1030-1041`) | `mnBodyDist = statedBand − textHeight`; negative sets `mbDynamicHeight = false` and pins `mnHeight` at the stated band |
| `ScPrintFunc::UpdateHFHeight` (`printfun.cxx:789-793`) | returns immediately for a band that is not dynamic, so the pinned height survives to print time |
| `ScPrintFunc::PrintHF` (`printfun.cxx:1870`) | sets a **clip region** of exactly `Rectangle(aStart, Size(nLineWidth, nHeight − nDistance))` |
| `ImpEditEngine::DrawText_ToPosition` (`editeng/source/editeng/impedit3.cxx:3367-3372`) | takes the area's whole primitive range and **returns having emitted nothing at all** when it does not meet the clip; when it meets it partly, wraps the area in a `MaskPrimitive2D` and keeps every line |

`probe-bandclip.py` — nine authored fixtures, the control first — puts all of it on the binary:

| case | band | reference | ours before | ours after |
|---|---:|---|---|---|
| A control, 11 pt, one line | 28.80 | drawn, y 25.20 | 25.25 | 25.25 |
| B 8 pt | 1.44 | **absent** | drawn | **absent** |
| B 8 pt | 2.16 | drawn | drawn | drawn |
| C 20 pt | 4.32 | **drawn** | drawn | drawn |
| C 20 pt | 5.76 | drawn | drawn | drawn |
| D two 11 pt lines | 14.40 | **both**, 21.6 / 33.9 | 21.65 / 32.85 | 21.65 / **33.95** |
| D two 11 pt lines | 36.00 | both, 21.6 / 33.9 | 21.65 / 32.85 | 21.65 / 33.95 |
| E the FAA shape | 5.67 | **neither** | both, 92.5 / 102.5 | **neither** |
| E the same, roomy | 101.03 | both, 100.3 / 110.3 | 92.5 / 102.5 | **100.25 / 110.26** |

Three findings, and each is a claim the round is standing on:

1. **The "threshold" is `ascent − inkAscent`** — how far below a line's top its ink starts. A
   bisection in 0.1 pt steps at three sizes, with the mm100 rounding the margins go through taken
   into account, brackets it at **0.2056 to 0.2087 em**: 8 pt turns over between 1.59 and 1.70 pt,
   11 pt between 2.21 and 2.30, 20 pt between 4.11 and 4.20. Liberation Sans' bare
   `ascent − capHeight` is 0.2173 em, **outside that bracket on the wrong side**; two per cent of
   round-capital overshoot puts it at 0.2035, outside on the side that draws. That is what
   `SheetBandText.RoundCapitalOvershoot` is, and it is deliberately biased towards drawing.
2. **Round 55's 20 pt bracket is refuted.** It recorded "between 4.32 and 5.76 pt for 20 pt text";
   4.32 pt draws.
3. **The clip is per *area*, not per line** (case D). A two-line area whose second line's ink is
   below the band keeps **both** lines, at the positions a roomy band gives them. The existing
   `SheetSmallBandTests.AFooterThatOverflowsThePaperLeavesItsLastLineOffThePage` is the test that
   detects the per-line mutation, and it detects it because it was already there.

**Case E is the one a threshold could never have produced.** 5.67 pt is comfortably above every
bracket round 55 measured, and nothing is drawn, because seven empty lines put the ink 90 pt down.

### 3.2 And a second, larger defect the same probe found

The reference draws band text in **the workbook's own default cell font** — family, size, weight
and posture. We drew it in a hard-coded ten-point upright Liberation Sans.

| what varies | reference | ours before | ours after |
|---|---|---|---|
| default size 8 / 10 / 11 / 14 / 20 | run starts at 508.90 / 499.95 / 495.45 / 481.95 / 454.90 | **500.09 for all five** | 509.05 / 500.09 / 495.61 / 482.17 / 455.28 |
| default family Calibri / Times New Roman / Courier New / DejaVu Serif | `Carlito`, `LiberationSerif`, `LiberationMono`, `DejaVuSerif` | `LiberationSans` every time | the reference's, every time |
| `<b/>` on `fonts[0]` | `LiberationSans-Bold` | `LiberationSans` | `LiberationSans-Bold` |
| `<i/>` on `fonts[0]` | `LiberationSans-Italic` | `LiberationSans` | `LiberationSans-Italic` |
| `&"Liberation Sans,Bold"` | band bold, body regular | both regular | both, as the reference |
| `&B` | band bold | regular | bold |
| `&"…,Regular"` over a bold `fonts[0]` | band **upright**, body bold | both regular | both, as the reference |

`SheetBandHeight` — the file that decides how *tall* the same band is — has taken the workbook's
default font since it was written, and reads the `&"Family"` code to do it. `SheetPageDecoration`
read neither. **The two halves of the same band disagreed on 81 of the corpus's workbooks.**

**A measurement that reads the wrong way if you take the obvious observable.** The weight and the
slant have to be keyed on the PDF's *font list*, not on advance widths: the Liberation faces are
metric-compatible, so a bold band is exactly as wide as an upright one and `&"-,Bold"` moves no
token by a thousandth of a point. A first pass at this probe measured x positions, saw them
identical, and concluded the reference ignores the style code. It does not.

## 4. Prediction against measurement

| | predicted | measured |
|---|---|---|
| sheets verdicts | 276 → **276**, zero movement either way | **276, zero movement** |
| `FAA-2019-0995-0002_attachment_2` | 33/33; words **10015 → 9995**, exactly the reference | **33/33; 9995, exactly the reference**, on both case aliases |
| page counts anywhere | 0 change | **0 change** |
| any other document's word count | 0 change | **WRONG — four moved**, all tokenisation and none content (§ 6) |
| worksheets clipped by change A | exactly one, the FAA `sheet10` `oddHeader` | **exactly one** |
| documents whose band ink moves | 81 xlsx/xlsm plus an unmeasured part of the 64 `.xls` | **41 of the 81 measurably**, plus 4 `.xls` |
| words / slides tracks | 0 | **0 — no shared layer touched** |
| tests | +12 to +18 in `XRay.Office.Spreadsheets` only | **+16, `XRay.Office.Spreadsheets` only** |

**Seven of eight.** The miss is the one the prediction's own blind-spot list is nearest to but
does not name: it said the gate cannot see change B, and then predicted no word count would move
anywhere. Four did, and all four are pdftotext's word boundaries moving because a run now splits
where the face or the size changes — `PageII:6Architecture` became `Page …`, which is what the
reference has. **Blind spot 1 fired exactly where it was pointed**: three of the four are `.xls`,
the arm neither census could see.

## 5. Blind spots, scored

| # | what it said | what happened |
|---|---|---|
| 1 | `.xls` is invisible to both censuses; a surprise will land in the sweep | **fired.** `NPIAS_App_A.xls` came out of the first sweep emitting a font the reference does not, and three of the four word-count movements are `.xls` |
| 2 | cap height is a proxy for the glyph box; wrong at the boundary in the direction that deletes text | **fired, and was caught by the probe before the corpus saw it.** The bare `ascent − capHeight` suppresses the 4.32 pt case that the reference draws; the overshoot allowance exists because of it |
| 3 | case D is one geometry | untested elsewhere; still open |
| 4 | the `&"Family,Style"` arm is measured on fixtures, not on its 17 corpus documents | still true, and now larger: the style arm turned out to carry weight and slant as well |
| 5 | the gate cannot see change B; "no verdict movement" is a weak control | **correct, and it is why `band-agreement.py` exists** |
| 6 | bold and italic are implemented from the record and not measured | **fired within the round.** They were not implemented at all in the first cut, and `NPIAS_App_A.xls` is what said so |
| 7 | `evenHeader`/`evenFooter` not separated | still true |

## 6. The four word counts that moved, and none of them is content

| document | ours before → after | reference |
|---|---|---|
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 23611 → 23618 | 23513 |
| `FY2023-AIP-grants.xlsx` | 51041 → 51039 | 51045 |
| `orbus_togaf_tool_csq.xls` (both aliases) | 46837 → 46836 | 46780 |

All three are the same effect and it is worth stating in the direction that matters. Our base
glued header tokens together — `PageII:6Architecture`, `PageIII:`, `11ADM` — because a whole area
was one run at one size; now the run splits where the face or the size changes and the tokens come
apart. The reference has `Page 6 Score 0` where we had `PageII:6Architecture` and now have
`Page …`, so **the tokenisation moved towards the reference while the count moved away from it**.
That is a case where the gate's own metric is the worse of the two available readings, and it is
recorded here rather than smoothed over.

`FY2023-AIP-grants`'s footer moved from **1.86 pt out to 0.11 pt out** at the same time.

## 7. The vision round: three reviewers, two findings confirmed and two refuted

Three fresh subagents, each given one composed page and nothing else — no project documents, no
source, no shell — and asked to describe each half separately before comparing.

**Confirmed by a second instrument:**

* `FAA-2019-0995-0002_attachment_2` p28 — "grey cell shading on the *MISSING* rows in the
  reference and not in ours". `pdf-ops.py --only fill` says the reference issues **three `#C0C0C0`
  fills** spanning x 214–754 and we issue **zero** non-white fills on that page. Real, and
  unworked.
* `fm-provider-service-measures` p36 — "our table body starts lower and its last row collides with
  the footer; the reference's does not". `pdftotext -bbox`: our first body token is at y **62.41**
  against the reference's **43.95**, and the last at 757.88 against 739.42 — a uniform
  **18.46 pt** downward translation of the whole body, on a page whose header token agrees to
  0.0005 pt. **`FY2023-AIP-grants` p1 has the same defect at 18.49 pt**, and it is pre-existing:
  base and after are identical on it.

**Refuted, both by an instrument that answers the exact claim:**

* "The reference runs `FILTER ASSY HYD291143 CMM` together where we break the line." Both sides
  break it identically, at the same place. **This is the second round running that a reviewer has
  reported this same difference on this same cell and been wrong** — round 55's reviewer said it
  first and § 5 of that round's results refuted it. A phantom that reproduces across reviewers is
  worth naming: it is the strongest instance yet of `HANDOVER.md` § 7.
* `NPIAS_App_A` p12 — "we draw full vertical column rules; the reference draws horizontal rules
  only". We draw **22** vertical strokes on that page and the reference draws **24**. The claim is
  not merely wrong, it points the wrong way.

## 8. The 24.2.7.2 audit — two sites re-checked, both VERIFIED, and one re-marked

| site | outcome |
|---|---|
| `Layout/SheetShapeText.cs` `DefaultSize` | **VERIFIED on 26.2.4.2.** 12 pt by two instruments — the flat-ODS export gives the bare run `fo:font-size="12pt"`, and the *rendering* gives it an ink height of 13.274 pt against 12.175 for `sz="1100"` and 19.926 for `sz="1800"`; 12/11 and 12/18 both give 13.28. **The 1100 control ran first**, and the 1800 box is there so that a reader which always answers 12 can be told from one that reads the shape's own 18 pt default. `audit_shapetext.py` |
| `Layout/SheetNotes.cs` (column-major note order) | **VERIFIED on 26.2.4.2.** `Hazard Analysis Template.xls` still lists `D1 F2 H2 J2 L1 N2 P2 R2` and ours lists the same eight in the same order. The claim discriminates: reading order would put `L1` second, and both put it fifth |
| `Layout/SheetPageDecoration.cs` | **re-marked.** Round 55 marked it `WRONG` and reported rather than implemented. This round replaces the fitted law with the mechanism and implements it |

Counters, re-derived with the file's own commands at this tree: **42 open sites, 17 marker lines
(15 `VERIFIED`, 2 `WRONG`, 0 `UNDECIDED`)**. The file said `14 — 9 verified, 3 wrong, 1 undecided,
1 half-wrong`; none of those four figures reproduces, at this tree or at the base (**42 / 15 / 13 /
2 / 0**). Corrected there, for the fourth time in that file's history.

**And one of the three commands that file gives is wrong.** `git grep -c "audit: X" -- …` prints
`path:count` over the working tree, so `awk -F: '{s+=$2}'` is right — but add a tree-ish and the
output becomes `commit:path:count`, `$2` is the path, and the total comes out **0**. This round
read "0 markers at the base commit" off it before noticing. Fixed at the file.

`XRay.Office.Spreadsheets` is now **eight of ten** re-checked, seven correct. The one found wrong is
still the only one that was about **page furniture** rather than text metrics — which after two
rounds is a pattern rather than a coincidence, and it is why the file now says to take
`XlsxNoteCaptions.cs` (furniture) before `SheetText.cs` (metrics).

## 9. `ChartLayout.IntervalsThatFit` — censused, not touched

`census-autoaxis.py` over all 946 documents: **309 `chartSpace` parts, 256 value axes with an
automatic major unit against 30 with a stated one, in 129 documents — 79 sheets, 47 slides, 3
words.** A stated major unit is inert to this rule, so 256 axes across all three tracks is its
reach, and `ChartLayout` is in **`XRay.Office.Core`**. The brief's "census first" was right: this is
not four tokens on `005`, it is a law over 129 documents and a shared layer.

**What the round established about it without changing a line.** The reference's
`estimateMaximumAutoMainIncrementCount` (`chart2/source/view/axes/VCartesianAxis.cxx:1559-1618`)
is `nTotalAvailable / nSingleNeeded` in **integer 1/100 mm**, where `nTotalAvailable` is
`|aEnd − aStart|` of the *axis main line* and `nSingleNeeded` is `m_nMaximumTextHeightSoFar` —
which `VAxisBase::recordMaximumTextSize` fills from **`ShapeFactory::getSizeAfterRotation` of the
label's `SvxShape`**, not from the text.

Three candidate divergences, and one of them is already bounded:

1. **The mm100 truncation is nearly ruled out by arithmetic.** Truncating the divisor can only
   raise the quotient by a factor `1 + 1/floor(needed)`, which for an 11.5 pt label (405 mm100) is
   0.25%. It can turn 8 into 9 only if the exact ratio lies in `[8.978, 9)`. Possible, and narrow
   — worth *measuring* on `005` rather than assuming, but it is not a general explanation.
2. **`available`**: we use the plot rectangle's height where the reference uses the axis line's.
3. **`needed`**: we measure the *text*, and the site argues for that from two corpus outcomes
   (`54.6/11.5` and `108.8/11.5`), while `recordMaximumTextSize` reads the *shape*. Note the sign:
   the reference gets **more** intervals than we do on `005`, so its divisor is smaller or its
   dividend larger — **the shape's insets cannot be the explanation**, because they would make its
   divisor larger. Whoever takes this should start from that, and from authored charts with a
   known axis length bisected until the count changes, rather than from `005`.

## 10. Shared layer

**No.** Every file this round touches is in `XRay.Office.Spreadsheets`:
`Layout/SheetPageDecoration.cs`, `Layout/SheetBandText.cs`, `Layout/SheetHeaderFooter.cs`,
`Layout/SheetPrintSetup.cs`, `Layout/SheetFonts.cs`, `Layout/SheetShapeText.cs`,
`Layout/SheetNotes.cs`, `Ooxml/XlsxPrintSetup.cs`, `Xlsb/XlsbPrintSetup.cs`,
`MsBinary/XlsPrintSetup.cs`. The words and slides tracks cannot see it and **no cross-track sweep
is owed**. `OdsPrintSetup` is deliberately untouched, so an ODF band keeps the application default
it had; there is no `.ods` in the sheets corpus and that arm is unmeasured either way.

## 11. Tests

**+16, all in `XRay.Office.Spreadsheets`** (940 → 956). Whole tree **4811 passed, 0 failed, 1
skipped** against the base's 4795 / 0 / 1. `XRay.Office.Fidelity` is **521 passed / 31 failed /
552**, byte-for-byte the base's figure. `dotnet build -v q -nologo` → **0 warnings, 0 errors.**

**Eight mutations run through `verify-test.sh`, seven detected:**

| mutation | detected by |
|---|---|
| the clip removed | `AnAreaWhoseInkFallsBelowItsBandIsNotDrawnAtAll` |
| the clip made per *line* (ink top taken from the last line, not the first) | **`SheetSmallBandTests.AFooterThatOverflowsThePaperLeavesItsLastLineOffThePage`** — an existing test, and this is why the clip is safe for a footer whose lines run past its band |
| ink top taken as the line box top rather than the ink | `ABandNarrowerThanTheDistanceToTheInkDrawsNothing` |
| the band's default size reverted to a fixed ten point | three at once |
| the band's default family dropped | two |
| the face code dropped by the parser again | four |
| the section switch stops resetting the face | `AFaceCodeIsKeptOnTheSegmentsThatFollowIt` |
| `XlsxPrintSetup` stops setting `BandFont` — **the wiring** | six, including `TheWorkbooksDefaultFontReachesTheBandThroughThePrintSetup` |

The one not detected — `Length.Min` → `Length.Max` on a single-run line — is an **equivalent
formulation** on that fixture and not a drift guard; the same mutation *is* detected once the
filter is dropped and the existing three-line footer fixture is in scope. Both facts are reported
because they are different findings.

**`verify-test.sh` earned its place again, in a new way.** The wiring test was written first this
time, from round 55's lesson — and the mutation that pays for it is the one that breaks *six*
tests, because `SheetPageDecoration` falls back to `SheetDefaultFont.Calc` and a reader that
stopped setting `BandFont` would silently put every face assertion back on ten-point Liberation
Sans.

## 12. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. **No row changes status** — the round
moves no verdict, and that was predicted.

## 13. What the next round should do first

1. **The 18.46 pt body offset.** Two witnesses, both currently passing, both to a twentieth of a
   point: `fm-provider-service-measures` p36 (62.41 against 43.95, uniform to the last row) and
   `FY2023-AIP-grants` p1 (61.67 against 43.18). Both workbooks have a header band and both put
   the *band* within 0.11 pt while putting the *body* 18.5 pt low. On `fm-provider`'s sheet 7 the
   number `statedBand − nominal` is **18.4 pt** exactly, which is `mnBodyDist` — so the first
   thing to test is whether `HeaderHeight` is being counted twice, once in the band and once in
   the body origin. Found by a blind reviewer and confirmed by `pdftotext -bbox`.
2. **The grey cell fills.** `FAA-2019-0995-0002_attachment_2` p28: three `#C0C0C0` fills in the
   reference, zero non-white fills in ours. Found by a blind reviewer, confirmed by
   `pdf-ops.py --only fill`. Census it first — a fill rule reaches every sheet.
3. **`ChartLayout.IntervalsThatFit`** — § 9 has the census (256 axes, 129 documents, all three
   tracks), the sign argument that rules out the shape insets, and the arithmetic bound on the
   mm100 truncation. It needs authored charts with a known axis length, not `005`.
4. **The four `_advanced_excel_pie` documents** — still the largest cluster, and the gate needs
   only two of their five tokens.
5. **`Ooxml/XlsxNoteCaptions.cs`** for the next 24.2.7.2 site, before `Layout/SheetText.cs`: it is
   the furniture claim, and both sheets sites found wrong so far were furniture.
6. Still unworked, all ink: the chart area's light-grey border (387 strokes to our 0 on `005`); a
   data label group's stated `bg1` fill; the chart title's 9.8 pt vertical offset.
7. **A band's `&K` colour is still dropped, and now that the band carries a face it is the last
   thing on it that does not print.** Not measured this round; written down so it is not
   re-derived.
