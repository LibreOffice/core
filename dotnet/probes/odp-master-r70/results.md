# `.odp` was the worst column in the first ODF gate because a master page was not drawn

## Environment

    ours   = XRay.Office.Cli @ f4b8c3825 (base) and @ f4b8c3825 + this round
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball)
    fonts  = system fontconfig; ALL FOUR tarball confounds aside as of 2026-09-07 -- the
             Carlito/Caladea/Liberation/DejaVu duplicates, the Latin Noto, and the four
             LiberationSansNarrow. 38 faces in .duplicates-aside, 71 left in the directory;
             `fc-match "DejaVu Sans"` answers DejaVuSans.ttf and `fc-list | grep -i narrow`
             is empty.
    rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
             max(2%, 15), then unembedded fonts.
    corpus = /home/user/corpus-odf (the .odp column, 302 documents) and /home/user/sample-files
             (the original slides track, 302 documents)

Every stored TSV here repeats that header. The reference half was rendered fresh rather than
taken from the bank, and **it reproduces `probes/odf-gate-01/rows.tsv`'s reference columns on
302 of 302 `.odp` rows, page counts and glyph counts alike.** So the *suspect* flag `CLAUDE.md`
puts on every ODF gate row taken before the Narrow faces were moved aside **does not apply to
the `.odp` column**: none of its reference renderings moved. Our half at the base commit
likewise reproduces the banked verdict on 302 of 302, so the instrument here is the scoreboard's.

## The result

| | match | of | |
|---|---:|---:|---|
| `.odp`, at the base commit | 120 | 302 | 39.7% |
| `.odp`, after this round | **247** | 302 | **81.8%** |
| the original slides track, before | 292 | 302 | unchanged |
| the original slides track, after | 292 | 302 | **0 rows moved, 0 verdicts moved** |

129 rows moved from `words` to `match` and 2 from `match` to `words`; net **+127**. On the
original `.pptx`/`.ppt` track **not one of the 302 renderings changed a page count or a glyph
count** — the diff cannot reach it, and that was checked rather than assumed.

## Start with the verdicts, and the brief's first instruction does not apply

The brief said to split the 182 non-matching rows by verdict and take `pages` first, on the
sound reasoning that a deck's page count is its slide count. **There is no `pages` verdict in
the `.odp` column at all.** All 302 slide counts agree; 181 rows are `words` and one is
`words,unembedded`. So the whole column is a text-quantity question, and the strongest signal
available is not the verdict but the *shape of the deficit*.

**That shape is what gave the cause away in ten minutes.** Ranking the 182 by
`ref_glyphs − our_glyphs`, six values account for 93 documents:

| deficit | documents |
|---:|---:|
| 121 | 35 |
| 89 | 25 |
| 87 | 14 |
| 20 | 9 |
| 106 | 7 |
| 141 | 3 |

A deficit that is *identical to the character* across 35 unrelated documents is a fixed block of
text we never draw. `069_Funnel_Diagram_for_PowerPoint_with_3_Steps` is the smallest witness:
one slide, ours 48 glyphs against 68, and the twenty characters are `www.presentationgo.com` —
a `draw:custom-shape` on the **master page**, which `OdpSlideLayout` never looked at.

`006_2-Way_Horizontal_Hierarchy` shows the whole of the 121 and is worth writing out, because
the arithmetic closes exactly: three slides, two of which carry that strapline from their own
master (20 + 20), and a third whose `draw:page` is **empty apart from a page thumbnail** — every
visible thing on it, `Designed with … by … www.PresentationGO.com … The free PowerPoint and
Google Slides template library`, 81 characters, is on the master `Designed_by_PresentationGo`.
20 + 20 + 81 = 121, and after the fix the document is 645 against 645 — which is also exactly
what the *same deck as `.pptx`* has scored all along.

## The classification, by cause

`classify.tsv` carries one row per non-matching document with the measurements each judgement
rests on: what its masters contribute, how many `display="none"` shapes it holds, what its
master's running objects would contribute, and **the same deck's verdict in its original
`.pptx`/`.ppt` spelling** — which is the cheapest discriminator available and was not used
before. A deck that passes as OOXML and fails as ODF is the ODF reader's; a deck that fails both
ways is not.

| cause | rows | witness |
|---|---:|---|
| **A master page's background objects are not drawn** — *closed* | **129** | `slides/done-013/odp/chapter_4_0.odp` 17954 → 18643 of 18883 |
| A master's **running objects** — footer, date-time, slide number — reach only the slides that carry their own copy — *open* | 23 | `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.odp` 27639 → 29151 of 33915 |
| Something else in the ODF reader; no shared cause found among them | 17 | `ev122_jj-liew-tort-of-bribery-webinar.odp` 5594 of 6738, unmoved |
| Master background objects were part of it, and a residual remains | 7 | `SRDMG(16)024_60 GHz onboard airplanes.odp` 8696 → 9571 of 10312 |
| **Shared with the OOXML reader** — the same deck fails its gate as `.pptx`/`.ppt` too, so not an ODF defect | 6 | `OnTrac_StarCertificationProgram-3Day.odp` 7740 ours / 6233 ref, and 7709 / 6233 as `.pptx` |
| Regressed: an unrelated ODF defect crossed the band once the master was drawn | 2 | `038_Competitive_Advantage_Card…odp` 1464 → 1585 against 1449 |

**So the honest answer is not "182 separate things".** 136 of the 182 are the one cause, and a
further 23 are its close relative. Six are not ODF defects at all.

## What was closed, and the three rules it took

`dotnet/src/XRay.Office.Presentations/OpenDocument/OdpSlideLayout.cs`.

### 1. Draw the master page under every slide that names it

`OdpSlideLayout.Page` walked the `draw:page` and nothing else. `InheritedShapes` now walks
`master.Element` first, so the master's shapes are placed *beneath* the slide's — order is the
whole of z-order here, and appending them instead paints a master's full-bleed picture over
every slide, which no gate column can see.

The OOXML path has done this since round 40 (`PptxSlideLayout.InheritedShapes`), with a remark
explaining that **an Impress master page is a PPTX master and a PPTX layout merged into one**.
The two readers of the same deck therefore disagreed, and nothing in the suite could notice:
the sample corpus holds no ODF presentation, and the only master-page coverage in the tree goes
through the *binary* PowerPoint reader (`PptMasterShapeTests`).

### 2. A presentation object on a master is never drawn on a slide

`sd/source/core/sdpage.cxx`:2986-2991 — *"presentation objects on master slide are always
invisible if slide is shown"* — so every `presentation:class` frame on the master is skipped.
Without this, `Click to edit Master title style` appears on every slide of every deck.

`presentation:background-objects-visible` is the one real switch and it is a property of the
*slide's* drawing-page style, not of the master: LibreOffice stores it as the
`backgroundobjects` bit of the slide's master-page visible-layer set
(`sd/source/ui/unoidl/unopage.cxx`:794-809). Five of the 302 state it.

**A master shape parked off the page needs no rule of its own.** The template family that makes
this measurable carries three per master — a copyright line 0.28 cm below the sheet, a credit
group at negative `svg:x`, an instruction block past the right edge — and neither 26.2.4.2's PDF
nor ours carries a character of them, because the media box is the cull on both sides.
`pdftotext -bbox` over the reference finds **no word outside the page**, so this was checked
rather than assumed.

### 3. `draw:display` decides whether a shape reaches paper

`xmloff/source/draw/ximpshap.cxx`:840-844 sets two flags from one attribute:
`Visible = always | screen` and `Printable = always | printer`. Rendering to PDF is printing.

**This is not a curiosity, it is how a PowerPoint master survives a round trip**, and it is the
reason rule 1 alone regressed four documents. Impress can hold only one presentation object of
each kind per master, so LibreOffice's ODP export writes a PPTX master's *Slide Number*, *Footer*
and *Date* placeholders out as ordinary `draw:custom-shape` carrying their prompt text and
`drawooo:display="none"`. Read without the attribute they are background objects, and a reader
that has just learned to draw master shapes puts `<#> Footer Date` under every slide.

**887 occurrences in 49 of the 302, 885 of them on a master page, and every single one in the
extension namespace `http://openoffice.org/2010/draw`** — the exporter always writes
`drawooo:display` (`shapeexport.cxx`:816) while the importer accepts either spelling. A reader
that looks only at `draw:display` finds nothing and concludes the attribute is unused. That is
the same trap `OdfNamespaces.ChartExtension` already records for `coordinate-region`.

Adding rule 3 took the round from 238 to **247** and the regressions from 4 to 2.

## A fourth rule was implemented, measured and withdrawn

Worth recording, because the reasoning was good and the conclusion was wrong.

`SdPage::checkVisibility` answers a footer / header / date-time / slide-number text object from
the visualised page's `HeaderFooterSettings` (`sdpage.cxx`:2957-2984); ODF states those as
`presentation:display-header`, `-footer`, `-date-time` and `-page-number` on the page's
drawing-page style; and across the 302 `.odp` **all 1630 slide-level frames of those four kinds
state one explicitly and every one of them states `false`** — 464 footer, 781 page-number, 385
date-time, no header, and not a single `true` anywhere in the column. It reads exactly like a
switch nobody has honoured.

It is not. That branch of `checkVisibility` is guarded by `bSubContentProcessing`: it fires while
the **master's** content is being drawn as a slide's background, and a slide's own copy of the
frame is an ordinary shape by then. Two measurements refuted the rule before it was committed:

- 26.2.4.2 draws the footer of a page stating `display-footer="false"` in
  `odp-master-background.fodp`, and the footer of the page stating `"true"`, identically.
- Of the 42 corpus documents the rule would have touched, **24 already match** without it, and
  the largest of the rest is 2318 glyphs *short* of the reference rather than long. Suppressing
  would have moved them the wrong way.

`OdpSlideLayout` carries the refutation as a remark and `OdpMasterShapeTests` carries it as a
test, so the next round does not re-derive it.

## What is left, and where the next round should go

**The master's running objects, 23 documents and about 8500 characters.** Where a page states
`presentation:display-footer="true"` (and its siblings), Impress draws the master's footer,
date-time and page-number frames on that slide, filled from the deck's
`presentation:footer-decl` / `date-time-decl` / `header-decl` through the page's
`presentation:use-*-name`, with the slide number substituted. We draw them only on the slides
that happen to carry their own copy. `introduction_to_bea_tuxedo.odp` is the clearest witness:
26.2.4.2 puts `Introduction to BEA Tuxedo`, `UKOUG2008 ©www.go-faster.co.u` and the page number
on every one of its 40 pages and we put them on the first. **42 of the 302 switch a master
running object on**; 24 of those are still non-matching, worth an estimated 8516 characters.

The OOXML path already does all of this (`SlideFields`, and `PptMasterShapeTests`'
`AMastersRunningPlaceholdersReachEverySlideUnderIt`), so this is the same *"two readers of one
format"* gap as the master page itself, one layer down.

Beyond that the residual really is scattered: 17 documents with no shared cause found, and 6
that are not ODF defects at all.

## Two things the instrument taught, both worth carrying

**A worker directory keyed on the document's path is wrong when the corpus holds spaces.**
`soffice` truncates `-env:UserInstallation=file://…` at the first space, then scatters profile
directories through the sweep root while the document silently fails to convert — 27 phantom
directories and a stalled sweep before it was noticed. Keying on a hex digest of the path fixes
it and also fixes the slot-index collision `CLAUDE.md` already warns about. `sweep.py` carries
the comment.

**`sleep` returns immediately in this session's shell.** `sleep 400` between two `date` calls
took three seconds. A poll loop written with it spins, and "the container clock is running
slowly" is the wrong conclusion to draw from it — the harness is eliding the sleep.
`python3 -c "import time; time.sleep(N)"` waits properly.

## Verification

- `dotnet build XRay.Office.slnx -v q -nologo` → **0 warnings, 0 errors**.
- Ten non-fidelity projects, run individually and totalled: **5940 passed, 0 failed, 0 skipped**
  — the 5932 baseline plus this round's 8 new tests, and the eight are all in
  `XRay.Office.Presentations.Tests` (958 → 966).
- `XRay.Office.Fidelity.Tests`: **542 passed, 10 failed, 0 skipped** of 552, the same ten names as
  the baseline — `PageDrawingComparisonTests` ×4, `TabStopComparisonTests` ×4,
  `SheetDrawingComparisonTests`, `JustificationShrinkComparisonTests`.
- Both new rules are pinned by tests that fail without them: reverting rule 1 fails 3 of the 8,
  reverting rule 3 fails 1. Checked with the source restored by `cp` and `touch`, and with
  `obj`/`bin` removed, so MSBuild could not skip the project.
