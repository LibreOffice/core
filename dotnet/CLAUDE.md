# Paperless — working notes

Paperless is a **pure C# / .NET** library set for content extraction and headless
rendering of the file formats LibreOffice's Writer, Calc and Impress support.

It lives in the `dotnet/` subdirectory of a LibreOffice source checkout. The surrounding
C++ tree is **reference material, not a build dependency** — we read it to learn how the
formats behave, and we run an installed `soffice` to generate ground truth.

## Scope

**In scope.** Word processing (`docx docm dotx dotm doc dot rtf odt ott fodt`),
spreadsheets (`xlsx xlsm xltx xltm xlsb xls xlt ods ots fods csv`), presentations
(`pptx pptm potx potm ppsx ppsm ppt pot pps odp otp fodp`), plus the legacy
OpenOffice.org 1.x forms (`sxw sxc sxi`).

**Out of scope.** Draw, Math and Base. Do not add them. Also: writing/export of any
format (Paperless reads), macro execution (never — Paperless only reports that macros are
*present*), and editing.

## Absolute rules

1. **Never build the C++ tree.** It takes hours and is never needed. Use an installed
   `soffice` for reference output — see the `libreoffice-reference` skill.
2. **Never execute macros.** Macro-enabled formats are read as data. `CanCarryMacros` on
   `FormatInfo` exists so callers can surface the risk; nothing executes.
3. **Rasterise with SkiaSharp, shape with HarfBuzzSharp.** HarfBuzz is what LibreOffice
   shapes with, so advance widths agree by construction. Font metrics come from a
   hand-rolled OpenType reader in `Paperless.Text` — matching LibreOffice's line heights
   needs raw `hhea`/`OS/2` access and our own precedence rules. Before adding any graphics
   dependency, read the note at the top of `Directory.Packages.props`.
4. **Detect formats by content, never by extension.** Mislabelled files are common, and
   some distinctions (DOCX vs DOCM, which application owns an OLE2 file) cannot be made
   from a name at all. The extension is a tie-breaker hint only.
5. **Be lenient when reading.** Real files violate their own specifications constantly.
   Repair what you can, skip what you cannot, record it as a `Diagnostic`. Reserve
   exceptions for genuinely unreadable input.
6. **Zero build warnings.** `TreatWarningsAsErrors` is on solution-wide. Keep it that way.

## Layout

```
dotnet/
  Directory.Build.props        shared MSBuild settings; read the licensing note
  Directory.Packages.props     central package versions
  Paperless.slnx               solution (the newer XML format; dotnet 10 default)
  research/                    in-depth notes on the LibreOffice implementation
  src/                         the libraries
  tools/Paperless.Cli          the `paperless` command-line tool
  tests/                       unit tests, the test kit, and the fidelity harness
```

### Dependency layering

Arrows point at dependencies. Nothing may point back up.

```
                       Paperless.Core          (zero external dependencies)
                            |
      +---------------+-----+------+---------------+-------------+
      |               |            |               |             |
 Containers        Text          Vector        Rendering       Markup
 (OLE2/OPC/ODF)  (fonts,        (EMF/WMF/SVG)  (Skia, PDF,   (XHTML and
                  shaping,                      SVG)          Markdown out)
                  layout)
      |               |            |
      +-------+-------+------------+
              |
    +---------+----------+-------------+
    |                    |             |
  Ooxml            OpenDocument     MsBinary      (shared per-family infrastructure)
    |                    |             |
    +---------+----------+-------------+
              |
   +----------+-----------+--------------+
   |                      |              |
 WordProcessing      Spreadsheets    Presentations
   |                      |              |
   +----------+-----------+--------------+
              |
          Paperless          (facade: sniff and dispatch)
              |
        Paperless.Cli
```

**`Paperless.Markup` serves all three families, so it cannot live in any of them.** It projects
the shared `ContentNode` tree onto semantic XHTML and then onto Markdown, needs nothing but
`Paperless.Core`, and sits beside the other Core-only libraries rather than inside Core, which
holds the abstractions everything agrees on rather than projections of them.

**`Paperless.Core` has no external dependencies and must stay that way.** It holds the
abstractions everything else agrees on: units, geometry, colour, the format catalogue, the
document model, and the drawing IR. A dependency added here is inherited by every
consumer.

`Core/Charts` is the test of that rule and shows where the line falls. A chart's *model* and its
*layout* — `ChartPlot`, `ChartScale`, `ChartLayout` — are geometry over the abstractions Core
already holds, so they belong here; the readers that turn a `c:chartSpace` or a `chart:chart` into
that model parse XML and stay in `Paperless.Ooxml` and `Paperless.OpenDocument`. Putting the model
one layer up instead is what forced the ODF reader into `Paperless.Presentations`, where a
spreadsheet could not reach it.

`Core/Numbers` came down for the same reason and by the same test, and it is worth stating as a
rule rather than as a second exception. **The question is not "who uses it" but "what does it
depend on".** The number-format engine — parsing `#,##0.00` and rendering a double through it —
began in `Paperless.Spreadsheets` because a cell is what wanted it, and a chart's axis composed in
`Core/Charts` then could not reach it; every tick was written in its shortest round-trip form, which
is right for a whole-number scale and wrong for every currency, percentage and date axis. The move
was safe because the engine is pure computation over a string: its five files import
`System.Globalization` and `System.Text` and nothing else, so Core's zero-dependency rule is intact.
Read it as: **a thing belongs in Core when it depends on nothing above Core, whatever it was written
for.** What did *not* move is the reading — `XlsxStyles`, `OdsCellFormats` and
`OdfNumberFormat` parse markup and stay in their own libraries, the last of them compiling an ODF
`number:*-style` element tree into a format code exactly as `xmloff` does before handing it to one
formatter.

## Key design decisions, and why

**All lengths are EMUs, in a `Length` struct.** 914400 per inch divides evenly by twips
(the DOC/DOCX/RTF unit), 1/100 mm (the ODF and draw-layer unit), and points. Storing a
single exact integer avoids the rounding drift that accumulates when converting through
`double` at every boundary.

**Extraction and rendering are separate paths.** `IDocument` gives you content;
`IPaginatedDocument.Layout()` is a distinct, deferred step. Extraction is the common case
and must not pay for fonts, layout or a rasteriser — it costs a small fraction of
rendering.

**One drawing IR, `IDrawingSink`.** Modelled on LibreOffice's `GDIMetaFile`/`MetaAction`
display list and its `drawinglayer` primitives, because those are the two chokepoints all
LibreOffice output passes through — so anything a supported document can express fits
through them. Coordinates stay resolution-independent; text stays glyph runs rather than
outlines so PDF output can be real searchable text.

**One content tree for all three families.** Callers indexing a mixed corpus want text,
tables and structure without branching on whether a file was a deck or a spreadsheet.

**Shared infrastructure is factored by what the formats actually share**, not by
tidiness: Escher/MS-ODRAW is one library because DOC, XLS and PPT all delegate their
drawings to it, so implementing it once buys shapes in all three.

## Fidelity: the thing that will bite you

**Rendering errors cascade.** One wrong measurement — a font metric, a margin, a line
break — shifts everything after it, so a single bug manufactures hundreds of unrelated-
looking failures across a corpus. Fix cascades before anything else; they are cheap to fix
and expensive to work around.

The three highest-risk areas, in order:

1. **Font resolution and metrics.** A substitution that is not metric-compatible changes
   advance widths, hence line breaks, hence pagination. The machine must have Carlito and
   Caladea installed (`fc-match Calibri` → `Carlito`) or every OOXML comparison is
   meaningless. Line height derivation from hhea vs OS/2 metrics has specific precedence
   rules — see `research/06-rendering.md` section B.
2. **DrawingML theme colour resolution.** Get the `lumMod`/`shade`/`tint` chain wrong and
   every themed shape on every slide is the wrong colour at once.
3. **Vector import (WMF/EMF/EMF+).** Full support is committed and there is no C# prior
   art — roughly fifty EMF+ record types alone. Real `.pptx` and `.docx` files embed these
   constantly, so this is the largest single body of work in the project rather than a
   tail-end detail. Port from LibreOffice's `emfio/`. SVG is the exception: it reuses
   `Svg.SceneGraph`/`Svg.Model`, translated from `ShimSkiaSharp`'s command list into
   `IDrawingSink`.

## Workflow

```bash
cd dotnet
dotnet build Paperless.slnx          # must stay warning-free
dotnet test  Paperless.slnx          # ~1100 tests, a few minutes
```

**Do not add `-r`/`--runtime`.** The SDK rejects it on a solution outright —
`NETSDK1134: Building a solution with a specific RuntimeIdentifier is not supported` — and it
is unnecessary: `Directory.Build.props` already pins every test and tool project to the host
RID, computed from the OS and process architecture. Passing `-r linux-x64` to an individual
project is accepted and does nothing, which is the intended state. Read the comment beside the
setting before changing it; it records two traps that both look exactly like the property
having no effect.

That pin is not a tidiness measure. Without it the build resolves SkiaSharp's and
HarfBuzzSharp's native binaries for **twenty-one** runtime identifiers and copies all of them
into every output directory — 687 MB per test project, of which the host can run one. A clean
whole-solution build costs **463 MB with the pin and 6095 MB without it**, which is the
difference between fitting in a container's disk allowance and exhausting it.

### Running less than everything

A full run rebuilds nothing if the tree is already built, so the cost is the tests themselves —
and **essentially all of it is `Paperless.Fidelity.Tests`**, which shells out to `soffice` once
per document. It is also the *only* project that does: the other seven reach LibreOffice not at
all, so they need none of the setup below and finish in seconds.

| Project | Needs `soffice` | Rough cost |
|---|---|---|
| `Paperless.Fidelity.Tests` | yes, 23 files | minutes |
| everything else | no | seconds |

Those are wall-clock figures on an already-built tree; most of each is the SDK's up-to-date
check rather than the tests, which is why naming one project is worth doing but naming one
*test* rarely is.

So when iterating, name the project — and reach for the filter only inside the slow one:

```bash
dotnet test tests/Paperless.Text.Tests/Paperless.Text.Tests.csproj                # ~10 s
dotnet test tests/Paperless.WordProcessing.Tests/Paperless.WordProcessing.Tests.csproj   # ~15 s
dotnet test tests/Paperless.Fidelity.Tests/Paperless.Fidelity.Tests.csproj \
    --filter "FullyQualifiedName~TableComparisonTests"                            # ~45 s
```

Run every project before committing anyway. The failure this project cares about most is the
cascade — one wrong measurement moving every line after it — and it surfaces in projects you had
no reason to think you had touched.

### A truncated run reports success

**Check the count, not just the colour.** Under heavy load the test host can die part-way and
still print `Passed! - Failed: 0`, having silently dropped the tests it never reached. Measured
on one commit with several parallel builds running: the fidelity project reported **470 passed**
on one run and **353 passed** on the next, both `Failed: 0`, against **471 discovered**
(`dotnet test --list-tests`). Nothing had changed between them.

This is worse than a failure, because it looks like a pass. Two habits make it safe:

- Compare the passed count against the previous known-good count for that project. A drop with
  zero failures is a truncated run, not a fixed test.
- `dotnet test Paperless.slnx` is the most likely to truncate and the least likely to say so —
  it has also been OOM-killed outright. Run the projects individually and total them yourself.

### Before trusting a green run

`Paperless.Fidelity.Tests` needs an installed LibreOffice and **skips with a reason when it is
missing**, so a bare `dotnet test` on a fresh container reports a green run while that project
covers nothing at all. A fresh container has none of what it needs. Install it, then confirm
with `check-env.sh` below:

```bash
apt-get install -y --no-install-recommends \
    libreoffice-writer libreoffice-calc libreoffice-impress \
    fonts-crosextra-carlito fonts-crosextra-caladea fonts-liberation \
    poppler-utils
```

`libreoffice-core` alone gives an `soffice` that starts, reports a version and then fails on
every document — which is why `LibreOfficeRunner.IsAvailable` decides by converting a probe file
rather than by finding the binary. The fonts are not optional either: without Carlito and
Caladea every OOXML comparison measures a substituted face and is meaningless. A correct run
reports **0 skipped**; any other number means part of the suite covered nothing.

Comparing against LibreOffice — use the skills, they encode hard-won details:

| Skill | Use for |
|---|---|
| `libreoffice-reference` | Generating reference PDFs, page PNGs and text with headless `soffice` |
| `render-comparison` | Comparing renderings and diagnosing *why* they differ |
| `extraction-comparison` | Comparing extracted text; also the right first step for a visual bug |
| `paperless-corpus` | Building and curating test documents |

### The sample corpus

`theolivenbaum/sample-files` holds 534 real-world documents — collected from the open web
and kept as found, mislabelled extensions and malformed markup included — ordered by what
their LibreOffice rendering demands of a renderer and cut into batches of at most ten:

```
words/batch-001 … batch-021     doc  docx     200 documents
slides/batch-001 … batch-017    ppt  pptx     163 documents
sheets/batch-001 … batch-018    xls  xlsx     171 documents
```

Some of those extensions are **upper-case on disk** — four files are `.DOC`, `.XLS`, `.XLSX`.
A case-sensitive glob quietly counts 530 instead of 534, which is the same mistake as
trusting an extension at all, in miniature. Match case-insensitively or, better, do not
filter by extension.

Per-family tracks, because a single global ordering front-loads the easy end almost
entirely with word processing and leaves the other two families idle for forty batches.
Three tracks let three workers run in parallel and never touch the same file.

**Sheets is not deferred.** It was originally scheduled last on the grounds that a
spreadsheet's value is in its cells rather than its pagination; that was retired once the
track turned out to hold the corpus's largest systematic defects — one workbook paginating
1170 pages against 220 — so deferring it was hiding them rather than deprioritising them.
All three tracks now advance in parallel and never wait for one another.

```sh
.claude/skills/corpus-batches/scripts/batch-check.sh /workspace/sample-files 'words/batch-003' out 3
.claude/skills/corpus-batches/scripts/batch-check.sh /workspace/sample-files 'words/batch-00[1-2]' out 3
```

**Both of those runs are the workflow, and the second is not optional.** Make the current
batch match, then re-prove every earlier batch in the track. This is the cascade rule
again in corpus form: a fix aimed at batch *n* routinely breaks batch *n−4* in a way that
looks nothing like the change, and advancing on the first condition alone is how a corpus
rots from the front.

**`TODO.raster-ceiling.md` lists 37 pages the word gate cannot win.** LibreOffice rasterises
an embedded object on those, so its PDF holds a picture where ours holds real searchable text —
ours is the better output and `wc -w` scores it as failure. An embedded metafile is the
commonest cause and not the only one: 16 of the 37 are on documents holding none. Check that
list before working any word-count failure; several agents have each re-derived it the hard way.

The `corpus-batches` skill holds the rest — why the ordering and the batch size are what
they are, what parity does and does not prove, and what a dispatch brief for a parallel
agent has to contain. `TODO.batches.md` is the scoreboard.

Verify the environment before trusting any comparison:

```bash
.claude/skills/libreoffice-reference/scripts/check-env.sh
```

## Research notes

Written from a deep read of the C++ implementation. Consult the relevant one *before*
implementing an area — they contain exact record layouts, algorithms and file:line
citations, and will save far more time than they cost to read.

| File | Covers |
|---|---|
| `research/01-formats-and-detection.md` | The filter/type registry; the detection algorithm with concrete signatures |
| `research/02-writer.md` | Writer's document model, layout engine, and the DOCX/DOC/RTF/ODT importers |
| `research/03-calc.md` | Calc's cell storage, formula engine, importers, and print pagination |
| `research/04-impress.md` | The shape model, custom-shape geometry, PPTX/PPT/ODP importers, slide rendering |
| `research/05-infrastructure.md` | OLE2/CFB byte layouts, ZIP/OPC/ODF packaging, encryption, EditEngine, item sets, encodings |
| `research/06-rendering.md` | VCL output, fonts and metrics, drawinglayer primitives, PDF export, headless entry points |

## Conventions

- British spelling in identifiers and prose (`Colour`, `normalise`) — consistent with the
  existing code.
- XML doc comments on public API. Say *why*, not just what; the what is usually evident
  from the signature.
- Avoid the name `Path` for new types: it collides with `System.IO.Path` under implicit
  usings. The geometry type is `GraphicsPath`.
- Prefer `readonly record struct` for small value types, `sealed record` for immutable
  reference types.
- `Span`/`ReadOnlySpan` for binary parsing hot paths. `AllowUnsafeBlocks` is on.
