# LibreOffice filter names for the Paperless formats

Filter names for use in `--convert-to '<ext>:<FilterName>[:<options>]'`.

**Every name below was verified empirically** against LibreOffice 24.2.7.2 by actually
running the conversion and checking the output file appeared — not by reading the registry
alone.

## The naming trap

The filter registry lives in `filter/source/config/fragments/filters/*.xcu`, one file per
filter. **The file name is not the filter name.** Special characters are replaced with
underscores in the file name, while the real name — the `oor:name` attribute inside — uses
spaces and punctuation:

| `.xcu` file name | Actual filter name |
|---|---|
| `MS_Word_2007_XML.xcu` | `MS Word 2007 XML` |
| `calc_MS_Excel_2007_XML.xcu` | `Calc MS Excel 2007 XML` |
| `ODT_FlatXML.xcu` | `OpenDocument Text Flat XML` |
| `Text___txt___csv__StarCalc_.xcu` | `Text - txt - csv (StarCalc)` |

So most names contain spaces and **must be quoted** on the command line:

```bash
soffice --headless ... --convert-to 'docx:MS Word 2007 XML' --outdir out/ in.odt
```

The `*_pdf_Export` and `*8` names are the exceptions that need no quoting.

To find a name not listed here:

```bash
grep -o 'oor:name="[^"]*"' filter/source/config/fragments/filters/<file>.xcu | head -1
```

## Word processing

| Target | Filter name | Type name |
|---|---|---|
| DOCX | `MS Word 2007 XML` | `writer_MS_Word_2007` |
| DOTX | `MS Word 2007 XML Template` | `writer_MS_Word_2007_Template` |
| DOCM | `MS Word 2007 XML VBA` | `writer_MS_Word_2007_VBA` |
| DOC | `MS Word 97` | `writer_MS_Word_97` |
| DOT | `MS Word 97 Vorlage` | `writer_MS_Word_97_Vorlage` |
| RTF | `Rich Text Format` | `writer_Rich_Text_Format` |
| ODT | `writer8` | `writer8` |
| OTT | `writer8_template` | `writer8_template` |
| FODT | `OpenDocument Text Flat XML` | `writer_ODT_FlatXML` |
| TXT | `Text` | `generic_Text` |
| HTML | `HTML (StarWriter)` | `generic_HTML` |

## Spreadsheet

| Target | Filter name | Type name |
|---|---|---|
| XLSX | `Calc MS Excel 2007 XML` | `MS Excel 2007 XML` |
| XLTX | `Calc MS Excel 2007 XML Template` | `MS Excel 2007 XML Template` |
| XLSM | `Calc MS Excel 2007 VBA XML` | `MS Excel 2007 VBA XML` |
| XLSB | `Calc MS Excel 2007 Binary` | `MS Excel 2007 Binary` |
| XLS | `MS Excel 97` | `calc_MS_Excel_97` |
| XLT | `MS Excel 97 Vorlage/Template` | `calc_MS_Excel_97_VorlageTemplate` |
| ODS | `calc8` | `calc8` |
| OTS | `calc8_template` | `calc8_template` |
| FODS | `OpenDocument Spreadsheet Flat XML` | `calc_ODS_FlatXML` |
| CSV | `Text - txt - csv (StarCalc)` | `generic_Text` |
| HTML | `HTML (StarCalc)` | `generic_HTML` |

**XLSB is import-only.** Its registry flags are `IMPORT ALIEN 3RDPARTYFILTER PREFERRED`
— no `EXPORT` — so `--convert-to xlsb` fails silently (exit code 0, no output file), which
is exactly the trap that makes checking for the output file mandatory. LibreOffice reads
XLSB fine; it just cannot write it. To get an XLSB test file you need Excel or a
third-party writer.

`Text - txt - csv (StarCalc)` exports **only the first sheet.** For multi-sheet ground
truth use the PDF, whose print layout covers every sheet.

## Presentation

| Target | Filter name | Type name |
|---|---|---|
| PPTX | `Impress MS PowerPoint 2007 XML` | `MS PowerPoint 2007 XML` |
| POTX | `Impress MS PowerPoint 2007 XML Template` | `MS PowerPoint 2007 XML Template` |
| PPSX | `Impress MS PowerPoint 2007 XML AutoPlay` | `MS PowerPoint 2007 XML AutoPlay` |
| PPTM | `Impress MS PowerPoint 2007 XML VBA` | `MS PowerPoint 2007 XML VBA` |
| PPT | `MS PowerPoint 97` | `impress_MS_PowerPoint_97` |
| POT | `MS PowerPoint 97 Vorlage` | `impress_MS_PowerPoint_97_Vorlage` |
| PPS | `MS PowerPoint 97 AutoPlay` | `impress_MS_PowerPoint_97_AutoPlay` |
| ODP | `impress8` | `impress8` |
| OTP | `impress8_template` | `impress8_template` |
| FODP | `OpenDocument Presentation Flat XML` | `impress_ODP_FlatXML` |

Impress has **no plain-text filter.** Use `impress_html_Export` for text ground truth.

## Export filters for reference output

These are per-family, which is why `--convert-to pdf` resolves differently depending on
which application loaded the document:

| Target | Writer | Calc | Impress |
|---|---|---|---|
| PDF | `writer_pdf_Export` | `calc_pdf_Export` | `impress_pdf_Export` |
| PNG | — | — | `impress_png_Export` |
| HTML | `HTML (StarWriter)` | `HTML (StarCalc)` | `impress_html_Export` |
| Text | `Text` | `Text - txt - csv (StarCalc)` | — |

`impress_png_Export` renders **only the first slide.** For all pages, export PDF and
rasterise it.

## Filter options

### JSON form (most filters)

Third colon-separated field. Recognised when the string starts with `{`. Each value is an
object with a string `type` and a string `value`:

```bash
soffice --headless --norestore -env:UserInstallation=file:///tmp/p \
  --convert-to 'pdf:writer_pdf_Export:{"UseTaggedPDF":{"type":"boolean","value":"false"},"ReduceImageResolution":{"type":"boolean","value":"false"}}' \
  --outdir out/ in.docx
```

PDF export keys:

| Key | Type | Notes |
|---|---|---|
| `SelectPdfVersion` | `long` | `0` default; `1` PDF/A-1b; `15`/`16`/`17` for PDF 1.5/1.6/1.7 |
| `UseTaggedPDF` | `boolean` | Set **false** for reference output: tagging changes nothing visually and enlarges the file |
| `ReduceImageResolution` | `boolean` | Set **false** for reference output, or images get downsampled |
| `MaxImageResolution` | `long` | DPI cap when reducing |
| `Quality` | `long` | JPEG quality 1-100 |
| `ExportBookmarks` | `boolean` | |
| `PageRange` | `string` | e.g. `"1-3"` |
| `ExportNotesPages` | `boolean` | Impress: also export speaker-notes pages |

PNG export keys:

| Key | Type | Notes |
|---|---|---|
| `PixelWidth` | `long` | Output width in pixels |
| `PixelHeight` | `long` | Output height in pixels |
| `Translucent` | `boolean` | Keep alpha. Leave **off** when comparing: the PDF path composites onto white, so a transparent background would differ for a reason that is not a bug |

### Positional form (CSV only)

CSV predates the JSON mechanism and uses a comma-separated token string instead. Field 1
is the separator's character code, field 2 the text delimiter, field 3 the character set
(`44` = comma, `34` = `"`, `76` = UTF-8):

```bash
--convert-to 'csv:Text - txt - csv (StarCalc):44,34,76'
```

Verified working.
