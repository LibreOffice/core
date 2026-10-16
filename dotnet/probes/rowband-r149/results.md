# Round 149 — the row below pays the whole band (seat O84, and the shape of O83)

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
**Corpus** `/home/user/sample-files` (946 manifest rows) and the converted `.odt` column
(337), 1283 renderings a leg. **Tree** `/home/user/libreoffice-core`, branch
`claude/renderer-comparison-artifact-m1g0wy`. Claims are marked **[src]** (the C++ checkout, which
is 27.2.0.0.alpha0+ and *not* the reference binary's source) or **[bin]** (26.2.4.2's own output).

Round 146 established that O83, O84 and O85 are **one** rule. Round 147 closed the painting third
(O85). This closes the **height** third (O84) and moves the model itself, which is what the other
two were symptoms of.

---

## 1. The rule, restated because it is the whole change

> **A horizontal border between two table rows occupies a band whose TOP edge sits on the row
> boundary and which hangs downwards into the row below; its width is the RESOLVED border, and the
> row below pays the whole of it while the row above pays nothing.**

[src] `SwTabFramePainter::Insert` (`sw/source/core/layout/paintfrm.cxx`:3061-3064) sets
`RefMode::Begin` on a cell's two horizontal borders — *"drawn below the reference points"*,
`mfRefModeOffset = +width/2` — and `Centered` on its two vertical ones; `SwRowFrame::Format`
(`tabfrm.cxx`:5338-5430) takes
`nTopPrtMargin = max(lcl_GetTopSpace, pPreviousRow->GetBottomLineSize() + nTopLineDist)` and
`SetBottomMarginForLowers(nBottomLineDist)` — the bottom **padding**, never the bottom line.

[bin] is round 146's, re-measured here rather than quoted: its seven fixtures are re-scored in §3.

**What this tree did instead**, until now: charge each row `(maxStatedTop + maxStatedBottom)/2`,
seed the first grid line half a band below the table's top edge, and draw every band **centred** on
its line. That is *algebraically identical* for as long as every horizontal rule in the table has
one width and the table does not split — which is why one rule seated as three defects and why the
model survived ninety rounds.

## 2. The change

**`TableLayouter`** — `BorderHeight(row)` is gone and three helpers replace it:

| helper | what it is |
|---|---|
| `TopBand(above, row)` | the resolved band at the boundary above `row`: the widest of the row's own top rules and of the row above's bottom rules. For the first row, its own top rule — the table's outer top. |
| `BottomBand(row)` | the outer bottom rule, which no row pays because there is none below it. |
| `OwnTopRule(row)` | the row's **own stated** top rule, which is what a `w:trHeight` floor is raised by. |

and the four places they are used:

- the first row's rectangle starts at the table's frame top — `tops[0] = 0`, not `band/2`;
- `heights[row] = max(content + TopBand, trHeightFloor + OwnTopRule + insets)`, **two different
  border terms in the two branches**, which is why round 131 could fit no linear function of the two
  facing widths ([bin] `trheight.docx`: two arms draw the same 3 pt rule at the same y and differ by
  3.000 pt of height, and a third draws a 3 pt rule and grows by 0.5 — its own statement, every time);
- a cell's content is inset by the whole band above it and by nothing below it;
- the table's total height gains `BottomBand` after the rectangles are built.

**`PageDrawing`** — `WidestAt` is gone, and with it round 147's compensation: with the grid line on
the boundary, every band simply begins at its line and no reference to the widest of them is needed.
`WithWordJoins` becomes `WithSpans`, which settles a vertical's span in one place because **the two
ends of a vertical are not symmetrical**:

- at the top it reaches nothing — the band it must meet begins on the boundary it already starts at;
- at the bottom it crosses the whole band, unless Word's join rule stops it at the table's outline.

[bin] `table-borders.docx`, 26.2.4.2's three verticals: `70.201..161.901` (outer — the first band's
top to the last band's bottom), `70.701..161.401` (interior, full height — below the outer top band,
stopping at the top of the outer bottom one) and `70.701..142.501` (interior, ending on an
*interior* boundary, which it crosses). **All three are a whole rule apart from one another** and
all three are reproduced; getting the second right and the third wrong, which one intermediate cut
of this change did, is how the asymmetry was found.

## 3. The fixtures: 96 of 96, against 60 of 96

Round 146's own seven fixtures, rendered by this tree and scored against **its banked 26.2.4.2
renderings** — one reader for both halves (`score.py`; round 148 spent a whole agent reconciling two
instruments each written for one side). A whole-table displacement and a wrong band shape are
reported separately, and a page whose band counts differ is not scored at all.

| fixture | shift, base | shift, head | shape, base | shape, head |
|---|---|---|---:|---:|
| `align` | −2.551..0.824 | **−0.051..−0.001** | 17/25 | **25/25** |
| `double` | −0.138..1.949 | **−0.063..−0.026** | 14/28 | **28/28** |
| `height` | −1.801..0.474 | **−0.051..−0.026** | 20/29 | **29/29** |
| `height-nosettings` | −1.801..0.474 | **−0.051..−0.026** | 20/29 | **29/29** |
| `trheight` | −1.751..0.499 | **−0.001..−0.001** | 9/14 | **14/14** |
| `split` | *not scoreable* | −12.551..−0.001 | — | 46/48 |
| `splitnil` | *not scoreable* | *not scoreable* | — | — |

Within 0.1 pt, and the residual shift of 0.026–0.063 pt is the two PDF writers' own text-origin
constant, which `dotnet/CLAUDE.md` already records at that magnitude. Band **thickness** was exact
on both legs everywhere (round 147 had already settled the widths).

`split` and `splitnil` are **O83** and are deliberately not closed: `splitnil` is round 146's one
measured exception — a border *copied* across a cut by `InsertFollowTopBorder` really is drawn
centred and really does charge no height — and `SliceRow` cannot see the row above it, so a
continuation part is charged the row's own stated top rule with a comment saying so.

## 4. Reach and confinement: 372 of 1283, and 0 of 609 outside word processing

Our half of the whole corpus and of the converted `.odt` column rendered twice, at the round's base
and with the change, under `SOURCE_DATE_EPOCH=0`, **one output directory per document** keyed on the
whole path (`sweep.sh`). 1283 renderings a leg, **0 failures on either**.

| track | moved | of |
|---|---:|---:|
| words `docx` | 153 | 271 |
| words `doc` | 26 | 66 |
| converted `odt` | 193 | 337 |
| **slides `pptx`/`ppt`** | **0** | 302 |
| **sheets `xlsx`/`xls`/`xlsm`** | **0** | 307 |

**372 movers, 911 byte-identical.** The zero on the other two tracks is the layering holding:
`TableLayouter` and `PageDrawing` are `XRay.Office.WordProcessing`, and a deck's and a sheet's tables
are laid out elsewhere. It is worth stating as a measurement rather than as an inference, because
this change moves more than half the words track and a leak would be easy to miss among that.

## 5. Do the movers get better? Yes, and by a lot — measured against 26.2.4.2

The 179 words movers scored against the banked 26.2.4.2 renderings (`sweepscore.py`, the same reader
for both halves, 0.1 pt):

| | base | head |
|---|---:|---:|
| comparable bands | 6658 | **7320** |
| **absolute** — our band top within 0.1 pt of the reference's | 3187 (**47.9 %**) | 5478 (**74.8 %**) |
| **shape** — the same after removing each page's median displacement | 4505 (**67.7 %**) | 6291 (**85.9 %**) |
| bands on count-mismatched pages, not scored | 49851 | 49189 |

**It is not winning by drawing fewer bands**: the comparable set *grew* by 662 and the unscoreable
set shrank by 662, so 662 pages' band counts came into agreement as well.

## 6. No gate column moves, and the seat predicted that it would

`OPEN-ISSUES.md` said of O84: *"This is the one of the three that moves page counts, so it is the one
to gate: O78 is a 1.5 pt knife-edge that a previous attempt made worse."* **Measured, it moves none.**

Over the 179 words movers against the banked reference:

- **page counts: 171 of 179 page-exact before, 171 after.** Not one document's distance from the
  reference changed, in either direction.
- **alphanumeric characters: changed on 0 of 179 documents.**

The reason is arithmetic rather than luck: for a table whose horizontal rules all have one width the
old and new charges sum to the same table height — the row above loses half a band and the row below
gains half — so only the *distribution* within the table moves, and O78's knife-edge is untouched.
**The prediction in the seat was wrong and is corrected rather than quietly dropped.** It also means
this change cannot be scored by the gate at all, which is the `w:pgBorders` argument again: the
evidence is the band agreement in §5 and the confinement in §4.

## 7. One test asserted the old model as a rule

`TableBorderSpaceTests.TheGridLinesThemselvesDoNotMove` held that *"the stroke is drawn along the
cell rectangle's edge, and the rectangle's top is half a band below the table's"*, and its remark
said pinning it was *"what stops the inset above from being paid for by moving every rule in every
table half a border down the page"*. That is exactly the defect. It is rewritten to the measured
rule as `AGridLineSitsOnTheBoundaryAndItsBandHangsBelow`, with the citation and the reason the old
form survived — not deleted, because the quantity it pins is still worth pinning.

The other 2033 WordProcessing tests pass unchanged, and `XRay.Office.Fidelity.Tests` returns to its
known baseline of 10 failures of 552 (the advance-truncation family `dotnet/CLAUDE.md` records as
left failing on purpose). An intermediate cut of this change took that to 20 — the extra ten were
all `table-borders.*`, and they are the vertical-span asymmetry of §2, found because the suite
caught it.

## 8. What this does not establish

- **O83 is still open** and `splitnil` still scores nothing: a page cut needs the rule at the cut,
  which `SliceRow` cannot compute from the row it is handed. §3 says what is charged instead.
- **The 662 pages whose band counts came into agreement were not examined individually.** They are
  reported as a count because it is the honest way to say the comparable set grew; no claim is made
  about any one of them.
- **The `.odt` column's 193 movers were not scored against a reference**, only diffed between legs —
  there is no banked 26.2.4.2 rendering of that column in this container and rendering one costs
  about forty minutes. The words track's 179 are the measured half.
- **The residual 25.2 % of bands that are still not within 0.1 pt absolutely** is not characterised.
  Part of it is O83 and part is O78/O82; which part is which is a round of its own.
