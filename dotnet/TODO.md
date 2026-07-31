# Paperless — master plan

Status: **Phase 0 complete; Phase 1 all but the OOXML and legacy spreadsheet and presentation
formats; Phase 2 complete for word processing; Phase 3's two output backends written and
verified.** Every word-processing format reads and *lays
out* — `odt ott fodt`, `docx docm dotx dotm`, `doc dot` and `rtf` — with pages, headers and
footers, tables, sections and columns all verified against LibreOffice's own rendering to a
tenth of a point, and everything a table draws — cell borders and shading — now compared
stroke for stroke and fill for fill in all four. The ODF spreadsheet and presentation formats
extract. `xlsx`, `pptx`, the legacy `xls` and `ppt`, and CSV do not read at all yet.
**A word-processing document now renders to a file**: a PDF with real searchable text and
subsetted embedded fonts, checked against LibreOffice's own PDF operator for operator, and PNG
or JPEG at a chosen DPI.
**And a deck now lays out**: PPTX and ODP produce a page per slide with shapes placed, filled,
outlined and their text set, compared against LibreOffice's own PDF for the same deck in both
formats — see Phase 3 and `src/Paperless.Presentations/TODO.md` for what draws and what does not.
**Both backends now paint everything the display list can express**: gradients as shadings and
shaders, tiled bitmap fills, and pictures decoded through Skia. What is left there is the
*reading* — nothing in any format's reader emits a gradient or a picture yet.

Each library has its own `TODO.md` with detail; this file is the ordering and the reasoning
behind it.

## Start here (next session)

Two fronts, and they are independent.

**Word processing** is deep into Phase 3's layout half, and the thing to know before picking it
up is that **the rasteriser is not a prerequisite for any of it.** Everything a word processor
draws that is not text — a footnote separator, a cell border, a cell shade — is a filled or
stroked path in LibreOffice's PDF, and `PdfFills` and `PdfStrokes` in the TestKit read them. So
those features are verifiable at the same tenth of a point as text, without pixels, and they
were: footnotes and endnotes in all four formats including the feedback loop into pagination,
cell shading in all four, and cell borders in all four, consolidated the way LibreOffice
consolidates them.

*(An earlier version of this note said the opposite — that borders could not be verified without
the rasteriser, so it had to be built first. That was the wrong conclusion from a true premise. A
word-*position* comparison genuinely cannot see a border; the PDF's content stream states every
one as an explicit path. The lesson generalises past borders, so it is left here rather than
quietly deleted: before concluding a feature cannot be compared, check what the reference
actually writes, not what `pdftotext` can be made to report.)*

Floating frames now read from **all four** word-processing formats and the body text wraps round them,
which needed a **second** circularity resolved and is worth knowing before touching pagination: a
frame's position depends on where its anchor paragraph landed, and that paragraph's lines depend on the
hole the frame makes in them, so pagination became a bounded loop rather than a single pass. DOC came
last because its frames arrive through the Escher drawing layer rather than through markup, and once
the reading existed it needed no new layout at all.

What remains in the layout half is table auto-layout for a table whose columns state no widths, the
note-numbering restart *application* pass, RTL text in the layouter — newly unblocked, since bidi
levels are resolved and carried but nothing consumes them — and the tail of the frame work: contour
wrap, two stretches of text on one line (what a `parallel` wrap round a frame touching neither margin
needs), and frames in cells and headers. Read `src/Paperless.WordProcessing/TODO.md`, whose open items
each say what is missing and why.
**The formats that do not read at all** are `xlsx`, `pptx`, `xls`, `ppt` and CSV. The
spreadsheet pair is the larger prize, since `ods` already extracts and `Paperless.Spreadsheets`
has the model. Two pieces of shared infrastructure are worth building deliberately rather than
per format, because each buys three formats at once: **DrawingML text bodies** in
`Paperless.Ooxml`, shared by DOCX, XLSX and PPTX, and **Escher/MS-ODRAW** in
`Paperless.MsBinary`, which DOC, XLS and PPT all delegate their drawings to — the DOC reader has
an open item waiting on exactly that.

A third piece of that shared infrastructure is now **built**: **DrawingML theme-colour
resolution**, in `Paperless.Ooxml/DrawingML`. It was written for DOCX's `w:themeColor`, but the
chain is DrawingML's, so it is also what XLSX's `theme=` index on a `fgColor` and PPTX's
`a:schemeClr` need — neither has to grow its own, and neither should. What is left for each is
only its own spelling of a reference: XLSX addresses a slot by number and applies its `tint`
attribute as a signed fraction, which `Color::addExcelTintTransformation` turns into a `lumMod`
and a `lumOff`; PPTX has to find its `a:clrMap` on the slide master and the overrides on the
layout. The chain itself is verified against LibreOffice's rendering to the byte — read
`src/Paperless.Ooxml/TODO.md` first for the two facts about it that a reimplementation would get
wrong, since both are invisible in the output until the colours are compared numerically.

Three decisions from the ODF and DOCX work should carry over rather than be rediscovered:

- **Keep "set here", "inherited" and "defaulted" apart.** `OdfPropertyOrigin` and
  `WordPropertyOrigin` are what make hard formatting distinguishable from an inherited style —
  and in DOCX the toggle rule is *impossible* to implement without knowing which layer set a
  value, which is the concrete payoff for not collapsing the three cases.
- **One content walker per family group, not per format.** `OdfContentReader` serves all three
  ODF families because the text content model is shared; DrawingML text bodies are similarly
  shared across DOCX, XLSX and PPTX, so `Paperless.Ooxml` is where that belongs.
- **Normalise the input once, at load.** Strict-versus-transitional namespaces and
  `mc:AlternateContent` are both handled in `OoxmlXml.Normalise` rather than at every point that
  walks the tree. The bug that prevents — a text box extracted twice — is invisible until a
  diff.

Before trusting any comparison, run
`.claude/skills/libreoffice-reference/scripts/check-env.sh`. **A fresh container will not
have the LibreOffice application modules, poppler-utils, or the Carlito/Caladea fonts** — the
script prints the exact `apt-get` lines. `libreoffice-core` alone gives an `soffice` that
starts, reports a version and then fails on every document, which is why
`LibreOfficeRunner.IsAvailable` decides by converting a probe file rather than by finding the
binary.

## Where things stand

| Done | |
|---|---|
| ✅ | Solution: 12 libraries + CLI + 9 test projects, warning-free on .NET 10, 852 tests passing |
| ✅ | `Paperless.Core` API surface: units, geometry, colour, document model, drawing IR |
| ✅ | `FormatCatalogue`: all 43 formats described |
| ✅ | Content-based format identification, verified on all 17 corpus formats |
| ✅ | OLE2/CFB reader, hand-rolled, tolerant of real-world damage |
| ✅ | ZIP / OPC / ODF package readers, with zip-bomb, traversal and XXE guards |
| ✅ | `paperless identify`, text and JSON |
| ✅ | 17-format test corpus committed, so unit tests need no LibreOffice install |
| ✅ | Six research documents (~6000 lines) covering the LibreOffice implementation |
| ✅ | Four comparison skills with verified working scripts |
| ✅ | Dependencies audited: all permissive, none gated behind a build-time licence check |
| ✅ | ODF extraction: `odt ods odp ott ots otp fodt fods fodp`, with style resolution, metadata, text, lists, tables, notes, comments, frames and shape text |
| ✅ | DOCX extraction: `docx docm dotx dotm`, with the §17.7.3 toggle rule, numbering, fields, tracked changes, tables with vertical merges, notes, comments, headers/footers and text boxes |
| ✅ | RTF extraction: byte-level tokeniser, destination skipping, `\ansicpg`/`\fcharset`/`\uN` encoding, flows, fields, and tables whose merges carry no flag |
| ✅ | Legacy code pages (`Paperless.Text.Encodings.LegacyCodePages`), shared by RTF and the legacy binary readers to come |
| ✅ | `paperless extract` and `paperless metadata`, text and JSON |
| ✅ | `LibreOfficeRunner` and an automated extraction comparison against LibreOffice, skipping cleanly when it is not installed |

| ✅ | DOC (WW8) extraction: the piece table, FKP formatting indexes, the eight subdocuments, list labels computed from `LSTF`/`LVL`/`LFO`, tables with the merges LibreOffice writes with no flag |
| ✅ | Text layout: a hand-rolled OpenType reader, HarfBuzz shaping, UAX #14 line breaking, and paragraph layout with alignment, justification, tabs, indents and line spacing |
| ✅ | Word-processing page layout: pagination, headers and footers, tables as grids that split across pages with repeating headings, several sections per document, and columns — all four formats, all compared against LibreOffice's own rendering |
| ✅ | Footnotes and endnotes: read from all four formats, placed with the feedback loop into pagination that a footnote needs and the pages of their own that an endnote takes, with the escapement rule measured off LibreOffice's PDF content stream |
| ✅ | Table cell borders and shading in all four formats, compared stroke for stroke and fill for fill — including the two rules that separate a Word table from a Writer one: the table's origin against its left border, and Word's own grid-line joins |
| ✅ | Note numbering: the sequence, the start value and the *restart rule* read from all four formats. Applying a restart is pagination's half and is still open |
| ✅ | Tracked changes, bookmarks and fields recorded in the model with their author, timestamp and range, from all four formats — extraction still resolves changes rather than showing marks, which is what LibreOffice's *renderer* does and is now measured rather than assumed |
| ✅ | Bidi (UAX #9), script itemisation (UAX #24) and mid-run font fallback in `Paperless.Text`, differentially tested against ICU — the library LibreOffice itself resolves bidi with — and against LibreOffice's own PDF portion boundaries |
| ✅ | **PDF output**: a hand-rolled writer with `hb-subset` font embedding, real searchable text and a `ToUnicode` built from the cluster map — compared against LibreOffice's own PDF *operator for operator*, per line, over ten documents in four formats |
| ✅ | **Raster output**: PNG and JPEG at a chosen DPI over SkiaSharp, deterministic, and `paperless render` with a format, a page range and a DPI |
| ✅ | **Gradients, tiled bitmap fills and raster image decode**, in both backends: axial and radial shadings with stitched ramps and luminosity soft masks in PDF, the matching Skia shaders, one shared band decomposition for the two kinds neither can state, and a Skia-backed decoder feeding the display list's `RasterImage` |

| Not started | |
|---|---|
| ❌ | `xlsx`/`pptx`, `xls`/`ppt` and CSV readers |
| ❌ | Decryption (detection works; decryption does not) |
| ❌ | SVG writer |
| ❌ | Floating frames with text wrap; table auto-layout; RTL text in the layouter |
| ❌ | Spreadsheet print layout and slide rendering |
| ❌ | Vector import (WMF/EMF/EMF+/SVG) |
| ❌ | The CLI beyond `identify`, `extract` and `metadata` |

## Ordering principle

Two rules drive the sequence below.

**Build the widest-leverage thing first.** OLE2/CFB unlocks DOC *and* XLS *and* PPT.
Escher unlocks shapes in all three legacy formats. OPC unlocks all of OOXML. Doing these
early means later work is mostly format-specific detail rather than plumbing.

**Get extraction working across all formats before rendering any of them.** Extraction is
the common use case, needs no fonts or rasteriser, and is far easier to verify — and a
parsing bug found in text is enormously cheaper to fix than the same bug found in pixels.
Rendering on top of a shaky reader wastes effort twice.

---

## Phase 0 — Foundations

Nothing else can be verified until these work.

- [x] **Format identification** (`Paperless.Core`, `Paperless.Containers`). Done:
      `FormatCatalogue` covers all 43 formats; `FormatIdentifier` detects by content via
      ODF `mimetype`, OOXML main-part content type (resolved through `_rels/.rels`), OLE2
      root stream names, and text/XML signatures. Verified against all 17 corpus formats,
      including with deliberately wrong extensions and with no file name at all.
- [x] **OLE2 / CFB reader** (`Paperless.Containers`). Done, hand-rolled: header, DIFAT/FAT
      chains, mini-FAT and mini-stream, directory walk with cycle guard. Tolerates truncated
      files, out-of-range sectors, cyclic chains and duplicate entries, reporting each as a
      `Diagnostic`. `OpenMcdf` proved unnecessary and its package reference has been
      removed.
- [x] **ZIP + OPC + ODF packages** (`Paperless.Containers`). Done: `ZipPackageBase` over
      `System.IO.Compression` with part-name normalisation and zip-bomb/traversal guards;
      `OpcPackage` resolves content types and relationships and finds the main part by
      following the officeDocument relationship; `OdfPackage` reads `mimetype`, cross-checks
      the manifest and detects per-entry encryption. XML is parsed with DTD processing
      prohibited throughout (XXE).
- [x] **`paperless identify`** end to end. Done: text and `--json` output, correct on all 17
      corpus formats, with sysexits-style exit codes.
- [x] **Corpus and fidelity harness wiring**. Done: `tests/corpus/minimal/` and
      `tests/corpus/features/` are committed; `LibreOfficeRunner` drives headless `soffice`
      with a private profile, batched conversions and an availability probe that survives a
      `libreoffice-core`-only install; `TextComparer` normalises the way the
      `extraction-comparison` skill documents. `OdfExtractionComparisonTests` runs the whole
      ODF corpus against LibreOffice and skips with a reason when it is absent.

## Phase 1 — Extraction, all formats

Per format: metadata, then text, then tables/structure.

- [x] ODF text/spreadsheet/presentation (`odt ods odp` + template and flat variants). Done:
      the three-container style resolution with set-here/inherited/defaulted kept apart,
      `meta.xml`, the shared text walk, and tables shared between Writer tables and Calc
      sheets. The minimal corpus matches LibreOffice's text filter exactly; the features
      corpus accounts for every difference (see `src/Paperless.OpenDocument/TODO.md`).
      Remaining ODF gaps are tracked there — tracked changes, number-format application,
      embedded objects, and the OpenOffice.org 1.x namespaces.
- [ ] OOXML (`docx xlsx pptx` + variants). Shared handling in `Paperless.Ooxml` first — done:
      namespace normalisation, `mc:AlternateContent` resolution and `docProps` metadata.
      - [x] `docx docm dotx dotm`. The interesting part was ECMA-376 §17.7.3's toggle rule; see
            `src/Paperless.WordProcessing/TODO.md`.
      - [ ] `xlsx` and `pptx`.
- [ ] Legacy binary (`doc xls ppt`). Hardest. Needs the record-stream reader, sprm/BIFF
      decoding, the WW8 piece table, and codepage handling.
- [x] RTF. The two things worth knowing: an unknown `\*` destination must be skipped whole, and
      LibreOffice writes a horizontally merged table cell with no merge flag at all — the span
      comes from the column grid.
- [ ] CSV.
- [ ] `xlsb` (import only — LibreOffice cannot write it, so test files need Excel).
- [ ] Encrypted documents, one scheme at a time
      (`research/05-infrastructure.md` section C).
- [x] `paperless extract` and `paperless metadata`, in text and JSON. The output layout is
      part of the `extraction-comparison` skill's contract: `--outdir DIR` writes
      `<stem>.txt` per input.

**Exit criterion:** extraction matches LibreOffice's text export across the whole corpus,
allowing for the reference filters' known omissions (headers, comments, notes, shape text
— where extracting *more* is correct).

## Phase 2 — Text layout

The part that decides whether rendering can ever match.

- [x] **Font resolution and metrics** (`Paperless.Text`). Hand-rolled OpenType table
      reader — we need raw `hhea`/`OS/2` access and our own precedence rules, per
      `research/06-rendering.md` section B, not a library's opinion of them.
      Reproduce LibreOffice's substitution order. **Report substitutions**: a silent one
      explains most mysterious reflows.
- [x] **Shaping** via HarfBuzzSharp — same engine LibreOffice uses, so advances agree.
- [x] **Line breaking** (UAX #14), hand-rolled — nothing in the runtime does this. Generate
      the `Line_Break` and `East_Asian_Width` tables, implement LB1-LB31. LibreOffice's breaks
      are ICU's, and one tailoring difference has surfaced: LibreOffice 24.2 breaks a justified
      line differently from a ragged one, which is recorded with its measurement in the
      word-processing TODO.
- [ ] Verification against Unicode's `LineBreakTest.txt`, which needs the file — not reachable
      from this container, so the implementation is checked against LibreOffice's own breaks
      instead. Re-checked while the bidi work was done: `unicode.org` still answers a CONNECT with
      403, so `BidiTest.txt` is equally out of reach. The bidi implementation did not need it,
      because ICU is on the machine and is a *better* oracle than a conformance file — it is the
      one LibreOffice runs. There is no equivalent for UAX #14: ICU's `BreakIterator` is not
      exposed by the BCL, so the line breaker's differential is against another implementation.
- [x] **Bidi and script sub-runs** (UAX #9 and UAX #24). The property tables *and* the differential
      cases come from **ICU** rather than from the UCD, because ICU is the library LibreOffice
      itself resolves bidi and script with — so agreeing with it is agreeing with Writer, and it
      supplies the conformance oracle `unicode.org` cannot. 7,944 generated cases agree with ICU,
      and the sub-run boundaries agree with LibreOffice's own PDF portions. Detail, and the two
      places ICU departs from the letter of UAX #9, in `src/Paperless.Text/TODO.md`.
- [ ] Drawing right-to-left text. The levels and the visual order are computed and carried on
      `MeasuredParagraph`; nothing in the layouter consumes them yet, so a mixed-direction
      paragraph measures correctly and draws its runs in logical order.
- [ ] Vertical text.
- [x] Paragraph layout: alignment, justification, tabs, indents, spacing, line spacing.

**Exit criterion — met for word processing.** For a text-heavy document Paperless breaks lines
where LibreOffice breaks them, in all four formats, which is what made every page-level
comparison after it meaningful. It was worth as much as expected: nearly every bug found since
was found because a page comparison put a word a measurable distance from where it belonged.

## Phase 2.5 — Structured text output (XHTML, then Markdown)

The step between "what does this document say" and "what does it look like", and it is deliberately
placed here rather than under rendering: **it needs the content tree and nothing else.** No fonts, no
line breaking, no rasteriser, no page geometry. Everything it emits — headings, lists with their
levels, tables, emphasis, links, notes — is already in the tree that `paperless extract` walks, and
plain text is simply the projection that throws all of it away.

It is also what most callers actually want. A retrieval or summarisation pipeline fed flat text has
lost the heading hierarchy that says what a passage is *about* and the table structure that says which
number belongs to which column; fed a rendered PDF it has to get it back by inference.

**Two stages, and the order was the point.** A semantic **XHTML** writer over `ContentNode`, then an
**XHTML → Markdown** transformation on top of it. It paid off exactly where it was expected to: stage
one is compared against LibreOffice's own export node for node, and stage two has no document parsing
in it at all, so the escaping — the part that silently corrupts output — is checkable by parsing the
result back. It also put the lossy step last: row and column spans and nested tables are still present
and correct in the XHTML, and only the final hop has to decide what to drop.

Both live in **`src/Paperless.Markup`**, a library beside `Paperless.Core` depending on nothing but it.
Not inside Core, which holds the abstractions everything agrees on rather than projections of them; and
not in one of the three family libraries, because it serves all three and the layering forbids the
sideways dependency that would need.

- [x] Stage one: a **semantic** XHTML writer serving all three families through the one tree.
      LibreOffice's own export was used as an oracle and never as a target — Paperless emits `h1`–`h6`,
      `ul`/`ol`/`li`, `table`/`thead`/`tr`/`th`/`td` with `colspan`/`rowspan`, `em`/`strong`/`s`/`u`/
      `sup`/`sub`/`code`, `a[href]`, `blockquote`, `img`, and `section`/`aside`/`header`/`footer` for
      the flows only Paperless extracts. Paperless-specific meaning rides on `class` and `data-`
      attributes rather than on invented elements, so the result stays ordinary XHTML.
- [x] Stage two: **XHTML → Markdown**, a pure transformation. `MarkdownWriter.FromXhtml` takes no
      `MarkupOptions` at all, deliberately: every option is consumed while walking the content tree, so
      a second copy of them at this stage could only disagree with the first.
- [x] The flavour, recorded once — GitHub-Flavored Markdown. See *Settled decisions*.
- [x] **Escaping**, done first and tested hardest. Escaped by *construct* rather than by character
      class: escaping every ASCII punctuation mark is permitted and safe and turns `snake_case_name`
      into `snake\_case\_name`, which defeats the point of emitting Markdown rather than text. So `*`
      is escaped everywhere (it is intraword emphasis in CommonMark) and `_` only at a word boundary
      (it is not); `#`, `>`, `-`, `+`, `=` and the `.` after a leading number only at the start of a
      line. Thirty-two hazard strings are each asserted four ways — the text survives, the block stays
      one paragraph, no inline construct appears, and it survives inside a table cell — all by parsing
      the output back. Removing `*` from the escape set fails twelve of them.
- [x] **Tables Markdown cannot express**: an HTML fallback, with the loss named. See *Settled decisions*.
- [x] **Per family.** A slide is a heading plus its content, numbered from the section's own index so
      that excluding a hidden slide leaves the rest with the numbers the document gives them; speaker
      notes are set apart as a labelled block quote. A sheet is a table under its tab name, bounded at
      1000 rows and 64 columns with the truncation announced in the output — never silent. Footnotes
      did **not** map onto `[^1]`, and the reason is measured rather than preferred; see
      *Settled decisions*. Comments become labelled block quotes carrying their author. Tracked changes
      needed no decision after all: extraction already applies them, so deleted text never reaches the
      tree and there is nothing for a projection to drop.
- [x] Images: a **reference placeholder**, not an omission. `src` is the container part holding the
      bytes (`word/media/image1.png`), which is a real reference that resolves against the original
      file; an empty `src` means "this page" in HTML, which is worse than saying nothing. Revisit when
      raster decode lands and there is something to point at.
- [x] `paperless extract --format text|json|xhtml|markdown`. `--json` stays as a shorthand and
      `<stem>.txt` is unchanged, because the `extraction-comparison` skill's scripts name that file.

**How it was verified, and what the reference turned out to be worth.** The premise held: LibreOffice
has no Markdown export filter but does export XHTML for all three families, and being well-formed XML
it is parsed rather than scraped. **Measured across eleven word-processing documents — ODT, FODT,
DOCX, DOC and RTF — Paperless and LibreOffice agree exactly on every heading's level, every list
item's nesting depth, every table cell's row, column, `colspan` and `rowspan`, and every external
hyperlink target.**

Two corrections to the note this phase used to carry, both from measuring rather than reasoning:

- The figures for `text-features.odt` were slightly off. LibreOffice's export has **eight** `li`, not
  twelve, and eight `a` **elements** of which only **one** carries an external `href` — the other seven
  are heading bookmarks and a `#ftn1` footnote anchor, artefacts of its own linking scheme. `h1`×1,
  `h2`×2, `h3`×1, `ul`×2, `ol`×2 and eight `td` were right.
- **The three families are not equally useful as oracles, and this was the surprise.** Only the Writer
  export is semantic. The **Calc** export is a bare `<table>`: no sheet name, no headings, and the used
  range padded with empty rows the file does not state (seven rows where the sheet has five). The
  **Impress** export is one absolutely positioned `<div>` per slide with no heading anywhere — the
  slide's title survives only as the `div`'s `id`. So Writer is compared on everything and the other
  two only on slide count and cell grids, which they do answer exactly. Asserting more would have been
  asserting a parity that is not there to be had.

Escaping and the span decision were checked against the tree and a parser instead, as planned:
`Paperless.Markup.Tests` emits, parses back with **Markdig** (BSD-2-Clause, test-only, on the test
project alone — `Paperless.Markup` writes Markdown and never reads it) and asserts on the syntax tree.
Sixteen corpus documents are put through both stages and checked word for word, in order, against the
content tree they came from, with the writers' deliberate additions enumerated from the tree rather
than hard-coded.

**Three gaps in the content tree**, noted rather than worked around, since the readers are not this
phase's to change:

1. A **note has no inline anchor**. Every reader bakes the anchor number into the text of the run
   beside it — "a footnote reference1 here" — so a `ContentSection` of kind `Note` cannot be tied back
   to the place it is referenced from without matching text rather than reading structure. This is what
   sank GFM footnotes.
2. A **slide's title placeholder is not marked**. `ContentSection` carries no title for a PPTX slide
   and nothing distinguishes the title paragraph from the body ones, so the slide's heading is
   "Slide *n*" and its actual title follows as an ordinary paragraph. LibreOffice's own export names
   each slide `div` after the title, so the information exists in the file.
3. The **DOC reader drops the list level on a continuation paragraph** where the ODT reader keeps it,
   so a list interrupted by one restarts its numbering in the Markdown. The document's own marker is
   preserved on `data-marker` in the XHTML either way.

**Still open.** Nothing in this phase blocks, but two things are worth doing when their prerequisites
land: fill in an image's `src` once raster decode gives it a target, and add `sheet-xls.xls` and
`sheet-csv.csv` to the round-trip corpus once those readers exist — they are deliberately absent rather
than skipped, because a skip would go on reporting itself long after it stopped being true.

**Non-goal: reading Markdown.** Paperless reads office formats; Markdown is an output projection, and
adding it as an input format would be scope Paperless has said no to.

## Phase 3 — Rendering

- [x] Skia raster backend consuming `IDrawingSink`. Paths, strokes with dashes, caps, joins and
      hairlines, positioned glyph runs, transparency groups as offscreen layers, and PNG and JPEG
      encode at a caller-chosen DPI. It was never a prerequisite for anything, which was a
      correction made here earlier and holds: a *word-position* comparison cannot see a border,
      but LibreOffice's PDF content stream states every one as an explicit path and `PdfFills`
      and `PdfStrokes` read those.
- [x] Word-processing page layout: pagination, headers and footers, tables as grids that split
      across pages with repeating heading rows, several sections per document, and columns.
- [x] Footnote placement, which feeds back into pagination rather than merely adding a note: the
      note area takes its room out of the body's, so a page with notes holds less text, and the
      loop shortens the page until it holds. Endnotes instead take pages of their own after the
      last body page, numbered i, ii, iii rather than 1, 2, 3. Notes read from all four formats,
      with the separator rule above them drawn and compared.
- [x] Table cell shading and cell borders in **all four** formats, the borders consolidated into one
      stroke per grid line as LibreOffice writes them. Two rules had to be found before the Word family
      would agree, and the second was found twice independently, from DOCX/RTF and from DOC: Writer
      positions a table by the *centre* of its left border where Word uses the border's outer edge, and
      LibreOffice joins a Word table's grid lines by Word's rule — an inner line stops a whole border
      width short of the outline it meets instead of overshooting it by half.
- [ ] The rest of it: floating frames with text wrap, table auto-layout for a table stating no column
      widths, and RTL text in the layouter — bidi levels are resolved and carried, and nothing consumes
      them yet, so a mixed-direction paragraph measures correctly and still draws in logical order.
- [ ] **Two layout findings the PDF comparison turned up**, both invisible to a word-box comparison
      and both waiting on `Paperless.WordProcessing`. A **16 pt heading's baseline sits 1.95 pt
      higher** than LibreOffice's in all four formats — the block is the same 57.650 pt tall on both
      sides and the baseline sits differently *within* a line taller than the body, so it does not
      cascade. And **`footnotes.rtf` draws its note separator 1.286 pt too high**, about 26 twips in
      the note area the RTF reader reserves, where the other four formats agree to a hundredth of a
      point on the same document. Measurements in `src/Paperless.Rendering/TODO.md`.
- [ ] Spreadsheet print layout — `ScPrintFunc`'s pagination is the routine to port
      faithfully. A spreadsheet has **no intrinsic pagination**: print settings *are* its
      page geometry.
- [x] **Slide layout, for PPTX and ODP.** A deck is an `IPaginatedDocument`, a slide is an
      `IPage` of the deck's own size, and a shape is placed, filled, outlined and its text laid
      out through the same `Paperless.Text` engine the word processor uses. Checked against
      LibreOffice's own PDF for the same deck in both formats: sheet sizes page for page,
      rectangular fills position **and colour**, and every text run's pen, baseline, size and
      glyph count — placements to a twentieth of a point, text to a tenth.
      Three rules were worth the reading, and each is a cascade if wrong.
      **Flip composes before rotation** (`oox/source/drawingml/shape.cxx:1098-1224`), which only a
      shape that is not symmetric can prove and which the corpus deck's mirrored-and-rotated
      triangle does. A group's **`chOff`/`chExt` child space** scales its children, so a group
      whose child extent is half its extent doubles everything inside it. And **a slide's line
      height comes from the font size, not from the font**, because the PPTX importer sets
      `FontIndependentLineSpacing` on every text body
      (`oox/source/ppt/pptshapecontext.cxx:186`) and EditEngine then makes the ascent one em and
      the line 1.2 em (`editeng/source/editeng/impedit3.cxx:501,3138`). That last is 1.7 pt on
      every line of every shape in Liberation Sans, and no word-processing comparison could have
      found it — Writer never sets the flag.
- [ ] The rest of slide rendering: pictures, tables, gradients and the other non-solid fills —
      **the backends now draw all three, so what is left is the reading**, and the two mappings
      worth not rediscovering are recorded in `src/Paperless.Rendering/TODO.md`: ODF's
      `draw:start-color` on a radial gradient paints the *outer* edge, and a radial's outer
      radius is half the shape's **diagonal** rather than half its width —
      shadows and the other effects, bullet and number markers, the preset-geometry evaluator
      beyond the six shapes transcribed by hand, `p:style`'s style-matrix references, and the
      rung of the inheritance chain that gives an unstated run its size, typeface and colour.
      Each is written up with its citation and with what would settle it in
      `src/Paperless.Presentations/TODO.md`. **PPT (binary) does not lay out at all**: its shape
      tree reads, but a placeholder's rectangle comes from `SlideAtom`'s eight layout placeholder
      ids and nothing reads those yet.
- [x] **Gradients and bitmap fills, in both backends.** A linear gradient is a PDF axial
      shading (`/ShadingType 2`) and a Skia linear shader; a radial or elliptical one is
      `/ShadingType 3` and a radial shader, the ellipse's squash carried by the gradient's own
      transform. The ramp is one exponential function per pair of stops, stitched with a
      `/FunctionType 3` beyond two — which is what `a:gsLst` and ODF 1.3's
      `loext:gradient-stop` can state and a two-colour attribute cannot. A gradient whose stops
      fade gets a luminosity soft mask, since a shading has no alpha channel and Skia's shader
      does, and one display list must not give two pictures. Conical and rectangular have a
      native form in neither backend, so both expand them through **one shared band
      decomposition** ported from `Gradient::AddGradientActions`. A tiled bitmap is one image
      draw per tile inside a clip in PDF and a repeating image shader in Skia, both laid on the
      same grid.
      **The two sides do not state a gradient the same way and cannot be made to**, so the
      comparison is picture for picture rather than operator for operator: Impress decomposes
      every shape gradient into flat bands before its PDF writer sees it — `"tdf#150551 for PDF
      export, use the decomposition"`, `vclmetafileprocessor2d.cxx` — so its shading writer is
      unreachable from a slide and its PDF of `tests/corpus/features/paint-fills.fodp` holds no
      shading at all, in 91602 bytes of page-one content stream against our 2570. Measured at
      150 dpi, per channel: our raster against its rendering, **0.0016**; our PDF against its
      PDF read by the same rasteriser, **0.0007**, and **0.0000** — identical, pixel for pixel —
      on the picture page.
- [x] **Raster image decode (via Skia).** `RasterImageDecoder` in `Paperless.Rendering` turns
      encoded bytes into the display list's `RasterImage`, sniffing PNG, JPEG, GIF, BMP, WebP and
      ICO **by content** so a mislabelled part still passes a JPEG through to `DCTDecode`. It is
      public and lives beside the backends because `Paperless.Core` must stay dependency-free and
      Skia already carries the codecs LibreOffice reaches for (`research/06-rendering.md` §D.1).
      **A family library that wants it needs a `ProjectReference` on `Paperless.Rendering` that
      none of them has yet** — one line each, and the reason `p:pic`, `w:drawing` and a sheet's
      logo are still unpainted.
- [ ] Vector import: full WMF, EMF, EMF+ and SVG. The largest single body of work here and
      no C# prior art — start it early rather than treating it as a tail-end detail.
- [x] **PDF output.** Hand-rolled: objects, a classic `xref`, deflated content streams, and simple
      `/TrueType` fonts subsetted through `hb-subset` — bound directly to the native library, because
      HarfBuzzSharp's managed binding does not expose the subsetter (verified against 14.2.1.1). Text
      is real text: glyph ids as one-byte codes with a `ToUnicode` CMap built from the cluster map,
      which is why the display list keeps glyph runs rather than outlines. `pdftotext` gets the same
      words out of our PDF as out of LibreOffice's.
      **And it is checked the way this project checks everything else.** `PdfTextRuns`, `PdfFills`,
      `PdfStrokes` and `PdfPageSizes` are pointed at *ours* as well as LibreOffice's and the two
      compared operator for operator, over ten documents in four formats: line counts match exactly,
      font sizes match exactly, baseline pitch agrees to **0.000 pt**, and the pen differs by exactly
      the **two twips** LibreOffice's export adds — a deviation that was previously inferred and is
      now read straight off both content streams. Table borders match stroke for stroke and pen width
      for pen width; shades and note separators fill for fill; sheet sizes page for page.
      Detail, and the four places our file deliberately differs, in `src/Paperless.Rendering/TODO.md`.
- [x] **Raster output — PNG and JPEG**, at a caller-chosen DPI, from the Skia backend. PNG is the one
      that matters: lossless and deterministic, so a page written twice is byte-identical and a golden
      image can be committed. JPEG is for thumbnails and a test pins why it must not be compared — a
      flat fill does not survive the round trip.
      Verified against `--convert-to pdf` rasterised by `pdftoppm -r 150`, which is the route that
      gives an exact DPI; the direct `writer_png_Export` gives LibreOffice's own idea of a page image
      at its own scale (measured, 795 x 1124 for A4) and is the wrong reference when the comparison
      needs a known one. Two measurements came out of it and both are in
      `src/Paperless.Rendering/TODO.md`: **the pixel size is the ceiling** of the page size, because
      poppler takes the ceiling and one pixel of disagreement stops the comparison dead; and **glyphs
      are drawn from their outlines**, because Skia's glyph cache places a mask at a whole pixel
      vertically and that alone moved a page's ink a quarter of a point and made the comparison script
      report a reflow cascade on a page whose layout was exact.
- [ ] SVG writer.
- [x] `paperless render` to PDF, PNG or JPEG, with `--format`, `--pages` and `--dpi`. Its output
      layout is part of the `render-comparison` skill's contract: one input writes `page-1.png`,
      `page-2.png` … straight into `--outdir`, the same names `pdftoppm` gives the reference; several
      inputs each get a subdirectory named after the file, as `lo-convert.sh` does.
      SVG output and `paperless convert` are still to come.

## Phase 4 — Fidelity

- [ ] Run the whole corpus through the comparison harness; triage by failure signature
      (cascade vs missing vs localised), not by raw difference percentage.
- [ ] Golden reference images committed with the LibreOffice version that produced them.
      Rasterised output is byte-deterministic, so checksums are meaningful.
- [ ] Performance: extraction should be well under a second for typical files.
- [ ] Fuzz the readers. Malformed office files are a classic attack surface and the readers
      are pure parsing code over untrusted input. `OdfLenientReadTests` covers the cases a
      fuzzer finds first — a truncated part, a malformed part, a missing part, an XXE, an
      absurd repeat count, a cyclic style chain, nesting deep enough to overflow a stack —
      but that is a floor, not a substitute for actual fuzzing.
- [ ] Public API review, then a first release.

---

## Known deviations from LibreOffice

Recorded rather than reproduced. Where LibreOffice renders a document wrongly, copying the
defect is explicitly a non-goal; each entry here is a place the comparison harness allows a
difference on purpose, with the evidence that Paperless is the one following the format.

Per-library lists, each with its measurements: layout's in
`src/Paperless.WordProcessing/TODO.md` (the two twips LibreOffice's PDF export adds to every
pen position, the 0.15% it measures a line wider than HarfBuzz, and the twip its font ascents
differ by), and output's in `src/Paperless.Rendering/TODO.md` (real rather than integer
`/Widths`, layout tables dropped from embedded subsets, no tagged-PDF structure, and the blank
at the end of a wrapped line not drawn).

**Footnote numbering with a section-level `w:footnotePr`.** In
`tests/corpus/features/word-features.docx`, LibreOffice 24.2.7.2 numbers the single footnote 0;
Paperless numbers it 1. ECMA-376 §17.11.17 puts the default `w:numStart` at 1, and the quirk is
LibreOffice's: removing the section-level `w:footnotePr` from the same file makes LibreOffice
render 1, and on a minimal DOCX with one footnote and no footnote properties at all the two
agree on 1. Allowed by name in `ExtractionComparisonTests.KnownDeviations`.

## Settled decisions

Recorded here because each one closes off alternatives that would otherwise look tempting
later.

**Line breaking: hand-roll UAX #14.** The runtime has no equivalent — verified by
reflecting over .NET 10's shipped assemblies. `StringInfo`/`TextElementEnumerator` give
grapheme clusters (UAX #29), not line-break opportunities, and `CharUnicodeInfo` exposes
only the general category, not the `Line_Break` or `East_Asian_Width` properties UAX #14
needs. .NET's globalization is ICU-backed on Linux but surfaces only collation, casing,
normalisation and calendars, so ICU's `BreakIterator` is out of reach. That left `ICU4N`
(prerelease-only) or a native binding; hand-rolling avoids both. Generate the property
tables from `LineBreak.txt` and `EastAsianWidth.txt`, implement LB1–LB31, and verify
against Unicode's `LineBreakTest.txt`. South East Asian scripts (Thai, Lao, Khmer,
Burmese) need dictionary-based breaking that rules alone cannot do; a dictionary built from
ICU's word lists ships as an embedded resource, so those breaks agree with LibreOffice's
rather than merely being defensible. Detail in `src/Paperless.Text/TODO.md`.

**Vector graphics: full support for WMF, EMF, EMF+ and SVG.** No subset, no
rasterise-via-LibreOffice shortcut. **SVG reuses `Svg.SceneGraph`/`Svg.Model`/`ShimSkiaSharp`**
— taken without `Svg.Skia` itself, that combination pulls in no SkiaSharp, stays
resolution-independent, and exposes hooks to shape SVG text through our own font stack, so
hand-rolling it would cost a CSS cascade and a filter engine for no fidelity gain. WMF, EMF
and EMF+ remain ours to write. This is still the largest single body of work in the project
and there is no C# prior art for EMF/EMF+, so plan the timeline around it rather than
treating it as a tail-end detail. Port from LibreOffice's `emfio/` and `svgio/` rather than
working from the specifications. Scope and ordering in `src/Paperless.Vector/TODO.md`.
Scripting, animation and external references stay excluded permanently — that is a security
decision, not a scope compromise.

**Structured text output is GitHub-Flavored Markdown, not CommonMark.** Forced rather than
chosen: CommonMark has no tables at all, and a projection of an office document that cannot
express a table is not worth having. The extensions used are pipe tables and strikethrough,
and nothing else — deliberately not Markdig's `UseAdvancedExtensions` in the tests either,
since that turns on syntax GitHub does not implement and a test passing because of an exotic
extension would be measuring the wrong thing. What GFM still cannot carry is named where it is
dropped: underline, superscript and subscript keep their text and lose their formatting, and an
ordered list's marker style collapses to decimal while its start number survives.

**A table GFM cannot express falls back to raw HTML; it is not flattened.** GFM tables have no
row or column spans, no nesting, and exactly one line per cell, and the corpus is full of all
three. An HTML `<table>` is valid Markdown — a raw HTML block — and stage one has already built
exactly the tree to serialise, so nothing is lost and nothing is guessed. **What it costs:** a
fallback table is not Markdown, so a consumer that reads Markdown as text, or strips HTML rather
than parsing it, sees tags or sees nothing. The alternative was flattening — repeating a spanned
cell across the columns it covers, splicing a nested table's rows into its parent — which invents
a grid the document does not have and is *silently* wrong where this is *visibly* HTML. A table
goes to the fallback when any cell spans, when any cell holds a nested table, when a cell holds
more than one block or a list, or when the rows are ragged. One thing GFM forces even in the good
case: it has no headerless table, so a table declaring no header rows has its first row promoted,
which is what a sheet and nearly every document table put labels in anyway.

**Footnotes are not GFM's `[^1]`, and that reverses the plan on evidence.** Two things have to
hold for `[^1]` to work and neither does. A definition has to be *referenced* — measured with
Markdig 1.3.2, a document whose only footnote definition is unreferenced renders as
`<div class="footnotes"><hr/><ol></ol></div>`, with the note's text nowhere in the output. And a
reference cannot be placed, because the content tree carries no inline anchor: every reader bakes
the anchor number into the text of the run beside it, so finding the spot would mean matching text
rather than reading structure. So a note is set apart the way a comment, a header and a speaker
note are — a block quote labelled with the number the document itself rendered. That loses the
link between anchor and note, which the tree does not have either, and keeps the text, which
`[^1]` would not. The measurement is pinned as a test, so if a parser starts rendering unreferenced
definitions the decision is worth revisiting and the suite will say so.

**A heading wins over a list level.** Word attaches its heading styles to an outline list, so in
the DOC and DOCX of a document whose ODT has plain headings, every heading arrives carrying
`ListLevel` 0 and an empty marker. Treated as a list item it comes out as `- # Top level heading`,
a bullet wrapping a heading, in every Word file in the corpus. LibreOffice's own XHTML export
*does* wrap those headings in `<li>`, so this is a deliberate difference from the reference rather
than a defect being fixed: the outline number is decorative, and emitting the DOCX and the ODT of
the same document differently would be worse than disagreeing with the export.

**The XHTML is indented by an explicit pass, not by `XmlWriter`.** The trap that cost the most
time here, and it looks like nothing. `XmlWriterSettings.Indent` decides progressively, so it
indents before the first child of `<p><strong>a</strong> tail</p>` — it has not yet seen the text
that makes the content mixed. Worse, an element whose content is *only* inline elements,
`<p><strong>a</strong><em>b</em></p>`, is element-only by that test, and indenting it inserts a
space between "a" and "b" that the document does not contain. So indentation is applied to a named
set of block containers where whitespace between children is insignificant, and to nothing else.
Two smaller ones in the same area: `XmlWriter` over a `StringBuilder` reports the encoding as
`utf-16` whatever the settings say, so the prologue is written by hand; and an empty non-void
element must be written `<p></p>`, because an HTML parser reads `<p/>` as an *opening* tag and
swallows the rest of the document — which an empty table cell produces.

**OLE2/CFB is hand-rolled, not OpenMcdf.** Tolerance of malformed real-world files was the
deciding requirement, and the reader needs control over chain-walking and directory
traversal that a library does not expose. The `OpenMcdf` dependency has been dropped.

**Rasterise with SkiaSharp, shape with HarfBuzzSharp, read font metrics ourselves.**
HarfBuzz is what LibreOffice shapes with, so advance widths agree by construction. Metrics
come from a hand-rolled OpenType table reader because matching LibreOffice's line heights
needs raw `hhea`/`OS/2` access and its own precedence rules.

## Open questions

Each of these should be resolved with a spike, not by guessing.

1. **Formula recalculation.** Trust cached results, or recalculate? Cached matches what a
   reference renderer shows; recalculating is correct when the cache is stale. Currently a
   `LayoutOptions` flag, defaulting to trusting the cache. Confirm that is right.
2. **SmartArt / DrawingML diagrams.** Files carry a pre-rendered fallback. Use it, or
   implement layout? The fallback is far cheaper and probably sufficient.
3. **Charts.** Out of scope as a standalone application, but charts are embedded in real
   documents constantly. Render them, or draw a placeholder? Needs a decision.

## Non-goals

Writing/export of any format. Macro execution. Editing. Draw, Math, Base. A UI.
Bug-for-bug reproduction of LibreOffice's own import defects — where LibreOffice renders a
document wrongly, record a known deviation rather than copying the bug.
