# slides-r59 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `dc9ca5900c2`, branch `wt-slides-r59`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.
`prediction.md` beside this file was committed as `9cc46d92a5f`, before anything was built or
rendered post-change.

## The baseline reproduces, and one briefed instrument reading does not

| | briefed | measured at `dc9ca5900c2` |
|---|---|---|
| passing over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` | 1106.97 | **1107.04** |
| signed ink | 802.45 | **802.52** |
| major pages | 385 | **385** |
| differing pixels | 19702.17 over 4530 pages | **19702.23 over 4530 pages** |

The sweep's own `TOTAL` is **315** files for 302 manifest paths, four more than round 56's 311
with no commit to the corpus — the alias materialisation `CLAUDE.md` records, arriving again.
Everything here is scored against `MANIFEST.tsv` by a scorer that now **refuses to print** unless
every manifest path found a row (`score-manifest.py`, one added `SystemExit`), and the same guard
is in the new `diffpix.py`.

**`tf-agreement.py` prints `0.77063` at this base over 4515 pages, 1708 of them exact.** The
brief said round 56 measured `0.85188` where *its* brief said `0.77061`, and treated the
instrument's own reading as having moved. It has not: the figure at this base reproduces the
original 0.77061 to four decimal places. **Two of the three readings agree and round 56's is the
outlier** — most likely a different `ours` directory rather than a different tree, since the
script's mean is per document and a missing document changes it. Recorded rather than chased.

## The whole round

| | base | §1 | §1+§2 | §1+§2+§4 | **final (+§5)** |
|---|---:|---:|---:|---:|---:|
| passing over `MANIFEST.tsv` | **199 of 302** | 199 | 199 | **200** | **200 of 302** |
| page counts changed | | 0 | 0 | 0 | **0 of 302** |
| `abs_ink` | 1107.04 | 1106.79 | 1097.26 | 1097.23 | **1039.95 (−67.09)** |
| signed ink | 802.52 | 802.16 | 788.97 | 788.96 | **738.86** |
| major pages | 385 | 383 | 379 | 379 | **375** |
| **differing pixels over 4530 pages** | 19702.23 | 19688.93 | 19669.89 | 19669.62 | **19414.43 (−287.80)** |

**Forty-one documents moved on ink — 22 improved, 19 worsened — and 42 on differing pixels, 39
improved and 3 worsened. The regressions are named, not netted.**

| Δ `abs_ink` | document | before → after |
|---:|---|---|
| **−54.26** | `Wildlife for REDAC September 11.pptx` | 61.92 → **7.66**, differing pixels 346.73 → **102.53** |
| **−7.20** | `Demick_JetBlue.pptx` | 12.85 → **5.65** |
| −1.67 | `171128IPAP.pptx` | 13.69 → 12.02 |
| −1.26 | `082_Infographic_Funnel_with_4_Stages…pptx` | 2.97 → 1.71 |
| −1.04 | `southern-classic-kennesaw-state-university-final.pptx` | 12.69 → 11.65 |
| −0.87 | `089_Infographic_Radial_Matrix…pptx` | 1.51 → 0.64 |
| −0.80 | `081_Infographic_Funnel_w_3_Stages…pptx` | 2.50 → 1.70 |
| −0.74 | `scatter_chart.pptx` | 1.34 → 0.60 |
| −0.61 | `3492.pptx` | 4.80 → 4.19 |
| −0.50 | `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 47.76 → 47.26 |
| −0.39 … −0.33 | five `*_advanced_powerpoint_bar.pptx` | ≈0.50 → ≈0.12 each |
| −0.20 … −0.01 | `line_chart`, `1_Country-Updates_DRC_English`, `stacked_bar_chart`, `Intersil…`, `010605Vul.ppt`, `flying-by-numbers`, `Airport Planning 09112013` | |
| **+1.02** | `N2_E_Maestroni_Swarm_COP.pptx` | 3.93 → **4.95** |
| **+0.41** | `bar_chart.pptx` | 0.24 → **0.65** |
| +0.33 ×4 | `003/011/019/027_advanced_powerpoint_line.pptx` | 0.03 → 0.36 |
| +0.29 | `combo_bar_line_chart.pptx` | 0.47 → 0.76 |
| +0.27 … +0.19 | four `*_advanced_powerpoint_column.pptx` | ≈0.03 → ≈0.25 |
| +0.18 | `FAAAIandtheArtandScienceofV&Vfinal.pptx` | 4.14 → 4.32 |
| +0.08 … +0.02 | `stacked_area_chart`, `RPA P4`, `038_Competitive_Advantage_Card…`, four `*_advanced_powerpoint_area` | |

### The unsigned ink column reports a geometry fix as a regression, and the pixels say otherwise

`bar_chart.pptx` is the clean case and it is worth reading before anything else in that table.
Its plot rectangle went from `dLeft +4.21 dBottom +4.89` to `dLeft −0.05 dBottom +0.64` — the
defect the round set out to fix, gone — and the `cmp` report for its single page went

```
   base    diff% 4.98   ink% 0.24   |ink|% 0.24   4 regions
   after   diff% 2.85   ink% 0.12   |ink|% 0.34  17 regions
```

**The differing-pixel count nearly halved and the unsigned ink rose.** One large one-sided offset
became seventeen small two-sided ones, and `|ink|%` sums region magnitudes without sign. Nineteen
of this round's twenty-two "worsened" documents are that shape: on differing pixels the same set
is **39 improved to 3 worsened**. The brief's own rule — *the ink column is the wrong instrument
for a size-only change* — is what this table is for.

## 1. The plot rectangle is a tick length the axis never draws

The brief's item 1, and it is not a displacement: it is a **reservation for a tick mark the file
says is not there.**

A whole-corpus census of every rendered chart page's plot rectangle
(`plotrect-census.py`, read off the *gridline* families in both PDFs rather than off a fill,
because the two stacks paint the wall differently) reproduces both of the brief's known figures
before it says anything new — `Demick_JetBlue` page 4 `dLeft +5.69 dBottom +5.54 dRight −0.09
dTop +1.08`, `N2_E_Maestroni` page 7 `dLeft +15.60 dBottom −21.65 dRight +8.56` — and 634 of its
853 candidate pages agree on all four edges to 0.5 pt, which is the control that says its zero is
a real zero. **Then it shows a cluster at `dLeft ≈ +4.2, dBottom ≈ +4.9` on a dozen unrelated
documents.** `TickLength` is `AXIS2D_TICKLENGTH` = 150 hundredths of a millimetre = **4.252 pt**.

The discriminator is `c:majorTickMark`, which this reader did not read at all. `make-tick-probe.py`
patches that one property, one axis at a time, in a corpus chart already stating `none` on both
axes — so no arm states the reference's own default and the arms differ in nothing else. Rendered
through 26.2.4.2, **6 of 6**:

```
                 dLeft   dBottom
  none            0.00     0.00        in (both axes)      0.00   0.00
  out (val)      +4.25     0.00        out (cat)           0.00  +4.25
  out (both)     +4.25    +4.25        cross (both)       +4.25  +4.25
```

`lclGetTickMark` (`axisconverter.cxx:104-115`) maps `out` and `cross` to a style carrying
`OUTER` and `in` and `none` to one that does not; only an outward tick extends past the plot
area for `VDiagram::adjustInnerSize` to be charged for it.

**And the labels do not move with it.** The leftmost value label's pen sits at the same `x` in all
four arms, so what the tick buys is the *gap* between the label and the axis. A reader that
stopped reserving the tick and kept offsetting the label by it would move every label 4.25 pt
outward; that half is a test of its own.

**An absent element is not `none`.** `AxisModel`'s constructor defaults it to `out` for an
MSO-2007 chart part and to `cross` for a later one (`axismodel.cxx:42-48`), and both reserve — so
the default here is `Outer` and only a stated `none` or `in` changes anything. The corpus states
the element on 481 of its 494 axes; the default decides 13, in two documents.

### What it was worth, and what it left

Plot rectangles over the 57 chart pages the instrument can measure (a page qualifies only when
*both* sides show a comparable gridline family):

| | base | final |
|---|---:|---:|
| all four edges within 0.5 pt | 10 | **13** |
| `dLeft` over 1 pt | 22 | **9** |
| `dBottom` over 1 pt | 23 | **11** |
| `dTop` over 1 pt | 10 | **9** |
| `dRight` over 1 pt | 31 | **31** |

**The right edge did not move and is now the dominant remaining plot-rectangle defect** — 31 of
57 chart pages, unchanged, and a different mechanism from the one this round fixed. That is the
next round's first item and this census is the instrument for it.

## 2. The automatic gridline and axis line are the theme's subtle line style — and it is three
things, not one

The brief's item 2 named two colours. The measurement names four things, and one of them the
brief does not mention: **every axis line in this reader was black.**

`make-grid-probe.py`, five arms, one thing patched per arm in one deck, each rendered through
26.2.4.2 and read back with `strokecols.py`:

| arm | major grid | minor grid | axis line | width |
|---|---|---|---|---|
| base, theme `tx1` = `000000` | `#666666` | `#8B8B8B` | `#666666` | 0.73 pt |
| theme `dk1` → `2050C0` | `#676E9C` | `#8B8FA7` | `#676E9C` | 0.73 pt |
| theme `dk1` → `FFFFFF` | `#BCBCBC` | `#BCBCBC` | `#BCBCBC` | 0.73 pt |
| `lnStyleLst[0] w` 9525 → 38100 | `#666666` | `#8B8B8B` | `#666666` | **3.00 pt** |
| `lnStyleLst[0] w` 9525 → 4763 | `#666666` | `#8B8B8B` | `#666666` | **0.37 pt** |

We drew `#B3B3B3` for both grids, `#000000` for every axis line, and a hairline for all three.

**The white arm is the arm that decides it.** A tint of white is white whatever the tint, so two
different tints can only collapse onto one value if something *after* them is doing the darkening
— and `shade 50000` of white is `#BCBCBC` exactly. A constant grey, which is what this reader
drew, cannot produce the middle row at all. The mechanism is `LineFormatter`: it copies
`Theme::getLineStyle(THEMED_STYLE_SUBTLE)` whole and resolves it with the automatic entry as the
`phClr`, so the theme's own `shade`/`satMod` act on the tint and the same line style supplies the
width. `DrawingChartAutoFormat.ThroughSubtleLineStyle` already existed for a series stroke; only
the three furniture tables were missing.

The three widths are `round(EMU / 360)` hundredths of a millimetre, to the hundredth of a point
on all three arms.

**The tick labels do not move with the axis line.** Both stacks draw `Demick_JetBlue` page 4's
labels `#000000` — that is `c:txPr`'s answer, not this one — so only the *line* changed.

After §1 and §2, page 4's stroke census is the reference's: 28 minor horizontals at `#8B8B8B`,
22 category majors at `#666666`, 21 category minors at `#8B8B8B`, 8 stated `#F07F09`, at 0.75 pt
against the reference's 0.73. Two small residues remain and are recorded rather than fixed: we
draw **20** category minor lines where the reference draws 21 (one interval short at the end),
and our extra face count on that document is unchanged.

## 3. `WmfReader`'s other fixed fields — asked, and the answer is one real gap with no ink in it

Round 56 found `EmfReader` treating a NUL-*terminated* face name as NUL-*padded* while
`WmfReader` read the same structure correctly. The brief asked whether the disagreement runs the
other way anywhere else. **It does, in one place, and it is a bounds check rather than a
terminator.**

`WmfReader.CreateFont` reads a fixed 32 bytes for the face name without checking the record's own
end. `wmf-facename-census.py` over the whole corpus: **450 `CreateFontIndirect` records, 442 of
them with the full 50-byte payload and 8 short** — 26, 32, 36 and 20 bytes, in three words
documents. All eight carry a terminator inside the record, so `IndexOf((byte)0)` stops before the
over-read matters and no corpus document draws a wrong face because of it. Recorded, not fixed:
the fix is a `Math.Min` and the measurement to justify it does not exist.

The two readers have exactly one fixed-width string field each and no others; every other buffer
in `Wmf/`, `Emf/` and `EmfPlus/` is length-prefixed or sized from the record.

**The other 11 EMF face-name documents from round 56's census were not individually re-checked**
and remain open — but see §4, which found a *different* metafile-font defect in five of the same
kind of document by following the corpus rather than the list.

## 4. `010605Vul.ppt` — the three words, and they were never words

The brief's item 4, and the charstream test settles it in one reading. Stripped of all
whitespace, our extraction and the reference's are **5989 characters each** and differ only by
substitution: the reference emits `U+F0E8` and `U+F059` where we emit `è` and `Y`, 25 times.
Nineteen of those tokens carry a letter for us and none for the reference, which is the whole of
963 against 944.

So the brief's framing — *"three extractable words we should not be splitting"* — is **refuted**:
there is no tokenisation difference at all. There is a font.

Following it: page 9's graphic is an **EMF**, and every one of its symbol runs names
**`Monotype Sorts` at `lfCharSet = 2`**. A symbol-charset font addresses glyphs by slot;
LibreOffice moves the byte into the Private Use Area and recodes it into OpenSymbol, and we
decoded the byte through Windows-1252 and drew a Latin letter in a serif face. The `.ppt` font
table has no symbol font in it at all — two entries, Times New Roman and Arial, both
`lfCharSet = 0` — which is why every census keyed on the *presentation* format found nothing.

`(c & 0x00ff) | 0xf000` then `ConvertChar::RecodeChar`'s table, gated on the character set **and**
one of the fourteen families `SymbolFontRecode` has a table for — which is LibreOffice's own rule,
because a symbol-encoded request never reaches fontconfig and the substitution it lands on is
StarSymbol or OpenSymbol and nothing else. Both text paths, since a 16-bit `EMR_EXTTEXTOUTW`
widens the slot rather than translating it.

| `010605Vul.ppt` | before | after |
|---|---:|---:|
| extractable words (reference 944, band 962.88) | 963 | **947** |
| verdict | `words` | **`match`** |
| `abs_ink` | 1.07 | 1.05 |
| embedded font list | 6, no OpenSymbol | 7, OpenSymbol added |

**The verdict round 56 spent is back.** The font list is now one face longer than the reference's
rather than one shorter: we add OpenSymbol correctly and still draw a Liberation Serif the
reference does not. That is a residue, it is not a gate column, and it is named here rather than
rounded off.

Reach, `metafile-symbol-census.py` over EMF and WMF blobs in every corpus document including the
compound-file ones: **92 font objects in 10 documents** — 5 slides decks, 4 words documents and one
workbook — naming Symbol (63), Monotype Sorts (25), Wingdings (2) and `UniversalMath1 BT` (2).
The last has no recode table and is deliberately left alone. Only `010605Vul.ppt` moved a word
count; `Airport Planning 09112013.ppt` moved 0.01 of ink and the other three slides decks moved
nothing measurable.

## 5. The 24.2.7.2 audit — the largest single item in the round

Re-derived at `b2795920159` with the file's own commands, dated 2026-08-21:

```
open sites 39 in 30 files
   WordProcessing 11   Spreadsheets 9   Presentations 9   Text 6   Core 2   Rendering 1   Ooxml 1
markers        VERIFIED 19   FIXED 2   WRONG 1   UNDECIDED 0
```

The open count reproduces the file's own last table exactly; the marker lines have gone 19 → 22
since it was written.

**Site re-checked: `PptxSlideLayout.cs:1591`, the `a:fillToRect` focus.** It states three rules
and round 39 measured all three on the superseded binary. Re-running *that round's own four-arm
fixture* (`probes/slides-r39/make-gradient-path-fixture.py`) through 26.2.4.2's flat-ODF export:

| `a:fillToRect` | focus | round 39, 24.2.7.2 | **26.2.4.2** | |
|---|---|---|---|---|
| `t="-80000" b="180000"` | (50, −80) → (50, 0) | radial 50%/0% | **radial 50%/0%** | clamp **VERIFIED** |
| `r="99000" b="99000"` | (0.5%) → 0 | linear 225° | **radial 0%/0%** | truncation **VERIFIED** |
| `r="98000" b="98000"` | (1%) | radial 1%/1% | **radial 1%/1%** | truncation **VERIFIED** |
| `l="100000" t="100000"` | (100, 100) | linear 45° | **radial 100%/100%** | corner test **WRONG** |

Two of the three rules hold and the third is gone. The truncation to whole per cent is still
observable — 0.5% lands on 0 and 1% does not — but the *branch it fed* no longer exists on the
running binary: 26.2.4.2 exports `radial` for all four arms, corners included. The corner branch
is removed and `SlideGradientPathTests` is re-stated against the running binary rather than
against the superseded one.

**It is worth −54.26 of unsigned ink and −244.20 of differing pixels on one document.**

| document | ink | differing pixels |
|---|---|---|
| `Wildlife for REDAC September 11.pptx` | 61.92 → **7.66** | 346.73 → **102.53** |
| `082_Infographic_Funnel_with_4_Stages…pptx` | 2.97 → 1.71 | 12.79 → 9.05 |
| `089_Infographic_Radial_Matrix…pptx` | 1.51 → 0.64 | 12.17 → 7.94 |
| `081_Infographic_Funnel_w_3_Stages…pptx` | 2.50 → 1.70 | 12.35 → 9.42 |
| `3492.pptx` | 4.28 → 4.19 | 50.36 → 50.27 |

Five documents, all improved, none worsened, no verdict moved. `Wildlife for REDAC September 11`
was the track's third largest document by unsigned ink and is now outside its top twenty.

**The lesson the audit file already records, arriving again**: a re-check is worth running even
when two of its three rules come back clean, because the one that does not is the one nobody
would have looked at. And the marker convention earned its keep here — the site's own prose said
"measured against LibreOffice 24.2.7.2", which is what put it on the list.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdicts **0**, band −1 … +1 | **+1**, 199 → 200. Outside the band on its good arm, and for a change (§4) whose mechanism was not known when the prediction was written |
| 2 | page counts 0 of 302 | **0 of 302** ✓ |
| 3 | documents moved on ink **12 – 22** | **41** ✗ — and the composition is wrong in an instructive way: 22 came from §1+§2 as predicted, 5 from §5 which did not exist as a plan, and the rest from a `bar_chart`-shaped family the prediction named but under-counted |
| 4 | `abs_ink` **−4 … −14** | **−67.09** ✗, past the band by a factor of five, and §5 is 54 of it |
| 5 | `Demick_JetBlue` 12.85 → **below 11** | **5.65** ✓ |
| 6 | `N2_E_Maestroni` 3.93 → **below 3.5**, major stays 1 | **4.95** ✗ on ink, major 1 → 1 ✓. Its differing pixels are 149.74 → 149.67 — unmoved — because **its plot area does not take the computed path at all**: it states a `c:manualLayout` and §1 cannot reach it. The brief's "N2's plot is 15.6 pt right" reproduces exactly and is a *manual-layout* defect, not the one this round fixed |
| 7 | plot-rect census: pages exact on all four edges 10 → **20 or more** | **13** ✗ — but the per-edge counts moved as the mechanism predicts (`dLeft` 22 → 9 over 1 pt, `dBottom` 23 → 11) and `dRight`, a defect this round does not touch, holds 31 of the 57 pages below the threshold on its own |
| 8 | controls: `tf-agreement`, exact `/Tf` pages, sheared glyphs all unchanged | see below |

**The documents-moved band has now missed in five of the last six rounds, and this round missed
it upward by taking on a fifth item after the prediction was written.** The honest reading is that
the quantity is only predictable for the changes named in the prediction, and every round so far
has found at least one it had not named.

## Refutations

1. **The brief's item 1, one level down.** The plot rectangle is not displaced; it is
   over-reserved by exactly one `AXIS2D_TICKLENGTH` on any axis whose `c:majorTickMark` says
   `none` or `in`. Six probe arms, 6 of 6, on a corpus chart with one property patched per arm.
2. **The brief's item 1 for `N2_E_Maestroni`.** Its 15.6 pt reproduces exactly and is *not* the
   same defect: its chart states a plot-area `c:manualLayout`, so `PlotAreaOf` returns before any
   reservation is computed and the fix cannot reach it. Two of round 56's "one defect" are two
   defects.
3. **The brief's item 4.** `010605Vul.ppt`'s three words are not a tokenisation difference — the
   two charstreams are the same length to the character and differ only by 25 substitutions of a
   Private Use Area code point for a Latin letter. The cause is a symbol-charset font inside an
   EMF, invisible to every census keyed on the presentation format.
4. **`objectformatter`'s tint is not the whole of the automatic colour.** A `tx1` of white gives
   `#BCBCBC` for *both* tints on the reference, which no tint alone can produce; the theme's own
   `shade 50000` around the substituted `phClr` is the other half. And the automatic layer states
   a **width**, which neither the brief nor this reader had.
5. **The unsigned ink column as this round's instrument.** Nineteen of twenty-two documents that
   "worsened" on `abs_ink` improved on differing pixels, `bar_chart.pptx` most clearly:
   `diff% 4.98 → 2.85` while `|ink|% 0.24 → 0.34`, because one one-sided offset became seventeen
   small two-sided ones.

## The vision reading

Three pages, each chosen for a stated reason, each handed to a fresh subagent with the composed
image and nothing else — forbidden from reading any project file or running any command, and
asked to describe each half alone before comparing.

### `Demick_JetBlue.pptx` page 4, cropped to the plot area at 200 dpi

Chosen because it is the round's central page and because round 56 recorded that *the composed
pair is unreliable below about ten points* — so this one was cropped and rendered at 200 dpi
rather than composed whole.

The reviewer reported, unprompted:

* **"The top half's teal markers point up; the bottom half's teal markers point down. This is the
  clearest content difference."** That is round 56's open item 8 — `typegroupconverter.cxx` names
  the automatic marker cycle *square, diamond, arrow down, arrow up* and `ChartMarker` has one
  triangle — found independently by a reader who had not been told it existed. **Confirmed** by
  the source and by round 56's own measurement; still not implemented.
* **"The top half's last category label runs off the right edge and is clipped; the bottom's ends
  cleanly with white space after it… the x-label row extends roughly 50–80 px further right, even
  though the plot boxes are horizontally aligned."** A new lead, and it points at the same right
  edge the plot-rect census says is untouched. **Not yet checked by a second instrument.**
* Registration: *"Left border x≈75 and right border x≈1345 in both. No shift left or right… the
  two renderings are vertically aligned."* — which is §1 and §2 landing, seen rather than measured.
* The reviewer flagged its own uncertainty about the title, correctly: the crop cut it.

### `010605Vul.ppt` page 9 at 150 dpi

Chosen because the charstream test had already localised the verdict to 25 characters and a page
reading was the cheapest way to find out what they *look* like.

The reviewer reported: **"Wherever the bottom half shows a black right-arrow symbol, the top half
shows the literal letter `è`… Both of these are the classic symbol-font fallback signature: the
Wingdings code points `è` and `Y` map to an arrow and a star, and the top render is failing to
apply the symbol font."** That is §4, named from the pixels, with the right mechanism and the
wrong font family — the file's face is Monotype Sorts, not Wingdings, and both are in the same
recode table. **Confirmed** by the EMF font census (25 `Monotype Sorts` records at
`lfCharSet = 2`) and by the fix moving the word count 963 → 947.

It also delivered three leads with no metric behind them, none of which this round chased:
the leftmost run of the timeline is drawn **black by us and red by the reference**; the node
markers are **blue by us and maroon by the reference**; and the coat-of-arms picture is
**pixelated with an extra solid black bar along its lower edge**. All three are open.

### `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` page 8 at 130 dpi

Chosen because that document is the track's largest remaining at 47.26 and page 8 is its worst
page (`|ink|% 43.54`), and no round has read it.

The reviewer reported, describing each half alone first: the reference draws **a full-bleed black
chart-object background** (≈600 px tall, edge to edge) and **a dark grey plot wall** inside it,
its title in **white**, and its bars with a visible **white→light-grey vertical gradient**; we
draw no chart background, no plot wall, a black title and bars in "a nearly flat, extremely pale
near-white that is almost invisible against the white page". It also reported that we **wrap** the
first bar's data label onto three lines — `88%` / `(548/621` / `)` — where the reference fits
`(548/621)` on one, and that the reference's plot extends ≈55 px further right.

**Not yet checked by a second instrument**, and it is the single largest unexplained document on
the track. The "extends further right" half is the third independent sighting of a right-edge
difference this round.

## Tests

Three new files, **24 new tests**, and the total reconciles: **4907 = 4883 + 24**.

| test | mutation | outcome |
|---|---|---|
| `ChartLayoutTickMarkTests` (10) | `OuterTick` → `TickLength` unconditionally | **DETECTED**, 5 of 10 |
| `DrawingChartFurnitureTests` (10) | `ThroughSubtleLineStyle(…)` → `placeholder` | **DETECTED**, 3 of 10 |
| `DrawingChartFurnitureTests` (10) | `TicksOf`'s `_ => Outer` → `_ => None` | **DETECTED**, 2 of 10 |
| `MetafileSymbolFontTests` (4) | `IsSlotAddressed`'s family test → `false` | **DETECTED**, 2 of 4 |
| `SlideGradientPathTests` (3, re-stated) | the corner branch put back | **DETECTED**, 2 of 3 |

Five mutations, five detected by reintroduction; none of the five classes is a drift guard.

Each class's inert cases are controls by design: the axis whose edge must *not* move when the
other axis' tick changes, the absent `c:majorTickMark` that must read `Outer` and not `None`, the
chart with no theme that must keep chart2's own grey, and the ANSI run in the same face that must
stay a Latin letter.

**One test was wrong when first written and the failure is worth recording.** It asserted that a
recoded symbol run resolves to OpenSymbol; `0xE8` in Monotype Sorts recodes to `U+27A8`, a real
Unicode arrow, which OpenSymbol does not cover and the ordinary text face does — so the run
correctly falls to DejaVu Sans and draws the right glyph. Asserting the *face* made the test a
fixture for the machine's font set rather than for the recode. It now asserts the recoded code
points, and uses `0x59 → U+E223` — which *is* one of OpenSymbol's — for the face half.

Ten non-Fidelity projects, one at a time: Core 347, Containers 109, Text 617, Vector 302,
Rendering 153 (+1 skipped, the same `PdfFontTests` case as at baseline), Markup 259,
OpenDocument 125, WordProcessing 1180, Spreadsheets 974, **Presentations 841** — **4907 passed,
0 failed, 1 skipped**. `cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

**A test project reported four failures under load and none of them was real.** A first run of
`XRay.Office.Vector.Tests` alongside a corpus sweep reported one genuine failure plus three SVG
ones; run alone, all 302 pass. `CLAUDE.md`'s rule held: a result taken under load is not a result.

## Controls

| | base | final | predicted |
|---|---|---|---|
| sheared glyphs (reference 16008) | 15792 | **15792** | unchanged ✓ |
| pages whose sheared-glyph counts disagree | 82 | **82** | unchanged ✓ |
| `tf-agreement` mean | 0.77063 over 4515 pages | **0.77063** | unchanged ✓ |
| exact `/Tf` pages | 1708 of 4515 | **1708 of 4515** | unchanged ✓ |

Both controls hold exactly. The sheared-glyph figure is also exactly round 56's final one, which
is this round's base: nothing here touches a glyph's transform, and nothing here changes a drawn
em size — the plot rectangle moves labels without resizing them, and the automatic line colour is
not a text property.

## Shared layers — this diff reaches all three tracks and the parent must gate the corpus

Three of the five changes are outside `XRay.Office.Presentations`:

* **§1 and §2** touch `XRay.Office.Core/Charts` and `XRay.Office.Ooxml`.
* **§4** touches `XRay.Office.Vector`.
* **§5** is `XRay.Office.Presentations` only.

Census reach outside slides, counted on what the parts state (`chart-census.py`,
`metafile-symbol-census.py`):

| change | sheets | words |
|---|---|---|
| §1 `c:majorTickMark` on a live axis of a chart on the computed path | **56 chart parts**, ~62 workbooks | **2 chart parts**, 2 documents |
| §2 automatic major grid / axis line / minor grid | 8 / 9 / 0 documents | 1 / 1 / 1 document |
| §4 symbol-charset metafile fonts | 1 workbook (`TICAPCapability_Final.xls`) | 4 documents |

The sheets side is led by `Keywords_Mapping_Graphs_and_Charts.xlsx` (22 live `none`/`in` axes),
then `046_Cost_analysis_with_Pareto_chart`, `029_Annual_budget`, `041_Business_budget`,
`053_Personal_asset_inventory`, `009/010/023_advanced_excel_*`, `004/006_Contextures_chart_sample`,
`052_Manufacturing_output_chart`, `061_Regional_sales_chart`, `040_Blood_pressure_tracker`,
`037_Personal_money_tracker` and the rest of `chartset-00*`. The words side is
`bulletin.docx`-scale: `150_5335_5a.doc` (9 symbol font objects), `150_5300_13_chg10.doc` (5),
`Technical Report and Technical Update Guidelines.docx`,
`easa-regulations-update-20.docx`, and `014_Project_Timeline_Template_Blue_and_Green_Theme.docx`
for §5's gradient.

**Measured rather than argued**, by sweeping each track whole at this tree and scoring the
verdict column against `MANIFEST.tsv`'s `status`:

| track | passing over `MANIFEST.tsv` at this tree | manifest disagreements |
|---|---|---|
| **words** | **319 of 337** (337 of 337 visited) | **0** |
| **sheets** | **275 of 307** (305 of 307 visited) | 3 — see below |

**Words is unchanged and clean.** Sheets needs three sentences, because the raw number is one
below the merge note's 276 and *none of the difference is a regression from this round*:

* `003_advanced_excel_pie.xlsx` and `019_advanced_excel_pie.xlsx` read `manifest=done
  sweep=words`. Both were **already `words`** in round 58's own *base* and *after* sweeps, with
  the identical word counts — 138/143 and 135/140 — so the manifest's `status` is stale for those
  two rows and they have been failing since before this round. Neither has an axis: a pie takes
  no tick reservation, draws no gridline and draws no axis line, so no change here can reach them.
* `044_Cash_flow_forecast_Use_this_template_b8fa1f35.xlsx` reads `manifest=open sweep=match`, and
  that one **is** this round: 427/438 extractable words at round 58's base *and* after, and
  **438/438** here — the reference's count exactly. It carries one live `none` axis, and 4.25 pt
  more plot width is what changed its label arrangement.
* **Two `.xlsm` manifest paths were never visited**, and that is a defect in the sweep script
  rather than in the tree: `track-ink-sweep.sh`'s `find` filter lists `.xls`, `.xlsx`, `.ods` and
  `.csv` and **not `.xlsm`**, so `003_Contextures_chart_sample_9bda2719.xlsm` and
  `007_Contextures_chart_sample_667d8e47.xlsm` are unreachable by it. Reported rather than
  patched: the sheets track owns that script's filter and one of those two documents is in this
  round's §1 census.

Eight more sheets and four more words documents moved their word count **toward** the reference's
between round 58's sweep and this tree — `Keywords_Mapping_Graphs_and_Charts.xlsx` 4511 → 4514 of
4519, four `advanced_excel_*` charts onto the reference's figure exactly — but that comparison is
**confounded twice** and is offered as an indication only: round 56's slides merge landed between
the two sweeps (it is what takes `bulletin.docx` to 3253/3253), and round 58's sweep re-rendered
its own reference, so four of its rows move on the *reference* side. The verdict table above is
the authoritative statement; the parent's whole-corpus gate is the authority over that.

## Left open, in the order the next round should take them

1. **The plot rectangle's RIGHT edge.** The left and bottom edges are largely settled — `dLeft`
   over 1 pt on 22 of 57 chart pages down to 9, `dBottom` 23 down to 11 — and `dRight` **did not
   move at all**: 31 of 57 pages, before and after. It is a different mechanism from the tick
   reservation, `plotrect-census.py` is the instrument for it, and three independent sightings
   point at it this round: the census, a page reviewer noting JetBlue's category-label row runs
   50–80 px further right than the reference's, and a second reviewer noting Pavese's plot wall
   reaches the page edge where ours stops 55 px short. Start with the last value label's
   half-width overhang (`right -= valueLabel / 2`) and the secondary-axis band.
2. **`8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`, 47.26 and now the track's largest.** A page
   reading of its worst page says the reference draws **a full-bleed black chart-object background
   and a dark grey plot wall** where we draw neither, its title in white where we draw black, and
   its bars with a white→grey **gradient** where ours are near-invisible pale flat. Plus a data
   label we wrap onto three lines and the reference fits on two. Four separate claims, none of
   them yet checked by a second instrument, on the biggest document on the track.
3. **`N2_E_Maestroni_Swarm_COP.pptx`'s `c:manualLayout`.** Its 15.6 pt is real and reproduces, and
   §1 cannot reach it: `layoutTarget="inner"` with `x + w = 1.024` — the stated width overruns the
   frame's right edge — and the reference's plot is 129.46…719.60 where the fractions say
   145.06…737.41. `lclCalcRelSize` clamps the size to `1 − x`
   (`oox/source/drawingml/chart/converterbase.cxx:326-340`) and that clamp is **not** what the
   reference draws either (it would give 574.9 pt of width against the reference's 590.1), so the
   rule is a third thing and needs its own probe. 51 corpus chart parts state a plot-area manual
   layout, 20 of them in slides.
4. **The automatic marker cycle.** `typegroupconverter.cxx` names *square, diamond, arrow down,
   arrow up* and `ChartMarker` has one triangle. Round 56 recorded it; a page reviewer who had
   never seen that note reported it as the clearest content difference on JetBlue page 4. No ink
   in it, and it is now confirmed twice.
5. **The fitted bullet's vertical placement** — 1.9 pt too high, `ALIGN_BOTTOM` /
   `aBulletArea.Bottom()`, `outliner.cxx:909-919`. **Untouched for five rounds**, and this round
   did not touch it either.
6. **The 11 EMF face-name documents from round 56's census** are still individually unchecked.
   §4 found a second, different metafile-font defect in five documents of the same kind by
   following the corpus rather than that list, which is an argument for doing both.
7. **`WmfReader.CreateFont` reads 32 bytes without a record bound.** Eight of 450 corpus records
   are short; all eight carry a terminator inside the record, so nothing draws wrong today. A
   `Math.Min` with no measurement behind it.
8. **`010605Vul.ppt` still embeds one face more than the reference** — OpenSymbol is now right and
   a Liberation Serif the reference does not draw is still there. Not a gate column.
9. **The category minor grid is one line short** on `Demick_JetBlue` page 4: 20 against the
   reference's 21, at the last interval.
10. **`2015-Civil-Rights-Website-training.ppt`, 29.64**, still untouched; `wordArtVert`; the
    `pitchFamily` family nibble (product decision with the user, still open); and the `.ppt`
    `cdirFont` (Escher 137), still deliberately unread.
11. **Three page-reading leads on `010605Vul.ppt` page 9** with no metric behind them: our
    timeline's leftmost run is black where the reference's is red, our node markers are blue where
    the reference's are maroon, and our coat-of-arms picture is pixelated with an extra solid
    black bar along its lower edge.
