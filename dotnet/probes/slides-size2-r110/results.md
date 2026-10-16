# slides-size2-r110 — an empty paragraph's line is floored at its bullet's box, and the residual is not one question

Round 110, slides track, one seat: **O15**, the per-page dominant drawn text size over the
51-document binary PowerPoint column.

Three results.

1. **A fifth input to the autofit block height, found by single-variable experiment against
   26.2.4.2, landed and tested.** An empty paragraph's line is raised to the height of the box
   its bullet would be drawn in, *after* the line-spacing arms — so a proportional line spacing
   cannot shrink it past the box and the box does not scale with the percentage. The box exists
   even where no bullet is drawn, and it exists for `.ppt` and ODF and **not** for OOXML, because
   the three importers suppress an empty paragraph's bullet in different places.
2. **The reach is small, one-sided and completely confined.** Over the whole slide corpus at both
   legs — 51 `.ppt`, 251 `.pptx`, 302 `.odp`, 604 documents, 0 failures — **26 renderings change
   and 578 are byte-identical**; **0 page counts and 0 alphanumeric counts move anywhere**; and
   **0 of the 251 `.pptx`** move at all, which is the reader split measured on the corpus rather
   than argued.
3. **The register is 2 fixed and 2 newly wrong, and the two newly wrong are one document with a
   picture bullet this tree does not read.** 11 → 11 pages, 55.10 → 54.10 pt of summed error,
   9 → 7 documents. All four pages that move are in two documents. The four-leg instrument says
   the two regressions are an **import** defect being amplified by a rule that is itself right:
   handed 26.2.4.2's own ODF of that deck, this tree at head answers the reference's size on
   three of its four O15 pages.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidesize2`, `agent/slidesize2`, base **`a311b00e2`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, the 51 `.ppt` among its 948 PDFs |
| corpora | `/home/user/sample-files` (51 `.ppt`, 251 `.pptx`) and `/home/user/corpus-odf` (302 `.odp`) |
| working directory | `/home/user/r110-work` |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is **not** the reference binary's source |

**Both legs, as this track requires.** Every `file:line` below is the `27.2.0.0.alpha0+` checkout
and is the *explanation*. The evidence is 26.2.4.2's own output: its flat ODP of a corpus deck,
edited in one place at a time and rendered back through the same binary; its `.pptx` of the same
file, likewise; and its banked renderings of the corpus. `variants.txt` is that record in full.

---

# 1. What the brief's split turned out to be

The brief carried round 103's division of the residual — pages one row apart with identical
alphanumeric counts against pages whose counts differ. Round 108 had already refuted it and
replaced it with a four-leg instrument (the reference reading the `.ppt`, the reference reading
its own flat ODP of that `.ppt`, this tree reading the `.ppt`, this tree reading that flat ODP),
which divides the residual by *where* the answer is decided. This round worked the three
**LAYOUT** pages that instrument named, and they are not one thing either:

| page | round 108 called it | what it is |
|---|---|---|
| `JesuitAssocOfStudentPersonnel` 24 | LAYOUT | the bullet box floor. **Fixed.** |
| `ws_prod-…-M.017-(French)-France` 14 | LAYOUT | a picture bullet the `.ppt` reader does not read. Not fixed. |
| `RRM-training-syllabus-…-Dec-2009` 16 | LAYOUT | the reference's lines are *wider* and wrap earlier. Not fixed. |

Only the first was a block-height question at all.

## 1.1 The instrument, and why the shrink was turned off first

The registered statistic is a `constScaleLevels` row, which is quantised: a page is 13.01 or
15.99 and nothing between, so a wrong block height of 1 unit and one of 400 look the same. The
first move was therefore to cut `draw:page` 24 out of 26.2.4.2's own flat ODP into a one-slide
file — which reproduces the disagreement exactly, `ref 13.01/150` against `ours 15.99/150` — and
then set `style:shrink-to-fit="false"`. Both sides then draw 20.01 pt and the comparison becomes
a **continuous baseline pitch**, which is what every table in `variants.txt` is measured in.

```sh
soffice --convert-to fodp JesuitAssocOfStudentPersonnel.ppt        # 26.2.4.2
# cut draw:page 24 into base.fodp; sed shrink-to-fit true -> false
probes/slides-size2-r110/render2.sh /abs/v5-nofit.fodp v5-nofit 1  # both halves
python3 probes/slides-r108/spans.py out-v5-nofit/ref/v5-nofit.pdf 1
```

# 2. The rule

**Fixed in this tree.**

## 2.1 What the experiment says

With the shrink off, the reference and this tree agree on the **first** baseline of the body
exactly, and on the gap that spans one of the page's empty paragraphs exactly — and disagree by
3.11 pt on each of the four gaps that span one of the others.

```
ref  243.61  295.11  343.50  395.01  446.51  502.10
ours 243.61  292.00  340.38  388.77  437.16  489.63
```

Six edits separate the cause from everything it is confounded with (§2–§6 of `variants.txt`):

* Swapping the paragraph **style** between a floored and an unfloored empty paragraph moves
  nothing on either side. Putting one character in the paragraph makes the two sides agree on
  every baseline of the page. So it is emptiness, not the margins.
* Sweeping `fo:line-height` over 100, 95, 90, 85, 80, 70, 60 and 50 %, the reference's empty line
  stands at a **constant 787 hundredths of a millimetre from 90 % downwards** and follows the
  text line above it at 95 and 100. That is a floor, not a scaling.
* Moving the same empty paragraph between five structures at 80 %: second `<text:p>` of a
  `<text:list-item>` — floored; its own `<text:list-item>` — floored; a `<text:list-header>` of a
  list whose level 1 is a `text:list-level-style-bullet` — floored; a `<text:list-header>` of one
  whose level 1 is a `text:list-level-style-number` with an empty `style:num-format` — **not**;
  a bare `<text:p>` outside every list — **not**. The discriminator is the level's bullet.
* Halving the level's `fo:font-size` removes the floor entirely; doubling it raises the empty line
  to 1576; naming DejaVu Sans instead of Arial moves it 787 → 821, which is that face's
  `(1901 + 483) / 2048` against Liberation Sans' `(1854 + 434) / 2048`. So the box is the
  **bullet font's own ascent plus descent**, at the bullet's own size.
* Over em sizes 10, 20, 28 and 40 pt the difference is 1.56, 3.11, 4.34 and 6.27 pt against the
  `(2288/2048 − 1.2 × 0.8) em` those four predict — 1.57, 3.14, 4.40, 6.29.

**No bullet is drawn on any of the floored paragraphs**, on either side.

## 2.2 The mechanism

`ImpEditEngine::createLinesForEmptyParagraph` (`editeng/source/editeng/impedit3.cxx`:514-524)
sends a paragraph with no characters down a path of its own, `CreateAndInsertEmptyLine`
(`:1845-1996`), which carries its own copy of the line-spacing arms and then ends:

```cpp
if ( !bLineBreak )
{
    tools::Long nMinHeight = aBulletArea.GetHeight();
    if ( nMinHeight > static_cast<tools::Long>(pTmpLine->GetHeight()) )
    {
        tools::Long nDiff = nMinHeight - static_cast<tools::Long>(pTmpLine->GetHeight());
        // distribute nDiff upwards and downwards
        pTmpLine->SetMaxAscent( pTmpLine->GetMaxAscent() + nDiff/2 );
        pTmpLine->SetHeight( nMinHeight );
    }
}
```

`bLineBreak` is `GetNode()->Len() > 0`, so the line appended after a paragraph's trailing hard
break is excluded and a paragraph holding text is never floored by its own bullet. The block is
the **last** thing done to the line, after `Min`, `Fix` and `Prop`, which is why the floor does
not scale with the percentage. `aBulletArea` is `OutlinerEditEng::GetBulletArea`
(`editeng/source/outliner/outleeng.cxx`:61-70) → `Outliner::ImpCalcBulletArea` →
`Outliner::ImplGetBulletSize` (`outliner.cxx`:1315-1355), which returns `Size(0,0)` for
`SVX_NUM_NUMBER_NONE`, the graphic's size for `SVX_NUM_BITMAP`, and otherwise sets
`ImpCalcBulletFont`'s font on the reference device and reads `GetTextHeight()` back — the
device's `mnLineHeight`, ascent plus descent with no leading.

Two properties of that chain decide the rest:

* **`Outliner::GetNumberFormat` asks the paragraph's depth and the numbering rule and never
  `EE_PARA_BULLETSTATE`** (`outliner.cxx`:1289-1312). Suppressing the glyph does not remove the
  box. That is why an empty paragraph with no bullet drawn is still floored.
* **The box is cached on the paragraph and the cache is filled by the fit's unscaled pass** —
  the property `SlideTextLayout.BulletBoxHeight` already carries, measured in round 100 over
  sixteen fitted boxes. The floor uses the same unscaled box, and the autofitted witness lands on
  the reference's row with it.

## 2.3 The reader split, which is a third of the change

Every importer suppresses an empty paragraph's bullet and **they do not do it in the same place**:

| reader | what it clears | is there still a box? |
|---|---|---|
| OOXML (`oox/source/drawingml/textparagraph.cxx`:192-196) | `NumberingLevel` = **−1** | no — `GetNumberFormat` returns null below zero |
| `.ppt` (`filter/source/msfilter/svdfppt.cxx`:2363-2366) | `EE_PARA_BULLETSTATE` only; the depth was set at `:2309` | **yes** |
| ODF | nothing; the level is the list nesting | **yes** |

Measured rather than read: the authored fixture converted to `.pptx` by 26.2.4.2 and rendered
back by it answers a pitch of **38.43 pt on slides 1, 3, 4 and 6** — the floor gone from every one
of them, the 200 % bullet included — where the ODF original answers 41.53, 38.41, 63.98 and 42.49
(`variants.txt` §7). The corpus agrees: **0 of 251 `.pptx` renderings move**.

`SlideParagraph.EmptyKeepsMarkerLevel` is that split, set by `PptTextBody` and `OdfTextBody` and
left false by `PptxTextBody`.

## 2.4 The change

Three source files.

* `SlideTextLayout.BulletFloored` — the floor, applied to `Appended`'s answer rather than inside
  it, because it is outside the four line-spacing arms in the reference too. It reuses
  `BulletBoxHeight` for the box and `Shaped(…, requireText: false)` for the bullet's resolved
  face; the only change to `Shaped` is that one flag.
* `SlideParagraph.EmptyKeepsMarkerLevel` — the reader split above.
* `PptTextBody` and `OdfTextBody` set it.

## 2.5 The test and the fixture

`tests/corpus/features/odp-empty-bullet-line.fodp`, hand-authored: six slides differing in one
thing each — the bullet present or not, the paragraph empty or holding a character, the bullet at
100 % or 200 %, the line spacing at 80 % or 100 %, the bullet's face Liberation Sans or DejaVu
Sans. Every number in `SlideEmptyBulletLineTests` is 26.2.4.2's own, read off its PDF of that
file, and all six slides agree to **0.00 pt** at head.

**Measured at the base rather than asserted**: with the source change reverted and the test left
in place, **3 of the 7 fail and 4 pass** — the four that pass are the two structural controls, the
line-spacing control, and the OOXML reader control, all of which are the same either way. That is
exactly the asymmetry the theory is built on.

One trap the fixture cost a first cut of: without `office:font-face-decls`, `style:font-name`
resolves to nothing and 26.2.4.2 draws the bullet from its own default face, which moved slides 1
and 4 by 34 and 68 hundredths of a millimetre. The decls are in the file and the header says why.

---

# 3. Reach — measured, over the whole slide corpus, base against head

`confine.py` renders a list with one CLI and records pages (from the real page tree, via
PyMuPDF — **not** `slides-r99/tfy.py`, which miscounts), alphanumeric characters and the PDF's
md5, one directory per *document*, deleting each render as it goes. Both binaries were built
before any sweep started and neither tree was rebuilt while one was running.

```sh
probes/slides-size2-r110/confine.py <cli> /home/user/sample-files  ppt.list  out.tsv 3
probes/slides-size2-r110/confine.py <cli> /home/user/corpus-odf    odp.list  out.tsv 3
```

| column | documents | rendered both legs | **renderings that move** | page counts differing | alphanumeric counts differing |
|---|---:|---:|---:|---:|---:|
| `.ppt` | 51 | 51 | **14** | 0 | 0 |
| `.pptx` | 251 | 251 | **0** | 0 | 0 |
| `.odp` | 302 | 302 | **12** | 0 | 0 |
| **total** | **604** | **604** | **26** | **0** | **0** |

A count of markup would have said something else: an empty paragraph at a bulleted level is
common. What moves a *rendering* is only the case where the box is taller than the line the
paragraph's own spacing gives it, which needs a proportional line spacing below about 95 % as
well.

## 3.1 Ink, over the renderings that move

`inksweep.sh` (the `.ppt` half, against the banked 26.2.4.2 renderings) and `inkodp.sh` (the
`.odp` half, against references produced for the 12 by 26.2.4.2 in the same run) score **every**
differing page rather than only the MAJOR ones — `pdf-image-diff.py --quiet` prints only MAJOR
rows and summing that output is a narrower statistic that is not comparable to this one.

| | base | head |
|---|---:|---:|
| `.ppt`, 14 moved: sum \|ink\|% | 69.51 | **69.40** |
| `.ppt` MAJOR pages | 9 | **9** |
| `.odp`, 12 moved: sum \|ink\|% | 109.03 | **108.37** |
| `.odp` MAJOR pages | 111 | **111** |
| improve / worsen / level | — | **14 / 5 / 7** |

`JesuitAssocOfStudentPersonnel` is the largest single movement in both columns — 6.61 → 6.04 as
a `.ppt` and 6.00 → 5.42 as an `.odp`. The three that worsen on the `.ppt` side are 0.01, 0.42
and 0.68, and the 0.68 is `ws_prod-…-France`, §4.1.

## 3.2 The registered statistic

`sizesweep.py` over the 51 `.ppt`, scored with `slides-r107/sizescore.py` against the banked
reference table (`size-summary.txt`).

| | base = `a311b00e2` | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **11** | **11** |
| documents holding one | 9 | **7** |
| total \|size error\| over 1534 pages | **55.10** | **54.10** |
| of the differing pages, same alphanumeric count | 6 | 5 |
| **fixed / newly wrong** | — | **2 / 2** |
| pages whose dominant size moved at all | — | **4 of 1534** |

Fixed: `JesuitAssocOfStudentPersonnel` 24 (15.99 → 13.01, the reference's) and
`ws_prod-g-doc-Events-Part-M-presentation` 21 (18.0 → 17.01, the reference's — one of round 108's
three ODP-NOT-FAITHFUL pages, which turns out to be this rule and not the round trip's opacity).
Newly wrong: `ws_prod-…-M.017-(French)-France` 8 and 15, both §4.1.

The base column was **measured on the base binary this round**, not carried over: it reproduces
round 108's 11 / 55.10 exactly.

---

# 4. The eleven that remain, page by page

| document | page | ref | ours | what it is |
|---|---:|---:|---:|---|
| `ws_prod-…-M.017-(French)-France` | 8 | 15.99 | 15.00 | picture bullet (§4.1) |
| `ws_prod-…-M.017-(French)-France` | 14 | 18.99 | 20.01 | picture bullet (§4.1) |
| `ws_prod-…-M.017-(French)-France` | 15 | 18.99 | 17.01 | picture bullet (§4.1) |
| `ws_prod-…-M.017-(French)-France` | 16 | 14.99 | 17.01 | picture bullet (§4.1) |
| `RRM-training-syllabus-…-Dec-2009` | 16 | 18.99 | 20.01 | the reference's lines are wider (§4.2) |
| `2015-Civil-Rights-Website-training` | 22 | 14.0 | 17.01 | ODP-NOT-FAITHFUL (§4.3) |
| `gfopportunitiesforlinkagespres_2010_en` | 27 | 25.99 | 28.01 | ODP-NOT-FAITHFUL (§4.3) |
| `Thailand17` | 8 | 24.01 | 16.68 | text presence, 92 alnum against 500 (§4.4) |
| `W3_Case_Study_…_Ed` | 10 | 24.01 | 16.68 | the same slide in another deck (§4.4) |
| `Fundamentals_Module_1_basics` | 6 | 32.0 | 7.08 | text presence, 44 against 87 (§4.4) |
| `Thailand17` | 11 | 11.99 | 11.0 | text presence the other way, 548 against 358 (§4.4) |

## 4.1 A picture bullet — four pages, one document, and it is an import defect

`nBuBlip != 0xffff` with a graphic behind it makes the level `SVX_NUM_BITMAP`
(`svdfppt.cxx`:3448-3465), whose label is a picture and whose box is `GetGraphicSize()`. This
tree does not read it: it falls back to the level's bullet **character**, which on this deck is a
Wingdings 2 slot recoding to a Private Use code point no installed face holds — drawn at
**31.011 pt beside 20.013 pt text**, with the text indented behind it. 26.2.4.2 draws no text
bullet at all on those pages and starts its text at the stated label width.

Three measurements say this is the cause and that it is an *import* defect:

* 26.2.4.2's own flat ODP of the deck gives `text:list-level-style-image` on `Default-outline1`
  and `Default-outline2` and on `L6`, `L8`, `L9`, `L11`, `L15` and `L16`; pages 8, 14, 15 and 16
  are exactly the pages that use one.
* Handed that ODP, **this tree at head answers 15.99, 20.01, 18.99 and 15.0** against the
  reference's 15.99, 18.99, 18.99 and 14.99 — three of the four right, where reading the `.ppt`
  it is 15.0, 20.01, 17.01 and 17.01 and none of the four is.
* Pages 8 and 15 are the two the floor moves. The floor is being computed from a fictitious
  31 pt character's box, so a rule that is right in general is wrong on a paragraph whose label
  this tree has the wrong *kind* for.

**Gating the floor on the paragraph's own `BulletBlip` was tried and is inert**: this deck's blip
is inherited from the master's `aExtParaSheet[instance].aExtParaLevel[level]`
(`svdfppt.cxx`:3425-3446), which nothing in `PptTextReader` reads — it reads the shape's own
`ExtendedParagraphAtom` only. The guard was measured against all four pages, changed none of
them, and was removed rather than left in as dead code that tells a story it does not deliver.
The comment in `PptTextBody` records that.

**This is a seat of its own** — reading a `.ppt` picture bullet: the master's extended paragraph
sheet, the BLIP behind it, a graphic label with a graphic's box
(`height = round(fontHeight × 0.2540 × bulletHeight)`, `svdfppt.cxx`:3455) — and it owns four of
O15's eleven pages.

## 4.2 `RRM-training-syllabus-…` 16 — a width question, not a height one

With the shrink off, both sides draw 20.01 pt and the first baseline agrees; what differs is
where the lines break. The reference fits *less* on a line: it wraps `M r. Warren M aines – GM
Servisair YVR` before `Findings` where this tree does not, and `…occurred during wet` before
`conditions` where this tree does not. `pymupdf` reports the reference's text as `M r.` and
`M ishap` — a positional gap after each capital M — which says the reference is drawing those
stretches as separate portions and accumulating each portion's rounding. Not measured further,
and not this seat: the block arithmetic over the lines is not in question, the lines' *widths*
are.

## 4.3 Two pages the instrument cannot resolve — terminal until something else changes

`2015-Civil-Rights-Website-training` 22 and `gfopportunitiesforlinkagespres_2010_en` 27 are
round 108's ODP-NOT-FAITHFUL pages that survive this round: 26.2.4.2 reading its own flat ODP of
the file answers something different from 26.2.4.2 reading the file, so the four-leg instrument
is blind there and the ODF single-variable method this round used cannot be run on them at all.
Round 108's third such page, `ws_prod-g-doc-Events-Part-M-presentation` 21, is **fixed** by this
round's rule, so the class is not a diagnosis — it is a statement about the instrument.

## 4.4 Four pages that are not size questions

`Thailand17` 8 and `W3_Case_Study…` 10 (the same slide in two decks) draw **500 alphanumerics
against the reference's 92**; `Fundamentals_Module_1_basics` 6 draws 87 against 44; `Thailand17`
11 draws 358 against 548. Round 108 said these should leave the O15 statistic and become their
own seat, and nothing this round found changes that: the dominant *size* follows from drawing the
wrong text, and no block-height rule can reach them.

---

# 5. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then each project alone with
`--no-build`, individually rather than as a solution.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 560 / 560 |
| Markup | 259 / 259 |
| OpenDocument | 160 / 160 |
| Rendering | 164 / 164 |
| Spreadsheets | 1333 / 1333 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1938 / 1938 |
| **Presentations** | **1114 / 1114** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else, confirmed by name in a second run of
those four classes: `PageDrawingComparisonTests` ×4 (`paginated.doc/.docx/.fodt/.rtf`),
`TabStopComparisonTests` ×4 (`list-label-overrun.doc/.docx/.fodt/.odt`),
`SheetDrawingComparisonTests` ×1 (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
×1 (`justify-shrink-2013.docx`). **No eleventh.** Every project reported its full total and none
was aborted. Presentations' 1114 is 1107 plus the seven new `SlideEmptyBulletLineTests`;
Spreadsheets' 1333 is round 108's 1315 plus other seats' merges into the base.

---

# 6. What could not be settled, and one procedural note

* **The `.ppt` picture bullet** (§4.1). Named, measured, and worth four of the eleven. Not
  implemented: it needs the master's extended paragraph sheet, the BLIP store behind it, and a
  graphic label with a graphic's box, none of which this reader has.
* **`RRM-training-syllabus-…` 16** (§4.2) is a portion-width question and was not pursued.
* **The two ODP-NOT-FAITHFUL pages** (§4.3) carry no evidence through this instrument.
* **The four text-presence pages** (§4.4) are not size questions and should leave this statistic.
* **The blind reading the brief asked for could not be delegated.** `page-vision`'s own warning
  applies: this container has no subagent that can open a local PNG, and
  `mcp__Claude_Code_Remote__create_session` runs elsewhere and cannot see one. Checked once and
  not pursued further. Every reading here is therefore the seat's own and contaminated, and each
  is corroborated by arithmetic that does not depend on it — PDF content-stream operators
  (`spans.py`), the reference's own flat ODP and `.pptx` of the same file, and the banked
  renderings. The one page looked at as an image, `JesuitAssocOfStudentPersonnel` 24 at 110 dpi
  with the reference above and this tree below, is indistinguishable half from half, which is
  a check on the fix rather than evidence for the rule.
* **The source leg is a later version.** Every `file:line` above is `/home/user/libreoffice-core`
  at `27.2.0.0.alpha0+`. The measurement legs are 26.2.4.2's own output throughout.

---

# 7. The files

| | |
|---|---|
| `variants.txt` | the single-variable series in full: the line-height sweep, the five structures, the bullet size and face, the em sweep, the OOXML control, the fixture's verification |
| `render2.sh` | renders one flat ODP through 26.2.4.2 and through this tree's CLI and prints both dominant sizes |
| `confine.py`, `confine.txt` | the whole-corpus confine: pages, alphanumerics and md5 for 604 documents at both legs, and the scored table |
| `base-ppt.tsv` / `head-ppt.tsv`, `base-pptx.tsv` / `head-pptx.tsv`, `base-odp.tsv` / `head-odp.tsv` | its six columns |
| `sizesweep.py`, `base-sizes.tsv`, `head-sizes.tsv`, `size-summary.txt` | O15's statistic at both legs, scored against `slides-r107/sizes-ref.tsv` |
| `inksweep.sh`, `ink-ppt.txt`, `inkodp.sh`, `ink-odp.txt`, `moved-ppt.list`, `moved-odp.list` | ink over every differing page of the 26 renderings that move |
| `ppt.list`, `pptx.list`, `odp.list` | the three columns |
| `tests.log` | the suite |
