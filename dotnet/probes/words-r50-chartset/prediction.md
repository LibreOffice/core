# words-r50-chartset — prediction

Written and committed **before** any post-change rendering. Environment: LibreOffice
**26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, corpus
`/c/sandbox/workdir/sample-files`, base commit `ac147b7e5bb`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`.

## Baseline reproduced

`batch-check.sh … 'words/chartset-*' … 6` → `TOTAL 137  MATCH 110  MISMATCH 27  REF-CANNOT-RENDER 0`.

The 27 mismatching paths are **exactly** the 27 rows `MANIFEST.tsv` marks `family=words,
batch=chartset-*, status=open`. Baseline matches the manifest document for document. (The
words brief says "26 of the 37"; the manifest itself says 27 — 27 chartset + 3 ceiling +
1 extra + 3 metrics + 1 missing + 2 pagination = the 37 open. The sweep agrees with the
manifest, not with the brief's arithmetic.)

## What was measured before predicting

**The ten `Printable_Graph_Paper_Template` documents are the control the brief asked for:
five fail (080, 084, 086, 088, 089) and five pass (081, 082, 083, 085, 087).** All ten are a
grid table plus a handful of anchored drawings. The cluster is **not one defect** — it is two,
and the split is visible in the markup:

| | `w:tblpPr` present | verdict |
|---|---|---|
| 080, 084, 089 | yes (`vertAnchor="page"`) | `pages` 2/1 |
| 081, 082, 083, 085, 087 | yes | match |
| 086, 088 | **no** | `pages` 2/1 |

### Defect A — `w:tblpPr` is not honoured vertically and the table is not taken out of the flow

Measured from the PDFs' own stroke geometry. Our table's first horizontal rule sits at the top
margin on every one of the eight positioned documents; the reference's sits where `w:tblpY`
and `w:vertAnchor` put it, and the law is exact:

| doc | vertAnchor | tblpY (tw) | predicted ref y (pt) | measured ref y | ours |
|---|---|---:|---:|---:|---:|
| 080 | page | 1786 | 752.59 | 751.84 | 769.40 |
| 084 | page | 1025 | 790.64 | 790.09 | 769.40 |
| 089 | page | 1741 | 754.84 | 754.54 | 769.65 |
| 082 | page | 1513 | 766.24 | 765.44 | 769.40 |
| 085 | page | 1606 | 761.59 | 760.44 | 769.27 |
| 087 | page | 1025 | 790.64 | 789.84 | 769.46 |
| 083 | (none→text) | 525 | 743.40 | 743.34 | 769.65 |
| 081 | (none→text) | 579 | 494.35 | 491.15 | 520.41 |

Seven of eight within 1.15 pt (the residual is the half border width the stroke is centred on);
081 is 3.2 pt out and is not explained.

The consequence that costs a verdict is not the position, it is the flow. On 080 **both sides
draw the identical 86 strokes on page 1** — the table fits either way. The reference then draws
its four body texts on page 1 at y = 814.29 / 783.09 / 765.94 and its logo image on page 1; we
draw the same four texts and the same image on **page 2, at y = 814.30 / 783.70 / 765.95** —
the same offsets, one page later. The table consumed the flow, the anchor paragraphs after it
were pushed off page 1, and the drawings they anchor went with them.

`FlowLayouter` already has `floatsPositionedTables` and `PageFurnitureSet.cs:198` passes it
`true` for a running head. Its remarks say of the body: *"no measurement was taken there"*.
This is that measurement, and it says the body needs it too.

### Defect B — a paragraph whose only content is floating drawings is sized as if it held text

086 and 088 carry no positioned table and their tables are drawn to within 0.01 pt of the
reference's (21 verticals and 29 horizontals, identical). Four authored variants of 088, one
variable at a time, rendered through both stacks:

| variant | ours | ref |
|---|---:|---:|
| A original — `wp:anchor` drawing, mark `w:sz="4"` (2 pt) | **2** | 1 |
| B drawing run deleted, mark 2 pt | **1** | 1 |
| C drawing kept, mark raised to `w:sz="22"` (11 pt) | 2 | 2 |
| D drawing deleted, mark 11 pt | 2 | 2 |

B and D together prove the paragraph mark's size *is* honoured for an empty paragraph. A and B
isolate the cause to the presence of the drawing run. Seven further variants — `posOffset` 0,
−900000 and −266065, `wp:extent cy` cut to 9525, `behindDoc="1"`, `wrapNone`→`wrapSquare`, and
`relativeFrom="paragraph"`→`"page"` — **all** stay 2/1, so no property of the frame is
involved, only that a drawing run exists.

The seat is `DocxLayoutSource.cs:1708`: `case "drawing":` emits `AnchorCharacter` (U+0001)
into the walker's text for a *floating* drawing as well as an inline one. `Paragraph()` then
reads `walker.Text.Length == 0 ? mark : body` and takes the body style — 11 pt from
`docDefaults` — where Writer, which drops the run into a fly and leaves the paragraph empty,
takes the mark's 2 pt. On 088 the table ends 8.45 pt above the bottom margin: a 2 pt line fits
and an 11 pt line does not.

## What I will change

1. **B first** (small, in the reader): a paragraph whose entire text is anchor characters
   standing for *floating* `w:drawing` frames is sized by its paragraph mark, exactly as an
   empty paragraph is.
2. **A second** (large, in `Paginator.Fill`): a body `PageTable.IsPositioned` is placed at the
   offset `w:tblpY`/`w:vertAnchor` resolve to and does not advance the flow.

## Predicted verdict movement

**Fix B: +2 verdicts.** `086_…Gray_Theme` and `088_…Quality_layout`, both `pages` 2/1 → 1/1.
Their word columns are 4/4 and 4/2, both inside `max(2%,3)` once the page count agrees.

**Fix A: +3 verdicts**, and I am deliberately not predicting more. `080`, `084`, `089`, all
`pages` 2/1 → 1/1 with word columns 6/6, 4/4 and 2/2. I am **not** predicting movement on the
six `Project_Timeline_Template` documents (011, 012, 013, 015, 016, 017, 018) even though all
seven carry a positioned table, because five of them fail on `text` with 13 to 117 words
missing and a page-count change alone will not close that.

**Total predicted: +5 of 27, chartset 110/137 → 115/137, words 300/337 → 305/337.**

Downside risk, stated as a number rather than a hope: **27 currently-passing words documents
carry a `w:tblpPr` table** and **31 currently-passing words documents carry a text-free
paragraph holding only floating drawings** (15 of them with an explicit mark `w:sz`). Either
fix can cost verdicts there. If the net is negative I will revert the offending half and report
the refutation with the sweep numbers, per COMMON.md.

## What these censuses cannot see

- **Fix B's census matched `w:pPr/w:rPr/w:sz` literally.** A mark differing from the body by
  `w:rFonts` alone — a taller *face* at the same `w:sz` — changes the line height too and is
  invisible to this count. The words track already knows faces are the unworked half of the
  line-spacing question, so this under-reaches in exactly the known direction.
- Neither census resolves style inheritance or `docDefaults`; both read the paragraph's own
  markup. A paragraph style that sets a mark size would be missed.
- Neither census reads `.doc`, `.rtf` or `.odt`. `w:tblpPr` has RTF (`\tposy…`) and DOC
  (`sprmTPc`) spellings and an ODF equivalent (a text frame holding a table); a change made in
  the DOCX reader reaches none of them, and a change made in `Paginator` reaches all of them
  through whichever reader sets `IsPositioned`. Only the DOCX reader sets it today.
- The `.doc`/`.xls`/`.ppt` binaries cannot be token-rewritten, so `trace-text.py` cannot
  attribute anything on them.
- **The other two tracks.** `Paginator`, `FlowLayouter` and `DocxLayoutSource` are all inside
  `XRay.Office.WordProcessing`, which is not a shared layer, so slides and sheets cannot be
  reached by either fix. No cross-track sweep is owed. If either fix ends up touching `Core`,
  `Containers`, `Text`, `Vector`, `Rendering` or `Markup`, that changes and I will say so.

## What I already refuted, before changing anything

- **"The graph-paper five are one defect with five witnesses."** They are two defects with
  three and two witnesses. The five passing siblings separate them: 082/085/087 have the
  positioned table and pass, so the position alone costs nothing; what costs a verdict is
  whether anything the flow still owed lands past the page bottom.
- **"The word-count failures are tokenisation ceilings."** The whitespace-stripped charstream
  comparison was run on all 19 chartset word failures. **18 of 19 have genuinely different
  characters** — real content defects, not ceilings. The single exception is
  `069_Work_Breakdown_Structure_Template_Professional_Format`: 371 characters on both sides,
  the same 371, in a different order. That one is a reading-order/tokenisation ceiling and is
  proposed for reclassification.
- **"The mark's `w:sz` is ignored for an empty paragraph."** Variants B and D refute it: with
  the drawing run gone, a 2 pt mark gives one page and an 11 pt mark gives two, on both stacks.
