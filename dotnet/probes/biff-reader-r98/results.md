# Round 98 — the BIFF side of the sheets track: the `CF` rules, the `TXO` runs, and a witness two rounds missed

    ours   = XRay.Office.Cli @ ac7745deb (base) and @ ac7745deb + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 0229ac93fcf0…
             plus the banked reference PDFs at /home/user/gate-orig-r83/ref (the 947 originals)
    corpus = /home/user/sample-files — the 64 `.xls`, of which 63 open as OLE2
    rule   = column 9, `glyphs`, within max(2 %, 15); `gate-columns.py` applies it to one
             directory of our renderings against the banked reference
    fonts  = the five tarball confounds moved aside as `dotnet/CLAUDE.md` records; none moved
             this round
    date   = 2026-09-11

## 0. The headline

| seat | what it was | what it is |
|---|---|---|
| **O25** BIFF `CF` rules | unread; the reference paints 43 conditions in 4 `.xls` | **fixed** — and the census was short: **187 conditions in 5 of 64**, because round 95's `CONDFMT` walk missed `EHEST-Pre-departure-checklist` entirely |
| **O18** `Background_Declaration_Template` 136.07 | *a candidate cause, not a demonstrated one* | **demonstrated and closed**: **136.07 → 0.07**, MAJOR pages **15 → 1**. The fills are the whole residual |
| **O19** BIFF `TXO` runs | unread; 155 boxes in 17 `.xls` | **fixed** — and the reach is **9 documents, not 17**: the other 8 hold only cell comments, whose text never goes through the shape path |

Corpus reach over the 64 `.xls`, rendered at the base and at head under `SOURCE_DATE_EPOCH`, one
output directory per document: **12 renderings move and 52 are byte-identical.** Summed unsigned
ink over the 64 goes **463.84 → 320.96** and MAJOR pages **137 → 120**. **No gate verdict moves**
and no page count moves anywhere — a fill and a font size add no alphanumeric character. The
`.xlsx`/`.xlsm` half of the corpus, which the shared-predicate refactor passes through, is
**243 of 243 byte-identical**.

And the standing rule held again. **`EHEST-Pre-departure-checklist` states 120 `CONDFMT` and 144
`CF` — more than the other four witnesses together — and reading its rules moves not one byte of
its rendering**, because 126 of the 144 are `cellIs equal 2` over a blank checklist whose cells
hold 0 or nothing. (It does move, by 8.89 → 6.88, but that is O19's text boxes and not its 144
rules.) §2.3 is the control that separates "the reader found no match" from "the reader read
nothing", and it is a patched rule rather than an argument.

## 1. The census round 95 took, and the fifth document in it

Round 95 walked every worksheet substream of the 64 `.xls` for `CONDFMT` and reported **4 of 64,
29 records, 35 ranges**. Round 97 inherited that witness list when it asked the reference what it
paints, and reported **29 formats / 43 conditions / 43 inked**.

`cf-census.py` walks the same streams for `CONDFMT`, `CF` *and* `CF12`:

| document | `CONDFMT` | `CF` | `CF12` | cells under a `CONDFMT` range |
|---|---:|---:|---:|---:|
| `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` | **120** | **144** | 39 | 1676 |
| `Background_Declaration_Template.xls` | 23 | 26 | 0 | 1972 |
| `NPA_21_21_Sentenced_Comments.xls` | 3 | 9 | 3 | 65535 |
| `TICAPCapability_Final.xls` | 2 | 6 | 0 | 37 |
| `Hazard Analysis Template.xls` | 1 | 2 | 0 | 48 |
| | **149** | **187** | **42** | **69268** |

`ref-census.py` is round 97's script with the fifth witness added, and 26.2.4.2's own
`--convert-to fods` agrees record for record:

| document | `calcext:conditional-format` | conditions | named styles | of those, carrying ink |
|---|---:|---:|---:|---:|
| `EHEST-Pre-departure-checklist` | 120 | 144 | 144 | **132** |
| `Background_Declaration_Template` | 23 | 26 | 26 | **26** |
| `NPA_21_21_Sentenced_Comments` | 3 | 9 | 9 | **9** |
| `TICAPCapability_Final` | 2 | 6 | 6 | **6** |
| `Hazard Analysis Template` | 1 | 2 | 2 | **2** |
| | **149** | **187** | **187** | **175** |

So **5 of 64, not 4**, and **187 conditions, not 43**. The register's O25 entry and round 97's
write-up both carry the smaller figure; it is the right shape and the wrong size.

*The 64th `.xls` is `Special-Procedures_2025-07-10.xls`, which opens `PK\x03\x04` and is read by
the SpreadsheetML path. That note from round 95 stands.*

**`CF12` is not read, and neither does the reference read it.** Excel 2007 writes the extended
rule families into a `CF12` record (`0x087A`) beside the `CF` it also writes for compatibility;
`ImportExcel8::Read` has no case for `EXC_ID_CF12`, so 26.2.4.2 sees only the `CF`. The corpus's
42 `CF12` are matched one for one by a `CF` that is read.

## 2. O25 — what a `CF` record holds, and what the corpus states

### 2.1 The record

`XclImpCondFormat::ReadCF` (`sc/source/filter/excel/xicontent.cxx`:526-713 **in this tree**,
which is 27.2.0.0.alpha0+ and not the reference binary's source; every arm below is confirmed a
second time against 26.2.4.2's own output in §2.2 and §5). A type byte, a comparison byte, two
formula lengths, a flag word naming the format blocks, two ignored bytes, then the blocks in a
fixed order and the two RPN formulas last.

Four things in it are not what the field names suggest, and all four are load-bearing here.

**The anchor is the *first* range of the list, not its top-left corner.** `ReadCF` takes
`maRanges.front().aStart` (`:657`) and hands it to every `ScCondFormatEntry` as the position the
token array was written for. That is a different rule from the OOXML path, where
`ScRangeList::GetTopLeftCorner` picks the smallest range start under `ScAddress`'s
`(tab, col, row)` ordering — and the corpus discriminates: `EHEST`'s twelve-range block over
`E46:E57 H46:H57 … E10:E30 H10:H30` has first range `E46` and top-left corner `E1`, and
26.2.4.2's own `calcext:base-cell-address` for it is **`E46`**.

**The area block's "used" flags are inverted.** `XclImpCellArea::FillFromCF8`
(`xistyle.cxx`:1073-1091) reads `mbForeUsed = !get_flag(nFlags, EXC_CF_AREA_FGCOLOR)` — a *set*
bit means the rule leaves that part of the cell alone — and then applies two corrections, of
which the first is what every corpus rule relies on: a stated background with no pattern, or a
solid one, **becomes a solid fill of the background colour**.

**The font block's six fields have four different sentinels.** 118 bytes
(`XclImpFont::ReadCFFontBlock`): the height and the colour are unused above `0x7FFF`, the weight
and the posture share `EXC_CF_FONT_STYLE` in the first flag dword, the strikeout has its own bit
in the *same* dword, and the underline is governed by a bit in the **third**. And
`EXC_CF_FONT_STYLE` means opposite things in two words — in `nFontFlags1` it says the rule does
not change the posture, and in `nStyle` it *is* the italic flag.

**An unreadable mode does not consume the block's condition index.** `ReadCF` returns before
`++mnCondIndex` for a type it does not recognise, so the next `CF` takes that one's place.
`XlsConditionalFormats.ReadCf` keeps a null entry for a rule it reads but cannot evaluate and
returns early — without an entry — for one it cannot decode at all, which is the same shape.

### 2.2 The formulas, which are a much smaller language than a cell formula's

Every `CF` formula in the corpus, decoded (`cf-census.tsv`, column 9):

| shape | rules |
|---|---:|
| `cellIs` against a `tInt` | 129 |
| `cellIs` against a `tStr` | 16 |
| `cellIs between` two `tInt` | 2 |
| `tRefV` `>` `tRefV` (Direct) | 18 |
| `tRefN` + `NOT` (Direct) | 13 |
| a reference compared with a reference or a string (Direct) | 4 |
| a bare `tStr` or `tRefN` (Direct) | 5 |
| | **187** |

**187 of 187 are inside the grammar `XlsConditionFormula` implements**, and nothing outside it is
guessed at: a token it does not know answers null and the rule paints nothing, which is the
reference's own answer to a formula it cannot convert (`xicontent.cxx`:667-671).

Two token details each cost a wrong rendering if read the obvious way:

- **The class fold is `(opcode & 0x1F) | 0x20`, not `opcode & 0x3F`.** A reference token's class
  lives in bits 5 and 6, so `tRefV` is `0x44` and `0x44 & 0x3F` is `0x04` — not `0x24`. Every
  value-class reference in the corpus is written this way and the naive fold rejects all of them.
  *`XlsNameRanges.Read` makes the naive fold and is left alone, on a census rather than a guess:
  all 111 built-in `NAME` formulas over the 64 `.xls` open with the reference-class `0x3B` and
  none holds a value- or array-class token (§6), so no corpus document reaches its broken arm, and
  changing it was not this round's seat.*
- **`tRefN`'s column offset is a signed *byte*, and its row a signed 16-bit.** The address form
  `tRef` carries a fourteen-bit column; the offset form does not.
  `Background_Declaration_Template`'s `IPR` sheet states `([.M10]<[.K10])` as two `tRefN` with
  column offsets `0x0000` and `0x00FE`, and `0xFE` here is **−2**, not 254.

### 2.3 The control: `EHEST` reads and matches nothing, and that is a fact about the corpus

`EHEST` is 144 of the 187 conditions and its rendering does not move by a byte. Two explanations
fit — the reader never sees the records, or it sees them and no cell satisfies a rule — and "no
cell matches" is exactly the sentence a reader that read nothing would also produce. Round 95's
retracted leg was this shape.

`patch-cf.py` is the discriminator, and it is a one-attribute variant on the **rule** rather than
on a cell, so neither side's recalculation enters it. The first `CONDFMT` of the second worksheet
substream covers `E10:E12`; its single `CF` ends with the three bytes `1e 02 00` — `tInt 2` — and
those cells hold 0. Patched to `1e 00 00` the rule becomes `cellIs equal 0`:

| | original vs patched |
|---|---|
| 26.2.4.2 | page 7, `\|ink\|%` **0.05**, one region |
| this tree | page 7, `\|ink\|%` **0.05**, one region |

Both sides paint the same fill in the same place. The reader reaches the records; the corpus does
not satisfy them. **A rule count is not a reach figure** — N11 from round 96, arriving on the BIFF
side with 144 rules behind it.

*The 18 `[.$E$103]>[.$P$110]` rules are the same story: `E103` is 0 and `P110` is empty, and a
blank compares as zero.*

### 2.4 What the evaluation shares with the OOXML reader

`XlsxConditionalStyles`'s predicate core — the sheet's values, an operand, a comparison, a
`cellIs` and the comparison rules between two values — moved verbatim into
`Layout/SheetConditions.cs` and is aliased back into the OOXML reader at its top, so its body did
not change. Both filters build an `ScCondFormatEntry` with an `ScConditionMode`; only the
notation differs. The comparison rules are subtle enough to be worth *not* writing twice — a
blank equals zero and equals the empty string, text is compared without regard to case, a number
never equals a string — and they were measured once, in round 94.

**The move is verified as a move, not asserted as one.** Reducing both files to non-comment,
non-blank lines and comparing the base's `XlsxConditionalStyles.cs` against the head's plus
`SheetConditions.cs`, **13 base lines are absent at head** and every one of them is a declaration
this round deliberately changed: the six `private` type headers that became `internal`,
`Sheet.Read`'s signature, and the four `Reaches`/`_values` lines that became `Extend`/`Set`.
Nothing else in seven hundred lines moved. The 25 lines that are new are the six aliases, the
renamed `ReadSheet`, three call sites and the two builder methods.

**And the move is confined, measured the way a render sweep measures it.** The whole
`.xlsx`/`.xlsm` half of the corpus — **243 documents** — rendered at the base and at head under
`SOURCE_DATE_EPOCH`, one output directory per document: **243 of 243 byte-identical**
(`xlsx-confinement.tsv`). The OOXML reader's behaviour did not move by a byte, which is the only
form of that claim worth making about a refactor of 466 lines.

`ScConditionMode::Direct` is `nVal1 != 0.0` (`conditio.cxx`:1078-1079 and :1276), so a formula
whose result is a string is false however the cell is filled. That is why
`Background_Declaration_Template`'s two `formula-is("")` and `formula-is(">0")` rules — whose
whole formula is a string literal — paint nothing at 26.2.4.2, and paint nothing here.

## 3. O18 — the fills are the whole of the 136.07, and that is measured

The brief was explicit that this must not be assumed. Measured, against the banked 26.2.4.2
reference:

| | sum `\|ink\|%` | pages | MAJOR |
|---|---:|---:|---:|
| base | **136.07** | 25 | **15** |
| with the `CF` rules read | **0.07** | 25 | **1** |

The mechanism is visible in one page. On page 8, our base rendering paints **208 840 pt² of
`#FFFFCC`** — the cells' *stated* fill — where 26.2.4.2 paints the same area in **`#969696`**,
the colour the conditional format's area block names. The filled *area* already agreed to 0.02 %;
what disagreed was the colour — on fourteen of the twenty-five pages, which is fourteen of the
base's fifteen MAJOR pages. After the fix our page 8 states `#969696` over 208 840 pt² against
the reference's 208 870.

**What is left is 0.07 on one page**: page 2 carries one fill we draw and the reference does not
(0.51 % of the page) and four small displaced-glyph regions. O18 is closed; that remainder is not
worth a seat of its own and is recorded here rather than re-seated.

## 4. O19 — the `TXO` formatting runs, and a reach figure that is 9 and not 17

`ReadText` took the characters and stopped; `TextOf` built one run per line at a hardcoded ten
point in the default face. The runs are in the **second** `CONTINUE` after the record — the first
holds the characters — eight bytes each, a character index and a `FONT` index with four reserved
(`XclImpDrawing::ReadTxo`, `xiescher.cxx`:4242-4271; `XclImpString::ReadObjFormats`,
`xistring.cxx`). The last entry names the index one past the string and is a terminator.

`txo-census.py` reproduces round 92's figure exactly — **155 text boxes with text in 17 `.xls`,
522 runs, 62 boxes in 13 documents stating more than the opening run and its terminator**.

**But only 9 of the 17 move under this change, and the other 8 are not a defect.** Pairing each
`TXO` with the `ftCmo` type of the `OBJ` before it:

| object type | boxes | documents |
|---|---:|---:|
| `text` (6) and `button` (7) | 108 | 9 |
| `note` (25) | 47 | 9 |

(`TICAPCapability_Final` is in both rows, which is why the two add to 17 documents.)

Every one of the 8 that do not move under O19 holds **only** notes — `orbus_togaf_tool_csq`,
`18-02RD301_ILS_components_Master_9-13-18`, `RMP 2011-2014 and Inventory`, `AIM_OPR_LIST`,
`T0A0D0000090006XLSE`, `Template Pilot Logbook JAR-FCL V3.0`, `Background_Declaration_Template`
and `Hazard Analysis Template` — and a note's text does not go through the shape path at all.
`XclImpNoteObj` calls `SetInsertSdrObj(false)` and turns the text into a `ScPostIt` on the cell
(`xiescher.cxx`:1852-1883); this tree's `BuildNotes` is the same division. **Six of the eight are
hidden comments that neither renderer draws**, measured by searching both renderings for the first line
of every box's text: `orbus_togaf_tool_csq` 0 of 2, `18-02RD301` 0 of 16, `AIM_OPR_LIST` 0 of 1,
`T0A0D0000090006XLSE` 0 of 1, `Template Pilot Logbook` 0 of 2 and `Background_Declaration_Template`
0 of 1 on **both** sides. The other two are visible and are drawn identically by both —
`RMP 2011-2014 and Inventory` 12 of 12 and `Hazard Analysis Template` 8 of 8.

So the brief's *"the obvious place to point the `TXO` run reader is `Background_Declaration_
Template`, `orbus_togaf_tool_csq` and `TOGAF9-Tool-ConfReqts-CSQ`"* is two-thirds wrong: the
first two hold nothing but comments. **Census the object type behind a `TXO`, not the `TXO`.**

The nine that do move and what they are worth are in §5. The run array is also the one place
where a `FONT` index of `0xFFFF` appears — `orbus_togaf_tool_csq` writes it as its terminator —
and it resolves to nothing, which is right, because a terminator names a character index past the
string and never applies.

**Text before the first run takes no font at all.** `lclCreateTextObject`
(`sc/source/filter/excel/xihelper.cxx`) starts with an empty item set and sends it as soon as it
reaches the first run's index, so whatever precedes it keeps the edit engine's own default rather
than the object's or the first run's. It is modelled and the corpus cannot test it: **all 155
corpus run arrays open at character zero**, so nothing reaches the branch.

**A `TXO` with no run array is not styled either.** `XclImpTextObj::DoPreProcessSdrObj` branches
on `XclImpString::IsRich()`, which is `!maFormats.empty()` (`xistring.hxx`:56), and takes
`NbcSetText` for the plain case — no font is applied there at all.

**What is not read is the colour and the posture.** `SheetShapeRun` carries a face, a size and a
weight and nothing else, so a run whose `FONT` states italic or a colour is drawn upright in the
cell's own ink. Extending the run would reach the painter and three other readers; it is left,
and it is the honest residue of this seat.

## 5. Reach, measured rather than censused

Our half of the 64 `.xls` rendered three times — at the base, with O25 alone, and with both —
under `SOURCE_DATE_EPOCH`, one output directory per document. **12 of 64 move; 52 are
byte-identical.** (Rendered a fourth time at the commit's own binary, after the two late
defensive edits of §2.1: **64 of 64 identical to the third leg**, so the figures below are the
committed code's.)

| document | base | +O25 | +O25+O19 | MAJOR |
|---|---:|---:|---:|---|
| `Background_Declaration_Template` | 136.07 | **0.07** | 0.07 | 15 → 1 |
| `TOGAF9-Tool-ConfReqts-CSQ` | 16.76 | 16.76 | 16.76 | 4 → 4 |
| `EHEST-Pre-departure-checklist` | 8.89 | 8.89 | **6.88** | 1 → 0 |
| `TICAPCapability_Final` | 7.45 | 5.39 | **4.93** | 3 → 2 |
| `Hazard Analysis Template` | 2.19 | 2.20 | 2.20 | 0 → 0 |
| `NPA_21_21_Sentenced_Comments` | 1.95 | **0.52** | 0.52 | 2 → 1 |
| `PC1000` | 1.79 | 1.79 | 1.92 | 0 → 0 |
| `SIL_TDB609` | 1.36 | 1.36 | **1.08** | 1 → 1 |
| `apron-area` | 1.23 | 1.23 | **0.87** | 1 → 1 |
| `SIL_TDB605` | 1.18 | 1.18 | **0.94** | 1 → 1 |
| `2012-GA-Survey-Chapter-5-Tables` | 0.56 | 0.56 | **0.40** | 0 → 0 |
| `2012-GA-Survey-Chapter-6-Tables` | 0.16 | 0.16 | **0.14** | 0 → 0 |
| **the twelve** | **179.59** | **40.11** | **36.71** | **28 → 11** |
| **all 64** | **463.84** | 324.36 | **320.96** | **137 → 120** |

**Nine improve, two worsen, one is level.** `Hazard Analysis Template` rises by 0.01 and
`PC1000` by 0.13; `TOGAF9-Tool-ConfReqts-CSQ`'s bytes change and its ink does not move to two
decimals.

**`PC1000` is worth a sentence, because its ink rose while its text got *closer* to the
reference.** Counting characters by drawn size over the whole document: the base drew **0** at
11 pt and 2278 at 10 pt, the head draws **469** at 11 pt and 1809 at 10, and 26.2.4.2 draws
**472** at 11 pt and 1837 at 10. So the sizes moved from wrong to right and the summed unsigned
ink still rose 0.13, because a larger face lays down more ink wherever this tree's own position
or clip is already a little out. That is `|ink|%` behaving as round 97 §7 records — a region's
contribution is the absolute value of a signed mean — and not a regression in what is drawn.

`ink-before.tsv` is the per-document table for all 64 at the base, ranked; `ink-after-o25.tsv` and
`ink-after2.tsv` hold the movers on the two later legs. The 52 that do not move are byte-identical
and their base rows stand unchanged by construction.

**No gate verdict moves.** `gate-columns.py` applies the gate's own rule — column 9, `glyphs`,
within max(2 %, 15) — to the twelve movers against the banked reference:

- page counts: **12 of 12 identical** before and after, and every one already equal to the
  reference's;
- verdicts: **12 of 12 identical**, eleven `match` and `PC1000` `glyphs` both times;
- the largest character movement in the whole set is **3** (`EHEST` 37444 → 37447), from a shape
  whose text now sets at its own size.

Which is the argument for ranking on ink rather than on the gate: 136 points of ink went out of
one document and the column the scoreboard reads did not move by a character.

## 6. What this round could not settle

- **A `CF` number-format block is skipped rather than read.** `EXC_CF_BLOCK_NUMFMT` changes what a
  cell *says*, and this reader is the fill-and-font half. **No corpus `CF` states one** — 0 of 187
  — so implementing it would be untested code.
- **A `CF` border block is skipped, and its reach is nil — measured at the reference, not
  assumed.** 13 of the 187 rules state one: twelve in `EHEST` and one in `TICAPCapability_Final`.
  Resolving every conditional style the reference writes for the five witnesses, **exactly 12
  carry an `fo:border` and all twelve are `EHEST`'s** — the `TICAPCapability_Final` rule that
  states `font,border,area` in the file comes back from 26.2.4.2 with a background colour and
  nothing else, so its block sentinels say unused and the reference applies no border from it
  either. And all twelve of `EHEST`'s are `cellIs equal 2` over the blank checklist of §2.3, so
  **no corpus cell takes a conditional border at 26.2.4.2.** Implementing it would also need a
  conditional-border layer in `SheetFormatting`, which does not exist —
  `SetConditionalBackground` is a colour and nothing else.
- **A form control's label takes the *first* run's font for the whole label at the reference, and
  this tree applies the runs one by one.** `XclImpTbxObjBase::ConvertFont` (`xiescher.cxx`:2110-2119)
  calls `WriteFontProperties(…, rFormatRuns.front().mnFontIdx)` — one font for the control, chosen
  from the opening run — while an ordinary text object goes through `lclCreateTextObject` and takes
  every run. **The distinction has nil reach**: all **60** corpus `TXO` on a control object — 54
  buttons in `EHEST` and 6 in `PC1000` — state exactly the opening run and its terminator, so the
  two rules give the same answer on every one of them. Recorded rather than implemented.
- **A shape run's colour and posture are not modelled**, §4. No corpus witness has been measured
  for what that costs, because measuring it means extending `SheetShapeRun` first.
- **`TOGAF9-Tool-ConfReqts-CSQ` at 16.76 over 28 pages with 4 MAJOR is untouched by either seat**
  and is the next `.xls` worth a look after `grants-2005` (96.11) and `orbus_togaf_tool_csq`
  (51.20), neither of which this round moves. All three are outside the conditional-format and
  text-box families.
- **`XlsNameRanges.Read`'s class fold is the naive `opcode & 0x3F`**, which turns a value-class
  `tArea3d` (`0x5B`) into `0x1B` and stops the walk. Censused rather than assumed: every one of
  the **111 built-in `NAME` formulas in the 64 `.xls` opens with `0x3B`, the reference class**,
  and **0 tokens anywhere in them are value- or array-class** — so the arm is unreachable on this
  corpus. Left alone rather than "fixed" blind inside another seat's round, and recorded so the
  next reader of that file does not take the fold as correct.

## 7. The suite

Ten non-fidelity projects, run individually and totalled here rather than through the solution:
Core 521, Containers 109, Markup 259, Text 728, OpenDocument 146, Presentations 1045,
Rendering 164, Vector 309, WordProcessing 1614, Spreadsheets **1276** — all green, 0 skipped.
Spreadsheets was 1271 at the base; the five new tests are the two fixtures' (§8).

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552**, and the ten are the known names:
`PageDrawingComparisonTests` ×4 (`paginated.docx`/`.fodt`/`.rtf`/`.doc`),
`TabStopComparisonTests` ×4 (`list-label-overrun.fodt`/`.docx`/`.odt`/`.doc`),
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
(`justify-shrink-2013.docx`). The total is read out of this run's own output rather than
transcribed from the brief.

**Both new test classes fail at the base.** Reverting `XlsDrawing.cs`, `XlsWorkbookReader.cs` and
`BiffRecords.cs` to `ac7745deb`, deleting `XlsConditionalFormats.cs` and rebuilding gives
`Failed: 5, Passed: 0` on the two classes; restoring gives `Failed: 0, Passed: 5`.

## 8. Files

| file | what it is |
|---|---|
| `cf-census.py`, `cf-census.tsv` | every `CONDFMT`/`CF`/`CF12` in the 64 `.xls`, with each rule's type, operator, format blocks and raw RPN |
| `ref-census.py`, `ref-census.tsv` | round 97's reference census with the fifth witness added |
| `patch-cf.py` | the one-attribute variant of §2.3 — `cellIs equal 2` → `equal 0` on `EHEST`'s first block |
| `name-token-census.py`, `name-token-census.txt` | every opcode in a built-in `NAME` record's token array, for the class-fold check of §6 |
| `txo-census.py`, `txo-census.tsv` | every `TXO` with text, its character count, its run count and the `FONT` indices they name |
| `sweep-ours.py` | round 92's sweep, unchanged; one output directory per document |
| `score-ink.py` | round 92's scorer over `pdf-image-diff.py` |
| `gate-columns.py` | the gate's page and `glyphs` columns for one directory of our renderings |
| `ink-before.tsv` | all 64 at the base, ranked on summed unsigned ink |
| `ink-after-o25.tsv`, `ink-after2.tsv` | the twelve movers on the two later legs |
| `gate-before.tsv`, `gate-after2.tsv` | the gate columns for the twelve movers, before and after |
| `movers-all.txt`, `movers-txo.txt` | the twelve, and the nine O19 moves inside them |
| `xlsx-confinement.tsv` | the 243 `.xlsx`/`.xlsm` renderings' md5 at the base and at head, for §2.4 |
| `xls-manifest.tsv`, `xlsx-manifest.tsv` | the `.xls` and `.xlsx`/`.xlsm` rows of `MANIFEST.tsv`, which is what the sweeps read |
