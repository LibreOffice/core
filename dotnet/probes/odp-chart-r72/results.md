# An ODF axis' index, a shape that turns its own text, and why the reference draws some labels with no words in them

## Environment

    ours   = XRay.Office.Cli @ 97628b7fe (base) and @ 02a57bd85 (base + this round's three commits)
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball)
    fonts  = system fontconfig; all four tarball confounds aside -- the Carlito / Caladea /
             Liberation / DejaVu duplicates, the Latin Noto, and the four LiberationSansNarrow.
             `fc-match "DejaVu Sans"` answers DejaVuSans.ttf; `fc-list | grep -i narrow` is empty.
    rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
             max(2%, 15), then unembedded fonts.
    corpus = /home/user/corpus-odf (the .odp column, 302 documents) and /home/user/sample-files
             (the original slides track, 302 documents)
    date   = 2026-09-07

The reference half is **reused unchanged** from `probes/odp-master-r70/ref.tsv`, which that round
rendered fresh and which reproduced `probes/odf-gate-01/rows.tsv` on 302 of 302 rows. Nothing in
this round's diff can reach `soffice`. Our half was re-rendered at the base commit first and
scored **285 of 302**, reproducing the previous round's figure exactly, which is what makes the
rest of the numbers comparable.

## The result

| | match | of | |
|---|---:|---:|---|
| `.odp`, at the base commit `97628b7fe` | 285 | 302 | 94.4% |
| after the axis resolution and `draw:text-rotate-angle` | 289 | 302 | 95.7% |
| after `text:line-break` | **289** | 302 | **95.7%** |
| the original slides track, at the base commit | — | 302 | |
| the original slides track, after | — | 302 | **0 rows moved by this diff** |

**Four rows moved and every one of them the right way.** No verdict moves back, and the eight
rows whose numbers change without moving are seven improvements and one that is discussed in §2.4
and is not a regression.

| | at base | after |
|---|---|---|
| `combo_bar_line_chart.odp` | 166/219 | **219/219** |
| `scatter_chart.odp` | 83/105 | **105/105** |
| `schematicplaymar21.odp` | 11122/11470 | **11470/11470** |
| `schematicplay.odp` | 11142/11490 | **11490/11490** |

**The original slides track does not move at all.** Rendered twice — once with a binary built at
`97628b7fe` and once at `02a57bd85`, `obj`/`bin` removed on both legs so MSBuild could not skip a
project — **0 of 302 rows differ in pages, glyphs, unembedded fonts or status**. That is stronger
than the bank comparison, which shows one movement (`N2_E_Maestroni_Swarm_COP.pptx`, `words` →
`match`, 29088 → 28178 glyphs) that belongs to the chart-fit and axis-crossing commits between
`/home/user/gate-2f47`'s `2f4709c08` and this round's base, not to this diff. The static argument
agrees: `git grep` finds no reference to `OdfChartPlot`, `OdfChartStyles` or `OdpSlideLayout` from
any OOXML or binary reader, only from `OdsDrawings` and `OdfPictures`, which are ODF.

---

## 1. The ODF chart reader — the brief's three rows

### 1.1 An axis' index is its position among its own dimension, and this took the last one

ODF states an axis' index nowhere. `SchXMLAxisContext` counts how many axes of the same
`chart:dimension` have already been read and takes the count as this axis' index
(`xmloff/source/chart/SchXMLAxisContext.cxx`:266-274):

```cpp
// check for number of axes with same dimension
m_aCurrentAxis.nAxisIndex = 0;
for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
    if( m_rAxes[ nCurrent ].eDimension == m_aCurrentAxis.eDimension )
        m_aCurrentAxis.nAxisIndex++;
```

So document order is the whole rule and both the category axis and the value axis want the
**first** of their dimension. `OdfChartPlot` defaulted the value axis (`valueAxis ??= axis`) and
*assigned* the category one (`categoryAxis = axis`) — which is the same shape of bug the BIFF
round found in `XlsChartBuilder`, where one `_valueScale` field was overwritten by the second
`CHAXESSET`, and it bites for the same reason: **the file writes the primary first.**

LibreOffice writes a combination chart's `secondary-x` after the primary and gives it
`chart:visible="false"`, so taking the last x axis read the category labels off an axis that is
not drawn. `combo_bar_line_chart.odp` drew none of `Q1`…`Q4`.

### 1.2 `chart:attached-axis` is the only link ODF states

A series names a **y** axis by its `chart:name`, and it is measured against the secondary one
exactly when that axis' index is above zero (`SchXMLSeries2Context.cxx`:333-345 and :388-394):

```cpp
case XML_ELEMENT(CHART, XML_ATTACHED_AXIS):
    for( nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
        if( aValue == mrAxes[nCurrent].aName && mrAxes[nCurrent].eDimension == SCH_XML_AXIS_Y )
            mpAttachedAxis = &( mrAxes[ nCurrent ] );
...
if( mpAttachedAxis && mpAttachedAxis->nAxisIndex > 0 )
    mnAttachedAxis = 2;    // secondary axis => property has to be set (primary is default)
```

`ChartSeries.AxisIndex` and `ChartPlot.SecondaryValueScale` and its five siblings now come from
that. The name is the only link, so a file whose axes carry no `chart:name` puts every series on
the primary — which is what LibreOffice's own importer does with it too, and is asserted.

Reading it fixed the primary scale as well as adding the secondary: `combo_bar_line_chart.odp`'s
one axis read 0…50000, the line series' range, where 26.2.4.2 draws a primary of 0…45000 for the
bars and a secondary of 0…50000 for the line. All three are now the reference's, exactly.

### 1.3 A scatter chart's x axis is a value axis, and its abscissae are `chart:domain`

Both dimensions of a scatter chart are numeric, so ODF spells the x axis exactly as a category
axis is spelt and the chart's own `chart:class` is what tells them apart. Two things were missing
and each on its own is enough to draw no x labels:

- `ChartPlot.DomainScale` and `DomainFormat` were never set, so `ChartLayout.AddDomainAxis` had
  no scale;
- `chart:domain`'s cell range was not read, so `ChartLayout.DomainScaleOf` found no numbers at
  all — it reduces over `ChartSeries.XValues` and returns null when there are none.

### 1.4 A `chart:data-point` may state its label's words

`SchXMLSeries2Context.cxx`:1245-1290 turns a `chart:data-label`'s paragraphs into
`CustomLabelFields` of type `TEXT` and sets `DataCaption` to `CUSTOM`, so the stated string is the
whole label rather than a field of it — which is what `ChartDataLabel.Text` means. It is the only
way ODF puts a point's own words on a chart, and `scatter_chart.odp`'s `A`…`D` are written that
way and no other.

**It is worth more than the one row it was found for.** On
`8_P-Pavese_AIRBUS-ATB-journee-CRATB.odp` page 16 we drew the bare values `938`, `878`, `728`…
and 26.2.4.2 draws `938 (23%)`, `878 (21%)`, `728 (18%)`… — the point labels' own strings. Reading
them puts thirty characters of the reference's own text on that page.

### 1.5 What the three rows cost and what they bought

| | before | after |
|---|---|---|
| `combo_bar_line_chart.odp` — the reference's ticks | one axis 0…50000, no `Q1`…`Q4` | 0…45000 and 0…50000, `Q1`…`Q4`, identical to 26.2.4.2 |
| `scatter_chart.odp` | no x labels, no point labels | `2`…`10` and `A`…`D`, identical |
| `.odp` gate | 285 | **289** |

---

## 2. `draw:text-rotate-angle` — the brief's "`draw:auto-grow-height` overflows off the page"

### 2.1 The brief's attribute is not the mechanism

The previous round's remainder filed the two `schematicplay` twins as
*`draw:auto-grow-height="true"` overflows off the page bottom*, on a per-page count. Read at the
page rather than at the count, the body is not overflowing downwards at all: **it is drawn on its
side**, in fifteen-point-wide columns running off the top and bottom of the sheet.

`draw:auto-grow-height` is genuinely read nowhere in `dotnet/src` — the census is 9711 `"true"` in
246 documents and 26545 `"false"` in all 302 — but neither of these two documents' bodies is one
of them: `gr17` states `draw:auto-grow-height="false"`. The attribute they turn on is
`draw:text-rotate-angle="90"`, censused at **38 statements in 7 of the 302** and read nowhere
either.

**So `auto-grow-height` needed no machinery and no attribute: it was not the defect.** Whether it
is a defect anywhere else is untested here, and it is left as a census with no witness.

### 2.2 The mechanism

`draw:text-rotate-angle` on the `draw:enhanced-geometry` is the text's own rotation *inside* the
shape, and it composes with the shape's own. `xmloff/source/draw/ximpcustomshape.cxx`:917 puts it
in the geometry item as `TextRotateAngle`; `SdrObjCustomShape::GetExtraTextRotation`
(`svx/source/svdraw/svdoashp.cxx`:486-514) answers it; and
`ViewContactOfSdrObjCustomShape::createViewIndependentPrimitive2DSequence`
(`svx/source/sdr/contact/viewcontactofsdrobjcustomshape.cxx`:171-191) rotates the text box by
`360 − fExtraTextRotation` about the text rectangle's centre before the shape's own matrix is
applied.

It is what a converted deck states instead of turning the text itself: LibreOffice writes a wide,
short text box that arrived as a rotated PowerPoint shape as a **tall narrow shape turned a
quarter turn with its text turned the other way**. `schematicplaymar21.odp`'s body is
`svg:width="2.469cm" svg:height="29.821cm"` with
`draw:transform="rotate (-1.5707963267949) translate (33.099cm 3.3cm)"`.

### 2.3 The half that cost a measurement: the padding does the swapping

The obvious reading is that a quarter turn swaps the layout box's two dimensions. **It does not,
and implementing it that way wraps the text at twice the room it has and centres the overflow off
both edges of the page.**

LibreOffice writes a turned shape's insets in the text's *own* orientation. `gr17` carries

```
fo:padding-top="13.701cm"  fo:padding-bottom="13.822cm"
fo:padding-left="-13.52cm" fo:padding-right="-13.651cm"
```

on a shape 2.469 cm wide and 29.821 cm tall — a **negative** left and right padding that widens
the text area past the shape and a tall positive top and bottom that narrows it.
`SdrTextObj::AdjustRectToTextDistance` (`svx/source/svdraw/svdotext.cxx`:577-618, called from
`SdrObjCustomShape::TakeTextAnchorRect` at `svdoashp.cxx`:2628-2643) adds them to the anchor
rectangle and `TakeTextRect` takes its width as `nMaxAutoPaperWidth`. So the wrapping limit is
63.15 pt plus 383.3 plus 387.0 = **833.45 pt**, and 26.2.4.2's own lines on that page span
99.07 … 932.05, which is 832.98.

The check that separates the two readings is in the suite: `odp-text-rotate.fodp` states a turned
shape and an upright one carrying the same paragraph, and 26.2.4.2 draws them **identically** —
first line x 92.10…491.02 against 92.13…491.04, both at y 97.74. Swapping the box breaks the
turned one and leaves the control passing.

### 2.4 Reach and cost

38 statements in 7 documents: 90 degrees in three decks (`schematicplaymar21`, `schematicplay`,
`Course Selection 2025-26 Current Grade 09`) and ±180 in four more, plus one `.ods`. The gate
moved two rows; none of the ±180 documents moved a glyph or a page.

---

## 3. `text:line-break`, and what the third chart row actually is

### 3.1 The brief's third row is not a radar chart and the reference does draw those labels

The brief describes `038_Competitive_Advantage_Card…odp` as *a radar chart's rotated category
labels drawn where the reference draws none*. Three things in that sentence are wrong and the
third is the interesting one.

- It is `chart:class="chart:bar"` in both of its two embedded charts. There is no radar chart.
- Our arrangement is **right**: the reference turns those labels 45 degrees exactly as we do.
- The reference draws them as **filled Bézier outlines**, so `pdftotext` reads nothing there and
  the gate counts them as ours alone.

Measured on page 1: the reference's glyph-sized `#595959` fills cluster at 419.24…473.27 (14 of
them, being `ProductQuality`) and 483.05…665.18 (50), against our five turned text lines at
417.62…664.51. The whole span agrees to **1.6 pt over 246**, and the arrangement, the rhythm and
the angle are the same.

**So `canAutoAdjustLabelPlacement` answers *yes* here and it is right to.** The axis is a bar
chart's horizontal category axis with horizontal text, so its last three lines
(`chart2/source/view/axes/VCartesianAxis.cxx`:549-556) return `!m_bStackCharacters`, which is
true. What the brief expected — that a radar axis is neither horizontal nor vertical and would
therefore fall through to `return false` — does not arise, because there is no radar axis on this
document. (`VPolarAxis` does not go through `VCartesianAxis` at all; nothing in this round
exercised one.)

### 3.2 Why the reference outlines them, established

`variants.py` builds thirteen one-attribute variants of the file and renders each through
26.2.4.2. The full table is in its docstring; the three rows that settle it are:

| variant | textlines | glyph fills | legend width, size |
|---|---:|---:|---|
| `base` (11 pt, five long names, auto 45°) | 2 | **66** | 59.37, **13.94** |
| `oc45` (one name shortened, `style:rotation-angle="45"`) | 2 | **66** | 59.37, **13.94** |
| `short45` (names `A`…`E`, `style:rotation-angle="45"`) | **7** | 3 | 59.35, **14.00** |

The two legend keys are always in `textlines`, so 2 means no label was drawn as text at all.

**It is not the angle.** `short45` is the same 45 degrees in the same file and its labels stay
text. `oc15` and `oc30` stay text too, and `oc90` and `oc270` stay text — while `oc45` and `oc60`
are outlined.

**It is a shear, and the legend is the witness.** The unrotated legend key measures 14.00 pt in
every variant whose labels fit inside the chart's page and 13.94 to 13.87 in every variant whose
labels overflow it, at an unchanged width of 59.35 to 59.38. That is the anisotropic fit
`dotnet/CLAUDE.md` already records: an embedded chart is scaled to its own *drawn* extent by two
factors, one per axis (`ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters`,
`svx/source/sdr/contact/viewcontactofsdrole2obj.cxx`:88-116), so overflowing labels squeeze the
whole chart — here 1.00034 across against 0.99571 down, a 0.46% anisotropy.

An anisotropic scale composed with a rotation that is **not a right angle** decomposes to a shear,
and `VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`
(`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141) accepts a text primitive only when
there is none:

```cpp
// Acceptance is restricted to no shearing and positive scaling in X and Y (no font mirroring
// for VCL)
aLocalTransform.decompose(aFontScaling, aTranslate, fRotate, fShearX);
// tdf#95581: Assume tiny shears are rounding artefacts ... especially if the effect is less
// than a pixel.
if (std::abs(aFontScaling.getY() * fShearX) < 1)
```

Everything the guard rejects falls through to the primitive's own decomposition, which for text is
filled polygons. That accounts for all thirteen rows: no anisotropy at 15°, 30° and on the short
labels (no shear); anisotropy but a right angle at 90° and 270° (no shear); anisotropy and 45° or
60° (a shear of about 1.8 units against the guard's 1).

**This is the rule `dotnet/TODO.raster-ceiling.md` asked for and did not have.** That file records
the outlining ceiling on four documents and says outright that *"what is still unestablished is
the rule — which rotated text LibreOffice outlines and which it emits as per-glyph shows"*. The
rule is: a text run that is both **turned off a right angle** and inside a chart that had to be
**fitted anisotropically**. Both halves are necessary; neither is sufficient.

### 3.3 `text:line-break` is real, and this file said ODF had no attribute for it

`OdfChartPlot.AxisTextOf` carried *"Line breaking has no ODF attribute at all"* and passed
`LineBreakAllowed: false` for every axis. It is `text:line-break`, on the same
`style:chart-properties` as its `chart:` neighbours and mapped as `PROP_TextBreak` under
`XML_NAMESPACE_TEXT` (`xmloff/source/chart/PropertyMaps.cxx`:188). **The fourth instance of the
namespace trap** this project already records for `drawooo:display`, `loext:shadow-blur` and
`chartext:coordinate-region` — and the first where the wrong guess was that the attribute did not
exist.

It is not a detail of spacing. `canAutoAdjustLabelPlacement` refuses outright while line breaking
is on (`VCartesianAxis.cxx`:544-545), so an axis whose labels collide **wraps** them rather than
turning them; read as false, every crowded ODF category axis reached the rotation instead.

Census over the 302 `.odp`: **379 statements in 92 documents**, 160 `true` in 60 and 219 `false`
in 65.

**Measured: one row's glyph count moves and no verdict does.**
`N2_E_Maestroni_Swarm_COP.odp` goes from 340 alphanumeric characters clear of the reference to
**84** — the Gantt page whose category wrap `dotnet/CLAUDE.md` records as the one thing the chart
fit could not confirm. The visible half is larger than the countable one, because turning a label
changes no character: on `8_P-Pavese…odp` page 16 the nine hour-range labels
(`[08 h ; 10 h[` …) go from turned to upright and wrapped, and land within **0.94 pt** of
26.2.4.2's.

---

## 4. What is left, classified

`remainder.tsv` carries one row per non-matching document. Thirteen rows against the seventeen
this round inherited, and three of them are reclassified rather than merely restated.

| cause | rows |
|---|---:|
| raster ceiling — the reference draws a picture where we draw the object's text | 3 |
| **outlining ceiling** — the reference draws turned labels as polygons | 2 (+1 sharing a row) |
| something else in the ODF reader, no shared cause | 4 |
| a 180-degree `draw:transform` applied to the shape's text | 1 |
| autofit landing one `constScaleLevels` row from 26.2.4.2 | 1 |
| `CFF ` embedding | 1 |

**Three reclassifications, all of them out of "ODF chart reader" or "unclassified":**

- `038_Competitive_Advantage_Card…odp` — **not** the chart reader. The outlining ceiling; ours is
  the better output. §3.1.
- `Demick_JetBlue.odp` — **not** unclassified. Its pages 4, 5 and 7 carry 170, 210 and 156
  glyph-sized fills on the reference against 23, 27 and 23 turned text lines of ours, which is the
  same three pages `TODO.raster-ceiling.md` already records for the `.pptx` twin. Pages 6 and 8
  are a −90 deficit of ours, and the two cancelling is why the row reads +88 rather than +178.
- `Statement of Work presentation.odp` — **not** unclassified. Its page 3 holds three
  `draw:custom-shape` with `draw:transform="rotate (-3.14159265358979)"`; we draw their text at
  `dir (−1, 0)` and 26.2.4.2 draws it upright. 375 such rotations in 45 of the 302, nearly all on
  shapes carrying no text, which is why only this one shows.

## 5. Verification

- `dotnet build XRay.Office.slnx -v q -nologo` → **0 warnings, 0 errors**.
- Ten non-fidelity projects, run individually and totalled by hand: **6046 passed, 0 failed,
  0 skipped** — the 6033 baseline plus this round's 13 new tests (7 axes, 3 text rotation, 3 line
  breaking). No failure appeared on any project, so nothing needed re-running.
- `XRay.Office.Fidelity.Tests`: **542 passed, 10 failed, 0 skipped** of 552 — the same ten names as
  the baseline: `PageDrawingComparisonTests` ×4, `TabStopComparisonTests` ×4,
  `SheetDrawingComparisonTests`, `JustificationShrinkComparisonTests`.
- Both new rules are pinned by tests that fail without them, checked by mutating the source,
  `touch`ing it and removing `obj`/`bin` so MSBuild could not skip the project: making
  `draw:text-rotate-angle` unreadable fails 2 of 3, and reinstating the width/height swap fails a
  different 2 of 3. The source was then restored with `cp` and `touch`, `diff`ed against the copy,
  and the rebuilt binary re-rendered `combo_bar_line_chart.odp` at **219** characters, byte for
  byte the sweep's own figure.
- The `.odp` half re-scored at the base commit as well as at the head, both with our half
  re-rendered and the reference reused: **285 → 289 of 302**.
- The original slides track rendered at both commits: **0 of 302 rows differ**.
