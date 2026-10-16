# Round 147 — a horizontal table border hangs below the boundary (seat O85)

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. The C++ tree read here is
`/home/user/libreoffice-core` at **27.2.0.0.alpha0+**, which is *not* that binary's source;
**[src]** marks a reading of it and **[bin]** a measurement against 26.2.4.2's own output.

`probes/tablerow-r146/` is where this round's rule, fixtures and censuses come from. This
directory holds only what the *fix* needed.

## 1. The rule, and why two thirds of it are deliberately not in this round

**[src]** `SwTabFramePainter::Insert` (`sw/source/core/layout/paintfrm.cxx`:3061-3064) sets
`RefMode::Begin` on a cell's two **horizontal** borders and `RefMode::Centered` on its two
**vertical** ones. `Begin` is documented as *"horizontal lines are drawn below the reference
points"* (`include/svx/framelink.hxx`:40-45) and implemented as `mfRefModeOffset = +width/2`.
The height half of the same statement is `SwRowFrame::Format` (`tabfrm.cxx`:5338-5430), where
`nTopPrtMargin` takes the **previous** row's bottom line size and the bottom margin is padding
only — so the row below pays for the whole band and the row above pays nothing.

**This tree's model is algebraically identical to that for as long as every horizontal border in
a table has one width and the table does not split**: half the border charged to each row puts
the grid line in the middle of the band, and a band centred on that line occupies exactly the
rectangle the reference hangs below the boundary. Round 146's agent established that, and it is
why the family seated as three separate defects.

So the three seats are three places where the algebra comes apart, and only the first is a
*drawing* question:

| seat | what comes apart | in this round |
| --- | --- | --- |
| **O85** | one boundary, two widths — the bands no longer share a rectangle | **fixed** |
| **O83** | a page cut — the continuation's first line has no row above to halve with | left |
| **O84** | the height — which row pays, and a `w:trHeight` floor charges the *stated* line while the band is the *resolved* one | left |

O83 and O84 need the layout rather than the painter, and O84 moves page counts, so it is gated
behind a words-track page-count comparison. Both keep their seats with the closed form measured.

## 2. What changed

`PageDrawing.WidestAt` is the widest band at each horizontal grid line, and `DrawBorders` now
begins every band on that line at `At − widest/2` rather than at `At − own/2`.

**For a boundary of one width the two are the same expression**, which is the confinement
guarantee and is structural rather than measured: `widest == own` there, so not a twip moves on
any table whose boundaries agree with themselves. A vertical border is untouched and stays
centred, which is `RefMode::Centered`.

The `double` arm needed nothing: `BorderRules`:169 already makes a `w:val="double"` band
`width * 3`, and round 146 measured 26.2.4.2 drawing exactly three equal bands of `w:sz/8`,
top-aligned, at 7 of 7 sizes.

## 3. Measured on the fixture

`tests/corpus/features/words-table-border-align.docx` is round 146's `align.docx`: five one-table
arms whose three columns state 0.5, 3.0 and 1.5 pt across one boundary, with the statement moved
to the other side on the second page.

| | the three bands |
| --- | --- |
| 26.2.4.2 | `95.001..95.501`, `95.001..98.001`, `95.001..96.501` |
| this tree, after | `95.450..95.950`, `95.450..98.450`, `95.450..96.950` |

**One shared top edge and three different bottoms, on both sides.** The remaining 0.449 pt is
O84: our boundary still sits half the widest border below the reference's, because the row above
still pays for half of it. That is why the tests assert the *shape* — one top, three bottoms —
rather than the absolute y, and why the write-up does not claim the document matches.

## 4. The confinement sweep caught a one-EMU defect the tests could not

The first pass moved **25** documents, and three of them held no mixed-width boundary at all.
One, `150_5300_13_chg12.doc`, reported **815 drawn paths identical on both legs** under PyMuPDF
and a different md5 — which is exactly the shape of a difference too small for the instrument.
Diffing the page's own content stream found it: a **vertical** rule at `x = 314.6501` had become
`314.65`, one EMU.

The cause is arithmetic, not geometry. `Length`'s division **rounds** (`operator /`,
`Length.cs`:140), so `half − Inner/2` and `−half + Width − Inner/2` — the same expression
re-associated — differ by one EMU whenever the width is an odd number of them, and the first
cut had folded the vertical case into the new horizontal one to avoid a branch. A doubled
vertical border is where that shows.

`DrawBorders` now writes the two cases as two expressions, with the reason in a comment, and
the document is byte-identical to its base again.

**The lesson is the sweep's.** Every test in this round passed with that in place, because no
test asks about a doubled vertical border on a `.doc`; and no *reasonable* test would, because
the round is not about vertical borders. *A refactor that preserves a formula's value in exact
arithmetic does not preserve it in rounded arithmetic, and a renderer is full of rounded
arithmetic.* The cheap guard is the one that worked: sweep, and explain every mover.

## 5. Reach and confinement, measured

The words track — 337 documents from `MANIFEST.tsv` — rendered twice under `SOURCE_DATE_EPOCH=0`
with nothing but the binary changing, one output directory per document. The base leg is round
146's own `after` fingerprints, which is this round's `HEAD`.

```
documents scored: 337      failed on either leg: 0
byte-identical:   314      moved: 23
```

**Every mover is explained, and the two that the census did not predict are the sharper
finding.** `B11. TE.CAO.00129 Experience logbook.docx` and its longer-named copy hold no
mixed-width boundary in *the reference's* ink and do hold one in **ours**: page 5 carries two
grid lines with a 0.5 pt band beside a 1.0 pt one, which is exactly where the four rules that
moved are, each up by 0.25 pt. Round 146's census read 26.2.4.2's own rendering, so it is a
statement about what the reference draws — and what this change moves is what **we** draw.

## 6. What this round could NOT establish

**28 of the 49 documents the census predicted did not move, and I cannot say why.** Two of them
are the head of the census — `150_5300_13_chg8.doc` at 210 mixed boundaries and
`150-5370-10H.docx` at 206 — so this is not a tail effect.

The obvious explanation is the same asymmetry as above in the other direction: a boundary the
reference draws in two widths may resolve to one width in our own border model, in which case
nothing moves however mixed the reference draws it. **But I could not confirm it**, because
`mixed-lines.py`, written here to count mixed boundaries in an arbitrary rendering, answers **0**
for the reference on both of those documents. One of the two instruments is wrong and this round
did not establish which; `census-pdf.py` matches two shapes with a tolerance, including a
narrower band contained inside a wider one, where `mixed-lines.py` groups on an exactly rounded
top edge.

So the round's reach figure is **the sweep's 23 documents**, which is direct evidence, and the
census's 49 is recorded as a prediction that over-shot by more than two to one for reasons that
are still open. `mixed-lines.py` is kept with that warning at the top of it rather than deleted.

**Two things follow for whoever takes O83 and O84.** The census in `tablerow-r146` is an upper
bound on what a *painter* change can move and should not be quoted as this class's reach without
the corresponding count in our own ink. And the same question — do our borders resolve to the
same widths as the reference's at a given boundary? — is O82's territory and may be a seat of
its own.

## 7. Tests, and the suite

`TableBorderTopAlignmentTests`, four assertions on the new fixture: the three bands share one top
edge, their bottoms differ by their own widths, the same holds with the statement on the other
side of the boundary, and the table's own single-width outline is still two rules. The third is
the one that says the rule is about the boundary rather than about which row spoke.

The assertions are about the *shape* — one top, three bottoms — and never about the absolute y,
because the absolute y is still half a border out until O84 moves. A test asserting 95.001 here
would be asserting a defect.

`WordProcessing` 2029 green; Containers, Core, Markup, OpenDocument, Rendering, Spreadsheets,
Text and Vector unchanged and green; Fidelity's ten known failures unchanged.

---

## 8. Round 148's correction to §5 and §6 — the census was wrong, not the tree

Sections 5 and 6 left *"28 of the 49 predicted documents did not move"* open, and named the
disagreement between `tablerow-r146/census-pdf.py` and `mixed-lines.py` as unestablished.
`probes/mixedcensus-r148/results.md` settled it against **`census-pdf.py`**, three ways:

1. Its second shape **never compares thickness** — it counts a narrower rule within 6 pt below a
   wider one whatever the widths. Every one of `150_5300_13_chg8.doc`'s 210 and
   `150-5370-10H.docx`'s 206 is a pair of groups of *identical* thickness, and what it fires on
   is not a table: 351 of that page's 355 horizontal segments are 0.069 pt hairlines 8–10 pt
   long, a vector-traced departmental seal. Only three are real rules.
2. Its **first** shape — the one that is this round's question — answers **0** on both
   documents, which is exactly what `mixed-lines.py` answered. The two instruments never
   disagreed about mixed widths.
3. That first shape is inverted anyway: it caps the side-by-side *overlap* at 1.0 pt and puts no
   cap on the *gap*, so it rejects the mitre overlap two bands of different width actually make.
   That is why §5 called the two `B11. TE.CAO.00129` movers unexplained — they hold four mixed
   boundaries each **in the reference too**.

**Corrected reach for this class: 16 of 337 words documents and 67 boundaries** (14 / 59 on
thickness alone, which is sturdier because it does not depend on the length floor), against
round 146's 49 / 770. Cross-tabbed against this round's own 23 movers: **19 of the 20 documents
holding an extent-mixed boundary moved, and 19 of the 23 movers are covered**.

**Two claims in §6 are withdrawn.** `150-5370-10H.docx` *did* move — §6 lists it as a non-mover
and `sweep-words.txt`, from the same script, lists it as a mover; the fingerprints say it moved,
on the one boundary that is mixed in our ink and not the reference's. Only
`150_5300_13_chg8.doc` genuinely did not move, and it now has no reason to (0 and 0). And §5's
hypothesis — *a boundary the reference draws in two widths resolves to one width in ours* — is
not merely unproven but **backwards**: our ink holds about five times as many mixed boundaries
as the reference's (292 against 67). Exactly one corpus document behaves the way it predicted,
which is not a mechanism.

Still open: why `PAT-047`, `airbus-pdf-information-package_v1-4` and
`xx_SETIS_PWS_template_10.19.22` moved. Every horizontal band in all three is identical before
and after to three decimals, which is consistent with §4's one-EMU vertical but was not
established by diffing the content streams.
