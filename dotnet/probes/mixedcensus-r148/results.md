# Round 148 — reconciling the two mixed-width-boundary censuses

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. (`/usr/bin/soffice` is 24.2.7.2 and is not the
target.) The C++ tree read here is `/home/user/libreoffice-core` at **27.2.0.0.alpha0+**, which
is *not* that binary's source. **[src]** marks a reading of the C++ tree, **[c#]** a reading of
XRay.Office's own source, and **[bin]** a measurement against 26.2.4.2's own output.

This round ran no build, no test and no `xray-office render`; it is a measurement and reading
round only. Everything marked **[bin]** is read out of PDFs already on disk — round 146's
fixture renderings in `probes/tablerow-r146/out/`, the reference bank at
`/home/user/refpdfs-words-26.2.4.2/`, and round 147's own renderings under its scratch tree.

**In one line:** `mixed-lines.py` is right and round 146's `census-pdf.py` is wrong three ways;
both head documents carry **0** mixed boundaries in 26.2.4.2's rendering; the class's reach is
**16 of 337 documents and 67 boundaries**, not 49 and 770; and the 28 non-movers were an artefact
of the broken census, not a fact about our border model.

## 1. The ground-truth fixture does not discriminate — both scripts pass it

**[bin]** `probes/tablerow-r146/out/align.pdf` (26.2.4.2's own rendering of `align.docx`), page
by page, with each script's `mixed()` applied to that page alone (`cmp.py`):

```
page 0: census-pdf mixed=1 same=0 | mixed-lines mixed=1 of 3 groups
page 1: census-pdf mixed=1 same=0 | mixed-lines mixed=1 of 3 groups
page 2: census-pdf mixed=1 same=0 | mixed-lines mixed=1 of 4 groups
page 3: census-pdf mixed=1 same=0 | mixed-lines mixed=1 of 4 groups
page 4: census-pdf mixed=0 same=0 | mixed-lines mixed=0 of 3 groups
```

Page 1 of the fixture carries the known boundary of three widths — `95.001..95.501`,
`95.001..98.001`, `95.001..96.501` — and **both scripts score it 1**. Neither is broken in the
way the task's test was designed to catch. The fixture is a *necessary* check that both pass,
so it settles nothing on its own and the disagreement had to be located elsewhere.

## 2. Where the 210 and the 206 come from: shape (b), and shape (b) alone

**[bin]** `census-pdf.py`'s `mixed()` adds two independent shapes. Splitting its return into the
two contributions (`split.py`), against the banked reference renderings:

| document | shape (a) | shape (b) | total | same-width base |
| --- | --- | --- | --- | --- |
| `150_5300_13_chg8.doc` (17 pp) | **0** | 210 | 210 | 8 |
| `150-5370-10H.docx` (727 pp) | **0** | 206 | 206 | 735 |
| `align.pdf` (5 pp, fixture) | 4 | 0 | 4 | 0 |

Shape (a) is *"one band top carrying segments of more than one thickness side by side"* — the
round's actual question, and the thing `mixed-lines.py` also asks. On both head documents it
answers **0**, which is exactly what `mixed-lines.py` answers. **The two instruments agree
completely on shape (a).** There was never a disagreement about the question the round poses.

Shape (b) is *"a narrower group of segments hanging 0 < dy <= 6 pt below a wider group and
contained in it"*, intended as the double-border case where the inner rule is on some columns
only. It is 100% of the 210 and 100% of the 206.

## 3. Shape (b) is wrong twice over

**Its predicate never looks at the thickness.** `census-pdf.py`:96-98:

```python
if cover(g[k2]) < cv - 1.0 and all(
        any(a[0] >= b[0] - 1.0 and a[1] <= b[1] + 1.0 for b in g[k]) for a in g[k2]):
```

`a` and `b` are `(x0, x1, th)` triples and only `x0`/`x1` are ever compared. So shape (b) counts
*a narrower horizontal rule within 6 pt below a wider one*, whatever the two widths are. Every
one of the 210 and the 206 is a pair of groups of **identical** thickness — 0.069 pt on
`150_5300_13_chg8.doc`, 0.068 pt on `150-5370-10H.docx`. By the round's own definition none of
them is a boundary of more than one width. **[bin]**

**And what it is firing on is not a table.** The flagged y values cluster at the top of page 1 of
`150_5300_13_chg8.doc`, in a 16 pt-wide strip at x ≈ 79–95, stacked at 0.06 pt pitch. Reading
the page's operators directly (`peek.py`): the page holds **2323 `l` items**, of which 355 are
horizontal and at least 4 pt long, and of those **351 are 0.069 pt hairlines 8–10 pt long**. Only
three are real rules (0.75 pt × 495 pt) and one is 6.0 pt. That is a vector-traced graphic — the
departmental seal on the Advisory Circular cover — decomposed into hairlines, and shape (b)
reads every stack of them as a mixed table boundary. **[bin]**

Shape (b) also imposes no relation between the two groups beyond containment, and 6 pt is a
whole row gap: two genuinely separate rules 6.0 pt apart, of one width, score as one mixed
boundary. One of the samples is exactly that (`upper y~51.060`, `lower y~57.060`, both 0.069).

**So `census-pdf.py`'s `mixed_boundaries` column is not a count of mixed-width boundaries.** Its
shape (a) half is; its shape (b) half is a count of stacked narrowing horizontal ink, and on the
two head documents shape (b) is the whole figure.

## 4. The base rate said so before any of this

`150_5300_13_chg8.doc` scored **210 mixed against a base of 8** one-width column boundaries — a
document in which 96% of all multi-segment boundaries are mixed. A 17-page Advisory Circular is
not that document. The base rate beside the count was the tell, and it was printed in
`census-pdf.tsv` all along.

## 5. Which script is right

**`mixed-lines.py` is right on the question the round asks, and `census-pdf.py`'s shape (a) is
the same instrument.** The 210 and the 206 are shape (b), and shape (b) is not measuring
mixed-width boundaries. `mixed-lines.py`'s UNRECONCILED warning can come off; its **0** for
26.2.4.2 on both head documents is correct.

Neither is right in full, so this round replaces both with `mixed-census.py`, which differs from
`mixed-lines.py` in one respect that matters:

- **`mixed-lines.py` ignores x entirely.** It buckets every horizontal segment on a page by its
  rounded top edge and calls the bucket mixed if two thicknesses land in it. Two *unrelated*
  rules that happen to share a top edge to 0.01 pt therefore count as one mixed boundary.
  Over the whole track it finds **60 boundaries in 15 documents** against the chain rule's
  **59 in 14**; the one extra is
  `words/pagination-002/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx`,
  where the two segments at the shared top are not side by side. **[bin]**

`mixed-census.py` groups by exact top edge *and then* splits each top into maximal chains of
x-abutting segments (gap or overlap ≤ 1.0 pt), which is `census-pdf.py`'s `side` test applied
per chain rather than to the whole top. It reads both operators — a filled/stroked `re` and a
stroked `l` — which is r147's warning and is load-bearing: a stroke-only reader answers 0 on
documents whose borders are filled rectangles.

A length floor of 12 pt on a segment (to exclude hairline line-art) **changes nothing**:
59 boundaries with the floor, 59 without. The art is uniform in width, so it was never going to
produce a mixed chain — it only ever fed shape (b). **[bin]**

## 6. The real answer for the two named documents

**[bin]** In 26.2.4.2's own rendering, banked at `/home/user/refpdfs-words-26.2.4.2/1/`:

| document | pages | mixed-width boundaries | one-width column boundaries (base) |
| --- | --- | --- | --- |
| `150_5300_13_chg8.doc` | 17 | **0** | 0 (8 by `census-pdf.py`, all line-art) |
| `150-5370-10H.docx` | 727 | **0** | 725 |

Neither document carries a single horizontal boundary whose columns are drawn in more than one
width. They were the head of the census and they belong at the foot of it.

## 7. The corrected population

**[bin]** `mixed-census.py` over all 337 words-track documents, against the banked 26.2.4.2
renderings (`census-ref.tsv`):

```
documents read                            337
  >=1 mixed-width horizontal boundary       14 docs      59 boundaries
  >=1 one-width column boundary (base)     105 docs    2383 boundaries
```

**Read the count as: 14 of 337 words-track documents (4.2%) hold at least one horizontal table
boundary that 26.2.4.2 draws in more than one band width, 59 such boundaries in all, against a
base of 105 documents and 2383 boundaries drawn as two or more side-by-side column bands of one
width.** So roughly one boundary in forty of the boundaries this rule can reach is mixed, in one
document in eight of those that hold a multi-column boundary at all.

Round 146's figure was **49 documents / 770 boundaries**. It over-counted by 3.5× in documents
and 13× in boundaries.

*(This is the count of boundaries drawn in more than one **thickness**. §11 widens it to more
than one band **extent**, which additionally catches a double border beside a single one and
raises the figure to 16 documents / 67 boundaries. **16 / 67 is the number to quote for the
class's reach**; 14 / 59 is the narrower, more robust reading of the same population.)*

The 14:

| boundaries | document |
| --- | --- |
| 20 | `words/done-015/docx/DOA_Template_Form_Type_Certification_Programme.docx` |
| 7 | `words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx` |
| 5 | `words/pagination-001/docx/24-25_FAA_Holdover_Tables.docx` |
| 5 | `words/table-001/docx/A1. EASA Form 2.docx` |
| 4 | `words/table-001/docx/B11. TE.CAO.00129  Experience  logbook.docx` |
| 4 | `words/table-001/docx/approvals-and-standardisation-…-Experience--logbook.docx` |
| 4 | `words/extra-001/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx` |
| 3 | `words/done-010/doc/1528364855.doc` |
| 2 | `words/done-012/docx/FO.FCTOA_.000129 Application for activities related to FSTD.docx` |
| 1 | `words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx` |
| 1 | `words/extra-001/docx/ABCD-SDE-23-00 - Avionic System Description - 17.02.16 - v1.docx` |
| 1 | `words/extra-001/docx/ABCD-WB-08-00 Weight and Balance Report - v1 08.03.16.docx` |
| 1 | `words/done-011/docx/part-145-approval list 2025.docx` |
| 1 | `words/done-011/docx/part-145-approval list (1).docx` |

**All fourteen moved in round 147's sweep.** Recomputed here from the same two fingerprint files
r147 used (`r146/after/fp.txt` vs `r147/after2/fp.txt`, 23 movers, reproduced exactly).

## 8. The 28 non-movers do not exist

**There was never a 28-document shortfall.** It was entirely the broken census's own doing:

| | docs | of which moved |
| --- | --- | --- |
| r146 census predicted (`census-pdf.py`) | 49 | 21 |
| corrected census predicts (`mixed-census.py`, reference's ink) | 14 | **14** |

The corrected census has **no false negatives at all** on this sweep. Every document it names
moved; not one of the documents it does not name failed to move for a reason the census should
have caught. The 28 were shape (b) firing on vector line-art and on pairs of separate one-width
rules within 6 pt of each other.

**And it explains r147's two "unexplained" movers.** The two `B11. TE.CAO.00129 Experience
logbook` copies were reported as holding no mixed boundary in the reference's ink and one in
ours. They hold **4 each in the reference too**, and the reason round 146's census missed them is
a third defect in `census-pdf.py`, this time in shape (a). Its side-by-side test is

```python
side = all(v[i][1] - v[i + 1][0] <= 1.0 for i in range(len(v) - 1))
```

which caps the *overlap* of two consecutive segments at 1.0 pt and places no cap at all on the
gap — a negative difference always passes. On all four of these boundaries 26.2.4.2 draws the
1.0 pt band ending at 339.55 and the 0.5 pt band starting at 338.05, an **overlap of 1.5 pt**,
which is the mitre where two bands of different width meet. So shape (a)'s side test rejects
precisely the geometry a mixed-width boundary produces, and keeps the one that a uniform
boundary produces. **[bin]** `mixed-census.py` chains on the gap instead and admits any overlap.

r147 §5's sentence about those two documents should be restated: the asymmetry it describes — a
boundary mixed in our ink and not in the reference's — is real in general (see §9) but is not
what those two documents are.

## 9. Our own ink — the hypothesis is false, and backwards

r147 §6's hypothesis was that *a boundary the reference draws in two widths may resolve to one
width in our own border model*, which would have explained non-movers. **Measured, it is the
other way round: our ink holds five times as many mixed boundaries as the reference's.**

**[bin]** `mixed-census.py` over the same 337 documents, against round 147's own `after2`
renderings at `…/scratchpad/r147/after2/pdf/`:

```
                                      reference 26.2.4.2      ours (r147 after)
>=1 mixed-width boundary              14 docs   59 bdys       18 docs  286 bdys
>=1 one-width column boundary (base) 105 docs 2383 bdys       50 docs 1970 bdys
```

*(The "ours" leg is only measurable **after** r147's change. Before it we centred each band on
its own grid line, so bands of different width at one boundary had different tops and a
top-grouping census necessarily reads 0. That is a property of the instrument, not a finding.)*

Cross-tabulated against the 23 movers, recomputed from r147's fingerprints:

| mixed in reference | mixed in ours | documents | of which moved |
| --- | --- | --- | --- |
| yes | yes | 14 | **14** |
| no | yes | 4 | **4** |
| no | no | 319 | 5 |

**Eighteen of the 23 movers are covered, and every document with a mixed boundary in either
rendering moved.** The four that are mixed only in ours are
`ESPN-R - MCF - RA - Ed1.docx` (143), `150_5300_13_chg10.doc` (2), `150-5370-10H.docx` (1) and
`AC-150-5370-10G-updated-201604.docx` (1) — so `150-5370-10H.docx`, the census's number two,
**did move**, and moved on a boundary that is mixed in *our* ink and not in the reference's.
(r147 §6 lists it among the non-movers; `sweep-words.txt`, produced by the same script, lists it
among the movers with its 206. The fingerprints say it moved. The §6 sentence is wrong.)

`150_5300_13_chg8.doc` really did not move, and now has no reason to: **0** mixed boundaries in
the reference's ink and **0** in ours.

## 10. The five remaining movers, and a class no thickness census can see

**[bin]** Diffing our before (`r146/after`) and after (`r147/after2`) horizontal bands directly
(`whymoved.py`):

| document | horizontal bands changed | shift |
| --- | --- | --- |
| `150_5335_5a.doc` | 15 of 757 | −0.500 |
| `LHD-230-application-…-aircraftr.doc` | 1 of 13 | −0.125 |
| `PAT-047 - Architecture and Detailed Design Assessment.docx` | 0 of 106 | — |
| `airbus-pdf-information-package_v1-4.docx` | 0 of 108 | — |
| `xx_SETIS_PWS_template_10.19.22.docx` | 0 of 217 | — |

**`150_5335_5a.doc` is a real class the thickness rule cannot see, and `census-pdf.py`'s shape
(b) was right about which document it lives in.** Page 28, before and after:

```
before: top 292.425  x 287.25..360.75  th 0.5      after: top 292.425  x 153.65..288.25  th 0.5
        top 292.425  x 433.95..498.45  th 0.5             top 292.425  x 287.25..360.75  th 0.5
        top 292.925  x 153.65..288.25  th 0.5             top 292.425  x 359.75..434.95  th 0.5
        top 292.925  x 359.75..434.95  th 0.5             top 292.425  x 433.95..498.45  th 0.5
        top 293.425  x 287.25..360.75  th 0.5             top 293.425  x 287.25..360.75  th 0.5
        top 293.425  x 433.95..498.45  th 0.5             top 293.425  x 433.95..498.45  th 0.5
```

and 26.2.4.2 draws four runs at top 292.300 of which two continue to 293.300 — the *after*
shape. Two columns carry a `w:val="double"` 1.5 pt border and two carry a single 0.5 pt one.
**[c#]** A double border is emitted as three bands of `width/3` — `BorderRules.cs`:203 maps
`"double" => 3` and `:99` and `:111` spell the geometry out as *"3.12, a 2.88 gap, 3.12; 9 pt of
room"* for a 3 pt rule — and **[bin]** r146 §2 measured 26.2.4.2 drawing exactly three equal
bands at 7 of 7 sizes. So **every band at this boundary is
0.5 pt thick** and any census that compares *thicknesses* scores it 0. What differs is the
vertical **extent** of the band stack.

`extent-census.py` in this directory generalises the rule accordingly: a boundary is mixed when
two of its x-abutting members have different downward ink extent, which reduces to the thickness
rule for single borders and also catches double-beside-single. See §11.

**`LHD-230` is our page-global grid line, not a border rule.** Before, three runs shared the
grid line `At = 727.2125` with tops 726.9625 (0.5), 726.9625 (0.5) and 727.0875 (0.25); after,
`WidestAt` top-aligned all three. **26.2.4.2 puts them on two different grid lines** — `At`
727.4507 for the 0.5s and 727.3508 for the 0.25 — so in the reference this is not one boundary
at all and the census correctly reads 0. The three runs are 17.5 pt apart in x, so they are not
one table's columns either. This mover is our layout merging two grid lines the reference keeps
apart, and `PageDrawing.WidestAt` then aligning across the merge. That is O82/O84 territory and
this round did not chase it further.

**`PAT-047`, `airbus-pdf-information-package_v1-4` and `xx_SETIS_PWS_template_10.19.22` moved
with not one horizontal band changed, and I could not establish why from the drawings alone.**
Their fingerprint differs and every horizontal band is identical to three decimal places in
position, extent and thickness, so whatever moved is vertical ink, non-band ink, or below that
resolution. r147 §4 found exactly this shape once already — a one-EMU shift in a *vertical* rule
that PyMuPDF could not resolve — and these three are consistent with it, but **consistent is not
confirmed**: I did not diff their content streams and I am not claiming the cause.

## 11. The extent rule, and the corrected population restated

`extent-census.py` replaces "more than one *thickness*" with "more than one *downward extent*",
which contains the thickness rule (for a single border extent == thickness) and additionally
catches double-beside-single. Its three added guards are each there because the naive form
failed a check that is written down in the file:

- the stack starts at the segment's **own** thickness and the continuation must cover **half the
  narrower x run**, because the 1.5 pt mitre overlap on `B11. TE.CAO.00129` otherwise lets the
  neighbour's thickness leak in and the instrument loses the very boundaries it is for;
- the stack may jump a gap of **at most one band thickness**, because a double border is
  line / gap / line of `w:sz/8` each (**[c#]** `BorderRules.cs`:203) — with no gap allowed it misses `150_5335_5a.doc`, with
  6 pt allowed it becomes `census-pdf.py`'s shape (b);
- a **12 pt length floor**, because without it the 8–10 pt hairlines of vector line-art stack
  into "mixed" boundaries and shape (b)'s failure returns in a new costume. **[bin]**

Checked against the four cases whose answer is known before running it: `align.pdf` page 1 → 1,
`150_5300_13_chg8.doc` → 0, `150-5370-10H.docx` → 0, `150_5335_5a.doc` → 7 (> 0, and it moved).

**[bin]** Over all 337 words-track documents:

| rule | reference 26.2.4.2 | ours (r147 after) |
| --- | --- | --- |
| mixed **thickness** (`mixed-census.py`) | 14 docs, 59 bdys | 18 docs, 286 bdys |
| mixed **extent** (`extent-census.py`) | **16 docs, 67 bdys** | 19 docs, 292 bdys |
| one-extent base | 105 docs, 2375 bdys | 49 docs, 1964 bdys |

**The reach figure for seat O85, stated in full: 16 of 337 words-track documents (4.7%) hold at
least one horizontal table boundary that 26.2.4.2 draws with columns of different band extent —
67 such boundaries — against a base of 105 documents and 2375 boundaries drawn as two or more
side-by-side column bands of one extent.** So about one in 36 of the boundaries the rule can
reach, in about one document in seven of those that have such a boundary at all.

Round 146's 49 documents / 770 boundaries stands corrected to **16 / 67**: three times fewer
documents and eleven times fewer boundaries.

Against the 23 movers:

| extent-mixed in reference | in ours | documents | moved |
| --- | --- | --- | --- |
| yes | yes | 15 | **15** |
| yes | no | 1 | 0 |
| no | yes | 4 | **4** |
| no | no | 317 | 4 |

**Nineteen of the 23 movers hold an extent-mixed boundary in one of the two renderings, and 19
of the 20 documents that hold one moved.** The single extent-mixed reference document that did
not move is `words/chartset-005/docx/003_Free_Genogram_Diagram_Template_Easy_Format_60eb6e42.docx`
(1 boundary), and it is the *only* document in the corpus that behaves the way r147's hypothesis
predicted: mixed in the reference, not mixed in ours, therefore nothing to move. One document is
not a mechanism, and I am not claiming it as one.

The four uncovered movers are §10's `PAT-047`, `LHD-230`, `airbus-pdf-information-package_v1-4`
and `xx_SETIS_PWS_template_10.19.22`; §10 says what is and is not established about them.

## 12. The two named documents, final

**[bin]**

| | reference 26.2.4.2 | ours (r147 after) |
| --- | --- | --- |
| `150_5300_13_chg8.doc` mixed thickness / extent | **0 / 0** | 0 / 0 |
| `150_5300_13_chg8.doc` base | 0 | 1 |
| `150-5370-10H.docx` mixed thickness / extent | **0 / 0** | 1 / 1 |
| `150-5370-10H.docx` base | 725 | 725 |

`150_5300_13_chg8.doc` carries no multi-column boundary at all once line-art is excluded — its
tables are drawn as single full-width rules — and it correctly did not move.
`150-5370-10H.docx` carries 725 one-width column boundaries and **one** boundary that is mixed
in our ink only; it moved, on that one boundary.

## 13. What this round could not establish

- **Why `PAT-047`, `airbus-pdf-information-package_v1-4` and `xx_SETIS_PWS_template_10.19.22`
  moved.** Every horizontal band in all three is identical before and after to three decimal
  places. The cause is not visible in the drawings and I did not diff the content streams.
- **Whether `LHD-230`'s move is right.** Our layout puts three border runs on one grid line that
  26.2.4.2 puts on two (`At` 727.4507 and 727.3508), and `PageDrawing.WidestAt` then aligns
  across our merge. Which of the two grid lines is correct is a layout question this round did
  not open.
- **Whether the extent rule's 67 is stable under its own guards.** The 12 pt length floor, the
  one-thickness gap and the half-overlap test each change the answer, and each is justified by a
  single worked case rather than by a survey. A different corpus could need different numbers.
  The *thickness* figure of 59 is insensitive to the length floor (59 with, 59 without) and is
  the more robust of the two.
- **Nothing here was run against the current tree's binary.** No build, no test, no
  `xray-office render`. The "ours" leg is round 147's `after2` output as it was left on disk.

## 14. Disposition of the three instruments

| script | verdict |
| --- | --- |
| `tablerow-r146/census-pdf.py` | **Wrong three ways.** Shape (b) never compares thickness and fires on line-art; shape (a)'s `side` test caps overlap rather than gap and so rejects the mitre a mixed boundary makes. Its `mixed_boundaries` column should not be quoted again. |
| `tbalign-r147/mixed-lines.py` | **Right on the question asked**, and its 0 for the reference on both head documents is correct. The UNRECONCILED warning can come off. It over-counts slightly by ignoring x (60 vs 59 boundaries, 15 vs 14 documents over the track). |
| `mixedcensus-r148/extent-census.py` | This round's instrument. 16 / 67 in the reference, 19 / 292 in ours. |

Files here: `cmp.py` (per-page comparison of the two old scripts), `split.py` (shape (a) vs (b)),
`peek.py` (raw operators at a y), `whymoved.py` (before/after band diff), `mixed-census.py` and
`census-ref.tsv` / `census-ours.tsv` (thickness), `extent-census.py` and `ext-ref.tsv` /
`ext-ours.tsv` (extent), `pairs-ref.tsv` / `pairs-ours.tsv` (the two rendering legs),
`movers.txt` (the 23, recomputed from r147's fingerprints).
