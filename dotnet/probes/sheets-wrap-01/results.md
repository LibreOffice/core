# sheets-wrap-01 — how LibreOffice breaks a token with no break opportunity

Measured against the installed **LibreOffice 26.2.4.2**, which is the binary that made the
banked references. The C++ tree in this checkout is 27.2-alpha and was read for mechanism only.

`prediction.md` beside this was committed before any of it was measured. It is scored at the end.

## The question

`sheets/batch-006/xlsx/Published_Issuances_2024.xlsx` failed the word gate at 457/479. Every one
of 22 rows holds a URL in a wrap-enabled `LINK` column; the reference wraps each onto two lines,
breaking **mid-token** (`…Published%2` ⏐ `0Issuances…`), and we drew one line, shortened at the
cell edge. A blind reviewer had established from the rendered pair that column widths, row
heights, row count, body font metrics and the whole of the `DESCRIPTION` column's wrapping were
identical — so wrap width, measurement, autofit, row height and font were all ruled out, and what
remained was the break rule itself.

## What the file actually holds

`xl/worksheets/sheet1.xml` carries 22 sheet-level `<hyperlink ref="Fn" r:id="…"/>`. Every `F` cell's
`xf` (9, 11, 19, 20) sets `wrapText="1"`. Calc's OOXML import replaces such a string cell with an
**edit cell holding a single `SvxURLField`** — `WorksheetGlobals::insertHyperlink`,
`sc/source/filter/oox/worksheethelper.cxx:1062`. The content node's string is then **one**
`EE_FEATURE_FIELD` character; the URL a reader sees is the field's *representation*, which the node
does not contain.

## The probe

`make-probe.py` builds `sheet-wrap-fields.xlsx`: six wrap-enabled cells in one 30-character column,
the same three strings once plain and once carrying a sheet-level hyperlink, at a row height with
room for every line. It is the probe the reviewer proposed — a long space-free token and a long
spaced token, plain and linked — and it separates the two candidate explanations outright.

The reference's own lines, read from its PDF:

| # | cell | reference |
|---|---|---|
| 1 | URL, plain | `https://www.bsp.gov.ph/` ⏐ `Regulations/` ⏐ `Published%20Issuances/Images/` ⏐ `M-2024-039.pdf` |
| 2 | URL, **linked** | `https://www.bsp.gov.ph/Regulation` ⏐ `s/Published%20Issuances/Images/M` ⏐ `-2024-039.pdf` |
| 3 | `AAAA…PPPP`, plain | `AAAABBBBCCCCDDDDEEEEFFFFGGG` ⏐ `GHHHHIIIIJJJJKKKKLLLLMMMMNNN` ⏐ `NOOOOPPPP` |
| 4 | `AAAA…PPPP`, **linked** | identical to row 3, character for character |
| 5 | `alpha bravo …`, plain | `alpha bravo charlie delta echo` ⏐ `foxtrot golf hotel india juliet kilo` ⏐ `lima` |
| 6 | `alpha bravo …`, **linked** | `alpha bravo charlie delta echo foxtr` ⏐ `ot golf hotel india juliet kilo lima` |

X-Ray.Office before the fix drew rows 1, 3 and 5 exactly as above and rows 2, 4 and 6 as **one
unbroken line each**.

## The rule

**A field is atomic to the breaker and still divisible by the chop.**

`ImpEditEngine::ImpBreakLine` hands the *content node's* string to the break iterator
(`editeng/source/editeng/impedit3.cxx:2080-2083`). For a field that string is one character, so the
iterator offers no interior opportunity and EditEngine falls through to
`// No separator in line => Chop!` (`impedit3.cxx:2236-2247`), cutting at `nMaxBreakPos` — the last
character position still under the remaining width.

Row 6 is the decisive arm. **A space is not a break opportunity inside a field**: LibreOffice cuts
`foxtrot` in half with a blank one character away. No break-opportunity rule of any kind — Unicode's,
a URL-aware one, a punctuation-aware one — can produce that. Row 4 is the control: a token with no
opportunity in it comes out character-identical linked and unlinked, because the chop was all
either path had. So the character-break fallback in `XRay.Office.Text` was present and correct all
along, and the seat was the run being treated as atomic.

### The C++ comment says the opposite of what the binary does

`DrawEditParam::readCellContent` (`sc/source/ui/view/output2.cxx:2560-2567`) reads:

```
if ( mbBreak && !mbAsianVertical && pData->HasField() )
{
    //  Fields aren't wrapped, so clipping is enabled to prevent
    //  a field from being drawn beyond the cell size
    rWrapFields = true;
}
```

That comment is describing the **clip** it switches on at `:3239`, not a suppression of breaking:
`mbBreak` is untouched, so the paper stays the column's width and the text still has to fit it. The
port had taken the comment at its word — `SheetTextLayout.cs`, `Breaks(format, isValue) &&
!cell.IsField` — and that cost 22 rows their second line. **Another instance of the standing rule:
read the source for the mechanism, measure the binary for the behaviour.**

### Row height is a different rule, and it really does disagree

Converting the same probe with automatic row heights (no `ht`, no `customHeight`) and reading
LibreOffice's own `style:row-height` out of the `.fods`:

| cell | row height | lines |
|---|---|---|
| URL, plain | `0.6425in` | four |
| URL, **linked** | `0.1756in` | **one** |
| `AAAA…`, plain | `0.4866in` | three |
| `AAAA…`, **linked** | `0.1756in` | **one** |

So Calc's optimal-height pass measures a field cell at a single line whatever the column width is,
and the drawing path then wraps it and lets it overflow the row it did not size. The gate in
`SheetOptimalRowHeights` is therefore correct and was **kept**; only its comment was rewritten, since
it had justified itself with "a field is never broken across lines", which is false.

## Two further facts the fix exposed, both found by blind review

Neither was visible before, because before the fix there was never a second line.

Two fresh subagents were given the rendered pair with no repo access. Both independently reported
that our second line was **painted over the row beneath it** in four short rows where the
reference's is cut off. Confirmed in the PDF's own operators rather than in the raster, per the
`page-vision` rule — and the first attempt to do so found *zero* clip rectangles in the reference,
because the pattern was written for `re W n` and **LibreOffice writes the even-odd form `re W* n`**.
A pattern that misses that reports a reference which never clips.

### 1. A wrapping field is clipped to its cell, vertically as well as horizontally

`ScOutputData::Clip` (`output2.cxx:3442-3445`) ORs `bWrapFields` straight into `bClip` before
anything is measured, so the "don't clip for text height when printing rows with optimal height"
branch below it never gets to say otherwise, and the rectangle is `aAreaParam.maClipRect`, which the
text never grew.

Read out of the reference's content stream for `Published_Issuances_2024.xlsx`: **22 clip
rectangles**, one per link cell, each `402.096..534.824` wide and each exactly as tall as its row —
19.006, 12.939, 6.872, 28.689 pt — including the tall rows where nothing overflows. We had grown
four of them to 12.671 and 14.087 to fit the text. All 22 now agree with the reference to within
0.017 pt.

### 2. A field's lines are pitched by the face's ascent, not by its line height

`make-pitch-probe.py` builds sixteen single-cell workbooks — Calibri, Arial, DejaVu Sans and Times
New Roman at 8, 10, 14 and 20 pt — each hyperlinked, wrap-enabled, and holding a run of `X` so that
every line chops at the same glyph and the gap between two lines' bounding boxes is the pitch
exactly. All sixteen **line counts** already agreed; every pitch was wrong.

| face | pt | reference | ours before | ours after | `hhea` ascent |
|---|---:|---:|---:|---:|---:|
| Calibri → Carlito | 8 | 7.597 | 9.758 | 7.611 | 1950/2048 em |
| | 10 | 9.496 | 12.215 | 9.527 | |
| | 14 | 13.294 | 17.094 | 13.333 | |
| | 20 | 18.992 | 24.429 | 19.055 | |
| Arial → Liberation Sans | 8 | 7.200 | 8.930 | 7.236 | 1854/2048 em |
| | 10 | 9.099 | 11.179 | 9.058 | |
| | 14 | 12.699 | 15.644 | 12.677 | |
| | 20 | 18.113 | 22.358 | 18.117 | |
| DejaVu Sans | 8 | 7.398 | 9.305 | 7.420 | 1901/2048 em |
| | 10 | 9.298 | 11.648 | 9.288 | |
| | 14 | 13.011 | 16.301 | 12.998 | |
| | 20 | 18.595 | 23.296 | 18.576 | |
| Times New Roman → Liberation Serif | 8 | 7.087 | 8.852 | 7.123 | 1824/2048 em |
| | 10 | 8.901 | 11.081 | 8.917 | |
| | 14 | 12.501 | 15.507 | 12.478 | |
| | 20 | 17.802 | 22.162 | 17.833 | |

Every reference figure is the face's `hhea` ascent quantised to a tenth of a point. Worst residual
after the fix: **0.063 pt**, against 1.7–5.4 pt before. Every other cell keeps ascent-plus-descent,
which is `ScDrawStringsVars`'s `aMetric.GetAscent() + aMetric.GetDescent()` (`output2.cxx:734`) and
what `SheetFonts.LineHeightAt` already had right.

**The two are coupled and neither is shippable alone.** Fixing the clip without the pitch would
have cut the second line off in the *majority* of rows, where the reference shows it whole; fixing
the pitch without the clip leaves the overprint in the short rows.

**Scope, stated because it is the risky part.** What was measured is *fields*. Whether the other
cells Calc sends to an EditEngine — a rich cell, one holding a hard break — share the ascent-only
pitch is **untested here and deliberately not assumed**: they keep `LineHeight`, which is what the
corpus was fitted against. That is the obvious next probe and it is not this one.

## What changed

| file | change |
|---|---|
| `Layout/SheetFieldBreaker.cs` | new: an `ILineBreaker` offering the end of the text and nothing else |
| `Layout/SheetTextLayout.cs` | `breaks` no longer gated on `IsField`; `Wrap` takes an `atomic` flag and uses the breaker above; a field's line pitch is its ascent; a field's clip is its cell |
| `Layout/SheetOptimalRowHeights.cs` | comment only — the single-line rule is right, its stated reason was not |

## Reach

Rendered every document of all three tracks twice, at HEAD and with the change, with
`SOURCE_DATE_EPOCH` pinned, and diffed byte for byte.

| track | documents | changed |
|---|---:|---:|
| sheets | 171 | **17** |
| words | 200 | **0** |
| slides | 163 | **0** |

Words and slides were checked and are byte-identical; the change is inside
`XRay.Office.Spreadsheets` and reachable only through a spreadsheet cell that resolves to a hyperlink
field.

Gate verdicts against the banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`,
reused rather than re-rendered: **154 → 155 match**, and exactly one verdict moved —
`Published_Issuances_2024` from `words` to `match`. No document regressed. The other sixteen movers
keep their page counts; where their word counts moved they moved toward the reference
(`FMMO_NMPF_37C` 213 → 215 against 215; `AIM_OPR_LIST` 22007 → 22015 against 22013), and the rest
changed only ink, which no word count can see.

`Published_Issuances_2024.xlsx` is now **1/1 pages, 479/479 words, 0 unembedded, `match`**, and its
line texts match the reference character for character including the two rows in a larger face that
break one character later (`…Publishe` ⏐ `d%20Issuances…`).

## Batch regression

`sheets/batch-006` alone: **8 of 10**, the two failures being the Lease-Transition workbooks that
`TODO.raster-ceiling.md` records as an unwinnable gate ceiling.

`sheets/batch-001` through `006`: **58 of 60** on the first run and **57 of 60** on the second.
The difference is not ours — see below.

## Contradicting the brief

**`fse_identification_form.xlsx`'s verdict is decided by the reference, not by us.** The brief lists
it as one of four failures with another agent on it. Our rendering of it is byte-identical before
and after this change, and stable at 440 extractable words. The *reference* is not: three
consecutive conversions of the same file by the same 26.2.4.2 binary on the same day gave **440,
427, 427**. The banked reference holds 427. So the document scores `match` or `words` depending on
which reference render it is compared against, and the 60-document total is 58 or 57 for the same
reason. The workbook sets `calcPr iterate="1" iterateCount="25"` — iterative calculation, which
converges differently run to run — which is a plausible mechanism but is not itself measured here.
Anyone working that document should establish a stable reference before treating a word delta as a
defect.

**Reach was larger than predicted** — 17 of 171 against a predicted 1 to 4. The prediction counted
documents whose *gate verdict* would move and the measurement counts documents whose *bytes* move;
the clip and the pitch change ink in thirteen more documents that the gate cannot see. The
prediction should have said which it meant.

**The border defects on the same page do not share a seat**, as predicted. Wrapping, pitch and
clipping decide where glyphs go; the grid rules are drawn by `SheetGrid`/`SheetDecoration`, which
never consults `IsField`. The v2 crop reviewer, blind, reported the same rules present in both
halves at the same weights, with the caveat that a sub-pixel difference is below what the image can
resolve — so that item is neither confirmed nor refuted here and remains open.

## Prediction, scored

| # | claim | outcome |
|---|---|---|
| 1 | the C++ comment describes the clip, not a suppression of wrapping; 26.2.4.2 wraps a field | **right** |
| 2 | the break inside a field is a pure character chop, honouring no opportunity — not `/`, `-` or `%` | **right**, and the spaced arm made it sharper than predicted |
| 3 | only the linked arms fail; the plain space-free arm already passes | **right**, all four arms as predicted |
| 4 | words and slides untouched; 1–4 sheets move | **half right** — 0 and 0 as predicted, 17 sheets against 1–4 |
| 5 | the border defects do not share a seat | **right**, within what a blind reading can establish |

What the prediction missed entirely: that a field is clipped to its cell, and that its lines are
pitched by the ascent. Both were found by blind reviewers looking at the fixed page, not by
reasoning about the source — which is the case for handing the pair to someone who has not seen it.

## Method note, recorded because it cost real time

**`git stash` is shared across worktrees.** Establishing the pre-change baseline by stashing meant a
parallel agent's `git stash push` landed on top between the push and the pop, so the pop applied
*their* change to *this* worktree. The whole-track reach sweep that followed measured their diff —
68 of 171 sheets, none of them the target — and read as a fix that had done nothing. Their stash was
recovered by `git stash store` on the SHA the pop printed. Use a scratch copy or a second build
directory instead; never `git stash` in a shared checkout.
