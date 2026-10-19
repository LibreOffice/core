![X-Ray](https://raw.githubusercontent.com/curiosity-ai/x-ray-office/master/dotnet/assets/logo.png)

# X-Ray.Office

Renders Word, Excel and PowerPoint documents, along with their legacy binary, OpenDocument and
RTF relatives, to PDF and images, and extracts their text, tables and structure. Pure C#, ported
from LibreOffice's Writer, Calc and Impress import and layout code: no office installation, no
LibreOffice process, no COM interop.

X-Ray.Office is part of the [X-Ray](https://github.com/curiosity-ai/x-ray) family of .NET
libraries from Curiosity GmbH. Documentation: <https://xray.curiosity.ai/>.

## Install

```sh
dotnet add package X-Ray.Office
```

`X-Ray.Office` is the facade and depends on every other `X-Ray.Office.*` package, so this one line
brings the whole set. The package ids are hyphenated; the API lives under the `XRay.Office`
namespace, since a hyphen is not valid in a C# identifier. Targets `net10.0`.

## Quick start

```csharp
using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;

// Extract content: no fonts, no layout, no rasteriser.
ContentDocument content = OfficeDocument.Extract("report.docx");
string text = content.GetText();

// Render: layout is a separate, deferred step.
using IDocument document = OfficeDocument.Open("deck.pptx");
IPageSequence pages = ((IPaginatedDocument)document).Layout();

using (FileStream pdf = File.Create("deck.pdf"))
    new PdfRenderer().Render(pages, pdf);

using (FileStream png = File.Create("slide1.png"))
    new RasterRenderer(new RasterRenderOptions { Dpi = 150 }).RenderToPng(pages[0], png);
```

The format is always identified from the content, never from the file extension.

## Cancelling and limiting

Every call that does real work has an overload taking a `CancellationToken` and throws
`OperationCanceledException` when it fires: `OfficeDocument.Open` and `Extract`,
`IPaginatedDocument.Layout`, `PdfRenderer.Render`, `RasterRenderer.Render`, `SheetHtmlWriter.Write`.
`LayoutOptions` also carries `MaxPages`, a wall-clock `Timeout` (which throws `LayoutTimeoutException`, a
subtype of `OperationCanceledException`, so it can be told apart from a caller's cancellation) and
`MaxImagePixels`, which leaves pictures larger than that unread. `PdfRenderer` writes to the stream as it
goes, so a temp file can take a very large document, and it writes the document's title, author and
language, a link for every hyperlink and cross-reference, and an outline of its headings.

## Packages

| Package | Contents |
|---|---|
| `X-Ray.Office` | Facade: identify a file and dispatch to the right reader. Depends on all of the below. |
| `X-Ray.Office.Core` | Units, geometry, colour, format identification, the document model, the display list. No dependencies. |
| `X-Ray.Office.Containers` | OLE2/CFB, ZIP, OPC and ODF packages, decryption |
| `X-Ray.Office.Text` | Fonts, metrics, shaping, line breaking, paragraph layout; bundled metric-compatible fonts |
| `X-Ray.Office.Vector` | WMF, EMF, EMF+ and SVG import |
| `X-Ray.Office.Rendering` | SkiaSharp raster (PNG, JPEG) and PDF backends |
| `X-Ray.Office.Markup` | XHTML and GitHub-Flavored Markdown output of the content tree |
| `X-Ray.Office.Ooxml` | Shared OOXML: DrawingML, themes, VML |
| `X-Ray.Office.OpenDocument` | Shared ODF: styles, common attributes, flat XML |
| `X-Ray.Office.MsBinary` | Shared legacy binary: Escher, OLE property sets, codepages |
| `X-Ray.Office.WordProcessing` | docx, docm, dotx, dotm, doc, dot, rtf, odt, ott, fodt |
| `X-Ray.Office.Spreadsheets` | xlsx, xlsm, xltx, xltm, xlsb, xls, xlt, ods, ots, fods, csv; the formula engine; HTML export |
| `X-Ray.Office.Presentations` | pptx, pptm, potx, potm, ppsx, ppsm, ppt, pot, pps, odp, otp, fodp |

## Native dependencies

Rendering and text shaping use SkiaSharp and HarfBuzzSharp, which carry native libraries
(`X-Ray.Office.Rendering` and `X-Ray.Office.Text`). Content extraction does not load them.

## Licence

MPL-2.0, as the code derives from LibreOffice. `X-Ray.Office.Text` also ships the Carlito,
Liberation (SIL OFL 1.1), Caladea (Apache-2.0) and DejaVu (Bitstream Vera) fonts. Its licence
expression lists all but the Bitstream Vera licence of the DejaVu fonts (nuget.org does not accept that
id); the font licence table ships in the package under `fonts/README.md`. `X-Ray.Office.Core` ships the American English hyphenation patterns under
their own BSD-style terms, with their README beside them. Macro-enabled files are read as data;
macros are never executed.

Source: <https://github.com/curiosity-ai/x-ray-office>
