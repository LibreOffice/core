# Round 152 — a split row is ruled once at the cut, by its bottom rule (seat O83)

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
Claims are marked **[src]** (the C++ checkout, 27.2.0.0.alpha0+, *not* the reference binary's
source) or **[bin]** (26.2.4.2's own output). This is the last of the three seats round 146
established are one rule: O85 closed in 147, O84 in 149, O83 here.

---

## 1. Where round 149 left it

Round 149 moved the model to *the row below a boundary pays the whole resolved band, whose top edge
sits on the boundary and hangs downwards*, and every one of round 146's fixtures came out exact
except the two that split. It said why: *"a page cut needs the rule at the cut, which `SliceRow`
cannot compute from the row it is handed"*, and charged a part the row's own **top** rule with a
comment saying so.

Re-scored at HEAD before anything changed, `split.docx` was 46 of 48 bands within 0.1 pt with a
12.551 pt shift on page 2, and the dump shows exactly where: **all 30 interior boundaries on page 1
are exact and only the cut is wrong**, at 769.750 against 759.201.

## 2. Three errors, separable, each worth a line or a rule

**(a) The part was charged its own top rule.** `split.docx` states `w:top nil` and a 1 pt
`w:bottom` on every cell, so a part was charged **nothing**. [src] the rule at a cut is the cell's
*bottom* one: `lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines`
(`sw/source/core/layout/paintfrm.cxx`:2815-2833, used at :3089) has the first row of a follow table
draw its **top** line from the cell's **bottom** border style, and the master's own foot draws that
same rule. Charging `BottomBand(row)` recovered 1.0 pt of the 12.551.

**(b) The part did not charge the boundary band *above* it.** A part occupies that band as well as
its own content — the row's rectangle starts at the boundary and the content starts a band below it
— and `HeightAt` measures from the *content*. Tracing the chosen candidate showed `needed = 312.850`
against `room = 313.150` on a deadline of 771.050, so the part ran to 771.750 and overran the text
area by a band; one line too many fitted. `SliceRow` now takes the band from the caller —
`TableLayouter.BoundaryBand(table, row)`, since it is handed one row and cannot resolve it — and
that removed the remaining 11.55, a whole line.

**(c) A part that *finished* its row still charged a band at its foot.** The boundary below it is an
ordinary one and the row after it pays for it as `TopBand`, so it was ruled twice: [bin] bands at
118.050 and 119.050 where 26.2.4.2 draws one at 118.101, with everything below pushed down 1.0 pt.
Only a part the page **cuts** pays at its foot.

And the drawn extent is not the reported height: the band at the cut hangs *below* the last line
rather than inside the rectangle, exactly as the table's own outer bottom band does in `LayOut`.
Building the rectangles to the full height drew the cut's rule one band too low.

## 3. [bin] The result

| fixture | before | after |
|---|---|---|
| `split.docx` | 46/48, shift −12.551..−0.001 | **48/48, shift −0.051..−0.001** |
| `align`, `double`, `height`, `height-nosettings`, `trheight` | exact | exact, unchanged |

**And the seat's own witness is exact at all four of its own boundaries**, with the doubled lines
gone — one band per boundary now:

| | O83 records this tree drawing | 26.2.4.2 | now |
|---|---|--:|--:|
| page 9, head | 120.900 **and** 121.150 | 120.901 | **120.9** |
| page 10, foot | 721.600 **and** 721.850 | 721.601 | **721.6** |
| page 10, the cut | 768.50 | 768.501 | **768.5** |
| page 11, the cut | 72.00 | 72.001 | **72.0** |

## 4. What is left: the copied border, and it is the one place the reference centres a band

`splitnil.docx` is still not scoreable — 26.2.4.2 draws **two** bands per page and we draw one. Its
interior edges are `nil` on both sides, which is the only arm that reaches `InsertFollowTopBorder` /
`InsertMasterBottomBorder`: the reference copies a border from the *other end of the table* onto the
cut and draws it **centred on the table edge, charging no height** — page 1's cut band at
765.401..768.401 around a frame edge of ≈766.85, page 2's at 69.401..72.401 around 70.901.

[src] round 146 established why it is centred where everything else is top-aligned:
`InsertFollowTopBorder`:2924-2926 and `InsertMasterBottomBorder`:2999-3001 construct the
`SwLineEntry` from the `Style` **before** calling `SetRefMode(RefMode::Begin)` on it, so the entry
keeps the `Centered` default. **We draw no such band at all.** Reach uncensused; it needs a table
whose cells state neither edge at the cut, which round 131's `PBNN` arm is and `review-welsh` is not.

## 5. Mutation pin

`mutate.sh`, one arm per error, restoring with `cp` + `touch`:

| arm | reverted | tests red |
|---|---|---:|
| M1 | a part is charged its own top rule | 2 of 4 |
| M2 | a part does not charge the boundary band above it | 2 |
| M3 | a part that finishes its row still charges a band at its foot | 1 |

**M3 needed a sharper test than counting.** The first cut of the suite asserted that no two bands sit
within 2 pt of each other, which the PDF shows M3 violating — and the test passed anyway, because
**the drawing sink and the written PDF do not agree about how many paths that doubled boundary
produces**. Asserting the reference's own position for the first interior boundary of page 2 pins it
in both. The count assertion is kept beside it; it is the one that states the seat's own symptom.

**And M3's first cut did not compile** — `if (false)` is unreachable code, which `TreatWarningsAsErrors`
makes a build failure. Round 150 lost an arm to exactly that and the harness now greps for compiler
errors, which is how this one was caught rather than banked as a pin.

## 6. Confinement and reach: 31 of 1927 — and the corpus result is mixed

Our half of the corpus and of all three converted ODF columns rendered twice under
`SOURCE_DATE_EPOCH=0`, one output directory per document. **1927 renderings a leg, 0 failures.**

| family | ext | moved | of |
|---|---|---:|---:|
| words | docx | 5 | 271 |
| words | doc | 1 | 66 |
| rtf column | rtf | 9 | 337 |
| odt column | odt | 9 | 337 |
| ods / sheets / slides | — | **0** | 916 |

**31 movers, 1896 byte-identical.** Nothing outside word processing, as the layering requires.

**Scored against 26.2.4.2 on baseline position, the 31 are 11 closer, 9 further and 11 level, and the
median mean \|dy\| goes 10.533 → 11.225 pt — very slightly worse.** That is the honest figure and it
is not an improvement.

**And one document gains a page**: `review-welsh-…-mandelson.rtf` goes from 14 to 15 where 26.2.4.2
has 14. Its `.odt` twin stays at 14 and both improve on baseline error (40.38 → 32.71, 54.19 →
49.49). The page is not explained here.

### Why this is shipped anyway, and what would justify reversing it

The rule is right where it is measured: the authored fixture is **48 of 48 bands exact against 46 of
48**, the seat's own witness is exact at all four of the boundaries the seat names with the doubled
lines gone, and each of the three errors is separately pinned. What is *not* established is that
reserving those bands is a net gain on real documents, and the corpus says it is roughly neutral with
one page lost.

Two things make that the expected shape rather than a surprise. A part now reserves height it did not
reserve before, so any document already near a page boundary can tip — and round 152's re-measurement
of **O78** found exactly this: closing O84 moved that document's error from 1.50 to 5.00 pt, because
its rows state no bottom border and there is no half band for the row above to give back. Round 149's
prediction that O84 left O78 untouched was wrong, and the same caution applies here.

**A single page regression on one converted file, against a rule that is exact on its fixture and on
its witness, is worth shipping and recording rather than hiding** — but if a later round finds a
second page regression from this change, the reservation is over-charging somewhere and this is the
first thing to re-examine. The first cut charged *every* part the boundary band above it; correcting
the follow part to pay the **cut's** rule instead removed one such regression already and did not
remove this one.
