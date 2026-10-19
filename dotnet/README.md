![X-Ray](https://raw.githubusercontent.com/curiosity-ai/x-ray-office/master/dotnet/assets/logo.png)

# X-Ray.Office

[![NuGet](https://img.shields.io/nuget/v/X-Ray.Office?label=NuGet&color=5c2d91&logo=nuget&logoColor=white)](https://www.nuget.org/packages/X-Ray.Office/)
[![License](https://img.shields.io/badge/license-MPL--2.0-007ec6)](../COPYING.MPL)

Faithful rendering of Word, Excel and PowerPoint documents, and the formats around them, to PDF
and images, plus extraction of their text, tables and structure. A pure C# / .NET port of
LibreOffice's Writer, Calc and Impress import and layout code: no LibreOffice process, no office
installation, no COM interop.

X-Ray.Office is the office-document member of the [X-Ray](https://github.com/curiosity-ai/x-ray)
family of .NET libraries from Curiosity GmbH. Family documentation:
<https://xray.curiosity.ai/>. It lives in the `dotnet/` folder of a LibreOffice source checkout;
the surrounding C++ tree is the reference the port is read from, and is never built. The port was
called *Paperless* before it joined the family.

## Install

```sh
dotnet add package X-Ray.Office
```

`X-Ray.Office` is the facade and depends on every other package below, so installing it brings
the whole set. Package ids are hyphenated (`X-Ray.Office.Core`); assemblies and namespaces are not
(`XRay.Office.Core`), because a hyphen is not valid in a C# identifier. Targets `net10.0`.

## Scope

| Family | Formats |
|---|---|
| Word processing | `docx docm dotx dotm` · `doc dot` · `rtf` · `odt ott fodt` · `sxw stw` |
| Spreadsheets | `xlsx xlsm xltx xltm xlsb` · `xls xlt` · `ods ots fods` · `csv` · `sxc stc` |
| Presentations | `pptx pptm potx potm ppsx ppsm` · `ppt pot pps` · `odp otp fodp` · `sxi sti` |

Out of scope: Draw, Math and Base; writing any office format; macro execution. Macro-enabled files
are read as data: X-Ray.Office reports that macros are present and never runs them.

The format is always identified from the **content**, never from the file extension.

## Quick start

```csharp
using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Markup;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;
using XRay.Office.Spreadsheets.Html;
using XRay.Office.Spreadsheets.Layout;

// Extract content. No fonts, no layout, no rasteriser: a small fraction of the cost of rendering.
ContentDocument content = OfficeDocument.Extract("report.docx");
string text = content.GetText();
string markdown = MarkdownWriter.ToMarkdown(content);

// Render. Layout is a separate, deferred step.
using IDocument deck = OfficeDocument.Open("deck.pptx");
IPageSequence slides = ((IPaginatedDocument)deck).Layout();

using (FileStream pdf = File.Create("deck.pdf"))
    new PdfRenderer().Render(slides, pdf);                 // searchable text, subset fonts

using (FileStream png = File.Create("slide1.png"))
    new RasterRenderer(new RasterRenderOptions { Dpi = 150 }).RenderToPng(slides[0], png);

// A workbook can also be written as HTML: one table per sheet, in the shape Calc's own HTML
// filter writes, or as a strip of tabs with no script at all.
using IDocument book = OfficeDocument.Open("report.xlsx");
SpreadsheetPages sheets = (SpreadsheetPages)((IPaginatedDocument)book).Layout();

using (FileStream html = File.Create("report.html"))
    SheetHtmlWriter.Write(sheets, html, new SheetHtmlOptions
    {
        Navigation = SheetHtmlNavigation.Tabs,
        IdPrefix   = "report",   // names the radio group, so two workbooks can share a page
    });
```

### Cancelling, limiting and describing the output

Everything that does real work takes a `CancellationToken`, and a layout can also carry its own limits:

```csharp
using XRay.Office.Core.Threading;

using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(30));

using IDocument doc = OfficeDocument.Open("report.docx", cts.Token);
IPageSequence pages = ((IPaginatedDocument)doc).Layout(
    new LayoutOptions
    {
        MaxPages = 200,                                  // stop paginating here
        Timeout = TimeSpan.FromSeconds(10),              // wall clock, layout only
        MaxImagePixels = 25_000_000,                     // skip pictures larger than this, unread
        OversizedImages = OversizedImagePolicy.Skip,     // or Downscale (JPEG decodes at 1/2, 1/4, 1/8)
    },
    cts.Token);

// Written straight to the stream as it is produced; a temp file is the natural destination.
using FileStream pdf = File.Create("report.pdf");
new PdfRenderer().Render(doc, pages, pdf, cts.Token);    // document metadata, links, outline
```

A cancelled call throws `OperationCanceledException`. A layout that outran `Timeout` throws
`LayoutTimeoutException`, which derives from it, so one `catch` covers both and the two can still be told
apart. The PDF carries the title, author, subject, keywords and language, an XMP packet, a `/Link`
annotation for every hyperlink and for every link to a bookmark or table-of-contents entry, an outline built
from the headings (the sheets of a workbook, the slides of a deck), and page labels where the numbering
restarts. `PdfRenderOptions` switches each of them off (`WriteLinks`, `WriteOutline`, `WriteXmp`,
`WriteDocumentInfo`, `WritePageLabels`).

`OfficeDocument.Identify(path)` names a file's format without reading it, which is the cheap way
to filter a corpus. Callers who already know the family, or want a smaller dependency footprint,
can use `WordProcessingReader`, `SpreadsheetReader` or `PresentationReader` directly.

The same pipeline is available from the command line as `xray-office`
(`tools/XRay.Office.Cli`, not published): `identify`, `extract`, `metadata`, `render` and
`analyze`.

## Packages

| Package | Contents |
|---|---|
| `X-Ray.Office` | Facade: sniff a file and dispatch. Start here; it depends on all of the others. |
| `X-Ray.Office.Core` | Abstractions: units, geometry, colour, document model, drawing IR. Zero dependencies. |
| `X-Ray.Office.Containers` | OLE2/CFB, ZIP, OPC, ODF packages, decryption |
| `X-Ray.Office.Text` | Fonts, metrics, shaping, line breaking, paragraph layout; bundled metric-compatible fonts |
| `X-Ray.Office.Vector` | WMF, EMF, EMF+, SVG import |
| `X-Ray.Office.Rendering` | SkiaSharp raster (PNG, JPEG) and PDF backends |
| `X-Ray.Office.Markup` | XHTML and GitHub-Flavored Markdown writers over the content tree |
| `X-Ray.Office.Ooxml` | Shared OOXML: DrawingML, themes, VML |
| `X-Ray.Office.OpenDocument` | Shared ODF: styles, common attributes, flat XML |
| `X-Ray.Office.MsBinary` | Shared legacy binary: Escher, OLE property sets, codepages |
| `X-Ray.Office.WordProcessing` | Writer-equivalent formats |
| `X-Ray.Office.Spreadsheets` | Calc-equivalent formats, the formula engine, and the HTML export |
| `X-Ray.Office.Presentations` | Impress-equivalent formats |

Reference only what you need: a service that indexes spreadsheets can take
`X-Ray.Office.Spreadsheets` alone and pulls in neither the presentation reader nor a rasteriser.

**Native dependencies.** Rendering and shaping go through SkiaSharp (`X-Ray.Office.Rendering`) and
HarfBuzzSharp (`X-Ray.Office.Text`), both of which carry native libraries. Content extraction does
not load either.

## Building

Requires the .NET 10 SDK.

```bash
dotnet build XRay.Office.slnx -c Release   # warning-free; TreatWarningsAsErrors is on
dotnet test  XRay.Office.slnx -c Release
dotnet pack  XRay.Office.slnx -c Release -o artifacts   # the 13 packages under src/ (symbols embedded)
```

`XRay.Office.Fidelity.Tests` compares against an installed LibreOffice and skips, with a reason,
when there is none; every other test project is self-contained.

Packages are published to nuget.org by `.devops/build-x-ray-office-nuget.yml`, which versions the
whole set together with CalVer (`yy.M.<build id>`) on every change to `dotnet/` on `master`.

## Development

- [CLAUDE.md](CLAUDE.md): architecture, design decisions and the rules that matter
- [TODO.md](TODO.md): the plan and its ordering; per-library `TODO.md` alongside each project
- [research/](research/): notes on how LibreOffice implements these formats, with file:line
  citations. Read the relevant one before implementing an area.

### Comparing against LibreOffice

X-Ray.Office is developed against headless LibreOffice as the reference implementation. The skills
in [`.claude/skills/`](../.claude/skills/) cover it (`libreoffice-reference`, `render-comparison`,
`extraction-comparison`, `page-vision`, `office-corpus`, `corpus-batches`), with working scripts.

Check the environment first; font substitution silently invalidates comparisons:

```bash
../.claude/skills/libreoffice-reference/scripts/check-env.sh
```

LibreOffice is **not** built from this repository. An installed `soffice` is used to produce
ground truth.

## Licence

MPL-2.0, matching the LibreOffice source tree the port derives from (see
[COPYING.MPL](../COPYING.MPL)). Two packages also ship third-party data under its own terms:
`X-Ray.Office.Text` bundles Carlito and Liberation (SIL OFL 1.1), Caladea (Apache-2.0) and DejaVu
(Bitstream Vera), so its package licence expression is
`MPL-2.0 AND OFL-1.1 AND Apache-2.0`. The expression leaves out Bitstream Vera, which nuget.org does not
accept as an approved licence id; the DejaVu licence is in `Fonts/Bundled/README.md`, packed beside the
fonts; `X-Ray.Office.Core` bundles the
American English hyphenation patterns, whose BSD-style notice travels with them.

Every package dependency is permissively licensed: SkiaSharp and HarfBuzzSharp (MIT), the
Svg.Skia family (MIT, except `Svg.Custom`, which is MS-PL), BouncyCastle (MIT). See the note atop
[Directory.Packages.props](Directory.Packages.props) before adding another.
