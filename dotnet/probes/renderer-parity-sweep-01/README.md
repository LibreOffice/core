# Renderer parity sweep 01 — eight-lane fix investigation

Output of one round: the whole `sample-files` corpus rendered by X-Ray.Office and by headless
LibreOffice, the 192 documents that did not match catalogued, then split across eight
parallel agents partitioned by **source-file ownership** so their proposed patches land on
disjoint files. The harness that produced the sweep is `dotnet/scripts/renderer-parity/`.

## The environment this was measured in

`dotnet/CLAUDE.md` records that a stored figure is evidence about an environment rather than
about the code, and that stored data almost never says which environment. So, first:

| | |
|---|---|
| Reference binary | **LibreOffice 24.2.7.2** 420(Build:2) — `Producer: LibreOffice 24.2` in every reference PDF |
| Fonts | Carlito, Caladea, Liberation installed; `fc-match Calibri` → Carlito, `fc-match Cambria` → Caladea |
| X-Ray.Office | `582c8c671`, built Release, `net10.0/linux-x64` |
| Corpus | `theolivenbaum/sample-files` @ 946 documents |
| Comparison | 150 dpi, first five pages per document; page counts compared in full |

**This is not the binary the tree is developed against.** `dotnet/CLAUDE.md` § "This
container" records the project on **26.2.4.2**. That gap is the single largest finding of the
round and it is why several patches here are marked as retargets or `DO-NOT-APPLY`.

## Read this before acting on anything below

**At least 30 of the 192 documents are version divergences, not defects** — the tree is
correct for 26.2.4.2 and the reference bank is a release behind:

| lane | documents | evidence |
|---|---:|---|
| L5 slides | 24 of 35 | 24.2.7.2 answers a 2.5% autofit grid; four of its nine measured steps cannot be produced by `constScaleLevels` at any rounding. `Lepore.ppt` p2: `SlideAutofit.cs:250-255` records 20.013/20.409 pt measured on 26.2.4.2, **we draw exactly that**, this reference draws 21.005/21.005 |
| L6 sheets | 3 of 24 | our page counts **are** the 26.2 numbers exactly — 449, 88, 201 — for the three documents `CLAUDE.md` already names as having moved |
| L1 text | 2 of 6 | `SlideAutofit.cs` and `SystemFontResolver.cs:490-499` both state in their own remarks that they were measured on 26.2.4.2 |
| L8 drawing | 1 of 14 | the corner→linear gradient branch was removed in round 59 on 26.2 evidence; round 59 and `probes/slides-b-01` both record that they *could not* test 24.2.7.2, so "the reference changed" was inference — **it is now a measurement**, six arms on the installed binary, condition reproduced exactly |
| L2 docx | contested | the `w:trHeight` floor and section-break space-before; 23 of 28 documents carry `trHeight` rows |

Three lanes state they would have patched the wrong way without this being flagged mid-round.

## Apply order and tooling

Patches are per-root-cause and were each verified with `git apply --check` **and**
`patch -p1 --dry-run` from the repository root. Nothing here was built or tested — the
checkout was read-only for the whole round, by design, because eight agents sharing one tree
and four cores is exactly the condition under which `CLAUDE.md` records a rebuild landing
under someone else's measurement and a loaded test run reporting `Failed: 0` after dropping
tests silently.

- **L4**: `field-instruction-nesting` **before** `form-checkbox` (both rewrite the same guard).
- **L7**: the six independent diffs in any order, then `series-name-from-range`, then
  `unnamed-series-legend` (its context does not exist on a pristine tree; its own `#` header
  says so).
- **L8**: RC-1's fill limb must land **before** RC-3's z-order work, or `#062` turns from
  "no background" into "background over everything".
- **L5**: `git apply` only — the diffs carry placeholder `index` lines that `patch(1)` rejects.
- **L6**: `DO-NOT-APPLY-digit-width-carry-24.2.diff` is exactly what its name says.

A caution worth keeping: `git apply --check --recount` **silently repairs drifted `@@` hunk
counts**, so a hand-assembled diff passes it and fails every consumer that does not pass the
flag. One lane reported four clean patches on that basis when three were corrupt. Verify with
both tools, without `--recount`.

## Three more read-but-never-consumed properties

`CLAUDE.md` records this pattern as the cause four times and advises grepping for it on
purpose. Doing so found three more, plus two declared-with-no-consumer record types:

| property | reach | symptom |
|---|---|---|
| `w:w` character scaling | 11 of 271 corpus DOCX | spans 2.6% narrow; `<w:w>` appears nowhere in `dotnet/src` |
| `c15:datalabelsRange` | 5 sheets documents | data labels print the literal `[CELLRANGE]` |
| preset sub-path `fill`/`stroke` | 96 of 320 sub-paths say `fill="none"` | every connector renders as a filled triangle |
| `PPT_PST_TextRulerAtom` (4006) | all 14 `.ppt` in L5 | shape-level indents fall back to the master's |
| `PPT_PST_ExtendedParagraphAtom` (4012) | 3 documents | `(a)(b)(c)` and `I. II. III.` come out as round dots |

## What redirects the roadmap

L1 measured **296,847 aligned glyphs** and found the advance-width divergence explains
**17 of the 107 reflow documents (16%)**, not the bulk — the project's working assumption is
wrong by about a factor of six. 40 of the 107 break every shared line *identically*; their
divergence is vertical. Per-face ratios (Carlito 1.00108, DejaVu Sans 1.00069, Liberation
Sans 1.00009, Liberation Serif 0.99958) confirm and sharpen the seat already in `CLAUDE.md`
and show it survives the version move. No patch: closing it means reproducing FreeType's
hinted advance at LibreOffice's ppem, which is architectural.

## Readings in the published catalogue that these lanes refuted

Corrections to the sweep's own case notes, each established by measurement rather than by a
second look at the same image:

- **#029, #133** — not a page-numbering offset; neither file contains `w:pgNumType`. The
  reference prints a frozen cached value on every page because the `PAGE` field sits in a
  text box in a footer table, where Writer's draw-layer outliner has no page field at all.
  **Ours is correct on both.**
- **#055** — the data-point markers are drawn by both engines, and both draw the same grey
  grid; only the *legend key* lacks its symbol. `TODO.batches.md:16815,16966` already records
  two reviewers making this identical misreading on this identical page.
- **#002** — the black plot area is stated by the file and drawn by the reference too, on
  *its* page 8; our pagination put a different page under the comparison.
- **#033** — the ideographic comma is drawn; both PDFs read `1、` and `A、`.
- **#080** — the document contains no empty paragraphs at all; the gap is a 280-twip auto
  margin never handed back when a list ends at a cell wall.
- **#066** — a print-zoom difference, not missing borders; both engines draw the rules and
  bands. **#180** — the title rows collide in *both* renderings.
- **#049** — the hatch is correct; the block looks bigger because the rows are taller.
- **#176** — same image at identical size on both sides; it is on the wrong page.
- **#113** — the file uses thirteen spaces, not a tab.
- **#175** — recommend re-tagging `lo-broken`: a bar-of-pie that 24.2 cannot draw and we can.

## A second pass, with the reference's version held out

The findings above are entangled with the reference binary: 29 of the 43 corrections
they produced are version divergences, true of 24.2.7.2 and not of the 26.2.4.2 the
tree is built against. A second pass therefore re-checked the catalogue using only
evidence that is a statement about **our own output** -- what our PDF's content stream
holds, in what order it paints, and at what size. None of it can be invalidated by a
reference move, because the reference is not party to it. The instruments are
`audit.py`, `claims.py` and `zorder.py` beside the harness.

**One cause behind five separate readings: paint order.** In `060_Human_Body_Concept_Map`,
`019_` and `013_Project_Timeline`, `050_Visual_Product_Roadmap` and `045_Visual_Product_Roadmap`,
content the metrics report as *missing* is drawn and then painted over. `045`'s
`2021` is shown at stream offset 2473 and the black year box is filled at 4180, on top
of it. Each was established three ways -- an opaque fill covering the text block's
anchor at a later offset, a uniformly flat patch where the glyphs should be, and the
crop beside the reference's -- and the reference hides nothing on any of the same pages.
57 text blocks in all. It is a different fix, in a different place, from missing content.

**A degenerate header scale, not a missing title.** `Hazard Analysis Template.xls`
emits its header at **0.120 pt** where the reference sets **7.887 pt**, in the same
face and colour, correctly centred. The page carries exactly two spans under 1 pt --
the header and the footer -- and every other span is 10 pt, so one scale fault accounts
for both of the losses that reading named.

**Two readings overstated a reflow as a loss.** `RobertQ_Service.doc`'s numbered line
is on our page 3 and `087_Printable_Graph_Paper`'s `Title:`/`Date:` line on our page 2.
Neither is missing; in the second the oversized grid pushing that line onto a page the
reference does not have *is* the page-count divergence.

**Seven readings were upheld** by an independent check, and one class of 25 candidate
refutations dissolved entirely on inspection -- every one was a quote the reading used
as context rather than as the thing it called missing. Both outcomes are recorded in
`pl-readings.json`, which carries the 192 readings without their images.

## Layout

```
xray-office-differences.html   the published catalogue, self-contained (11.6 MB)
BRIEF.md              the shared contract every lane worked to
L<n>-<lane>/
  summary.md          root causes, seats, document counts, confidence
  findings.md         evidence, markup quotes, file:line, refuting probes
  patches/*.diff      one per root cause
```

## Outcome: where the 192 stand now

26 of the 30 lane patches landed, plus two root causes that had no patch, across
eight commits. Every document is in exactly one bucket, and `classify.py` puts it
there by measurement rather than by which patch claims to cover it:

| | | |
|---|---:|---|
| **fixed** | 11 | the rendering moved toward the reference between the sweep's binary and this one |
| **version mismatch** | 29 | correct for 26.2.4.2; the sweep's reference is 24.2.7.2 |
| **LibreOffice bug** | 9 | the reference is at fault and our output is the better one |
| **open** | 143 | a named cause, listed below |

Open, by cause: reflow / advance divergence 32, field values 23, table rules 18,
character styling 13, page count only 11, graphics we do not draw 10, charts 9,
overlap and clipping 9, list markers 9, content we do not draw 5, preset shape
geometry 4.

**Counting fixes by SSIM alone under-counts them, and this round has the clean
demonstration.** `Hazard Analysis Template.xls` went from **0.791 to 0.780** while
its header went from an invisible 0.120 pt to 7.920 against the reference's 7.887:
drawing content that was previously missing adds ink that does not land
pixel-perfect, so the score falls while the page gets better. Four of the five
paint-order documents move by less than 0.01 SSIM while their hidden-text counts go
16→0, 5→0, 17→1 and 1→0. The classifier therefore accepts a defect-specific
measurement as evidence of a fix, which is the same corroboration rule
`aggregate.py` uses in the other direction.

### Two root causes that no lane had a patch for

**Anchored frames were painted in document order.** A `wp:anchor` declares its own
`relativeHeight` and we never read it. Measured over the five documents where it
showed: all five declare it on every anchor -- 20 to 44 each -- and **not one is in
document order**. It does not present as a z-order fault but as missing content, so
five separate catalogue readings were this one cause. `060_Human_Body_Concept_Map`
rendered as a single blank grey sheet because we drew the whole page and then the
background over it.

**An unset page zoom was read as 1%.** `Math.Max(1, ZoomPercentage) / 100.0` stood
in four places and clamps to one *per cent*, so a placement that never had a zoom
set drew its bands at a hundredth of the stated size. `Hazard Analysis Template.xls`
states `&C&"Arial,Bold"&12` and we drew 0.120 pt.

### Held deliberately

`autofit-version-divergence` and `DO-NOT-APPLY-digit-width-carry-24.2` retune the
tree to 24.2.7.2 and say so. The two L2 patches are the same class without saying
so: `row-height-floor` fixes three fidelity comparisons here and breaks six unit
tests whose stored values `PROVENANCE.tsv` dates to **26.2.4.2**, and the lane's own
summary reads "a `w:trHeight` floor is raised by the row's border and cell margins;
24.2 raises it by neither". Landing them would make this container's numbers better
and the project's reference worse.

## The fidelity suite, after the fixes

Three more rounds landed since the table above, and the suite reached a state worth
recording: **every remaining failure is either the reference-version gap or the
advance-width divergence.** Nothing tractable is left in it.

| reference | failed of 552 |
|---|---:|
| distro 24.2.7.2 (this container) | **15**, from 18 |
| TDF tarball 26.2.4.2 | 30 |

Exactly **one** fails against both — `NoteRestartComparisonTests` on `note-restart.docx`
— and it is down to one 11 pt body line breaking elsewhere. Its note spans are
identical to the reference's, four each at 5.8, 6.4 and 10.0 pt, so the numbering and
the placement are right and what is left is the divergence `dotnet/CLAUDE.md` records
as architectural.

### Two of the four "ours" turned out to be stale literals

Both were tests carrying hard-coded numbers from a reference version, which is why each
failed against *both* binaries — once because ours no longer matches the old version,
once because the literal does not match the new one.

**`SlideAutofitParagraphSpaceComparisonTests`** asserted three font sizes that were all
24.2.7.2's. Measured on its fixture with nothing changed but the binary:

| box | 24.2.7.2 | 26.2.4.2 | ours |
|---|---:|---:|---:|
| 150 pt | 11.991 | 14.003 | **14.0031** |
| 200 pt | 17.008 | 18.992 | **18.9921** |
| 210 pt | 18.000 | 20.013 | **20.0126** |

Ours is 26.2.4.2's to the hundredth on all three, the control included. The autofit
ladder moved between the releases — `SlideAutofit.FitLevels` ports 26.2's
`constScaleLevels` where 24.2 bisected — so nothing in the renderer was wrong.

**`SheetSpilledTextComparisonTests`** demanded Calc's lead-in that `SpreadsheetPages`
deliberately dropped. Worth more than the fix: the mechanism `SpreadsheetPages` cites
for the difference — the `!bTaggedPDF` guard — **does not explain it**. Both binaries
write a tagged PDF for that document, `/MarkInfo /Marked true` read out of each, and
they differ anyway.

### One real fix

A note now keeps the page that cites it. The room offered to a paragraph was
`columnBottom - NoteHeight(notes)` where `notes` is what was cited *before* it, so a
citation inside the very lines being fitted was charged nothing and the note flowed away
from its anchor. On `footnote-pages.docx` page one that was 54 lines against the
reference's 57 with Note 12 pushed off; it is now 56 with both notes in place.

An earlier round did this unconditionally and reverted it, naming the document it broke.
The guard is what that case needed: shorten only when it leaves the paragraph something
here, and only when it actually buys the note. `template---tpr…docx` renders 7 pages
before and after.
