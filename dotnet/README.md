# Paperless

Pure C# / .NET libraries for **content extraction** and **headless rendering** of the file
formats LibreOffice's Writer, Calc and Impress support.

> **Status: early.** The container layer works and `paperless identify` names all supported
> formats from their content. No document *content* is readable yet — see [TODO.md](TODO.md)
> for the plan and what is next.

## Why

Existing .NET options each cover a slice: one library for `.docx`, another for `.xlsx`,
nothing for the legacy binary formats, and nothing that renders. Paperless aims at one
managed dependency that reads every common office format and can both extract its content
and draw it — no LibreOffice process, no native office install, no COM interop.

## Scope

| Family | Formats |
|---|---|
| Word processing | `docx docm dotx dotm` · `doc dot` · `rtf` · `odt ott fodt` · `sxw stw` |
| Spreadsheets | `xlsx xlsm xltx xltm xlsb` · `xls xlt` · `ods ots fods` · `csv` · `sxc stc` |
| Presentations | `pptx pptm potx potm ppsx ppsm` · `ppt pot pps` · `odp otp fodp` · `sxi sti` |

Out of scope: Draw, Math and Base; writing any format; macro execution (macro-enabled files
are read as data — Paperless reports that macros are present and never runs them).

## Two things it does

```csharp
// Extract content. No fonts, no layout, no rasteriser -- a small fraction of the cost
// of rendering.
ContentDocument content = PaperlessDocument.Extract("report.docx");
string text = content.GetText();

// Render. Layout is a separate, deferred step.
using IDocument doc = PaperlessDocument.Open("deck.pptx");
IPageSequence pages = ((IPaginatedDocument)doc).Layout();
new RasterRenderer(new RasterRenderOptions { Dpi = 150 })
    .RenderToPng(pages[0], File.Create("slide1.png"));
```

Format is always determined from **content**, not the file extension.

## Packages

| Package | Contents |
|---|---|
| `Paperless` | Facade: sniff a file and dispatch. Start here. |
| `Paperless.Core` | Abstractions: units, geometry, colour, document model, drawing IR. Zero dependencies. |
| `Paperless.Containers` | OLE2/CFB, ZIP, OPC, ODF packages, decryption |
| `Paperless.Text` | Fonts, metrics, shaping, line breaking, paragraph layout |
| `Paperless.Vector` | WMF, EMF, EMF+, SVG import |
| `Paperless.Rendering` | SkiaSharp raster, PDF and SVG backends |
| `Paperless.Ooxml` | Shared OOXML: DrawingML, themes, VML |
| `Paperless.OpenDocument` | Shared ODF: styles, common attributes, flat XML |
| `Paperless.MsBinary` | Shared legacy binary: Escher, OLE property sets, codepages |
| `Paperless.WordProcessing` | Writer-equivalent formats |
| `Paperless.Spreadsheets` | Calc-equivalent formats, including the formula engine |
| `Paperless.Presentations` | Impress-equivalent formats |

Reference only what you need: a service that indexes spreadsheets pulls in neither the
presentation reader nor a rasteriser.

## Building

Requires the .NET 10 SDK.

```bash
dotnet build Paperless.slnx     # warning-free; TreatWarningsAsErrors is on
dotnet test  Paperless.slnx
```

## Development

- [CLAUDE.md](CLAUDE.md) — architecture, design decisions and the rules that matter
- [TODO.md](TODO.md) — the plan and its ordering; per-library `TODO.md` alongside each project
- [research/](research/) — ~6000 lines of notes on how LibreOffice implements these formats,
  with file:line citations. Read the relevant one before implementing an area.

### Comparing against LibreOffice

Paperless is developed against headless LibreOffice as the reference implementation. Four
skills in [`.claude/skills/`](../.claude/skills/) cover it — `libreoffice-reference`,
`render-comparison`, `extraction-comparison`, `paperless-corpus` — with working scripts.

Check the environment first; font substitution silently invalidates comparisons:

```bash
../.claude/skills/libreoffice-reference/scripts/check-env.sh
```

LibreOffice is **not** built from this repository. An installed `soffice` is used to produce
ground truth.

## Licence

MPL-2.0, matching the surrounding LibreOffice source tree.

Every dependency is permissively licensed — SkiaSharp and HarfBuzzSharp (MIT), the Svg.Skia
family (MIT, except `Svg.Custom` which is MS-PL), PdfPig (Apache-2.0), BouncyCastle (MIT).
See the note atop [Directory.Packages.props](Directory.Packages.props) before adding
another.
