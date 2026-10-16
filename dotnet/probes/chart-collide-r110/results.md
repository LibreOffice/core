# O43 — the collision between two chart labels reserves the same five per cent the wrap does

Round 110, seat `agent/chartcollide`, based on `a311b00e2`. Reference **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`, tarball install), hyphenator switched off throughout by
r105's `de-DE` retag unless a row says otherwise. C++ read out of `/home/user/libreoffice-core`,
which declares `27.2.0.0.alpha0+` and is **not** the reference binary's source — every arm below
is stated as a source reading or as a measurement of 26.2.4.2, never as both.

## 0. What this round settles, in one table

| Question | Answer |
|---|---|
| What is the reference's label collision box made of? | **Not the shape's own bounds, not a padding in em, not a `SnapRect`.** It is the text, and what is added is a *separation*: the reference keeps `nReduce` — the five per cent `createTextShapes` takes off the wrap limit — clear between two adjacent labels. §3 |
| Fitted or derived? | Derived. The constant is the axis' own `(nScreenDistanceBetweenTicks*5)/100`, already in the tree as `WrapFraction`; nothing new was fitted. The measurement is **5.3 %** of the pitch and the source's number is 5.00 % — a residual of 0.3 % of a tick that is stated in §3.4 and left open in §7, not fitted away. |
| Does one mechanism explain both ladders? | **No, and the reason is that the two ladders measure two different things.** The one-word ladder measures the *wrap restart* (0.95 of the pitch, exactly, at five pitches); the two-word ladder measures the same restart on a *wrapped* label, where 26.2.4.2 restarts at **0.875** of the pitch instead of 0.95 and the deficit moves with the first word. The collision — the thing r108 was after — is visible in neither and needed a third experiment. §4 |
| Is the two-word boundary hyphenation? | **No.** Five pitches rendered with the labels tagged `en-US` and `de-DE` give identical arrangements, 5 of 5. §4.2 |
| Reach | §5 |
| What is left open | §7 |

## 1. The instrument

`ladder.py` rewrites `038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides` so that four
things vary independently and nothing else does:

* **the category label**, as a run of one repeated letter — no hyphenation point exists in one,
  which removes r106's confound, and no ligature or kern pair can enter the width, which removes
  one this round found the hard way (§1.2);
* **the number of categories**;
* **the category-axis font size**;
* **the chart frame's own width** (`p:graphicFrame/a:ext/@cx`, re-centred on the slide), which
  moves the tick pitch *continuously* where the category count moves it in steps.

`sweep.py` drives it; `confine.py` is the corpus half of §5. Every rendering is read by PyMuPDF as:
turned or upright (`dir` = (0.7071, −0.7071), or ≥ 6 glyph-sized filled paths per category, which
is the shear rule's outlined form), how many baselines the label's own words sit on, and — the
number the round turns on — **the tick pitch, measured as the centre-to-centre distance of one
label's own occurrences**. That is exact: a label is centred on its tick and every occurrence is
the same string, so every side bearing cancels.

### 1.1 The pitch is linear in the frame width, and the label does not resize with it

Ten frame widths from 1 800 000 to 6 000 000 EMU with a four-character label
(`w4` of the round's work directory):

```
cx 1800000 -> pitch 27.207    cx 3600000 -> 54.201    cx 6000000 -> pitch 90.703
```

`pitch = 1.511780e-5 × cx` to 4 significant figures at all ten, intercept −0.006 pt, and the
label's drawn ink is **10.204–10.206 pt at every one of them** — so no dynamic font resize is in
the way and the frame width is a clean pitch knob. Every pitch quoted below is either measured in
that rendering or computed from that line and cross-checked against a measured one.

### 1.2 Two instrument traps, both of which cost measurements

**A run containing `ff` is one glyph.** Carlito's `liga` fires and PyMuPDF's `Font.text_length`
does not apply it, so `nnnnnnffss` is ~0.5 pt narrower drawn than computed. Half of this round's
first ladders used `f` in their tails and their brackets are unreliable; every figure quoted in §3
and §4 comes from runs of **one** letter.

**Per-glyph pen positions read out of the PDF are quantised** and cannot be used as a metric —
`dotnet/CLAUDE.md`'s advance-divergence note in miniature. Consecutive `n` origins in the
reference's own PDF alternate 5.8836 / 5.9166 pt about a mean of 5.898, against a computed 5.9106;
`r` comes back 2.4 % short. The computed advance is the one to trust, and §3.3 bounds its error
independently.

### 1.3 Why `--convert-to fodp` is not the instrument for this question

It is for everything the *model* decides, and this round used it for nothing because the
arrangement is not in the model. `AxisLabelProperties::autoRotate45`
(`VAxisProperties.cxx`:403-408) writes 45 into the **view's** `AxisLabelProperties`; the model's
`TextRotation` is only ever read from (`:372`, the sole occurrence in `chart2/source/view/axes`)
and never written back. So a flat ODF export of a `.pptx` states the angle the *file* stated,
which is what makes a rendering the only channel here — and it is why every row of `rows.tsv` is
read out of a PDF.

**Confirmed against the binary, which is the cheap part.** `--convert-to fodp` of the `n×8` deck
(upright) and the `n×10` deck (turned at the same frame) writes **no `style:rotation-angle` at
all** in either, and the same two `text:line-break` statements in both — the two files are
indistinguishable on everything this round measures, while their PDFs differ by a whole
arrangement. Where the question is what the *model* resolved to, the flat export is still the
eight-second instrument; the arrangement is not in the model.

## 2. What the source says, and why it is not the answer

`doesOverlap` (`chart2/source/view/axes/VCartesianAxis.cxx`:186-207) builds a rectangle per label
from `xShape->getPosition()` and `xShape->getSize()`, rotates both by the label angle and
intersects them. `BaseGFXHelper::makeRectangle` is `(x, y, x+w, y+h)` and nothing else.

The shape is an `SvxShapeText` of kind `SdrObjKind::Text` built by the `createText` overload at
`ShapeFactory.cxx`:2042, which sets **no text distance** — r108's reading, re-checked here and
confirmed. Its width comes from `SdrTextObj::AdjustTextFrameWidthAndHeight`
(`svx/source/svdraw/svdotxat.cxx`:44-240): `nWdt = Outliner::CalcTextSize().Width() + 1`, clamped
to `GetMaxTextFrameWidth()` and floored at `GetMinTextFrameWidth()`, both in 1/100 mm. The
`+1` is a hundredth of a millimetre. `CalcTextWidth` is the widest line and adds one more unit
(`ImpEditEngine::CalcParaWidth`, `editeng/source/editeng/impedit2.cxx`:3560-3581).

Following the label through `ShapeFactory::makeTransformation` → `SdrTextObj::TRSetBaseGeometry`
→ `SetSnapRect(1×1)` → `AdaptTextMinSize` → `TextProperties::ItemSetChanged` →
`NbcAdjustTextFrameWidthAndHeight` leaves the box centred on the tick anchor at the text's own
width plus two hundredths of a millimetre. **On that reading two adjacent labels collide exactly
when their mean width exceeds the tick pitch**, which is what this tree implements.

**Three structural facts from the same read that the measurements below rest on**, and they are
worth keeping whatever the constant turns out to be:

1. **A 45° rotation can only follow the wrap restart.** `canAutoAdjustLabelPlacement` (`:539-556`)
   returns false while `m_bLineBreakAllowed` is true, and an OOXML category axis has it true
   (`axisconverter.cxx`:356-365). So on the first pass a collision can only raise the rhythm; the
   only route to 45° is `lcl_hasWordBreak` clearing the flag at `:888-905` and the labels then
   colliding as single lines. Every turned rendering in this round is one line per label, checked
   in the PDF text.
2. **After the restart there is no wrap limit at all.** `isBreakOfLabelsAllowed` returns false, so
   `nLimitedSpaceForText` stays −1, no `TextMaximumFrameWidth` and no `ParaIsHyphenation` are set,
   and the label is one line of its full width.
3. **The rotation is not automatic once the restart has fired** — the collision test is real, and
   §3.1 is the measurement that shows it.

## 3. The collision, isolated

### 3.1 The experiment: alternate a long label with a short one

With **equal** labels the wrap restart (0.95 of the pitch) and the collision threshold are within
half a per cent of one another and the wrap is always the binding one — which is why three rounds
of one-word ladders could not see the collision at all. Alternating a long label (past the wrap
limit, so the restart fires) with a short one puts the collision on its own: the restart still
fires, and whether the axis turns is then decided by the *pair*.

`038` at its own frame, 5 categories, pitch 54.193 pt, long label `n×10` = 59.106 pt:

| short | mean of the pair | arrangement |
|---|---|---|
| `ii` 10.327 | 34.72 | upright |
| `n×6` 35.464 | 47.29 | upright |
| `n×7` 41.375 | 50.24 | upright |
| `n×8` 47.285 | 53.20 | **turned** |
| `n×10` 59.106 | 59.11 | **turned** |

**So the rotation is not unconditional after the restart** — the same restarted axis is upright
with a short neighbour and turned with a wide one — and the collision is on the pair, exactly as
`doesOverlap` says.

### 3.2 Where the pair boundary is, at four pitches

Two forms of the sweep, both with runs of one letter only. *(a)* fix the pitch and sweep the short
label; *(b)* fix both labels and sweep the frame width. In *(b)* the long label is kept **narrower
than the pitch** so that its ends do not overflow the plot — a chart whose labels overflow is
drawn at a smaller plot and its pitch is not the calibration's (§3.5).

| pitch (pt) | long | short | boundary mean | reserve (pt) | reserve / pitch | runs |
|---|---|---|---|---|---|---|
| 54.193 | 53.196 | swept | 51.158 up, 51.369 turned | 2.82–3.04 | (0.0521, 0.0560] | tails |
| 71.847 turned / 72.036 up | 70.928 | 65.017 | 67.972 | 3.88–4.06 | (0.0539, 0.0564] | **pure** |
| 108.398 | 106.391 | swept | 102.739 up, 103.063 turned | 5.34–5.66 | (0.0492, 0.0522] | tails |
| 121.559 turned / 121.668 up | 118.213 | 112.302 | 115.258 | 6.30–6.41 | (0.0519, 0.0527] | **pure** |

`reserve = pitch − (w₁+w₂)/2` at the boundary. Two things come out of it and the second is a
correction to the first draft of this page:

* **A reserve that is a *constant* is refuted outright.** It is 2.9 pt at pitch 54 and 6.4 pt at
  pitch 122, a factor of 2.2 across a factor of 2.2 in pitch. Whatever it is, it goes with the
  spacing.
* **It is not a single clean fraction either.** The two rows whose labels are runs of one letter —
  the only two with no tail-advance question in them — give (0.0539, 0.0564] and (0.0519, 0.0527],
  which **do not intersect**. Folding in the advance model's own ±0.13 % (§3.3) widens them to
  (0.0529, 0.0577] and (0.0508, 0.0540], which intersect at **(0.0529, 0.0540]** — so a single
  fraction survives only at about **5.3 %**, and only once the metric's uncertainty is spent on it.
  Solving for a common advance *and* a common fraction over the four inequalities has **no
  solution at all**.

### 3.3 The instrument's own floor, measured rather than assumed

Everything above is a ratio between a computed Carlito advance and a measured pitch, so it inherits
whatever error the advance model carries. That error is bounded by the round's other measurement,
which has **no free parameter at all**: for *equal* one-word labels the axis turns exactly at the
wrap limit, and the wrap limit is `0.95 × pitch` by construction in the source. Five frame sweeps,
runs of `n` at 6, 8, 10, 12 and 14 letters (`w7`):

| run | last turned pitch | first upright pitch | implied advance of one `n` |
|---|---|---|---|
| `n×6` | 37.039 | 37.412 | (5.8645, 5.9236] |
| `n×8` | 49.643 | 49.970 | (5.8951, 5.9339] |
| `n×10` | 62.135 | 62.470 | (5.9028, 5.9347] |
| `n×12` | 74.305 | 74.744 | (5.8825, 5.9172] |
| `n×14` | 86.909 | 87.415 | (5.8974, 5.9317] |

Intersection **(5.9028, 5.9172]**, and the computed value is **5.9106** — inside it, and the model
is good to **+0.11 % / −0.13 %**. Read the other way, the same five rows say the one-word turn threshold is
`0.95 × pitch` to within [0.94895, 0.95125) over pitches from 37 to 87 pt, with no free parameter.

Folding that +0.11 % / −0.13 % into §3.2's two pure-run rows gives **(0.0529, 0.0577]** at pitch
72 and **(0.0508, 0.0540]** at pitch 121.6, which is the (0.0529, 0.0540] the section quotes.

### 3.4 What is implemented, and the 6 % that is not explained

`nReduce = (nScreenDistanceBetweenTicks * 5) / 100` — `VCartesianAxis.cxx`:753-759, under the
comment *"reduce space for a small amount to have a visible distance between the labels"* — is
0.0500 of the pitch and is the only constant of that size anywhere in the path. It is already in
this tree as `WrapFraction`, so **the implementation adds no new number**: the separation two
labels must keep is `spacing × (1 − WrapFraction)`, added to the sum of their half-widths, which
is the same arithmetic as widening each box by it and therefore turns with the labels.

**The measured window is (0.0529, 0.0540] and 0.0500 is below it, by about 6 % of itself and
0.3 % of the pitch.** That is stated rather than fitted away, and the choice is deliberate:

* 0.053 is a number fitted to one chart's ladders with no mechanism behind it, which is exactly
  what this seat exists to avoid — and the two clean configurations do not even agree on it
  without spending the metric's whole uncertainty.
* 0.050 is `nReduce`, it is already the constant this file uses for the wrap, and the two
  measurements that *do* have no free parameter — the five one-word frame sweeps of §3.3 — put the
  wrap limit at 0.95 to [0.94895, 0.95125), so the same 5 % is measured exactly where the source
  says it is and 6 % out where the source says nothing at all.
* The difference between the two is 0.3 % of a tick, and §5's 176 chart-bearing corpus renderings
  are byte-identical under either.

So what is implemented is the mechanism — *the collision keeps the wrap's own gap clear* — at the
source's constant, and §7 carries the 6 % as open.

### 3.5 One thing that reads as a squeeze and is not

A chart whose labels overflow its page is squeezed anisotropically by
`ViewContactOfSdrOle2Obj` — `dotnet/CLAUDE.md` records it — so a shrunken drawn pitch looks like
that fit. **Here it is not**: with the long label at 59.106 on a 54.199 pt pitch the drawn pitch is
52.875, and the drawn **ink** of the very same label is 58.858 pt, identical to its ink in an
unsqueezed rendering to three decimals. Nothing scaled; the *plot* is smaller. Read the label's own
ink before attributing a pitch change to the fit — this round's first reading of §3.2 was 0.934
because it used the calibration pitch on rows whose plot had shrunk.

## 4. The two-word ladder measures the wrap, not the collision

### 4.1 It is the restart, and it fires far below the wrap limit

The reference's own wrap limit is directly observable as the width at which a two-word label goes
from one drawn line to two. Prefix `ooo ` + a ladder run, 5 categories, pitch 54.193:

* one line to `ooo nnnnft` at **51.185 pt**, two lines from `ooo nnnnts` at **52.152 pt** —
  so the paper is in [51.185, 52.152), and `0.95 × 54.193 = 51.483` is inside it. (Both of those
  labels carry an `f` and so are subject to §1.2's ligature caveat; the bracket is 1.9 % wide and
  the caveat is worth ~0.5 pt, so it is a bracket on the paper and not a fourth decimal place.)

So the wrap limit is 0.95 of the pitch on the reference's own showing, and the source's reading of
what should then happen is unambiguous: a two-word label whose second word fits that paper breaks
at the blank, no line starts inside a word, `lcl_hasWordBreak` is false, and the axis is left
upright on two lines. **26.2.4.2 restarts much earlier.** Two frame sweeps with the same prefix and
a pure run as the second word (`w13`, `w14`):

| second word | last turned pitch | first upright pitch | second word / pitch |
|---|---|---|---|
| `n×8` = 47.285 | 54.046 | 54.515 | (0.8674, 0.8749] |
| `n×16` = 94.570 | 106.203 | 108.092 | (0.8749, 0.8905] |

**≈0.875 of the pitch, not 0.95** — a deficit of 0.0755 of the pitch. The two brackets *adjoin* at
0.8749 and are strictly disjoint by one step of the sweep, so a single fraction near 0.875 is
inside the resolution of both (the step is 1.6 % of the ratio, and §3.3's advance uncertainty is a
further ±0.13 %); it is not established more finely than that.

**And the deficit moves with the *first* word**, which no fraction of the pitch can do. At pitch
54.193, the boundary on the second word is

| first word | boundary | / pitch |
|---|---|---|
| `o ` (8.477 pt) | (47.878, 49.065] | (0.883, 0.905] |
| `ooo ` (20.342) | (47.390, 48.186] | (0.874, 0.889] |
| `oooooo ` (38.140) | ≤ 46.978 | ≤ 0.867 |

Only the upper bound of the third row is safe — its lower one is `nnnnnnfft`, which ligates (§1.2)
— but the three upper bounds alone are enough: 0.905, 0.889 and 0.867 cannot be one number.

### 4.2 It is not hyphenation, and that is measured on this binary

`/opt/libreoffice26.2/share/extensions` ships `dict-en`, `dict-es` and `dict-fr` and **no German**,
so the `de-DE` retag is a genuine no-hyphenator state. Rendering the same five frame widths with
the labels tagged `en-US` instead gives **the identical arrangement and the identical line count at
5 of 5** — `turned, turned, upright, upright, upright` both ways. Whatever moves the two-word
restart 7.5 % of a pitch below the wrap limit, it is not the hyphenator.

### 4.3 So the seat's premise needs correcting

r108 recorded the two-word boundary as *"somewhere else entirely … a boundary in (44.70, 47.06]
that no single threshold shares with the one-word ladder"* and asked for a mechanism that explains
both ladders. It is now clear that **the two ladders never measured the same thing**: both of them
measure the *wrap restart*, one on an unwrapped label (where it is exactly 0.95 of the pitch) and
one on a wrapped label (where it is ≈0.875 and depends on the first word). Neither of them can see
the collision, because for equal-width labels the collision threshold sits about 0.3 % of a tick
below the wrap threshold and is never reached first. That is why a third experiment was needed and why no single
threshold was ever going to fit the two.

*(A correction to r108's own numbers while they are being used: its two-word widths are not this
round's. `Scratched` is 45.181 pt through Carlito and chart2's 96 dpi pixel em, not 44.70;
`Screeched` 47.219, not 47.06. The seven r108 quotes are 0.02 %–1.1 % below the ones this round and
r91 compute, with no constant ratio between them, so they came from a third instrument.)*

## 5. Reach, measured on renderings

`../chart-fit/census.tsv`'s **176 chart-bearing corpus documents** — counted both ways, so the
seven `.xls` charts that live in a BIFF substream are in it — rendered twice by
`confine.py`: once with the CLI built at the round's base and once with it built from this
diff, `SOURCE_DATE_EPOCH` pinned, one output directory per document, keyed on the path. Each
rendering's sha256, page count and alphanumeric characters (`batch-check.sh`'s column 9 rule
verbatim) are banked and the PDF deleted; the container has four gigabytes free.

**0 renderings of the 176 move, and none failed on either leg.** All 176 sha256 digests are equal
before and after, so no page count, no character count and no gate verdict can have moved either.
`reach.tsv` has the row per document, with the banked 26.2.4.2 page and column-9 counts from
`/home/user/gate-2f47/rows.tsv` beside them.

**The base rate beside it**, scored from that bank with `batch-check.sh`'s own rule: of the 176,
**173 are page-exact against 26.2.4.2, 153 are inside the `max(2 %, 15)` character band, and 153
are both** — before and after, identically, because the bytes did not move. So the population this
change was scored on is one where four fifths already agree and the fifth that does not is not
failing on anything this touches.

That is a real reach figure and not a null result to bury: **the corpus contains no chart whose
labels land in the five per cent this changes.** The window is narrow by construction — a pair of
adjacent labels whose mean width is between 0.95 and 1.00 of the tick pitch *and* whose axis has
already restarted its wrap — and the three documents this row was opened for
(`055_Project_timeline`, `027_Simple_personal_cash_flow`, `064_Small_business_cash_flow`) are not in
it: they are the §4 residual, which is a wrap question.

**So the change is confirmed on the authored fixtures instead, and on r108's own banked ones rather
than on new ones.**

* `probes/chart-geom-r108/fine-width.tsv` — fifteen one-word labels from 51.702 to 54.267 pt on
  `038`'s 54.202 pt pitch, which 26.2.4.2 turns 15 of 15 and which r108 measured this tree turning
  **1 of 15**. Re-rendered through the CLI built from this diff: **15 of 15 turned**.
* `probes/chart-geom-r108/width-ladder-oneword.tsv` and `-twoword.tsv` — 102 renderings, three
  letters x 17 lengths x two label shapes. Agreement with 26.2.4.2 goes **98 → 99 of 102**, exactly
  **one** row moves (`n×9` alone, the one-word row r108 flagged) and **none regresses**. The three
  that still disagree are all two-word rows — `Cost i×12`, `Cost n×8`, `Cost W×5` — which is §4's
  residual and not this mechanism.

## 6. Tests

`dotnet/tests/XRay.Office.Core.Tests/ChartAxisCollisionReserveTests.cs`, six cases.

| case | what fails at the base, and in which direction |
|---|---|
| `APairIsCollidedByFiveHundredthsOfTheSpacingItDoesNotOccupy` | Alternating 98 pt and 93 pt on a 100 pt pitch is turned; 98 and 84 is not. Against the tick itself neither mean (95.5, 91) reaches 100, so the base leaves both upright. |
| `TheReserveFollowsTheTickSpacing` | The same pair × 2.5: 245 and 232 on a 250 pt pitch is turned, 245 and 210 is not. A *constant* five points of reserve leaves the first upright, so this is the case a constant cannot pass. |
| `AWordPastTheWrapLimitButInsideTheTickIsTurned` | `fine-width.tsv` in one assertion: one word of 96 pt on a 100 pt pitch. |
| `AWordInsideTheWrapLimitIsNotTurned` | The control: 94 pt never restarts the wrap, so line breaking stays on and rotation is refused whatever the boxes do. |
| `AVerticalAxisKeepsNoReserve` | The opposite direction — a reserve applied to a vertical axis, whose own wrap limit has no reduction in it, would thin one the reference leaves alone. |
| `TheReserveDoesNotReachTheBandTheAxisGivesUp` | The band is the label's *height*, and it must not move with the pitch. |

**And one existing case was refuted and is corrected rather than relaxed.**
`ChartAxisWrapLimitTests.AOneWordLabelThatBreaksButDoesNotCollideStaysUpright` asserted that a
96 pt label on a 100 pt pitch comes out upright — which is `doesOverlap` as this tree's chart2
reads and is not what 26.2.4.2 does (15 of 15 in `fine-width.tsv`, and the one-word threshold at
0.95 of the pitch over five pitches in §3.3). It is now
`AOneWordLabelPastTheWrapLimitTurnsInsideTheTick`, with the measurement in its remarks and the
94 pt control beside it. Two prose claims that said the same thing — in that class' own remarks
and in `ChartAxisLabels.Wraps` — are corrected with it.

**Totals, run one project at a time from this diff's own build, on a box at load average 70**
(`/tmp/r110-chartcollide-tests.log`; the first attempt was discarded because another session in
another worktree was appending to the same scratchpad log, which is how a WordProcessing result
for a tree that is not this one turned up between two of mine):

```
Containers 109   Core 566   Markup 259   OpenDocument 160   Presentations 1107
Rendering  164   Spreadsheets 1333   Text 728   Vector 309   WordProcessing 1938
                                                   6673 passed, 0 failed, 0 skipped
```

Every one of the ten is at or above the count round 108 recorded for it, so none is a truncated
run; Core is +6 (this round's six) and Presentations +6 and Spreadsheets +8 from rounds merged
since.

**`XRay.Office.Fidelity.Tests`, which needs an installed LibreOffice and is the one that can skip
silently: `Failed: 10, Passed: 542, Skipped: 0, Total: 552`** — 0 skipped, so it covered
everything. All ten failures are the families `dotnet/CLAUDE.md` records as left failing on
purpose, and none of them is a chart:

```
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop   x4  (docx/odt/fodt/doc)
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt x4 (paginated.*)
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt  x1
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt            x1
```

The first eight are the one family CLAUDE.md's advance-divergence note describes — a position
reconstructed N glyphs deep inside one reference text object, where the channel's own resolution
exceeds the tolerance — and the last two are its two named separate cases. `SlideChartFace`,
`SlideChart` and every other chart comparison passes, which is the check that matters here: they
are the fidelity tests this diff could reach, and §5's 176 byte-identical renderings say why they
did not move.

## 7. What is open, and why

* **The 0.3 % of pitch between the measured reserve (5.3 %) and the source's `nReduce` (5.0 %),
  and the fact that the two clean configurations do not share one fraction at all** until the
  advance model's ±0.13 % is spent on them. §3.2, §3.4. Two data points and two unknowns — the
  advance and the law — do not determine the law; a third pure-run pitch and a metric channel
  better than a computed Carlito advance are what it needs, and the PDF's own pen positions are
  not that channel (§1.2).
* **The two-word restart at 0.875 of the pitch.** §4. Measured at two pitches and three first-word
  widths, refuted as hyphenation, and unexplained. This is what `055_Project_timeline` and
  `027_Simple_personal_cash_flow` need — both are documents where 26.2.4.2 turns an axis this tree
  leaves upright — and it is a *wrap* question, not a collision one, so `ChartAxisLabels.Wraps` is
  where it will land rather than `Collides`.
* **The reserve on a vertical category axis, on a 45° axis, and on an axis that never had line
  breaking at all** (a value axis, or an ODF axis stating `text:line-break="false"`). None of the
  three was measured, and the second is the awkward one: the reserve is measured *after* the wrap
  restart, where `nLimitedSpaceForText` is −1 and no `nReduce` exists in that pass — so whatever
  carries it there is not the limit being recomputed, and "the axis has a wrap limit" is not the
  condition. It is applied on every horizontal axis here, which is the simplest extrapolation and
  costs nothing measurable (§5's 176 renderings are byte-identical). A vertical
  axis' wrap limit has no reduction in it at all (`VCartesianAxis.cxx`:768-773), so the
  implementation reserves nothing there; a rotated axis carries the reserve because the same
  arithmetic widens the box, and its consequence for the rhythm of an already-turned axis is
  untested — 9 of the 13 corpus documents that draw at 45° already matched the reference's turned
  count before this round and none of the 176 moves, so nothing there is disturbed either.
* **Whether the shape is genuinely wider than its text, or the separation is reduced.** The two are
  indistinguishable in every experiment here — `m·(w₁+w₂)/2 > pitch` and
  `(w₁+w₂)/2 + κ·pitch > pitch` are the same predicate — so `Depth`, which is the band the axis
  gives up, is deliberately left alone.

## 8. Files

* `results.md` — this.
* `ladder.py` — the variant writer and the PDF reader.
* `sweep.py` — the ladder and frame drivers.
* `confine.py` — the corpus reach sweep; renders, banks three numbers, deletes the PDF.
* `rows.tsv` — every rendering this round read, one row each.
* `reach.tsv` — the 176 chart-bearing documents, base against after.
