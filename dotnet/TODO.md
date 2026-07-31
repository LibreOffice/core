# Paperless — master plan

Status: **Phase 0 complete; Phase 1 all but the OOXML spreadsheet and presentation formats;
Phase 2 complete for word processing.** Every word-processing format reads and *lays out* —
`odt ott fodt`, `docx docm dotx dotm`, `doc dot` and `rtf` — with pages, headers and footers,
tables, sections and columns all verified against LibreOffice's own rendering to a tenth of a
point. The ODF spreadsheet and presentation formats extract. `xlsx`, `pptx`, the legacy `xls`
and `ppt`, and CSV do not read at all yet.

Each library has its own `TODO.md` with detail; this file is the ordering and the reasoning
behind it.

## Start here (next session)

Two fronts, and they are independent.

**Word processing** is deep into Phase 3's layout half. Footnote placement is done — including
the feedback loop into pagination, since the note area takes its room out of the body's, so
adding a note can push the line that cites it onto the next page and thereby remove the note
again — and notes are read from all four formats. What remains is floating frames with text wrap,
cell borders and shading, and note numbering *restarts*. Read
`src/Paperless.WordProcessing/TODO.md`, whose open items each say what is missing and why. One
warning about borders: they cannot be verified the way everything else in this library has
been, because a word-position comparison cannot see them and `Paperless.Rendering`'s rasteriser
is still a stub. That makes the rasteriser the thing to build before the borders, not after.

**The formats that do not read at all** are `xlsx`, `pptx`, `xls`, `ppt` and CSV. The
spreadsheet pair is the larger prize, since `ods` already extracts and `Paperless.Spreadsheets`
has the model.

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
| ✅ | Solution: 12 libraries + CLI + 9 test projects, warning-free on .NET 10, 280 tests passing |
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

| Not started | |
|---|---|
| ❌ | `xlsx`/`pptx`, `xls`/`ppt` and CSV readers |
| ❌ | Decryption (detection works; decryption does not) |
| ❌ | Rendering backends: `Paperless.Rendering`'s rasteriser and PDF writer are stubs |
| ❌ | Floating frames, cell borders and shading |
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
      instead.
- [ ] Bidi and script runs; vertical text.
- [x] Paragraph layout: alignment, justification, tabs, indents, spacing, line spacing.

**Exit criterion — met for word processing.** For a text-heavy document Paperless breaks lines
where LibreOffice breaks them, in all four formats, which is what made every page-level
comparison after it meaningful. It was worth as much as expected: nearly every bug found since
was found because a page comparison put a word a measurable distance from where it belonged.

## Phase 3 — Rendering

- [ ] Skia raster backend consuming `IDrawingSink`. **The next thing to build**, and not only
      for its own sake: cell borders, shading and floating-frame outlines cannot be verified
      against LibreOffice without it, because a word-position comparison cannot see them.
- [x] Word-processing page layout: pagination, headers and footers, tables as grids that split
      across pages with repeating heading rows, several sections per document, and columns.
- [x] Footnote placement, which feeds back into pagination rather than merely adding a note: the
      note area takes its room out of the body's, so a page with notes holds less text, and the
      loop shortens the page until it holds. Endnotes instead take pages of their own after the
      last body page, numbered i, ii, iii rather than 1, 2, 3. Notes read from all four formats.
- [ ] The rest of it: floating frames with text wrap, and cell borders and shading.
- [ ] Spreadsheet print layout — `ScPrintFunc`'s pagination is the routine to port
      faithfully. A spreadsheet has **no intrinsic pagination**: print settings *are* its
      page geometry.
- [ ] Slide rendering: shapes, the placeholder/theme inheritance chain, preset geometries,
      text bodies with autofit.
- [ ] Raster image decode (via Skia).
- [ ] Vector import: full WMF, EMF, EMF+ and SVG. The largest single body of work here and
      no C# prior art — start it early rather than treating it as a tail-end detail.
- [ ] PDF writer with subset font embedding (`hb-subset` via HarfBuzzSharp).
- [ ] SVG writer.
- [ ] `paperless render` and `paperless convert`.

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
