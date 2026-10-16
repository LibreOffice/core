# Round 132 — a scaled page quantises its rule before the multiply, and a `.ppt` connector is not routed

Two seats, unrelated in subject: **O70**, where the reference puts the rule quantisation on the
other side of a page transform from this tree, and **O76**, where the register asks for a
connector router that the reference never runs.

Reference is **26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`) throughout. The C++ read here is the checkout's, which
is `27.2.0.0.alpha0+` from a bulk import (**C8**) — so every arm below is confirmed twice, once in
source and once against 26.2.4.2's own output.

---

## 0. The headline

| seat | state | measurement |
|---|---|---|
| **O70** | **FIXED** | On the 31 documents holding a scaled rule, 1418 paired reference text rules go from **544** agreeing to 0.0015 pt to **1412**; mean \|Δthickness\| 0.0051 → **0.0004 pt**, mean \|Δdepth\| 0.0055 → **0.0007 pt**. Over the whole **307-document sheets track**, 11 957 paired rules go from **11 080** agreeing to **11 950**, disagreements **877 → 7**. Zero documents down on either. |
| **O76** | **CLOSED — the seat's mechanism is REFUTED, and what remains is NOT WORK on reach** | `SetEdgeTrackPath` marks the edge track *user-defined* and `ImpRecalcEdgeTrack` returns early for one (`#i120437`), so no routing happens at all: 26.2.4.2 draws the Escher preset's own line geometry. Confirmed against the binary — its `--convert-to fodp` writes each connector as a `<draw:connector>` with an explicit `svg:d` bending at **0.6657** of the span, which no router produces. Reach of what is left: **3 of 947 documents**, and in two of them the connectors do not pair with the reference on their end points at all, so the adjustment is not even the first defect there. |

**Confinement, byte for byte.** The base binary and the fixed one each rendered all **947** corpus
documents under a pinned `SOURCE_DATE_EPOCH`; **947 of 947 succeeded on both** and exactly **31
differ**. By track: **docx 0 of 272, doc 0 of 66, pptx 0 of 251, ppt 0 of 51, xlsm 0 of 2** —
bit-identical — against **xlsx 27 of 241 and xls 4 of 64**. `hash-diff.tsv`.

---

## 1. O70 — the mechanism, settled on the witness before any code was written

### 1.1 What the two sides do

Round 123 established the chain and O64 the device: the PDF writer instantiates the font on its own
**720 dpi** reference device, `FontMetricData::ImplInitTextLineSize` answers a whole number of
*those* pixels, and `drawStraightTextLine`'s `HCONV` — `DevicePixelToLogicHeight`
(`vcl/source/pdf/pdfwriter_impl.cxx`:6586) — reaches `CoordinateMapper::ViewToLogicDistanceY`, an
`llround` to a whole logical unit (`vcl/source/outdev/CoordinateMapper.cxx`:279).

O70 is that chain meeting a page transform. Calc prints a zoomed sheet through a **map mode
carrying the zoom**, so `ImplLogicHeightToDevicePixel` takes the scale *into* the device pixel
count and `DevicePixelToLogicHeight` takes it back *out* again — and the `llround` therefore lands
on a whole logical unit of the **unscaled** page, which is multiplied by the zoom on its way to the
paper. This tree scaled first and rounded afterwards, landing on a whole unit of the *scaled* page.

### 1.2 The witness, by hand, with no free parameter

`RMP 2011-2014 and Inventory.xls`. The reference PDF states three font sizes — 6.0, 6.004 and
7.2 pt — and the 6.004 is the one under 260 of its spans. That size is recoverable: Calc snaps a
10 pt Arial to **353** hundredths of a millimetre, and `353 × 0.6 = 211.8` hundredths is
**6.00425 pt**, written `6.004`. No other whole percent produces it (0.8 gives 6.010, 0.85 gives
5.999, 0.9 gives 5.995, 0.95 gives 6.006). So the page is at Calc's own **60%** and the em is 10 pt.

At 60 device pixels the descent branch answers **3** pixels of thickness and **7** pixels from the
baseline to the stroke's centre (`nLinePos + nLineHeight/2`, tdf#154235). Then:

| | thickness | centre depth |
|---|---|---|
| quantise **after** the scale — this tree until now | `round(3 × 2540/720)` = 11/100 mm = **0.31181 pt** | `round(7 × 2540/720)` = 25/100 mm = **0.70866 pt** |
| quantise **before** it — the model | `round(3 × 2540/720 / 0.6) × 0.6` = `18 × 0.6` = 10.8/100 mm = **0.30614 pt** | `round(7 × 2540/720 / 0.6) × 0.6` = `41 × 0.6` = 24.6/100 mm = **0.69732 pt** |
| **26.2.4.2 draws** | **0.306** | **0.697** |

`appendMappedLength` writes thousandths of a point, so 0.30614 is written `0.306` and 0.69732
`0.697`. Both to the digit, and 86 of the document's 86 paired rules are this one case.

### 1.3 The model was tested against 1418 rules before the code changed

`fitzoom.py` and `fitzoom-page.py` need no renderer at all. This tree's value is `round(p × U)`
logical units for an integer pixel count `p` and `U = 2540/720`, so `p` is recoverable from round
123's banked `compare-scaled.tsv.pairs`; the claim is then that the reference is
`round(p × U / z) × z`. The free parameter is **one whole percent** — `SheetPagination.ZoomPercentage`
is an `int` and `ScPrintFunc::CalcZoom` bisects on one — fitted against up to 840 equations per
document (a thickness and a depth per rule).

```sh
python3 fitzoom.py      ../dblunder-r123/compare-scaled.tsv.pairs > fitzoom.tsv
python3 fitzoom-page.py ../dblunder-r123/compare-scaled.tsv.pairs > fitzoom-page.tsv
```

| | rules | this tree exact | model exact |
|---|--:|--:|--:|
| one zoom per **document** | 1418 | 544 | **1317** |
| one zoom per **page** — a workbook prints each sheet at its own zoom, `printfun.cxx`:2718 | 1418 | 544 | **1412** |

The per-document fit recovers **60** for `RMP` — the percent derived independently from the stated
font size above — and reproduces 86 of its 86 rules; `seihon_zassi_kikou_20221215` fits 420 of 420
on one number, `7-memento` 66 of 66, `Published_Issuances_2024` 40 of 40. **The 1412 is a
prediction made before the change, and the change hit it exactly.**

### 1.4 The change

`MetricGrid` gains a `PageScale`, and **only `ToLength` reads it** — that is the one conversion
that crosses the map mode outwards. `ToPixelEm` needs no correction because the em it is handed is
already the drawn size, so the scale is inside it; on the witness both sides give 60.

```csharp
public Length ToLength(long pixels)
{
    if (Dpi <= 0) return Length.Zero;

    double scale = double.IsFinite(PageScale) && PageScale > 0.0 ? PageScale : 1.0;

    if (scale == 1.0)
    {
        return FromLogical(
            (long)Math.Round(pixels * UnitsPerPixel, MidpointRounding.AwayFromZero));
    }

    return FromLogical((long)Math.Round(
        pixels * UnitsPerPixel / scale, MidpointRounding.AwayFromZero)) * scale;
}
```

`SheetTextLayout` threads `context.Scale` through `Decorate`/`DecorateSegment` to
`MetricGrid.TextLine.Scaled(scale)`.

### 1.5 Confinement, as code rather than as a spot check

`MetricGrid` is not a sheets type — charts and slides reach `ToLength` too — so "only Calc passes a
zoom" has to be shown and not asserted. Three statements, each checkable by reading:

1. **The unscaled branch is the original expression, character for character.** It is an early
   return, not the scaled expression with a `1.0` in it, so the identity does not rest on what
   dividing a double by one does.
2. **`PageScale` can only be set by `Scaled`, and `Scaled` has exactly one caller.**
   `git grep 'MetricGrid.*\.Scaled(' dotnet/src` returns one line,
   `SheetTextLayout.cs`:532. Every `MetricGrid` construction in the tree passes two or three
   positional arguments — `Printer`, `Reference`, `Presentation`, `Spreadsheet`, `Chart`,
   `TextLine`, `WriterTextLine` and the two in `MetricGridTests` — so `PageScale` is its 1.0
   default everywhere else, and `AsWordDocument`'s `with` carries it through unchanged.
3. **`Scaled` refuses anything that is not a usable transform.** 1.0, zero, a negative and a
   non-finite all give the grid back unchanged, pinned by
   `AGridWithNoUsableTransformIsTheGridItself`.

And then measured anyway — §3.

### 1.6 Before and after, against the banked reference

`compare-rules.py` renders each document with this tree and pairs its text rules with 26.2.4.2's by
page, left edge and depth. The reference half is `/home/user/gate-r129/ref`, one banked PDF per
corpus document, so no `soffice` ran for any of this.

```sh
python3 ../dblunder-r123/compare-rules.py "$CLI" /home/user/sample-files \
        /home/user/gate-r129/ref ../dblunder-r123/scaled-docs.txt after.tsv
```

**The 31 documents holding a scaled rule** — 1762 reference text rules, 1418 paired, the pairing
count identical before and after:

| | before | after |
|---|--:|--:|
| agree to 0.0015 pt on thickness **and** depth | **544** | **1412** |
| agree to 0.02 pt on both | 1412 | 1414 |
| mean \|Δ\| thickness | 0.0051 pt | **0.0004 pt** |
| mean \|Δ\| depth | 0.0055 pt | **0.0007 pt** |
| worst | 0.1992 / 0.3975 pt | 0.1992 / 0.3975 pt |

**21 of the 31 documents go to 100% and none goes down.** `before.tsv`, `after.tsv` and their
`.pairs`.

**The whole sheets track**, 307 documents, which is the corpus-level control on the C#:

| | ref rules | paired | agree | disagree |
|---|--:|--:|--:|--:|
| before | 12 404 | 11 957 | 11 080 | 877 |
| after | 12 404 | 11 957 | **11 950** | **7** |

**Zero of the 307 documents lost a rule.** `sheets-before.tsv`, `sheets-after.tsv`.

### 1.7 The six that remain, and why none of them is this defect

| document | ref | ours | what |
|---|--:|--:|---|
| `FAA-2019-0995-0002_attachment_2__xlsx` ×2 | 3.9690 @ 0.7500 | 3.9968 @ 0.7500 | depth out by 0.0278 pt — **one** hundredth of a millimetre; thickness exact |
| `INDEX_Digital_Transformation_Toolkits__xls` | 4.7620 @ 1.7501 | 4.7906 @ 1.7500 | the same, 0.0286 pt |
| `NAS-Infrastructure-Roadmaps-v16.0__pptx` | 10.8560 @ 0.4501 | 10.8421 @ 0.4500 | 0.0139 pt, half a hundredth |
| `NCW-2024-Guide-__pptx` | 0.9060 @ 0.7920 | 0.9072 @ 0.7937 | 0.0012 and 0.0017 pt, just outside the channel's floor |
| `apron-area__xls` | 0.7080 @ 0.3110 | 1.1055 @ 0.5102 | round 123's own worst pair, 0.1992 / 0.3975 |

**All six documents rendered byte-identically before and after** (§3), so the fix does not touch
them: they are at a page transform of one and their defect is something else. `apron-area`'s
0.1992 / 0.3975 is the figure round 123 recorded as its worst and it is **unchanged**, which is the
point — it was never residue from this.

### 1.8 A refutation of one of my own steps, recorded

I first argued that a document whose reference rules are all whole logical units cannot move,
because if `round(p × U / z) × z` is an integer `k` then `|k − pU| ≤ 0.5z < 0.5` and so
`k = round(pU)`. **That argument is one-directional and I nearly quoted it as confinement.** It
says only that when the *new* value is a whole unit it equals the old one; it does not say the new
value is a whole unit, and a document round 123 classified as unscaled can therefore still move.
Six did (§3.1). The byte comparison is what settles confinement; the argument is not.

---

## 2. The regression control that was asked for, and what it can and cannot see

`corpus-rules.py` scores round 123's model against 26.2.4.2's banked rendering of all 947
documents. Re-run here against `/home/user/gate-r129/ref` rather than the r122 bank it was written
against:

```sh
PYTHONPATH=../dblunder-r123 python3 ../dblunder-r123/corpus-rules.py /home/user/gate-r129/ref \
    > corpus-rules.txt 2> corpus-rules.err
```

| kind | scored | exact |
|---|--:|--:|
| single underline | 9125 | **9084** |
| strikethrough | 5978 | **5978** |
| | **15 103** | **15 062** (99.73 %) |

with 1184 scaled, 151 document-embedded and 84 unreadable-face rules set aside, and the same 41
misses. Every figure reproduces round 123's exactly, and the `scaled-docs.txt` it writes is
byte-identical to round 123's — which is a **C11** check worth having on its own, since it is the
same model against a *different* banked rendering of the same corpus.

**And it is 15 062 of 15 103 before and after, because it structurally cannot move.** The script
shells out only to `fc-list`, never to our CLI: it scores the Python reimplementation of the VCL
chain in `predict-pos.py`, which has no page transform in it and which this round did not touch.
It also buckets every scaled rule out by construction. So the number the round was to be judged on
is a property of the model and the bank, not of this change — the sheets-track figures in §1.6 are
the control on the C#, and §3 is the control on everything else.

**Two of its 41 misses are nevertheless now right.** `067_Basic_invoice_Use_this_template` is one
of the "two rows whose thickness lands on a whole unit by luck on a scaled page" that round 123
listed: 26.2.4.2 draws 0.3970 pt — a clean 14 hundredths, which is why the scaled-page detector
passed it over — at a depth of 0.8930 where the unscaled chain predicts 0.9071. `compare-rules.py`
scores that document **0 of 2 before and 2 of 2 after**. The predictor still misses it, because the
predictor does not know the zoom.

---

## 3. The confinement measurement — 947 documents, twice, byte for byte

`render-all.py` renders every corpus document with one binary under `SOURCE_DATE_EPOCH=1700000000`
and hashes the PDFs. Rendering is byte-deterministic: three documents rendered twice with the same
binary gave identical digests before the sweep started.

```sh
python3 render-all.py "$CLI" /home/user/sample-files all-docs.txt after-hashes.tsv 3
#   ... revert, rebuild ...
python3 render-all.py "$CLI" /home/user/sample-files all-docs.txt base-hashes.tsv 3
```

**On disk this is a chunked sweep of size one.** Each document is rendered into its own temporary
directory, hashed, and the directory removed in a `finally` before the worker takes the next one —
one directory per *document* and never per worker slot, which is `CLAUDE.md`'s rule and the thing
that cost round 124 a hundred renders. Peak footprint is three documents' PDFs, the whole probe
directory is **1.5 MB**, and free disk was **3.9 GB before the run and 3.8 GB after**. Nothing was
kept and nothing needed deleting between chunks; the totals below are whole-corpus totals, not
partial ones.

| track | rendered | bytes changed |
|---|--:|--:|
| docx | 272 | **0** |
| doc | 66 | **0** |
| pptx | 251 | **0** |
| ppt | 51 | **0** |
| xlsm | 2 | **0** |
| xlsx | 241 | 27 |
| xls | 64 | 4 |
| | **947** | **31** |

947 of 947 rendered successfully under both binaries. **Words, slides and every chart on them are
bit-identical**, which is the confinement claim measured rather than argued. `hash-diff.tsv`.

### 3.1 The 31, against round 123's list of 31

They are not the same 31, and both differences are informative.

**Six moved that round 123 did not classify as scaled**: `067_Basic_invoice_Use_this_template`,
the two `Lease-Transition-Records-Checklist` workbooks, `6880ac7361ca1b99a9230811_ST Capability
List Rev.16 - Web`, `CSA_CCM_v1.2` and `NPA_21_21_Sentenced_Comments`. The first is one of round
123's own 41 misses and goes 0 of 2 to 2 of 2 (§2). **For the other five the sign of the move is
unmeasured**: `compare-rules.py` pairs no rule at all on four of them and none of the three it
finds on the fifth, so this round can say their bytes changed and not whether they changed for the
better. They are recorded here rather than counted as a win.

**Six of round 123's list did not move**: the two `.pptx`, and `FAA-2019-0995-0002_attachment_2`,
`INDEX_Digital_Transformation_Toolkits`, `TICAPCapability_Final` and `apron-area`. Those four are
at a page transform of one — which is exactly why four of the six residual disagreements in §1.7
are theirs.

### 3.2 Impress and Writer needed no change, and the census says so rather than the reasoning

The seat said "three layout models". Only one of them turned out to have a page transform on this
path.

- **Impress.** The two scaled-set decks fit a transform of **100%** in §1.3 and already agreed on
  26 of 27 and 3 of 4 rules; both render byte-identically before and after. An autofitted slide
  scales the *stated size* and the device then quantises at the scaled size, which is a different
  mechanism and is already right.
- **Writer.** None of the 31 is a word-processing document, and all 338 of them are
  byte-identical. `MetricGrid.WriterTextLine` is untouched.

Recorded as measured-nil rather than as "not done".

---

## 4. O76 — there is no router, and the register's prescription would have built one

### 4.1 The refutation, in source

The register says the seat is "model what an `SdrEdgeObj` actually draws, which is a routed
connector with its own vertex logic". It is not. `SvxMSDffManager::ImportShape`'s connector branch
ends with `SetEdgeTrackPath( aPoly )` (`filter/source/msfilter/msdffimp.cxx`:4888), where `aPoly`
is the custom shape's own `GetLineGeometry(true)` taken at :4812 — and:

```cpp
// svx/source/svdraw/svdoedge.cxx:1761-1779
void SdrEdgeObj::SetEdgeTrackPath( const basegfx::B2DPolyPolygon& rPoly )
{   ...
    *m_pEdgeTrack = XPolygon( rPoly.getB2DPolygon( 0 ) );
    m_bEdgeTrackDirty = false;
    m_bEdgeTrackUserDefined = true;
    ...
}

// :574-580
void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    // #i120437# if bEdgeTrackUserDefined, do not recalculate
    if(m_bEdgeTrackUserDefined)
    {
        return;
    }
```

So the routing never runs. The `SdrEdgeKindItem` (`OrthoLines`/`Bezier`/`OneLine`) and the four
630-unit node distances the branch puts on the object are metadata for a recalculation that
`#i120437` forbids; what is drawn is the **preset's own polygon**, after the rotation and the two
mirrors that :4793-4810 apply to the custom shape *before* the geometry is taken.

### 4.2 The refutation, against the binary — C8's second leg

26.2.4.2's own `--convert-to fodp` writes a user-defined edge track out as an explicit path, so the
question can be asked of the binary directly rather than inferred from a rendering. Each of the
three connector-bearing decks converted **twice into separate profiles** (**C11**); the two
`.fodp` are byte-identical in each case.

On `ws_prod-…-Approval-of-Flight-Conditions-RM-NAA-16032007.ppt` every connector is a
`<draw:connector>` carrying its own `svg:d`, and the first segment as a fraction of the viewBox is:

```
M4899 6820h701v-191h701          vb 1403x192     0.4996
M12898 11425h3702v-1602h3702     vb 7405x1603    0.4999
M12898 6629h667v96h334           vb 1002x97      0.6657     <-- not a midpoint
M16199 7924v1301h-6600v1301      vb 6601x2603    0.4998
M6901 16526h-843v-644h-819       vb 1663x645     0.5069     <-- not a midpoint
M18900 5925h-3382v500h-3382      vb 6765x501     0.4999
M6901 16526h-931v-644h-907       vb 1839x645     0.5063     <-- not a midpoint
M19301 15227h-3400v-801h-3399    vb 6800x802     0.5000
```

**A router bends an orthogonal connector at the midpoint.** Three of these eight do not, and
0.6657 is two thirds to four digits. That is the preset at its stored adjustment, written straight
out as the edge track — the mechanism above, measured on the reference binary.

### 4.3 The census, with the base rate

All three decks, every `<draw:connector>` 26.2.4.2 exports:

| deck | connectors | straight | no adjustable bend | 3-segment at the default | 3-segment off it | curved |
|---|--:|--:|--:|--:|--:|--:|
| `…Approval-of-Flight-Conditions…` | 22 | 3 | 7 | **6** | **6** | 0 |
| `introduction_to_bea_tuxedo` | 97 | 6 | 5 | 0 | 0 | 86 |
| `pods05` | 230 | 184 | 24 | 0 | 0 | 22 |
| | **349** | 193 | 36 | 6 | 6 | 108 |

So **193 of 349 are straight lines with no adjustment at all** and another 36 have no adjustable
bend — that is the base rate, and it is most of the population. Of the bent family, half sit at the
default and would not move under any adjustment fix. For the 86 curved ones in
`introduction_to_bea_tuxedo` the first control point's fraction clusters at **0.745–0.757 on 30**
(the default) and at **0.666–0.667 on 21** (a stated two thirds), with **34 the axis heuristic
cannot classify** — so the curved half is *indicative and not settled*, and it is stated that way.

### 4.4 Why round 127's arm made things worse, and what the seat actually is

Round 127 measured that honouring the stored adjustment moved the interior-vertex residual
**6.63 → 8.73 pt** and read that as "the preset is not what is drawn". §4.1 and §4.2 say the preset
*is* what is drawn. The reconcilable reading is the one the census gives: the arm applied a
conversion to all 57 shapes when half of the bent ones and a third of the curved ones sit at the
default, so a conversion that is wrong anywhere is paid for everywhere. Round 127's own two named
witnesses already point that way and it says so — one where the **default** is right to 0.004 pt
and one where the **stated** value is right to 0.064.

**The seat is therefore not a router and not "apply the adjustment" either. It is the conversion
and the clamp**, on the shapes that state a non-default value.

### 4.5 The decision

**NOT WORK on reach.** The reachable population is the connectors whose reference bend is not the
default: **6 bent in one deck and about 21 curved in another**, inside **3 of 947 documents**
(0.3%). Two of those three decks — `introduction_to_bea_tuxedo` and `pods05` — hold 327 of the 349
connectors and round 127 measured that their polylines *do not pair with the reference on their end
points at all*, so on 94% of the population the adjustment is not the first defect and fixing it
would not be visible. Against that, the change is a per-preset conversion audit of the Escher
adjustment against `EnhancedCustomShape2d`'s clamp for nine connector types.

The register row is corrected rather than deleted: a future round reading it as written would build
a router that `#i120437` guarantees is never called.

---

## 5. Tests

### 5.1 New

`RuleWidthTests.AScaledPageQuantisesItsRuleBeforeTheTransformAndNotAfterIt` — the witness of §1.2,
asserting **both** answers so the two are separated by the change and not by a tolerance: the
scaled grid gives `18 × 0.6` and `41 × 0.6` hundredths (0.306 and 0.697 pt, which is what
26.2.4.2's PDF says), the unscaled grid at the same size gives 11 and 25 (0.3118 and 0.7086, which
is what this tree drew).

`RuleWidthTests.AGridWithNoUsableTransformIsTheGridItself` — five rows: 1.0, 0.0, −0.5, NaN and
+∞ all give the grid back unchanged. This is the guard §1.5 rests on.

### 5.2 The full run, discovered against passed, project by project

| project | discovered | passed | failed | skipped |
|---|--:|--:|--:|--:|
| Containers | 109 | 109 | 0 | 0 |
| Core | 591 | 591 | 0 | 0 |
| Markup | 249 † | 259 | 0 | 0 |
| OpenDocument | 169 | 169 | 0 | 0 |
| Presentations | 1205 | 1205 | 0 | 0 |
| Rendering | 164 | 164 | 0 | 0 |
| Spreadsheets | 1387 | 1387 | 0 | 0 |
| Text | 750 | 750 | 0 | 0 |
| Vector | 309 | 309 | 0 | 0 |
| WordProcessing | **1980** | **1980** | 0 | 0 |
| **ten projects** | | **6923** | **0** | **0** |
| Fidelity | 552 | 542 | **10** | 0 |

† `--list-tests` reports 249 distinct test *names* for Markup and the run executes 259 *cases*; the
extra ten are theory rows the listing folds into their method. Every other project's two numbers
agree, so this is the instrument and not a lost or duplicated test.

`XRay.Office.WordProcessing.Tests` is the project two rounds saw truncate in one day; **1980
discovered, 1980 passed** on this run, matching round 128's corrected figure. The ten non-fidelity
projects total **6923**, which is round 128's 6917 plus this round's six new cases.

The **ten fidelity failures are the ten standing ones** and no others — four
`PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt`, four
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop`, one
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt` and one
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt`. `0 skipped`, so the suite
covered its 23 documents rather than skipping for a missing `soffice`.

---

## 6. What this round did not settle

- **The five new movers of §3.1 whose rules nothing pairs.** Their bytes changed and no instrument
  here can say in which direction. A rule the scanner cannot pair is not evidence either way.
- **The six residual rules of §1.7.** Four are a single hundredth of a millimetre of depth on
  unscaled pages, one is `apron-area`'s pre-existing 0.1992 / 0.3975, and one is a slide rule
  0.0017 pt out. None is characterised and none is O70.
- **The curved half of O76's census.** 34 of 86 control-point fractions are not classifiable by the
  dominant-axis heuristic used here.
- **Whether a `.ppt` connector's adjustment is converted correctly** — §4.4 narrows O76 to that
  question and does not answer it.
- **`predict-pos.py` still has no page transform**, so `corpus-rules.py` will keep reporting the two
  `067_Basic_invoice` rules as misses even though the renderer now gets them right. Teaching the
  predictor the zoom would need the zoom, which is the thing the page does not state (round 123's
  own reason for excluding scaled pages).

---

## 7. Files

| file | what |
|---|---|
| `fitzoom.py`, `fitzoom.tsv` | one whole-percent transform per document, fitted to round 123's banked pairs |
| `fitzoom-page.py`, `fitzoom-page.tsv` | the same, one per page; this is where the 1412 was predicted |
| `before.tsv`, `after.tsv` (+ `.pairs`) | the 31 scaled documents, ours against the banked reference |
| `sheets-before.tsv`, `sheets-after.tsv` (+ `.pairs`) | all 307 spreadsheets, the same comparison |
| `render-all.py`, `base-hashes.tsv`, `after-hashes.tsv`, `hash-diff.tsv` | the 947 × 2 byte comparison |
| `corpus-rules.txt`, `corpus-rules.err`, `scaled-docs.txt` | round 123's model re-scored against the r129 bank |
| `all-docs.txt`, `sheets-docs.txt` | the two document lists |

**To re-run `corpus-rules.py` you need `ln -s predict-pos.py ../dblunder-r123/predict_pos.py`.**
Round 123 banked the module as `predict-pos.py` and imports it as `predict_pos`, so the script
cannot import itself as banked. The symlink is deliberately **not** committed — a symlink reads as
a zero-byte file to `git add` and this tree has been bitten by that.
