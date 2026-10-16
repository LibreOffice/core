# Round 55 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r55`, base
`e11ee5ac386`. Read `prediction.md` (`26b120bcb74`) beside this file first; it was committed before
a line of the change was written and before anything was rendered post-change.

## 1. Baseline reproduced, to the document

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 288 MISMATCH 37`. Scored against
`MANIFEST.tsv`'s 307 sheets paths — the raw total counts 18 case-alias entries twice — that is
**274 match / 33 mismatch**, and the 33 mismatching paths are **exactly** the 33 rows the manifest
marks `open`. Zero disagreements in either direction.

## 2. Result

**sheets 274 → 276 of 307**, and **slides `ceiling-002` 0 → 1 of 5** as a measured side effect.
No document regressed anywhere.

| document | before | after | verdict |
|---|---|---|---|
| `013_Contextures_chart_sample…xlsx` | 7/7 pages, 165/169 words | 7/7, **169/169** | `words` → **`match`** |
| `012_Contextures_chart_sample…xlsx` | **5/6** pages, 126/130 words | **6/6**, **130/130** | `pages,words` → **`match`** |
| `FAA-2019-0995-0002_attachment_2.xlsx` | 33/33, fonts **5/6** | 33/33, fonts **6/6** | `match` held, fonts now agree |
| `WiGr_2021W_1_…pptx` (slides) | 51/51, 2157/1958 | 51/51, **1956/1958** | `words` → **`match`** |
| `Structural Testing.pptx` (slides) | 4366/4340 | 4335/4340 | `match` held |
| `RPA P4 - Advanced Material.pptx` (slides) | 1477/1479 | unchanged | `match` held |

Our column moved on **exactly** those six documents across both tracks and on no other. Everything
else that moved in the sweep diff moved on the **reference** side alone — `047_Date_tracker_Gantt`,
`PBN Matrix NAAs (V01)`, `TK-Syllabus-Comparison-Document-v2`, `ans_mappings_of_eccairs_terms`,
`SIL_TDB648` (both case aliases) and `FAA-2019-0995-0002_attachment_2` (both aliases) — which is
the date-volatility trap, and every diff below is split by side for that reason.

**`SIL_TDB648` was the named risk and is untouched**: 90/90 pages, 7498 words, `match`, and its
`editAs="oneCell"` reading is not merely unbroken but was independently *validated* by § 3.1's last
variant.

## 3. What the round found, and it is not what the brief expected

The brief said `013`'s page-1 picture is an `editAs="oneCell"` EMF we draw at the stated 326.25 pt
where the reference draws about 100 pt wider, that the reference answers to neither `editAs` nor
`a:ext` nor the anchor's second corner, and that **dropping `editAs` reproduces the reference to
0.2 pt**. Every one of those measurements reproduces. The sentence attached to them is wrong: the
reference is not reading that anchor at all, and dropping `editAs` reproduced it by coincidence.

### 3.1 The probe: vary the *other* side

Round 54 varied the DrawingML anchor three ways and the reference was inert to all three. A null
result from an instrument that cannot register the effect is not evidence of absence, so this round
varied the worksheet's **legacy VML drawing** instead. Five variants of the corpus file, one edit
each, each with a stated expected direction written before it ran
(`probes/sheets-r55/probe-vml-camera.py`):

| variant | reference | ours |
|---|---|---|
| the corpus file (control) | `1000` at 133.8, `Jan` at 534.3, 23 words on page 1 | 129.5, 414.5 |
| the sheet's `legacyDrawing` relationship removed | **the picture disappears — 23 words to 5** | unchanged |
| the VML `x:Anchor`'s `to` column 6 → 9 | **`1000` moves to 140.2, `Jan` off page 1** | unchanged |
| the VML `style` width halved | **unchanged** | unchanged |
| the VML `x:Anchor` deleted | **narrows to 112.6 / 397.6** — the CSS rectangle | unchanged |
| the `mc:AlternateContent` wrapper unwrapped | **the picture is drawn TWICE** — 129.5 *and* 133.8, 41 words against 23 | — |

The last line settles it. With the `a14` wrapper gone the reference draws the DrawingML anchor at
**129.524**, which is our own 129.540 — so the `editAs`/`a:ext` reading `SIL_TDB648` depends on is
**right**, and the picture Calc actually draws is a *second, separate object*: the `v:shape` in
`xl/drawings/vmlDrawing1.vml`.

### 3.2 The mechanism, in LibreOffice's own words

`ContextHandler2Helper::prepareMceContext` (`oox/source/core/contexthandler2.cxx:222`) keeps the
list of Markup Compatibility namespaces the `oox` filters honour. `a14` is on it **commented out**,
with the reason attached: `// u"a14", // We do not currently support inline formulas and other a14
stuff`. Writerfilter keeps its own list — `wps`, `wpg`, `w14`, `wpc` — and `a14` is not on that one
either. `013`'s whole `xdr:twoCellAnchor` sits inside `mc:Choice Requires="a14"` beside an **empty**
`mc:Fallback`, so Calc reads no DrawingML anchor, and round 54's three variants could not have moved
anything.

The picture then comes from the sheet's `<legacyDrawing>`: `VmlDrawing::isShapeSupported`
(`sc/source/filter/oox/drawingfragment.cxx`) imports every VML shape except `XML_Note`, and
`ShapeBase::calcShapeRectangle` (`oox/source/vml/vmlshape.cxx:509-516`) sizes it from
`x:ClientData/x:Anchor` in preference to the CSS — which the third and fifth variants confirm in
both directions on the binary rather than in the source.

### 3.3 The change

1. **`DrawingML2010` out of `OoxmlNamespaces.UnderstoodExtensions`.**
2. **`XlsxLegacyPictures`** — a new reader for the worksheet's legacy VML pictures, anchored by
   `x:ClientData/x:Anchor`, sharing its VML helpers with `XlsxNoteCaptions` through a new
   `XlsxVml`.
3. **The slicer special case in `OoxmlXml` removed.** It existed only because `a14` was understood;
   with `a14` refused, the general rule reaches all seven of the corpus's `a14` slicer choices and
   the special case was unreachable. Its *outcome* was measured and is unchanged — the three slicer
   documents render byte-identically — and its tests are kept.

**The relationship type is the wrong key, and using it would have broken three matching documents.**
`legacyDrawingHF` — the header and footer's watermark images — uses the *same* `vmlDrawing`
relationship type. Keying on the type draws `PBN Matrix NAAs (V01)`'s 24 header watermarks, and one
each on `UAE Type Accepted Aircraft Models` and `Application_Compliance_Checklist`, as objects on
the grid. The reader follows the worksheet's own `<legacyDrawing r:id>` instead, and
`AHeaderOrFooterDrawingIsNotAnObjectOnTheSheet` is the test that holds it.

### 3.4 Census and reach, by two independent instruments

`census-a14.py` reads every `.xml`/`.vml` part of all 946 corpus documents and keys each
`mc:Choice` on the **resolved URI** of every prefix its `Requires` names: **2324 choices**, of which
**34 resolve to `a14`, in 10 documents** — 7 sheets, 3 slides, **0 words**. A second, deliberately
dumber pass (regex for a prefix bound to the `a14` URI *and* a `Requires` naming it) returns the
same 10 documents. `census-vml.py`: **5 VML `Pict` shapes in 3 spreadsheets**, 1 `Scroll`, 359
`Note`s, and 35 shapes with no `x:ClientData` — all of which are `legacyDrawingHF` header images.

| a14 document | what the choice wraps | fallback | measured outcome |
|---|---|---|---|
| `013_Contextures`, `012_Contextures` | `xdr:pic` (camera tool) | **empty** | both **closed** |
| `015_Free_Gantt_Chart_Template` | `xdr:sp` scroll bar, `hidden="1"`, `a:ext cx=0 cy=0` | empty | unchanged, 8/8 and 1986/1986 |
| `FAA-2019-0995-0002_attachment_2` | 3 × `xdr:sp` OLE placeholders, `hidden="1"`, `cx=0 cy=0` | empty | 33/33 held; **fonts 5/6 → 6/6** |
| `DynamicBubbleChart`, `037_Personal_money_tracker`, `049_Expenses_calculator` | slicer `graphicFrame` | content | **byte-identical** |
| `Structural Testing`, `RPA P4`, `WiGr` (slides) | `a14:m` inline OMML | content | two held, one **closed** |

## 4. Prediction against measurement

| | predicted | measured |
|---|---|---|
| `013_Contextures` | `match`, 169/169 | **`match`, 169/169** |
| `012_Contextures` | `match`, 130/130, 6/6 | **`match`, 130/130, 6/6** |
| `015_Free_Gantt` | unchanged | **unchanged** |
| `FAA-2019-0995-0002` | 33/33, `match` | **33/33, `match`** — and fonts 5/6 → 6/6, not predicted |
| the three slicer documents | byte-identical | **byte-identical** |
| `Structural Testing`, `RPA P4` | `match` held | **held** |
| `WiGr_2021W_1` | **`words`, still open** | **`match` — WRONG, and in the good direction** |
| sheets verdicts | 274 → **276** | **276** |
| slides verdicts | 0/5 and 14/14, unchanged | **1/5** and 14/14 |
| words verdicts | 0 | **0** — no words document has an `a14` choice, by two censuses |

Eight of nine exact. **The miss is the one the prediction file named as unbounded**: blind spot 3
said `WiGr`'s direction was not predicted, only its verdict, "because twenty math choices swapped
for their fallbacks could move its word count a long way in either direction". They moved it 201
words, from 199 over the reference to 2 under, and it closed. The blind spot fired exactly where it
was pointed and the number written beside it was still wrong — which is the second round running
that the blind-spot section has been the most accurate part of the prediction.

Blind spot 4 — the `legacyDrawingHF` trap — is the one that paid for itself: it was written down
*before* the reader was written, and the reader was keyed on the worksheet element from the first
line because of it.

## 5. The page, and two of four readings refuted

Four pages, each chosen for a stated reason rather than by `--worst` except where noted, each to a
fresh subagent with no access to this project's documents, source or shell, asked to describe each
half separately before comparing and to give direction.

**Two readings were artefacts of the composite and a second instrument refuted both.**

- The `013` page-1 reviewer reported "the left chart's legend shows swatches **with** the labels
  Jan/Feb/Mar; the right shows only the swatches". `pdftotext` says both pages carry
  `Jan Feb Mar`, and every token on the page agrees in position to **0.12 pt**.
- The `012` page-1 reviewer reported "the right chart's four category clusters are spread with
  visibly more horizontal white space between them than the left's". `East`/`West`/`North`/`South`
  are at 206.94/284.18/362.49/442.69 in the reference and 206.91/284.14/362.42/442.61 in ours.

That is the § 7 pattern again — two fluent readings of a downscaled 78%-of-composed image — and it
is worth recording that **this time the same instrument refuted them within a minute**, because the
question ("are these two objects in the same place?") had a cheap numeric answer.

**The positive control is strong.** Page by page, `012` and `013` now reproduce the reference
*token for token*, including the reference's own clipping: page 2 of each holds `outh Jan Feb Mar`
in both — the second half of `South` sliced by the horizontal page split, and the legend text
repeated across it. We reproduce the reference's artefact, not merely its totals.

### 5.1 The one blind reading that was real, and it explains a whole document's residual

The `FAA-2019-0995-0002_attachment_2` reviewer — on page 28, which `--worst` chose — reported "the
top half shows a `PAGE 2_ OF 33` marker in the upper-right corner of the table area; I do not see
this marker in the bottom half". Confirmed, and larger than the reading:

**Five pages (28 to 32) draw `PAGE n OF 33` in ours and none in the reference. Twenty tokens —
which is exactly the document's entire word surplus, 10015 against 9995.** The tokens sit at
x = 807–836 on an 842 pt landscape page, hard against the right edge, wrapping onto two lines. The
string is in no cell of any worksheet and no header or footer element; `sharedStrings.xml` holds a
`PAGE 1 OF 1` that is a different string. Not chased further this round, and not caused by it:
our count was 10015 before the change and 10015 after.

The same reviewer's second observation — "the reference runs `FILTER ASSY HYD291143 CMM` together
where we break the line" — is **refuted**: both wrap identically, at the same place.

## 6. The 24.2.7.2 audit — `SheetPageDecoration.cs`, and it is **WRONG**

`probes/sheets-r55/audit_pagedecoration.py`, authored fixtures against the installed 26.2.4.2, on
the generator shape `probes/sheets-r53-totalsrow/audit_mkwb.py` establishes is read correctly.
**The control ran first and passed**: at Excel's own 0.4 in band both sides draw both bands and
agree to 1.2 pt.

The site claims two things and only one survives.

| claim | outcome |
|---|---|
| a band of **exactly zero** draws nothing — not the space and not the ink | **holds**, and so does every negative band |
| the reference draws the band "at every stated band **above zero**" | **false** |

The reference draws **nothing** at a stated band of 0.72 pt or 1.44 pt either, and where it starts
depends on the text: the threshold is between **1.44 and 2.16 pt for 8 pt** header text and between
**4.32 and 5.76 pt for 20 pt** text — about 0.27× the point size over two points. It is a
*text-fit* rule, not a constant, and certainly not zero. Fourteen renderings, one variable each.

**Reported, not fixed**, and the reason is reach rather than difficulty. `census-bands.py` reads all
**267** corpus worksheets that state header or footer content and finds **four** with a positive
band under 6 pt — `085_Simple_Gantt_chart` 3.6 pt, `020_Free_Blood_Pressure_Chart` 3.6 pt,
`fm-provider-service-measures` 3.6 pt and `FAA-2019-0995-0002_attachment_2` 5.67 pt — all on
documents that match today, and all above the bracketed threshold for ordinary 10–11 pt header
text. Guessing the exact law to move four passing worksheets is the trade this project's rules say
not to make. Marked at the site.

### 6.1 What the probe found beside it, and this one is fixed

Two of the census's worksheets state a **negative** band — `023_Waterfall_Chart_Template`'s header
at −3.6 pt and `2025_Active_Civil_Airmen_Statistics`' footer at −5.76 pt — because the file's
`header` margin is larger than its `top`. There, 26.2.4.2 still starts the body at the **page**
margin, exactly where it starts it at every non-negative band, and we started it at the *band*
margin: **18 pt** lower on the fixture. One `Math.Min` on each of the two margins, in
`XlsxPrintSetup`.

**It moves nothing in the corpus and that is measured, not assumed.** The whole sheets track was
swept again after it: 325 rows, **8 differ, and all 8 differ on the reference side only** — our
column is byte-identical across the change. Both witnesses were also rendered directly and their
page-1 text is identical to the pixel before and after. So this is a correctness fix with a measured
mechanism and no corpus witness, and its three tests are the only thing holding it.

### 6.2 The audit's own counters do not reproduce

`TODO.24-2-7-audit.md` states **44 open hits** and **12 markers**. Re-derived with the commands the
file itself gives, at this round's base commit `e11ee5ac386`: **42 open, 13 marked**. Corrected in
that file. This round takes it to **42 open, 14 marked** — the open count does not fall when a site
is verified, by that file's own convention.

## 7. Tests

**Fifteen new, and fourteen of them are detectors verified by reintroduction.**

| mutation | detected by |
|---|---|
| `a14` put back in `UnderstoodExtensions` | three at once — `ASlicerChoiceLosesToItsFallbackWhenItsRequiresIsA14`, `AnA14ChoiceLosesToItsFallbackWhateverItWraps`, `AnA14ChoiceWithNoFallbackBesideItIsDropped` |
| the `legacyDrawing` element key replaced by the relationship type | `AHeaderOrFooterDrawingIsNotAnObjectOnTheSheet` |
| a VML anchor's pixel offsets read as EMUs | `TheAnchorsOffsetsAreScreenPixels` |
| `Note` shapes no longer skipped | `ANoteShapeIsNotReadHere` |
| hidden shapes drawn | `AHiddenShapeIsNotDrawn` |
| a missing image still anchors an empty picture | `APictureWhoseImageIsMissingIsDropped` |
| the image relationship resolved against the sheet rather than the VML part | `APictureWhoseImageIsMissingIsDropped` |
| the anchor kind changed to `OneCell` | `TheAnchorIsTheClientAnchorsTwoCorners` |
| the anchor's `To` corner read as the `From` corner | two of them |
| a missing `<legacyDrawing>` defaulted to `rId1` | `AWorksheetWithNoLegacyDrawingReadsNothing` and the header/footer one |
| a shape with no client anchor anchored at A1 | `AShapeWithNoClientAnchorIsSkipped` |
| the `XlsxReader` call site replaced by an empty list | `ALegacyPictureReachesTheSheetsDrawings` |

**`verify-test.sh` found a real gap and it is why the last row exists.** The first ten tests all
drove `XlsxLegacyPictures.Read` directly, so replacing the reader's call site in `XlsxReader` with
`[]` broke none of them — the reader could have been entirely right and never called. The wiring
test was written *because* the mutation came back clean.

**One drift guard, labelled as one at the site.** `AShapeWithNoImageDataIsNotAPicture` survives
having the `v:imagedata` requirement made inert, because a shape with no image names no
relationship and is dropped a few lines later by the guard `APictureWhoseImageIsMissingIsDropped`
covers. Kept, because the two say different things.

**One existing test asserted the behaviour this change reverses**, and it was rewritten rather than
deleted. `ASlicerChoiceWithNoFallbackBesideItIsStillTaken` claimed an unreadable choice with no
fallback is still taken, on the stated ground that "dropping the choice would lose an anchor rather
than gain a placeholder" — an argument, not a measurement. It is now
`AnA14ChoiceWithNoFallbackBesideItIsDropped`, against the measured case: removing `013`'s
`legacyDrawing` relationship leaves exactly that shape and takes the reference's page 1 from 23
extractable words to **5**.

`ASlicerChoiceLosesToItsFallbackWhenItsRequiresIsUnderstood` is renamed and re-argued rather than
touched in what it asserts, and the remark at it now records that the *outcome* was measured twice
and reached by two different mechanisms, of which only the second is right.

## 8. Shared layer

**Yes.** `XRay.Office.Ooxml/OoxmlNamespaces.cs` and `XRay.Office.Ooxml/OoxmlXml.cs`. All three tracks
read them.

The census names the reach exactly — **10 documents: 7 sheets, 3 slides, 0 words** — and it is
confirmed by a second, independent pass over the same 946 packages. I have swept the **whole sheets
track** twice and all three affected slides batches before and after, and the figures are in § 2.
**The parent should still run the cross-track sweep**, and the prediction for it is: sheets +2,
`slides/ceiling-002` +1, `slides/done-013` and `slides/done-015` unchanged at 14/14, words unchanged.

`XRay.Office.Spreadsheets` changes (`XlsxVml`, `XlsxLegacyPictures`, `XlsxNoteCaptions`,
`XlsxDrawings`, `XlsxReader`, `XlsxPrintSetup`, `SheetPageDecoration`) reach the sheets track only.

## 9. Build and tests

`dotnet build -v q -nologo` → **0 warnings, 0 errors.**

Ten non-Fidelity projects, run one at a time and totalled by hand: **4787 passed, 0 failed, 1
skipped**, against the base's 4772/0/1 — a delta of exactly the **15** new tests, all in
`XRay.Office.Spreadsheets` (925 → 940). `Fidelity` is **521 passed / 31 failed / 552**, byte-for-byte
the base's figure.

## 10. `MANIFEST.tsv`

Lives in the corpus repository and was not touched. Three rows change status, all in one direction:

| path | from | to |
|---|---|---|
| `sheets/chartset-012/xlsx/012_Contextures_chart_sample_9900da76.xlsx` | `open` | `done` |
| `sheets/chartset-012/xlsx/013_Contextures_chart_sample_21b98e22.xlsx` | `open` | `done` |
| `slides/ceiling-002/pptx/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211017-171222.pptx` | `open` | `done` |

## 11. What the next round should do first

1. **`FAA-2019-0995-0002_attachment_2`'s five `PAGE n OF 33` blocks (§ 5.1).** Twenty tokens on
   five pages, they are the document's *entire* residual divergence, they are ours-only, and they
   sit at x = 807–836 on an 842 pt page — hard against the right edge, wrapping. A blind reviewer
   found them and `pdftotext` confirmed them; the string is in no cell, no `oddHeader` and no
   `oddFooter`, which is the interesting part. It moves no verdict (the document matches by a wide
   band) but it is the cleanest unexplained ours-only ink on the track.
2. **`ChartLayout.IntervalsThatFit`.** Untouched again. `005`'s plot rectangle agrees with the
   reference to 1.2 pt and its tick *step* does not; `available / needed` gives 8 where
   `estimateMaximumAutoMainIncrementCount` gives 9 on the same geometry. Four tokens on `005`, and
   it is a law reaching every column chart, so it wants a census before a line.
3. **The four `_advanced_excel_pie` documents** — the largest cluster left, `135/140` ×3 and
   `138/143`, and the gate needs only **two** of their five tokens.
4. **The header/footer band's text-fit threshold (§ 6).** Bracketed but not solved: 0.27× the point
   size over two points. Four corpus worksheets sit near it and all four pass today, so this needs
   the law established on authored fixtures — six or eight sizes — before anything is written.
5. **`SheetNotes.cs` or `SheetShapeText.cs`** for the next 24.2.7.2 site.
   `XRay.Office.Spreadsheets` is now **six of nine** re-checked; five were correct and this one was
   half wrong. The prior is no longer "probably still fine" on this track either.
6. Still unworked, all ink: the chart area's light-grey border (387 strokes to our 0 on `005`);
   a data label group's stated `bg1` fill (33 white fills to our 0); the chart title's 9.8 pt
   vertical offset, of which 3.83 pt is explained.
7. **The legacy VML *controls*.** `VmlDrawing::isShapeSupported` imports Button, Checkbox, Drop,
   Scroll and the rest and rebuilds them as OLE form controls; this round reads only the shapes
   carrying `v:imagedata`. Corpus exposure is one hidden `Scroll`, so it is worth nothing today and
   is written down so the next reader of `XlsxLegacyPictures` does not have to re-derive it.
