# The words documents that PASS, ranked by one-sided ink against 26.2.4.2 — and the first three read

**Measured 2026-09-06 at `ffda5d02e` plus this round's two changes**, in `/home/user/wt-words67`.

| | |
|---|---|
| ours | `XRay.Office.Cli` built from this worktree, snapshotted to a fixed directory so no rebuild could swap it mid-sweep |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 0229ac93fcf0d7cb**, its eight Latin duplicate faces aside |
| gate | `/home/user/gate-2f47/` at `2f4709c08`, whose reference half is 24.2.7.2 and is reused rather than re-rendered |
| corpus | `/home/user/sample-files`, words track, **338 documents**, of which the gate calls **312** `match` |
| metric | `pdf-image-diff.py`'s per-page `\|ink\|%` at 512 px on the long edge, summed — `track-ink-sweep.sh`'s own ranking column — with `verdict.py`'s `pdffonts` comparability screen in front of it |

`rank.py` is the harness. It is deliberately not a new metric: the screen is `verdict.py`'s
`faces()` rule (a *swap* voids a comparison, an extra face on one side only is a note), and the ink
is `compare-images.py`'s `ink_delta` through `pdf-image-diff.py`.

---

## The ranking

Of the 312 gate-passing documents, **307 are scoreable against 26.2.4.2** and 5 are not, all five
because 26.2 paginates them differently from both us and 24.2 — the version gap, to be read rather
than scored:

```
150_5300_13_chg8.doc                     18 / 17
237287_…geolink2neu.doc                   8 /  4
FRE-03_mcar_part-3_and_IS_v2.9.docx      76 / 61
SPA-06_mcar_part-6_and_IS_v2.9.docx      85 / 64
SPA-11_mcar_part-11_v2.9.docx            49 / 39
```

**By total `|ink|%`** (the whole table is `rank-after.tsv`):

| # | document | pages | abs ink | signed | major | worst page | Δwords | note |
|--:|---|--:|--:|--:|--:|--:|--:|---|
| 1 | `PES-Technical-Report-Template_Jan_2019.docx` | 14 | **35.91** | +32.13 | 12 | 2 (8.88) | +2 | |
| 2 | `EHEST-SMS-Safety-Management-Manual-V2.docx` | 82 | 20.07 | +16.54 | 7 | 64 (3.38) | +48 | ref falls back to DejaVu Sans Condensed obliques |
| 3 | `hdss-bulletin-issue-285-25-june-2025.docx` | 10 | **17.98** | +11.48 | 6 | 9 (4.23) | +56 | |
| 4 | `Case-Study-Heathrow-Airport.docx` | 3 | **14.70** | +14.05 | 3 | 1 (11.55) | 0 | |
| 5 | `AirbusCallouts.doc` | 24 | 13.05 | +9.06 | 3 | 2 (3.35) | −3 | |
| 6 | `t_TEMPforInvProgs.docx` | 26 | 11.11 | +5.88 | 5 | 22 (3.06) | 0 | |
| 7 | `part-145-approval list (1).docx` | 8 | 11.07 | +3.16 | 6 | 3 (2.93) | −2 | unscoreable-font |
| 8 | `RMI_…GettingOffOil.doc` | 6 | 10.85 | +9.86 | 2 | 2 (7.58) | 0 | |
| 9 | `AFS-050-004-F2_0i.docx` | 8 | 10.82 | +10.33 | 3 | 8 (3.00) | −2 | unscoreable-font |
| 10 | `docs-quality-MA.IMS.00001-…manual.docx` | 44 | 10.48 | +6.73 | 2 | 30 (2.77) | −7 | |

**By `|ink|%` per page**, which a long document otherwise buries:

| # | document | pages | per page | worst page |
|--:|---|--:|--:|--:|
| 1 | `Case-Study-Heathrow-Airport.docx` | 3 | **4.900** | 1 (11.55) |
| 2 | `016_Project_Timeline_Template_Complete_Guide.docx` | 1 | 4.690 | 1 |
| 3 | `029_Unit_Circle_Chart_Pie_Theme.docx` | 1 | 3.760 | 1 |
| 4 | `028_Unit_Circle_Chart_Optimized_Graph.docx` | 1 | 3.590 | 1 |
| 5 | `027_Unit_Circle_Chart_Graphical_Chart.docx` | 1 | 3.350 | 1 |
| 6 | `023_Unit_Circle_Chart_Circular_Percentage.docx` | 1 | 2.620 | 1 |
| 7 | `PES-Technical-Report-Template_Jan_2019.docx` | 14 | 2.565 | 2 (8.88) |

**The signed column is +32.13, +16.54, +11.48, +14.05 at the head of the table: we draw *more* ink
than the reference on all four.** That is the shape of the first finding.

---

## No fresh reader was available, and the check is two calls rather than a guess

`CLAUDE.md` asks for the pair to go to a subagent, because you cannot un-see a page. **There is no
such tool in this session and the readings below are therefore contaminated by their own author.**
This was tested rather than assumed: `mcp__Claude_Code_Remote__create_session` does exist and does
spawn a sibling, but the sibling gets its own `anthropic_cloud` container, so it cannot open
`/home/user/tmp-words67/pairs/*.png`, and this session has no `list_events` — `SendMessage` answers
*"No agent named … is reachable"* and `get_session` returns the session record without its
transcript, so even a sibling that could see the image could not report back.

Each reading below was therefore taken **before** looking at the document's markup or at anything
stored about it, and then checked against arithmetic that does not depend on it. **One of the three
readings was wrong and the arithmetic caught it**, which is the argument for the discipline: see
`hdss` below.

---

## 1. `Case-Study-Heathrow-Airport.docx`, page 1 — a page border we do not draw at all

**Read blind.** Text, tables, bullets, logo, footer and line breaks identical, word for word. The
reference has a **thick dark-green rectangle running round the whole page just inside the paper
edge, with a black drop shadow on its right and bottom**; we draw nothing there. The reference fits
one bullet fewer on the page.

**Named, before measuring:** (a) `w:pgBorders` is not read at all; (b) it is read and drawn with
zero width or in white; (c) it is not a page border but a paragraph border on a full-width frame;
(d) the one-bullet difference is the border's own space, or is unrelated.

**Settled.** The section's `w:sectPr` carries

```xml
<w:pgBorders w:offsetFrom="page">
  <w:top w:val="single" w:sz="36" w:space="15" w:color="396533" w:shadow="1"/>  … and three more
</w:pgBorders>
```

— 36 eighth-points is 4.5 pt, `396533` is the dark green, `w:shadow="1"` is the shadow. And
`git grep pgBorders -- 'dotnet/src/**/*.cs'` returns **nothing**: page borders are not implemented
anywhere in the tree. Cause (a).

**Reach: 7 of the 272 corpus DOCX declare `w:pgBorders`, and every one of them declares real
`single` borders** — six on all four sides, one on left and right only:

```
092_Business_Case_Template_Convenient_Format.docx      100_Business_Case_Template_Modern_Format.docx
091_Business_Case_Template_Complete_Guide.docx         UAS-SGI_waiver_approval_request_form.docx
Case-Study-Heathrow-Airport.docx                       AFS-050-004-F2_0i.docx
OM template for non-complex NCC operators_August 2016.docx
```

Two of the seven are in this ranking's top ten (#4 and #9). None of the seven is a gate failure, so
the gate has never seen it.

The one-bullet difference (d) is **not** settled: ours holds 354 words on page 1 to the reference's
350, and 456 to 463 on page 2, so the border is drawn `offsetFrom="page"` at 15 pt from the paper
edge and ought not to touch the text area at all. Whether LibreOffice shrinks the text area for it
anyway is left open — it is a *pagination* question and this round did not need to answer it to
draw the border.

### And it is closed

The border is now read and drawn, from geometry measured off 26.2.4.2's own content stream rather
than derived from the specification: four `4.5 w` strokes at `0.2235 0.3961 0.2 RG` with
centrelines at `space + width/2` from the near edges and the same *plus the shadow's own width*
from the far ones, and the shadow as two black bars offset down and right by that width. The one
thing that cannot be guessed is that **the shadow shrinks the box** rather than hanging off the
paper.

Ours against the reference, side by side on page 1 of `Case-Study-Heathrow-Airport.docx`:

| | ours | ref26 |
|---|---|---|
| top stroke | `(15.00, 824.65)-(575.80, 824.65)` | `(15.00, 824.64)-(575.85, 824.64)` |
| bottom | `(15.00, 21.75)-(575.80, 21.75)` | `(15.00, 21.69)-(575.85, 21.69)` |
| left | `(17.25, 19.50)-(17.25, 826.90)` | `(17.25, 19.44)-(17.25, 826.89)` |
| right | `(573.55, 19.50)-(573.55, 826.90)` | `(573.60, 19.44)-(573.60, 826.89)` |
| shadow, bottom bar | `(19.50, 15.00)-(580.30, 19.50)` | `(19.40, 15.04)-(580.25, 19.49)` |
| shadow, right bar | `(575.80, 15.00)-(580.30, 822.40)` | `(575.80, 19.44)-(580.25, 822.49)` |

Every edge within a twentieth of a point; the right shadow bar runs 4.4 pt further down in ours,
where it is behind the bottom bar and paints the same pixels.

`|ink|%` against 26.2.4.2 over all seven, before and after:

| document | before | after |
|---|---:|---:|
| `Case-Study-Heathrow-Airport.docx` | **14.70** | **2.58** |
| `AFS-050-004-F2_0i.docx` | 10.82 | 8.23 |
| `100_Business_Case_Template_Modern_Format.docx` | 1.63 | 0.64 |
| `091_Business_Case_Template_Complete_Guide.docx` | 1.45 | 0.43 |
| `092_Business_Case_Template_Convenient_Format.docx` | 1.27 | 0.19 |
| `UAS-SGI_waiver_approval_request_form.docx` | 0.35 | 0.02 |
| `OM template for non-complex NCC operators….docx` | — | — |
| **total, six scoreable** | **30.22** | **12.09** |

Page 1 of the witness goes **11.55 to 0.07**. The seventh paginates differently from 26.2 and
`pdf-image-diff` rightly refuses it.

**The gate does not move, which is the point.** Scored against `/home/user/gate-2f47/parity.tsv`
with and without the border drawn, the words track is `MATCH 314 PAGES 20 PAGES,WORDS 2 WORDS 2`
both times, with the same three rows differing from the bank. A border adds no words and no pages.
68 sampled words documents that declare no border render **byte-identical** to the pre-change
binary.

## 2. `PES-Technical-Report-Template_Jan_2019.docx`, pages 1 and 2 — the cover art is one page late

**Read blind, page 2 first.** Ours is a full-bleed cover: a green band top and bottom, a pale-blue
line-art substation across the whole page, the IEEE PES and IEEE logos. The reference's page 2 is
nearly empty — a header, `THIS PAGE LEFT BLANK INTENTIONALLY`, and the folio `ii`. First reading:
*"we repeat a first-page header on every page"*, which `CLAUDE.md` lists as a known corpus class.

**The per-page report refutes that immediately**, and it is why the image is a lead and not a
cause. Page 1 reads *"ink **missing from ours**"* over 18.82 % of the page bottom and 8.45 % of the
top; page 2 reads *"ink **we draw** that the reference does not"* over 19.05 % and 4.16 % — the same
regions, the opposite sign. The artwork is not repeated. **It is one page late**, and so is
everything after it: our page 3 carries the reference's page 2, our page 4 its page 3, our page 5
its page 4, while both documents end at 14 pages.

**Named:** (a) our section 1 emits a second page that the reference does not, and the cover's
anchored frames land on it; (b) `w:titlePg` in section 1, which declares only a `default` header,
makes us synthesise an empty first page; (c) a frame anchored `relativeFrom="paragraph"` at
`posOffset` 9 315 450 EMU = **733.5 pt** on a zero-margin 792 pt page pushes past the page bottom
and we move it to the next page where Writer clamps it.

**Partly settled.** Section 1 is `pgMar` all zero with `w:titlePg`; its body holds five
`wp:anchor`/`wp:inline` drawings and four `w:pict`, at paragraph offsets of 36, 234 and 733.5 pt;
neither header part holds any drawing at all, so the artwork is body content. Our page 2's text
layer holds **only the running header** — no body paragraph — so it is a page that exists to hold
frames. That is (a), and (c) is the mechanism most likely to produce it. The discriminator not yet
run is a dump of the laid-out blocks and frames per page, which says whether page 2 holds a
paragraph or only frames.

## 3. `hdss-bulletin-issue-285-25-june-2025.docx`, page 9 — and a reading the measurement refuted

**Read blind.** Ours is a full page: a blue angled banner across the top, a running header, two
headings, prose, bullets, numbered items, the Victoria Department of Health logo bottom-left. The
reference's page 9 is the *tail* of ours — it starts at the two bullets we have near our bottom —
and has **no banner and no running header**. Ours also appeared to be **missing `OFFICIAL` beside
the logo**, which the reference draws.

**`OFFICIAL` is drawn, and the reading was simply wrong.** `pdftotext` finds it 11 times in both
PDFs, and `pdf-ops.py` puts it at `(275.32, 31.40)` in ours against `(275.50, 31.29)` in the
reference, 10 pt DejaVu Sans in both. A confident, specific, false observation — from a reader who
had already spent the round looking at pages. It is recorded because it is the argument for the
uncontaminated reviewer this container cannot provide.

**What survives the check** is the pagination: per-page word counts run
`240/237, 323/342, 397/407, 393/472, 436/400, 404/465, 353/430, 496/443, 385/180, 167/162` — they
wander both ways rather than drifting one way, and both documents end at 10 pages. So it is not a
single accumulated deficit; content is distributed differently around several page boundaries.

**Named, none settled:** (a) our line height is short, so more lines fit; (b) the banner or header
takes less vertical room in ours, giving the body more; (c) paragraph space-before/after is smaller
in ours; (d) a keep-with-next or page-break rule the reference honours and we do not. The
discriminator is `first-divergence.py` on page 4, the first page where the counts separate by more
than 3 %.

---

## What this ranking is worth as a habit

The gate calls all four of the documents above `match`, and three of the four are wrong in a way a
reader would notice from across a room. **Page borders were an entire unimplemented feature reached
by 7 corpus documents and no gate column could see any of it.** It is closed in this round; the
other two findings are named and left.

## Reproducing

```sh
./render-ref26.sh 6                                   # the 26.2.4.2 reference half
./render-ours.sh <cli> <outdir> 5                     # ours, with SOURCE_DATE_EPOCH set
python3 rank.py <ours-dir> <ref26-dir> rank.tsv 4     # the ranking
./makepair.sh <id> <page> 120                         # one labelled image to read
python3 score.py <ours-dir> /home/user/gate-2f47 words/   # the gate, reference half reused
```

`score.py` replays `batch-check.sh`'s rule **as of 2026-09-05** — page count, then max(2 %, 15)
*alphanumeric characters* — against the banked gate. Written first with the older token rule, it
reported eight verdict movements of which five were the rule and not the tree; the gate's ninth
column (`glyphs`) is what says which rule a banked run was scored with.
