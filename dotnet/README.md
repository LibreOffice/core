# XRay.Office

Pure C# / .NET libraries for **content extraction** and **headless rendering** of the file
formats LibreOffice's Writer, Calc and Impress support.

> **Status: early.** The container layer works and `xray-office identify` names all supported
> formats from their content. No document *content* is readable yet — see [TODO.md](TODO.md)
> for the plan and what is next.

## Why

Existing .NET options each cover a slice: one library for `.docx`, another for `.xlsx`,
nothing for the legacy binary formats, and nothing that renders. XRay.Office aims at one
managed dependency that reads every common office format and can both extract its content
and draw it — no LibreOffice process, no native office install, no COM interop.

## Scope

| Family | Formats |
|---|---|
| Word processing | `docx docm dotx dotm` · `doc dot` · `rtf` · `odt ott fodt` · `sxw stw` |
| Spreadsheets | `xlsx xlsm xltx xltm xlsb` · `xls xlt` · `ods ots fods` · `csv` · `sxc stc` |
| Presentations | `pptx pptm potx potm ppsx ppsm` · `ppt pot pps` · `odp otp fodp` · `sxi sti` |

Out of scope: Draw, Math and Base; writing any format; macro execution (macro-enabled files
are read as data — XRay.Office reports that macros are present and never runs them).

## Two things it does

```csharp
// Extract content. No fonts, no layout, no rasteriser -- a small fraction of the cost
// of rendering.
ContentDocument content = OfficeDocument.Extract("report.docx");
string text = content.GetText();

// Render. Layout is a separate, deferred step.
using IDocument doc = OfficeDocument.Open("deck.pptx");
IPageSequence pages = ((IPaginatedDocument)doc).Layout();
new RasterRenderer(new RasterRenderOptions { Dpi = 150 })
    .RenderToPng(pages[0], File.Create("slide1.png"));

// Or, for a workbook, write it as HTML instead of drawing it: one table per sheet, in the
// shape Calc's own HTML filter writes. The document paints its own ground and follows the
// reader's colour scheme, while the sheet keeps the white paper its colours were chosen
// against - so it stays legible dropped into a page of any colour.
using IDocument book = OfficeDocument.Open("report.xlsx");
SpreadsheetPages sheets = (SpreadsheetPages)((IPaginatedDocument)book).Layout();

SheetHtmlWriter.Write(sheets, File.Create("report.html"));

// Or as the workbook's own shape - a strip of tabs, one sheet at a time. Still one
// self-contained file, and no script: the switching is a radio group and two CSS rules,
// so it survives a sandboxed frame and a policy that admits no inline script. It prints
// as every sheet.
SheetHtmlWriter.Write(sheets, File.Create("tabbed.html"), new SheetHtmlOptions
{
    Navigation = SheetHtmlNavigation.Tabs,
    IdPrefix   = "report",   // names the radio group, so two workbooks can share a page
});
```

Format is always determined from **content**, not the file extension.

## Packages

| Package | Contents |
|---|---|
| `XRay.Office` | Facade: sniff a file and dispatch. Start here. |
| `XRay.Office.Core` | Abstractions: units, geometry, colour, document model, drawing IR. Zero dependencies. |
| `XRay.Office.Containers` | OLE2/CFB, ZIP, OPC, ODF packages, decryption |
| `XRay.Office.Text` | Fonts, metrics, shaping, line breaking, paragraph layout |
| `XRay.Office.Vector` | WMF, EMF, EMF+, SVG import |
| `XRay.Office.Rendering` | SkiaSharp raster, PDF and SVG backends |
| `XRay.Office.Ooxml` | Shared OOXML: DrawingML, themes, VML |
| `XRay.Office.OpenDocument` | Shared ODF: styles, common attributes, flat XML |
| `XRay.Office.MsBinary` | Shared legacy binary: Escher, OLE property sets, codepages |
| `XRay.Office.WordProcessing` | Writer-equivalent formats |
| `XRay.Office.Spreadsheets` | Calc-equivalent formats, the formula engine, and the HTML export |
| `XRay.Office.Presentations` | Impress-equivalent formats |

Reference only what you need: a service that indexes spreadsheets pulls in neither the
presentation reader nor a rasteriser.

## Building

Requires the .NET 10 SDK.

```bash
dotnet build XRay.Office.slnx     # warning-free; TreatWarningsAsErrors is on
dotnet test  XRay.Office.slnx
```

## Development

- [CLAUDE.md](CLAUDE.md) — architecture, design decisions and the rules that matter
- [TODO.md](TODO.md) — the plan and its ordering; per-library `TODO.md` alongside each project
- [research/](research/) — ~6000 lines of notes on how LibreOffice implements these formats,
  with file:line citations. Read the relevant one before implementing an area.

### Comparing against LibreOffice

XRay.Office is developed against headless LibreOffice as the reference implementation. Four
skills in [`.claude/skills/`](../.claude/skills/) cover it — `libreoffice-reference`,
`render-comparison`, `extraction-comparison`, `office-corpus` — with working scripts.

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
