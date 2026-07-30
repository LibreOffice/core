# Paperless — master plan

Status: **skeleton only.** Every project builds warning-free and the test suite runs, but
no format is readable yet. Each library has its own `TODO.md` with detail; this file is the
ordering and the reasoning behind it.

## Where things stand

| Done | |
|---|---|
| ✅ | Solution: 12 libraries + CLI + 8 test projects, building warning-free on .NET 10 |
| ✅ | `Paperless.Core` API surface: units, geometry, colour, format catalogue, document model, drawing IR |
| ✅ | Dependency layering fixed and enforced by project references |
| ✅ | Six research documents (~6000 lines) covering the LibreOffice implementation |
| ✅ | Four comparison skills with verified working scripts |
| ✅ | Dependencies audited: all permissive, none gated behind a build-time licence check |

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

- [x] **Format identification** (`Paperless.Core`, `Paperless.Containers`). Done:
      `FormatCatalogue` covers all 43 formats; `FormatIdentifier` detects by content via
      ODF `mimetype`, OOXML main-part content type (resolved through `_rels/.rels`), OLE2
      root stream names, and text/XML signatures. Verified against all 17 corpus formats,
      including with deliberately wrong extensions and with no file name at all.
- [x] **OLE2 / CFB reader** (`Paperless.Containers`). Done, hand-rolled: header, DIFAT/FAT
      chains, mini-FAT and mini-stream, directory walk with cycle guard. Tolerates truncated
      files, out-of-range sectors, cyclic chains and duplicate entries, reporting each as a
      `Diagnostic`. `OpenMcdf` was not needed and its package reference should be dropped
      once nothing else wants it.
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
- [ ] **Line breaking** (UAX #14), hand-rolled — nothing in the runtime does this. Generate
      the `Line_Break` and `East_Asian_Width` tables, implement LB1-LB31, verify against
      Unicode's `LineBreakTest.txt`. LibreOffice's breaks are ICU's, so expect small
      tailoring differences to surface later.
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
      are pure parsing code over untrusted input.
- [ ] Public API review, then a first release.

---

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

**Rasterise with SkiaSharp, shape with HarfBuzzSharp, read font metrics ourselves.**
HarfBuzz is what LibreOffice shapes with, so advance widths agree by construction. Metrics
come from a hand-rolled OpenType table reader because matching LibreOffice's line heights
needs raw `hhea`/`OS/2` access and its own precedence rules.

## Open questions

Each of these should be resolved with a spike, not by guessing.

1. **`OpenMcdf` vs hand-rolled CFB.** Depends entirely on how it behaves on malformed
   input. Test against LibreOffice's own corpus in `sw/qa/`, `sc/qa/`, `sd/qa/`.
2. **Formula recalculation.** Trust cached results, or recalculate? Cached matches what a
   reference renderer shows; recalculating is correct when the cache is stale. Currently a
   `LayoutOptions` flag, defaulting to trusting the cache. Confirm that is right.
3. **SmartArt / DrawingML diagrams.** Files carry a pre-rendered fallback. Use it, or
   implement layout? The fallback is far cheaper and probably sufficient.
4. **Charts.** Out of scope as a standalone application, but charts are embedded in real
   documents constantly. Render them, or draw a placeholder? Needs a decision.

## Non-goals

Writing/export of any format. Macro execution. Editing. Draw, Math, Base. A UI.
Bug-for-bug reproduction of LibreOffice's own import defects — where LibreOffice renders a
document wrongly, record a known deviation rather than copying the bug.
