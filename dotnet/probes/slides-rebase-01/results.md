# Slides rebase 01 — the track re-baselined at 26.2.4.2, and the chart cluster settled

Reference binary **LibreOffice 26.2.4.2 620(Build:2)**, font set including `fonts-dejavu-core`.
Ours at `61c36d2bd5d`, built by the coordinator, rendered with `SOURCE_DATE_EPOCH=1700000000`
and `TZ=UTC`.

The prediction committed before any measurement is `prediction.md` beside this file. It is
scored at the end, and two of its five numbers are wrong in the way the project keeps finding:
the measurement reproduced and the sentence attached to it did not.

---

## 1. The headline — slides is **132 of 163**

| | |
|---|---:|
| documents | 163 |
| **match** | **132** |
| `words` | 31 |
| `pages` | **0** |
| `unembedded` | **0** |
| page-exact | **163 of 163** |
| absolute page error | **0** |
| absolute word error | 5362 |

Per batch: 001 9/9 · 002 7/10 · 003 8/10 · 004 6/10 · 005 9/9 · 006 10/10 · 007 7/10 ·
008 8/10 · 009 9/10 · 010 7/10 · 011 9/10 · 012 8/10 · 013 8/10 · 014 7/10 · 015 9/10 ·
016 7/10 · 017 4/5.

The stored figure was 151/163 against 24.2.7.2 with a different font set. **132 and 151 are not
comparable and neither is a regression in our code**: nothing under `dotnet/` changed between
them. What changed is the reference. See §3 for how much of the 19-verdict gap is an artefact of
the gate rather than a defect.

**Every failure is `words`.** Not one page count and not one font-embedding flag is wrong
anywhere in the track.

---

## 2. Gate check 1 is structurally stable — verified against three independent perturbations

The claim under test: a deck's page count is its slide count, so check 1 should survive a
change of reference binary. It does, and it survives more than that.

| perturbation | page counts moved |
|---|---:|
| reference binary 24.2.7.2 → 26.2.4.2 (font set held) | **0 of 163** |
| font set, DejaVu absent → present (LibreOffice held at 26.2.4.2) | **0 of 163** |
| *our own* renderer, DejaVu absent → present | **0 of 163** |
| ours vs reference, both correct | **0 of 163** |

The third row is worth having: it shows the stability is a property of the format, not of
LibreOffice. Our own output moved **52 of 163 word counts** and **81 of 163 font-count rows**
when DejaVu appeared, and **zero** page counts.

### The independent control: `slideN.xml` against reference page count

Font-independent, container-level, and it does not go through either renderer.

**109 of 112 pptx decks: zip `ppt/slides/slideN.xml` count == reference page count, exactly.**

The three mismatches are not errors — each is exactly accounted for by hidden slides, since
LibreOffice's PDF export omits `<p:sld show="0">` by default:

| deck | slideN.xml | ref pages | `show="0"` |
|---|---:|---:|---:|
| `batch-012/pptx/Sylva%20introduction%20session.pptx` | 16 | 15 | 1 |
| `batch-014/pptx/Structural Testing.pptx` | 92 | 89 | 3 |
| `batch-015/pptx/redac-sas-201703-hf-research-division.pptx` | 14 | 13 | 1 |

`16 − 1 = 15`, `92 − 3 = 89`, `14 − 1 = 13`. **112 of 112 once hidden slides are counted** —
the control returns exactly zero unexplained mismatches. The 51 `.ppt` decks have no zip and
are outside this control.

### A second control worth recording

Two entirely separate `soffice` sweeps of the 163 decks — mine and the coordinator's, different
worker counts, different output roots — agree on **page count, word count and font count for all
163 documents, zero differences**. Reference rendering here is deterministic, so a reference
figure that moves is a real change and not noise.

---

## 3. The word gate on slides is largely counting bullet glyphs

**Verified: 160 of 163 reference word counts moved between 24.2.7.2 and 26.2.4.2.** The
predecessor figure reproduces exactly. What was attached to it is wrong.

The movement is almost entirely **standalone non-alphanumeric tokens**. Over the whole track the
total word delta was 18334 and the total count of tokens with no letter and no digit was 15873.
Their composition, over a 40-document sample: `•` 1717, `–` 568, `` 383, `` 215,
`` 163, `` 94, `●` 69, `` 67, `` 43 — U+2022 and the Symbol/Wingdings
private-use bullet glyphs. `wc -w` scores each one as a word.

Applying the same split to the *current* comparison, ours against the corrected reference:

| | of 163 | of the 31 failures |
|---|---:|---:|
| raw words within the 2% band (the gate) | 132 | — |
| **non-alphanumeric tokens removed from both sides** | **144** | **13 now within band** |

**Control, run first:** of the 132 documents that already match, **131 still match** on the
non-junk count. The instrument does not manufacture agreement — it moves one matching document
out and thirteen failing documents in.

Eight of those thirteen agree on real words **to the digit**:

| ours | ref | ours bullets | ref bullets | ours real | ref real | deck |
|---:|---:|---:|---:|---:|---:|---|
| 838 | 871 | 23 | 56 | **815** | **815** | `080214-Intl-pol-frameworks…ppt` |
| 1025 | 932 | 116 | 23 | **909** | **909** | `ws_prod…PtF-background-+-principles.ppt` |
| 842 | 818 | 43 | 19 | **799** | **799** | `ws_prod…2007-Privileges.ppt` |
| 2855 | 2712 | 193 | 50 | **2662** | **2662** | `ws_prod…MDM.032-(ENGLISH)-CZ.ppt` |
| 6471 | 6291 | 256 | 76 | **6215** | **6215** | `ws_prod…M.017-(French)-France.ppt` |
| 389 | 343 | 57 | 11 | **332** | **332** | `ws_prod…European-Safety-Strategy-Initiative.ppt` |
| 210 | 216 | 60 | 66 | **150** | **150** | `1-secretariat.ppt` |
| 1953 | 2156 | 68 | 271 | **1885** | **1885** | `Aerospace_Journey_of_Flight…ppt` |

**Every one of the thirteen is a `.ppt`**, and twelve of the thirteen are decks where *we* emit
more bullet tokens than the reference does. So the lead is specific: **the `.ppt` text path
emits a different number of bullet characters into the PDF text layer than LibreOffice does**,
and that difference alone accounts for 13 of the 31 verdicts. This is the rare case where one
fix would move a double-digit number of verdicts.

**A caution that is part of the finding.** I cannot separate "LibreOffice 26.2 newly emits
bullets" from "poppler 26.01 newly extracts them" — the container offers no earlier LibreOffice
and the old figures were taken on a different image. What is measurable is that the reference
moved *toward* us: of the 11 decks that failed at 24.2.7.2 by our *over*-counting, 5 land inside
the band of the new reference with our output unchanged.

---

## 4. The remaining 18 — real content differences

Ranked by non-alphanumeric-adjusted deficit or surplus. These are the track's genuine word-gate
work; the thirteen above are not.

| ours real | ref real | Δ | deck |
|---:|---:|---:|---|
| 1926 | 2544 | **−618** | `batch-007/ppt/architecture6.ppt` |
| 992 | 1518 | **−526** | `batch-016/ppt/pres_ioc_phuket.ppt` |
| 1064 | 1467 | **−403** | `batch-012/pptx/Sylva%20introduction%20session.pptx` |
| 1326 | 1030 | +296 | `batch-012/pptx/OnTrac_StarCertificationProgram-3Day.pptx` |
| 2143 | 2010 | +133 | `batch-008/pptx/8_P-Pavese_AIRBUS…pptx` |
| 529 | 638 | **−109** | `batch-009/pptx/NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` |
| 713 | 608 | +105 | `batch-017/pptx/Demick_JetBlue.pptx` |
| 2473 | 2576 | −103 | `batch-013/ppt/RRM-training-syllabus…Dec-2009.ppt` |
| 2454 | 2260 | +194 | `batch-016/pptx/16 - UTM - (NASA).pptx` |
| 2154 | 1958 | +196 | `batch-014/pptx/WiGr_2021W…pptx` |
| 5285 | 5126 | +159 | `batch-014/pptx/N2_E_Maestroni_Swarm_COP.pptx` |
| 2196 | 2270 | −74 | `batch-010/pptx/southern-classic-kennesaw-state-university-final.pptx` |
| 900 | 812 | +88 | `batch-010/ppt/W3_Case_Study…Ed.ppt` |
| 1135 | 1089 | +46 | `batch-010/ppt/Fundamentals_Module_1_basics.ppt` |
| 1189 | 1133 | +56 | `batch-016/pptx/FAAAIandtheArt…pptx` |
| 941 | 973 | −32 | `batch-013/pptx/7-Zulkefli_Part147n66_IKMAS.pptx` |
| 670 | 685 | −15 | `batch-004/pptx/solog_orientation_august_2019.pptx` |
| 418 | 437 | −19 | `batch-014/pptx/Ramp Up Campaign - French.pptx` |

Three of these — `WiGr_2021W`, `FAAAIandtheArt`, and `NAS-Infrastructure-Roadmaps-v16.0` (which
now matches) — the user reviewed as **false positives where we render better than the
reference**; they are excluded from any target, per the brief.

---

## 5. Task 3 — `NWD-GLA…pptx`: the word count is **not** misleading

The user's note was "looks exactly the same, check if the word count is not misleading". It is
not. Measured three ways:

| source | words |
|---:|---|
| the deck's own `<a:t>` text across its 13 slides | 640 total, **609 alphanumeric** |
| reference PDF | 698 total, 60 bullets, **638 alphanumeric** |
| ours | 577 total, 48 bullets, **529 alphanumeric** |

The reference's figure is fully accounted for by the deck's own text — 638 against the source's
609, the surplus being repeated placeholder text ("Official" ×13, "NWD" ×2) that the layout
supplies. **We are short by 109 real words, 17% of the deck.** The deficit is real text loss,
not tokenisation, and the reviewer's "looks the same" is consistent with it: the previously
recorded diagnosis is that pages 6 and 12 hold a subtitle placeholder the reference draws at
**0.99 pt** (a 330 × 90.7 pt frame holding 88 pt text under
`<a:normAutofit fontScale="25000" lnSpcReduction="20000"/>`) and we emit no text record for that
shape at all. A 1 pt block is not ink, so the image diff cannot see it and the eye cannot either
— which is exactly why the gate and the pixels disagree here. **Diagnosed, still not fixed**, and
it is the only slides failure that under-draws by this much.

---

## 6. The chart cluster — three of the four items are already closed, and I can prove it

The brief asked me to verify rather than assume. Three of its four items describe work that has
**already landed**, and the fourth's premise is wrong. Each verdict below is backed by a
measurement, not only by a read.

### 6.1 `8_P-Pavese_AIRBUS…pptx` — table fills: **fixed, measured exact**

The brief said the user was right and a previous brief was wrong. Both are true, and the fix is
already in `HEAD` at `c2fa7537f6b5`, which is an ancestor of the r41 baseline.

Read: the deck names three style ids — `{5C22544A-…}` on slides 3/11/22, `{21E4AEA4-…}` on
slide 14, `{72833802-…}` on slides 17–21. All three resolve:

- `{5C22544A-…}` is defined in the deck's own `ppt/tableStyles.xml`;
- all three are in the ported built-in map at
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingPredefinedTableStyles.cs:84,104,105`,
  and all three are in groups that are implemented (`Medium-Style-2`, `Light-Style-2`);
- the lookup is called from
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:663`,
  with the built-in fallback at
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingTableStyle.cs:185`.

Measured, page 14, fill-colour census of both PDFs:

| colour | ours | reference |
|---|---:|---:|
| `#FBECE7` | **30** | **30** |
| `#F8D7CD` | **25** | **25** |
| `#F4B183` | 17 | 17 |
| `#FFFFFF` | 22 | 23 |

Exact on both of the colours the user reported missing. **Item closed.** The one remaining
`#FFFFFF` is a single fill and not the reported defect.

### 6.2 `16 - UTM - (NASA).pptx` — notdef boxes: **fixed, and the brief's hypothesis is refuted**

The standing hypothesis was "a sixth wiring hole of a familiar shape: glyph fallback reaching the
slide text path but not the chart text path". **Both halves of that are wrong.**

*Refutation 1, by measurement.* The defect is gone. Font sets, ours against the reference:

```
ours: Carlito-Regular, LiberationSans-Bold, DejaVuSans-Bold, LiberationSans, Carlito-Bold,
      DejaVuSans, Carlito-BoldItalic, LiberationSans-Italic, Carlito-Italic,
      LiberationSans-BoldItalic, LiberationMono                     — 11, all embedded
ref : the same eleven faces                                        — 11, all embedded
```

No GNU Unifont anywhere, and `unembedded` is 0 across the entire track. The cause was never
chart text: it was **62 `EMR_EXTTEXTOUTW` records whose whole string is one tab** inside a single
EMF on slide 29, each falling through to Unifont, which draws the C0 range as a hex box. That
was fixed in `MetafileTextEngine`.

*Refutation 2, by reading.* `FallbackShaper` is constructed **exactly once** in the tree —
`/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Spreadsheets/Layout/SheetFonts.cs:263`
— and consumed only at `SheetTextLayout.cs:589,632,964`. It is not reachable from the chart text
path, **but it is not reachable from the ordinary slide text path either**: pptx chart labels have
no drawing code of their own and funnel into `SlideTextLayout`, the same code slide shape text
uses. Both shape at
`/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Layout/SlideTextLayout.cs:1206`
(`TextShaper.Default.Shape(...)`), with no itemisation options and no fallback resolver;
`SlideFonts` (`/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Layout/SlideText.cs:344-382`)
holds a `SystemFontResolver` privately and exposes no `Fallback` property, unlike the words
track's `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.WordProcessing/Layout/LayoutFonts.cs:50`.

So the shape is not "two halves that do not join"; it is **one half missing for the whole
presentations track**. If it is ever wanted, the two call sites are:

- measuring/line-breaking:
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Layout/SlideTextLayout.cs:658`
  (`ParagraphLayouter layouter = new(first);` — the structural twin of `SheetTextLayout.cs:589`),
  which first needs `SlideText.cs:346` to expose its resolver;
- drawing: `SlideTextLayout.cs:1206`, which needs the `FontItemiser.Split` treatment of
  `SheetText.cs:232`, not `FallbackShaper` (that type's own remarks, `FallbackShaper.cs:11-16`,
  say it is for measuring only).

**But no corpus document needs it.** Zero `unembedded` verdicts across 163 decks, and the one
document that had one no longer does. Recorded as a real gap with **measured reach zero**.

### 6.3 `Demick_JetBlue.pptx` — the one item still open, now located to a line

The brief's round-39 diagnosis — "`DrawingStyleMatrix` already reads `a:lnStyleLst`/`a:fillStyleLst`;
what is missing is the automatic-colour rule and a route from the chart reader to the matrix" —
**does not reproduce**. Both of those landed at `d2d4d1eba69`, which is in `HEAD` but *not* in the
r41 baseline `e5f54617c`:

- the automatic-colour rule is
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartAutoFormat.cs`,
  ported from `objectformatter.cxx`, cycle shading included
  (`DrawingChartAutoFormat.cs:259-290`);
- the route exists:
  `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Ooxml/PptxSlideLayoutChart.cs:57`
  passes `theme.Styles` into `DrawingChartPlot.Read`.

And it works. Measured on page 4, ours draws **`#F07F09` ×31, `#9F2936` ×23, `#1B587C` ×23** —
the deck's theme accent 1, 2 and 3 exactly, with the cycle shade correctly zero (3 series,
`c:idx` 0–2, pattern length 6, so `cycle = cycles = 0` and the step lands on zero).

**The reference draws those three accents darkened**: `#B45D03` ×46, `#761D26` ×46, `#12415C` ×46.

The mechanism, established by reading the deck: its theme's **first `a:lnStyleLst` entry** —
`THEMED_STYLE_SUBTLE`, the one automatic series formatting names — is

```xml
<a:ln w="9525" cap="flat" cmpd="sng" algn="ctr">
  <a:solidFill><a:schemeClr val="phClr"><a:shade val="50000"/><a:satMod val="103000"/></a:schemeClr></a:solidFill>
  <a:prstDash val="solid"/></a:ln>
```

`accent1 = F07F09` under `shade 50000` in linear RGB computes to `(176, 92, 5)`, which `satMod
103000` nudges to the reference's `#B45D03 = (180, 93, 3)`. The other two accents check the same
way. So the reference substitutes the accent for `phClr` **inside the subtle line style** and lets
that style's own colour transforms act on it; we take the accent raw.

**The wiring hole, exactly.** The width half of this already does the right thing and the colour
half does not, in the same file:

- `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs:1348-1362`
  — `AutoLineWidth` fetches `styles.LineStyle(DrawingChartAutoFormat.SubtleStyleIndex)` at
  **line 1357**, reads **only `@w`** from it at line 1358, and then explicitly discards the theme
  at **line 1360** (`_ = theme;`). The `a:solidFill` with its `a:shade`/`a:satMod` is never looked at.
- `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingStyleMatrix.cs:153`
  — `LineStyle(int)` returns the raw `a:ln` **with `phClr` left in place**, and its own doc comment
  states the choice being made: *"the caller that needs it — a chart's automatic series formatting
  — wants the width and supplies its own colour from the accent cycle"*. That sentence is the bug,
  written down.
- `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartAutoFormat.cs:175`
  — `ColourOf(style, frame, stroke, seriesIndex, maxSeriesIndex, theme)` **takes no
  `DrawingStyleMatrix`**. That is the join. Its three call sites are `DrawingChartPlot.cs:777`,
  `:779` and `:1277`, and `automatic.Styles` (the matrix, carried on `ChartAutoContext`,
  `DrawingChartPlot.cs:673-674`) is in scope at all three and passed only to `AutoLineWidth`.

**To implement:** give `ColourOf` the matrix; for `stroke: true`, take
`styles.LineStyle(SubtleStyleIndex)`, substitute the resolved accent for its `phClr`, and resolve
the result. The substitution helper already exists and is already used elsewhere —
`DrawingStyleMatrix.Substitute(XElement, Colour)` at
`/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingStyleMatrix.cs:263`,
called by `PptxDiagramStyles.cs:191`. Nothing new has to be written; a parameter has to be
threaded and one existing helper called.

**The subgrid half, still open and now quantified.** `Demick_JetBlue`'s five chart parts state
`c:minorGridlines` — 2 per chart, 10 in the deck — and nothing in the tree reads it:
`/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs:374`
(`GridOf`) tests only `majorGridlines`. Page 4 measured: the reference strokes **49 `#8B8B8B` and
35 `#666666`**; we stroke **21 `#B3B3B3`**. So both the count and the colour are wrong — the
missing minor grid and a default grid colour (`0xB3B3B3`, from `GridProperties.cxx:64-66`) that
26.2.4.2 no longer uses. That is the user's "no subgrid", and it needs a reader as well as a
colour.

The legend key's line-and-swatch is drawn — `ChartLayout.cs:3226` is the flat width a legend key
takes when its series draws a line — so once the series colour is right the legend key follows it.

### 6.4 `Fundamentals_Module_1_basics.ppt` / `W3_Case_Study…ppt` — arrows: **premise refuted**

The brief asked whether these decks' shapes reach `PptShapeGeometry.PresetOf` before assuming an
entry is missing. **They do, and the entries are not missing.** Traced by reading:

| step | file:line |
|---|---|
| `Sp` record `0xF00A` | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.MsBinary/Escher/EscherRecordTypes.cs:49` |
| instance = top 12 bits | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.MsBinary/Records/DffRecord.cs:93-96` |
| shape type assigned | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.MsBinary/Escher/EscherDrawingReader.cs:222-226` |
| carried on the shape | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.MsBinary/Escher/EscherShape.cs:49` |
| **the `PresetOf` call** | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/MsBinary/PptSlideLayout.cs:925` |
| preset → path | `/c/sandbox/workdir/libreoffice-core/dotnet/src/XRay.Office.Presentations/Layout/SlidePresetGeometry.cs:39-43` |
| **rectangle fallback** | `SlidePresetGeometry.cs:43` (`: Rectangle(size)`), built at `:65-71`; null-name source at `PptShapeGeometry.cs:282` |

Shape-type histograms, read out of the Escher records of the two decks:

- `Fundamentals_Module_1_basics.ppt`: type 1 ×147 (genuine rectangles), **13 ×1 rightArrow**,
  **69 ×7 leftRightArrow**, **87 ×4 leftBrace**, **88 ×1 rightBrace**, **104 ×2 curvedUpArrow**,
  plus 0 ×87, 3 ×6, 20 ×77, 75 ×101, 202 ×144.
- `W3_Case_Study…ppt`: type 1 ×90, **72 ×2 irregularSeal2**, **103 ×3 curvedLeftArrow**,
  plus 0 ×28, 3 ×3, 20 ×8, 75 ×18, 202 ×38.

All 21 arrow-family ids are in the 148-entry table and every one of their names exists as a real
definition in `PresetShapeGeometry.txt`, so a resolved name does not hit the rectangle fallback
either. The custom-geometry override that outranks the preset (`PptSlideLayout.cs:934-937`,
`PptCustomGeometry.Has` at `PptCustomGeometry.cs:74-78`) is **not firing**: none of the
13/69/72/87/88/103/104 shapes carries Escher property 325 (`pVertices`) or 326 (`pSegmentInfo`).

So **the mapping table is correct and adding entries would move nothing.** The defect is
downstream of name resolution. The next round should start by unit-testing
`SlidePresetGeometry.Outline("rightArrow", …)` and `("leftRightArrow", …)` directly — if those
produce a degenerate path, the fault is in `CustomShapeGeometry`'s guide/formula evaluator, not
in `PptShapeGeometry`.

---

## 7. The prediction, scored

| # | predicted | measured | |
|---|---|---|---|
| P1 | refpages moved: **0 of 163** | **0 of 163** | ✅ |
| P2 | `slideN.xml` == refpages for **≥108 of 114**, mismatches all hidden slides | **109 of 112**, all 3 mismatches exactly hidden slides | ✅ (deck count was 112, not 114) |
| P3 | refwords moved at all: **150–163** | **160 of 163** | ✅ |
| P4 | refwords beyond the 2% band: **3–12 of 163** | **141 of 163** | ❌ **badly wrong** |
| P5 | verdicts surviving: **144–151** | **132 of 163** measured properly | ❌ |
| C1 | `DrawingStyleMatrix` resolves the style lists; missing the auto-colour rule and the matrix route | the rule and the route both already exist; the hole is one unpassed parameter | ❌ |
| C2 | chart text cannot reach `FallbackShaper` — a sixth wiring hole | true but irrelevant: **slide** text cannot reach it either, and the defect had a different cause and is fixed | ❌ |
| C3 | the built-in table is ported but never consulted | it is ported **and** consulted, and the fills are exact | ❌ |
| C4 | the `.ppt` shapes never reach `PresetOf` | they reach it, and it covers them | ❌ |

**Five of nine wrong, and P4 is the instructive one.** I predicted the predecessor's "160 of 163
moved" was true and its implication false — that the 2% band would absorb the movement. The
band absorbs nothing: 141 of 163 move past it. The *shape* of my reasoning was right for a
different reason than I gave, and only §3's split of bullet tokens from real words recovers it.

Four of the five chart priors were wrong in the same direction: **I assumed a briefed defect was
still open when the fix had already landed.** Three of the brief's four chart items describe work
that is in `HEAD`. The lesson for the next brief is cheap to apply — `git log -1 -- <file>` and
`git merge-base --is-ancestor` on the fix commit against the baseline commit, before planning a
round around a defect.

---

## 8. What is open, in order

1. **`.ppt` bullet-character emission** — 13 of the 31 failures, 8 of them agreeing on real words
   to the digit. The largest verdict-moving lead the track has had.
2. **`architecture6.ppt` (−618), `pres_ioc_phuket.ppt` (−526), `Sylva…pptx` (−403)** — real text
   loss, unexplained, and each larger than every chart defect combined.
3. **`Demick_JetBlue` series colour** — §6.3, one parameter through `ColourOf`.
4. **`c:minorGridlines`** — unread at `DrawingChartPlot.cs:374`; 10 instances in `Demick_JetBlue`
   alone; and the major-grid default colour no longer matches 26.2.4.2.
5. **`NWD-GLA` 0.99 pt placeholder** — 109 real words, diagnosed, not fixed.
6. **`.ppt` arrow geometry** — downstream of `PresetOf`; start at `SlidePresetGeometry.Outline`.
7. **Presentations glyph fallback** — genuinely absent (§6.2), measured reach **zero**. Do not
   spend a round on it.

## 9. Files

- reference sweep (mine, independent): `/tmp/…/scratchpad/slides/refbase-dejavu/ref-baseline.tsv`
- canonical reference used for the scoreboard: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`
- our renderings and the parity table: `/tmp/…/scratchpad/slides/ours/parity.tsv`
- the superseded no-DejaVu pair, kept as the evidence for §2: `/tmp/…/scratchpad/slides/refbase/`
  and `/tmp/…/scratchpad/slides/ours-nodejavu/`
