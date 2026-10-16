# sheets-e-01 — prediction, committed before any measurement

Written after reading `sheets-d-01/results.md`, `gate-01/results.md`, the last three sheets merge
notes, and the **27.2.0.0.alpha0+ C++ tree** (`sc/source/ui/view/output2.cxx`,
`sc/source/ui/view/output.cxx`) plus our own `SheetTextLayout.cs` / `SheetPageDecoration.cs`.
**Nothing below has been measured.** No PDF has been opened, no probe rendered, no census run.
Everything sourced from the C++ is **inferred** and the tree is not the reference binary — three
rounds have already died to that gap, so every inference below is stated as a thing to refute.

Subjects, in the briefed order: (1) `###`, (2) the accounting `$`/`-`, (3) the uncoalesced grid.

---

## P0 — the brief's own direction on `###` is inverted

The brief says *"we draw 1101 where the reference draws 2"*. `gate-01` says the opposite in three
separate places — §3 item 3 (*"the reference emits `###` 1101 times … and we emit it twice"*), §7's
sheets table (`ODs-February…xlsx`, non-alnum **ours 148 / ref 1255**), and §10 item 3 (*"we emit 895
more real words than the reference and do not emit `###`"*).

**P0.1** Measurement will show, on `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx`:
reference ≈**1101** `###` tokens, ours ≈**2**. The brief is wrong by direction. We **under**-produce.

**P0.2** Consequently the fix is *adding* `###`, and the direction of every downstream number in
this round is the reverse of what the brief implies.

## P1 — LibreOffice's `###` rule, as I expect the authored probe to measure it

Inferred from `output2.cxx:1974-2030` (`DrawStrings`) and `:610-716`
(`ScDrawStringsVars::SetTextToWidthOrHash`). The gate for hashing is
`bCellIsValue && (aAreaParam.mbLeftClip || aAreaParam.mbRightClip)`, and then:

**P1.1** A **non-`General`** number format hashes **outright**, with no attempt to shorten:
`(nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0 → SetHashText(); return true`. A fixed `0.00`, a
currency, a percent and a **date** all hash the moment the cell is one pixel too narrow.

**P1.2** A **`General`** format does **not** hash on the first failure. It re-renders through
`SvNumberformat::GetOutputString` with `nWidth / nMaxDigitWidth` digits, corrects that count for
the narrower sign / decimal-separator / `E` glyphs, re-renders again, and hashes **only** if the
result is *still* wider than the cell. So a wide General number goes to scientific notation, not to
`###`. (Our `SheetGeneralWidth` already claims this; the probe re-establishes it on 26.2.4.2.)

**P1.3** A **text** cell never hashes at any width — `SetTextToWidthOrHash` returns false for a
type that is neither `CELLTYPE_VALUE` nor `CELLTYPE_FORMULA`.

**P1.4** A **formula in error** hashes regardless of format and regardless of how much of `#REF!`
would fit — `pFCell->GetErrCode() != FormulaError::NONE → SetHashText()`.

**P1.5** A value cell is **never** widened into an empty neighbour: `GetOutputArea` is called with
`bCellIsValue || bRepeat || bShrink` and that argument gates the whole spill loop
(`output2.cxx:1330`). So "too narrow" is decided against the cell's own width minus one pixel minus
margins, whatever is beside it. A **string** in the same column at the same width spills instead.

**P1.6** **Shrink-to-fit suppresses `###`.** `bShrink` shrinks the font first and the re-measured
text is no longer clipped, so `bCellIsValue && clipped` is false by the time the hash gate is
reached. Weakest of the six; I am reading control flow across a hundred lines.

**P1.7** **Wrap does not save a plain number.** `bBreak` is forced off for a plain number format
(i#111387), so a wrapping column still hashes a wide number; a **date** in a wrapping column wraps
instead of hashing, because a date is not a plain number format.

**P1.8** A **merged** cell hashes on the merged width, not the origin column's.

Each of these is one variable against a control in a single authored `.fods`, rendered by
26.2.4.2 itself. At least two points per variable (one width that fits, one that does not).

## P1.9 — where I expect the corpus divergence actually to sit, and it is *not* the hash rule

Our `SheetTextLayout.Hash` (`:894-...`) **already** implements P1.1/P1.2 by format. So 2-against-1101
is almost certainly not the hash rule but the **clip decision** upstream of it: `area.IsClipped` is
false on that workbook where LibreOffice's `mbRightClip` is true. Candidates, with my prior:

| | candidate | prior |
|---|---|---:|
| (i) | the cells are not `isValue` on our side — `cell.Value` is a `string` (cached formula result, or a shared string) so `isValue` is false and the hash gate is never reached | 30% |
| (ii) | our column widths on that workbook are wider than the reference's, so nothing clips | 30% |
| (iii) | the cells `Breaks()`, so the clip branch is bypassed | 15% |
| (iv) | our shaped text is narrower than LibreOffice's for these cells (font/metric), so `needed <= width` | 15% |
| (v) | something else entirely | 10% |

**Blind spot, named:** a static census over `xl/worksheets/*.xml` + `xl/styles.xml` can see the
format code, the column width and the cell type, but **cannot** see our shaped text width, so it
cannot discriminate (ii) from (iv) on its own. That needs an instrumented render, not a census.

## P2 — the accounting `$`/`-`

`fy2011-aip-grants.xls`: ours 11 538 non-alnum tokens, reference 9 020, and `gate-01` already
recorded that the reference **runs the padding together** — `$-` ×379, `$$-` ×132, `$$$-` ×75,
`$$$$-` ×52 — while the letter-or-digit counts are **43 201 on both sides, exact**.

**P2.1** The divergence is **not** in how many `$` or `-` glyphs are drawn. Counted as *glyphs*
(characters in the text layer, not whitespace-delimited tokens) the two sides will agree to within
a few percent. The token gap is **poppler joining adjacent glyphs into one token on one side and
not the other**, i.e. a *spacing* difference, not a *content* difference. Point estimate: glyph
counts within 5%; token counts differ by the briefed 2 518.

**P2.2** Mechanically: the accounting format's `_(` (skip one `(`-width) and `* ` (repeat-fill)
make LibreOffice emit the `$`, the fill and the `-` **inside one text-showing run** — `RepeatToFill`
(`output2.cxx:572-608`) literally *inserts fill characters into the string* — where we position the
`$` and the `-` as two separately placed runs. So the reference's `$` and `-` for one accounting
zero are in the same `Tj`/`TJ`; ours are not.

**P2.3** Therefore this is **not a verdict item and never was** — the corrected gate already scores
this document as `match` (43 201 = 43 201). Predicted verdict movement from anything found here:
**zero**. It is a text-layer fidelity item.

**P2.4** If P2.1 is refuted and we really do draw ~2 500 more `$`/`-` glyphs, the cause I would look
at first is the repeat-fill count: `RepeatToFill` truncates `nCharsToInsert` toward zero and bails
when `nSpaceToFill <= nCharWidth`, so an off-by-one in the fill count multiplies across 40 000 cells.

**Blind spot, named:** poppler's word-joining threshold is not observable from the PDF. I can
measure the inter-glyph gap the two sides emit; I cannot measure the threshold, so any statement of
the form "below X pt poppler joins" is inferred.

## P3 — the grid

`6f9e605c-fded-11e3-bd0e-00144feab7de.xls` page 1: reference **107** vertical 0.1 pt rules, ours
**17**; horizontals 34 against 33.

**P3.1** **We do draw the grid on that page.** The 17 verticals *are* `DrawGrid`'s output, one per
placed column over the full block height, and 17 is the page's column count. The briefed question
"do we draw the grid at all" has the answer **yes**. (If the count is not the column count, this is
refuted and the round changes shape.)

**P3.2** The reference's 107 is `ScOutputData::DrawGrid`'s **`bSingle`** branch
(`output.cxx:456-513`). A column takes it when **any** of three things holds:
`nWidthXplus1 == 0` (the next column is hidden or zero-width), or some row has
`cellInfo(nX+1).bHOverlapped` (a merge), or some row has `cellInfo(nX).bHideGrid`. In that branch
the vertical is emitted **once per row** instead of once per column.

**P3.3** The cause on this page is **`bHideGrid`**, and `bHideGrid` is set by `GetOutputArea`
(`output2.cxx:1338` and `:1345`) on **every column a string overflowed across**. Not merges. I put
70% on overflow, 20% on a zero-width column, 10% on merges.

**P3.4 — the part that is actually visible, and the reason this is worth doing.** In the `bSingle`
branch the per-row segment is **skipped** for exactly the rows where `bHideGrid` or `bHOverlapped`
holds. So LibreOffice's grid has **holes**: a cell that a long label spills across gets **no
right-hand grid rule**. That is ink, not structure. The 107-against-17 is the *symptom*; the
defect is that we draw a rule through the middle of overflowing text and LibreOffice does not.

**P3.5** Direction warning. Implementing only the holes lowers our count; implementing only the
per-row split raises it. Neither alone lands on 107. If I implement the holes and not the split I
predict the page's vertical count goes **17 → somewhere in 14–17**, still far from 107, and the
*raster* moves while the *count* barely does — the mirror image of `sheets-d-01`, where the count
moved and the raster did not.

**P3.6** Per-row splitting reproduces a ~0.11 pt gap at each row boundary (`nNextY - nOneY`) and is
**fidelity-only**: abutting-with-a-gap hairlines put down less ink than one continuous rule, so
unlike the border round this one *would* show in the raster, very slightly. I do not expect to get
to it.

## P4 — reach, direction and verdict movement

**P4.1 `###` reach.** Documents on the sheets track whose rendering changes if the clip decision is
corrected: I predict **8–35 of 171**, point estimate **18**. Skewed low because most workbooks size
their columns to their content.

**P4.2 `###` direction.** On `ODs-February` our `###` count goes 2 → within ±15% of 1101. Across the
track our total `###` count rises toward the reference's on every document it changes and falls on
none. If any document's `###` count overshoots the reference's I have over-applied the rule.

**P4.3 Verdict movement — and this is the exception the brief flags.** `###` is extractable text,
so unlike every other sheets round the gate *can* see it. But the corrected metric counts only
letter-or-digit tokens, and `###` carries neither — so hashing a cell **removes one real word**
(the number) and adds one invisible token. On `ODs-February` that takes ours from 16 610 toward
15 509 against the reference's 15 715, i.e. Δ +895 → Δ −206, **inside** the 2%+3 band of 317.
So its `words` component should flip fail → pass. **The document still fails check 1 on pages**, so
**the scoreboard does not move on it.**

Corpus-wide I predict **verdict movement 0**, with a stated 30% chance of **+1** and a 10% chance
of **−1** (a document currently passing check 2 by cancelling errors, the `Thailand17` shape).
Sheets stays **146 of 171**. Naming this plainly because the brief asked: **I expect no verdict
movement even though this is the one round where the gate could see the change.**

**P4.4 Cross-track.** Any change here is inside `XRay.Office.Spreadsheets`, which words and slides
cannot reach. That is a static argument, and it is only valid if the diff stays inside that
project; if it touches `XRay.Office.Core` or `XRay.Office.Text` I owe a 534-document sweep.

**P4.5** I expect to reach **subject 1 fully, subject 2 to a diagnosis, and subject 3 to a
diagnosis at best**. A precise seat on one beats three gestures, per the brief.

## P5 — what this round will not be able to see

* **`xray-office analyze` and `pdftotext` both read a *token*.** Neither can tell me how many `#`
  glyphs are in the text layer versus how many `###` tokens poppler assembled. Any glyph-level
  claim has to come from reading the PDF's own operators, and I will label it as such.
* **Nothing in the gate can see the grid**, at all. Subject 3's evidence is entirely a stroke
  census; a "146 unchanged" after it is evidence of safety and no evidence of correctness.
* **`bHideGrid` has no PDF trace.** I can see that a rule is missing; I cannot see *why*
  LibreOffice omitted it. Attribution to overflow versus merge is inference unless an authored
  fixture separates them, and separating them is what the fixture is for.
* **Column width in the reference is not directly observable.** I can infer it from the drawn
  grid positions; on a page with no grid I cannot.
* **The 27.2 tree is not 26.2.4.2.** Every `output.cxx` / `output2.cxx` line number above is a
  statement about a binary I am not measuring.
