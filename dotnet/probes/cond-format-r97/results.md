# The two conditional-format successors: the compared string, and the bar

Round 97, seated by `probes/cond-format-r96/results.md` §4 and §5 and by register entries **O21**
and **O22**. Base `61bc19e03`, worktree `/home/user/wt-cfdraw`, branch `agent/cfdraw`, reference
`/opt/libreoffice26.2/program/soffice` (**LibreOffice 26.2.4.2**, build `0229ac93fcf0…`).

**This file was written twice.** A container restart killed the round mid-way and the worktree was
committed unvalidated as `0885c3e09`; everything below was then re-derived rather than re-read.
**The re-derivation changed six citations, one census figure and one whole arm** — §0 is the audit
and it is the most useful part of the round. Nothing here is carried over on the strength of the
first pass having said it.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. Nothing below rests on a reading: every fixture expectation is 26.2.4.2's own PDF
read back as filled rectangles, stroked paths, embedded images and coloured text spans
(`fixture-paint.py`, which the first pass did not have), and every corpus figure is a hash sweep
beside `pdf-image-diff.py`.

**Both seats reach a terminal state, and the second one splits.** O21 is fixed with a measured
reach. O22's `dataBar` half is fixed — with arm (4) **refuted and inverted** by this pass. Its
`iconSet` half is not nil reach and is re-seated as O29 with a corrected census. §6 says which.

---

## 0. The audit: what the first pass got wrong

### The C++ tree is not a 26.2.4.2 checkout, and no round has said so

`/home/user/libreoffice-core` is the fork's vendored LibreOffice, and `configure.ac` says
`27.2.0.0.alpha0+` — **master**, last upstream sync 2026-07-29. The reference binary's own build
hash, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`, **is not an object in that repository**
(`git cat-file -t` fails), and there is no `co-26.2` branch or `libreoffice-26.2.*` tag in it. So a
`file:line` citation here is line-exact in *that tree* and is not, by itself, a statement about the
binary every measurement is taken against.

That is not a reason to distrust the mechanisms — it is a reason to say which leg carries which
claim. **Every behavioural claim below is confirmed against the 26.2.4.2 binary**, by `fods`, by
PDF, or by both; the source is what names the mechanism, and the binary is what establishes that
26.2.4.2 does it. Where the two could disagree the binary wins, and in this round they did not
disagree anywhere.

### Six of seventeen cited ranges were wrong

Every range in the first pass's write-up was re-resolved by hand. `✓` means the cited range is
exactly the construct named.

| cited by the first pass | what is actually there | |
|---|---|---|
| `richstring.cxx`:60-63 — `setText` | 60-63 | ✓ |
| `conditio.cxx`:804-859 — `FillCache` | 804-859 | ✓ |
| `condformatbuffer.cxx`:351-356 — `DataBarRule` ctor | 351-356 | ✓ |
| `condformatbuffer.cxx`:384-389 — `importAttribs` | 384-389 | ✓ |
| `condformatbuffer.cxx`:1630-1643 — axis mapping | 1630-1643 | ✓ |
| `condformatbuffer.cxx`:1665-1703 — the `CFVO` case | 1665-1703 | ✓ |
| `condformatbuffer.cxx`:1710-1715 — `importDataBar` | 1710-1715 | ✓ |
| `conditio.cxx`:1913-1917 — `GetData`'s bar arm | 1913-1917 | ✓ |
| `fillinfo.cxx`:326-330 — `FillInfo`'s bar arm | 326-330 | ✓ |
| `colorscale.cxx`:1004-1043 — "the `AUTOMATIC` arm" | **1008-1049** | ✗ |
| `colorscale.cxx`:1066-1073 — "the `COL_LIGHTRED` fallback" | **1073-1085**, the colour at **1082** | ✗ |
| `condformatbuffer.cxx`:1656-1663 — `mbNeg` | **1658-1664** | ✗ |
| `colorscale.cxx`:1185-1252 — `GetIconSetInfo` | **1186-1253** | ✗ |
| `condformatbuffer.cxx`:455-467 — `NoIcons` | **456-467** | ✗ |
| `output.cxx`:955-985 — `drawIconSets` | **960-989** | ✗ |
| `output.cxx`:883-950 — `drawDataBars` | 883-**953** | ~ |
| `output2.cxx`:1691-1697 — `DrawStrings`' `bDoCell` | 1691-**1698** | ~ |

**The `AUTOMATIC` one is worth more than an off-by-four.** The sentence it supported was *"the
`AUTOMATIC` arm contains neither `nMinLength` nor `nMaxLength`"* — and line **1004**, inside the
cited range, is
`pInfo->mnLength = nMinLength + (nValue - nMin)/nDiff * (nMaxLength-nMinLength);`. The citation
falsified its own sentence. The sentence is true of the real arm, 1008-1049, and one refinement
comes with the correction: **`NONE` uses both lengths and `MIDDLE` uses only `nMaxLength`**
(1050-1068), so "used only in the `NONE` and `MIDDLE` arms" is right about where and loose about
which.

Three citations were missing rather than wrong, and each closes a step the write-up asserted:

- **`lcl_GetCellContent`** (`conditio.cxx`:766-802) is the link `FillCache` → `maText` that §1
  needed and did not have: for a `CELLTYPE_STRING` its line 793 is
  `rArgStr = rCell.getSharedString()->getString()`, the raw shared string.
- **`FillCache` lowercases the key** — `mpCache->maStrings.emplace(ScGlobal::getCharClass().lowercase(aStr), 1)`,
  `conditio.cxx`:842 — so `duplicateValues` is case-insensitive. Unmentioned by the first pass.
  Not a defect introduced here; recorded so the next round does not rediscover it.
- **`extlstcontext.cxx`:165-194** is where an `x14:cfRule` becomes a rule of its own, and its own
  comment says so: *"an ext entry does not need to have an existing corresponding entry"*. That is
  the source-side proof of §3's census correction, which the first pass argued only from markup.

### One arm is refuted, one census figure is wrong, and one sentence about the icon is wrong

Each is worked through where it belongs — arm (4) in §2, the census in §3, the icon in §6. In
summary:

| the first pass said | the measurement says |
|---|---|
| `mbNeg` is the presence of an `x14:negativeFillColor` and nothing else, so `COL_LIGHTRED` is **unreachable** from OOXML | `mbNeg` **defaults to true**; `COL_LIGHTRED` is the **usual** OOXML answer. 26.2.4.2 paints such a bar `#ff0000`. The reader was wrong and is fixed |
| `iconSet` is 20 rules in **9** documents | 20 rules in **10**. `icon-ink.txt` already listed ten rows |
| `drawIconSets` paints the icon **ten points square** | it paints it at the **cell's own font height**. The banked areas run 19.4 to 103.9 pt² per icon |

---

## 1. O21 — a cell's drawn text and its compared value are two different strings

### The mechanism, in three functions

`RichStringPortion::setText` is the whole of what Calc's SpreadsheetML importer does to a shared
string:

```cpp
void RichStringPortion::setText( const OUString& rText )
{
    maText = AttributeConversion::decodeXString(rText);
}
```

`sc/source/filter/oox/richstring.cxx`:60-63. The `_xHHHH_` escapes are resolved and **nothing else
happens**: every control character the file states is still in the string when the document holds
it. What `XlsxCellText.Of` describes — a lone `U+0009` dropped in a string with no line feed, a
`U+000D` dropped, the rest of `U+0000`–`U+001F` dropped — is the *drawing* layer's behaviour, and a
conditional format never reaches it.

The chain from there to the comparison is two more functions, and the first pass cited only the
middle one. `ScConditionEntry::FillCache` (`conditio.cxx`:804-859) walks the rule's ranges and keys
its `duplicateValues` cache on whatever `lcl_GetCellContent` hands back; that function
(`:766-802`) answers `rCell.getSharedString()->getString()` for a string cell — the raw `maText`,
with the control character still in it. The key is **lowercased** on the way in (`:842`), so the
comparison is case-insensitive as well as control-character-exact.

So the reader needed the stored spelling beside the drawn one, and now has it:
`XlsxCellText.Stored` is stage 1 alone, `XlsxSharedStrings.StoredAt` answers it, and
`XlsxConditionalStyles.Sheet.ValueOf` reads that rather than the indexer. The second copy is
recorded **only where it differs** from the drawn one, which on the corpus is a handful of strings
in a table of tens of thousands.

### The fixture, and the second half of the finding it produced

`tests/corpus/features/sheet-cf-stored-vs-drawn.xlsx`, 2592 bytes, authored by
`make-fixtures.py`. Column A is `\talpha`, `alpha`, `alpha`, `bravo`, `\tbravo`, `\tbravo` under one
`duplicateValues` rule, every tab written `_x0009_`. Column C is `\t`, three spaces and `zulu`
under `containsBlanks` and `notContainsBlanks`.

26.2.4.2's own PDF of it, re-rendered this pass and reproducing the banked record row for row
(`fixture-reference/painted.txt`):

| cell | stored | drawn | reference paints |
|---|---|---|---|
| A1 | `\talpha` | `alpha` | nothing |
| A2, A3 | `alpha` | `alpha` | `#FFC7CE` / `#9C0006` |
| A4 | `bravo` | `bravo` | nothing |
| A5, A6 | `\tbravo` | `bravo` | `#FFC7CE` / `#9C0006` |
| C1 | `\t` | *(empty)* | `#CCE5FF` — the **not**-blank rule |
| C2 | `   ` | `   ` | `#FFC7CE` — the blank rule |
| C3 | `zulu` | `zulu` | `#CCE5FF` |

The A column is the direct statement of the finding: **the reference draws A1 and A2 as the same
five glyphs and compares them as different keys**, and A5/A6 are duplicates of each other while A4
is a duplicate of nothing. Reading the drawn text makes all six a duplicate and paints all six.

**C1 is the half the brief did not ask for and it comes free.** `containsBlanks` is not a mode but
the substituted formula `LEN(TRIM(#B))=0` (round 96 §1), and Calc's `TRIM` takes **spaces and not
tabs** — so a cell holding one tab is *not* blank to the reference and *is* blank to a reader that
normalised the tab away first. The fixture shows the reference giving C1 the blue not-blank rule
and C2, three spaces, the red blank one. The same one-line change fixes both, because both go
through `Sheet.ValueOf`.

`XlsxConditionalPredicateTests.AConditionComparesTheStoredStringAndNotTheDrawnOne` asserts all
seven cells. **Re-derived this pass rather than quoted**: with only the three O21 source files at
`61bc19e03` and everything else at the change, the test fails with
`painted.Fill(0) should be null but was "#FFC7CE"` and passes with them.

### The reach, measured and small

The one corpus witness is round 96's: `6880ac7361ca…ST Capability List Rev.16 - Web.xlsx`, whose
`A3` is `"\tD5758620001301"` and whose `A2805` is `"D5758620001301"` — both re-read out of the
package this pass, and the tab is a **literal** `U+0009` in the XML rather than an `_x0009_`
escape, so `decodeXString` is not even involved.

| | base | **after** | 26.2.4.2 |
|---|---:|---:|---:|
| `#9C0006` spans, whole document | 18 | **16** | **16** |
| pages carrying them | 1, 25, 26, 27, 28, 85 | **25, 26, 27, 28** | **25, 26, 27, 28** |
| pages | 217 | 217 | 217 |

The reference's half was counted afresh: 16 spans, 5 on page 25, 7 on 26, 2 on 27, 2 on 28. The
two spans that go are the two the round-96 write-up named. See §4 for the ink figure.

---

## 2. O22, first half — `dataBar` is read, and half of its meaning is in the `x14` extension

Five arms, each read out of the source **and** confirmed twice at 26.2.4.2 — against
`--convert-to fods` of the real corpus documents (`fixture-reference/corpus-databar-fods.txt`,
produced this pass; the first pass had no corpus `fods` leg at all) and against an authored
fixture's rendering.

**(1) `minLength` and `maxLength` are ignored unless the axis is `NONE` or `MIDDLE`.**
`DataBarRule`'s constructor sets `mxFormat->meAxisPosition = databar::NONE`
(`condformatbuffer.cxx`:351-356) and `importAttribs` defaults the two lengths to **10 and 90**
(`:384-389`). Only the extension moves the axis, and `ExtCfDataBarRule::finalizeImport` maps
anything that is not `none` or `middle` — the `automatic` default of `:1714` included — to
`AUTOMATIC` (`:1630-1643`). `ScDataBarFormat::GetDataBarInfo`'s `AUTOMATIC` arm
(`colorscale.cxx`:**1008-1049**) contains neither `nMinLength` nor `nMaxLength`; the `NONE` arm
(991-1007) uses both and the `MIDDLE` arm (1050-1068) uses only `nMaxLength`.

So two files whose main-namespace markup is identical draw different bars. Both fixtures state
`<cfvo type="min"/><cfvo type="max"/>` over the values 0, 25, 50, 75, 100 in one column;
`sheet-cf-data-bar-auto.xlsx` adds an `x14:dataBar` with `autoMin`/`autoMax`, `gradient="0"`,
`minLength="0" maxLength="100"` and no `axisPosition` — which is what all nine corpus rules look
like.

| | reference's fods | bar widths in the reference's PDF, in a 46.91 pt cell |
|---|---|---|
| `sheet-cf-data-bar-lengths` | `axis-position="none"`, `min-length="10" max-length="90"` | 4.82, 14.23, 23.53, 32.88, 42.29 |
| `sheet-cf-data-bar-auto` | *no* `axis-position`, still `min-length="10" max-length="90"` | *none*, 11.73, 23.47, 35.18, 46.91 → **0 : 1 : 2 : 3 : 4** |

The second row is the one that matters: the two lengths **survive the import** and stop reaching the
bar, which is why reading the fods alone would have got it wrong. And **a cell sitting on the
automatic minimum draws no bar at all** — `nValue <= nMin` gives `mnLength = 0` at `:1044-1045` and
`drawDataBars` returns at `:914-915` before painting.

*The first pass rendered the first row as an exact `1 : 3 : 5 : 7 : 9`. It is not exact.* The
`NONE` arm predicts 4.69, 14.07, 23.46, 32.84, 42.22 and the measurement is 0.07 to 0.13 pt wider
at every step, because `drawDataBars` sets a **line** colour before `DrawGradient` (`:919`) and the
gradient's outline extends the rectangle PyMuPDF reports. The ratios are 1 : 2.95 : 4.88 : 6.82 :
8.77. The arm is unaffected — the discriminator is the *other* fixture — but the tidy ratio was an
artefact of the instrument and is withdrawn.

**(2) The extension's own `minLength`/`maxLength` are never read.** `importDataBar`
(`condformatbuffer.cxx`:1710-1715) reads `gradient` and `axisPosition` and nothing else.
**Measured on the corpus, not inferred**: all nine rules state `minLength="0" maxLength="100"` in
their extension, and 26.2.4.2's `fods` of all six documents writes `min-length="10"
max-length="90"` for every one of the nine. None of the nine states an `axisPosition` and all nine
say `gradient="0"`.

**(3) The extension's `cfvo` types overwrite the main namespace's, and the value only when it
parses whole.** `:1665-1703`. `autoMin`/`autoMax` both become `COLORSCALE_AUTO` (`:1678-1681`),
which the main namespace cannot spell; a `num` takes the `<xm:f>` beside it only when
`nSize == msScaleTypeValue.getLength()` (`:1696`).

**The corpus split is measured — 3 rules `autoMin`/`autoMax` (`076` twice, `088`) and 6 `num
0`/`num 1`** — and the `fods` confirms it: those three come back `auto-minimum`/`auto-maximum`
against main-namespace markup saying `min`/`max`.

*The first pass illustrated the parse-whole condition with `069_Blue_modern_balance_sheet`'s
`$C$11`, and that is a category error.* `069` states **no `dataBar` at all**; its `$C$11` is in an
`x14:cfRule type="iconSet"`, which `extlstcontext.cxx`:188-194 routes to `IconSetContext` and
`IconSetRule` — a different class from the `ExtCfRuleContext`/`ExtCfDataBarRule` pair that
`:165-187` builds for a bar, so it never reaches `:1665-1703`. **No corpus `dataBar` extension
states a formula `cfvo` and no fixture does either**, so the parse-whole branch is read from the
source with **no witness on either leg**. It is implemented because it is cheap and it is recorded
here as unwitnessed rather than as measured.

**(4) — REFUTED. A negative value with no `x14:negativeFillColor` is drawn in the source's own red,
not in the bar's colour.**

The first pass had this exactly backwards: *"`mbNeg` is set by the presence of that element and by
nothing else, and the `COL_LIGHTRED` fallback beside it is therefore unreachable from an OOXML
import."* Both halves are false.

`mbNeg` is **`ScDataBarFormatData`'s own default of `true`** (`sc/inc/colorscale.hxx`:107). Every
assignment to it in `sc/` is one of three, and none of them can make an OOXML bar's `mbNeg` false:
`condformatbuffer.cxx`:1662 sets it **true** beside `mxNegativeColor`; `condformatuno.cxx`:1228 is
the UNO setter; and the only place it is cleared is the **ODF** importer,
`sc/source/filter/xml/xmlcondformat.cxx`:483. So `GetDataBarInfo`'s
`if(mpFormatData->mbNeg && nValue < 0)` (`colorscale.cxx`:1073) holds for every OOXML bar, and a
rule with no `mxNegativeColor` takes the fallback beside it — `COL_LIGHTRED`, `:1082`, inside the
block 1073-1085 that the first pass cited as 1066-1073.

**Confirmed twice at 26.2.4.2, on a fixture authored for it.**
`sheet-cf-data-bar-default-negative.xlsx` is `sheet-cf-data-bar-negative.xlsx` with the
`x14:negativeFillColor` removed and nothing else changed. Its `fods` writes
`calcext:negative-color="#ff0000"` — exactly what eight of the nine corpus rules write — and its
PDF paints:

| cell | value | `…-negative` (states `#c00000`) | `…-default-negative` (states none) |
|---|---:|---|---|
| A1 | −100 | x 53.89–77.36, `#c00000` | x 53.89–77.36, **`#ff0000`** |
| A2 | −50 | x 65.62–77.36, `#c00000` | x 65.62–77.36, **`#ff0000`** |
| A3 | 0 | *none* | *none* |
| A4 | 50 | x 77.33–89.06, `#2e75b6` | x 77.33–89.06, `#2e75b6` |
| A5 | 100 | x 77.33–100.80, `#2e75b6` | x 77.33–100.80, `#2e75b6` |

Identical geometry, one colour changed: a clean one-attribute control.
`XlsxDataBarTests.WithNoNegativeFillColourANegativeValueTakesTheSourcesOwnRedAndNotTheBarsColour`
fails against the salvaged reader and passes against the fixed one.

**Corpus reach of the correction is nil, and that is measured rather than asserted.** The 9 rules
cover **75 numeric cells across the corpus and not one of them is negative**, counted straight out
of the six packages over each rule's own `sqref`; and 26.2.4.2's own renderings of all six
documents contain **zero `#ff0000` filled rectangles**. So the branch is unreachable on this
corpus in both directions, which is also why the sweep in §4 is unaffected by the fix.

What the rest of the negative fixture pins stands, and was re-measured: the zero is
`-100 × nMin/(nMax − nMin)` = 50 % (`:1024`), which is the midpoint of the 53.89–100.80 paint
rectangle; A3 draws **neither** a bar nor an axis, because the zero-length `return` at `:914-915`
is above the axis code at `:938-950`; and the axis spans the **whole** cell (y 70.81–85.69) while
the bar is inset (y 71.04–85.49) — `Point aPoint1(nPosZero, rRect.Top())` at `:942` against a
`aPaintRect` adjusted at `:887-890`.

**(5) `showValue="0"` takes the cell's value off the page rather than hiding it.** It sets
`mbOnlyBar`, which becomes `!mbShowValue` (`colorscale.cxx`:1090), and `ScOutputData::DrawStrings`
then clears `bDoCell` (`output2.cxx`:1691-1698). 26.2.4.2's PDF of `sheet-cf-data-bar-only.xlsx`
holds five bars and **no text-showing operator at all**.

**The claim that this happens after the row's height is settled is now measured rather than
asserted.** `sheet-cf-data-bar-only` and `sheet-cf-data-bar-lengths` differ only in `showValue`,
and their five bars are drawn at *identical* y positions — 70.98, 85.86, 100.72, 115.60, 130.48 on
both — so suppressing the value does not shorten the row. **And the corpus witnesses it**:
`036_Simple_to-do_list` states `showValue="0"` and its `fods` comes back `show-value="false"`,
which the first pass did not notice. `SheetFormatting.HidesValue` is asked in
`SheetPageDrawing.DrawCell`, the same place in the pipeline.

### The geometry, and the one constant that had to be measured

`drawDataBars` (`sc/source/ui/view/output.cxx`:883-953) insets the cell by `2 * nOneX` and
`2 * nOneY` (`:887-890`), where `nOneX = PixelToLogic(Size(1,1)).Width()` on the output device. For
a PDF export that device is `vcl::PDFWriter`'s reference device at 720 dpi, so one pixel is two
twips and the inset is **0.2 pt on each edge**. Measured rather than taken on trust, and the
sharpest instance is the negative fixture's own axis, which is *not* inset while its bar is:

| | cell / axis | bar | inset per edge |
|---|---|---|---|
| `…-negative`, rows down | axis y 70.81–85.69 | 71.04–85.49 | 0.23 / 0.20 |
| `088_To-do_list`, G5 across | 518.35–600.75 | 518.54–600.55 | 0.19 / 0.20 |
| `088_To-do_list`, rows down | 21.01 pt tall | 20.62 pt tall | 0.195 |

`SheetPageDecoration.BarInset` is 0.2 pt. The bar itself is then
`[zero, zero + (right − zero) × length/100]` for a positive length (`:910-912`) and the mirror for
a negative one (`:902-907`), and the reference truncates that product to whole logic units
(`static_cast<tools::Long>`), which this does not — worth 1/20 pt at most.

**The reference half of the corpus geometry was re-derived this pass** from the banked
26.2.4.2 rendering of `088_To-do_list` page 1, and reproduces the first pass's table exactly:

| value | 26.2.4.2 | ours (after) |
|---:|---|---|
| 0.5 | 518.54–559.56 × 126.85–147.47 | *see §4* |
| 1.0 | 518.54–600.55 × 147.85–168.46 | |
| 0.75 | 518.54–580.06 × 168.84–189.46 | |
| 0.25 | 518.54–539.06 × 189.83–210.45 | |

The colour is `#bfa9b7` on the reference — theme slot 4 at tint 0.39997558519241921, which the
`fods` confirms as `positive-color="#bfa9b7"`.

### Precedence

`ScConditionalFormat::GetData` walks `maEntries` and takes a bar only when it has none
(`conditio.cxx`:1913-1917), and `ScDocument::FillInfo` does the same across formats
(`fillinfo.cxx`:326-330). So the *first* bar to reach a cell keeps it, under round 96's ordering:
blocks in document order, rules within a block by priority.

The corpus states the discriminating case for the priority half.
`076_Inventory_list_accessibility_guide` declares two `dataBar` rules over `J6:J16` **inside one
block**, priorities 21 and 22, whose `fods` resolves them to `#d9d9d9` and `#989494`. **Counted
this pass** in 26.2.4.2's own PDF: **eleven `#d9d9d9` rectangles and zero `#989494` anywhere**. No
corpus document states two bars in two different blocks, so the block half is read from the source
and not measured here — said rather than implied.

### What is deliberately not modelled

**A gradient bar is painted solid.** `mbGradient` is `ScDataBarFormatData`'s own default of *true*
(`colorscale.hxx`:106) and only `x14:dataBar/@gradient` changes it; the reference's true branch is
`DrawGradient` with a linear gradient from the bar's colour to `COL_TRANSPARENT` at 255 steps
(`output.cxx`:917-931), and its PDF of `sheet-cf-data-bar-lengths.xlsx` comes back as **209 slices
fading from `#2e75b6` to white**, a count VCL derives from the device's own pixel width. **0 of the
9 corpus rules state it** — all nine carry an extension saying `gradient="0"`, confirmed in both
the packages and the `fods` — so it is recorded and not reproduced, and `SheetDataBar.Gradient`
carries the reason.

**The axis dash pattern is the source's and has no corpus witness.** `LineInfo(LineStyle::Dash, 1)`
with four dashes of three logic units and three between them (`:944-947`). All nine corpus rules
resolve a minimum at or above zero, so their zero sits on the left edge and `drawDataBars` returns
at `:939-940` before the axis: **0 of 9 reach it**, and the banked reference renderings of all six
documents contain **no dashed stroke at all**.

**With the positive control, because a scan that finds nothing is worth nothing without one.** The
same scan over the two negative fixtures finds **4 dashed strokes each**, with the dash array
`[.08504 ×8] 0` — and `.08504` pt is exactly three hundredths of a millimetre, so the measurement
also settles that `LineInfo`'s "logic units" here are 1/100 mm and not twips. That is what
`SheetPageDecoration.AxisDash` is derived from, and it is the same unit that makes `2 * nOneX` at
720 dpi come to 0.19999 pt rather than to anything else.

---

## 3. The census, corrected in both directions — and corrected again this pass

Counted 2026-09-11 by `census-drawrules.py`, which opens **every** corpus file as a zip rather than
filtering on `.xlsx`/`.xlsm` — round 96's own warning about `Special-Procedures_2025-07-10.xls` —
and which counts an `x14:cfRule` as a rule of its own when no main-namespace rule claims its `id`
through an `<x14:id>`. Re-run this pass and reproduces exactly.

| | main-namespace rules | documents | `x14` extensions of those | `x14`-only rules | documents |
|---|---:|---:|---:|---:|---:|
| `dataBar` | 9 | 6 | 9 | **0** | 0 |
| `iconSet` | 2 | 2 | 0 | **18** | 8 |

`244` corpus documents open as OPC spreadsheets, one more than round 96's 243.

**`dataBar` is exactly what round 96 said: 9 rules in 6 documents.**

**`iconSet` is 20 rules in 10 documents — not 2 in 2, and not the 9 the first pass wrote.** The two
main-namespace rules are in `075_Idea_planner_tasks` and `sistem-rekod-markah-srm`; the eighteen
`x14`-only ones are in `041`, `042`, `066`, `069`, `076`, `077`, `078` and `088`. Those two sets are
**disjoint**, so the union is ten, and `icon-ink.txt` has listed ten rows since the first pass
wrote it — the prose and its own data file disagreed and the prose was quoted.

The reason the eighteen exist at all is in the reference's source, and the comment is explicit:
`ExtLstLocalContext` builds a fresh `ScDataBarFormatData`/`ScIconSetFormat` for an `x14:cfRule`
whose `id` matches no main-namespace rule — *"an ext entry does not need to have an existing
corresponding entry"*, `extlstcontext.cxx`:165-194. **A census that reads only `cfRule` in the
SpreadsheetML namespace cannot see an `x14`-only rule**, which is the general form of the lesson
and is in the script's docstring.

---

## 4. What moved, over the whole corpus

Rendered **our half of all 947 corpus documents twice**, at the round's base (`61bc19e03`) and with
the change, under `SOURCE_DATE_EPOCH`, one output directory per *document*, at two frozen copies of
the CLI outside the tree — so no rebuild could reach either leg while it ran, and the worktree was
rebuilt several times during them without touching the measurement. Both legs completed **947 of
947 with `ok` on every row and no failure on either side** (`sweep-base-hashes.tsv`,
`sweep-after-hashes.tsv`; the script is round 96's `sweep-ours.py`, unchanged, with
`sweep-keep.txt` as its keep list).

**940 of 947 renderings are byte-identical. Seven move, and they are exactly the seven documents
the two seats predict** — the six that state a `dataBar` and the one cell O21 is about. Nothing
else in the corpus changed by a byte, in any family.

Scored on summed unsigned ink against the banked 26.2.4.2 reference at
`/home/user/gate-orig-r83/ref/` (`score-movers.sh`, which is `pdf-image-diff.py` per page):

| document | pages | base | **after** | MAJOR |
|---|---:|---:|---:|---|
| `066_Agile_Gantt_chart` | 5 | 4.43 | **4.10** | 3 → 3 |
| `076_Inventory_list_accessibility_guide` | 9 | 1.00 | **0.99** | 0 → 0 |
| `015_Free_Gantt_Chart_Template_for_Excel` | 8 | 0.95 | **0.92** | 1 → 1 |
| `085_Simple_Gantt_chart` | 3 | 0.90 | **0.90** | 0 → 0 |
| `036_Simple_to-do_list` | 4 | 0.48 | **0.44** | 1 → 1 |
| `088_To-do_list_with_progress_tracker` | 1 | 0.29 | **0.04** | **1 → 0** |
| `ST Capability List Rev.16 - Web` | 217 | 26.85 | **26.80** | 0 → 0 |
| **sum** | | **34.90** | **34.19** | **6 → 5** |

**Six improve, one is level, none worsens**, and no page count moves anywhere — 8, 4, 5, 9, 3, 1
and 217 on both legs and on the reference.

**So the reach figure is 7 of 947 renderings, and it is not the rule count.** Round 96 recorded
this as N11 after 1215 newly-read rules moved three renderings; the same arithmetic applies here
and in the same direction. *9 `dataBar` rules and 1 string* is what was read; **7 documents** is
what changed; and the honest headline is the second. Quoting "9 rules in 6 documents" as O22's
reach — which the salvaged register entry did — states the input as though it were the output.

`088_To-do_list` is the clean one and is worth naming separately: **0.29 → 0.04 and its only MAJOR
page gone**, because a one-page document whose single unread feature was its four bars has nothing
else to hide the change. Its geometry against 26.2.4.2, both halves measured this pass:

| value | 26.2.4.2 | ours (after) |
|---:|---|---|
| 0.5 | 518.54–559.56 × 126.85–147.47 | 518.66–559.65 × 126.90–147.49 |
| 1.0 | 518.54–600.55 × 147.85–168.46 | 518.66–600.64 × 147.89–168.48 |
| 0.75 | 518.54–580.06 × 168.84–189.46 | 518.66–580.15 × 168.88–189.48 |
| 0.25 | 518.54–539.06 × 189.83–210.45 | 518.66–539.15 × 189.88–210.47 |

**Widths agree to 0.03 pt and positions to 0.12**, with the same `#bfa9b7` on both sides. The
position offset is one constant in one direction, which is this tree's own text-origin offset from
the reference's and not a data-bar quantity. *The first pass said 0.02 on widths; it is 0.03.*

And `076_Inventory_list_accessibility_guide` is the precedence witness: **eleven `#d9d9d9`
rectangles and no `#989494`** in 26.2.4.2's PDF, and the same eleven and the same absence in ours
after the change.

For O21, the span census across all three renderings, re-derived this pass rather than quoted:

| | pages | `#9C0006` spans | on pages |
|---|---:|---:|---|
| base | 217 | 18 | 1, 25, 26, 27, 28, 85 |
| **after** | 217 | **16** | **25, 26, 27, 28** |
| 26.2.4.2 | 217 | 16 | 25, 26, 27, 28 |

Exact, and the reference's sixteen are 5 on page 25, 7 on 26, 2 on 27 and 2 on 28. **Reach for O21
alone is 1 cell in 1 of 947 renderings, 2 spans**, and it moves that rendering onto the reference's
span census exactly.

**The negative-colour correction of §2 arm (4) is inside these numbers and changes none of them.**
The after leg was rendered before that fix; re-rendering all six `dataBar` documents with the
final binary afterwards gives **six of six byte-identical** PDFs, which is the empirical form of
the nil-reach census (`negative-under-databar.py`: 75 numeric cells under a corpus `dataBar`, 0
negative).

---

## 5. Confinement and the suite

**The diff is confined to `dotnet/src/XRay.Office.Spreadsheets`** — nine files, two of them new — and
nothing under `dotnet/src` outside it changes. That is the argument for reusing the banked
reference half; §4 is the *measurement* of the same thing, and it is stronger than the argument:
940 of 947 renderings byte-identical means no words document, no deck and no other workbook moved,
rather than that none should have.

Clean build of the whole solution at `-c Release`: **0 warnings, 0 errors**, with
`TreatWarningsAsErrors` on.

| project | | |
|---|---:|---|
| `XRay.Office.Core.Tests` | 521 | ✓ |
| `XRay.Office.Markup.Tests` | 259 | ✓ |
| `XRay.Office.Text.Tests` | 728 | ✓ |
| `XRay.Office.Containers.Tests` | 109 | ✓ |
| `XRay.Office.Vector.Tests` | 309 | ✓ |
| `XRay.Office.Rendering.Tests` | 164 | ✓ |
| `XRay.Office.OpenDocument.Tests` | 146 | ✓ |
| `XRay.Office.WordProcessing.Tests` | 1885 | ✓ |
| `XRay.Office.Spreadsheets.Tests` | 1270 | ✓ (1269 at the base, +1 for arm (4)) |
| `XRay.Office.Presentations.Tests` | 1044 | ✓ |
| **ten projects** | **6435** | **0 failed** |

`XRay.Office.Fidelity.Tests`: **542 passed, 10 failed of 552**, and the ten are exactly the known
names — `PageDrawingComparisonTests` ×4 (`paginated.` `rtf`/`docx`/`doc`/`fodt`),
`TabStopComparisonTests` ×4 (`list-label-overrun.` `doc`/`docx`/`fodt`/`odt`),
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
(`justify-shrink-2013.docx`). No eleventh failure.

**And one instrument note that this round nearly banked a false figure on.**
`XRay.Office.WordProcessing.Tests` reported **`Passed! … 870`** on the first run of the pass and
**1885** on the last, both with `Failed: 0` and nothing between them touching that project.
`dotnet test --list-tests` discovers **1885**, and 1885 reproduced on a second consecutive run — so
the *first* run was the truncated one, and it announced itself as a pass. `dotnet/CLAUDE.md`'s
*"a truncated run reports success"* is written around a **drop** in the count and this was the same
fault seen from the other side: the suspicious number was the one taken **first**, with no earlier
figure to compare it against. The habit that catches it is the one that file already gives —
compare against `--list-tests`, not against your own previous run — and it is worth doing on the
first run of a round rather than only when a count falls.

---

## 6. What is left, and which of the two states it is in

**O21 — fixed.** Mechanism cited by `file:line` and re-resolved this pass, fixture whose
expectation is 26.2.4.2's own rendering, a test re-derived as failing at the base and passing
after, and a measured reach in §4.

**O22, `dataBar` — fixed**, with arm (4) refuted, the reader corrected and a sixth fixture added
for it. Reach in §4.

**O22, `iconSet` — not closed, and re-seated as O29 with a corrected census.** It is neither fixed
nor nil reach, and saying otherwise would be the kind of promotion the ground rules warn about.
What is established:

- **Reach is 20 rules in 10 documents**, not 2 in 2 and not 9 documents (§3), and 26.2.4.2's own
  renderings of those ten draw **60 icons covering 1549 pt²** in seven of them — 27 on
  `066_Agile_Gantt_chart`, 12 each on `077_Inventory_list_with_highlighting` and
  `078_Modern_inventory_list`, 4 on `075_Idea_planner_tasks`, 2 each on
  `069_Blue_modern_balance_sheet` and `076_Inventory_list_accessibility_guide`, 1 on
  `088_To-do_list`. `041_Business_budget`, `042_Business_monthly_budget` and
  `sistem-rekod-markah-srm` state a rule and draw no icon. `icon-ink.txt`, re-run this pass.
- **Every part of it except the glyph is readable**, and is written down here so the next round
  does not re-derive it. `ScIconSetFormat::GetIconSetInfo` (`colorscale.cxx`:**1186-1253**) walks
  all the entries and keeps the **last** index whose `Compare` holds — it does not break
  (`:1206-1216`) — with each entry's own mode, default `EqGreater` (`:1203`), which `gte="0"` turns
  into `Greater` (`condformatbuffer.cxx`:118-124). `GetMinValue`/`GetMaxValue`
  (`colorscale.cxx`:1312-1334) take a `COLORSCALE_VALUE` or `COLORSCALE_FORMULA` first entry's own
  number and otherwise the range's. `mbReverse` reflects the index across the entries after the
  search (`:1226-1230`). A custom entry spelt `NoIcons` is stored with index **−1**
  (`condformatbuffer.cxx`:**456-467**) and `GetIconSetInfo` then returns **`nullptr`**
  (`:1236-1239`), so such a cell has no `ScIconSetInfo` at all and therefore **keeps its own text
  however `showValue` is set** — which is why the `showValue="0"` half of the family cannot be
  implemented on its own either. Two guards the first pass missed: a cell that is not numeric
  (`:1189-1190`) and a set of fewer than three entries (`:1195-1196`) both answer `nullptr`.
- **What blocks it is artwork, not reading.** `drawIconSets` (`output.cxx`:**960-989**) paints
  `ScIconSetFormat::getBitmap`'s image in the cell's bottom-left corner (`:988`), and that image is
  one of LibreOffice's own icon-theme assets. There is nothing in the file to derive it from, and
  copying the assets into this tree is not a rendering decision to take in a round. Drawing the
  bucket without the glyph — suppressing the value and painting nothing — would be further from the
  reference than doing nothing, because the icon is the ink and the value is not drawn beside it.
- ***It is not "ten points square", which the first pass wrote and its own data refutes.*** The
  ten points at `output.cxx`:967 is a fallback for a null `mnHeight`, and `GetIconSetInfo`
  **always** sets that field, from the cell's own `ATTR_FONT_HEIGHT` (`colorscale.cxx`:1222-1224),
  so the branch at `:969-980` always wins and the icon is as tall as the cell's font, with its
  width scaled by the bitmap's aspect ratio (`:982-984`). The banked areas say so plainly: 19.4 pt²
  per icon on `066` against 103.9 on `069`, a range of more than five to one, where a constant ten
  points would be 100 pt² throughout.

So: **`iconSet` stays seated**, with its census, its mechanism and the one thing that would have to
be decided before it can be closed. What is left is exactly *what glyph to draw*, and that is a
question about assets rather than about the format.

**One thing found on the way and left, because it is O20's family and not either of these.**
`088_To-do_list`'s remaining residual: its `expression` rule `AND($G3=0,$G3<>"")` over `B3:I7`
paints row 3 in the reference and in neither of our legs, and its `dxf` is
`<fill><patternFill><bgColor theme="0" tint="-0.0499893…"/></patternFill></fill>` — **a
`patternFill` with no `patternType`**. Whether this tree requires the attribute or the formula
evaluator lacks `AND` is not established here and was deliberately not chased: changing how a
`dxf` fill is read reaches 55 documents and 1215 rules, which is a round of its own.

**And one instrument note that cost this pass an hour.** `fixture-paint.py` renders each fixture
into **its own** output directory, because `soffice --convert-to` names its output after the input
stem alone. All six new fixture stems were checked for uniqueness across
`dotnet/tests/corpus`, `/home/user/sample-files` and `/home/user/corpus-odf` before rendering —
one occurrence each. `dotnet/tests/corpus` holds 69 duplicated stems between its own
subdirectories, so this is not a hypothetical.
