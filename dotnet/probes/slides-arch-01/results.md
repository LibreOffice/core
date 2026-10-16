# slides-arch-01 — results

**Brief:** `slides/batch-007/ppt/architecture6.ppt`, page-exact at 31/31, **1926 words against
2544** — "we draw 618 fewer words than the reference, about a quarter of the document", the
largest word deficit left on the slides track.

**Outcome in one line:** the deficit is entirely the reference's, the document is a ceiling and
has been added to `TODO.raster-ceiling.md`; the round's actual find is that **every bullet on
every binary `.ppt` was drawn hard black**, which the gate cannot see. Fixed, reach measured at
**26 of 163** slides renderings and 935 bullet glyphs, zero regressions.

---

## 1. Which kind of round this was

**Kind 1 of the brief's three traps: the reference splits its own words.** Established first, in
one command, before anything else was looked at.

| | ours | reference |
|---|---:|---:|
| words (`pdftotext \| wc -w`) | 2011 | 2642 |
| gate words (letter-or-digit tokens) | 1926 | 2544 |
| **whitespace-stripped characters** | **11048** | **11038** |

We draw **ten more characters** than the reference. `difflib` ratio 0.9868 over 11k characters,
and every non-equal opcode is one of four things, none of them content — the bullet's Private Use
Area code point, table reading order, three hyphens `pdftotext` de-hyphenates out of the
*reference's* line breaks, and two words (`each layer.`) the **reference** loses off the bottom of
its page 13.

`xray-office extract` returns **2011 words**, exactly matching our rendered PDF. So the model and
the page agree completely: nothing is missing from either, and the "text extracts but does not
render" / "renders but does not extract" fork in the `extraction-comparison` skill does not apply.

### Where the 618 are

Five pages, and only five: **10, 14, 21, 24, 27** — the pattern-table slides.

```
page 10 ours  179 ref  323 delta +144
page 14 ours  194 ref  315 delta +121
page 21 ours  206 ref  344 delta +138
page 24 ours  181 ref  278 delta  +97
page 27 ours  175 ref  306 delta +131
```

On those pages LibreOffice positions the table text glyph by glyph. Page 10's description cell:

```
REF   14.00pt EAAAAA+LiberationSans   65 glyphs in 64 show(s)  "Separat es present at ion and int eract "
OURS  14.00pt DAAAAA+LiberationSans   74 glyphs in 11 show(s)  "Separates presentation and interaction f"
```

`pdftotext` inserts a space at each gap, so `Separates` scores 2 and `presentation` scores 3.
This is the `MIAMI` → `M` `IAM` `I` mechanism already recorded on
`solog_orientation_august_2019.pptx`, at forty times the scale.

### Why LibreOffice does it, which is the part that looks like our bug and is not

The reference's `TJ` arrays carry per-glyph corrections of −12 to −164 thousandths of an em, all
negative, all widening. Solving for the advance each implies, on `Description` at 14 pt bold:

| glyph | D | e | s | c | r | i | p | t | i | o |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| implied by the reference | 831 | 679 | 594 | 594 | 493 | 344 | 719 | 477 | 344 | 688 |
| **DejaVu Sans Bold** | 830 | 678 | 595 | 595 | 493 | 343 | 716 | 478 | 343 | 687 |
| Liberation Sans Bold (what it draws) | 722 | 556 | 556 | 556 | 389 | 278 | 611 | 333 | 278 | 611 |

**LibreOffice measured that text with DejaVu Sans Bold and drew it with Liberation Sans Bold.**
The `/Widths` array in its own PDF is Liberation's, so the corrections exist precisely to force
the glyphs onto positions computed from a face it did not use. The line comes out ~15% wide, the
table needs an extra row of lines per cell, and on page 13 it overruns the page: the last row's
`each layer.` is drawn past the bottom edge and the body text runs through the footer.

Our pen is not implicated. On the same page the 24 pt title `(MVC) pattern` measures
**157.39 pt ours against 157.28 pt reference** — 0.07% — while the 14 pt table text differs by 15%.

### Verdict on the gate

**Unwinnable, and the ceiling is on the reference's side.** To reach 2544 we would have to inflate
our own tokenisation by 32% by adopting per-glyph positioning, which is strictly worse output than
the contiguous shows we emit. Recorded in `TODO.raster-ceiling.md` under shape 3.

`fonts 5/5` is genuine here rather than coincidental — both sides embed
`LiberationSans-Bold`, `Carlito-Regular`, `OpenSymbol`, `LiberationSans`, `LiberationSans-Italic`,
and the reference's `/Widths` for each are that face's own. This is not the trap the brief warned
about.

---

## 2. What the round actually found

### The defect

**Every bullet drawn from a binary `.ppt` was hard black.** All 80 OpenSymbol glyphs in this
document were `0 0 0 rg`. The reference draws 74 of them in the paragraph's own `#46424D` and
**6 in red**, matching the first character run of the paragraph they label.

A blind reviewer, given only the page-7 pair and no numbers, found it independently and ranked it
first: *"the bottom half's level-2 square bullets are colour-matched to the first text run of
their line (two of the five are red). The top half's are all uniformly dark."* It then correctly
refused to call that a diagnosis and listed the three causes an image cannot separate — inheritance
behaviour, wrong-run inheritance, and desaturation in the compositor. Per the `page-vision`
warning, the absence was confirmed in the PDF's own operators and not in the raster.

### The cause

`PptTextBody.Marker` resolved the bullet colour unconditionally:

```csharp
uint colour = properties.States(StatesBulletColour) ? properties.BulletColour : level.BulletColour;
...
PptColour.ResolveText(colour, scheme)          // always a colour, never null
```

PowerPoint writes a `bulletColor` into a paragraph's properties whether or not the bullet has one,
and gates it behind a **separate flag** — `PPT_ParaAttr_BuHardColor`, bit 2 of the bullet-flags
word that the mask's low four bits share. With the flag clear the word means nothing and the
bullet takes the colour of the paragraph's **first character run**:

```cpp
// filter/source/msfilter/svdfppt.cxx:5891-5916 (the paragraph's own set)
if ( bHardBulletColor )  rRetValue = mxParaSet->mnBulletColor;
else { ... rRetValue = rPortion.mpImplPPTCharPropSet->mnColor; }      // the FIRST portion
```

and the same rule again at `:6019-6055` for the fall-through to the master's level. The flags word
is read at `:4881-4883`, where bits 0, 1 and 2 become `BulletOn`, `BuHardFont` and `BuHardColor`.

**Our reader kept bit 0 and threw the rest away** (`PptTextReader.cs:544`), so the flag was not
merely mis-tested — it was not in the model at all.

The layout side was already correct and had been waiting for it: `SlideMarker.Colour` is
documented as *"or null for the first run's"* and `SlideTextLayout.cs:413` paints
`marker.Colour ?? first.Colour`. The ODP reader has always passed null. Only the PPT reader
insisted.

### The change

Two files, 53 lines including comments.

- `src/XRay.Office.Presentations/MsBinary/PptTextReader.cs` — `PptParagraphRun` gains
  `ushort BulletFlags`, carrying the shared word whole instead of only its bit 0.
- `src/XRay.Office.Presentations/MsBinary/PptTextBody.cs` — the hard-colour flag is read from the
  paragraph when its mask names it and from the master's level otherwise, and the colour is passed
  only when it is hard:

```csharp
bool hardColour = properties.States(StatesBulletHardColour)
    ? (properties.BulletFlags & BulletHardColourFlag) != 0
    : (level.BulletFlags & BulletHardColourFlag) != 0;
...
hardColour ? PptColour.ResolveText(colour, scheme) : null
```

`PptStyleSheet` already parsed the level's flags word and already kept all four bits; only bit 0
had a reader.

### Verification on the subject

All 80 bullets, joined position by position against the banked reference:

| | slate `#46424D` | red | unmatched by the join |
|---|---:|---:|---:|
| ours, before | 0 (76 black) | 0 | 4 |
| **ours, after** | **70** | **6** | 4 |
| reference | 70 | 6 | 4 |

Exact parity, including which two of the ten level-2 bullets are red.

---

## 3. Measured reach

Whole slides track rendered twice — once at `HEAD` before the change, once after — with
`SOURCE_DATE_EPOCH=1700000000` set so the two are byte-comparable with nothing masked.

**26 of 163 renderings changed. 137 byte-identical.** Every one of the 26 is a binary `.ppt`; no
`.pptx` or `.odp` moved, which is what the change should do and is worth having measured rather
than assumed.

Two independent measures of *direction*, both against the banked references:

| measure | result |
|---|---|
| bullet glyphs joined to the reference by position | **935 now take exactly the reference's colour**, **0 moved away**, 10 neither, 403 the join could not place |
| colour-multiset distance to the reference, position-free | **21 of 26 documents closer, 0 further**, 5 unchanged in aggregate |

The 403 unplaced and the 10 "neither" are both concentrated in documents whose *layout* diverges
from the reference for unrelated reasons — `1-secretariat.ppt` puts our nearest run 54–448 pt from
the reference's, so the join fails there. That document's position-free distance still fell from
**106 to 34**. Neither measure shows a single document moving away.

The 26:

```
010605Vul                          Aerospace_Journey_of_Flight_Chapter    Airport Planning 09112013
080214-Intl-pol-frameworks         Architecture                           EG1_dsrc tech
1-secretariat                      Fundamentals_Module_1_basics           JesuitAssocOfStudentPersonnel
Lepore                             RRM-training-syllabus-Chapter-3         Thailand17
W3_Case_Study_of_a_Tsunami_Warning  WC_Update-Aug03                       architecture6
berlin                             concepts-surrounding-cloud-computing   gfopportunitiesforlinkagespres_2010_en
hofman                             pods05                                 pres_ioc_phuket
undp_presentation_revised_17_may   ws_prod-g-doc-…-Approval-of-Flight-Conditions
ws_prod-g-doc-…-IACA-presentation  ws_prod-g-doc-…-NATO-activities        ws_prod-g-doc-…-ESM
```

### The white bullets, which were the risk and are correct

157 bullets across four decks turned **white**, which is exactly the failure mode §P5.3 of the
prediction flagged as invisible to the gate. It was checked rather than argued:

| document | newly white | reference at those spots |
|---|---:|---|
| `JesuitAssocOfStudentPersonnel.ppt` | 21 | white, 21 of 21 |
| `Airport Planning 09112013.ppt` | 15 | white, 13; unplaced, 2 |
| `Fundamentals_Module_1_basics.ppt` | 71 | white, 14; unplaced, 57 |
| `1-secretariat.ppt` | 59 | white, 10; grey, 10; unplaced, 39 |

They are white-on-dark decks and the reference draws them white too. This is a case where the
gate's blindness would have hidden either answer, and only the banked reference could settle it.

---

## 4. Regression

`batch-check.sh`, `slides/batch-001` … `slides/batch-007`, before and after:

```
TOTAL 68  MATCH 66  MISMATCH 2  REF-CANNOT-RENDER 0       (both runs)
```

The path, pages, words and verdict columns are **identical row for row** between the two sweeps —
`diff` returns nothing. The two failures are the two that were failing before:

- `slides/batch-004/pptx/solog_orientation_august_2019.pptx` — 15/15, 670/685 words, the
  documented ceiling, untouched and expected to stay failing;
- `slides/batch-007/ppt/architecture6.ppt` — 31/31, 1926/2544 words, this round's ceiling.

`slides/batch-007` alone: 10 documents, 9 match, 1 mismatch.

## 5. Tests

Every project run individually, counts compared against the previous known-good rather than
against the colour of the output.

| project | before | after |
|---|---|---|
| `XRay.Office.Containers.Tests` | — | 109 passed, 0 failed |
| `XRay.Office.Core.Tests` | — | 313 passed, 0 failed |
| **`XRay.Office.Fidelity.Tests`** | **30 failed, 520 passed, 550 total, 0 skipped** | **30 failed, 520 passed, 550 total, 0 skipped** |
| `XRay.Office.Markup.Tests` | — | 259 passed, 0 failed |
| `XRay.Office.OpenDocument.Tests` | — | 125 passed, 0 failed |
| `XRay.Office.Presentations.Tests` | 646 | **651** passed, 0 failed (5 added) |
| `XRay.Office.Rendering.Tests` | — | 150 passed, 1 skipped |
| `XRay.Office.Spreadsheets.Tests` | — | 758 passed, 0 failed |
| `XRay.Office.Text.Tests` | — | 310 passed, 0 failed |
| `XRay.Office.Vector.Tests` | — | 295 passed, 0 failed |
| `XRay.Office.WordProcessing.Tests` | — | 818 passed, 0 failed |
| **total** | | **4339 run, 30 failed** |

Build is warning-free.

`tests/XRay.Office.Presentations.Tests/PptBulletColourTests.cs` adds five, and **no test in the tree
asserted a bullet colour before them**. They were checked against the unfixed tree rather than
merely written: with `PptTextBody.cs` reverted and `PptTextReader.cs` kept, two fail
(`ABulletWhoseHardColourFlagIsClear…`, `AMastersLevelWithTheFlagClear…`); with both reverted the
file does not compile, because `PptParagraphRun.BulletFlags` does not exist. Both halves of the
fix are covered.

They are synthetic records deliberately: every `.ppt` in `tests/corpus` was written by
LibreOffice's own exporter, which sets the hard-colour flag on everything it emits, so the case
that matters cannot be reached from a committed deck.

---

## 6. Scoring the prediction

| | claim | outcome |
|---|---|---|
| P1.1 | streams match, 11048 vs 11038 | measured before writing |
| P1.2 | shape 3, five pages | measured before writing |
| P1.3 | gate unwinnable | **stands** |
| P2 | hard-colour flag | measured before writing |
| P3.1 | the 15% is DejaVu-vs-Liberation, not our pen | **stands**, and neither was chased |
| P4 | 26 of 163, 935 glyphs, 0 away | measured before writing |
| **P5.1** | sweep stays 66 of 68, no verdict moves | **correct** — columns identical row for row |
| **P5.2** | `architecture6` stays 1926/2544 | **correct**, unchanged |
| **P5.3** | white-on-dark bullets do not vanish | **half wrong, and the useful half.** 157 bullets *did* turn white. The prediction was right that the sweep would not see it and right that it was not a defect — the reference draws them white — but wrong to expect it not to happen. It happened on four documents and would have shipped unexamined had the prediction not named it |
| **P6.1** | Fidelity stays 30 of 550 | **correct**, exactly |
| **P6.2** | other projects unchanged | **correct** |
| **P6.3** | no existing test asserts a bullet colour | **correct** |
| **P7.1** | still the only batch-007 failure, joins the ceiling list | **correct** |
| **P7.2** | visible page improved, gate column did not move | **correct** |

Twelve of thirteen scoreable claims held. The one that did not is the one worth keeping: naming a
risk the gate is blind to is what caused it to be measured instead of assumed.

---

## 7. What was found and deliberately not fixed

- **Level-2 outline text is 18.00 pt here against the reference's 18.99**, with the level-2 bullet
  at 18.00 against 18.51 and the level-1 bullet at 22.00 against 22.20. Confined to **pages 7 and
  30**, the only two-level bullet slides; every other size in the document matches exactly
  (14.00, 20.01, 22.00, 24.01, 28.01, 11.99, 32.00, glyph for glyph). The mechanism is
  `Outliner::ImpCalcBulletFont` — `bulletSize = firstCharHeight × GetBulletRelSize()/100 × fFontY`
  with `PPTParaSheet::UpdateBulletRelSize` turning a negative stored height into a percentage by
  integer division (`svdfppt.cxx:4081-4091`) — but the arithmetic does not close on the measured
  numbers and was not chased. 774 glyphs on this document. **Unmeasured elsewhere.**
- **The `BuHardFont` gate (bit 1) is still not implemented.** LibreOffice applies the same
  hard/inherit rule to the bullet's *face* as to its colour (`svdfppt.cxx:5920-5927`). Our bullets
  already resolve to the same face as the reference on every page of this document, so there was
  nothing measured to fix and it was left alone. `BulletHeight` has no such gate in LibreOffice and
  correctly has none here.
- **The bullet's `ToUnicode` code point** differs — `U+E47A` ours, `U+F0B2` the reference — because
  LibreOffice keeps the symbol slot at `0xF000 | code` while we map it to the OpenSymbol glyph we
  draw. Both are unreadable Private Use Area noise; the glyph drawn is identical; neither counts as
  a word. Left alone.
- **The reference's DejaVu-measured / Liberation-drawn table text** is a defect on the reference's
  side and not reachable from ours.

## 8. Reproducing

```sh
export XRAY_OFFICE_CLI=.../dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000

# the character-stream check that decides the kind of round — run this first, always
"$XRAY_OFFICE_CLI" render --outdir /tmp/a slides/batch-007/ppt/architecture6.ppt
pdftotext /tmp/a/architecture6.pdf - | tr -d '[:space:]' | wc -c      # 11048
pdftotext refpdfs-26.2.4.2-fonts/slides/architecture6__ppt.pdf - | tr -d '[:space:]' | wc -c   # 11038

# the mechanism, in the reference's own operators
.claude/skills/render-comparison/scripts/pdf-ops.py dump <ref>.pdf --page 10 --only text
#   14.00pt EAAAAA+LiberationSans   65 glyphs in 64 show(s)

# the bullets
.claude/skills/page-vision/scripts/pair.sh "architecture6__ppt" --page 7 --dpi 129 --outdir /tmp/pairs
```
