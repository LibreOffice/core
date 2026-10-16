# Round 156 — O105: a `.doc`'s covered cell carries the merge master's borders

## 0. What this round is

Round 154 gave all four readers a carrier for the rules a cell a vertical merge covers states, and
closed the seat in DOCX, RTF and ODF. WW8 closed only two of its six failing arms, and the round
recorded the rest as **O105**: *"a `.doc`'s covered cell carries borders that the file does not state,
so reading them is not enough"* — on four arms 26.2.4.2 charged a 1 pt top rule that no cell in the
file states.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, base `b322e47b3` *(round 155)* |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`** |
| date | 2026-09-19 |

---

## 1. [bin] The instrument is the reference's own view of the file, and it answers in words

`--convert-to fodt` on the `.doc` prints every cell's resolved borders and every cell's style name.
Over round 154's fixture 7, converted to `.doc` by 26.2.4.2 itself
(`data/doc-covered-borders.txt`):

| the lower-left cell | its style | its border |
|---|---|---|
| an ordinary cell (the nine `M` arms) | `TableN.A2` — **its own** | what the file states: `none`, or 1 pt |
| a covered cell (the nine `V` arms) | `TableN.A1` — **the master's** | `1pt solid` on all four sides, in every arm |

A covered cell comes out carrying **the merge master's own cell style**. That is consistent with two
rules — *the master's borders* and *the thicker of the two* — and one attribute separates them.

## 2. [bin] Two arms, and the master wins in both directions

`make-master.py` builds four two-row tables, each on a `w:trHeight` floor of 450 twips (22.50 pt),
one per page:

| arm | the master states | the covered cell states | 26.2.4.2's row height | what it means |
|---|---|---|--:|---|
| **A** | 3 pt | `nil` | **25.50** = 22.5 + 3 | the master's, where the cell states nothing |
| **B** | 1 pt | 3 pt | **23.50** = 22.5 + 1 | the master's, where the cell states **more** |
| C | 3 pt | `nil`, **no merge** | 22.50 | the control: its own |
| D | 1 pt | 3 pt, **no merge** | 25.50 | the control: its own |

**Arm B is the whole of it**: the thicker-of-the-two reading predicts 25.50 there and the reference
draws 23.50. Before this round we drew **21.50** on A and **25.50** on B — the covered cell's own
statement, exactly backwards, which is round 154's WW8 half doing the wrong thing with the right
carrier.

## 3. The change

`ResolveVerticalMerges` already walks upwards to find the merge's owner so it can count the span, so
this is one line beside that: `cell.Borders = owner.Borders`. Nothing draws a covered cell, so it
reaches exactly one number — the row's height, through `PageTableRow.CoveredTopRule`.

## 4. [bin] All four fixtures, all four formats

Round 154's four fixtures converted to `.doc` by the reference:

| | before r154 | after r154 | **after r156** |
|---|--:|--:|--:|
| arms differing on `.doc` | 6 of 34 | 5 of 34 | **0 of 34** |

With round 155's ODF half and this, **all four word-processing formats agree with 26.2.4.2 on all 34
arms**: `.docx` 34, `.rtf` 34, `.odt` 34, `.doc` 34.

## 5. The tests and the pin

`words-vmerge-master-borders.doc` is §2's fixture — 13 kB, four arms, one per page — and its
expectations are 26.2.4.2's own rendering of it. `Ww8CoveredCellBordersTests` is four tests, two of
them the controls that stop the rule being read as *"a lower cell's top rule is ignored"*.
Mutation-pinned (`mutate.sh`) against three rival rules:

| arm | result |
|---|---|
| the covered cell keeps its own borders (round 154's state) | **2 of 4 failed** |
| the thicker of the two | **1 of 4 failed** — arm B alone |
| no borders at all (before round 154) | **2 of 4 failed** |
| base (the master, always) | 4 passed |

## 6. [bin] Confinement and reach: 4 of 1620, all `.doc`

| family | moved | of |
|---|--:|--:|
| words `.doc` | **4** | 66 |
| everything else | **0** | 1554 |

`confinement.txt`. Per span, over the four movers (`score2.py`): **529 closer to 26.2.4.2 and 84
further**, 3 documents net better and 1 worse.

| document | spans moved | closer | further |
|---|--:|--:|--:|
| `150_5300_13_chg10.doc` | 329 | **329** | 0 |
| `150_5300_13_chg12.doc` | 143 | **143** | 0 |
| `foca_form_1.doc` | 2 | **2** | 0 |
| `P200904290238_0238_51880.doc` | 140 | 55 | **84** |

## 7. [bin] The one document that worsens — and a wrong seat, withdrawn

`P200904290238_0238_51880.doc` goes from a mean baseline distance of 7.84 pt to 17.20, and the shift
is on **pages 2 and 3 only** — page 1 *improves*, from −8.12 to −5.93 — so a table early in the
document grew by about 18 pt. It holds **merges of depth 2, 3, 4 and 7** (`xray-office extract
--format xhtml`, counting `rowspan`), and depth is the attribute this round's arms did not vary.

***The first version of this section said a deep merge whose master states a bottom rule leaves the
later covered rows short, and seated it as O107. That is withdrawn: the rendering it rested on was
made with the BASE binary and attributed to the head one.*** Re-rendered with the binary this round
ships, `deep.doc` and `deep2.doc` agree with 26.2.4.2 on **all four rows of both arms of both
files** — merged and unmerged, with the master's bottom stated and with it `nil`.
`data/deep-recheck.txt`:

| fixture | arm | 26.2.4.2 | ours |
|---|---|---|---|
| `deep` (master states a 3 pt bottom) | merged | 25.5, 25.5, 25.5 | **25.5, 25.5, 25.5** |
| | control | 25.5, 19.5, 22.5 | **25.5, 19.5, 22.5** |
| `deep2` (master states no bottom) | merged | 25.5, 25.5, 25.5 | **25.5, 25.5, 25.5** |
| | control | 22.5, 22.5, 22.5 | **22.5, 22.5, 22.5** |

So the rule holds at depth four as well as at depth two, and the corpus document's 18 pt is
**unexplained**. O107 is re-seated as that one document with no mechanism attached, and the two
instrumented readings that did survive the correction are worth keeping for whoever takes it: the
WW8 reader's covered rules are identical for the two fixtures (top 3.00, bottom 1.00 on every
covered row), and `TableLayouter.Measure` computes 25.50 for all four rows of both.

**The lesson is the one CLAUDE.md already states and this round paid for anyway**: a rendering is
evidence about the binary that produced it, and a probe directory holding `ours/` and `ours-base/`
side by side will hand you the wrong one if you do not re-render after a rebuild.


## 8. The suite

| project | result |
|---|---|
| `XRay.Office.WordProcessing` | **2088** passed — 2084 at the round's base plus this round's 4 |
| `XRay.Office.Core` / `Text` / `Vector` / `Containers` | 591 / 750 / 309 / 109 passed |
| `XRay.Office.Spreadsheets` / `Presentations` / `Markup` | 1484 / 1205 / 259 passed |
| `XRay.Office.Fidelity` | **10 failed of 552**, 0 skipped — the base figure exactly, and the known set |

Build: 0 warnings, 0 errors.
