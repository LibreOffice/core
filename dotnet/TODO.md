# Paperless — master plan

Status: **skeleton only.** Every project builds warning-free and the test suite runs, but
no format is readable yet. Each library has its own `TODO.md` with detail; this file is the
ordering and the reasoning behind it.

## Where things stand

| Done | |
|---|---|
| ✅ | Solution, 13 libraries + CLI + 8 test projects, building warning-free on .NET 10 |
| ✅ | `Paperless.Core` API surface: units, geometry, colour, format catalogue, document model, drawing IR |
| ✅ | Dependency layering fixed and enforced by project references |
| ✅ | Six research documents (~6000 lines) covering the LibreOffice implementation |
| ✅ | Four comparison skills with verified working scripts |
| ✅ | Dependency licensing audited — Six Labors packages ruled out (they fail the build without a paid key) |

| Not started | |
|---|---|
| ❌ | Every format reader |
| ❌ | Layout engines |
| ❌ | Rendering backends |
| ❌ | The CLI's actual subcommands |

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

- [ ] **Format identification** (`Paperless.Core`, `Paperless.Containers`).
      Implement `IFormatIdentifier` and the `IFormatCatalogue` from the table in
      `research/01-formats-and-detection.md`. Content-based, extension only as a
      tie-breaker.
- [ ] **OLE2 / CFB reader** (`Paperless.Containers`). Byte layouts are in
      `research/05-infrastructure.md` section A. Must tolerate malformed files —
      wrong CLSIDs, overlong FAT chains, inconsistent directory trees are all common.
      Decide then whether `OpenMcdf` suffices or a hand-rolled Span-based reader is needed.
- [ ] **ZIP + OPC + ODF packages** (`Paperless.Containers`). `System.IO.Compression` for
      the ZIP layer; hand-roll content-type resolution and relationships.
- [ ] **`paperless identify`** end to end. First externally visible behaviour, and it makes
      everything after it debuggable.
- [ ] **Corpus and fidelity harness wiring**. Commit `tests/corpus/minimal/`, implement
      `LibreOfficeRunner`, get one comparison running end to end even though it fails.

## Phase 1 — Extraction, all formats

Per format: metadata, then text, then tables/structure.

- [ ] ODF text/spreadsheet/presentation (`odt ods odp` + template and flat variants).
      **Start here**: XML, well-specified, and it exercises the shared style-resolution
      machinery that OOXML also needs.
- [ ] OOXML (`docx xlsx pptx` + variants). Shared theme/relationship handling in
      `Paperless.Ooxml` first.
- [ ] Legacy binary (`doc xls ppt`). Hardest. Needs the record-stream reader, sprm/BIFF
      decoding, the WW8 piece table, and codepage handling.
- [ ] RTF, CSV.
- [ ] `xlsb` (import only — LibreOffice cannot write it, so test files need Excel).
- [ ] Encrypted documents, one scheme at a time
      (`research/05-infrastructure.md` section C).
- [ ] `paperless extract` and `paperless metadata`.

**Exit criterion:** extraction matches LibreOffice's text export across the whole corpus,
allowing for the reference filters' known omissions (headers, comments, notes, shape text
— where extracting *more* is correct).

## Phase 2 — Text layout

The part that decides whether rendering can ever match.

- [ ] **Font resolution and metrics** (`Paperless.Text`). Hand-rolled OpenType table
      reader — we need raw `hhea`/`OS/2` access and our own precedence rules, per
      `research/06-rendering.md` section B, not a library's opinion of them.
      Reproduce LibreOffice's substitution order. **Report substitutions**: a silent one
      explains most mysterious reflows.
- [ ] **Shaping** via HarfBuzzSharp — same engine LibreOffice uses, so advances agree.
- [ ] **Line breaking** (UAX #14). LibreOffice uses ICU's rules, so matching its breaks
      means matching ICU. Evaluate `ICU4N` (currently prerelease-only) against
      hand-rolling.
- [ ] Bidi and script runs; vertical text.
- [ ] Paragraph layout: alignment, justification, tabs, indents, spacing, line spacing.

**Exit criterion:** for a text-heavy document, Paperless breaks lines at the same places as
LibreOffice. Until then, page-level comparison is meaningless — every page after the first
will differ for reasons that have nothing to do with drawing.

## Phase 3 — Rendering

- [ ] Skia raster backend consuming `IDrawingSink`.
- [ ] Word-processing page layout: pagination, headers/footers, footnotes, tables spanning
      pages, floating frames and text wrap.
- [ ] Spreadsheet print layout — `ScPrintFunc`'s pagination is the routine to port
      faithfully. A spreadsheet has **no intrinsic pagination**: print settings *are* its
      page geometry.
- [ ] Slide rendering: shapes, the placeholder/theme inheritance chain, preset geometries,
      text bodies with autofit.
- [ ] Raster image decode (via Skia); vector import (EMF/WMF/EMF+ — the highest-risk item
      in the project, no C# prior art).
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
      are pure parsing code over untrusted input.
- [ ] Public API review, then a first release.

---

## Open questions

Each of these should be resolved with a spike, not by guessing.

1. **Line breaking.** `ICU4N` has no stable release. Take the prerelease, hand-roll
   UAX #14, or bind ICU natively? Affects every layout decision downstream, so decide
   early.
2. **EMF+.** No C# library. Hand-roll (large), skip vector fidelity and rasterise via
   LibreOffice (defeats the purpose), or render only the embedded EMF fallback that many
   files also carry? Spike this before committing to a rendering timeline.
3. **`OpenMcdf` vs hand-rolled CFB.** Depends entirely on how it behaves on malformed
   input. Test against LibreOffice's own corpus in `sw/qa/`, `sc/qa/`, `sd/qa/`.
4. **Formula recalculation.** Trust cached results, or recalculate? Cached matches what a
   reference renderer shows; recalculating is correct when the cache is stale. Currently a
   `LayoutOptions` flag, defaulting to trusting the cache. Confirm that is right.
5. **SmartArt / DrawingML diagrams.** Files carry a pre-rendered fallback. Use it, or
   implement layout? The fallback is far cheaper and probably sufficient.
6. **Charts.** Out of scope as a standalone application, but charts are embedded in real
   documents constantly. Render them, or draw a placeholder? Needs a decision.

## Non-goals

Writing/export of any format. Macro execution. Editing. Draw, Math, Base. A UI.
Bug-for-bug reproduction of LibreOffice's own import defects — where LibreOffice renders a
document wrongly, record a known deviation rather than copying the bug.
