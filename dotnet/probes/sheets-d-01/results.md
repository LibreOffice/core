# sheets-d-01 — border coalescing, measured

LibreOffice merges collinear, identically-styled cell edges into one stroke; we emitted one per
cell edge. This round establishes the merging rule by authored probe against the installed
**26.2.4.2**, implements it, and measures reach and **direction** across the 171-document sheets
track.

Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/` (171 PDFs, per-format identity
`stem__ext.pdf`). `check-env.sh` before anything else: *LibreOffice 26.2.4.2 620(Build:2)*,
Carlito/Caladea/Liberation/DejaVu all resolving, `pdftoppm` and `pdftotext` 26.01.0,
"Environment is good." Ours: worktree `/c/sandbox/workdir/wt-sheets-d`, branch `wt-sheets-d`, based
on `735e08c525f`; `XRAY_OFFICE_CLI` set explicitly on every render;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

**Measured** means read out of a PDF or produced by an authored probe. **Inferred** means read out
of the C++ and reasoned about — and the C++ here is **27.2.0.0.alpha0+, not the reference binary**,
so nothing is claimed on it alone. The two are kept apart in every section.

---

## 0. The prediction

`prediction.md` beside this file, committed as `9f77f43c589` **before any probe was rendered and
before a line of source was changed**. Scored:

| # | predicted | outcome |
|---|---|---|
| P1.1 | width, colour, dash pattern and sub-line structure each break a run | **right, on all four** |
| P1.2 | a style change gives exactly two strokes | **right** |
| P1.3 | a hole gives two and does not bridge | **right** |
| P1.4 | merging is over the whole emitted set, not per row or column | **right** — and it survives a hidden row or column, which I did not predict either way |
| P1.5 | maximal per grid line; a rectangle never closes | **right** |
| P2.1 | an end with a used collinear continuation gets **zero** extension | **refuted** — the extension is computed regardless; it is the *merge* that discards it |
| P2.2 | therefore LibreOffice does not overlap at a broken joint either | **refuted, decisively** — at a break with a crossing border it overlaps by that border's full width, 2.835 pt on the probe |
| P2.3 | the outer ends keep the crossing extension | **right** |
| P2.4 | the overlap, not the segment count, is the visible defect | **refuted — and this is the round's largest finding**; see §5 |
| P3.1 | 120–165 renderings change, point estimate 150 | **right on the band**, 136; point estimate 14 high |
| P3.2 | 0 of 200 words and 0 of 163 slides | asserted **statically**, not swept — see §7 |
| P3.3 | verdict movement **zero** | **right**, and it holds to the digit on all three gate columns |
| P4.1 | every changed page moves toward the reference in stroke count | **nearly** — 9,463 closer, 4,408 unchanged, **1 further**, and that one is not a coalescing defect |
| P4.2 | hairline pages get measurably closer in pixels | **refuted** — 400 of 408 sampled pages have a **byte-identical** raster |
| P4.3 | 0–5 pages get further | **right** (1) under the shared-class metric |
| P4.4 | the pixel instrument will under-report | **right, and by more than I allowed for** — it reports *nothing at all* |
| §6 | the standing claim in `DrawBorders`' own remarks is wrong | **right** |

Two further refutations the prediction did not reach are in §6 and §8: an existing test asserting
ten strokes where 26.2.4.2 draws four, and the brief's own scoreboard figure.

---

## 1. The instrument, checked before anything was counted

The brief warns that an instrument seeing only one renderer's idiom scores the other's success as
zero. Checked first, on both sides:

* **Both renderers write a cell border as `m … l S` with the width in `w`.** Neither uses `re`, and
  neither encodes the width in the path. A stroke census is therefore symmetric here, and
  `sheets-c-01`'s `Tm`-versus-`cm` trap has no analogue.
* Fills are `re f` on both sides and are collected separately
  (`probes/sheets-d-01/strokes.py` labels every mark with the idiom that made it).

**One thing nobody had recorded, and it changes what a stroke count means.** The reference emits
some strokes **twice, at identical coordinates**. On page 3 of `T0A0D0000090006XLSE.xls` its
nineteen strokes are **seventeen distinct lines**, with `V 380.324` and `H 143.234` each written
twice. That is `Array::CreateB2DPrimitiveRange` expanding its loop one cell beyond the range on
every side (`framelinkarray.cxx:1440-1446`), so the extra column's left edge duplicates the last
column's right edge. Across the 13,872 pages compared below the reference lays down **11.5 million
points** of exactly-coincident repeat, on 11,380 of those pages. Every count in this report is of
**distinct** lines for that reason, and the raw count is reported beside it wherever the two differ.

## 2. The brief's figures, reproduced

| brief | measured here |
|---|---|
| `T0A0D0000090006XLSE.xls` p3: reference 19, ours 103 | **19 and 103** — and the 19 is 17 distinct |
| our segments overlap at the joins by ~0.75 pt | **0.75 pt exactly**: `53.455→108.942` then `108.192→159.540`, the crossing vertical being 0.75 pt wide |
| `grants-2005.xls` p79: reference 60, ours 370 | **60 and 370** — the 60 is 58 distinct |
| `7-memento…b.xls` p2: 34 per-row segments against 5 merged runs | the shape reproduces; on the whole page it is **110 distinct lines against 52** |

Overlap on page 3 totals **64.50 pt**, which is exactly 0.75 pt × (12 horizontals × 3 interior
joints + 5 verticals × 10 interior joints).

## 3. The merging rule, as measured

`tests/corpus/features/sheet-border-runs.fods` — fourteen runs, each varying **one** property
against a four-cell control, plus a second sheet for hidden lines — rendered by 26.2.4.2 itself.
The full stroke census is reproducible with
`probes/sheets-d-01/strokes.py ref/sheet-border-runs.pdf`.

Columns are 2 cm: A 56.665–113.36, B –170.05, C –226.74, D –283.44, E –340.13.

| # | what varies | LibreOffice draws | verdict |
|---|---|---|---|
| 1 | nothing — four identical thin red tops | **1** stroke, `56.665 → 283.436` | control |
| 2 | colour, mid-run | **2**: red `56.665→170.05`, blue `170.05→283.436`, **abutting exactly** | colour breaks |
| 3 | width, mid-run | **2**: 1.4 pt then 2.85 pt, abutting | width breaks |
| 4 | solid → dashed | **2**: the two dashed cells merge with each other | pattern breaks |
| 5 | a hole (C states nothing) | **2**: `56.665→170.05` and `226.743→283.436`; **not bridged** | a gap breaks |
| 6 | single → double | **3**: the double's two lines at ±1.389 pt of centre, the single at centre | sub-line count breaks |
| 7 | a 2.85 pt vertical crossing the interior joint of an unbroken run | **1** stroke, `56.665→283.436`, **no extension at the crossing** | a crossing does **not** break |
| 8 | the same crossing, at a joint broken by colour | **2**: `56.665→**171.468**` and `**168.633**→283.436` — **2.835 pt of overlap**, the vertical's full width | see §5 |
| 9 | one grid line stated by C17/D17's *bottom* and A18/B18's *top* | **1** stroke `56.665→283.436` | merging crosses attributes |
| 10 | a left border on four consecutive rows | **1** stroke, `394.695→462.613`, four rows tall | merging is not axis-specific |
| 11 | the same, green on two rows then blue on two | **2** of two rows each, abutting at 428.654 | colour breaks a vertical too |
| 12 | C25's *left* and B26's *right*, one grid line, two rows | **1** stroke `343.757→377.716` | merging crosses attributes and rows |
| 13 | a run whose two **outer** ends each meet a 2.85 pt vertical | **1** stroke `55.247→284.854` — A's left and D's right each pushed out by **1.4175 pt** | the outer extension survives |
| 14 | a four-cell top run with **column B hidden**; a four-row left run with **row 3 hidden** | **1** stroke each: `56.665→226.743` over three visible columns, `717.306→768.245` down three visible rows | a hidden line does **not** break a run |

**The rule, stated.** Two collinear cell edges that share an endpoint merge into one stroke iff
their border is equal in **width**, **colour**, **line pattern** and **number and arrangement of
sub-lines**. Merging runs along a grid line and never turns a corner, is maximal, crosses the
boundary between the two cell attributes that state one grid line, and is blind to which cell,
row or column an edge came from. A hidden row or column is not in Calc's array and does not
interrupt a run. **The merged stroke keeps the extension at its two outer ends and discards every
interior one.**

**Inferred, and matching:** `tryMergeBorderLinePrimitive2D`
(`drawinglayer/source/primitive2d/borderlineprimitive2d.cxx:300-417`) requires a shared endpoint,
a zero cross product, an equal `StrokeAttribute` (the dash array) and, per sub-line, an equal
`LineAttribute` (width, colour, join, cap) and equal `isGap`; the merged primitive takes A's start
extends and B's end extends, dropping the interior pair.
`SdrFrameBorderPrimitive2D::create2DDecomposition`
(`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:782-841`) offers each new segment to
**every** primitive already accumulated and tries both join orders, which is why the rule is global
rather than per row.

**Inferred, and not measurable from a PDF:** a run cannot cross a printed **band**.
`ScPrintFunc::PrintPage` makes up to four separate `PrintArea` calls per page — repeated corner,
repeated column band, repeated row band, data (`sc/source/ui/view/printfun.cxx:2303-2335`) — each
with its own `ScOutputData`, hence its own array and its own primitive group. Corroborated
downstream rather than proved: page 6 of `2025_Active_Civil_Airmen_Statistics_FINAL.xlsx` shows the
reference's eleven verticals arriving in pairs, `146.92→645.506` and `644.769→662.003`, overlapping
by 0.737 pt at the band boundary — exactly what two separate arrays that both extend into the
crossing horizontal produce. **This half of the rule has no authored fixture and no test** (§9).

### Two predictions that died here

**P2.1 and P2.2 are refuted by variant 8.** I had read `getExtends`
(`sdrframeborderprimitive2d.cxx:301-390`) as pinning an extension to zero whenever a *collinear*
neighbour is in the connect-style vector, because `findCut` fails on parallel lines and leaves the
`CutSet` at its all-zero default, and the minimum is taken. The consequence would have been that
LibreOffice never overlaps at an interior joint, merged or not. It overlaps by 2.835 pt at exactly
such a joint. The extension is computed from the crossing regardless of what runs collinearly;
**merging is the only thing that removes it.** That makes the fix smaller and the existing
extension machinery correct as it stands — and it is the third prediction this session to die to
the gap between the 27.2 tree and the 26.2.4.2 binary.

## 4. The implementation

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetPageDecoration.cs`, one file, +173/−25.

* `Edges.Build` (`:751-838`) records each cell edge as a `Piece` under a `RunKey` instead of
  appending it to a stroke list.
* `RunKey` (`:711-728`) is `(IsHorizontal, Line, Band, Border)`. `Border` is the whole
  `SheetBorder` record — width, gap, secondary, colour, pattern — compared structurally, which is
  the measured rule in one expression.
* `Edges.Coalesce` (`:882-921`) sorts each key's pieces by their position along the run and emits
  one `Edge` per maximal block of consecutive positions.
* `Edges.ExtensionAt` (`:840-844`) is **unchanged and deliberately still answered from the
  uncoalesced index**, so an outer end keeps its overshoot and an interior joint loses it. Rebuilding
  the index after coalescing would have broken every interior horizontal, because a merged vertical
  has endpoints only at the two ends of the whole run.
* `DrawBorders` (`:191-205`) passes the page's repeat ranges so a run stops at a band.

Adjacency is by **placed** index, not by sheet row or column — which is variant 14's answer, and
the reason it is worth a fixture: a hidden line is absent from Calc's array, so keying on the
sheet's own numbering would have split every run that spans one.

### The defect this round introduced and then measured out

The first implementation keyed a run on the grid line's **coordinate** in twips. That is wrong when
two grid lines coincide, which a zero-height row produces. Page 2 of
`6880ac7361ca1b99a9230811_ST Capability List Rev.16 - Web.xlsx` has one: two lines' pieces landed in
one bucket with duplicate indices and the run builder emitted **five mutually overlapping segments**
where there should have been two coincident ones. Measured before it was reasoned about — the
page's overhang went **251 pt → 710 pt** against a reference of 1.8 pt, and it was the largest
single entry in the first direction census. Keying on *which edge of which placed row or column
stated the line* (`2·index`, or `2·index+1` for a bottom or right edge) makes a repeated index
impossible by construction. Recorded here rather than quietly fixed, because "a coordinate is an
identity" is the kind of assumption that reads as obviously safe.

## 5. Direction — and the refutation of the brief's premise

### 5.1 The vector measurement, 136 documents

All 171 rendered twice — once at the pre-fix source (`9f77f43c589`, rebuilt and **validated**: the
probe fixture's page 1 comes out at 55 strokes on that binary against 25 on the fixed one) and once
at the fix — then compared byte for byte with `SOURCE_DATE_EPOCH` pinned on both legs. Both legs
re-counted from disk: **171 of 171** written on each.

**136 of 171 renderings change.** Of the 35 that do not, **33 draw no stroke at all** in their first
five pages; the other two draw only 0.1 pt grid rules, 0.2375 pt furniture and one lone 0.75 pt
border with nothing to merge into.

`probes/sheets-d-01/direction.py` compares, page for page, the 129 changed documents whose page
count equals the reference's (7 are excluded because their pagination diverges, so page *N* is not
page *N*): **13,872 pages.**

| metric | before | after | reference |
|---|---:|---:|---:|
| distinct lines, all classes | 2,761,996 | **350,206** | 364,637 |
| distinct lines, classes both sides draw | 1,281,082 | **221,502** | 227,067 |
| overhang double-ink (pt) | 1,789,135 | **459,213** | 703,617 |
| pages with zero overhang | 2,920 | **12,869** | 12,567 |
| total PDF bytes, all 171 | 75.03 MB | **53.38 MB** | — |

Direction, restricted to the `(width, colour)` classes **both sides draw on that page** — the
restriction matters and is explained below:

> **9,463 pages closer · 4,408 unchanged · 1 further.**
> Pages whose distinct-line count is *exactly* the reference's: **4,122 → 13,498 of 13,872 (97.3%).**

Unrestricted the figures are 11,924 closer / 1,872 unchanged / 76 further, and the 76 are an
artefact of summing across classes. `TK-Syllabus-Comparison-Document-v2.xlsx` accounts for the
largest block of them: its reference draws red 0.794 pt strike-through rules that we never draw at
all, so any count that adds them to the border count scores a missing feature as over-merging.
**A coalescing measurement has to be per stroke class or it measures something else.**

The three briefed documents, distinct lines:

| | before | after | reference |
|---|---:|---:|---:|
| `T0A0D0000090006XLSE.xls` p3 | 103 | **17** | 17 (19 raw) |
| `grants-2005.xls` p79 | 370 | **58** | 58 (60 raw) |
| `7-memento…b.xls` p2 | 110 | **52** | 52 (57 raw) |

All three are now exact, and every coordinate is within 0.035 pt of the reference's.

### 5.2 The one page that gets further, and it is not this change

`6f9e605c-fded-11e3-bd0e-00144feab7de.xls` page 1. Per stroke class:

| class | before | after | reference |
|---|---:|---:|---:|
| H 0.75 pt black — the cell borders | 15 | **2** | **2** |
| H 0.1 pt black | 33 | 33 | 34 |
| V 0.1 pt black | 17 | 17 | **107** |

The border class lands exactly on the reference. The page reads as "further" only because the
shared-class sum still folds in the 0.1 pt **grid**, where the reference draws 107 verticals and we
draw 17 — unchanged by this round, and the opposite defect: `ScOutputData::DrawGrid` emits its rule
**one segment per row**, abutting with a ~0.11 pt gap at each row boundary (`170.561: 103.038→115.653`,
then `115.766→128.380`, …), where we draw one line down the column. Recorded as a lead in §9.

### 5.3 The raster does not change — and that refutes the briefed defect

The brief says the 0.75 pt overlap "doubles the ink on hairlines — so the visible defect is that our
rules look heavier and slightly ragged". **Measured: it does not, and they do not.**

Rendering the before and after banks at 150 dpi and comparing the bitmaps byte for byte over three
pages of each of the 136 changed documents — first, middle, last — **400 of 408 pages are
byte-identical**. At 200 dpi on the briefed pages the ink is equal to the last digit:

| page | ink (px), before | after | reference |
|---|---:|---:|---:|
| `T0A0D…XLSE` p3 | 110,374 | **110,374** | 109,522 |
| `7-memento…b` p2 | 444,167 | **444,167** | 440,501 |
| `criminology…` p1 | 219,940 | **219,940** | 219,956 |
| `00514292` p4 | 90,667 | **90,667** | 90,738 |
| `Aircraft_Database` p2 | 80,930 | **80,930** | 76,706 |

The reason is that the overlap is **interior and opaque**. Segment *A* ends at 108.942, segment *B*
begins at 108.192; *B* covers *A*'s antialiased right edge with its own solid interior, and the union
in *x* is exactly the merged run's span, because the overshoot at each end of the run is the same
either way. Painting opaque black twice on a pixel is idempotent. So the double-inking is real in
the content stream and invisible on the page.

The eight pages that do change, on four documents, and every one moves **closer** to the reference
in total ink:

| document / page | what changed | \|ink − ref\| before → after |
|---|---|---|
| `EASA_PRODUCT_LIST_-_ALL.xlsx` p1 | **dash phase**: 52 `[0.75 0.75] 0 d` strokes become 2, so the pattern runs once across the whole rule instead of restarting in every cell | 1570 → **1532** |
| `List of EQS securities_0.xls` p1 | z-order at coloured crossings | 3828 → **3538** |
| `NPIAS_App_A.xls` p62 | z-order | 14652 → **14573** |
| `PC1000.xls` p1 | z-order, 2 ink units of 293,836 | 21632 → **21630** |

**The dash-phase case is the one place where coalescing is visible ink**, and it is a clean win.
The other three move by two to 290 units out of a quarter of a million, which is a handful of
pixels where a coloured horizontal and a coloured vertical cross and the later stroke wins;
grouping strokes by run changes which one that is, and it happens to change it toward the reference
on all three. `NPIAS_App_A` p62's mean per-pixel distance moves the other way by 0.00002 on a page
already 10% different — recorded, not smoothed over, and below any threshold worth acting on.

**So the honest statement of what this fix buys** is: the content stream now says what LibreOffice's
says (350,206 distinct lines against 364,637, from 2,761,996), the PDFs are **29% smaller**, one
dashed-border document renders correctly, and *the rest is structure rather than appearance.* The
brief's stated symptom does not survive measurement. That does not make the fix wrong — it makes it
a fidelity fix rather than a visual one, and the difference is worth stating plainly because the
next round should not expect pixels to move.

### 5.4 Where we still differ, after the fix

Restricted to shared classes, 13,498 pages of 13,872 are exact. Of the remainder:

* **314 pages have fewer distinct lines than the reference** ("we merge more"), 194 of them on
  `environment-edb-docs-edb-emissions-databank.xls`.
* **60 pages have more** ("we merge less"), 34 of them on `Aircraft_Database.xlsx`.
* **205 pages' overhang moves away from the reference**, headed by `ECA Sinters.xls` (78) and
  `cy01_state.xls` (36) — pages where the reference overhangs at a band boundary or a style break
  and we now do not, i.e. **we are cleaner than the reference**, not dirtier. Our total overhang is
  459,213 pt against its 703,617 pt.
* The reference's **11.5 million points of exactly-coincident duplicate strokes** (§1) are
  reproduced by neither leg and deliberately not chased; drawing a line twice at the same place is
  an artefact of its loop bounds, not a rendering rule.

None of these were investigated to a seat. They are the residue and they are named rather than
rounded away.

## 6. What this refutes in our own tree

**The remarks on `DrawBorders` said Calc does not merge, and cited a real measurement that cannot
show it.** The old text: *"Calc does not [merge], and measuring LibreOffice's own PDF of
`sheet-decor-ods.ods` confirms it — B4's box arrives as four separate `m … l S` pairs rather than one
closed path."* B4's four edges run in four directions and `tryMergeBorderLinePrimitive2D` refuses a
non-zero cross product by construction, so that observation cannot distinguish "does not merge" from
"merges collinear runs only". The same paragraph then argued the segments were harmless because
*"three abutting butt-capped segments … put down the same ink"* — true of abutting segments and
false of the extended ones the code actually emitted. Both sentences are replaced.

**An existing test asserted ten strokes where 26.2.4.2 draws four.**
`SheetMergedDecorationTests.AMergedBlocksBorderIsItsOriginsAndItsInteriorEdgesAreNotDrawn` required
`2·RowCount + 2·ColumnCount` = 10 strokes round a 2 × 3 merged block. Re-rendering that test's own
fixture, `probes/sheets-r37/merge-decor.fods`, under 26.2.4.2 gives exactly **four** `#0000FF` 2 pt
strokes:

```
H 768.245  113.102 → 226.998        V 113.357  717.051 → 768.500
V 226.743  717.051 → 768.500        H 717.306  113.102 → 226.998
```

and ours now sit within 0.035 pt of each. The test's **own docstring already said so** — *"the
reference PDF holds one red rectangle over the whole block, four blue lines round it"* — and its
assertion contradicted it for as long as it existed. Updated, with the measurement in place of the
prose.

## 7. Reach outside the track

**Not swept, and stated as such.** The whole diff from the prediction commit to `HEAD` under `src/`
is **one file**, `XRay.Office.Spreadsheets/Layout/SheetPageDecoration.cs`. The only projects
referencing `XRay.Office.Spreadsheets` are the facade `XRay.Office` and the spreadsheets tests;
`XRay.Office.WordProcessing` and `XRay.Office.Presentations` are siblings and cannot reach it. P3.2 is
therefore a static argument rather than the 534-document sweep `sheets-c-01` owed — that round moved
`XRay.Office.Core`, this one does not. The distinction is the point: a shared-layer move owes a
measurement, a leaf change owes a proof that it is a leaf.

## 8. Verdict movement, and a correction to the brief's scoreboard

**Zero, exactly as predicted, and predicted plainly.** All three gate columns — page count,
letter-or-digit word count, unembedded fonts — are **identical to the digit on all 136 changed
documents**. Recomputed over all 171 against the banked reference:

| gate | before | after |
|---|---|---|
| letter-or-digit words (current) | 146 match, 16 words, 6 pages+words, 3 pages | **identical** |
| raw `wc -w` (superseded) | 144 match, 18 words, 5 pages+words, 4 pages | **identical** |

**The brief's "Sheets is 144 of 171" is the pre-`gate-01` number.** Under the word check the gate
now uses — a token is a word iff it carries a Unicode letter or digit — the track is **146 of 171**,
and has been since that check landed; the 144 reproduces exactly under the old `rawwords` column,
which is what makes the two reconcilable. This round moves neither.

The reference bank was validated against its own manifest before any of this: **171 of 171** page
counts equal `ref-baseline-all.tsv`'s `refpages`, and that file's `refwords` matches my **raw**
column on 171 of 171 and the new metric on 9 — so the manifest is an old-metric artefact and the
new-metric reference words used above are freshly computed from the banked PDFs.

## 9. Tests

`tests/XRay.Office.Spreadsheets.Tests/SheetBorderRunTests.cs`, **13 cases**, every expectation quoted
from 26.2.4.2's own render of the fixture. Fixture:
`tests/corpus/features/sheet-border-runs.fods`, **authored**, not collected.

### Verified by reintroduction

| mutation | detected by |
|---|---|
| coalescing disabled outright (`while (false && …)` in `Coalesce`) | **all 13** |
| the run key made colour-blind (`border with { Colour = Black }`) | 6, including `AColourChangeSplitsTheRunInTwo`, `AVerticalRunSplitsOnColourLikeAHorizontalOne` and `AJointThatIsBrokenStillOvershootsItsCrossing` |
| the run key made pattern-blind (`border with { Pattern = Solid }`) | exactly 1 — `APatternChangeSplitsTheRunInTwo` |

The first mutation proves the cases detect *under*-merging; the second and third prove the splitting
cases detect *over*-merging, which is the risk a single blunt mutation cannot separate. Nothing in
this file is a drift guard.

`AJointThatIsBrokenStillOvershootsItsCrossing` is the deliberate control: it pins the 2.835 pt
overlap that LibreOffice *does* draw, so the fix can never be re-read as "never extend at a joint".

### Not verified, and named

**The band rule has no test.** Forcing `rowBand` and `columnBand` to a constant — deleting the rule
that a run may not cross a repeated header band — is **detected by no test in the project**
(`verify-test.sh` exit 1, confirmed). There is no repeat-rows fixture here and building one needs a
multi-page sheet; the rule is inferred from `printfun.cxx:2303-2335` and corroborated only
indirectly (§3). It is the weakest part of this change and the first thing a successor should pin.

### One existing test changed

`SheetMergedDecorationTests` — see §6. Its assertion moved from 10 to 4 because 26.2.4.2 draws 4;
the change is a correction, not an accommodation.

## 10. Build and test counts

`dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.**

| project | briefed | now | Δ |
|---|---:|---:|---:|
| Core | 298 | 298 | |
| Containers | 109 | 109 | |
| Text | 289 | 289 | |
| Vector | 295 | 295 | |
| Rendering | 121 (1 skipped) | 121 (1 skipped) | |
| Markup | 259 | 259 | |
| OpenDocument | 125 | 125 | |
| WordProcessing | 775 | 775 | |
| Spreadsheets | 643 | **656** | +13 |
| Presentations | 609 | 609 | |
| **total** | 3523 | **3536** | **+13** |

**0 failed.** `XRay.Office.Fidelity.Tests` was not run, as instructed.

## 11. Leads this produced, none of them worked

1. **The printed grid is the mirror image of this defect.** `ScOutputData::DrawGrid` emits its rule
   **one segment per row**, with a ~0.11 pt gap at each row boundary; we emit one line per column.
   Measured on `6f9e605c-…xls` p1: reference 107 vertical 0.1 pt rules, ours 17. It is the only
   remaining source of the "further" verdicts in §5.2, and unlike the border case the reference is
   the *less* tidy side, so a fix here is fidelity-only with a gap at every row boundary to
   reproduce.
2. **Fills are still per-cell.** Deliberately untouched: adjacent fills abut rather than overlap, so
   unlike borders they put down identical ink, and `sheets-b-01` already measured total fill area
   agreeing to 0.3%. A merge here would buy file size and nothing else.
3. **The reference duplicates its far-edge strokes**, 11.5 M points across the corpus. Reproducing
   it would raise our stroke counts to match its raw counts and is almost certainly not worth doing;
   recorded so a future count comparison does not mistake it for a defect on our side.
4. **314 pages where we merge more than the reference and 60 where we merge less** (§5.4), unseated.
   `environment-edb-docs-edb-emissions-databank.xls` (194) and `Aircraft_Database.xlsx` (34) are the
   two concentrations.
5. **`ScPrintFunc`'s band separation is inferred.** §9 names the missing fixture.

## 12. What this round could not see

* **The gate is blind to everything here, by construction**, and this time that is the whole answer
  rather than a caveat: no glyph is emitted by `DrawBorders`, so "146 unchanged" is evidence the
  change is safe and no evidence at all that it is right. §5's vector census is what carries it.
* **The raster is blind to it too**, which is the round's surprise (§5.3) and is measured rather
  than asserted: 400 of 408 sampled pages byte-identical at 150 dpi.
* **Seven documents are excluded from the direction census** because their pagination diverges from
  the reference's — `CIS_Debian`, `FAA-2019-0995-0002_attachment_2`, `ODs-February`, `SIL_TDB648`,
  `ans_mappings_of_eccairs_terms`, `grants-2005`, `orbus_togaf_tool_csq`. Their per-page numbers are
  quoted only where the two sides' page *N* is known to be the same band (`grants-2005` p79).
* **Sub-tenth-point positions are not asserted anywhere.** Ours sit 0.02–0.04 pt from the
  reference's throughout, a pre-existing offset this round neither creates nor closes.
* **The dash *array* is not compared**, only its presence. The reference strokes the fixture's
  dashed border at 1.3889 pt where we use 1.4, and its double rule's two lines sit 2.778 pt apart
  where ours sit 2.834 — two small pre-existing width differences, visible in §3's census and not
  chased.
