# Research notes

In-depth notes on how LibreOffice implements the formats Paperless targets, written from a
direct read of the C++ sources in this repository.

**Read the relevant document before implementing an area.** They contain exact record
layouts, algorithms, and `file:line` citations, and will save far more time than they cost.

| Document | Lines | Covers |
|---|---|---|
| [01-formats-and-detection.md](01-formats-and-detection.md) | 1033 | The filter/type registry; the detection algorithm with concrete magic bytes, stream names, content types and CLSIDs |
| [02-writer.md](02-writer.md) | 919 | Writer's node/attribute model, the frame-based layout engine and portion model, and the DOCX / DOC(WW8) / RTF / ODT importers |
| [03-calc.md](03-calc.md) | 1067 | Calc's column cell storage, the separate formatting structure, the formula compiler and interpreter, importers, and print pagination |
| [04-impress.md](04-impress.md) | 990 | The `SdrObject` shape model, custom-shape geometry and its equation evaluator, PPTX / PPT / ODP importers, theme colour resolution, slide rendering |
| [05-infrastructure.md](05-infrastructure.md) | 1022 | OLE2/CFB byte layouts, ZIP/OPC/ODF packaging, all the encryption schemes, EditEngine, item-set semantics, encodings and metadata |
| [06-rendering.md](06-rendering.md) | 969 | VCL output abstraction, font matching and metrics, drawinglayer primitives, PDF export, headless conversion entry points |

## Reliability

Citations were checked mechanically: **481 of 491** distinct `path:line` references resolve
to a real file with the line number in range (98%), and **none** pointed past the end of a
file. The ten that do not resolve are shorthand rather than errors — collective forms like
`impedit2/3/4/5.cxx`, or a path abbreviated to its last two segments.

That said, these are secondary sources. When a detail decides an implementation choice,
confirm it against the cited source. The line numbers are pinned to this checkout and will
drift as upstream changes.

## Scope of the reading

Deliberately excludes Draw, Math and Base, and excludes export paths: Paperless reads.

Where a document recommends a C# approach or library, treat it as a starting position rather
than a decision — the dependency audit in `Directory.Packages.props` already overrode one
such recommendation, since `SixLabors.Fonts` turns out to fail the build without a purchased
licence key.
