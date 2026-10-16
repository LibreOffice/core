# odt-startx-r88 — an ODF `text:section` is a Writer text section, and its columns were never laid out

Measured 2026-09-08/09 in `/home/user/wt-odtstartx` (branch `agent/odtstartx`, base `a3700bba2`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2 0229ac93**, with all five
tarball font confounds moved aside (`.duplicates-aside` 38 faces, `.noto-aside` 8, `.condensed-aside`
8; `ls .../truetype | grep -icE 'carlito|caladea|liberation|dejavu|^Noto(Sans|Serif)-'` answers 1, and
that one is `DejaVuMathTeXGyre.ttf`, which duplicates nothing installed).

Our half of the `.odt` track was re-rendered at HEAD by `sweep-odt-ours.sh` against the banked
reference of `/home/user/gate-odf-r80`; the bank at `/home/user/odt-r87-after/ours` is this round's
base, confirmed by re-rendering one document and comparing it byte for byte with the date masked
(`movers.py`, 1 compared, 0 differ).

---

## 0. The brief's Group A does not exist, and the instrument is why

The brief ranked the residual on `probes/odt-page-r87/residual-startx.txt` and named four rows as
*"far larger than the nine it flagged"*. Three of the four are an artefact of the instrument and the
fourth is a real defect of a different kind. **Neither of the two corrections is about the tree.**

### 0.1 The columns were misread, and `011` is not page-exact

`residual-startx.txt` is `startx.py`'s own output and its columns are
`matched | mean |Δx| before | mean |Δx| after | within-0.1pt before/after | document`. The brief read
column 2 as a *maximum*, column 3 as the mean, and column 4 as *pages agreeing*. So *"341.96 pt max,
100 lines, 11/11 pages agreeing"* is really *100 matched lines, mean 341.96 pt, and 11 of them within
a tenth of a point*. `011_Project_Timeline_Template_Beautiful_Theme` is a **one-page** document, and
the gate fails it on **glyphs** — 993 against 885 — so it is neither page-exact nor invisible to the
gate; it is the `chartset` autofit-or-clip question `dotnet/CLAUDE.md` already names.

### 0.2 The 341.96 pt is difflib matching identical labels in draw order

That document draws the same nine-line `Lorem ipsum … congue` block nine times across a timeline. The
distinct start-x of those nine blocks are

```
ours  98.10 176.00 254.75 332.00 408.95 485.30 563.50 643.85 722.40
ref   98.20 176.10 254.85 332.10 409.05 485.40 563.60 643.95 722.50
```

— **the same nine positions on both sides, to 0.10 pt**. The two renderers emit them in different
orders, `difflib` pairs them positionally, and the mean of the resulting nine-way permutation is
341.96 pt. The two Venn templates are the same shape with three labels: 204.95 / 378.80 / 596.15
against 205.05 / 378.95 / 596.25.

### 0.3 A justified line is one text object per stretch here and one per line in the reference

The second correction is independent and costs as much. PyMuPDF's `get_text('dict')` reports **635
"lines" against 479** for one page of `644730BRI`, because this tree writes a justified line as one
text object per stretch; the matcher then pairs a fragment against a whole line.

`startx.py` in this directory is the corrected instrument: spans are merged on (page, baseline) and
the line's start is the leftmost of them, and it reports beside the mean the share of matched lines
whose text is **unique on both sides**, which is the column to read for a template. Re-measured at
HEAD, unchanged:

| document | brief's figure | corrected | matched | unique |
|---|---:|---:|---:|---:|
| `011_Project_Timeline_Template_Beautiful_Theme` | 341.96 pt | **0.142 pt** | 54 | 14 |
| `040_Venn_Diagram_Template_Three_Circle` | 108.73 pt | **0.114 pt** | 13 | 1 |
| `031_Venn_Diagram_Template_Blue_Theme` | 108.71 pt | **0.111 pt** | 13 | 1 |

**There is no horizontal-placement defect on any of the three.** Read a `|Δx|` ranking with the
matched-line count *and* the unique-line count beside it — `probes/odt-page-r87` said the first half
of that and this is the other half.

---

## 1. What the fourth row was, and why it is also the head of Group B

`644730BRI0mna000BOX361539B00public0.odt` survives both corrections: 65.54 pt over 21 matched lines.
Its x histogram says what it is at a glance.

```
ref    203 lines at x=72   197 at x=320      (two columns)
ours   157 lines at x=30    14 at x=37       (one column, and the wrong margin)
```

The document is a World Bank newsletter whose body is one `text:section`:

```xml
<style:style style:name="Sect1" style:family="section"><style:section-properties
    text:dont-balance-text-columns="true" fo:margin-left="0.5835in" fo:margin-right="0.6835in">
  <style:columns fo:column-count="2" fo:column-gap="0.5in"> … </style:columns>
</style:section-properties></style:style>
```

Its page layout states `fo:margin-left="0.4165in"`, and **0.4165 + 0.5835 = 1 inch exactly** — the
margin its `.doc` original declares for that section. So the section's margins are a *difference*
from the page's, and 42.1 pt is the whole of the horizontal error; 290.5 pt is the second column we
did not have.

**Established at the reference before anything was written.** `variants.py` renders the document
through 26.2.4.2 with the section's columns removed, with its margins removed, and with both removed:

| 26.2.4.2 renders | x histogram | ours vs it |
|---|---|---:|
| the file as it stands | 203 @ 72, 197 @ 320 | **90.42 pt** over 19 matched lines |
| `style:columns` removed | 181 @ 72, 7 @ 30 | 32.52 pt |
| `fo:margin-*` removed | 183 @ 30, 145 @ 324 | — |
| both removed | **160 @ 30, 14 @ 38** | **0.201 pt** over 77 matched lines, 185 lines against our 180 |

With the section's two attributes gone, 26.2.4.2 draws what this tree drew. **The section is the
whole of the divergence.**

### The same object is the top of the brief's Group B

Censused over the 338 converted `.odt` (`census-sections.py`, `census-columns.py`): **139
`text:section`, of which 63 in 33 documents state more than one column** and 10 in 5 state a margin.
The documents at the head of that census are the brief's Group B:

| columned sections | document | Group |
|---:|---|---|
| 15 | `150_5300_13_chg10` | B |
| 9 | `150_5300_13_chg12` | B |
| 7 | `150_5300_13_chg8` | B |
| 3 | `JEMIT_Template` | B |
| 1 | `ABCD-SDE-23-00 - Avionic System Description` | B |
| 1 | `644730BRI0mna000BOX361539B00public0` | A |

and their reference renderings all carry a second column this tree did not draw — 312 for `JEMIT`,
342/306 for `chg10`, 319 for `ABCD-SDE-23-00`. The four Group B rows that hold no `text:section` at
all — `ABCD-WB-08-00`, `ABCD-FE-01-00`, `150_5335_5a`, `docs-quality-MA.IMS.00001` — are a different
question and are untouched here.

**So the round is neither of the brief's two groups: it is the one mechanism that spans the survivor
of A and the head of B.** `.ods` and `.odp` hold no `text:section` at all, so this is the `.odt`
column alone.

---

## 2. The seat, in LibreOffice's own source

**A multi-column stretch inside a page is not a page style; it is a `SwSectionFrame`.** Both Word
importers say so and both build the same object.

* `SectionPropertyMap::CloseSectionGroup` — *"prefer setting column properties into a section, not a
  page style if at all possible"* — then
  `xSection = rDM_Impl.appendTextSectionAfter(m_xStartingRange); ApplyColumnProperties(xSection, …)`
  (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1905-1913).
* `wwSectionManager::InsertSection` builds a `SwSectionFormat`, puts the section's own indents on it
  as an `SvxLRSpaceItem` — `nSectionLeft = rSection.GetPageLeft() - nPageLeft`, and the right twin —
  and only then calls `SetCols` (`sw/source/filter/ww8/ww8par6.cxx`:735-745). **The indent is a
  difference from the page style's margins**, which is exactly what `644730BRI`'s two numbers sum to.
* `SwSectionFrame::_UpdateAttr`/`Init` puts that `SvxLRSpaceItem` on the frame's *print* area —
  `aRectFnSet.SetLeft(aPrt, rLRSpace.ResolveLeft())`, under the comment `#109700# LRSpace for
  sections` — and creates the column frames whenever `nCols > 1 && !IsInFootnote()`
  (`sw/source/core/layout/sectfrm.cxx`:130-181). A section frame begins where the flow reaches it, so
  **both its columns and its indents take effect mid-page**, which a page style's margins do not.

LibreOffice's ODF export writes that object back out as a `text:section` whose section-family style
carries `style:columns` and, where they differ from the page's, `fo:margin-left`/`fo:margin-right`.
Reading it is the exact inverse of what produced every one of these files.

**And the per-column widths are read only where LibreOffice reads them.**
`XMLTextColumnsContext::endFastElement` takes the `style:column` descriptions only under
`!bAutomatic && maColumns.size() == nCount`, and `bAutomatic` is set by the mere *presence* of
`fo:column-gap` (`xmloff/source/text/XMLTextColumnsContext.cxx`:216-222, :268-315); everything else
gets `setColumnCount` and an automatic distance, which is even columns. A `style:rel-width` is the
column's **outer** width, its own indents included — `SwFormatCol::Calc` apportions the frame between
the wish widths and then takes each column's left and right margin off its own share, and the DOCX
importer builds the same shape (`pColumn[nCol].Width = (fWidth + fLeft + fRight) * fRel`,
`dmapper/PropertyMap.cxx`:868-874).

`census-columns.py` separates the two arms over the 338 converted `.odt`:

| where | arm | elements | documents |
|---|---|---:|---:|
| `style:section-properties` | states `fo:column-gap` | 107 | 95 |
| `style:section-properties` | per-column widths, **no** gap | **5** | **4** |
| `style:page-layout-properties` | states `fo:column-gap` | 8 | 4 |
| `style:page-layout-properties` | per-column widths, no gap | 0 | 0 |

**33 of the 107 gap-stating sections also state unequal `style:rel-width`**, which LibreOffice
ignores — so reading those widths would move 33 sections the reference draws even. The four documents
in the second row are `absrc-pac-01-info-note-en` and the three `150_5300_13` revisions.

---

## 3. The change

* **`OdfSectionGeometry`** reads a `text:section`'s section-family style — the column count, the gap,
  the two indents, and `text:dont-balance-text-columns` (ODF states the negative, so absent means
  balanced) — and answers null for a section that changes nothing, which is 76 of the 139.
* **`OdfPageGeometry.ColumnRulerOf`** builds a `ColumnRuler` from the per-column descriptions under
  LibreOffice's own condition, and serves both the page layout and the section.
* **`OdtLayoutSource.EnterColumnSection`** allocates a section derived from the master the walk is on,
  and `LeaveColumnSection` allocates the one that restores it — through a section of its own rather
  than by returning to the master's index, whose break is `NextPage` and would start a page where the
  file merely ends a columned stretch. Guarded to the body and to one level: of the 139 sections,
  **none is nested inside another and none is inside a table**.
* **`OdtWordDocument.Derived`** turns each into a `WritingSection` on the master's own paper, running
  heads and page numbering, with the indents *added* to the master's margins.
* **`WritingSection.IsTextSection`** is what tells the paginator the difference. On a continuous break
  it takes the section's left and right margins at once instead of deferring them to the next sheet —
  the deferral that `b050-19.docx` measured is a *page style's* margins and is untouched — and
  `ContinuousPageDescriptors` skips such a section outright, because the rule it applies is about a
  continuous section whose page style reaches no page and a text section never asked for one.

### The arithmetic is confirmed at the reference with no free parameter

`gen-section-columns.py` builds a three-section DOCX — continuous, two-column, continuous — and
26.2.4.2 converts it to the `text:section` shape every corpus document has. Rendering
`tests/corpus/features/odt-section-columns.odt` both ways:

| | 26.2.4.2 | this tree |
|---|---|---|
| PARA01–08 | 72.10 | 72.00 |
| PARA09–14, TWOEND | **324.10** | **324.00** |
| PARA09 baseline | 163.20 | 163.20 |
| THREEAFTER (below the section, same page) | 72.10, y **512.90** | 72.00, y 512.90 |

`72 + 216 + 36 = 324` is the whole of it, and the 0.10 pt is the constant this corpus carries. Every
line agrees, the balance falls in the same place, and the flow returns to the full measure on the
same page.

**And the unequal arm is confirmed the same way.** Replacing that fixture's `style:columns` with
`absrc-pac`'s — `rel-width` 3240 and 6120, `fo:start-indent`/`fo:end-indent` 0.25 in, no gap —
26.2.4.2 draws the second column at **252.10**, which is `72 + 144 + 36`: the first column's share of
the 468 pt measure is `3240/9360 × 468 = 162`, less its own 18 pt end indent. Rewriting the two
widths to 22687 and 42848, which sum to 65535 instead of 9360, changes nothing, so the magnitudes are
normalised and only the ratio counts.

---

## 4. What moved

`rows-before.tsv` is `/home/user/odt-r87-after/rows.tsv` (this round's base) and `rows-after.tsv` is
`/home/user/r88-odt-b/rows.tsv`; both are scored against the same banked reference bytes, so only our
side moved, and **no row failed on either side in either run**.

| | `.odt` track |
|---|---|
| before | **291** of 338 match |
| after | **292** of 338 match |

| document | before | after |
|---|---|---|
| `644730BRI0mna000BOX361539B00public0` | `pages` 4/5 | **match** 5/5 |
| `JEMIT_Template` | `pages,words` 6/4, 8860/8677 | **match** 4/4, 8675/8677 |
| `ABCD-SDE-23-00 - Avionic System Description` | `pages,words` 30/29 | `words` **29/29** |
| `absrc-pac-01-info-note-en` | match 7/7 | **`pages` 6/7** |

and three more improve without changing a verdict: `150_5300_13_chg10` 85 → **79** pages against 76,
`chg8` 21 → **19** against 18, `chg12` 35 → **34** against 32.

**One verdict is not the size of the change.** A column position adds no glyphs and no pages of its
own, so what the defect actually moves is where a line starts. **11 of the 338 renderings change**
with the conversion date masked (`movers.py`, `movers.txt`), and over their matched lines:

| matched | mean \|Δx\| from 26.2.4.2 | lines within 0.1 pt |
|---:|---:|---:|
| 9896 → 10617 | **8.593 pt → 4.083 pt** | **4858 → 5199** |

Per document, of the 11: **7 better, 4 worse** (`startx.txt`). `ABCD-SDE-23-00` goes 45.95 → **0.566**
pt, `JEMIT_Template` 33.83 → **2.27**, `644730BRI` 65.54 → **10.02**, `022_Unit_Circle_Chart` 21.26 →
8.75.

Only 11 of the 33 columned documents move at all, and the reason is worth recording: **23 of the 33
are the `chartset` templates** — Organogram, Unit Circle, Storyboard, Venn — whose single page is
drawings with almost no flowed text, so a two-column section over nothing changes nothing.

### The four that get worse are one class, and it is the reference declining its own columns

`absrc-pac-01-info-note-en` (0.23 → 9.11 pt), `手机免提系统TSB` (3.97 → 16.37), `150_5300_13_chg12`
(17.60 → 21.33) and `mde087077~283` (0.10 → 5.55). On the first two, **26.2.4.2 draws the section in
one column although the file states two**, and this tree now draws two.

Established by variant, not inferred. On `absrc-pac` (`sect-variants.py` and the two scripts beside
it), 26.2.4.2's rendering is **byte-for-byte identical in its line positions** with the section's
columns removed, with the per-column widths removed, with a `fo:column-gap` added, with the widths
rewritten to sum to 65535, with the `text:table-of-content` removed, and with the table removed. So
the columns are inert in that file. They are **not** inert in the style: the same fixture as §3
carrying `absrc-pac`'s exact `style:section-properties` — `dont-balance`, `writing-mode`, `editable`,
3240/6120, no gap — is drawn by 26.2.4.2 in two columns at 72.10 and 252.10.

**What does restore them is the content.** Replacing that section's table *and* its
table-of-content with 39 ordinary paragraphs makes 26.2.4.2 draw the section in two columns at
**72.10 and 252.10** — the positions this tree computes — 19 lines in the first and 20 in the second.
Removing either the table or the index alone does not. So the condition is about what the section
holds, and naming it is left; the arithmetic under it is right.

The proximate cost is visible in our own rendering and is a second, older gap: on `absrc-pac` page 1
this tree draws the section's table across the **whole measure** (`INFORMATION HIGHLIGHTS` at
x = 311.27 in a 144 pt first column) rather than inside the column the flow is in, and the flow then
resumes in the second column. **3 of the 33 columned documents hold a table inside a columned
section** (`census-tables-in-sections.py`): `absrc-pac` and two of the three `150_5300_13` revisions.
A table that is laid out against the page rather than against its column is the next thing this
mechanism wants.

### Confinement, measured rather than argued

The **33 columned `.odt`'s own `.docx`/`.doc` originals** from `/home/user/sample-files`, their
**`.rtf` twins** from the converted corpus, and ten `.ods` and ten `.odp` — 86 documents — were
rendered by a binary built with this round's seven source files reverted to `a3700bba2` and again with
them restored, in the same tree, with `obj`/`bin` cleared on both legs and the restore done with `cp`
and an explicit `touch`:

```
86 renderings compared with the date masked, 0 differ
```

`confine.sh`, `confine.list`, `confinement.txt`. So the same content through the WW8, DOCX and RTF
readers does not move by a byte, and neither do the two other ODF columns — which is what the guards
predict: `WritingSection.IsTextSection` is false for every reader but this one, and
`OdfPageGeometry.ColumnRulerOf` fires on no page layout in the corpus (all 8 state a gap).

The restored binary was then checked against the sweep it is claimed to have produced: one document
re-rendered and compared byte for byte with the date masked, **0 differ**.

---

## 5. Tests

`XRay.Office.WordProcessing.Tests` **1834 → 1842 passed, 0 failed** (eight new in
`OdtTextSectionTests`). The other nine non-fidelity projects run individually: 519, 109, 728, 302,
164, 143, 1226, 1044, 259 passed, **0 failed and 0 skipped throughout**.
`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed / 0 skipped**, and the ten are exactly the
expected names — `PageDrawingComparisonTests.EveryLineIsDrawn` ×4 (`paginated.docx/.doc/.fodt/.rtf`),
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`,
`JustificationShrinkComparisonTests`.

`OdtTextSectionTests` — eight assertions: a section that changes nothing reads as nothing; the count
and gap; the indents; balancing as the absent state, both arms; a stated gap making the columns even
however they are described; the unequal apportioning, against the figures 26.2.4.2 itself draws; and
the fixture laid out end to end.

`tests/corpus/features/odt-section-columns.odt` is 26.2.4.2's own conversion of a DOCX
(`gen-section-columns.py`), not a hand-written flat ODF — the trap `dotnet/CLAUDE.md` records for
authored ODF probes.

## 6. Files

| file | what it is |
|---|---|
| `sweep-odt-ours.sh` | re-renders our half of the `.odt` track against a banked reference |
| `render-list.sh` | renders an arbitrary list of documents, keyed by stem and extension |
| `startx.py` | the corrected start-x instrument: spans merged per baseline, unique-line column |
| `dump.py` | every text line of a PDF as (page, x, y, text), or two PDFs' lines matched |
| `census-sections.py` | `text:section`, its column count and its margins, by document |
| `census-columns.py` | every `style:columns` by where it sits and how it describes its columns |
| `census-tables-in-sections.py` | which columned sections hold a table |
| `variants.py` | one-attribute variants of a document's section styles, rendered through 26.2.4.2 |
| `sect-variants.py` | the same for one named section style |
| `gen-section-columns.py` | the DOCX behind the fixture |
| `movers.py`, `movers.txt`, `movers.list` | which renderings changed, with the date masked |
| `rows-before.tsv`, `rows-after.tsv` | the two sweeps |
| `startx.txt` | the before/after start-x table over the movers |
| `confine.sh`, `confine.list`, `confinement.txt` | the no-reach check, built both ways |
