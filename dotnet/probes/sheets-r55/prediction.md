# Round 55 — sheets — prediction

Committed **before** a line of the change was written and before anything was rendered
post-change. Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch
`wt-sheets-r55`, base `e11ee5ac386`.

## Baseline, reproduced first

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 288 MISMATCH 37`, which scored
against `MANIFEST.tsv`'s 307 sheets paths is **274 match / 33 mismatch**, and the 33 mismatching
paths are **exactly** the 33 rows the manifest marks `open` — zero disagreements in either
direction. Round 54's figure to the document.

Slides baselines for the three batches this change can reach: `slides/ceiling-002` `TOTAL 5 MATCH
0`, `slides/done-013` + `slides/done-015` `TOTAL 14 MATCH 14`.

## What the change is

Two edits, and the second only exists because of the first.

**1. `a14` is not a Markup Compatibility namespace `oox` supports, and we treat it as one.**
`ContextHandler2Helper::prepareMceContext` (`oox/source/core/contexthandler2.cxx:222`) carries the
list of MCE namespaces the OOXML filters honour, and `a14` is on it **commented out**:
`// u"a14", // We do not currently support inline formulas and other a14 stuff`. `p14`, `p15`,
`x12ac`, `v`, `cx1`, `cx2`, `cx4` are honoured; `a14` is not. Writerfilter keeps its own list
(`wps`, `wpg`, `w14`, `wpc`) and `a14` is not on that one either. `OoxmlNamespaces.UnderstoodExtensions`
contains `DrawingML2010`, so we take an `a14` choice where all three LibreOffice filters take the
fallback. It comes out.

**2. `013`'s page-1 picture is the worksheet's legacy VML shape, not its DrawingML anchor.**
Round 54 varied `editAs`, `a:ext/@cx` and the anchor's `to` column and the reference did not move
for any of them. It could not: the whole `xdr:twoCellAnchor` sits inside `mc:Choice Requires="a14"`
beside an **empty** `mc:Fallback`, so Calc reads no DrawingML anchor at all. Five authored variants
on the *other* side (`probe-vml-camera.py`), each with a stated expected direction:

| variant | reference | ours |
|---|---|---|
| the corpus file | `1000` at 133.8, `Jan` at 534.3 | 129.5, 414.5 |
| the sheet's `legacyDrawing` relationship removed | **the picture disappears — 23 words to 5** | unchanged |
| the VML `x:Anchor`'s `to` column 6 → 9 | **1000 moves to 140.2, `Jan` off page 1** | unchanged |
| the VML `style` width halved | **unchanged** — the client anchor wins over the CSS | unchanged |
| the VML `x:Anchor` deleted | **narrows to 112.6 / 397.6** — the CSS path, as `calcShapeRectangle` says | unchanged |
| the `mc:AlternateContent` wrapper unwrapped so the anchor is plainly visible | **the picture is drawn twice**: 129.5/414.5 *and* 133.8/534.3, 41 words against 23 | — |

The last line is the one that settles it. With the a14 wrapper gone the reference draws the
DrawingML anchor at **129.524**, which is our current 129.540 — so our `editAs`/`a:ext` reading is
*right*, and `SIL_TDB648` is not at risk, because that code is not being touched. What is wrong is
that we read an anchor Calc never sees, and do not read the VML shape Calc draws.

So: read the worksheet's `<legacyDrawing r:id>` VML part and import its non-`Note` shapes that
carry `v:imagedata`, anchored by `x:ClientData/x:Anchor`
(`ShapeBase::calcShapeRectangle`, `vmlshape.cxx:509-516`; `VmlDrawing::isShapeSupported` excludes
only `XML_Note`; `ShapeAnchor::importVmlAnchor` sets `CellAnchorType::Pixel`).

## Documents I expect to change, and by how much

Census (`census-a14.py`, all 946 corpus documents, keyed on the *resolved* URI of every
`mc:Choice/@Requires` prefix): **2324 choices**, of which **34 resolve to `a14`, in 10 documents**.
`census-vml.py`: **5 VML `Pict` shapes in 3 sheets documents**, one `Scroll`, 359 `Note`s and 35
shapes with no `x:ClientData` at all.

| document | now | predicted | why |
|---|---|---|---|
| `013_Contextures_chart_sample` | `words` 165/169, 7/7 | **`match` 169/169** | the picture gets the two-cell span |
| `012_Contextures_chart_sample` | `pages,words` 126/130, **5/6** | **`match` 130/130, 6/6** | same shape, same mechanism |
| `015_Free_Gantt_Chart_Template` | `match` 1986/1986, 8/8 | **`match`, unchanged** | its a14 choice is a `hidden="1"` scroll bar with `a:ext cx="0" cy="0"` and `fPrintsWithSheet="0"`; no text, no ink |
| `FAA-2019-0995-0002_attachment_2` | `match` 10015/9995, 33/33 | **`match`, 33/33** | its three a14 choices are `hidden="1"` OLE placeholders at `cx=0 cy=0`; its three VML `Pict`s are newly drawn, over rows the sheet already prints |
| `DynamicBubbleChart`, `037_Personal_money_tracker`, `049_Expenses_calculator` | `match` | **`match`, byte-identical** | their a14 choices wrap slicers and already lose to `WrapsAnUnreadableGraphic`; the general rule now does the same job |
| `Structural Testing.pptx` | `match` 4366/4340 | **`match`** | one a14 `a14:m` inline-math choice; 86.8 of band, 26 used |
| `RPA P4 - Advanced Material.pptx` | `match` 1477/1479 | **`match`** | one a14 math choice; 29.5 of band, 2 used |
| `WiGr_2021W_1_…pptx` | `words` 2157/1958 | **`words`, still open** | 20 a14 math choices; we are 199 words *over* a band of 39, and the fallback can only take tokens away |

**Verdict movement predicted: sheets 274 → 276 of 307 (+2). Slides unchanged at 0/5 and 14/14.
Words unchanged: no words document in the corpus has an `a14` choice or a worksheet.**

Pre-change evidence for the two closures, so the number is not a hope: rendering `012` and `013`
with `editAs` stripped — which routes our existing two-cell path over the *same* cell span the VML
anchor states — gives `012` **6 pages / 130 words** and `013` **7 / 169**, against references of
6/130 and 7/169. Exact on both.

## What this census cannot see — write it down before the sweep

1. **The prefix resolution is not properly scoped.** `census-a14.py` gathers `xmlns:` declarations
   with a regex over the whole part and lets the last one win. A part that rebinds a prefix
   mid-document is mis-resolved. It reports no rebinding — but that check is the same instrument,
   so it is not independent.
2. **It counts choices, not what the other branch draws.** For the three slides documents the
   fallback has content and I have measured nothing about it. That is the regression risk and the
   one thing the table above assumes away.
3. **`WiGr`'s direction is not predicted, only its verdict.** Twenty math choices swapped for their
   fallbacks could move its word count a long way in either direction; I claim only that 199 over a
   band of 39 does not close.
4. **The VML reader keys on `<legacyDrawing r:id>`, not on the `vmlDrawing` relationship type**,
   because `legacyDrawingHF` uses the *same* relationship type and points at header watermark
   images — 24 of them in `PBN Matrix NAAs (V01)`, one each in `UAE Type Accepted Aircraft Models`
   and `Application_Compliance_Checklist`, all three currently passing. Keying on the type would
   have drawn header watermarks as sheet objects on three matching documents. If any worksheet
   reaches a VML part by some third route, it is invisible to both the census and the reader.
5. **LibreOffice imports every non-`Note` VML shape, including legacy form controls**; this reads
   pictures only. The census says the corpus exposure is one hidden `Scroll`, but it keys on
   `x:ClientData/@ObjectType` and cannot see a control that declares none.
6. **The gate cannot see ink.** The EMF is drawn either way and only its rectangle changes. The
   pre-change proxy above went through the *DrawingML* anchor while the shipped code goes through
   the *VML* one; they are numerically identical on these two files (6 px = 57150 EMU, 14 px =
   133350 EMU, from-corners equal) — but that equality is a property of these two documents, not of
   the code, and nothing in the census would notice if the pixel conversion were wrong elsewhere.
7. **The reference half of the gate is not reproducible for date-bearing sheets.** Six documents
   moved on the reference side alone last round. Every sweep diff below is split by which side
   moved.
8. **The 24.2.7.2 audit site is a separate exercise** and its outcome is not predicted here: a
   marker is a claim like any other and can be wrong in either direction, so predicting `VERIFIED`
   would be predicting the answer rather than the measurement.

## Test plan

Every new test through `verify-test.sh` with a stated mutation, and each reported as a detector or
a drift guard. One existing test — `ASlicerChoiceWithNoFallbackBesideItIsStillTaken` — asserts a
behaviour this change reverses, and it is a **drift guard whose rationale was an argument, not a
measurement**. It will be rewritten rather than deleted, against the `no-vml-rel` variant above,
where an a14 choice with no usable fallback is measurably dropped by the reference.
