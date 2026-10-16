# A crowded category axis turns because 26.2.4.2 **hyphenates** the label, not because a word is too wide

Round 91, measured 2026-09-10 against `/opt/libreoffice26.2/program/soffice`
(**LibreOffice 26.2.4.2**, tarball, all five bundled-font confounds aside; DejaVu present,
`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`). Witness
`slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx`,
seated by `probes/chart-axisrot-r90/findings.md`.

**Nothing in `dotnet/src` changed in this round.** The mechanism is identified, cited and
measured; implementing it needs a hyphenator, which is a feature and not a round. See
*What a fix would cost* at the end.

---

## 0. What the round was sent after, and what is wrong with the brief

r90's finding is right about the *picture* — 26.2.4.2 turns `038`'s five category labels 45°
onto one line each and this tree wraps them onto two upright lines (`pair-038-axis.png`,
mine, at 300 dpi, reproducing r90's crop) — and wrong about the cause it names.

> *"`m_bLineBreakAllowed` … is read from the axis's `TextBreak` property … This tree wraps
> unconditionally, which is the behaviour of an axis whose `TextBreak` is permanently true."*
> — and, as the load-bearing question, *"Where `TextBreak` comes from … this document's is
> evidently false."*

**`TextBreak` on this axis is `true`, in the reference as well as here, and it is not
"permanently true" in this tree either.**

- `AxisConverter::convertFromModel` (`oox/source/drawingml/chart/axisconverter.cxx`:356-365)
  sets `PROP_TextBreak` **`true`** for every non-date category axis, and only turns it off
  for a *stated* non-zero rotation:
  `bool bTextBreak = true; if (getProperty(fRotationAngle, PROP_TextRotation) && fRotationAngle != 0.0) bTextBreak = !bUseFixedInnerSize && (fRotationAngle == 90.0 || fRotationAngle == 270.0);`
- `038`'s `c:catAx` states `<a:bodyPr rot="-60000000">`, which is outside
  `convertTextRotation`'s `[-5400000, 5400000]` and therefore reads as **zero**
  (`objectformatter.cxx`:1085-1093) — so `bTextBreak` stays true.
- `DrawingChartPlot.AxisTextOf` (`XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`:943) already
  answers `LineBreakAllowed: !date && rotation is 0.0 or 90.0 or 270.0`, i.e. **true here**, and
  `ChartAxisLabels.Resolve` already models the whole ladder — the wrap-restart, the stagger, the
  45°, the thinning. **This tree does not wrap unconditionally; it wraps because no word of
  `038` is wider than the limit, and neither is it in the reference.**

So the round's first instruction — *find what each importer does with `TextBreak`* — is answered
and is a dead end. The trigger is elsewhere.

## 1. The tree's wrap limit is right, and a 36-point sweep says so

`ChartAxisLabels.Wraps` breaks when a word's advance exceeds `0.95 × tick pitch`
(`VCartesianAxis.cxx`:753-759). `sweep-frame-width.py` holds one unbreakable label —
`nnnnnnnnn`, nine `n`, **53.196 pt** advance at 11 pt Carlito on chart2's 96 dpi device — and
sweeps the chart frame's `cx` from 270 to 340 pt in 2 pt steps, which moves the pitch
continuously. `frame-sweep.txt`:

| frame | diagram (0.96×frame) | pitch | 26.2.4.2 |
|---:|---:|---:|---:|---|
| … 288 | 276.48 | 55.30 | turns |
| **290** | 278.40 | **55.68** | **turns** |
| **292** | 280.32 | **56.06** | **upright** |
| 294 … | 282.24 | 56.45 | upright |

The pitch is `0.96 × frame / 5` **exactly** — the unmodified document's 282.30 pt frame gives
the 54.20 pt pitch both renderers draw — so the model has no free parameter. Two candidate
limits were put against it:

- **A**, this tree's: `0.95 × final pitch` → predicts the switch at frame **291.6 pt**.
- **B**, *the arrangement is decided at the rectangle `reduceToMinimumSize()` +
  `adjustInnerSize(maximum labels)` leaves* (`ChartView.cxx`:557-560, :580-588) → **318.1 pt**.

Measured switch: **between 290 and 292**. **A is right to 0.2%; B is refuted by 9%.**

*(Two caveats for reading that table. The sweep refutes B outright and it cannot **separate**
A's word-break boundary from the collision boundary that follows it, because at this pitch the
two coincide: above frame 291.6 no word break fires and the label is one line anyway, and below
it the break fires, the labels go to one line and then collide. The collision itself needs the
shape to be about **2.8 pt** wider than the text advance — `53.196 + 2.835 = 56.03` falls between
the two measured pitches to 0.03 pt, and 2.835 pt is 100 units of 1/100 mm — but where that comes
from is not established here.)*

## 2. But the limit does not explain `038`, and every word of `038` fits

`isolate-label.py` puts each of the nine words of `038`'s five labels into all five category
slots on its own. **All nine are drawn upright, one line, at the document's own frame**, so not
one of them exceeds the limit:

| word | advance (pt) | 26.2.4.2 |
|---|---:|---|
| Cost 20.099 · Brand 27.252 · Service 32.706 · Quality 32.893 · Product 36.013 | | upright |
| Efficiency | 43.638 | upright |
| Customer | 44.517 | upright |
| Innovation | 49.252 | upright |
| Reputation | 50.878 | upright |

(`advances.py` reads Carlito's own `hmtx` and applies `MetricGrid.Chart.PixelEmScale`,
`round(11×96/72)/(11×96/72) = 1.022727`. Each is 0.13–0.57 pt above the reference's drawn **ink**
for the same word, which is the side bearings and the right sign: an advance is wider than the
ink it carries.)

**A width rule cannot be the trigger.** Set every category to one whole label and the reference
splits four ways with the *wider* labels wrapping:

| all five categories = | word 1 | word 2 | whole | 26.2.4.2 |
|---|---:|---:|---:|---|
| `Product Quality` | 36.013 | 32.893 | 71.487 | wrapped |
| `Customer Service` | 44.517 | 32.706 | 79.804 | wrapped |
| `Cost Efficiency` | 20.099 | **43.638** | 66.318 | **TURNS 45°** |
| `Brand Reputation` | 27.252 | 50.878 | 80.711 | **TURNS 45°** |

`Cost Efficiency` is the narrowest of the four, by whole label *and* by widest word, and it is
one of the two that turn.

## 3. The trigger is hyphenation, and three pairs establish it with no free parameter

`PropertyMapper::getTextLabelMultiPropertyLists` sets **`ParaIsHyphenation` true** — beside
`TextMaximumFrameWidth`, inside the same `if (nLimitedSpace > 0)` and nowhere else
(`chart2/source/view/main/PropertyMapper.cxx`:550-557) — and `DrawModelWrapper` installs
`LinguMgr::GetHyphenator()` on the drawing outliner under the comment *"Hyphenation and
spellchecking"* (`chart2/source/view/main/DrawModelWrapper.cxx`:72-86). So a category label
narrow enough to need breaking is laid out **with hyphenation on**, EditEngine may put a
hyphenated fragment of the next word on the current line, and `lcl_hasWordBreak`
(`VCartesianAxis.cxx`:369-404) then sees a line whose start is not a word start — which is
exactly its trigger — sets `m_bLineBreakAllowed = false` and restarts the axis (`:888-905`).

Three pairs, all at the document's own frame, all in `label-variants.tsv`:

**(a) A wider word wraps and a narrower one turns.** Same first word, second word swapped for
four English words with no hyphenation point:

| all five categories = | second word | advance | 26.2.4.2 |
|---|---|---:|---|
| `Cost Efficiency` | *ef-fi-cien-cy* | **43.638** | **TURNS** |
| `Cost Thoughts` | one syllable | 42.610 | wrapped |
| `Cost Strengths` | one syllable | 43.742 | wrapped |
| `Cost Stretched` | one syllable | 44.401 | wrapped |
| `Cost Scratched` | one syllable | 45.181 | wrapped |

`pair-hyphenation.png` is `Cost Efficiency` over `Cost Stretched`, 300 dpi, reference on both
halves.

**(b) The same two words, order swapped.** `pair-wordorder.png`:

| all five categories = | 26.2.4.2 |
|---|---|
| `Cost Efficiency` | **TURNS 45°** |
| `Efficiency Cost` | wrapped — `Efficiency` on line 1, `Cost` on line 2 |

With `Efficiency` first it fits its own line and nothing is hyphenated; with `Efficiency`
second, `Cost ` (22.643) leaves room for `Effi-` (18.386) and the line after it starts inside
`Efficiency`. **No property of the label's characters, widths or line count differs between
these two documents.**

**(c) The pair repeated with a longer first word**, so it is not an artefact of `Cost`:
`Quality Efficiency` **turns**, `Quality Stretched` (the wider word) **wraps**.

The predictions were written before the renders in every case: `Quality ` 35.474 + `Ef-` 12.371
= 47.845, inside the 51.49 limit, so a fragment fits; `Quality ` + no fragment of `Stretched`
exists, so the break falls at the blank.

## 4. What else the variants pin, and the one observation nothing explains

- **A plain width limit exists as well** and sits between 45.181 and 47.285 pt on these
  all-identical sets: `nnnnnnnn nnnnnnn` turns with neither word hyphenatable and a widest word
  of 47.285, while `Cost Scratched` (45.181) wraps. That is 0.833–0.872 of the 54.20 pitch, well
  below the 0.95 §1 confirms on a *single-word* label — so the limit that applies to a line
  **inside a wrapped label** is smaller than the one that applies to the label as a whole. Not
  resolved.
- **Turning line breaking off does not imply turning the axis.** `Innovation` (49.252) alone,
  and `Reputation` (50.878) alone, are each drawn upright **on one line** with no wrap — which
  is the signature of a break that fired and then found no collision. This is why r90's
  three-step ladder reads as four outcomes in the data: wrapped, one-line upright, turned as
  text, turned as outlines.
- **One variant is not explained by anything above and is recorded as open.** `drop3` — the five
  real labels with `Cost Efficiency` replaced by `x` — **wraps**, `Brand Reputation` included,
  at the full 54.19 pt pitch (measured off its own drawn labels); the identical label in
  `all2` turns. The two differ only in the other four labels, so the limit in force must differ
  between them, and no rectangle model tried here can produce a limit ≥ 50.878 for one and
  < 50.878 for the other. Re-rendered and reproduced (`recheck_drop3`).

## 5. Reach

`turned-census.py` counts, per document and per side, the text lines turned off the horizontal
and the glyph-sized filled paths, over the banked whole-corpus gate at
`/home/user/gate-orig-r83` — **947 documents on each side, all scored**. `turned-census.tsv`.
It is a screen and not a verdict: turned text can come from a rotated table cell or a Fontwork
as well as from an axis, and a glyph-fill excess can be a gradient, a blurred shadow or a
picture bullet as well as an outlined turned label.

| | documents |
|---|---:|
| the reference turns **text** where we turn none | **5** |
| we turn text where the reference turns none | 8 |
| both turn text | 27 |
| the reference draws ≥30 more glyph-sized fills **and** we turn nothing | **27** |

**The five unambiguous ones** — the reference turns real text, we turn none — are
`055_Project_timeline_with_milestones` (38 lines), `033_Event_planning_tracker` (12),
`027_Simple_personal_cash_flow_statement` (8), `047_Date_tracker_Gantt_chart` (5) and
`TOGAF9-Tool-ConfReqts-CSQ` (1). All five are workbooks, which is where a chart's labels stay
text because the chart was not squeezed enough to shear them.

**The 27 fill-excess candidates are a superset that contains the witness**: `038` is in it at
**+128** (24 fills ours against 152). It also contains documents whose excess is plainly
something else — `Sean Monogue` (+360, an ODF-embedded-font deck), `Wildlife for REDAC`
(+7915) — so the number to carry forward is *"27 to screen"*, not *"27 affected"*.

**The eight in the other direction are mostly not the opposite error.** Six of them —
`057_Simple_balance_sheet` (ours 20 turned, fills 87/336), `Demick_JetBlue` (68, 255/663),
`N2_E_Maestroni_Swarm_COP` (19, 57/225), `8_P-Pavese_AIRBUS` (20, 45/141),
`Intersil_Italy_CAN_Bus` (9, 112/425), `Fundamentals_Module_1_basics` (18, 276/294) — carry a
large *reference* fill excess beside our turned text, which is both sides turning and only one
of them outlining. That is the shear ceiling `CLAUDE.md` records, and it is why the parent's
instruction not to sweep `057` in is right. The two that may be genuine are
`EHEST-SMS-Safety-Management-Manual-V2.docx` (2 lines, 14/27) and `150_5300_13_chg12.doc`
(2 lines, 162/2).

> **CORRECTED BY THE PARENT SESSION AT MERGE.** The paragraph below reads the wrong column and
> its conclusion does not hold. `252/272` is column 8, `rawwords`. **The gate decides on column 9,
> `glyphs`**, and this document's r83 row reads `1585/1449` — **+136**, exactly as the brief said,
> against a band of `max(2 %, 15) = 28.98`. The row *is* this document's:
> `slides/chartset-008/pptx/038_Competitive_Advantage_Card_…`, verdict `words`. **The brief's
> signature stands and does select `038`**; the reach section resting on discarding it should be
> re-derived on column 9. Everything else in this round — the `TextBreak` refutation, the
> hyphenation trigger and the frame sweep — is independent of this and unaffected.
>
> This is the third time this session that column 4 or 8 has been quoted where the gate uses
> column 9, and the second time it reached a write-up.

~~**The brief's proposed signature — a positive glyph delta — does not select these documents.**
`038` itself is **252/272** in the r83 gate: we draw *fewer* alphanumerics than the reference,
not 136 more. The +136 arithmetic in r90 counts what the reference loses to outlining and not
what it gains elsewhere on the same page, and the row it was read from is not this document's.~~

## 5a. One source change was considered and is not worth making

`DrawingChartPlot.AxisTextOf` answers `LineBreakAllowed` true for a stated rotation of 90° or
270° where `axisconverter.cxx`:363 requires `!bUseFixedInnerSize` as well — a real misreading of
one line. **Its reach is nil and the corpus cannot witness it**: of every chart part in the 947
documents, **zero** state an axis `a:bodyPr rot` of exactly ±5400000, and **zero** state
`c:layoutTarget val="inner"` at all. The second half of that is worth recording on its own,
because `CLAUDE.md` devotes a paragraph to `layoutTarget val="inner"` and to a round having been
sent after `VDiagram::adjustInnerSize` on account of it: **no corpus document states it.**

## 6. What a fix would cost, and why this round did not attempt one

Reproducing the trigger needs a hyphenator: Liang patterns for the label's language, applied
inside `ChartAxisLabels.Wrap`/`Wraps` at the same limit, so that a line beginning at a
hyphenation point counts as a mid-word break. Three things make that a feature rather than a
round:

1. **A pattern set has to be shipped or resolved**, and rule 3's *ship only the faces the distro
   packages ship* test applies to dictionaries too — LibreOffice reaches `hyph_en_US.dic`
   through `LinguMgr`, which is a system install, and this tree has no equivalent.
2. **The language is per run**, and a wrong hyphenation turns axes the reference wraps — the
   expensive direction, because every one of the corpus's chart-bearing documents is exposed.
3. **The residual in §4 is unresolved**: the limit that applies to a line inside a wrapped label
   is measurably smaller than 0.95 of the pitch, and `drop3` shows the limit is not a function
   of the pitch alone. Implementing hyphenation against a limit that is still wrong would fit two
   errors against each other.

**What a later round should carry**: the mechanism (§3) is settled and cited; the limit (§4) is
not, and that is the thing to measure next — with the frame sweep of §1 run on a *two-word*
label rather than a single word, which is the experiment that separates them.
