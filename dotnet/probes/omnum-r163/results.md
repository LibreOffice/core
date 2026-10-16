# omnum-r163 — numbering alignment (p29) and a "missing cell" (p30)

Document: `/home/user/sample-files/words/metrics-001/docx/OM template for non-complex NCC operators_August 2016.docx`

Reference: `/opt/libreoffice26.2/program/soffice` — **26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc`.
`PATH=/opt/libreoffice26.2/program:$PATH` is required: `lo-convert.sh` resolves `soffice` from
`PATH` and `/usr/bin/soffice` is 24.2.7.2 here.

## 0. PROVENANCE WARNING — our half is a STALE BUILD

`dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/` is dated **Sep 17 06:33**; HEAD is
`bccfeb64b` (Sep 20). Verified by string scan of the assemblies: `WordParity`, `DocxTocStyles`,
`DocxReferenceFields`, `ReferenceFieldText` and `XRAY_OFFICE_LIBREOFFICE_QUIRKS` are **absent**;
`ListLevelIndentsApplicable` is present. **Every `[bin]` figure below marked `(Sep-17)` is that
build, not HEAD.** All `[bin]` figures for the *reference* are 26.2.4.2 and are unaffected.

Nine commits touch `XRay.Office.WordProcessing` after the binary
(`git diff dd9c2baf7 HEAD -- dotnet/src/XRay.Office.WordProcessing/`). The ones in scope:

| commit | date | why it is in scope |
|---|---|---|
| `8af1cbb21` | 09-19 | list counters keyed on the **abstract** definition; `TakeStartOverride` |
| `ffbdb45ac` | 09-18 | a covered (vMerge) cell is charged to the row's height — `PageTable.cs` +29 |
| `a3159ce0d` | 09-18 | a split row is ruled once at the cut by its bottom rule — `Paginator.cs` |
| `f2207fde9`,`3a2ec4171` | 09-17 | table boundary bands / border placement — `PageDrawing.cs` |
| `72db85d5d` | 09-17 | style-stated character width reaches layout — `PageContent/Paginator` |

**Unchanged since the binary, so their source diagnosis is HEAD-valid:**
`WordParagraphFormats.ListLevelIndentsApplicable`, `DocxLayoutSource.Lists.cs`,
`Paginator.IsCoveredByAMerge`, and `PlaceTablePart`'s whole-rows loop and spanning-cell placement
(only `bandAbove` arguments were added to its `SliceRow` calls).

## 1. The A/B on `XRAY_OFFICE_LIBREOFFICE_QUIRKS` is VOID

Ran it anyway before the correction arrived: both arms 166 pages, `pdftotext -bbox` dumps
differing only in `<meta name="CreationDate">`. **That result is an artefact of the stale build
and says nothing.** Discarded. Rendering the same input twice without the variable also produces
different md5s (the PDF timestamp), so md5 is not an instrument here.

## 2. Reach of the two Word-parity rules, settled from XML + source

- **`REF` rule — cannot reach this document.** `word/document.xml` contains **0** `REF` field
  instructions (283 `instrText`, one `TOC`, 282 `PAGEREF`). `[src]`
- **`TOC \t` rule — reaches it, and reaches defect 1 squarely.** The single TOC instruction is
  `` TOC \h \z \t "Heading 1,2,Heading 2,3,Heading 3,4,Heading 0,1" ``. `[src]`
  Styles named: Heading 1, Heading 2, Heading 3, Heading 0. **Not** Heading 4, Heading 5.

## 3. Defect 1 — numbering label alignment, page 29

Page 29 is page 29 on both sides (content-matched; pages 1–153 align, the +1 page appears at 154).

### 3.1 Direction and magnitude — the NUMBER is wrong, not the text, so the GAP is wrong

`pdftotext -bbox`, pages 10–80, first word of each line matching `^\d+(\.\d+)*\.?$`, offsets from
the left margin (56.7 pt = 1134 tw). `[bin]` reference = 26.2.4.2, ours = Sep-17 build.

| heading | n | REF label | REF text | OURS label | OURS text | level `w:ind` (left,hang) | style `w:ind` |
|---|--:|--:|--:|--:|--:|---|---|
| H1 | 9 | **+28.45** | +46.45 | +0.00 | +42.55 | 927, 360 → 567/927 tw | 851/851 → 0/851 |
| H2 | 42 | **+85.15** | +103.15 | +0.00 | +42.55 | 2061, 360 → 1701/2061 | 851/851 |
| H3 | 59 | **+7.20** | +43.20 | +0.00 | +42.55 | 862, 720 → **142**/862 | 851/851 |
| H4 | 34 | +0.10 | +42.65 | +0.00 | +42.55 | 3413, 720 | 851/851 |
| H5 | 24 | +42.65 | +156.05 | +42.55 | +155.95 | 1080, 1080 | 1985/1134 |

The user's page-29 symptom is the H3 row: `2.1.5` at x 63.90 in the reference and 56.70 in ours.
**The label is 7.10 pt too far left in ours; the following text is in the same place (99.90 vs
99.25, a glyph-width difference); so the gap between number and text is 7.10 pt too wide.**
H1 and H2 are the same defect at 28.35 pt and 85.05 pt, and they are on other pages.

`142 tw = 7.10 pt = 862 − 720`, exactly the level-2 `w:ind`. The reference is using the
**numbering level's** indent for H1/H2/H3 and the **paragraph style's** for H4/H5.

### 3.2 It is a downstream consequence of the TOC `\t` rule — proved reference-side

Three modified copies of the document (only the TOC `instrText` changed, in `mod/`), rendered
with 26.2.4.2. `[bin]`

| variant | H1 | H2 | H3 | H4 | H5 |
|---|--:|--:|--:|--:|--:|
| as authored (`\t "Heading 1,2,Heading 2,3,Heading 3,4,Heading 0,1"`) | +28.45 | +85.15 | +7.20 | +0.10 | +42.65 |
| `\o "1-4"` (no `\t`) | **+0.10** | **+0.10** | **+0.10** | +0.10 | +42.65 |
| `\t "Heading 1,2"` only | +0.10 | +0.10 | +0.10 | +0.10 | +42.65 |
| `\t … + "Heading 4,5"` | +28.45 | +85.15 | +7.20 | **+134.75** | +42.65 |

The split follows the `\t` list exactly, in both directions. **Ours (+0.00) is what 26.2.4.2
itself draws when the `\t` switch is removed.**

### 3.3 The mechanism, from the reference's own resolved view

`soffice --convert-to fodt` of the document as authored and of the `\o "1-4"` variant: `[bin]`

| | H1 paras | H2 | H3 | H4 | H5 |
|---|---|---|---|---|---|
| as authored | no direct indent (32) | no (86) | no (146) | **hard** (112) | **hard** (42) |
| `\o "1-4"` | **hard** (32) | **hard** (86) | **hard** (146) | hard (112) | hard (42) |

In both, the five `Heading_20_N` *paragraph styles* carry `fo:margin-left`/`fo:text-indent`.
So: writerfilter materialises the style's `w:ind` onto **every** heading paragraph as direct
formatting; the `\t` rule then discards that materialised copy for the named styles; and with the
paragraph's hard indent gone the outline level's own `style:list-level-label-alignment` governs
(level 3 = `margin-left 0.5984in`, `text-indent −0.5in` = the measured 862/−720 tw).

`text:outline-style` levels 1–5 in the fodt are `927/−360`, `2061/−360`, `862/−720`, `3413/−720`,
`1080/−1080` tw — the `w:lvl/w:pPr/w:ind` values verbatim.

### 3.4 What our code does, and why the quirk switch will not fix it even at HEAD `[src]`

`WordParagraphFormats.Applicable` (`Ooxml/WordParagraphFormats.cs`:438-470) ports
`SwTextNode::AreListLevelIndentsApplicableImpl` (`sw/source/core/txtnode/ndtxt.cxx`:4851). For a
page-29 heading: no direct `w:ind`, no direct `w:numPr`, so it walks the style chain, meets
`Heading3`'s own `<w:ind w:left="851" w:hanging="851"/>` and returns `ListLevelIndents.No`.
`DocxLayoutSource.Lists.cs`:69-85 then keeps the style's indent. **That is Word's answer and is
the intended output under `TODO.word-parity.md`'s policy** (ECMA-376 §17.7.2: numbering sits
*below* paragraph styles in the hierarchy).

`DocxTocStyles.Prune` (`Ooxml/DocxTocStyles.cs`:124-142) copies the `w:pPr` keeping only
`pStyle`, `rPr`, `sectPr`, `pageBreakBefore`. The page-29 heading paragraphs' `w:pPr` is
**`pStyle` + `rPr` only**, so `Prune` is a no-op on them. Our pipeline never materialises the
style's indent onto the paragraph, so there is nothing for the rule to void, and the effect the
reference produces — the *style-chain* indent ceasing to block the level indent — is not modelled.
**Therefore `XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` at HEAD will still not reproduce the reference here.**
Needs confirming against a HEAD build.

### 3.5 Refuted on the way

- *A `w:suff`/tab-stop problem.* No: the text lands within 0.65 pt on both sides; only the label moves.
- *`w:lvlJc`.* All five levels are `left`.
- *An abstract-numbering indirection (`WordNumbering.cs`).* Not the *position*: `8af1cbb21`'s diff
  touches counters and `TakeStartOverride` only — no indent code. It **can** change the label
  *text* here (abstract 30 is shared by numIds 1, 30, 31, 32; the file has 35 `w:startOverride`),
  which needs re-measuring at HEAD.
- *Fixture arms A–E* (`fixtures/arm-*.docx`, five arms, all with `word/settings.xml`, one without
  as a control) reproduce the **style-wins** behaviour on both sides but do **not** reproduce the
  document's level-wins behaviour, because they carry no TOC field. Arm E did expose an unrelated
  real divergence — see §5.

## 4. Defect 2 — the "missing cell", page 30

Page 30 is page 30 on both sides.

### 4.1 The intended grid `[src]`

Table = "Storage of Flight Crew records". `w:tblGrid` = **4** `w:gridCol` (2972, 1985, 2557, 1848).
12 rows, every row **4 × `w:tc`**, **no `w:gridSpan` anywhere**. Column 4 vertical merges:

| row | col-1 text | col-4 |
|--:|---|---|
| 0 | Document (tblHeader) | — |
| 1 | Flight Duty and Rest Period | — |
| 2 | License | `vMerge restart`, "[responsible person]" |
| 3–7 | Conversion / Recurrent / Differences / Training and Checking / Recent experience | `vMerge` (continue) |
| 8 | Route and aerodrome competence | `vMerge restart`, "[responsible person]" |
| 9 | Training and qualifications … RNP | `vMerge` (continue) |
| 10 | Dangerous Goods No Carry | `vMerge` (continue) |
| 11 | Radiation Exposure Records | — |

So rows 8–10 are one merged block in column 4, anchored at row 8.

### 4.2 Row-by-row column counts, three ways

| row | intended | REFERENCE drew | OURS drew (Sep-17) |
|--:|--:|---|---|
| 0–7 | 4 | 4 (p30) | 4 (p30) |
| 8 | 4 | 4 (p30) | 4 (p30) |
| **9** | **4** | **4 — split across the page break**: cols 1–3 and the merged col 4 on p30 down to the page's table bottom; a 12 pt follow part (the row's trailing empty paragraph) on p31 | **0 on p30**; 4 on p31 — *but* col 4 is missing there, because the merged cell was already drawn on p30 |
| 10 | 4 | 4 (p31) | 4 (p31), col 4 absent |
| 11 | 4 | 4 (p31) | 4 (p31) |

**Which of the four meanings:** neither "borders missing" nor "content missing from a complete
grid". It is the third — **the row is absent from page 30's grid entirely for columns 1–3** —
plus a second, independent fault: **the row-spanning column-4 cell is present but drawn at its
full un-truncated three-row height**, so it extends ~105 pt below the rest of the table.

### 4.3 The measurements `[bin]`

Page-30 strokes (`pdf-ops.py dump --page 30 --only stroke`), PDF coords, y up:

REFERENCE: horizontals 452.34 (top) 440.34 416.84 347.34 312.34 277.34 242.34 207.34 183.84
148.84 **67.84** (bottom, full width 98.75→566.35); the 347/312/277/242/207/148 rules stop at
x = 474.35 (inside the merge). Verticals x = 98.5 / 247.1 / 346.3 / 474.1 / 566.6, **all spanning
67.59→452.59**.

OURS: horizontals identical down to 148.85 — then `98.20→474.40 @148.85` and a lone
`473.90→566.80 @ 43.40`. Verticals: x = 98.45 spans **148.60→452.60**, 247.05 and 346.30 span
149.10→452.10, but **x = 474.15 spans 43.65→452.10** and x = 566.55 spans 43.15→452.60.
So columns 1–3 end at y = 148.6 and column 4 runs on to y = 43.4.

Page 31: reference draws the follow table 730.64 → 635.64 with a **partial-width** rule at 706.64
(the row-9 tail, 12 pt) — Writer's follow flow line. Ours draws 730.60 → 555.15 with row 9 whole
at 92.5 pt and **no x = 566.55 vertical below 613.40**, i.e. no column 4 for rows 9–10.

Row 9's full height is 92.5 pt (8 lines: `Training and qualifications for the specific
operations:` wraps to 2, then EUR RVSM / NAT-MNPS / LVTO / STEEP APPR / RNP, then a 7th, **empty**
paragraph). The reference puts 81 pt (7 lines) on p30 and 12 pt on p31. Room below row 8 on p30 is
~81 pt, so the row fits only if it can be split.

Contaminated visual check (mine, having already read the instruments): `pair30.png` at 110 dpi —
our page 30's last table band shows an empty three-column area with "[responsible person]" alone
at the right, extending below the other columns. Consistent with the strokes. **No uncontaminated
reader was available: this container has no `Task`/subagent tool (checked via `ToolSearch`), and
`mcp__Claude_Code_Remote__create_session` spawns a sibling in another container that cannot open
`/home/user/...`.** page-vision §"Delegate the reading" documents this.

### 4.4 Root cause `[src]` — HEAD-valid

`Paginator.PlaceTablePart` (`Layout/Paginator.cs`:3845-3985):

1. The whole-rows loop `while (end < heights.Count && placed + whole + heights[end] <= room)`
   stops at row 9 (92.5 > ~81 available) — **no merge awareness**, so the table is allowed to
   break at a boundary *inside* the rows 8–10 merge span.
2. `cells.AddRange(TableLayouter.Offset(laid.Cells.Where(cell => cell.Row >= start && cell.Row < end) …))`
   places the row-8 anchor cell — `RowSpan = 3` — **at its full three-row height**, because
   nothing clamps a spanning cell to the rows actually on this page.
3. The follow-flow-line branch is then skipped by
   `&& !IsCoveredByAMerge(laid, end)` (`:3948`, helper at `:4076-4084`): row 9 is covered by the
   row-8 merge, so `SliceRow` is never called and the whole row moves to page 31.

Writer does both of the things we do not: `SwTabFrame::Split` splits the row, and
`lcl_AdjustRowSpanCells` (`sw/source/core/layout/tabfrm.cxx`:909-931) recomputes each
`GetLayoutRowSpan() > 1` cell's height as `lcl_GetHeightOfRows(pRow, nLayoutRowSpan)` — the rows
**present in that frame** — called at `:1028` and `:1512`.

`IsCoveredByAMerge`'s own remark asserts "Writer keeps the same case out of its own split by
re-formatting the line that a row span crosses (`lcl_AdjustRowSpanCells`); declining is the same
answer with none of the machinery." **That is the refuted claim.** `lcl_AdjustRowSpanCells` is
what *enables* the split, not what avoids it.

### 4.5 Minimal fixture — reproduces both halves, with a control

`fixtures/vm-filler{44..55}.docx` and `fixtures/vm-none-filler50.docx`, built by
`fixtures/table_arms.py`, each with a `word/settings.xml` (`compatibilityMode 15`). Three rows,
4 columns, all borders single, column 4 `vMerge` across all three rows; N filler paragraphs walk
the table across the page break. `[bin]`

| arm | REFERENCE | OURS (Sep-17) |
|---|---|---|
| filler44–51 | table intact / row 3 alone on p2 | same |
| **filler52** | p1: rows 1+2 · p2: row 3 | p1: **row 1 only** · p2: rows 2+3 |
| **filler53/54/55** | **row 2 SPLIT**: opening lines p1, `RNP` p2 | row 2 moved whole to p2 |
| **vm-none-filler50** (control, no `w:vMerge`) | p1 rows 1+2 · p2 row 3 | **identical** |

The control is the discriminator: remove the vertical merge and the two sides agree.

filler53 strokes `[bin]`: REFERENCE p1 table 135.14 → **45.64** with all five verticals spanning
45.39→135.39 and the row-1/2 rule at 122.04 stopping at x = 432.85; p2 805.64 → 766.64 with a
partial rule at 779.84. OURS p1: columns 1–3 end at 122.05, column 4 runs to **7.45** with its own
bottom rule at y = 7.20 (bottom margin is 36 pt — it is off the text area and nearly off the
paper); OURS p2 has **no column 4 at all** (no vertical at x = 525.05, no full-width rules).

### 4.6 Refuted for defect 2

- *A `w:gridSpan` we mis-sum.* There is no `w:gridSpan` in the table.
- *A row-height error.* Our row heights match the reference's to 0.01 pt for rows 0–8, and row 9's
  92.5 pt = the reference's 81 + 12 page parts. Nothing is mis-measured; the split is refused.
- *The known divergence.* No `REF` field in the document; the TOC `\t` rule voids paragraph
  formatting on built-in heading styles and cannot reach a table cell in `NCCNormal0`.

## 5. A second, unrelated divergence found by fixture arm E `[bin]` + `[src]`

`fixtures/arm-e-only-top-ind.docx`: Heading1 states `w:ind` and carries `w:numPr` with `w:numId`;
Heading2–4 are `basedOn` it and state only `<w:numPr><w:ilvl/></w:numPr>`.

| | H1 | H2 | H3 | H4 |
|---|--:|--:|--:|--:|
| REFERENCE | +0.10 | **+0.10** | +7.20 | +134.75 |
| OURS | +0.00 | **+85.05** | +7.10 | +134.65 |

`WordParagraphFormats.Applicable` returns "applicable" as soon as it meets a style whose
`w:pPr` has **any** `w:numPr` child (`:468`). Writer tests `RES_PARATR_NUMRULE`, which a
`w:numPr` containing only `w:ilvl` does **not** set — such a style carries a level, not a rule, so
the walk must continue to the parent, where Heading1's `w:ind` stops it. **Ours takes the level
indent for H2 where the reference takes the style's.** (This does not explain H3/H4 in that arm,
which the reference also takes from the level; that residual is unexplained and is not this
document's defect.) Filed here rather than chased.

## 6. What must be re-measured against a HEAD build

```sh
cd /home/user/libreoffice-core/dotnet && dotnet build XRay.Office.slnx
export PATH=/opt/libreoffice26.2/program:$PATH
CLI=tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli
D="/home/user/sample-files/words/metrics-001/docx/OM template for non-complex NCC operators_August 2016.docx"
P=probes/omnum-r163

# a) page count and the two defects, quirk off and on
$CLI render --format pdf --outdir $P/head-noquirk "$D"
XRAY_OFFICE_LIBREOFFICE_QUIRKS=1 $CLI render --format pdf --outdir $P/head-quirk "$D"
python3 - <<'EOF'   # the §3.1 table; see probes/omnum-r163 for the scanner
EOF

# b) the vMerge fixtures, both halves
for f in $P/fixtures/vm-*.docx; do $CLI render --quiet --format pdf --outdir $P/head-vm/$(basename $f .docx) "$f"; done
python3 $P/measure.py
```

Re-measure specifically:

1. **§3.1 label/text offsets** — `WordParagraphFormats`/`DocxLayoutSource.Lists` are unchanged, so
   the positions should be identical; **confirm**, and confirm the label *text* (`2.1.5`, …) has
   not changed under `8af1cbb21`, which shares one counter across numIds 1/30/31/32 and changes
   `w:startOverride` handling. If the numbers change, page 29's content changes with them.
2. **§3.4** — that `XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` still leaves page 29 unmoved (predicted from
   source: `Prune` is a no-op on these paragraphs).
3. **§4.3 stroke geometry and row heights** — `ffbdb45ac` charges a covered cell's rules to the
   row height and `a3159ce0d`/`f2207fde9`/`3a2ec4171` move the bands at a cut. Row 9's 92.5 pt and
   the ~81 pt of room are within a point of each other, so these can flip which page row 9 lands
   on **without fixing anything**. The structural facts (§4.4) are unchanged in HEAD source.
4. **§4.5 fixture arms** — same reason.
5. **The document's page count** — 166 (Sep-17) against the reference's 165, with the divergence
   appearing at page 154. Not investigated; not this brief.
