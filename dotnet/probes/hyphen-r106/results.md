# O10 — the hyphenator is implemented and shipped, it moves three renderings and none of them worse, and it does not close the gate row the brief expected

Round 106, measured 2026-09-12 against `/opt/libreoffice26.2/program/soffice`
(**LibreOffice 26.2.4.2**, tarball install). Implements the seat `probes/chart-hyph-r105`
sized and `probes/chart-axisrot-r91` §3 established.

**Two things in the brief are refuted by the measurement and the write-up leads with both.**

1. **The gate row does not close.** r105 §3.2 showed that on `038_Competitive_Advantage_Card`
   our column 9 is 1585, the reference's is 1449, and *the reference with its own hyphenator
   off* is 1585 — a clean identity, and the brief read it as "the whole +136 gate delta is this
   seat". It is not. That identity is between the reference's **two** states; it says nothing
   about what *our* rendering becomes when we turn the axis. **We now turn it, and our glyph
   count is still 1585**, because 26.2.4.2 draws those turned labels as **outlines** and we draw
   them as text. Closing the row needs the shear/outlining rule, not the hyphenator. §4.
2. **The reference's leading limit is a character count, not a width**, and the first cut of
   this fix measured the hyphen. §2.3.

---

## 0. What this round settles

| question | answer |
|---|---|
| Does a Liang hyphenator over `hyph_en_US.dic` reproduce 26.2.4.2's own hyphens? | **Yes, 4 of 4** — the reference draws `Gov-ernance`, `Con-sumption`, `Pes-simistic`, `Pas-senger` on three corpus charts and this implementation gives exactly those (§1.2) |
| Does it reproduce its rotate-or-wrap **decision**? | **19 of 22 authored fixtures**, against a base rate of **13 of 22** for the tree as it stood (§2) |
| What does it move on the corpus? | **3 renderings of 568 scored, all three chart-bearing, and all three improve** (§3, §4) |
| Does anything get worse? | **No document of the eight candidates worsens**; summed \|ink\|% 57.00 → 54.72 (§4) |
| Does any gate verdict move? | **No.** Three of the eight fail on `glyphs` before and after, five match before and after (§4) |
| Blast radius outside charts | **0 of 399 non-chart documents move**, and the call graph makes that structural (§5) |
| Licence position | `hyph_en_US.dic` + `README_hyph_en_US.txt` vendored (BSD-style); `fr`/`es` supplied at runtime and **not** committed (§6) |

---

## 1. The implementation

### 1.1 Where it lives, and why not where the brief said

`IHyphenator` was declared and unimplemented at
`dotnet/src/XRay.Office.Text/Layout/ILineBreaker.cs`:45-53. Its **contract** is unchanged and is
what the brief said it was — one method, empty result when no dictionary answers — but the
declaration has **moved to `XRay.Office.Core.Globalization`**, because the first consumer is
`ChartAxisLabels`, which is in `XRay.Office.Core`, and `XRay.Office.Core` may not reference
`XRay.Office.Text` (`dotnet/CLAUDE.md`'s layering diagram; Core has no dependencies by design).
It had no implementations and no call sites, so the move is a rename of a namespace and nothing
else; `ILineBreaker.cs` carries a comment pointing at it, and `XRay.Office.Text` still reaches it
through its project reference when paragraph line breaking wants one.

| file | what |
|---|---|
| `XRay.Office.Core/Globalization/IHyphenator.cs` | the interface, moved |
| `XRay.Office.Core/Globalization/HyphenationPatterns.cs` | the Hunspell pattern-file reader and Liang's algorithm |
| `XRay.Office.Core/Globalization/Hyphenators.cs` | the search path, the `Register` seam, `None`, `DefaultLanguage` |
| `XRay.Office.Core/Globalization/Hyphenation/hyph_en_US.dic` | 106,414 bytes, sha256 `486fb684…` |
| `XRay.Office.Core/Globalization/Hyphenation/README_hyph_en_US.txt` | 1,888 bytes, sha256 `9bcae072…` — **the licence lives here and not in the data** |
| `XRay.Office.Core/Charts/ChartAxisLabels.cs` | `Wraps` gains the hyphenating arm; `Hyphenates` is it |

The two data files are copied to a `hyphenation` folder beside the assembly and packed under
`contentFiles/any/any/hyphenation`, exactly as `XRay.Office.Text` ships its 28 font faces, and for
the same reason: a search path wants a file, and a user supplying French wants somewhere to put
it.

### 1.2 The algorithm is checked against the reference's own hyphens, not against a second implementation

`hyph_en_US.dic` is an encoding line, four keyword lines and 11,125 bare TeX patterns — no `%`
comments, no `NEXTLEVEL`, no non-standard `pattern/replacement` forms. `LEFTHYPHENMIN 2` and
`RIGHTHYPHENMIN 3` are **read out of the file**; they are floored at 2 because LibreOffice floors
them the same way (`lingucomponent/source/hyphenator/hyphen/hyphenimp.cxx`:435 passes
`std::max<sal_Int16>(dict->lhmin, 2)`, and the linguistic property that could raise them defaults
to 2, `linguistic/source/lngprophelp.cxx`:492-493). Curly quotes and apostrophes are folded and
the word is lower-cased before matching, character by character so an offset still means the same
thing in the caller's string (`hyphenimp.cxx`:315-333). Trailing full stops are stripped
(`:333`). **All of that is read out of a tree that is 27.2 alpha, not the 26.2.4.2 binary
(confound C8)** — which is why none of it is trusted on its own.

**The check that does not depend on the source**: r105 §1.2 read four hyphens out of
26.2.4.2's own PDF text layer, on three corpus documents whose charts it hyphenates visibly.
This implementation gives exactly those four:

| the reference drew | this implementation gives |
|---|---|
| `Risk Management & Gov-` / `ernance` | `Gov-ernance`, `Gover-nance` |
| `… and Fuel Con-` / `sumption` | `Con-sumption`, `Consump-tion` |
| `Pes-` / `simistic` | `Pes-simistic` |
| `Total Mainline Pas-` / `senger Revenue` | `Pas-senger`, `Passen-ger` |

Four of four, and in each case the point the reference took is in the list. They are
`HyphenationTests.TheReferencesOwnHyphensAreReproduced`.

Two further agreements fall out of r91's own controls without being fitted to them:
`Efficiency` breaks (`Ef-`, `Effi-`) and **`Stretched`, `Thoughts`, `Strengths`, `Scratched` and
`Cost` have no point at all** — which is r91's whole ordering, the one no width rule can produce,
arriving from the dictionary. `HyphenationTests.TheWitnessWordHyphenatesAndAllFiveOfItsControlsDoNot`.

### 1.3 Where it plugs into the axis, and what it deliberately does not touch

Only `ChartAxisLabels.Wraps` changes. It already asked one half of `lcl_hasWordBreak`'s question
— *is any single word wider than the slot* — and now asks the other: *would the hyphenating fill
start a line inside a word*. `Hyphenates` simulates the greedy fill over blank-separated words
and returns true at the first hyphenated break.

**`Wrap`, which produces the text that is actually drawn, is untouched, and that is not an
omission.** When `Hyphenates` answers true the axis restarts with line breaking off and the
wrapped text is never used; when it answers false nothing was hyphenated and the plain
blank-wrapped answer is already right. So no hyphen ever reaches the page, which is also what
26.2.4.2 does on a turned axis.

---

## 2. Does it reproduce the reference's decision? Twenty-two authored fixtures, both dictionaries, both renderers

### 2.1 The instrument

`word-sweep.py` writes 22 variants of the `038` witness with all five category labels rewritten
to `Cost <word>`. The first word, the font, the size, the frame, the tick spacing and the
category count are held fixed; only the second word varies, and the words are chosen so that
**width cannot order them**: `Quality` (7 characters) has a point and `Thoughts` (8) has none,
`Marketing` (9) has two and `Straights` (9) has none, `Breakthrough` (12) has one and
`Screeched` (9) has none.

Each variant is rendered four ways — the reference with its dictionary and with it switched off
by r105's `delang.py` retag to `de-DE`, and this tree with `XRAY_OFFICE_HYPHEN_DICTS` unset and at
`0`. `word-grid.tsv`:

| word | len | points | ref on | ref off | ours on | ours off | |
|---|---:|---|---|---|---|---|---|
| Quality | 7 | [4] | turned | upright | turned | upright | |
| Service | 7 | [3] | upright | upright | **turned** | upright | **DIFFERS** |
| Thoughts | 8 | [] | upright | upright | upright | upright | |
| Strength | 8 | [] | upright | upright | upright | upright | |
| Splashed | 8 | [] | upright | upright | upright | upright | |
| Stretched | 9 | [] | upright | upright | upright | upright | |
| Strengths | 9 | [] | upright | upright | upright | upright | |
| Scratched | 9 | [] | upright | upright | upright | upright | |
| Straights | 9 | [] | upright | upright | upright | upright | |
| Screeched | 9 | [] | turned | turned | **upright** | upright | **DIFFERS** |
| Squelched | 9 | [] | turned | turned | **upright** | upright | **DIFFERS** |
| Marketing | 9 | [3, 6] | turned | upright | turned | upright | |
| Efficiency | 10 | [2, 4] | turned | upright | turned | upright | |
| Reputation | 10 | [3, 4, 6] | turned | turned | turned | upright | |
| Governance | 10 | [3, 5] | turned | turned | turned | turned | |
| Throughput | 10 | [7] | turned | turned | turned | turned | |
| Excellence | 10 | [2, 5] | turned | turned | turned | upright | |
| Resilience | 10 | [2] | turned | upright | turned | upright | |
| Stringency | 10 | [5] | turned | turned | turned | upright | |
| Consumption | 11 | [3, 7] | turned | turned | turned | turned | |
| Streamlined | 11 | [6] | turned | turned | turned | turned | |
| Breakthrough | 12 | [5] | turned | turned | turned | turned | |

- **ours-with-dictionary against reference-with-dictionary: 19 of 22.**
- **The base rate, which is the tree as it stood: 13 of 22** (`word-agreement-off.txt`).
- ours-without against reference-without: 17 of 22 — that column is the *width* rule alone.
- **The reference's own arrangement moves when its dictionary is switched off on 4 of the 22**
  (`Quality`, `Marketing`, `Efficiency`, `Resilience`). Everything else it turns, it turns
  without a dictionary.

**Two honesty notes on that 19, and the second one costs three rows.**

- Predicting `upright` for everything scores 8 of 22 and predicting `turned` for everything
  scores 14 of 22, so 13 is a little below the trivial null and 19 is well above it.
- **On `Reputation`, `Excellence` and `Stringency` we agree with the reference for the wrong
  reason.** Its own off column shows it turns those three *by width*; we turn them by
  hyphenation. The arrangement matches and the mechanism does not, so the honest reading of the
  19 is *16 agreements plus 3 coincidences* — still against a base rate of 13, and the three
  were already right or wrong for width reasons that this seat does not touch.

### 2.2 The three that differ, and which seat each belongs to

- **`Screeched` and `Squelched` are not this seat.** The reference turns them **with its
  dictionary off** and neither word has a hyphenation point in any of the four installed files,
  so what turns them is the plain word-width limit — r91 §4's unresolved residue, seated as
  **O43**. We leave both upright.
- **`Service` is this seat and it is a false positive.** We hyphenate `Ser-vice`; 26.2.4.2 does
  not. §2.4 is what could and could not be established about it.

### 2.3 The leading limit is a character count, and the first cut of this fix got it wrong

`ImpBreakLine` walks the line's own `CharPosArray` to `nMaxBreakPos`, the first character of the
line that does not fit, and asks the hyphenator for a point at most
`nMaxBreakPos - nWordStart - 1` characters into the word —
`editeng/source/editeng/impedit3.cxx`:2143-2160, whose `+ 1` is commented *"Before the dickey
letter"*. `Hyphenator::hyphenate` enforces it as `i < Leading` (`hyphenimp.cxx`:445). **So one
character's room is reserved for the hyphen rather than the hyphen being measured**, and since a
hyphen is narrower than the letter it stands in for, the width form admits breaks the reference
refuses. The first cut of `Hyphenates` measured `Cost Ser-`; it now counts characters.

*Read out of the 27.2-alpha tree, and it changes no fixture in this round's set* — 19 of 22 both
ways — so it is kept on the strength of the source and of being the **more conservative** of the
two, not on a measurement that separates them. Said plainly because that is the weakest claim in
this write-up.

### 2.4 `Service`: what it is not

`leading-sweep.py` varies the *first* word — `A`, `Co`, `Cost`, `Costly`, `Costlier` — against
`Service`, `Quality` and `Efficiency`, which changes the room on line 1 and nothing else.
`leading-sweep.tsv`, 15 fixtures:

- ours-with-dictionary against the reference: **12 of 15**, base rate **9 of 15**.
- The reference's threshold moves with the room exactly as a leading limit predicts for
  `Quality` (turns at `Cost`, upright at `Costly`) and for `Efficiency` (turns at every width at
  which its label wraps at all).
- **It refutes the leading limit as the explanation for `Service`.** `Marketing`'s usable point
  is at 3, the same offset as `Service`'s, and `Marketing` hyphenates with *less* room than
  `Service` is given. And `Service` never hyphenates at any first word: at `A` and `Co` its
  label fits on **one** line and never wraps, and at `Cost`, `Costly` and `Costlier` it wraps
  and stays upright.
- It is not the word list either. `nearmiss` variants `Servicx`, `Servicz`, `Zervice` and
  `Qervice` — nonsense strings that carry the same `r3vic` pattern and cannot be in any
  wordbook — are all upright at the reference, while `Marketing` in the same batch turns. So
  `HyphenatorDispatcher`'s positive-dictionary short circuit
  (`linguistic/source/hyphdsp.cxx`:302-320) is not it.
- **Not established**: why 26.2.4.2 declines a point the file plainly states.
  `hyph_en_US.dic`:5367 is `r3vic`, an odd value between `r` and `v`, and no `%` exception or
  even-valued pattern cancels it. `Resilience`, whose only point is at 2, does hyphenate, so a
  minimum leading of 4 is refuted too. An attempt to reproduce it outside chart2 failed for its
  own reason: **26.2.4.2 hyphenates none of nine words in a 3.4 cm Writer measure with
  `fo:hyphenate="true"` on either the paragraph or the text properties**, so that instrument
  says nothing and the fixture is not banked.

Reach of the false positive on the corpus: **zero**. No corpus category-axis label moved because
of it (§3), and the 22-word set is authored precisely to find such a case.

---

## 3. Corpus reach — three renderings, and the blast radius is structural

### 3.1 The instrument, and why one binary is two legs

`sweep.py` renders a document with our own CLI under `SOURCE_DATE_EPOCH`, hashes the PDF and
deletes it. The **off** leg sets `XRAY_OFFICE_HYPHEN_DICTS=0`, which makes `Hyphenators.For`
answer null and `Wraps` skip its new arm entirely, so the off leg is the tree as it stood. Two
legs of one binary rather than two binaries removes the rebuild-under-a-sweep trap
`dotnet/CLAUDE.md` records — and the substitution is **checked rather than assumed**:

**Control.** `ChartAxisLabels.cs` was reverted to its base content, `XRay.Office.Core`'s `obj` and
`bin` deleted, the CLI rebuilt, and six documents rendered with **no environment variable at
all**. All six reproduce the banked off leg **byte for byte**, including all three movers;
three of the six differ from the on leg, which is the three movers.
`base-binary-control.tsv`. The change was then restored, rebuilt with `obj`/`bin` deleted, and
the same six reproduce the on leg exactly.

### 3.2 What moved

`hyphenator-off.tsv` is all **947** corpus documents, 0 unrenderable. `hyphenator-on.tsv` is
**568** of them — **all 169 chart-bearing containers** and 399 of the other 778 — 0 unrenderable.
*The on leg is not the whole corpus because the box was carrying three other sessions at load 15
and the sweep had slowed to about one document a minute; the 379 not covered are 328 words and 51
slides documents, and **none of them carries a chart**.*

| | documents scored both ways | renderings that move |
|---|---:|---:|
| chart-bearing | **169 of 169** | **3** |
| everything else | 399 | **0** |
| total | 568 | 3 |

| mover | track |
|---|---|
| `sheets/chartset-013/xlsx/033_Event_planning_tracker_…xlsx` | sheets |
| `slides/ceiling-001/pptx/N2_E_Maestroni_Swarm_COP.pptx` | slides |
| `slides/chartset-008/pptx/038_Competitive_Advantage_Card_…pptx` | slides |

**The confinement is structural as well as measured.** `Hyphenates` is called only from
`Wraps`, which is called only from `ChartAxisLabels.Resolve`, which is called only from
`ChartLayout.ArrangeCategories`. No word-processing paragraph, no slide text body and no
spreadsheet cell can reach it: `XRay.Office.Text`'s `LineBreaker` and `ParagraphLayouter` are not
touched by this diff at all. The 399 non-chart documents are the corroboration, not the
argument.

---

## 4. The per-document table, which is what the brief asked for

r105 §3.1 named **8** chart-bearing documents whose *reference* rendering changes when
26.2.4.2's hyphenator is switched off. Each is scored here against its banked 26.2.4.2 rendering
(`/home/user/gate-orig-r83/ref`, produced by that binary), ours-off and ours-on, on the gate's
own column 9 and on summed unsigned ink from `pdf-image-diff.py`. `eight-scored.tsv`:

| document | pages | glyphs ref | ours off | ours on | band | verdict off → on | \|ink\|% off | \|ink\|% on | MAJOR | 45° ref/off/on |
|---|---:|---:|---:|---:|---:|---|---:|---:|---|---|
| `038_Competitive_Advantage_Card` | 3 | 1449 | 1585 | 1585 | 29.0 | glyphs → glyphs | 1.78 | **1.03** | 1/1 | 0 / 0 / 10 |
| `033_Event_planning_tracker` | 3 | 2650 | 2720 | 2720 | 53.0 | glyphs → glyphs | 12.65 | **12.36** | 1/1 | **12 / 0 / 12** |
| `N2_E_Maestroni_Swarm_COP` | 30 | 28278 | 28178 | 28695 | 565.6 | match → match | 3.29 | **2.05** | **1 → 0** | 0 / 19 / 19 |
| `027_Simple_personal_cash_flow` | 10 | 7949 | 7932 | 7932 | 159.0 | match → match | 3.03 | 3.03 | 1/1 | 8 / 0 / 0 |
| `Keywords_Mapping_Graphs_and_Charts` | 46 | 27201 | 27295 | 27295 | 544.0 | match → match | 10.79 | 10.79 | 11/11 | 0 / 0 / 0 |
| `Demick_JetBlue` | 10 | 3179 | 3519 | 3519 | 63.6 | glyphs → glyphs | 5.51 | 5.51 | 2/2 | 0 / 68 / 68 |
| `southern-classic-kennesaw-…` | 23 | 10946 | 10979 | 10979 | 218.9 | match → match | 10.15 | 10.15 | 4/4 | 38 / 38 / 38 |
| `3495` | 26 | 4375 | 4375 | 4375 | 87.5 | match → match | 9.80 | 9.80 | 3/3 | 0 / 0 / 0 |
| **sum** | | | | | | | **57.00** | **54.72** | | |

- **Three documents move and all three improve. None of the eight worsens by any amount**, so
  there is no net figure hiding a regression and the per-document table and the sum say the same
  thing.
- **No gate verdict moves in either direction.** Three fail on `glyphs` before and after; five
  match before and after.
- **`033` is the clean hit**: 0 turned lines becomes **12**, which is the reference's **12**
  exactly, and the ink falls.
- **`N2_E_Maestroni` is the largest**, 3.29 → 2.05 with its one MAJOR page cleared. Its glyph
  distance from the reference grows (100 → 417) while staying well inside a 565.6 band; the ink
  says the page is closer, and on this document the reference's own chart labels are turned text
  that `pdftotext` reads differently, so the glyph column is the weaker instrument.

### 4.1 Why `038`'s gate row does not close, which the brief expected it to

Our column 9 was 1585 and stays 1585. r105's identity — the reference *with its hyphenator off*
also reads 1585 — is a statement about the reference's two states, and the step it implies (that
turning our axis takes us from 1585 to 1449) does not follow. 26.2.4.2 reads 1449 **because it
draws those five turned labels as filled outlines rather than as text**: r105 §3.1 counted 152
glyph-sized fills in its hyphenated rendering against 24 in its unhyphenated one, and this
round's own fixtures show the same rule firing on six of the 22 variants (77 to 92 fills, no
axis text at all) as soon as the labels are long enough to make the chart's fit squeeze
anisotropically. We turn the same labels and draw them as **real text**, so the characters stay
in the text layer and `pdftotext` still counts them.

**That is the better output** and `dotnet/CLAUDE.md` is explicit that outlining glyphs to make a
text-extraction gate greener is not a fix. So `038` keeps its gate row, and the row is the
shear/outlining rule's, not this seat's. **The seat's measured gate reach is zero rows.**

### 4.2 What 26.2.4.2 actually draws where our change moved something

Checked on the drawn pages rather than assumed, because a round once cut three header banners
whose ink did not move:

- `033`: the reference draws **12 lines at 44.8–45.2°** on its category axis and we drew none;
  we now draw 12. Read off both PDFs by each line's own `dir` vector — PyMuPDF reports a rotated
  span's axis-aligned box, so `dir` is the only discriminator.
- `038`: the reference draws **no axis text** there and 152 glyph-sized fills; the labels are
  turned and outlined. We now turn them (10 turned text lines) where we drew them upright on two
  lines.
- `N2_E_Maestroni`: 19 turned lines on both legs — its axis was already turned. What moves is
  the wrap of its **category labels**, which r105 §3.1 recorded as the reference's own change
  too ("category labels reflow one line to two"), and the ink and the MAJOR page follow.

---

## 5. Confinement, measured in the three tracks

`dotnet/CLAUDE.md`'s warning is that a change in `XRay.Office.Text` line breaking reaches every
format. **This change is not in `XRay.Office.Text` line breaking**: `ILineBreaker.cs` loses a
declaration and gains a comment, and `LineBreaker`, `ParagraphLayouter`, `CellBreaks` and
`MeasuredParagraph` are untouched.

| track | corpus | off leg | on leg | of which chart-bearing | moved |
|---|---:|---:|---:|---:|---:|
| words | 338 | 338 | **10** | 10 of 10 | **0** |
| slides | 302 | 302 | 251 | 67 of 67 | 2 |
| sheets | 307 | 307 | **307** | 92 of 92 | 1 |
| total | 947 | 947 | 568 | 169 of 169 | 3 |

The words column is thin on purpose and is the round's weakest measurement: only that track's
**ten** chart-bearing documents were scored on the on leg, because the box was carrying three
other sessions. The whole sheets track was scored and moved one document.

and the ten non-fidelity test projects are green with **0 skipped**:

| project | passed |
|---|---:|
| Containers | 109 |
| Core | 560 |
| Markup | 259 |
| OpenDocument | 160 |
| Presentations | 1081 |
| Rendering | 164 |
| Spreadsheets | 1301 |
| Text | 728 |
| Vector | 309 |
| WordProcessing | 1938 |
| **total** | **6609**, 0 failed, 0 skipped |

`XRay.Office.Fidelity.Tests` is at the briefed baseline, read out of this run's own output rather
than asserted: **`Failed: 10, Passed: 542, Skipped: 0, Total: 552`**, run twice against
`/opt/libreoffice26.2/program/soffice` with the same result, and the ten are exactly the known
names — `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` on `paginated.doc`,
`.docx`, `.fodt` and `.rtf`; `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` on
`list-label-overrun.doc`, `.docx`, `.fodt` and `.odt`;
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)`; and
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)`.
There is no eleventh.

---

## 6. The licence position, and how the other three languages are supplied

**Vendored**: `hyph_en_US.dic` (106,414 bytes, sha256 `486fb6840b1049d5…`) and
`README_hyph_en_US.txt` (1,888 bytes, sha256 `9bcae072bff45cc0…`), copied verbatim from
`/opt/libreoffice26.2/share/extensions/dict-en/`. The `.dic` carries **no** copyright comment of
its own — five keyword lines then bare patterns — so the notice exists only in the README, and
the two travel together in the source tree, in the output directory and in the NuGet package
(`contentFiles/any/any/hyphenation`). The terms are BSD-style: *"Unlimited copying,
redistribution and modification of this file is permitted with this copyright and license
information."* The licence text is **copied, not paraphrased**.

**Not vendored**: `hyph_fr.dic` (LGPL-2.1-or-later) and `hyph_es.dic`
(GPL-3.0+/LGPL-3.0+/MPL-1.1+ tri-licence), and `hyph_en_GB.dic`, which the corpus does not need
(r105 §1.3: retagging every chart run `en-GB` moves three documents by one hyphen each and **0**
arrangements).

**Three supported ways to add one**, consulted in this order:

1. `Hyphenators.Register(language, stream)` or `Register(language, IHyphenator)` — nothing on
   disk. `HyphenationTests.APatternFileSuppliedAsAStreamAnswersForItsLanguage`.
2. `XRAY_OFFICE_HYPHEN_DICTS`, a `Path.PathSeparator`-separated list of directories. Each is
   searched for `hyph_<tag>.dic` **and then one level down**, so pointing at a LibreOffice
   installation's `share/extensions` finds `dict-fr/hyph_fr.dic` without naming it.
   `HyphenationTests.ALibreOfficeInstallationsFrenchAndSpanishAreReachableThroughTheSearchPath`.
3. The shipped `hyphenation` folder beside the assembly.

`XRAY_OFFICE_HYPHEN_DICTS=0` (or `false`, `no`) turns hyphenation off entirely, which is the state
a deployment without the data is in — and is what makes the two-leg sweep of §3.1 possible.

**The tests that need a dictionary this tree does not ship skip cleanly**, with
`Assert.SkipUnless` on the file's existence, so a CI machine without LibreOffice reports them
skipped rather than failed. The `en_US` tests never skip, because that file is in the tree — the
Core project reports **0 skipped** on this machine, where LibreOffice is present and the French
and Spanish cases therefore run.

**The renderer's default behaviour does not depend on a LibreOffice installation.** Nothing
looks in `/opt` or `/usr/share/hyphen` of its own accord; English ships and everything else is
opt-in.

---

## 7. O43 — not taken, and this round adds two witnesses to it

The brief offered O43 (the plain width/collision rotation limit) if the main seat landed
cleanly. It is **not taken** — the main seat consumed the round — but §2's grid is new evidence
for whoever does:

- `Screeched` and `Squelched` are **two authored fixtures for O43 that need no corpus document**:
  the reference turns both **with its hyphenator switched off**, they hold no hyphenation point
  in any of the four installed files, and we leave both upright. Their near neighbours
  `Scratched`, `Stretched`, `Strengths` and `Straights` — the same length, the same shape — are
  upright at the reference too, so the boundary is somewhere between them and it is a width.
- The reference's off column of `word-grid.tsv` is a **22-point width ladder** measured on one
  chart at one pitch, which is what r91 §4 said the seat needs and did not have.
- `A Efficiency` in `leading-sweep.tsv` is a third: the reference turns it and we leave it
  upright, and the label is within 2 % of the limit, so it is the same boundary seen from the
  other side.

`027_Simple_personal_cash_flow` (8 turned reference lines, 0 ours) is unchanged by this round, as
r105 predicted.

---

## 8. Seat state

**O10 is fixed in this tree, with a corrected reach.** Implemented, mechanism established twice
(the four hyphens the reference itself drew, §1.2; the 22-fixture decision grid against the
reference with and without its own dictionary, §2), tests that fail at the base and pass after
(`ChartAxisHyphenationTests` — `WithNoDictionaryTheLabelsWrapUprightOntoTwoLines` is the base
behaviour and `WithTheShippedEnglishPatternsTheSameLabelsAreTurned` is this change), and a
measured corpus reach:

- **3 renderings of 568 scored**, all chart-bearing, **all three better against 26.2.4.2** and
  **none worse**;
- **0 gate rows** — and the brief's expectation that `038` would close is refuted with the
  reason (§4.1);
- **0 of 399 non-chart documents touched**, with the call graph making that structural.

**What is left, stated so it is not mistaken for closed:**

- **`Service`**, one word of 22, where we hyphenate and 26.2.4.2 does not, cause not
  established (§2.4). Corpus reach zero.
- **379 of the 947 corpus documents were not scored on the on leg**, all non-chart, because the
  box was at load 15. The claim they cannot move rests on the call graph plus 399 that were
  scored, not on all 778.
- **The character-count leading limit (§2.3) is not separated from the width form by any
  measurement in this round.** Both give 19 of 22. It is kept because the source says so and
  because it is the more conservative.
- **O43** keeps its seat and gains three authored witnesses (§7).
- **No page was read by eye this round.** Every claim above is a number out of a PDF; there is
  no blind reader in this container and none of these findings needed one.
