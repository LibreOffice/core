# Round 131 — a shared cell edge is one border, and the ten pages of wrong paper are two table faults that nearly cancel

Round 131, `/home/user/wt-wordstable`, branch `agent/wordstable`, base `fb9bf3b47`, 2026-09-14.
Two seats, both table geometry on the words track: **O82**, the residue round 126's own census
reports, and **O78**, the half of the FAA orientation flip that is not L4.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r129, `/home/user/gate-r129/ref`, rendered 2026-09-14 19:12–19:30, plus one authored probe rendered fresh in this round |
| our base leg | `/home/user/gate-r129/ours`, built from `/home/user/gate-r129-cli`. `git log ec0a11e1c..fb9bf3b47 -- dotnet/src dotnet/tools` is **0 files**, so that binary is this round's base |
| our after leg | `dotnet/tools/XRay.Office.Cli` built in this worktree, `SOURCE_DATE_EPOCH=1757462400`, one output directory per **document** |
| population | the **338** words-track documents of `MANIFEST.tsv`. The gap census here does **not** filter to gate `match` rows as round 126's did, so its base totals are that round's plus one document — stated below rather than compared silently |
| C++ tree | `/home/user/libreoffice-core`, read only. **C8**: it declares `27.2.0.0.alpha0+` and is not the reference binary's source. **No arm below rests on a source reading at all** — the rule was measured at 26.2.4.2 on an authored fixture, and the write-up names no `sw/` line for it |
| `/usr/bin/soffice` | 24.2.7.2 — not used for anything in this round |
| sweeps | our half of the words track only, twice; **no `soffice` was invoked by any sweep**, so nothing here contended with the two live rounds |

**Reading contamination.** No page was opened by eye in this round and nothing below rests on a
reading. Both seats are scored on drawn operators and on our own layout's cell rectangles.

**C11.** The authored probe was rendered **three** times into three separate user profiles on the
same UTC day, and the extracted geometry is byte-identical all three times
(`out/ref-lines-{a,b,c}.tsv`). Three rather than two, because gate r129's own correction says two
agreeing renders prove only that the same corner was sampled twice.

---

## 0. The headline

| | before | after | reference |
|---|--:|--:|--:|
| **O82** the reference bridges a gap between two of our rules, words track | 7 documents, 25 bridges, 20 pages, **2 605 pt** | 6 documents, 16 bridges, 12 pages, **1 094 pt** | — |
| the mirror direction (the base rate) | 9 documents, 45 bridges, 30 pages, **3 201 pt** | 9, 45, 30, **3 201 pt** | — |
| `review-welsh-…-mandelson.docx`, O82's characterised document | 9 bridges, 8 pages, **1 511 pt** | **0** | — |
| authored probe, the eight two-row arms 26.2.4.2 answers | — | **8 of 8** | — |
| words renderings that move | — | **31 of 338** (307 byte-identical) | — |
| page counts and alphanumeric counts that move | — | **0 and 0** | — |
| **O78** `FAA 2025-26 Holdover Tables.docx` p82, room at the foot | 26.6 pt | 26.6 pt | 28.1 pt |

**O82 is fixed for the document that carries 58 % of it and the rest is classified and seated.**
**O78 is not fixed, and it is now attributed**: its 1.5 pt is one cell wrapping that should not
(+6.60 pt) against eleven rows that charge half a border less than the reference (−5.25 pt), and
the two nearly cancel. Both halves are table row geometry, so the brief's *"the two seats may be
one mechanism"* is right — the mechanism behind O82's largest document is also one of O78's two
terms — but fixing that term **alone makes O78 worse**, which is why it is seated rather than
shipped. §4.

---

## 1. O82 — all seven documents classified, and three of them are a table border

Round 126 left *"one of six characterised, the other five (949 pt) not"*. The instrument that could
not tell them apart is `gap-census.py`, which reads a page. The instrument that can is **our own
layout**: `dump --tsv` prints every placed cell's rectangle and its four stated borders, so a
bridged gap can be tested against the cell edges that produced it rather than against a raster.
`classify.py` calls a bridge a **border** only when both hold — the band is within 0.6 pt of a cell's
top or bottom edge on that page, **and** both ends of the hole land within 0.6 pt of a column
boundary of the cells on that edge. The second test is what keeps a decoration off the list: a
0.75 pt underline that happens to run a quarter of a point under a row's grid line passes the first
and fails the second, and two of the seven documents do exactly that.

`bridges.py` reports each bridge rather than a per-document total; `all-bridges.tsv` and
`bridge-class.tsv` are its output.

| document | bridges | pt | verdict |
|---|--:|--:|---|
| `review-welsh-government-communications-mister-peter-mandelson.docx` | 9 | **1 511** | **table row border**, all nine — the top edge of a row whose two middle cells state `w:top w:val="nil"` |
| `150_5335_5a.doc` | 5 | 663 | **4 of 5 a table row border** (626 pt) and 1 a text rule (37 pt) — but a *different* border mechanism, §5 |
| `1_tpr_template__from_fy14_.docx` | 4 | 126 | **text rule**, all four: 0.75 pt underline segments on one line, no cell edge on the band |
| `195584360.docx` | 1 | 92 | **neither** — the census paired our two hyperlink underlines against a rule of the reference's that we draw 59 pt higher up the page. §2 |
| `template---tpr-technical-progress-report-with-guidance.docx` | 2 | 54 | **text rule**, both: underline segments, one of which sits 0.25 pt under a row's grid line and fails the column test |
| `TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.docx` | 1 | 14 | **table row border**, and a third mechanism again — §5 |
| `150_5300_13_chg10.doc` | 3 | 145 | **table geometry**, but the document is 77 pages against the reference's 78, so the pages the census pairs are not the same pages. Not a comparable measurement |

The seventh document is not in round 126's six because that round filtered the census to gate
`match` rows and this one mismatches on pages. It is included here and named as uncomparable rather
than dropped. It is also the one document `classify.py` could not be run on — `dump --tsv` aborts on
that `.doc` — so its three bridges were classified instead by counting the **vertical** strokes
crossing each band, which is what separates a table from a decoration when the cell rectangles are
not available: 8, 0 and 8 of ours and 8, 9 and 12 of the reference's. The middle band is the tell —
we draw no vertical there at all, because that page of ours holds different content.

**So the residue is 2 460 pt in round 126's population and 2 605 pt in the whole track, of which
1 511 pt — 58 % — is one mechanism on one document, 626 pt is a second mechanism on a second, 14 pt
is a third, 272 pt is a text rule and 237 pt is not measurable.**

### The base rate, so the reach means something (C9)

`sharededge-census.py`, over the 338 words documents. It counts three nested populations, because a
`nil` facing a border is invisible wherever both rows are drawn on one page:

| | documents |
|---|--:|
| words-track documents | **338** (272 DOCX, 66 in three other formats) |
| DOCX holding a table at all | **183** |
| …stating a horizontal shared edge the two sides disagree about | **64** (18 134 edges) |
| …where one of the two sides states `nil`/nothing and the other a border | **63** (17 656 edges) |

**So the class is very well populated and the defect reaches almost none of it**, which is the
point of measuring it: the resolution changes nothing on a page where both rows are drawn, so what
is left is the top of a table's *continuation* page. 31 of the 338 renderings move at all (§3) and
one document's bridges disappear.

The census is DOCX-only in its numerator and the other 66 documents are counted only as *holds a
table*, which keeps the denominator honest rather than pretending the numerator covers WW8, RTF and
ODF. Their own spellings of a nil edge are not censused.

---

## 2. `195584360.docx` is the instrument, not a rule — a refutation of the census's own row

Its single 92 pt bridge is our two hyperlink underlines at y 220.35 (0.6 pt fills under
`michael.rosauer@cpuc.ca.gov` and `maryjo.borak@cpuc.ca.gov`) against **a rule of the reference's
that is not a decoration at all**: a 1.0 pt stroke running the whole text width, 64.80 → 568.75, at
y 222.25. We draw that same rule — at y 163.05, because our page 9 sits about 59 pt above the
reference's. The census's `TOL_Y` of 1.2 with a ±1 band fallback let two objects 1.9 pt apart pair.

Two things follow, both worth more than the row. **A `gap-census` bridge on a document whose page
content is displaced is not evidence about a rule**, and neither the census nor round 126's reading
of it had a displacement guard. And the reference draws every one of that page's full-width rules
**twice** where we draw it once — a separate, uncharacterised difference that the cover metric
cannot see, because two coincident strokes merge.

---

## 3. The fix: a horizontal edge two cells share is one border, and both of them get it

### What 26.2.4.2 answers, measured

`make-borderprobe.py` writes `borderprobe.docx`: eight two-row arms in which only the two
`w:tcBorders` facing each other across one row boundary change, plus two long tables that split
across a page. `attribute.py` names each drawn rule by the arm whose cells straddle it, so nothing
is attributed by a y-range guess. Rendered three times into three profiles; identical.

| arm | the two facing edges | what 26.2.4.2 draws | the two row heights |
|---|---|---|--:|
| `BB` | bottom 1 pt / top 1 pt | one line, whole | 12.50, 12.60 |
| `BN` | bottom 1 pt / **top nil** | **one line, whole** | 12.50, 12.60 |
| `NT` | **bottom nil** / top 1 pt | **one line, whole** | 12.50, 12.60 |
| `NN` | bottom nil / top nil | **no line** | 24.10 for the two, **1.0 pt less** |
| `WB` | bottom 3 pt / top 0.5 pt | one line, **3 pt** | — |
| `WT` | bottom 0.5 pt / top 3 pt | one line, **3 pt** | — |
| `MIX` | one row's two cells disagree, the row above states both | one line, **whole** | — |
| `MIXN` | both sides nil on the left cell, both stated on the right | **half a line**, the right cell only | — |

Four things are settled by that and none of them needed the C++ tree:

* the winner is the **wider** of the two facing edges and it is written to **both** — `BN` and `NT`
  are the same as `BB` in both what is drawn and how tall the rows are;
* `nil` on both sides really is nothing, and the row is then a whole border shorter;
* the resolution is **per column**, not per row — `MIXN` draws half a line;
* the **row height** follows the resolved border. That half is measured and deliberately not
  shipped; §4.

### And a page break is where it shows

`PBNIL` is 44 rows, every one stating `w:top w:val="nil"` except the first, and it splits. Its
continuation page opens with a **whole** line at the outer extent. `PBNN` is 90 rows with every
interior edge nil on **both** sides: it splits too, and its continuation page still opens with a
whole 1 pt line at 70.90 while **not one interior line is drawn anywhere in it**. So at a table's
page cut the reference draws a line across the whole table whatever the cells state, and that is a
second rule this tree does not have — see O83, which is where the 0.25 pt half of it lives.

### What shipped

`PageTable.RowsWithSharedEdges` is a second, lazily-computed view of the rows in which each
horizontal edge two cells share carries the same border on both sides. `PageTable.Rows` is
untouched, and **only `PageDrawing.DrawBorders` reads the resolved view** — the layout reads the
stated one, so no row height, no page count and no line break can move. That is a deliberate
narrowing and §4 is why.

Two details of the implementation are measurements rather than choices:

* **The facing width is the NARROWEST over the columns the cell covers, not the widest.** This model
  carries one border per edge and the reference resolves per column (`MIXN`), so a cell facing three
  cells of which one states nothing must not take a border across its whole width. The first cut
  took the widest and it cost **`A_320.doc` 75 pt a page on 112 of its 118 pages** — our cover on
  that document went 259 698 → 266 859 pt against the reference's 267 116 while the per-page
  distance *doubled*, 7 430 → 14 191, which is the signature of extra ink landing on pages that were
  already over. With the narrowest rule `A_320.doc` is byte-identical to the base.
* **A cell spanning rows faces nothing across the boundaries inside its own span**, which is what
  the `below.Row == row + 1` test says.

**Five documents moved under the first cut and are byte-identical under the shipped one** —
`A_320.doc`, `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc`,
`LHD-230-application-for-the-approval-of-an-aircraftr.doc`,
`UG.CAO.00006 … User Guide for Applicants.docx` and
`certification-flight-standards-doc-oeb-supporting-documents.docx` — so the column rule is worth
36 movers against 31 and is what keeps the change inside the class it was measured on.

### Reach and cost

Our half of the words track rendered twice, `SOURCE_DATE_EPOCH` pinned, one directory per document.
The base leg is gate r129's own `ours`, which was rendered **without** the pin, so its
`/CreationDate` is masked before the byte comparison — without that mask the comparison reports
338 of 338 moved, which is `CLAUDE.md`'s *a reference PDF differs byte for byte and it means
nothing* arriving on our own half.

| | before | after | reference |
|---|--:|--:|--:|
| words renderings that move | — | **31 of 338**, 307 byte-identical | — |
| page counts that move | — | **0** | — |
| alphanumeric counts that move | — | **0** | — |
| Σ per-page \|ours − reference\| merged rule cover, over the 31 | 1 036 552 pt | **1 036 359 pt** | — |
| Σ \|our total cover − the reference's\|, over the 31 | 511 348 pt | **511 164 pt** | — |
| of the 31, total cover closer to the reference / further / level | — | **8 / 5 / 18** | — |

**Zero page counts and zero alphanumeric counts move, and that is structural rather than lucky**:
the resolved rows are read by `PageDrawing` alone, so no measurement the layout makes can see them.

**The cover metric is level and the census is not, and the two say different things.** What the
census measures is a *hole* — the seat's own shape — and it goes 2 605 → 1 094 pt with the mirror
direction untouched at 3 201 pt, which is the control that says no new hole was made in the
reference's direction. What the cover metric measures is total ink, and there the fix is a wash:
three documents land on the reference almost exactly (`private-hire-operators-licensed.docx`
15 070 → **15 439** against 15 439; `2ca950374241722b8ea7c132dfce63ae.doc` 12 267 → 12 778 against
12 780; `135.doc` 22 794 → 23 279 against 23 688) and two move further away because they were
already over-drawing (`02_mcar_part-2_and_IS_v2.10.docx` 593 pt over → 1 559;
`P200904290238_0238_51880.doc` 173 → 464). **On the seat's own document the two disagree in an
instructive way**: `review-welsh-…-mandelson.docx`'s bridges go to zero and its cover goes from
1 207 pt *short* of the reference to 472 pt *long*, because the line that was drawn in pieces is now
drawn whole **twice** — the duplicate being O83, which this round did not touch.

---

## 4. O78 — the 1.5 pt is two faults that nearly cancel, and one of them is O82's

Round 128 left the seat as *"a table row-height difference of a few tenths of a point per row, not
attributed"*. It is attributed here, and it is not one thing.

`FAA 2025-26 Holdover Tables.docx` page 82 carries one table of 33 rows. Pairing our horizontal
grid lines with the reference's — 34 against 34 once the reference's two doubled lines are merged
at 0.4 pt (`faa-p82-rows.tsv`):

| | rows | pt |
|---|--:|--:|
| rows we make **shorter** than the reference | **11** | **−5.25** |
| rows we make **taller** | 2 | **+6.90** |
| net | | **+1.55** |

and the last grid line is ours at 546.20 against the reference's 544.70, a difference of **1.50 pt**
— which is O78's whole margin.

**The eleven short rows are the height half of §3's rule, and every one of them is 0.5 pt short of a
1.0 pt border.** Each is a row all of whose cells state `w:bottom w:val="nil"` while the row below
states a 1 pt top: `BorderHeight` is `(max stated top + max stated bottom)/2`, so the row charges
0.5 where the reference charges 1.0. The `BN`/`NN` arms of §3 say outright that the reference charges
the **resolved** border.

**The two tall rows are one cell that wraps and should not.** Row 7's column 10 holds
`Not Available19` in a cell 63.10 pt wide (the reference's is 63.00) whose `w:tcMar` is absent and
whose table states no `w:tblCellMar` and no table style, so both sides use Word's 108-twip default
and the inner width is 52.30 pt for us and 52.20 for the reference. **Both sides measure the string
identically** — `Not` 12.4 pt and `Available19` 37.6 pt on both, with a 2.2 pt space between them,
52.2 pt in all. The reference sets it on one line and we break it, costing **+6.60 pt** of row.
So the fit decision turns on **a tenth of a point in fifty-two**, which is 0.2 % — above the
0.0077 % floor `CLAUDE.md` establishes for advance agreement and far below anything this round can
attribute. It is not the padding, not the column width and not the metric.

**Fixing the height half alone makes O78 worse, and that is measured rather than argued.** With the
resolution applied to `TableLayouter` as well as to the drawing, our page-82 table would end about
5 pt *lower* rather than 1.5, and the empty half of the `<w:br w:type="page"/>` paragraph at body
index 1226 would fit even less well. On the whole-corpus side the same change took
`review-welsh-…-mandelson.docx`'s first table from **1.45 pt short of the reference over 20 rows to
6.05 pt long**, because a row stating `w:trHeight` does not charge its border the way a
content-driven row does: measured row by row against 26.2.4.2, the change is right on rows 0, 4, 11
and 19 and wrong on rows 2, 3, 5, 7, 8, 10, 13, 14, 16 and 17 of the same table. That is seat **O84**.

**O78 therefore stays open, with its measurement.** Reach is unchanged at 1 of 329 on this exact
knife-edge; neither of its two terms is censused as a class.

---

## 5. Seats

### SEAT O83 — a row split across a page is placed half a border too high, so the grid line at the split is drawn twice

On `review-welsh-…-mandelson.docx` page 10 the row above ends at y 721.850 and the row below — the
one that continues onto page 11 — **starts at 721.600**, a quarter of a point above it. Page 9 is
the same fault the other way: the continuation part ends at 120.900 where the next row starts at
121.150. So we draw two grid lines 0.25 pt apart where the reference draws one, and the same
0.25 pt appears at the cut itself: our table's outer line at the foot of page 10 is at 768.50 where
the reference's is at 768.75, and at the top of page 11 ours is at 72.00 where the reference's is
at 72.25. §3's `PBNN` arm shows the reference drawing that cut line whatever the cells state.

The rule the layout already has for the *first* row of a table — *"the first grid line sits half a
border below the table's top edge"*, `TableLayouter` pass two — is not applied at a page cut.

**It is also what made §1's largest bridge visible mid-page**: before the fix the duplicate line
masked some of the holes from the census (two spans in one y-band merge) and exposed others.
After the fix every one of those lines is whole, and the duplicate remains: it is why
`review-welsh-…`'s merged rule cover goes from **1 207 pt short of the reference to 472 pt long**
while its bridges go to zero.

Reach: not censused. The class is a table that splits a page, which is common; the *visible*
consequence is a doubled grid line, which the gap census cannot see and the cover metric reads as
over-drawing.

### SEAT O84 — the row-height half of the shared edge, which is 5.25 pt of O78

§3's authored arms say the row height follows the resolved border and §4 says implementing it as
measured over-charges a `w:trHeight` row. What is missing is the rule for a row whose height comes
from `w:trHeight` rather than from its content: measured against 26.2.4.2 on
`review-welsh-…-mandelson.docx`'s first table, whose twenty rows all state one, the border charge
added to the stated height is 1.00, 0.75, 0, 0, 0.75, 0.75, 0, 0.25, 0.75, 0 and 0.25 pt on the rows
that are not content-driven, and no linear function of the two facing widths fits all of them.
The cost of leaving it is the eleven short rows of FAA page 82 and whatever the same shape is worth
elsewhere; the cost of shipping it as measured is 6.05 pt on a twenty-row table that was 1.45 pt out.

### SEAT O85 — a row's border bands are aligned at their top edge and this tree centres them on the grid line

`150_5335_5a.doc` is 626 pt of O82's residue in four bridges and it is **not** §3's mechanism. On its
page 39 the boundary between rows 8 and 9 carries a 1.5 pt double border on two cells and a 0.5 pt
single on the other six. 26.2.4.2 draws a continuous 0.5 pt line across the whole table at
y 278.75 and a second 0.5 pt line at 279.75 under the two double cells alone — so the double's
**outer** rule and the single share one y and the band's **top edge** is what the two have in
common. This tree centres each band on the grid line at 279.375: the double comes out at 278.875 and
279.875 and the single at 279.375, so the single is 0.625 pt below the reference's while the double
is within 0.13, and the census reads the two double cells' outer rule as a rule with a 266 pt hole
in it.

`PageDrawing.DrawBorders`' `Rule(offset)` is where it sits: `bands.Outer / 2 − half` centres the
band. Reach: not censused; the class is a row whose cells state borders of different widths, which
`sharededge-census.py`'s 18 134 disagreeing edges bound from above and do not measure.

### Named and not seated

* **`TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.docx`**, 14 pt on page 3, survives the
  fix. Its row 4 column 2 states nil on both its top and its bottom and the cell above states nil
  too, so §3's rule correctly gives it nothing — and the reference draws a 14.8 pt segment for it
  anyway, at y 465.10, a quarter of a point above the line it draws for the rest of the row. It is
  probably O85's mechanism and probably O83's too; one bridge is not enough to separate them.
* **The reference draws some full-width rules twice** (§2, `195584360.docx`). Not characterised.
* **`1_tpr_template__from_fy14_.docx`** and
  **`template---tpr-technical-progress-report-with-guidance.docx`**, 180 pt between them, are text
  rules and belong to O71's family rather than to O82's. On both, the underlined and un-underlined
  stretches of one line appear in the **reverse order** from the reference's — ours 39.74 pt then a
  21.19 pt gap then 4.42, the reference's 4.40 then 21.20 then 39.75, over the same 65.4 pt span —
  so the segments are a run-ordering difference within a line and not a decoration rule.

---

## 6. Tests

`SharedCellEdgeTests` is new: 8 cases in 3 methods, one theory per arm of §3's probe plus the
`MIXN` per-column case and a control that the *stated* rows are left as the file wrote them.

**Checked by mutation rather than left to look like coverage.** With `RowsWithSharedEdges` reduced
to `Rows` and nothing else changed, `dotnet test --filter SharedCellEdgeTests` reports **6 failed of
8**. The two that pass at the base are the two negative arms — `BB`, where both sides state the
border and there is nothing to resolve, and `NN`, where neither does — and each is there to pin the
half of a theory whose other half fails.

**The full run, projects individually** (`CLAUDE.md`: total them yourself, and a drop with zero
failures is a truncated run). Every count is checked against that project's own
`dotnet test --list-tests`, which collapses some theory rows and is therefore a lower bound — a run
*below* the listing is the signal:

| project | passed | failed | skipped | discovered |
|---|--:|--:|--:|--:|
| `XRay.Office.Core.Tests` | 591 | 0 | 0 | 591 |
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 | 109 |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 | 249 |
| `XRay.Office.OpenDocument.Tests` | 169 | 0 | 0 | 169 |
| `XRay.Office.Rendering.Tests` | 164 | 0 | 0 | 164 |
| `XRay.Office.Text.Tests` | 744 | 0 | 0 | 744 |
| `XRay.Office.Vector.Tests` | 309 | 0 | 0 | 309 |
| `XRay.Office.WordProcessing.Tests` | **1988** | 0 | 0 | 1988 |
| `XRay.Office.Presentations.Tests` | 1205 | 0 | 0 | 1205 |
| `XRay.Office.Spreadsheets.Tests` | 1387 | 0 | 0 | 1387 |
| `XRay.Office.Fidelity.Tests` | 542 | **10** | 0 | 552 |

**No project ran below its listing and none ran with a skip**, which is the pair of checks that says
no run was truncated and no project covered nothing. `XRay.Office.WordProcessing.Tests` is 1988 of
1988 discovered — round 126's 1974 plus this round's 8 and round 128's 6 — and it is the project two
rounds have had truncated in one day, so its count was read against the listing rather than against
the colour.

The ten fidelity failures are the standing set, and they are the **same ten by name** as round 126's
`fidelity-failures.txt`, diffed rather than eyeballed. Banked here as `fidelity-failures.txt`.

`dotnet build XRay.Office.slnx -c Release`: **0 warnings, 0 errors.** The full table is
`test-run.txt`.

---

## 7. Refutations of this round's own findings, kept rather than deleted

* **"The shared edge resolves to the wider of the two facing borders."** True per column and false
  per cell, and the difference is 75 pt a page on `A_320.doc`. §3.
* **"O82's residue is six documents."** Seven on the whole words track; round 126's six is its
  census's own filter to gate `match` rows, and the seventh is a document whose pages do not
  correspond to the reference's at all. §1.
* **"`195584360.docx`'s 92 pt is a decoration residue."** It is a mis-pairing between two objects
  1.9 pt apart on a page whose content is displaced by 59 pt. §2.
* **"338 of 338 renderings moved."** The base leg was rendered without `SOURCE_DATE_EPOCH` and the
  whole difference was `/CreationDate`. Masked; 36 move. §3.
* **"The first cut of `classify.py` finds five border bridges in the smaller documents."** Two of
  those five are underlines that happen to run a quarter of a point under a row's grid line. The
  column-alignment test is what separates them, and it was added after the first table was written.
* **"The height half of the rule can ship with the drawing half."** §4: it is right on the authored
  arms and wrong on ten of twenty rows of a real `w:trHeight` table.

---

## 8. The scripts and the banked data

| | |
|---|---|
| `make-borderprobe.py`, `borderprobe.docx` | §3, the ten-arm authored probe |
| `attribute.py`, `read-borderprobe.py` | one rendering → the rules, named by the arm that owns them |
| `out/ref-lines-{a,b,c}.tsv`, `out/ref-{a,b,c}.tsv` | 26.2.4.2 three times, three profiles — the C11 control, identical |
| `bridges.py`, `all-bridges.tsv` | §1, every bridged gap as its own row |
| `classify.py`, `bridge-class.tsv` | §1, border against text rule, decided on our own cell rectangles |
| `geom/*.tsv` | the cell rectangles those verdicts are read from (`dump --tsv`) |
| `sharededge-census.py`, `sharededge-census.tsv` | §1, the base rate |
| `gap-track.py`, `gap-base.tsv`, `gap-after.tsv` | §0, the census both ways over the whole track |
| `cover.py`, `cover-base.tsv`, `cover-after.tsv` | §3, merged rule cover and its distance from the reference |
| `sweep-words.py`, `movers.py`, `movers.tsv`, `movers.txt` | §3, our half of the track twice, and what moved |
| `fidelity-failures.txt` | §6, the ten standing failures by name |
| `show-rules.py` | every rule on one page, both shapes — the witness instrument |
| `faa-p82-rows.tsv` | §4, the 33 row heights of FAA page 82, ours against the reference's |
| `test-run.txt` | §6 |
