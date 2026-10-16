# `words-continuous-r72` — a continuous section has no page of its own, so it has no page descriptor

## Environment

    ours   = XRay.Office.Cli @ 6ab0681b9 (base) and @ HEAD of agent/words69, Debug
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball),
             all four font confounds aside — 38 faces in .duplicates-aside, 8 in .noto-aside,
             no LiberationSansNarrow anywhere, fc-match "DejaVu Sans" -> DejaVuSans.ttf
    gate   = /home/user/gate-2f47 (reference half rendered by 24.2.7.2) for the words track,
             /home/user/gate-odf-rows.tsv (26.2.4.2) for the .rtf column
    rule   = batch-check.sh of 2026-09-05: pages, then alphanumeric characters within
             max(2%, 15), then unembedded fonts

The reference halves of both gates were reused rather than re-rendered: the whole diff is
confined to `dotnet/src` and cannot reach `soffice`. The 26.2.4.2 renderings the mutation and
ink measurements use were made fresh, one `soffice` profile per document keyed on an md5 of
the document's own path.

---

## The rule

**A continuous section break starts no page, so Writer builds it no page descriptor — and
paper size, margins, header, footer, numbering format and number offset all live on one. All
six are inert together, and the section wears the descriptor still in force.**

`SectionPropertyMap::CloseSectionGroup`'s continuous branch
(`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1722-1852) inserts a Writer *text* section
and calls `InheritOrFinalizePageStyles`, which hands the section the previous one's page style
outright whenever it has not created one of its own (`:1309-1323`). It creates one only
because `DomainMapper_Impl::PushPageHeaderFooter` called `GetPageStyle` while importing a
header or footer the section named (`DomainMapper_Impl.cxx`:4169). Attaching that style to a
page is the `else if` at `:1746`, guarded on some slot **not** being linked to the previous
section: it walks the section's own paragraphs for the first carrying `BreakType_PAGE_BEFORE`
and sets `PageDescName` — and `PageNumberOffset` — on that one (`:1794-1801`), failing that on
the previous section's last paragraph if it carries one. With no such paragraph the style
stays in the document and on no page.

`wwSectionManager::InsertSegments` says the same in its own words —
*"In this nightmare scenario the continuous section has its own headers and footers so we will
try and find a hard page break between here and the end of the section and put the headers and
footers there"* — and caches the descriptor so it can put the old one back when the search
fails: `if (bFailed) aIter->mpPage = pOrig` (`sw/source/filter/ww8/ww8par.cxx`:4515-4560).
RTF reaches the dmapper rule, since writerfilter serves both.

`ContinuousPageDescriptors` is that rule; `PaginatedSection.StatesOwnFurniture` is the guard.

### The one place the two readers differ

**A `.doc`'s continuous section keeps its own side margins and a DOCX's does not.**
`wwSectionManager::InsertSection` puts `section.left − page.left` and its right-hand twin on
the *text section* as an `SvxLRSpaceItem` (`ww8par6.cxx`:758-770), unconditionally, for every
continuous section that becomes one — so they are not on the descriptor and do not fall with
it. writerfilter's `ApplySectionProperties` sets a writing mode and an endnote flag and
nothing else (`dmapper/PropertyMap.cxx`:792-812). Both halves are measured below.
`Ww8SectionTable.ResolveContinuousBreaks` already carried the sheet, the bands and the
vertical margins forward with the same exception and cites the same line;
`PaginatedSection.SideMarginsAreASectionIndent` is that exception where the descriptor rule
can see it.

---

## The measurements

### 1. Seven one-attribute mutations of the corpus witness (`mutate.py`, `measure.py`)

`hdss-bulletin-issue-285-25-june-2025.docx`: section 1 states `w:top="454" w:header="340"` and
names `header1.xml` (one empty `Header`-styled paragraph); section 2 is `continuous`, names
`header2.xml`, states `w:top="1418"` and carries `w:titlePg`; section 3 is `continuous`, names
nothing, and contains the document's single `<w:br w:type="page"/>`.

*top* is the topmost body word's `yMin` on that page, running-head line excluded.

| variant | ref26 pages | head on | ref26 p1/p2/p3 top | ours before | ours after |
|---|---:|---|---|---|---|
| *(unchanged)* | 10 | none | 154.13 / 29.60 / 27.83 | 154.13 / **73.15** / **71.47**, head on 2–10 | **exact** |
| `s1-nohdr` | 10 | none | 149.48 / 23.90 / 23.18 | 149.48 / 72.10 / 71.38 | **exact** |
| `s1-top1000` | 10 | none | 176.78 / 52.25 / 50.48 | 176.78 / 73.15 / 71.47 | **exact** |
| `s2-nohdr` | 10 | none | 154.13 / 29.60 / 27.83 | 154.13 / 73.15 / 71.47 | **exact** |
| `s2-top3000` | 10 | none | 154.13 / 29.60 / 27.83 | 154.13 / 73.15 / 71.47 | **exact** |
| `s3-top3000` | 10 | none | 154.13 / 29.60 / 27.83 | **11 pages**, 152.25 / 154.85 | **exact** |
| `s2-pagebreak` | 11 | 1–11 | 154.13 / 99.60 / 73.15 | already exact | **exact** |

Three of those are load-bearing on their own.

- **`s2-top3000` moves the reference not at all.** A continuous section's `w:pgMar` is inert
  when its descriptor lands nowhere, so the geometry travels with the header rather than
  separately.
- **`s3-top3000` moves it not at all either**, and section 3 *does* contain a hard page break.
  That is the `!m_b*LinkToPrevious` guard measured: a section naming no furniture is never
  offered a descriptor, so a break inside it has nothing to hang.
- **`s2-pagebreak` makes the reference apply the header *and* the 1418-twip top margin from
  that page on**, and we already agreed with it page for page. That is the other half: when a
  descriptor lands, both halves land together.

### 2. What explains the 39.1 pt

The previous round's sharpest constraint was that the reference's page-2 body sits 39.1 pt
down an A4 sheet, which is neither section's `w:top` (22.7 pt and 70.9 pt). **It is not a
margin.**

Every page keeps section 1's descriptor. `PrepareHeaderFooterProperties`
(`PropertyMap.cxx`:1148-1195) makes the Writer top margin the section's `w:header`, the
header's declared height `w:top − w:header`, and the header–body distance that less 1 mm:

    top margin        = 340 tw                     = 17.000 pt
    declared height   = (454 − 340) tw             =  5.700 pt
    body distance     = 5.700 − 1 mm               =  2.865 pt

`header1.xml` holds one empty paragraph in the `Header` style — `w:sz="18"`, Arial, resolving
to Liberation Sans, single spaced: `(1854 + 434 + 67)/2048 × 9` = **10.348 pt**, which is more
than the declared height. A DOCX header is imported dynamic-height with dynamic spacing, so it
eats the whole of the distance. The body starts at

    17.000 + 10.348 = 27.348 pt

and the first body line — `Body` style, `w:line="280" w:lineRule="atLeast"` = 14 pt against a
natural 12.07 — puts its baseline `14 − descent(2.225)` below the line top:

    27.348 + 11.775 = 39.123 pt   against the reference's measured 39.10

The residual is 0.02 pt. **Section 1's margin plus section 1's empty header, and no part of
section 2.**

### 3. The synthetic `ContinuousSectionGeometryTests` builds (`synthetic.py`)

Rendered through 26.2.4.2. `p*` is the topmost word's `xMin`.

| variant | ref26 pages | p1 | p2 | p3 | head drawn on |
|---|---:|---|---|---|---|
| `plain` — continuous section, no furniture, no break | 2 | 36.1 | 36.1 | — | — |
| `hdr` — names a default header, no break | 2 | 36.1 | 36.1 | — | **none** |
| `hdr-break` — names one and holds a `pageBreakBefore` | 3 | 36.1 | 72.1 | 72.1 | 2, 3 |

Ours matches on every page of all three to 0.1 pt, which is this environment's constant
horizontal offset.

**`plain` is what refuted the test.** `ContinuousSectionGeometryTests` asserted that page two
takes the continuous section's one-inch left margin; 26.2.4.2 leaves it on the first section's
36.1 pt. The corpus measurement the test was written from — `b050-19.docx`, page one's text
36 pt to 574 pt and pages two and three 72 pt to 539 pt — is real and is the *other* case:
that document's continuous section names a header **and** contains a hard page break, so its
descriptor does land, and it lands at the break rather than "at the next page". The synthetic
carried neither, so it read the right corpus behaviour off the wrong mechanism.

### 4. Reach (`census.py`)

**10 of the corpus's 272 DOCX** carry a continuous section that is not the document's first —
23 such sections whose descriptor lands nowhere, 4 that land at a hard break.

    Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.docx   10 dropped
    JEMIT_Template.docx                                                             5
    hdss-bulletin-issue-285-25-june-2025.docx                                       2
    Press release_EUREKA labels ITEA 3 Cluster.docx                                 2
    report-template.docx                                                            1
    template---tpr-technical-progress-report-with-guidance.docx                     1
    Regulations Governing the Status…Experts on Mission.docx                        1 + 1 landing
    ABCD-SDE-23-00 - Avionic System Description - 17.02.16 - v1.docx                1 + 1 landing
    b050-19.docx                                                                        1 landing
    b053-19.docx                                                                        1 landing

**The previous round's figure of 16 is an overcount of this rule's reach.** It counted
documents carrying a continuous `w:sectPr` at all, and seven of them — `t_TEMPforInvProgs`,
`easa-form-1`, `195584360`, `Allegiant_Company_Profile_with_History`,
`Pre-Application_Letter_of_Intent_Example`, `mde087077~283`, `231164_SystemDesignDocument` —
have it on the **first** section, where the break type says nothing at all
(`m_bIsFirstSection` takes the branch above it, `PropertyMap.cxx`:1738-1742).

### 5. The gate, both tracks, both directions

Our half re-rendered at the base commit and at HEAD; the reference halves are the banks'.

| track | base | after | verdicts moved |
|---|---:|---:|---|
| words, `/home/user/sample-files/words` vs `gate-2f47` | **315** / 338 | **315** / 338 | **none, either way** |
| `.rtf`, `/home/user/corpus-odf` vs `gate-odf-rows.tsv` | **215** / 328 | **216** / 328 | one, forwards |

The one `.rtf` movement is `644730BRI0mna000BOX361539B00public0.rtf`, `pages 5/4 → match 4/4`.
**It is the same document as the words track's one near-regression, and the pair is the
reader difference measuring itself**: as a `.doc` both 26.2.4.2 and we paginate it to 5 pages,
because its continuous section's side margins survive as a section indent; as LibreOffice's
own `.rtf` export of the same content both paginate it to 4, because there they do not.
Reading it either way for both would have cost one row on one track and gained none on the
other.

**8 of the words track's 338 renderings changed**, and no other document in the corpus moved
by a byte. `|ink|` is summed `|ink_ratio − 1|` over the document's pages against 26.2.4.2 at
100 dpi, through `compare-images.py`'s own metric:

| document | pages ref/ours | \|ink\| before | \|ink\| after |
|---|---|---:|---:|
| `hdss-bulletin-issue-285-25-june-2025.docx` | 10/10 | **3.173** | **0.397** |
| `Press release_EUREKA labels ITEA 3 Cluster.docx` | 2/2 | 0.049 | **0.001** |
| `150_5300_13_chg8.doc` | 17/18 | 1.928 | 1.926 |
| `ABCD-SDE-23-00 …docx` | 29/29 | 0.490 | 0.491 |
| `Regulations Governing the Status …docx` | 18/18 | 1.297 | 1.297 |
| `report-template.docx` | 20/20 | 1.258 | 1.259 |
| `150_5300_13_chg10.doc` | 78/77 | 163.318 | 162.938 |
| `150_5300_13_chg12.doc` | 31/31 | 1.882 | **2.241** |

`hdss` is the target and it moves by an order of magnitude; per page its ink ratio goes
3.062 → 1.227 on page 9, 1.340 → 0.995 on page 10, 1.231 → 0.995 on page 3, and five of its ten
pages go from a non-zero vertical shift to zero.

### 6. What it cost, measured and named

**One page of one document.** `150_5300_13_chg12.doc` page 13 drew a running head reading
*AC 150/5300-13 CHG 10 / 9/29/06*; it now draws none; 26.2.4.2 draws *CHG 12 / 1/3/08*. All
three disagree, and absent is worse than wrong by ink: that page's ratio goes 1.019 → 0.588
and the document's `|ink|` 1.882 → 2.241. **Every other page of it is unchanged to three
decimals**, its page count is unchanged at 31/31, and no gate verdict moves.

The seat is the inheritance *source*. `ContinuousPageDescriptors` takes the previous
**resolved** section's furniture, which is what makes the carry follow the descriptor chain
rather than the section chain — necessary, because §17.10.1 has a DOCX section inherit any
slot it does not name, so `hdss`'s third section would otherwise inherit the second's running
head, the very head whose descriptor has just been found to reach no page. Making the
replacement conditional on `StatesOwnFurniture` was tried and measured: it leaves page 13
exactly as it is and puts `header2` back on `hdss`'s last page. `wwSectionManager::InsertSection`
finds its page by walking **backwards to the last segment that has a descriptor**
(`ww8par.cxx`:743-746), which the chain reproduces in every case this corpus contains except
this one. It is one focused round's work and it is not this one's.

**Two changes were tried, measured to move nothing, and dropped rather than carried.**
Not restarting the descriptor's first page for an inherited section — principled, since the
first-page furniture slot and a `firstPage` page border are both descriptor properties — moves
**no byte of any of the eight documents that changed at all**, and does not fix page 13. It is
not in the tree.

---

## Item 2: the `.rtf` residual, classified

Round 71 left 34 rows carrying none of its four constructs. At HEAD there are **96** rows that
pass in their original spelling and fail as `.rtf`, and the construct census reproduces the
same residual exactly:

| construct present | of the 96 |
|---|---:|
| `{\listtext…\pard…}` | 51 (38 alone, 13 with another) |
| a positioned table (`\tpos*`) | 20 (8 alone) |
| a turned cell (`\cltxbtlr`/`\cltxtbrl`) | 9 (1 alone) |
| **none of the three** | **34** |

**The discriminator that classifies them is `xray-office extract` against our own rendering**,
which the gate columns cannot see: extraction and rendering are separate paths, and the
question is which of the two is short (`rtf-extract-vs-render.py`).

| class | rows | what it means |
|---|---:|---|
| `drawn-short-read-whole` | **15** | extraction reads the reference's glyph count — in 13 of the 15 **to the character** — and the rendering draws less. A drawing gap, not a reading one. |
| `text-agrees` | **9** | the text is right and the pagination is not. |
| `drawn-long` | **8** | we draw *more* text than the reference. |
| `read-short` | **2** | the only genuine reading gaps left. |

- **The 15 are one construct: a shape's own text, `{\shptxt}`.** Every one is a
  `chartset-*`/`drawingset-*` document. Worked example
  `043_Visual_Product_Roadmap_Template_Customizable_Format`: extraction gives **1099**
  characters and every word the reference draws; the rendering gives **41**, the two body
  paragraphs and nothing else. A three-line probe reproduces it (`.work/shp`): the same shape
  with `\shpleft1000…\shpright6000` draws its text in both renderers, and with the real file's
  `\shpleft9404\shptop1853\shpright15006\shpbottom5765\shpbxignore\shpbyignore` **26.2.4.2
  draws `SHAPE TEXT INSIDE THE BOX` and we draw `SHAPE TEXT`** — the box is placed against the
  column, runs off the right edge and is clipped. `\shpbxignore` defers the origin to the
  `posrelh` shape property, and `RTFSdrImport::resolve` maps **only** `posrelh == 1`
  (`sw/source/writerfilter/rtftok/rtfsdrimport.cxx`:696-717) — every other value, 3 included,
  leaves LibreOffice's own default, which is not our column fallback. That is the next seat on
  this column and it is one round's work with a 15-document reach.
- **The 9 are pagination with the text exact**, and the largest is the largest row in the
  column: `Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.rtf` at
  **169 pages against 148 with 189432 glyphs against 189432** — not one character missing.
  `II.1.2_Hypoxia` is 12/13 with identical glyphs and
  `prison-population-bulletin-june` 6/4 with identical glyphs. This is the same finding round
  71 reported for the three `mcar` documents — *the text is now read and the tables holding it
  do not paginate like Writer's* — arriving on a second, larger set.
- **Four of the 8 `drawn-long` are the raster ceiling in RTF form.**
  `clustered-column-result/template` and `pie-chart-result/template` carry `\pict` and no
  `\shp`: LibreOffice draws its own exported picture and its PDF holds no words, while we
  render the metafile and draw real searchable text. Ours is the better output and `wc` scores
  it as failure. `TODO.raster-ceiling.md` lists 37 such pages for the words track; these are
  the same class on the `.rtf` column and should be filed there rather than worked.
- **The 2 `read-short` are `1228841571067_2009_TPPT_13__2007_TPPT_102__R-108535759.rtf`**
  (13349 read against 14300) **and `fleetfastfacts16nov2023.rtf`** (635 against 733).

**Did item 1's rule reach the RTF documents?** Yes, and by exactly one row —
`644730BRI0mna000BOX361539B00public0.rtf`, 5 pages → 4 against the reference's 4 — with two
renderings changed of 338 and nothing moved backwards. The rule is real on that column and its
reach there is small, because LibreOffice's own RTF export writes `\sbknone` on very nearly
every section it emits (it appears in all 34 residual documents) while the *first* section is
where most of them put it, and a first section's break type says nothing.

---

## Reproducing

```sh
export TMPDIR=/home/user/wt-words69/.tmp
R=dotnet/probes/words-continuous-r72
python3 $R/mutate.py    /abs/variants && $R/render.sh /abs/variants /abs/vout
python3 $R/measure.py   /abs/vout
python3 $R/synthetic.py /abs/syn      && $R/render.sh /abs/syn      /abs/synout
python3 $R/census.py    /home/user/sample-files/words

XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
$R/sweep-ours.sh /home/user/sample-files words /home/user/gate-2f47 /abs/words-after 4
dotnet/probes/rtf-gate-r71/sweep-ours.sh /home/user/corpus-odf /abs/rtf-after 4
python3 dotnet/probes/rtf-gate-r71/score.py /abs/rtf-after/ours.tsv > /abs/rtf-rows.tsv

$R/ink.sh /abs/movers.txt /abs/ink
python3 $R/inkscore.py /abs/movers.txt /abs/ink/ref26 /abs/before/ours /abs/after/ours
python3 $R/rtf-classify.py /abs/rtf-rows.tsv /abs/ours-only.txt
python3 $R/rtf-extract-vs-render.py /abs/residual.txt /abs/after/ours /abs/rtf-rows.tsv
```
