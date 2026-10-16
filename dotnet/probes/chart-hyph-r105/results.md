# O10 — the hyphenator is the trigger, it is confirmed by switching the dictionary off, and its whole corpus reach is two documents

Round 105, measured 2026-09-11 against `/opt/libreoffice26.2/program/soffice`
(**LibreOffice 26.2.4.2**, tarball install; DejaVu present). Follows
`probes/chart-axisrot-r91`, whose §3 mechanism this round **confirms by a different and
stronger instrument** and whose reach section it **replaces**.

**This is a step-1 round and no `dotnet/src` file changed.** The brief asked for the data
question first and said to stop if the answer is that a fix needs vendored dictionary data.
**It does**, and the packet for that decision is §4. Everything below is measurement.

---

## 0. What this round settles

| question | answer |
|---|---|
| Is the trigger really hyphenation, not width? | **Yes** — proved by turning the hyphenator off with the words held fixed (§1), with a control at its null (§1.3) |
| What languages does the corpus need? | **One: `en_US`.** 102 of 103 category-axis documents carry pure-ASCII English labels; the one exception is French and is not affected (§2) |
| How many renderings does the seat move? | **2** documents visually (`038`, `033`); **1** gate row (`038`), where it accounts for the delta exactly (§3) |
| Can it be done without vendoring? | **No.** No `hyph_*.dic` exists on this machine outside the LibreOffice tarball, and the renderer may not depend on one (§4) |
| Licence of what would be vendored | `hyph_en_US.dic`, 106,414 bytes, **BSD-style / TeX-derived, unlimited copying and modification with the notice** (§4.2) |

---

## 1. The trigger, re-derived without assuming r91's answer

The brief said to treat the register's sentence as a hypothesis, because an earlier brief on
this seat named `TextBreak` and was refuted. r91 established hyphenation by *swapping words* —
`Cost Efficiency` turns, `Cost Stretched` (wider) wraps. That is a good experiment but it
changes the characters, so a critic can always propose some other property of the new word.

**This round holds every character fixed and varies only the dictionary.**

### 1.1 The instrument

26.2.4.2 ships hyphenation patterns for exactly four locales and no more, all under
`/opt/libreoffice26.2/share/extensions/`:

    dict-en/hyph_en_US.dic   dict-en/hyph_en_GB.dic   dict-fr/hyph_fr.dic   dict-es/hyph_es.dic

`find / -name 'hyph*.dic'` returns those four and nothing else — there is **no system
`hyphen-*` package on this machine**. So tagging a text run with a Western locale that is not
one of those four switches 26.2.4.2's hyphenator off for that run, and changes nothing else:
not a glyph, not an advance, not a line count, not the font.

`language-sweep.py` writes 16 variants of the `038` witness — all five categories set to one
label, `{Cost Efficiency, Cost Stretched}` × eight `a:defRPr @lang` values. `language-sweep.txt`
is the classified result:

| all five categories = `Cost Efficiency`, `@lang` = | dictionary installed | 26.2.4.2 draws |
|---|---|---|
| *(absent — the document's own state)* | falls back | **turns 45°, outlined** |
| `en-US` | yes | **turns 45°, outlined** |
| `en-GB` | yes | **turns 45°, outlined** |
| `fr-FR` | yes | **turns 45°, outlined** |
| `es-ES` | yes | **turns 45°, outlined** |
| `de-DE` | **no** | upright, wrapped `Cost ` / `Efficiency` |
| `ru-RU` | **no** | upright, wrapped `Cost ` / `Efficiency` |
| `zxx` | **no** | upright, wrapped `Cost ` / `Efficiency` |

and the negative control, `Cost Stretched` — no hyphenation point in any of the four
dictionaries — is **upright and wrapped in all eight**.

**The axis arrangement is a function of whether a hyphenation dictionary exists for the run's
locale.** No width, character or line-count difference exists across that first block to
explain it. r91's §3 stands and is now established twice, by two instruments that share no
assumption.

The mechanism in the C++ tree (**this tree is 27.2 alpha, not 26.2.4.2 — confound C8; the
measurement above is of the actual binary and does not depend on these lines**):

- `PropertyMapper::getTextLabelMultiPropertyLists` sets `ParaIsHyphenation` true, inside the
  same `if (nLimitedSpace > 0)` as `TextMaximumFrameWidth` and nowhere else —
  `chart2/source/view/main/PropertyMapper.cxx`:550-556.
- `DrawModelWrapper` installs `LinguMgr::GetHyphenator()` on the chart's drawing outliner —
  `chart2/source/view/main/DrawModelWrapper.cxx`:72-79.
- `lcl_hasWordBreak` returns true when a laid-out line's start is not a word start —
  `chart2/source/view/axes/VCartesianAxis.cxx`:369-404 — and at :888-905 that clears
  `m_bLineBreakAllowed`, drops the shapes and restarts the axis.

`LinguMgr::GetHyphenator()` is what makes the pattern file load-bearing: with no pattern file
for the locale, `lcl_hasWordBreak` never sees a mid-word line start and the restart at :901
never fires. That is exactly the `de-DE`/`ru-RU`/`zxx` row of the table.

### 1.2 The reference draws the hyphen, visibly, in chart text

Three corpus documents show 26.2.4.2's chart hyphenation directly in the text layer — base
render on the left, the same document with its chart runs retagged `de-DE` on the right:

| document | with the hyphenator | with it off |
|---|---|---|
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | `Risk Management & Gov-` / `ernance` | `Risk Management &` / `Governance` |
| `Demick_JetBlue.pptx` | `Fuel Costs, Operating Costs, and Fuel Con-` / `sumption ` | `… and Fuel ` / `Consumption ` |
| `southern-classic-kennesaw-state-university-final.pptx` | `Pes-` / `simistic`, `Total Mainline Pas-` / `senger Revenue` | `Pessimisti` / `c`, `Total Mainline ` / `Passenger Revenue` |

### 1.3 The control, and its base rate (confound C9)

The retag itself must not be what moves things. `control-en-GB.tsv` reruns all 103
category-axis documents retagged **`en-GB`** — a locale whose dictionary *is* installed:

- **3 of 103** documents change at all, and all three are a hyphen landing one letter
  differently (`en_GB` against `en_US` patterns);
- **0 of 103** change the axis arrangement.

So the base rate for "retagging every chart run flips an axis" is **0/103**, against 2/103 for
the `de-DE` retag. The difference is the dictionary, not the edit.

---

## 2. The language census — the corpus needs one dictionary

`chart-lang-census.py` walks all 766 OOXML containers in `/home/user/sample-files`, opens every
`*/charts/chart*.xml`, and records the cached category strings and every `@lang` stated in the
part and in the `c:catAx` subtree. `axis-language-census.tsv`, 103 rows.

- **169** containers hold a chart part; **103** state a `c:catAx`.
- `@lang` over those 103 documents' chart parts: `en-US` 175, `en-GB` 11, `ja-JP` 6, `en-ZA` 5,
  `fr-FR` 3. Per document: **71 `en-US` only**, **23 state no `@lang` at all**, 5 `en-US`+`ja-JP`,
  2 `en-GB`+`en-US`, 1 `en-US`+`en-ZA`, 1 `en-US`+`fr-FR`.
- **The `ja-JP` tags are an authoring artefact, not the label language**: all five documents are
  `slides/chartset-001/pptx/*`, and their labels are `January`, `February`, `Q1`, `Q4`.
- **1 document of 103 has a non-ASCII category label**:
  `slides/ceiling-002/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`
  (`Thérapeutique`, `Examens complémentaires`), and it states `fr-FR`. It is **not** in the
  affected set of §3 — its axis does not move when the hyphenator is switched off.

**So the corpus's whole need is `en_US`.** `hyph_en_GB.dic` would change three documents by one
letter each and no arrangement (§1.3); `hyph_fr.dic` and `hyph_es.dic` are needed by nothing.

Two things this census does not reach, stated so they are not mistaken for zero:

- **`23 of 103 documents state no language at all`**, `038` among them — its `c:catAx/c:txPr`
  carries `<a:defRPr sz="1100" …>` with no `lang`. The variant `eff_none` in §1.1 turns, so
  26.2.4.2 is hyphenating those with its **default document locale**, not with anything the
  document says. Any implementation has to pick that default itself, and English (USA) is what
  26.2.4.2 picks here.
- **181 legacy containers** (66 `.doc`, 64 `.xls`, 51 `.ppt`) carry their charts as BIFF/Escher
  records, not XML, so the retag instrument cannot reach them. §3.2 bounds them another way.

---

## 3. Reach — 2 documents, and 1 gate row

### 3.1 Switch the hyphenator off across the whole chart-bearing corpus

`delang.py` rewrites every `a:defRPr`/`a:rPr`/`a:endParaRPr` `@lang` inside every chart part to
`de-DE` and changes nothing else. Both sides of all 169 chart-bearing OOXML containers were
rendered with 26.2.4.2 (338 renders); `hyphenator-off.tsv`.

**8 of 169 documents render differently when 26.2.4.2's hyphenator is switched off.**
7 of the 103 with a `c:catAx`, 1 of the 66 without.

| document | what changes | gate r83 verdict |
|---|---|---|
| `slides/chartset-008/pptx/038_Competitive_Advantage_Card_…` | **axis arrangement**: turned+outlined (152 glyph-sized fills) → upright two-line labels (24) | **words** (fails) |
| `sheets/chartset-013/xlsx/033_Event_planning_tracker_…` | **axis arrangement**: 12 labels turned 45° → 0, wrapped upright | **words** (fails) |
| `sheets/chartset-014/xlsx/027_Simple_personal_cash_flow_…` | category column reflows (`Savings/Investment` → two lines), whole plot shifts; the 8 turned `$` labels are unmoved | match |
| `sheets/done-010/xlsx/Keywords_Mapping_Graphs_and_Charts.xlsx` | one hyphen (§1.2) | match |
| `slides/ceiling-001/pptx/N2_E_Maestroni_Swarm_COP.pptx` | category labels reflow one line to two | match |
| `slides/ceiling-002/pptx/Demick_JetBlue.pptx` | chart title hyphen (§1.2) | words (fails, for other reasons) |
| `slides/chart-001/pptx/southern-classic-kennesaw-…pptx` | two hyphens (§1.2); its 38 turned labels are unmoved | match |
| `slides/done-011/pptx/3495.pptx` | (no `c:catAx`) chart text reflows | match |

**Only two of the eight change the rotate-or-wrap decision.**

### 3.2 The gate arithmetic, which closes the attribution on `038` exactly

Column 9 of `/home/user/gate-orig-r83/rows.tsv` is `glyphs`, alphanumeric characters,
ours/reference:

| document | ours | reference | band `max(2%,15)` | ref **with** hyphenation | ref **without** |
|---|---:|---:|---:|---:|---:|
| `038_Competitive_Advantage_Card` | **1585** | 1449 | 28.98 | 1449 | **1585** |
| `033_Event_planning_tracker` | 2720 | 2650 | 53.0 | 2649 | 2649 |

- **`038`: our rendering reproduces 26.2.4.2's own output with its hyphenator switched off,
  to the character — 1585 against 1585.** The reference-with-hyphenation column reproduces the
  gate's reference column exactly too (1449 against 1449), so the two extractors agree on this
  document and the identity is real, not an artefact. **The whole +136 gate delta on `038` is
  this seat.** Its no-dictionary render draws `Product ` / `Quality`, `Brand ` / `Reputation`,
  `Cost ` / `Efficiency`, `Customer ` / `Service` upright on two lines — which is what this tree
  draws (r90's `axis-pair.png`, r91's `pair-038-axis.png`).
- **`033`: the axis flips but the glyph count does not** (2649 both ways; the gate's 2650 is the
  one-character disagreement between `pdftotext` and `pymupdf`). Turning a Calc chart's labels
  leaves them as text, so no alphanumeric is gained or lost. Its +70 gate delta is therefore
  **not** this seat, even though its axis genuinely differs. `033` is a visual defect here and
  its gate row is somebody else's.

**So: visual reach 2 documents of 947; gate-verdict reach 1 document of 947, and on that one
the seat accounts for the delta exactly.**

### 3.3 The whole 45° phenomenon, so the seat is not confused with it

`ref45.py` counts, over all 948 banked reference renderings, every text line turned to exactly
44.8–45.2° — chart2's automatic angle, separable from any angle an author states.
`ref-45deg-census.tsv`.

- **13 of 947** documents have the reference drawing text at 45°; 19 more draw turned text at
  some other angle, all stated (30.0°, 29.9°, 23.0° …). Base rate of *any* 45° text: **1.37 %**.
- We already match the turned-line count on **9 of those 13**.
- We differ on **4**: `055_Project_timeline` (0 against 38), `033_Event_planning_tracker`
  (0 against 12), `027_Simple_personal_cash_flow` (0 against 8), `064_Small_business_cash_flow`
  (32 against 13, the other direction), **plus** `038`, which is turned *and outlined* so it has
  no 45° text at all. Five rotation defects in the corpus.
- **Three of those five are not hyphenation.** §3.1's sweep leaves `055`, `027` and `064`'s
  arrangements untouched with the hyphenator off, and their labels have no hyphenation point to
  find: `5 Apr`, `5 May`, `$0`, `$2,000`, `Jan-xx`. They need the plain width/collision limit,
  which is r91 §4's unresolved residue and a **different seat**.

### 3.4 What r91's reach section got wrong, corrected here

r91 §5 offered "the 27 fill-excess candidates are a superset that contains the witness".
Screened against the axis census, **1 of those 27 documents states a `c:catAx` at all** — the
witness — against a base rate of 103/947 = **10.9 %**. At 3.7 % that screen is *below* its own
null: it selects nothing, and "27 to screen" should not be carried forward. Likewise r91's five
"reference turns text, we turn none": 3 state a `c:catAx` (60 % against the 10.9 % base, so that
screen *is* enriched), and of those three only `033` is hyphenation.

---

## 4. The vendoring decision — the packet, not the decision

### 4.1 Why runtime resolution does not answer it

`find / -name 'hyph*.dic'` returns **only the four files inside the LibreOffice 26.2 tarball**.
There is no `/usr/share/hyphen`, no `/usr/share/myspell`, no `hyphen-en-us` package. So a build
that resolves patterns at runtime from a system path finds nothing on this machine, and the
brief forbids depending on a LibreOffice installation — the tree's premise is a pure C# library
set. A runtime-discovery design would therefore have to fall back to "no hyphenation", which is
precisely today's behaviour and gets `038` wrong.

The seam already exists and is unimplemented: `IHyphenator.FindHyphenationPoints(word, language)`
is declared in `dotnet/src/XRay.Office.Text/Layout/ILineBreaker.cs`:45-53 with **no implementation
and no call site**, and its own remark already records that "LibreOffice uses Hunspell's, so
matching its line breaks requires the same dictionaries."

There is precedent for shipping third-party data here: `dotnet/src/XRay.Office.Text/Fonts/Bundled`
is 11 MB of font faces. `dotnet/CLAUDE.md`:729 states the rule that governs it — *ship only the
faces the distro packages ship*. Its analogue for hyphenation is not settled by this round and
is part of what is being handed over: a stock Debian/Ubuntu LibreOffice pulls `hyphen-en-us` as
a recommendation, so a machine with the distro packages usually *does* hyphenate; this container
does not have it.

### 4.2 Exactly what would have to be vendored, and under what terms

**One file** satisfies the whole measured corpus (§2):

| file | bytes | sha256 (first 16) | licence, read out of the file and the README beside it |
|---|---:|---|---|
| `/opt/libreoffice26.2/share/extensions/dict-en/hyph_en_US.dic` | 106,414 | `486fb6840b1049d5` | **BSD-style.** `README_hyph_en_US.txt`: *"Unlimited copying, redistribution and modification of this file is permitted with this copyright and license information."* Derived from plain TeX `hyphen.tex` (*"Unlimited copying and redistribution … permitted as long as this file is not modified. Modifications are permitted, but only if the resulting file is not named hyphen.tex"*) and the TUGboat exception log (*"Copyright 2007 TeX Users Group. You may freely use, modify and/or distribute this file."*). Conversion by László Németh. |

The file itself carries **no** copyright comment — 11,130 lines, five keyword lines then bare
patterns — so the notice lives only in `README_hyph_en_US.txt` (1,888 bytes) and that README
would have to be vendored with it.

The other three, for the record, since they are not all the same and the brief asked:

| file | bytes | sha256 (first 16) | licence |
|---|---:|---|---|
| `dict-en/hyph_en_GB.dic` | 107,031 | `9fdc97f7faabcda3` | BSD-style header; underlying `ukhyphen.tex` permits redistribution but forbids a modified copy being named `UKHYPH*`, and notes the OUP word list it was patgen'd from is not redistributable (the *patterns* are). |
| `dict-fr/hyph_fr.dic` | 23,631 | `476ca60b958400c8` | **GNU LGPL 2.1 or later** for the LibreOffice adaptation; underlying `hyph-fr.tex` MIT since 2016-03-20. |
| `dict-es/hyph_es.dic` | 56,284 | `a95be7365a482b93` | **Triple disjunctive: GPL-3.0+, LGPL-3.0+, or MPL-1.1+**, your choice; underlying TeX file MIT (Javier Bezos / CervanTeX), and the MIT notice is embedded in the `.dic` itself as `%` comments. |

**Nothing was copied into this repository.** Only paths, sizes, hashes and terms are recorded.

**The decision to make**: vendor `hyph_en_US.dic` + `README_hyph_en_US.txt` (108 KB, permissive,
one-language) into the tree, or leave the seat open. The en-US case is about as permissive as
third-party data gets — but it is still third-party data entering the repository, and the brief
is right that it is not a rendering fix's call to make.

### 4.3 What step 2 would cost once that is answered

Small and well specified, and none of it is the hard part:

- **Liang's algorithm over a Hunspell pattern file** — encoding line, `LEFTHYPHENMIN 2`,
  `RIGHTHYPHENMIN 3`, `COMPOUND*` (unused for a single word), then patterns with interleaved
  digits; `NEXTLEVEL` sections exist in `hyph_en_GB`/`hyph_fr` and would need handling if either
  is ever shipped, but **not** for `hyph_en_US`, which has none. Behind the existing
  `IHyphenator`.
- **A `no dictionary` path that is the tested default**, because that is what a deployment
  without the data hits — and it is today's behaviour, so its expected output is already banked.
- **The axis wiring**: in `ChartAxisLabels`, a line whose start is a hyphenation point counts as
  a mid-word break for the restart, mirroring `lcl_hasWordBreak`.

**Two cautions that outlast this round:**

1. **A wrong hyphenation turns axes the reference wraps.** §3.1 measures the blast radius from
   the other side: switching 26.2.4.2's hyphenator off moves **8 of 169** chart-bearing
   documents, of which 6 are inside the gate band today. A hyphenator that is *approximately*
   right would move some of those 6 out of band while fixing `038`. The safe test before landing
   is `hyphenator-off.tsv` run the other way — our own renders, hyphenator on, against the
   reference's — over the same 169.
2. **The limit is still wrong** (r91 §4): the width limit that applies to a line *inside* a
   wrapped label measures smaller than the 0.95-of-pitch limit that applies to a single-word
   label, and r91's `drop3` shows it is not a function of the pitch alone. Landing hyphenation
   against that limit fits two errors against each other. §3.3 gives that seat its own reach —
   `055`, `027`, `064`, three documents, none of them hyphenation.

---

## 5. Seat state

**O10 stays open, and it is now sized.** It is neither of the two terminal states: not fixed in
this tree, and not nil reach. What is left is one decision (§4.2) and one small implementation
(§4.3), and the reach it would buy is:

- **1 gate row** — `038`, whose column-9 delta of +136 this seat accounts for **exactly**
  (1585 → 1449);
- **2 renderings** visually — `038` and `033`;
- **8 chart-bearing documents of 169** touched by chart hyphenation at all, six of them already
  within the gate band, which is the regression surface rather than the prize.

**Not this seat, and re-seated by §3.3**: the plain width/collision rotation limit —
`055_Project_timeline` (38 turned lines we do not draw), `027_Simple_personal_cash_flow` (8),
`064_Small_business_cash_flow` (we turn 32 where the reference turns 13). Their labels — `5 Apr`,
`$2,000`, `Jan-xx` — hold no hyphenation point in any of the four installed dictionaries, and
their renderings are byte-identical with the hyphenator switched off.

## 6. What was not established

- **The 181 legacy containers are unmeasured by the retag instrument.** The 45° census of §3.3
  covers them (it reads the reference's own PDFs, all 947), and it finds exactly one legacy
  document drawing 45° text — `Template Pilot Logbook JAR-FCL V3.0.xls`, 70 lines against our 69,
  which is not a rotation defect. So the legacy remainder is bounded, but it is bounded by the
  *turned-text* screen alone: a legacy chart that the reference turned **and outlined**, as it
  did `038`, would leave no 45° text and would not be counted. r91's outlining screen is at its
  null (§3.4) and I have no better one.
- **Which default locale 26.2.4.2 uses for a chart run that states none.** The measurement says
  it hyphenates (`eff_none` turns) and that English (USA) is consistent with it; I did not
  isolate the setting that supplies it, and a deployment with a different default would differ.
- **No page was read by eye this round.** Every claim above is a number out of a PDF. Nothing
  needed vision and no blind reader exists in this container.
