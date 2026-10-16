# A master's running objects, an autofit nobody read, and a blur in the wrong namespace

## Environment

    ours   = XRay.Office.Cli @ fdab86b6a (base) and @ fdab86b6a + this round's three commits
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball)
    fonts  = system fontconfig; all four tarball confounds aside -- the Carlito / Caladea /
             Liberation / DejaVu duplicates, the Latin Noto, and the four LiberationSansNarrow.
             `fc-match "DejaVu Sans"` answers DejaVuSans.ttf; `fc-list | grep -i narrow` is empty.
    rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
             max(2%, 15), then unembedded fonts.
    corpus = /home/user/corpus-odf (the .odp column, 302 documents) and /home/user/sample-files
             (the original slides track, 302 documents, scored against /home/user/gate-2f47)

The reference half is **reused unchanged** from `probes/odp-master-r70/ref.tsv`, which that round
rendered fresh and which reproduced `probes/odf-gate-01/rows.tsv` on 302 of 302 rows. Nothing in
this round's diff can reach `soffice`, so re-rendering it would have measured the same bytes four
times over. Our half was re-rendered at the base commit first and reproduced the briefed
**247 of 302** exactly, which is what makes the rest of the numbers comparable.

## The result

| | match | of | |
|---|---:|---:|---|
| `.odp`, at the base commit `fdab86b6a` | 247 | 302 | 81.8% |
| after the master's running objects | 259 | 302 | 85.8% |
| after `style:shrink-to-fit` | 283 | 302 | 93.7% |
| after `loext:shadow-blur` | **285** | 302 | **94.4%** |
| the original slides track, before | 292 | 302 | |
| the original slides track, after | 292 | 302 | **0 rows moved by this diff** |

39 rows moved to `match` and 1 the other way; 110 of the 302 renderings changed at all.

**The one that moved the other way is the raster ceiling and not a regression in the output.**
`16 - UTM - (NASA).odp` is 14165 characters at the base against the reference's 13981 and 15082
after. On its pages 7 and 29 the reference draws a *picture* where we draw an embedded object's
real, searchable text — page 29 is 550 characters ours against 2 — so teaching the fit to draw
more drew more of what the reference never draws at all. It sits in `slides/ceiling-002`, which is
where the corpus already files that.

**The original slides track moves nothing, and the one row that differs from the bank was already
different before this round started.** `N2_E_Maestroni_Swarm_COP.pptx` reads 29088 in
`/home/user/gate-2f47/rows.tsv` and 28100 here. That is not this diff: a binary built from the
base commit's sources — the three files reverted with `git checkout`, `touch`ed, and
`obj`/`bin` removed so MSBuild could not skip the project — renders it at **28100 as well**. The
mover is one of the chart commits between the bank's `2f4709c08` and `fdab86b6a`. Every other one
of the 302 is byte-for-byte identical in pages and glyphs.

## 1. The master's running objects — the brief's group of 23

### The mechanism

The previous round's rule 2 — *skip every `presentation:class` frame on a master* — is right for
four of the five kinds and wrong for the fifth, and the C++ makes the split in two different
places rather than one.

`SdPage::CreatePresObj` marks a master's **title** (`sd/source/core/sdpage.cxx`:305-310),
**outline** (:314-321) and **notes** (:325-332) placeholders `SetNotVisibleAsMaster(true)`, and
marks nothing else. `ViewObjectContactOfSdrObj::isPrimitiveVisible`
(`svx/source/sdr/contact/viewobjectcontactofsdrobj.cxx`:81-85) drops any object so marked while a
master is drawn as a slide's sub-content. A **subtitle** is not marked, because it is
`PresObjKind::Text` (`sd/source/ui/unoidl/unopage.cxx`:427-430); it goes one line later, to
`SdPage::checkVisibility`'s *"presentation objects on master slide are always invisible if slide
is shown"* (`sdpage.cxx`:2986-2991), whose guard is `eKind != NONE && IsMasterPage() &&
pVisualizedPage != pCheckPage`.

The **header, footer, date-time and slide number** take neither route. They are answered by the
branch immediately *above* that one (`sdpage.cxx`:2957-2984), which reads the **visualised page's**
`HeaderFooterSettings` — so each is drawn on exactly the slides whose own drawing-page style
switches it on. ODF states those four bits as `presentation:display-header`, `-footer`,
`-date-time` and `-page-number` (`xmloff/source/draw/sdpropls.cxx`:376-379).

**The defaults are not uniform and only one of the four is off.** `HeaderFooterSettings()`
(`sdpage.cxx`:3222-3230) sets header, footer and date-time visible and the *slide number* hidden.
Every one of the converted corpus's **8810** page-to-running-object pairings states its attribute
explicitly, so no corpus row can witness a default; the probe's fourth page is the only place they
are visible.

**What such a frame draws has two shapes and they are not interchangeable.** LibreOffice's ODF
export writes a master's running object either with the characters in it, in which case those
characters are drawn on every slide the switch reaches, or with a `presentation:footer` /
`-header` / `-date-time` element, which is a *field* and resolves against the slide's own
`presentation:use-footer-name` and siblings through the document's declarations
(`xmloff/source/draw/ximppage.cxx`:301-360 copies the declaration onto the page as `FooterText`;
`sd/source/ui/app/sdmod2.cxx`:374-425 is what the `SvxFooterField` then reads). Over the 302
`.odp` the master frames divide **983 footers carrying a field against 11 carrying characters**,
and **992 date-times against 18**. Both witnesses are in the corpus and they disagree, which is
what settles it:

- `ws_prod-…-M.017-(French)-France.odp` states both at once. Its footer declaration says
  `DGINT/2`; its master's footer frame holds the characters `WG M.017: Amendement du Part-M…`;
  26.2.4.2 draws **the characters**. Its `Title1` master's *page-number* frame holds the
  characters `France`, and the reference draws `France` where a slide number would go.
- `introduction_to_bea_tuxedo.odp`'s master frames hold fields, and 26.2.4.2 draws **the
  declarations** — `Introduction to BEA Tuxedo` for the date-time and `UKOUG2008 ©www.go-faster.co.u`
  for the footer, on all forty pages.

`text:page-number` draws the slide's own one-based number. Its element content is the placeholder
LibreOffice stores against the field — the literal string `<number>` — and drawing that content is
drawing the placeholder. **1188 of the corpus's master page-number frames carry one.**

### The seat

`OdpRunningObjects.cs` (new), `OdpSlideLayout.InheritedShapes`/`Walk`, and `OdfTextBody.Collect`.
The running objects are resolved once per slide and travel down the walk as a parameter rather
than as a field, because one master frame draws different words on different slides and the
declaration therefore cannot be resolved per master.

### The reach

**42 of the 302 switch at least one master running object on**, worth 16 321 alphanumeric
characters. The `.odp` gate went **247 → 259**, and **21 of the brief's 23** rows are now `match`.
The two that are not are `manufacturing_process_simulation_working_group_overview_2023.odp` and
`redac-sas-201509-asisp-research.odp`, both of which had a much larger residual from another cause
(1749 and 606 characters against 6 and 74 of running objects).

Four documents moved the *wrong* way on this commit alone and all four were the page number: a
slide's own `text:page-number` used to draw the six alphanumeric characters of `<number>` and now
draws one or two, so a document sitting inside the band on a compensating error left it. Three of
the four came back with the next commit and the fourth (`chapter_4_0.odp`) with the one after.

## 2. `style:shrink-to-fit` — the largest thing the round found, and it was not in the brief

`SlideAutofit` has been a full port of `SdrTextObj::autoFitTextForCompatibility` since round 52
and **the ODF reader reached none of it**, because ODF states one property two ways.
`drawing::TextFitToSizeType` is mapped from *both* `draw:fit-to-size` and `style:shrink-to-fit`
with `MID_FLAG_MERGE_PROPERTY` (`sdpropls.cxx`:143-144): the first carries
`false` / `true` (proportional) / `all` (all lines) / `shrink-to-fit` (autofit) through
`pXML_FitToSize_Enum` (:677-684), the second carries the autofit bit alone through
`pXML_ShrinkToFit_Enum` (:686-693). The second spelling exists because an ODF 1.1 consumer reads
`draw:fit-to-size="true"` as *stretch*, so LibreOffice writes an autofitted shape as
`draw:fit-to-size="false" style:shrink-to-fit="true"` — and a reader that consults only the first
attribute concludes the shape does not autofit.

**Reach: `style:shrink-to-fit="true"` appears 3515 times, in all 302 of the converted corpus's
`.odp`.** All 25 525 of their `draw:fit-to-size` say `false`, so the ODF-namespace spelling
decides nothing anywhere in the corpus.

**No gate column could see it on a shape whose text merely overflowed**, which is why it survived:
the characters are the same either way and only the line count and the size move. It is visible in
the gate only where the overflow ran off the page — `manufacturing_process_…odp` page 2 is 161
characters against the reference's 1193 without it and 1328 with it, and page 7 goes from 57
against 768 to exactly 768.

Only the autofit value is honoured. PROPORTIONAL and ALLLINES — Impress's *fit to frame*, which
stretches the glyphs rather than choosing a smaller size — are a different transform, modelled on
none of the three presentation readers, and no corpus document states either.

`.odp` gate **259 → 283**.

## 3. `loext:shadow-blur` — the third instance of one trap

Four of a shadow's five attributes are `draw:`; the fifth is not. `PROP_ShadowBlur` is mapped to
`XML_NAMESPACE_LO_EXT` and to nothing else (`sdpropls.cxx`:169), so a reader looking for
`draw:shadow-blur` finds the attribute nowhere and reads every shadow as hard-edged.

That is not a cosmetic radius, because `SlideShadow.CarriesText` keys on it. LibreOffice
rasterises a blurred shadow — `ShadowPrimitive2D` renders its children to a bitmap and softens
that (`drawinglayer/source/primitive2d/shadowprimitive2d.cxx`:91-140) — so the reference's PDF
holds a picture with **no text**, while a hard shadow stays vector and its text is real. Read
without the extension namespace, every blurred shadow in the file put a second offset copy of its
shape's words into our text layer.

**1252 non-zero `loext:shadow-blur` in 120 of the 302, and zero `draw:shadow-blur` anywhere.**
The smallest witness is `010_3-Group_Hierarchy_for_PowerPoint_and_Google_Slides`: 895 characters
against the reference's 715, and 715 after. `pdftotext -bbox` shows the extra 180 as nine
`Lorem Ipsum` drawn twice, **2.098 pt apart in both axes** — which is `draw:shadow-offset-x="0.074cm"`.

This is the third instance of one trap and they should be read together:
`OdpSlideLayout.IsPrinted` records `drawooo:display`, and `OdfNamespaces.ChartExtension` records
`coordinate-region`. In each, the ODF-namespace spelling is what a specification reader looks for
and appears in no file the reference binary writes. **Before implementing any ODF attribute, grep
the corpus for both spellings.**

`.odp` gate **283 → 285**, on 6 renderings changed.

## What the brief's six "shared with the OOXML reader" rows turned out to be

They are **not one thing**, and only three of the six are what the label suggests.

| document | now | what it is |
|---|---|---|
| `OnTrac_StarCertificationProgram-3Day` | 7709/6233 | **raster ceiling.** Its pages 9 and 10 hold a `draw:object-ole` whose `ObjectReplacements` entry is an EMF. 26.2.4.2's PDF carries a 564 × 443 grey JPEG and 150 characters on page 10; ours carries no image and 1333 characters, because we replay the metafile as real text. **Ours is the better output and `wc -c` scores it as failure** — the `TODO.raster-ceiling.md` family, in ODF spelling, and the document is already filed in `slides/ceiling-001`. |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB` | 10377/9914 | the same on pages 5 and 6 (2 reference images each, ours none, +234 and +269), plus a separate −89/+55 on pages 15 and 16. |
| `Demick_JetBlue` | 2897/3179 | not the ceiling: short on pages 4, 6, 7 and 8 and long on 5, with no image split. Unclassified. |
| `vvsummit2022-Research-Roadmap…` | 8377/8377 | **glyph-exact after this round**; it now fails on `unembedded` alone. One `CFF ` face — Unifont — that `PdfFontCatalogue.IsCompactFontFormat` declines to embed, which `dotnet/CLAUDE.md` already records. |
| `Ramp Up Campaign - French` | 2414/2536 | −122 spread over four pages, no single seat found. |
| `NWD-GLA-Community-Outreach-Day-Oct-2025` | **match** | closed by `style:shrink-to-fit`: its pages 5, 6 and 12 were −334, −141 and −215 and are now exact. |

So the useful generalisation is that **"fails as `.pptx` too" is a good discriminator for *not an
ODF defect* and a bad one for *one cause*.** Two of the six were the raster ceiling, one was a
known PDF-writer limit, one closed with an ODF fix that the OOXML path had had for rounds, and two
are still open.

## What is left, classified

`remainder.tsv` carries one row per non-matching document with its glyph count at each of the four
measurement points.

| cause | rows |
|---|---:|
| raster ceiling — the reference draws a picture where we draw the object's text | 3 |
| ODF chart reader — axis labels and a secondary axis | 3 |
| overflow off the page bottom on a `draw:auto-grow-height="true"` body | 2 |
| something else in the ODF reader, no shared cause | 7 |
| autofit landing one `constScaleLevels` row from 26.2.4.2 | 1 |
| `CFF ` embedding | 1 |

The three chart rows are worth naming because they are one seat and it is not this round's:

- `combo_bar_line_chart.odp` — the reference draws a **secondary value axis** (a second 0–45000
  scale) and the **category labels Q1..Q4**; we draw neither.
- `scatter_chart.odp` — the reference draws the x axis' **numeric labels 2..10** and the four
  **data-point labels A..D**; we draw neither.
- `038_Competitive_Advantage_Card…odp` — the other way round: **we** draw a radar chart's rotated
  category labels (`Product Quality`, `Cost Efficiency`, …) that the reference does not, +68 on
  each of two pages.

`draw:auto-grow-height` is read nowhere in `dotnet/src`. The two `schematicplay` twins are the
witness — one page each, −348 each, on a `draw:custom-shape` stating
`draw:auto-grow-height="true" style:shrink-to-fit="false"` whose four bullets run past the sheet
in our layout and fit in 26.2.4.2's.

## Verification

- `dotnet build XRay.Office.slnx -v q -nologo` → **0 warnings, 0 errors**.
- Ten non-fidelity projects, run individually and totalled by hand: **5998 passed, 0 failed,
  0 skipped** — the 5985 baseline plus this round's 13 new tests (8 running objects, 3
  shrink-to-fit, 1 ODF blur namespace, and 1 from turning an existing shadow `Fact` into a
  two-case `Theory`). No failure appeared on any project, so nothing needed re-running.
- `XRay.Office.Fidelity.Tests`: **542 passed, 10 failed, 0 skipped** of 552 — the same ten names as
  the baseline: `PageDrawingComparisonTests` ×4, `TabStopComparisonTests` ×4,
  `SheetDrawingComparisonTests`, `JustificationShrinkComparisonTests`.
- All three rules are pinned by tests that fail without them, checked by mutating the source,
  `touch`ing it and removing `obj`/`bin` so MSBuild could not skip the project: forcing
  `AutoFit = false` fails 3 of 11; skipping every `presentation:class` frame on a master fails 6
  of 11; making the three declaration fields and the page number resolve to nothing fails 4 of 11.
  The sources were then restored with `cp` and `touch`, `diff`ed against the copies, and the
  rebuilt binary re-rendered `ws_prod-…-M.017` at **33915 characters**, byte-for-byte the sweep's
  own figure.
