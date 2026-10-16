# O43 — the wrap limit is 0.95, and the two-word restart is not a wrap limit at all

Round 111, seat `agent/chartwrap`, based on `a362d41f9`. Reference **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`), labels retagged `de-DE` throughout so its hyphenator is
off (r105's recipe), Carlito. C++ read out of `/home/user/libreoffice-core`, which declares
`27.2.0.0.alpha0+` and is **not** the reference binary's source: every arm below is stated either
as a source reading of that tree or as a measurement of 26.2.4.2, never as both.

## 0. What this round settles

| Question | Answer |
|---|---|
| Is the wrap limit 0.95 of the tick pitch, or 0.875? | **0.95, measured directly.** Four composition transitions of a many-word label put the room 26.2.4.2 wraps in at [0.9472, 0.9507) of the pitch, with no free parameter beyond the advance and with no threshold model in between. §1 |
| Then what is the 0.875 round 110 measured? | **A different quantity.** On a two-word label the axis restarts while the label's widest word still fits that 0.95 room. It is a real threshold and r110's numbers reproduce, but it is not a fraction of the pitch: it moves with the width of the label's *other* words, and it moves with the *number of categories*. §2 |
| Is it hyphenation, a character count, or a word order? | No, no and no. §2.2–2.4 |
| Is it a plot squeeze — the labels measured unwrapped overflowing the plot and shrinking the pitch the decision is taken at? | **Refuted**, by the same four transitions of §1: their label is 110 pt unwrapped against a 52 pt pitch, far more overflow than any two-word case here, and their room is still exactly 0.95. §2.6 |
| What is implemented? | **Not the two-word restart** — every form that fits it needs three fitted terms and still misses the category count, and this seat exists to avoid exactly that. What is implemented is a different, source-stated rule this round found while chasing the three documents: an OOXML **value** axis keeps chart2's own `TextBreak`/`TextOverlap`/`ArrangeOrder` defaults, where this tree gave it the category axis' importer statement. §3 |
| Reach | **0 of 176 chart-bearing corpus renderings move**, byte-identical base and after with `SOURCE_DATE_EPOCH` pinned. §4 |
| Do the three documents this row was opened for move? | **No, and none of the three is the two-word wrap case the row says it is.** §5 — `055` is a date-axis *range and step* difference, `027` is a value-axis *major unit* plus a value-axis rotation neither of which reaches `ChartAxisLabels`, and `064`'s turned-label counts already agree with the reference. |

## 1. The wrap room, read straight off a rendering

### 1.1 The instrument

`paper.py` reuses `probes/chart-collide-r110/ladder.py` — the same deck
(`038_Competitive_Advantage_Card`), the same `de-DE` retag, the same continuous pitch knob (the
`p:graphicFrame` extent `cx`, re-centred on the slide) — and adds one idea:

> **A label made of several short words can never break inside a word, so `lcl_hasWordBreak` can
> never fire and the axis never restarts. The label is simply drawn wrapped, and *which words land
> on which line* is the room it was wrapped in.**

That removes the whole of r110's inference chain. Every earlier figure for the wrap limit came from
*when the axis turns*, which is a threshold on top of a threshold; this is the room itself.

The reader groups the drawn tokens by baseline and divides each baseline's count by the category
count, because every category carries the same label — so two adjacent labels merging into one
PyMuPDF span cannot corrupt the count. It reports the count as a float and flags any row where it
is not a whole number; no row in this section was flagged.

```sh
cd dotnet/probes/chart-wrap-r111
python3 paper.py <work> multi 5 11 zz nnn 5 3350000 3560000  8   # transition A
python3 paper.py <work> multi 5 11 zz nnn 5 4780000 4990000  8   # transition B
python3 paper.py <work> multi 5 11 zz nnn 5 3980000 4190000  8   # transition C
python3 paper.py <work> multi 8 11 zz nnn 5 5400000 5620000 12   # transition D, 8 categories
```

### 1.2 The four transitions

The label is `zz nnn nnn nnn nnn nnn` at 11 pt: `zz` 8.888 pt, `nnn` 17.732, the blank 2.543, all
on chart2's own 96 dpi pixel em (r108's model, which §1.3 of r110 bounds at ±0.13 %).

| transition | needs | last pitch below | first pitch above | room / pitch |
|---|---|---|---|---|
| line 1 takes a third token (`zz nnn nnn`) | 49.438 | 52.003 | 52.464 | [0.94232, 0.95067) |
| line 1 takes a fourth (`zz nnn nnn nnn`) | 69.713 | 73.178 | 73.632 | [0.94677, 0.95265) |
| line 2 takes a third word | 58.282 | 61.074 | 61.528 | [0.94724, 0.95428) |
| line 1 takes a third token, **8 categories** | 49.438 | 51.972 | 52.154 | [0.94793, 0.95124) |

**They intersect at [0.94724, 0.95067) and 0.95 is inside it.** `nReduce`, the `(spacing*5)/100`
that `createTextShapes` takes off the limit *"to have a visible distance between the labels"*
(`VCartesianAxis.cxx`:753-759), is the source's own account of exactly that number, and it is
already this tree's `WrapFraction`. The fourth row is the control that matters for §2: **the room
does not move with the category count.**

Rows are in `boundaries.tsv`, `kind = paper`.

## 2. The two-word restart, which is a different threshold

### 2.1 r110's number reproduces, and the pitch it is read against is sound

`ooo` + a run of eight `n` (47.285 pt), five categories, sweeping the frame: turned at cx
3,550,000 and upright at 3,600,000, and r110's finer grid puts the boundary in cx (3,575,000,
3,606,000]. Converted by r110 §1.1's calibration that is a pitch in (54.046, 54.122], so the
threshold on the widest word is **L/pitch in [0.8737, 0.8786)** — r110's 0.875.

**One instrument caveat that had to be checked and was**: the *drawn* pitch of a **turned**
rendering is not the pitch the decision was taken at. At cx 3,550,000 the drawn pitch is 47.950
where the calibration says 53.668 — the diagonal labels overhang the plot and squeeze it by 11 %.
Every figure in this section is therefore read against the calibrated pitch, and the calibration is
confirmed on this round's own **upright** rows at 54.420 against a predicted 54.424 (0.007 %). r110
did the same; its `rows.tsv` carries both columns.

### 2.2 It moves with the width of the label's other words

Same second word (47.285), same five categories, first word swept over eight widths:

| first word | width (pt) | last turned pitch | first upright pitch |
|---|---|---|---|
| `iiii` | 10.327 | 52.912 | 53.517 |
| `oo` | 11.865 | 53.215 | 53.517 |
| `ooo` | 17.798 | 53.819 | 54.122 |
| `iiiiiiii` | 20.654 | 54.122 | 54.726 |
| `oooo` | 23.730 | 54.424 | 54.726 |
| `ooooo` | 29.663 | 54.726 | 55.331 |
| `iiiiiiiiiiii` | 30.981 | 55.331 | 55.936 |
| `oooooo` | 35.596 | 55.634 | 55.936 |

The pairwise brackets intersect at a slope of **(0.0949, 0.1147)** points of pitch per point of
first word. A slope of zero — which is what "the widest word must fit 0.95 of the pitch" predicts —
is refuted by two and a half points of pitch across that span.

### 2.3 It is a width and not a character count

`iiii` is four characters and 10.327 pt; `oo` is two characters and 11.865 pt. They land on top of
each other, and 0.605 pt of pitch per *character* — the rate the `o` series alone would also
support, since every `o` is the same width — predicts 54.58 for `iiii` against a measured
(52.912, 53.517] and 59.4 for `iiiiiiiiiiii` against a measured (55.331, 55.936]. Refuted. The
`i` rows sit where their **width** puts them, interleaved with the `o` rows in width order.

### 2.4 It is blind to word order and to how the rest is split

* `ooo ooo` (two words, 38.139 pt of text) gives (55.936, 56.541]; `oooooo` (one word, 35.596)
  gives (55.634, 55.936]. Ordered by width, adjacent, and nowhere near `ooo`'s (53.819, 54.122] —
  so it is the **prefix's total width**, not the first word's.
* `ooo nnnnnnnn zz` — a word added *after* the long one — gives (54.726, 55.331] where `ooo
  nnnnnnnn` gives (53.819, 54.122]. A word after counts like a word before.
* `nnnnnnnn ooo`, the run first, gives (52.912, 54.424] on a coarser grid, which overlaps `ooo
  nnnnnnnn`'s bracket. No order effect was found.

### 2.5 It scales with the font, and it moves with the number of categories

| | ratio L/pitch at the boundary |
|---|---|
| `ooo nnnnnnnn`, 8 pt, 5 categories | [0.8721, 0.8794) |
| `ooo nnnnnnnn`, 11 pt, 5 categories | [0.8737, 0.8786) |
| `ooo nnnnnnnn`, 14 pt, 5 categories | [0.8704, 0.8746) |
| `ooo nnnnnnnn`, 11 pt, **8 categories** | **[0.8969, 0.9016)** |
| `oo nnnnn`, 11 pt, 5 categories | [0.8704, 0.8777) |
| `oo nnnnn`, 11 pt, **8 categories** | **[0.8912, 0.8989)** |

Three font sizes agree, so the threshold is a length that scales with the text. **The two category
counts do not**, and the two pairs are disjoint by 2.4 % and 2.1 % of the pitch respectively, at
two quite different pitches (54 and 33). The 8-category calibration is this round's own, fitted on
that sweep's upright rows and stable to five significant figures across them
(9.4494e-6 pt per EMU, exactly 5/8 of the 5-category line, i.e. the same plot width).

**No property of a label and a tick pitch can move with the number of categories.** That is the
single fact that stops this from being implementable as a rule over `(words, spacing)`.

### 2.6 What it is not

* **Not the wrap room.** §1, four transitions, 0.95 at five *and* eight categories.
* **Not hyphenation.** r110 §4.2 rendered five pitches `en-US` and `de-DE` to identical
  arrangements 5 of 5; and independently, a run of one letter has no hyphenation point for any
  dictionary to find, which is what the pure-run ladders are for.
* **Not a plot squeeze in a deciding pass.** The obvious mechanism with the right sign *and* the
  right category dependence is: `createMaximumLabels` measures the labels **unwrapped**, the
  outermost of those overhang the plot, the plot is narrowed, and the wrap is decided at that
  narrower pitch — the narrowing is `overhang/cats`, which is where a 1/cats term would come from.
  **§1's four transitions refute it.** Their label is 110.3 pt unwrapped against pitches of 52 to
  74, three to four times the overflow of any two-word case in §2.2, and their room comes out at
  0.95 of the *drawn* pitch on the nose. A squeeze that big would have shown there first.
* **Not the shape's own box.** Following `lcl_hasWordBreak` → `SvxTextEditSource::UpdateOutliner`
  → `SdrTextObj::TakeTextRect` (`svx/source/svdraw/svdotext.cxx`:644-716) in the 27.2 tree, the
  test re-lays the label out with `SetMaxAutoPaperSize(nAnkWdt)` — the object's own anchor
  rectangle, i.e. the box `AdjustTextFrameWidthAndHeight` grew to `CalcTextSize().Width() + 1`.
  For a wrapped two-word label that box is the *widest line* plus two hundredths of a millimetre,
  so the second line fits it by construction and no mid-word break can occur. **That reading
  predicts a two-word axis never restarts at all**, which is not what 26.2.4.2 does. Recorded as a
  reading that does not survive its own prediction, not as an explanation.

### 2.7 What a fit would have to carry, and why it is not carried

Over the eleven pure sweeps, the best-supported linear form is
`1.05·L + 0.10·R + c > 0.95·spacing` with `L` the widest word, `R` the sum of the others and
`c ∈ (−0.183, −0.070]` — three terms, of which only the leading `0.95` is the source's. It still
does not carry §2.5's category count: forcing that in needs a fourth term of about `0.32·pitch/n`,
which corresponds to nothing in `VCartesianAxis.cxx`. Round 110 declined to carry a fitted 5.3 %
in place of the source's 5.0 %; this is the same call at four times the price.

## 3. What is implemented, and where it came from

Chasing the three documents (§5) turned up a rule that **is** stated in the source and that this
tree had wrong, and it is in the same family — it decides whether an axis wraps at all.

`AxisConverter::convertFromModel` sets `TextOverlap`, `TextBreak` and `ArrangeOrder` inside
`switch (aScaleData.AxisType) { case CATEGORY: case SERIES: case DATE:` (`axisconverter.cxx`:
324-326) and inside the `else` of that case's `mnTypeId == C_TOKEN(dateAx)` test (`:349-368`). So:

| element | chart2's `AxisType` | reaches the three lines? |
|---|---|---|
| `c:catAx` | `CATEGORY` | yes |
| `c:serAx` | `SERIES` (`:316`) | yes |
| `c:dateAx` | `CATEGORY`/`DATE`, other branch | no |
| `c:valAx` | `REALNUMBER` or `PERCENT` (`:306`, `:311`) | **no — it never enters the case** |

What the ones that miss out keep is `Axis.cxx`:239-242 — `TEXT_BREAK` **false**, `TEXT_OVERLAP`
**false**, `ARRANGE_ORDER` **AUTO**. `DrawingChartPlot.AxisTextOf` handled the `c:dateAx` arm and
not the `c:valAx` one, so every OOXML value axis in this tree carried `LineBreakAllowed = true`.

That is not bookkeeping. `canAutoAdjustLabelPlacement` refuses **both** auto-rotation and
auto-staggering while `m_bLineBreakAllowed` is true (`VCartesianAxis.cxx`:539-556), so an axis
carrying it can only raise its rhythm when its labels collide — wrapping off is the only route to a
45° axis. And chart2 would refuse to wrap a value axis anyway: `isBreakOfLabelsAllowed` opens with
*"no break for value axis"*, `!m_bUseTextLabels` (`:522-524`), and `m_bUseTextLabels` is set only
for `AxisType::SERIES` and `CATEGORY` (`VAxisBase.cxx`:66-86) — which is also why a `c:dateAx`
never wraps, independently of its `TextBreak`.

**Done**: `AxisTextOf` now takes the importer's three statements only for a `c:catAx` and a
`c:serAx`, and gives a `c:dateAx`, a `c:valAx` and an absent axis chart2's model defaults. The
change adds no constant.

## 4. Reach, measured on renderings

`../chart-fit/census.tsv`'s **176 chart-bearing corpus documents**, rendered twice by
`../chart-collide-r110/confine.py` — once with the CLI built at `a362d41f9` and once from this
diff — with `SOURCE_DATE_EPOCH=1700000000` so the PDF's own timestamp is out of the digest. Each
rendering's sha256, page count and alphanumeric characters (`batch-check.sh`'s column 9 rule) are
banked and the PDF deleted.

```sh
export SOURCE_DATE_EPOCH=1700000000
tail -n +2 probes/chart-fit/census.tsv | cut -f1 > /tmp/chartdocs.txt
for leg in base after; do
  python3 probes/chart-collide-r110/confine.py --out /tmp/conf-$leg \
      --cli /tmp/cli-$leg/XRay.Office.Cli --rows /tmp/rows-$leg.tsv --jobs 3 < /tmp/chartdocs.txt
done
```

**0 of 176 move**, and none failed on either leg: all 176 sha256 digests are equal, so no page
count, no character count and no gate verdict can have moved either. `reach.tsv` has the row per
document.

**Pinning the timestamp is not optional here, and the first run of this sweep is the reason it is
said out loud.** Without `SOURCE_DATE_EPOCH` **176 of 176** digests differ while every page count
and every character count is identical — an unpinned run reports a total reach and means nothing.

**Why the reach is zero, stated rather than buried**: value-axis labels in this tree do not go
through `ChartAxisLabels.Resolve` at all. `ChartLayout.AddValueAxis` and `AddDomainAxis` draw one
label per tick and read only the axis' *stated* rotation; only `ArrangeCategories` resolves an
arrangement, and it is reached for the category or date axis. So the corrected flags are read by
nothing that draws today. They are still worth carrying: they are what `Resolve` will be handed the
day a value axis is wired to it, and getting them wrong is what makes an axis thin where 26.2.4.2
turns (§5.2). They are held by six unit tests, three of which fail at the base.

## 5. The three documents in the O43 row, opened and read

Rendered with the base CLI and with 26.2.4.2 and compared line by line through PyMuPDF
(`dir` separates a 45° line from an upright one).

### 5.1 `055_Project_timeline_with_milestones` — a date-axis range and step, not a wrap

Its one visible axis is a `c:dateAx` with `majorUnit=10 days`, no stated min or max. This tree
draws **14** labels at 10-day steps, `5 Apr` to mid-August, all upright. 26.2.4.2 draws **38**
labels at 30-day steps — every third tick, so its rhythm is 3 — from `5 Apr` to `19 Apr` of three
years later, all turned 45°. At its pitch of 14.7 pt a `5 Apr` of about 14 pt collides outright, so
the turn follows from the scale and not from any wrap rule: at this tree's own pitch of 38.0 pt the
same label is 0.37 of a tick and nothing in `ChartAxisLabels` could turn it. **The gate shortfall
this row quotes — 960 characters against the reference's 1045 — is that axis' missing labels**, and
it is a date-axis scaling question. Its verdict does not move here.

### 5.2 `027_Simple_personal_cash_flow_statement` — a value axis, twice over

Page 6 is the only page that differs: the savings chart runs its money axis along the bottom, and
26.2.4.2 draws **eight** labels there, `$0` to `$14,000` in steps of 2,000, **turned 45°**. This
tree draws **four**, `$0` to `$15,000` in steps of 5,000, upright. So there are two differences and
neither is a wrap: the axis' automatic **major unit**, and the 45° the reference reaches once its
eight labels collide. The second is what §3 unblocks in the model and what `AddValueAxis` would
have to be wired to `ChartAxisLabels.Resolve` to reach; the first is upstream of it. Not this seat,
and both are worth their own row.

### 5.3 `064_Small_business_cash_flow` — already agrees on turns

Turned-line counts per page are identical to the reference: 6 on page 2 and 7 on page 4, 0
elsewhere on both sides. Whatever is left on this document is not an axis arrangement.

## 6. Tests

`dotnet/tests/XRay.Office.Presentations.Tests/DrawingChartAxisTextTests.cs`, six cases read from
markup literals.

| case | what it holds |
|---|---|
| `ACategoryAxisTakesTheImportersOwnOverlapWrapAndArrangement` | the arm that must not move: `c:catAx` with `rot="0"` keeps overlap, wrap and SideBySide |
| `AValueAxisKeepsChart2sDefaultsAndSoMayStillTurn` | **fails at the base** — `c:valAx` with `rot="0"` gave overlap and wrap |
| `AScatterDomainAxisIsAValueAxisAndKeepsThemToo` | **fails at the base** — and this is the one that reaches `Resolve`, since a scatter's domain axis becomes `CategoryAxisText` |
| `ASeriesAxisIsStatedLikeACategoryAxis` | the arm a "not a category axis" rule would get wrong: `SERIES` is inside the case |
| `ADateAxisKeepsChart2sDefaults` | the control that was already right |
| `AnAbsentAxisKeepsChart2sDefaults` | **fails at the base** — no element to take a statement from |

Confirmed against the base by swapping the file back for one run: **3 failed, 3 passed**, and the
three that pass are the three controls.

**Totals, one project at a time, from this diff's own build**
(`/home/user/r111-chartwrap/tests/*.txt`):

```
Containers 109   Core 573   Markup 259   OpenDocument 160   Presentations 1120
Rendering  164   Spreadsheets 1346   Text 728   Vector 309   WordProcessing 1938
                                                   6706 passed, 0 failed, 0 skipped
```

Every one of the ten is at or above the count round 110 recorded, so none is a truncated run;
Presentations is +13 of which 6 are this round's, and Core +7 and Spreadsheets +13 come from rounds
merged since.

**`XRay.Office.Fidelity.Tests`, the one that can skip silently: `Failed: 10, Passed: 542,
Skipped: 0, Total: 552`** — 0 skipped, so it covered everything, and the ten are exactly round
110's, none of them a chart:

```
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop    x4
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt x4
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt x1
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt x1
```

## 7. What is open, and why

* **The two-word restart itself.** §2. Measured over 20 continuous sweeps, three font sizes, two
  category counts and eight prefix widths; refuted as hyphenation, as a character count, as an
  order effect, as the wrap room and as a plot squeeze; and not implemented, because the only forms
  that fit carry three fitted terms and still miss the category count. What it needs is the
  quantity that moves with `1/cats`: the next thing to try is instrumenting the *sequence* of
  geometries chart2 lays an axis out at, which a rendering cannot see and a debug build of 26.2
  could — and this container may not build one.
* **`isBreakOfLabelsAllowed`'s first line, `m_aTextLabels.getLength() > 100`.** Not modelled: an
  axis with more than a hundred categories neither wraps nor rotates in chart2, and only thins.
  Scanned over the 176: the only charts in the corpus with more than a hundred categories are
  `171128IPAP.pptx`'s `chart5` and `chart6`, and both carry `<c:delete val="1"/>` on their category
  axis, so nothing is drawn and the rule cannot be confirmed against the binary on this corpus.
  Left unmodelled rather than added untested.
* **`m_bUseTextLabels` in `Resolve` itself.** §3 fixes the OOXML route, where a value axis' own
  `TextBreak` is false anyway, so the two rules coincide. They do not coincide for ODF: an
  `<chart:axis>` of value kind stating `text:line-break="true"` would be wrapped by `Resolve` and
  refused by chart2. No such document is in the corpus and nothing was measured.
* **The 0.3 % between r110's measured collision reserve (5.3 %) and the source's `nReduce`
  (5.0 %).** Untouched by this round and still open in r110 §7.
* **`055`'s date-axis range and step, and `027`'s value-axis major unit and 45°.** §5. Both are
  real, both are measured here against the reference, and neither is a wrap question.

## 8. Files

* `results.md` — this.
* `paper.py` — the instrument: writes the deck variants, renders them with 26.2.4.2, and reads the
  arrangement, the pitch and the words-per-line out of the PDF. `multi`, `pair`, `label` and
  `shape` modes.
* `boundaries.tsv` — every sweep this round read, one row per configuration, as the bracket the
  boundary falls in.
* `reach.tsv` — the 176 chart-bearing documents, base against after.
