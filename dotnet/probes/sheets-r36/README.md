# Sheets round 36 — a cell's face is chosen once, and coverage is per character

Base `b2e324b1b`, checked with `git log --oneline -1` before anything was measured. Own worktree,
own snapshotted CLI, `SOURCE_DATE_EPOCH` pinned to 1700000000 on every render.

## The baseline reproduced all four of the brief's figures to the digit

| | brief | measured at `b2e324b1b` |
|---|---:|---:|
| documents matching | 154/171 | **154/171** |
| absolute page error | 74 | **74** |
| exactly-correct page counts | 160 | **160** |
| absolute word error | 27428 | **27428** |

171 rows, no duplicate path, no `ref-failed`, no `ours-failed`. `baseline-parity.tsv` is the sweep.

## What shipped: glyph fallback in the spreadsheet cell text path

A cell's face is resolved once, from the family name its format carries. Coverage is a property of
a *character*. So a workbook whose cells name a Latin face and hold Japanese asked DejaVu Sans to
draw ideographs, and got its missing-glyph box at DejaVu Sans's own `.notdef` advance — invisible
text, and an advance nothing like the full-width ideograph LibreOffice lays out after its own
fallback.

The sheets path measures, breaks and draws through three different objects, so three places had to
learn it:

| Object | Route | What it decides |
|---|---|---|
| `FallbackShaper` (new, `Paperless.Text`) | splices per-face shaping into one `ShapedText` in the primary face's design grid | advances |
| `ParagraphLayouter`, new optional `shaper` | `LineCount`, `Wrap`, `RichLineRanges` | where lines break |
| `SheetText.Append` | itemises for drawing | which glyphs, and which font program is embedded |
| `SheetTextLayout.Measured` | `ItemisationOptions.GlyphFallback` | a rich cell's breaks |

`FallbackShaper` is deliberately **for measuring only**: it splices glyphs from several faces into
one run and does not report which came from where. That is fine for a line breaker, which asks only
for advances, and would be wrong for a painter — which is why `SheetText` itemises for itself
rather than reading the shaper's output.

### The oracle was LibreOffice's own `style:row-height`, not the reference PDF

`seihon_zassi_kikou_20221215.xlsx`, flat-ODF round trip, compared row by row with `GridProbe`:

| | rows disagreeing with `style:row-height` | pages |
|---|---:|---|
| before | **121 of 5159**, every one by exactly 268.8 twips | 83/84 |
| after | **0 of 5159** | **84/84** |

268.8 twips is one line pitch, and the pitch itself was already right — 835.0 = 3 × 268.8 + 28.6
and 1103.8 = 4 × 268.8 + 28.6. That is what says the defect was in the *width* and not in the
metrics, and it is why a change that only moves advances lands exactly.

### Swept reach, and how the prediction did

`PREDICTION.md` was committed before the sweep started: 8–30 documents byte-changed, 1–3 verdicts,
0 on words and slides, and an explicit expectation of landing in the *upper* half of the reach band.

| | predicted | measured |
|---|---|---:|
| sheets renderings byte-changed | 8–30 | **13 of 171** |
| verdicts moved | 1–3 | **1** |
| words renderings byte-changed | 0 | **0 of 200** |
| slides renderings byte-changed | 0 | **0 of 163** |

Both halves landed inside their bands, at the bottom of both, and the stated expectation of the
upper half was wrong. The words and slides figures are measured — every document rendered with both
snapshotted CLIs and byte-compared — rather than argued from "the parameter is optional".

| | before | after |
|---|---:|---:|
| documents matching | 154 | **155** |
| absolute page error | 74 | **73** |
| exactly-correct page counts | 160 | **161** |
| absolute word error | 27428 | **27161** |

`seihon_zassi_kikou_20221215.xlsx` `pages` → **`match`**. Every earlier batch holds.
`sectors-defense-and-aerospace.xlsx` keeps its match and its word count goes 23348 → 23074 against
the reference's 23066.

### Where we still differ from the reference, and it is not the wiring

We pick the fallback face from LibreOffice's own hard-coded generic list and then, failing that,
from anything installed, deterministically by family name. LibreOffice asks fontconfig with the
missing character as a charset. On this machine that means we draw Japanese from `IPAGothic` and
`WenQuanYiZenHei` where the reference draws all of it from `WenQuanYiZenHei`. Every row height on
`seihon` still comes out exact, so the two faces agree about an ideograph's advance — but this is a
standing difference, not a solved one, and a face whose ideograph advance was not one em would show
up as a wrong break rather than as a wrong-looking page.

## Refuted: the three "BIFF cell-format defects" are one defect, and it is not BIFF

The brief named three, measured on page 2 of `7-memento-2015-transports-aeriens-b.xls`
(190/191 pages). All three measurements reproduce. All three explanations are wrong, and they are
wrong the same way — each is **merged-cell decoration**, and none of them is a misread record.

**"A white fill emitted for every cell where the reference emits none."** Same colour, different
rectangles. The operator diff pairs our 12.7 pt-tall fills against reference fills of *101.8 pt*,
which is eight rows, and lists the seven unpaired ones as "only in ours". `ScOutputData::DrawBackground`
merges a run of rows with equal backgrounds (`lcl_EqualBack`) and extends across a merge's columns
before emitting one `DrawRect`. Nothing is filled that should not be, and no `XF` default is
misread.

**"A border colour resolved to palette index 56 where Calc resolves 30."** The index we resolve is
the index the file states. LibreOffice's own flat-ODF export of the workbook carries **both**
colours — 15 cell styles with `fo:border-left: 0.74pt solid #003366` and 20 with `#0066cc` — so its
import agrees with ours. What differs is which cell's border is drawn over a merged range: the
export shows a `table:number-rows-spanned="8"` cell whose style is `fo:border: 0.74pt solid #0066cc`
sitting over data cells whose style is `fo:border: 0.74pt solid #003366`, and the reference draws
one 115 pt vertical in `#0066cc` where we draw eight 12.7 pt segments in `#003366`.

That the reference's line is *one* primitive is itself decisive: `tryMergeBorderLinePrimitive2D`
(`drawinglayer`) merges two collinear border lines only when their `LineAttribute` matches, and
`LineAttribute` carries the colour. So a merged run proves every segment in it had the same colour
on the reference's side.

**"A fill colour off by one adjacent palette entry."** The three `#FF99CC` fills the reference draws
and we do not sit in the same column band as the merged fills above, and are the same class.

### The lead this leaves, stated as a lead

**A merged range's decoration comes from its origin cell and covers the whole range; we apply each
covered cell's own decoration.** `SheetPageDecoration` has no notion of a merge at all — `Edges.Build`
and `DrawBackgrounds` both walk placed rows and columns and ask `SheetFormatting.At(row, column)`.
LibreOffice reaches the origin through `Array::GetMergedStyleSourceCell` and suppresses the interior
edges through `IsMergedOverlapped*` (`svx/source/dialog/framelinkarray.cxx:782-850`), and the BIFF
import first copies the *last* column's right border and the *last* row's bottom border onto the
origin (`XclImpXFRangeBuffer::SetBorderLine`, `sc/source/filter/excel/xistyle.cxx`).

`SheetLayout.MergedRanges` already exists, so the plumbing is there. What is **not** measured is the
corpus reach: the workbook that found it holds 79 merges of two rows or more, and how many of the
other 170 documents draw a border inside one is unknown. It is also fidelity-only on the evidence so
far — the document is 190 pages against 191 and that page is not this.

## Not attempted, and why

- **The 3.4 twips on a non-wrapping multi-line row** (`bStdAllowed = false` for an edit-cell row).
  Still confirmed, still unimplemented, corpus reach still unmeasured.
- **The narrower leg of the solidus rule.** Unchanged: it needs the fitting limit to the character.
- **`aircraft_analysis_2016-04-27.xls`** is the reference's `Unifont` document and reads 44/46 both
  before and after, so whatever it needs is not glyph coverage in a cell. Its fallback never fires.
- **The `/Widths` truncation** — shared PDF writer, not a sheets round.

## Test counts

Core 284, Containers 109, Text 262, Vector 293, Rendering 119, Markup 259, OpenDocument 125,
WordProcessing 720, Spreadsheets **611** (605 before, +6 from `SheetGlyphFallbackTests`),
Presentations 573, Fidelity 550 — **0 skipped** throughout.

The six new tests were verified with `verify-test.sh` on the clean tree: removing the layouter's
shaper fails `TheLineBreakerCountsTheLinesTheCoveringFaceNeeds`, and removing the drawing pass's
itemisation fails `ShapingACellPutsTheCharacterInAFaceThatHasIt`. Two mutations, two different
tests, which is the point of asserting the breaking pass and the drawing pass separately.
