# LibreOffice File-Format Identification & Filter Registry — Reference

Scope: Writer (word processing), Calc (spreadsheet), Impress (presentation) formats only.
Draw, Math, Base and Chart-standalone formats are intentionally excluded except where
they share machinery (e.g. the OOXML/ODF detectors) that must be understood to reimplement
detection correctly.

This document is derived entirely from reading the LibreOffice core source tree at
`/home/user/libreoffice-core` (this checkout). All non-obvious facts are cited as
`path/to/file:line`. No behavior is guessed; anywhere the source was ambiguous or the
investigation ran out of budget, it is called out explicitly in **"Open questions /
ambiguities"** at the end.

---

## 1. Architecture overview

LibreOffice's format registry is a two-level configuration system, expressed as `.xcu`
(OOo/LO configuration-update XML) fragments, compiled at build time into the
`org.openoffice.TypeDetection.Types` and `org.openoffice.TypeDetection.Filter` config
sets and loaded at runtime by a singleton **`FilterCache`**
(`filter/source/config/cache/filtercache.cxx`).

Two kinds of registry item matter here:

* **Type** (`filter/source/config/fragments/types/*.xcu`) — a file-format identity: its
  extensions, URL patterns, MIME/media type, whether it's the OS-preferred type for that
  media type, which filter is used by default (`PreferredFilter`), and — crucially —
  which UNO service performs *deep* (content-based) detection for it (`DetectService`).
  A type has **no import/export behavior** by itself.
* **Filter** (`filter/source/config/fragments/filters/*.xcu`) — the thing that actually
  reads/writes a type: which UNO service implements it (`FilterService`, or empty for
  built-in ODF/legacy-binary filters wired some other way), its `Flags` bitmask
  (import/export/template/alien/…), the `DocumentService` it can attach to
  (`com.sun.star.text.TextDocument`, `com.sun.star.sheet.SpreadsheetDocument`,
  `com.sun.star.presentation.PresentationDocument`), and free-form `UserData` tokens
  consumed by the filter implementation itself (e.g. `"CWW8"`, `"OXML"`, `"macro-enabled"`).

A **type can have zero, one, or several filters** registered against it (e.g. `writer8`
has one filter of the same name; `pdf_Portable_Document_Format` has several export-only
filters per module: `writer_pdf_Export`, `calc_pdf_Export`, `impress_pdf_Export`).
A **filter always points at exactly one type** via its `Type` property.

The runtime UNO service that ties it together is
**`com.sun.star.comp.filter.config.TypeDetection`**
(`filter/source/config/cache/typedetection.cxx:61-70`), which implements
`com.sun.star.document.TypeDetection`. `SfxFilterMatcher::GuessFilter` /
`SfxFilterMatcher::DetectFilter` (`sfx2/source/bastyp/fltfnc.cxx:494-568`) and the
`XTypeDetection::queryTypeByDescriptor` UNO API both eventually go through this same
service and cache — there is exactly one detection algorithm, not two.

---

## 2. The `Flags` bitmask (filter capability flags)

Defined as `enum class SfxFilterFlags` in
`include/comphelper/documentconstants.hxx:92-121`:

| Flag (C++ enum) | Bit value | XCU token (string used in `<prop oor:name="Flags">`) | Meaning |
|---|---|---|---|
| `IMPORT` | `0x00000001` | `IMPORT` | filter can read this format |
| `EXPORT` | `0x00000002` | `EXPORT` | filter can write this format |
| `TEMPLATE` | `0x00000004` | `TEMPLATE` | (legacy/deprecated) marks a template filter |
| `INTERNAL` | `0x00000008` | `INTERNAL` | internal use, not user-facing |
| `TEMPLATEPATH` | `0x00000010` | `TEMPLATEPATH` | filter for a document **template** format (the modern flag actually used, e.g. `.ott`, `.dotx`) |
| `OWN` | `0x00000020` | `OWN` | one of LibreOffice's own (ODF) formats |
| `ALIEN` | `0x00000040` | `ALIEN` | non-native format (not a LO ZIP/XML container format) |
| `DEFAULT` | `0x00000100` | `DEFAULT` | default filter for its document type |
| `SUPPORTSSELECTION` | `0x00000400` | `SUPPORTSSELECTION` | supports exporting just a selection |
| `NOTINFILEDLG` | `0x00001000` | `NOTINFILEDIALOG` | hidden from the File-dialog format list |
| `OPENREADONLY` | `0x00010000` | `READONLY` | force read-only open |
| `MUSTINSTALL` | `0x00020000` | `NOTINSTALLED` | filter not installed |
| `CONSULTSERVICE` | `0x00040000` | `CONSULTSERVICE` | (unused-ish) consult install service |
| `STARONEFILTER` | `0x00080000` | `3RDPARTYFILTER` | filter is implemented as a full UNO component ("StarONE" = the historical UNO codename) rather than baked into the module |
| `PACKED` | `0x00100000` | `PACKED` | packed/ZIP-based own format |
| `EXOTIC` | `0x00200000` | `EXOTIC` | unusual/legacy format users rarely see |
| `COMBINED` | `0x00800000` | `COMBINED` | combined import/export registration |
| `ENCRYPTION` | `0x01000000` | `ENCRYPTION` | format supports password-to-open encryption |
| `PASSWORDTOMODIFY` | `0x02000000` | `PASSWORDTOMODIFY` | format supports password-to-modify |
| `GPGENCRYPTION` | `0x04000000` | `GPGENCRYPTION` | format supports GPG/OpenPGP-based encryption (ODF only) |
| `PREFERED` [sic] | `0x10000000` | `PREFERRED` | preferred filter among several for the same type |
| `STARTPRESENTATION` | `0x20000000` | `STARTPRESENTATION` | opening the file should auto-start a slideshow (PowerPoint "auto-play" `.pps`/`.ppsx`) |
| `SUPPORTSSIGNING` | `0x40000000` | `SUPPORTSSIGNING` | format supports digital signatures |

The C++ enum ↔ XCU string table (`flagFilterSwitcher`) lives at
`filter/source/config/cache/filtercache.cxx:1821-1849`; the string literals themselves
are in `filter/source/config/cache/constant.hxx:94-117`. `Flags` in an XCU file is a
space-separated list of these tokens, e.g.
`IMPORT EXPORT ALIEN 3RDPARTYFILTER ENCRYPTION PASSWORDTOMODIFY SUPPORTSSIGNING`
(`filter/source/config/fragments/filters/OOXML_Text.xcu:19`).

Design intent, straight from a source comment at
`include/comphelper/documentconstants.hxx:79-89`:

```
Import                  - should be self explaining
Export                  - should be self explaining
Template                - deprecated
TemplatePath            - filter for a document template
Own                     - one of the OOo file formats
Alien                   - no zip container based format
Preferred               - preferred filter for a particular type
3rdPartyFilter          - implemented as a UNO component
Default                 - default filter for this document type
Exotic                  - an unusual/legacy file-format, we don't normally see
```

Note `Alien` really means "not one of LibreOffice's OWN container formats" — OOXML
(a ZIP container!) is still flagged `ALIEN` because it's Microsoft's own format, not
LibreOffice's.

### Type-level properties (from `types/*.xcu`)

Each type node has these `prop` children (seen consistently across all files read):

| Property | Meaning |
|---|---|
| `DetectService` | UNO service name invoked for **deep** (content) detection of this type; may be empty for types resolved purely by extension |
| `URLPattern` | wildcard pattern(s) matched against the whole URL (used for `private:factory/...` virtual URLs, e.g. `impress8.xcu:20`: `private:factory/simpress*`) |
| `Extensions` | space-separated list of file-name extensions (case-insensitive) that flat-match this type |
| `MediaType` | canonical MIME/media type string for the format |
| `Preferred` | `true`/`false` — is this the OS-registered/preferred type for its `MediaType`+extension combo |
| `PreferredFilter` | the filter name normally used to load/save this type |
| `UIName` | localized display name |
| `ClipboardFormat` | clipboard-format name string (also reused, oddly, as a **content sniff pattern** for the generic XML detector — see §6.6) |

---

## 3. The big table: Writer / Calc / Impress formats

"Doc kind" = W (Writer/`com.sun.star.text.TextDocument`), C (Calc/`com.sun.star.sheet.SpreadsheetDocument`),
I (Impress/`com.sun.star.presentation.PresentationDocument`).
"I/E" = Import/Export capability from the filter's `Flags`.
Citations point at the **type** fragment unless noted; the paired **filter** fragment
usually lives at the same basename under `filter/source/config/fragments/filters/`.

### 3.1 ODF native formats

| Ext | Type name | Filter name | MIME type | Kind | I/E | Notes |
|---|---|---|---|---|---|---|
| `.odt` | `writer8` | `writer8` | `application/vnd.oasis.opendocument.text` | W | I/E | `Preferred=true`, `DetectService=com.sun.star.comp.filters.StorageFilterDetect`. Flags: `IMPORT EXPORT TEMPLATE OWN DEFAULT PREFERRED ENCRYPTION PASSWORDTOMODIFY GPGENCRYPTION` (`filter/source/config/fragments/filters/writer8.xcu:19`). `FileFormatVersion=6800`, `UserData=CXML`. `types/writer8.xcu:18-29` |
| `.ott` | `writer8_template` | `writer8_template` | `application/vnd.oasis.opendocument.text-template` | W | I/E | `types/writer8_template.xcu:18-29` |
| `.odm` | `writerglobal8` | `writerglobal8` | `application/vnd.oasis.opendocument.text-master` | W (master doc) | I/E | `URLPattern=private:factory/swriter/GlobalDocument*` `types/writerglobal8.xcu:18-29` |
| `.otm` | `writerglobal8_template` | `writerglobal8_template` | `application/vnd.oasis.opendocument.text-master-template` | W | I/E | `types/writerglobal8_template.xcu` |
| `.oth` | `writerweb8_writer_template` | `writerweb8_writer_template` | `application/vnd.oasis.opendocument.text-web` | W (HTML template) | I/E | `types/writerweb8_writer_template.xcu:18-29` |
| `.ods` | `calc8` | `calc8` | `application/vnd.oasis.opendocument.spreadsheet` | C | I/E | Flags: `IMPORT EXPORT TEMPLATE OWN DEFAULT ENCRYPTION PASSWORDTOMODIFY GPGENCRYPTION` (note: **no `PREFERRED`** flag on the filter, unlike writer8/impress8 — see §9) `filters/calc8.xcu:19` |
| `.ots` | `calc8_template` | `calc8_template` | `application/vnd.oasis.opendocument.spreadsheet-template` | C | I/E | `Preferred=false` on the type (`types/calc8_template.xcu:23`) |
| `.odp` | `impress8` | `impress8` | `application/vnd.oasis.opendocument.presentation` | I | I/E | `types/impress8.xcu:18-29` |
| `.otp` | `impress8_template` | `impress8_template` | `application/vnd.oasis.opendocument.presentation-template` | I | I/E | `types/impress8_template.xcu` |
| `.fodt`/`.odt`/`.xml` | `writer_ODT_FlatXML` | `OpenDocument Text Flat XML` | `application/vnd.oasis.opendocument.text-flat-xml` | W | I/E | Flat (single-file) ODF XML. `DetectService=com.sun.star.comp.filters.XMLFilterDetect`. `ClipboardFormat=doctype:office:mimetype="application/vnd.oasis.opendocument.text"` used as a content-sniff string (§6.6). `types/writer_ODT_FlatXML.xcu:18-31` |
| `.fods`/`.ods`/`.xml` | `calc_ODS_FlatXML` | `OpenDocument Spreadsheet Flat XML` | `application/vnd.oasis.opendocument.spreadsheet-flat-xml` | C | I/E | `types/calc_ODS_FlatXML.xcu:18-31` |
| `.fodp`/`.odp`/`.xml` | `impress_ODP_FlatXML` | `OpenDocument Presentation Flat XML` | `application/vnd.oasis.opendocument.presentation-flat-xml` | I | I/E | `types/impress_ODP_FlatXML.xcu:18-31` |

### 3.2 OOXML (Microsoft Office Open XML) — Writer

Every OOXML Writer format is **double-registered** under two type names sharing the
same extension: a "legacy/ECMA-named" type (`writer_MS_Word_2007*`, not preferred) and
an "ISO-named" type (`writer_OOXML*`, preferred). Both use `FilterService
com.sun.star.comp.Writer.WriterFilter` and `DetectService
com.sun.star.comp.oox.FormatDetector`. See §6.2 for why this duplication is harmless.

| Ext | Type name(s) | Filter name(s) | MIME type | Preferred? | Notes |
|---|---|---|---|---|---|
| `.docx` | `writer_MS_Word_2007` / `writer_OOXML` | `MS Word 2007 XML` / `Office Open XML Text` | `application/msword` / `application/vnd.openxmlformats-officedocument.wordprocessingml.document` | false / **true** | `types/writer_MS_Word_2007_XML.xcu:18-27`, `types/writer_OOXML.xcu:18-29`. Filters share flags `IMPORT EXPORT ALIEN 3RDPARTYFILTER ENCRYPTION PASSWORDTOMODIFY SUPPORTSSIGNING` (`filters/MS_Word_2007_XML.xcu:19`, `filters/OOXML_Text.xcu:19`) |
| `.dotx`/`.dotm` (legacy type covers both!) / `.dotx` (ISO type) | `writer_MS_Word_2007_Template` / `writer_OOXML_Text_Template` | `MS Word 2007 XML Template` / `Office Open XML Text Template` | `application/msword` / `.wordprocessingml.template` | false / true | Legacy type's `Extensions` = `dotx dotm` (`types/writer_MS_Word_2007_XML_Template.xcu:21`); ISO type's `Extensions` = `dotx` only (`types/writer_OOXML_Template.xcu:21`). Flags include `TEMPLATE TEMPLATEPATH`, **no `ENCRYPTION`** (`filters/MS_Word_2007_XML_Template.xcu:19`) |
| `.docm` | `writer_MS_Word_2007_VBA` / `writer_OOXML_VBA` | `MS Word 2007 XML VBA` / `Office Open XML Text VBA` | `application/msword` / `application/vnd.ms-word.document.macroEnabled.main+xml` | false / true | `types/writer_MS_Word_2007_XML_VBA.xcu`, `types/writer_OOXML_VBA.xcu:18-29` |

### 3.3 OOXML — Calc

| Ext | Type name | Filter name | MIME type | Preferred? | Notes |
|---|---|---|---|---|---|
| `.xlsx` | `MS Excel 2007 XML` (legacy) / `Office Open XML Spreadsheet` (ISO) — **type node name equals filter name here**, unlike Writer | `Calc MS Excel 2007 XML` / same names | `application/msword`-style not used; MIME is `application/vnd.openxmlformats-officedocument.spreadsheetml.sheet` for the ISO type, empty for legacy? — legacy type file not separately captured beyond filter names above; ISO type: `types/calc_OOXML.xcu:18-27` | false / true | `FilterService=com.sun.star.comp.oox.xls.ExcelFilter` for both (`filters/calc_MS_Excel_2007_XML.xcu:21`). Flags `IMPORT EXPORT ALIEN 3RDPARTYFILTER PREFERRED ENCRYPTION PASSWORDTOMODIFY SUPPORTSSIGNING` |
| `.xltx` | `MS Excel 2007 XML Template` / `Office Open XML Spreadsheet Template` | `Calc MS Excel 2007 XML Template` | `application/vnd.openxmlformats-officedocument.spreadsheetml.template` | true (ISO) | `types/calc_OOXML_Template.xcu:18-27` |
| `.xlsm` | `MS Excel 2007 VBA XML` / `Office Open XML Spreadsheet VBA` | `Calc MS Excel 2007 VBA XML` | `application/vnd.ms-excel.sheet.macroEnabled.12` | true | `types/calc_OOXML_VBA.xcu:18-27`, `UserData=macro-enabled` (`filters/calc_MS_Excel_2007_VBA_XML.xcu:22`) |
| `.xlsb` | `MS Excel 2007 Binary` | `Calc MS Excel 2007 Binary` | (empty `MediaType` in the type registration!) | false, but filter flagged `PREFERRED` | Excel **binary** OOXML (BIFF12), still ZIP-detected via `DetectService=com.sun.star.comp.oox.FormatDetector` because it's a ZIP/package format, just with binary XLSB parts. Flags: `IMPORT ALIEN 3RDPARTYFILTER PREFERRED` — **no EXPORT** (`filters/calc_MS_Excel_2007_Binary.xcu:19`). `ClipboardFormat=Biff12` (`types/MS_Excel_2007_Binary.xcu:26`) |

### 3.4 OOXML — Impress

| Ext | Type name | Filter name | MIME type | Preferred? | Notes |
|---|---|---|---|---|---|
| `.pptx` | `MS PowerPoint 2007 XML` (legacy) / `Office Open XML Presentation` (ISO) | `Impress MS PowerPoint 2007 XML` / `Impress Office Open XML` | `application/vnd.openxmlformats-officedocument.presentationml.presentation` | false / true | Both `FilterService=com.sun.star.comp.oox.ppt.PowerPointImport`. `types/MS_PowerPoint_2007_XML.xcu:18-27`, `types/impress_OOXML_Presentation.xcu:18-27` |
| `.potx`/`.potm` (legacy) / `.potx` (ISO) | `MS PowerPoint 2007 XML Template` / `Office Open XML Presentation Template` | `Impress MS PowerPoint 2007 XML Template` / `Impress Office Open XML Template` | `.presentationml.template` | false / true | Legacy `Extensions=potx potm` (`types/MS_PowerPoint_2007_XML_Template.xcu:21`) |
| `.pptm` | `MS PowerPoint 2007 XML VBA` / (no separate ISO-named VBA type for Impress observed) | `Impress MS PowerPoint 2007 XML VBA` | `application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml` | **true** already on the legacy-named type | `types/MS_PowerPoint_2007_XML_VBA.xcu:19-28` — note this is the one legacy-named type in the whole OOXML family whose type-level `Preferred` is `true` |
| `.ppsx` | `MS PowerPoint 2007 XML AutoPlay` / `Office Open XML Presentation AutoPlay` | `Impress MS PowerPoint 2007 XML AutoPlay` / `Impress Office Open XML AutoPlay` | `.presentationml.slideshow` | false / true | ISO-named filter flags include `STARTPRESENTATION` (`filters/impress_OOXML_AutoPlay.xcu:19`) |
| `.ppt` (old binary, **not** OOXML) | `impress_MS_PowerPoint_97` | `MS PowerPoint 97` | `application/vnd.ms-powerpoint` | false (only PPT type for `.ppt`, so it always wins by extension) | see §3.6 |

### 3.5 ODF/StarOffice legacy XML (OOo 1.x, `.sxw`/`.sxc`/`.sxi`) and ancient StarOffice binary

| Ext | Type name | Filter name | MIME type | Kind | Notes |
|---|---|---|---|---|---|
| `.sxw` | `writer_StarOffice_XML_Writer` | `StarOffice XML (Writer)` | `application/vnd.sun.xml.writer` | W | `DetectService=StorageFilterDetect`. Flags `IMPORT TEMPLATE OWN ALIEN PREFERRED ENCRYPTION EXOTIC` — **no EXPORT** (`filters/StarOffice_XML__Writer_.xcu:19`) |
| `.stw` | `writer_StarOffice_XML_Writer_Template` **and also** `writer_web_StarOffice_XML_Writer_Web_Template` | — | `application/vnd.sun.xml.writer.template` vs `application/vnd.sun.xml.writer.web` | W | **Same extension `.stw` registered by two different MIME/type pairs** — disambiguated purely by the ZIP `mimetype` stream content (§6.1), not by extension |
| `.sxg` | `writer_globaldocument_StarOffice_XML_Writer_GlobalDocument` | — | `application/vnd.sun.xml.writer.global` | W (master doc) | |
| `.sxc` | `calc_StarOffice_XML_Calc` | — | `application/vnd.sun.xml.calc` | C | |
| `.stc` | `calc_StarOffice_XML_Calc_Template` | — | `application/vnd.sun.xml.calc.template` | C | |
| `.sxi` | `impress_StarOffice_XML_Impress` | — | `application/vnd.sun.xml.impress` | I | |
| `.sti` | `impress_StarOffice_XML_Impress_Template` | — | `application/vnd.sun.xml.impress.template` | I | |
| `.sdw` | `StarOffice_Writer` | `StarOffice_Writer` | (none) | W | Pre-XML StarWriter binary. `DetectService=org.libreoffice.comp.Writer.StarOfficeWriterImportFilter`. Flags `IMPORT ALIEN 3RDPARTYFILTER PREFERRED` — **import only** (`filters/StarOffice_Writer.xcu:11-12`, `types/StarOffice_Writer.xcu:10-27`) |
| `.sdc` | `StarOffice_Spreadsheet` | — | (none) | C | analogous binary StarCalc format |
| `.sdd` | `StarOffice_Presentation` | — | (none) | I | analogous binary StarImpress format |

### 3.6 Legacy MS binary formats (OLE2/CFB compound files)

| Ext | Type name | Filter name | MIME type | Kind | Notes |
|---|---|---|---|---|---|
| `.doc`/`.wps` | `writer_MS_Word_97` | `MS Word 97` | `application/msword` | W | **Word 8/9/10/11 (Word 97–2003)**. `Preferred=true` at the type level (`types/writer_MS_Word_97.xcu:23`). Flags `IMPORT EXPORT ALIEN PREFERRED ENCRYPTION PASSWORDTOMODIFY`, `UserData=CWW8` (`filters/MS_Word_97.xcu:19-22`) |
| `.dot`/`.wpt` | `writer_MS_Word_97_Vorlage` | `MS Word 97 Vorlage` | `application/msword` | W (template) | |
| `.doc` | `writer_MS_Word_95` | `MS Word 95` | `application/msword` | W | Word 6/95 single binary format; `Preferred=false` |
| `.doc` | `writer_MS_WinWord_60` | `MS WinWord 6.0` | `application/msword` | W | |
| `.doc` | `writer_MS_WinWord_5` | `MS WinWord 5` | `application/msword` | W | WinWord 1/2/5, and PmWord1, detected by raw magic bytes, not OLE2 (§6.3) |
| `.xls`/`.xlc`/`.xlm`/`.xlw`/`.xlk`/`.et` | `calc_MS_Excel_97` | `MS Excel 97` | `application/vnd.ms-excel` | C | BIFF8. `Preferred=true`. `.et` is the WPS-Office/Kingsoft Spreadsheets extension mapped straight onto this same filter (`types/calc_MS_Excel_97.xcu:21`). Flags `IMPORT EXPORT ALIEN PREFERRED ENCRYPTION PASSWORDTOMODIFY` |
| `.xlt`/`.ett` | `calc_MS_Excel_97_VorlageTemplate` | `MS Excel 97 Vorlage/Template` | `application/vnd.ms-excel` | C (template) | |
| `.xls`/`.xlc`/`.xlm`/`.xlw` | `calc_MS_Excel_95` | `MS Excel 95` | BIFF5 | C | |
| `.xlt` | `calc_MS_Excel_95_VorlageTemplate` | — | BIFF5 | C (template) | |
| `.xls`/`.xlc`/`.xlm`/`.xlw` | `calc_MS_Excel_5095` | `MS Excel 5.0/95` | BIFF5 | C | combined 5.0/95 detector |
| `.xlt` | `calc_MS_Excel_5095_VorlageTemplate` | — | BIFF5 | C (template) | |
| `.xls`/`.xlw`/`.xlc`/`.xlm` | `calc_MS_Excel_40` | `MS Excel 4.0` | BIFF2/3/4 | C | |
| `.xlt` | `calc_MS_Excel_40_VorlageTemplate` | — | BIFF2/3/4 | C (template) | |
| `.ppt`/`.dps` | `impress_MS_PowerPoint_97` | `MS PowerPoint 97` | `application/vnd.ms-powerpoint` | I | `.dps` = Kingsoft Presentation extension aliased to same filter. Flags `IMPORT EXPORT ALIEN` (no ENCRYPTION flag recorded) `filters/MS_PowerPoint_97.xcu:19-22`, `UserData=sd` |
| `.pot`/`.dpt` | `impress_MS_PowerPoint_97_Vorlage` | `MS PowerPoint 97 Vorlage` | `application/vnd.ms-powerpoint` | I (template) | |
| `.pps` | `impress_MS_PowerPoint_97_AutoPlay` | `MS PowerPoint 97 AutoPlay` | `application/vnd.ms-powerpoint` | I | auto-play flavor of the same binary format |
| `.ppt`/`.pot` | `impress_PowerPoint3` | `PowerPoint 3` | — | I | PowerPoint 1–4 (pre-OLE2? actually via `libmwaw`), `DetectService=com.sun.star.comp.Impress.MWAWPresentationImportFilter` (`types/impress_PowerPoint3.xcu:9-22`) — shares extension `.ppt` with `impress_MS_PowerPoint_97`, disambiguated purely by content sniffing inside libmwaw |

### 3.7 RTF, HTML, plain text, CSV

| Ext | Type name | Filter name | MIME type | Kind | Notes |
|---|---|---|---|---|---|
| `.rtf` | `writer_Rich_Text_Format` | `Rich Text Format` | `application/rtf` | W | `Preferred=false` (it's the only rtf-extension type though). `DetectService=com.sun.star.text.FormatDetector`. Flags `IMPORT EXPORT ALIEN 3RDPARTYFILTER PREFERRED` (filter-level `PREFERRED`, type-level `Preferred=false` — these are two different properties!) `filters/Rich_Text_Format.xcu:19` |
| n/a (content-sniffed) | `generic_HTML` | `HTML`/`HTML (StarWriter)`/`calc_HTML_WebQuery` (per doc-service) | `text/html` | W/C | `DetectService=com.sun.star.comp.filters.PlainTextFilterDetect`, resolved via `IsHTMLStream()` tag-name sniff, see §6.7 |
| n/a | `generic_Text` | `Text` (Writer) / `Text - txt - csv (StarCalc)` (Calc) | `text/plain` | W/C | Same `PlainTextFilterDetect`; disambiguated by `DocumentService` if preselected, else by extension (`csv tsv tab xls` → Calc text filter) (`filter/source/textfilterdetect/filterdetect.cxx:209-216`). Transparently gunzips `.gz`-suffixed streams first |
| `.md`/`.markdown` | `generic_Markdown` | `Markdown` | (text/markdown, informal) | W | same detector |
| `.json` | `generic_JSON` | — | — | C (Calc JSON import) | |
| `.xml` (generic fallback) | `generic_XML` | — | `text/xml` | — | catch-all for XML that doesn't match a more specific `doctype:` pattern |

### 3.8 Notable "alien" import-only formats sharing extensions across modules

These illustrate that **extension alone frequently cannot determine document kind or
even module** — the same extension is legitimately registered by Writer, Calc and
Impress types simultaneously, each with a different `DetectService` that performs real
content sniffing:

| Ext | Registered as | Kind | DetectService |
|---|---|---|---|
| `.cwk` (ClarisWorks) | `writer_ClarisWorks`, `calc_ClarisWorks`, `impress_ClarisWorks` | W, C, I | `com.sun.star.comp.Writer.MWAWImportFilter`, `...Calc.MWAWCalcImportFilter`, `...Impress.MWAWPresentationImportFilter` respectively |
| `.wps` | `writer_MS_Word_97` (Extensions include `wps`), `writer_MS_Works_Document`, `writer_Mac_Works` | W (3-way collision!) | BIFF/OLE2 stream sniff vs `MSWorksImportFilter` vs `MWAWImportFilter` |
| `*` (wildcard, literal) | `MWAW_Text_Document`, `MWAW_Spreadsheet`, `MWAW_Presentation` | W, C, I | catch-all legacy-Mac-format sniffers; see §6.8 for how a literal `*` extension actually behaves in the matcher |
| `.doc` | `writer_MS_Word_97`, `writer_MS_Word_97_Vorlage`(no, dot), `writer_MS_Word_95`, `writer_MS_WinWord_60`, `writer_MS_WinWord_5`, `writer_Mac_Word`, `writer_DosWord` | W | up to 6 candidate types for one extension; resolved by rank + OLE2 stream sniffing, see §6.4/§6.9 |
| `.xml` | `writer_ODT_FlatXML`, `calc_ODS_FlatXML`, `impress_ODP_FlatXML`, `writer_DocBook_File`, `calc_ADO_rowset_XML`, `calc_MS_Excel_2003_XML`, `writer_MS_Word_2003_XML`, `generic_XML`, `Unified_Office_Format_*` (`.uof`) | W, C, I | all via `XMLFilterDetect`/`OrcusFilterDetect`, disambiguated by root-element / namespace / `doctype:` sniff |

### 3.9 Unified/Uniform Office Format (Chinese national standard, UOF)

| Ext | Type name | Filter name | Kind | Notes |
|---|---|---|---|---|
| `.uot`, `.uof` | `Unified_Office_Format_text` | `UOF text` | W | `DetectService=XMLFilterDetect`, `ClipboardFormat=doctype:vnd.uof.text` (content sniff string). Implemented as an **XSLT filter**: `FilterService=com.sun.star.comp.Writer.XmlFilterAdaptor`, `UserData` references `uof2odf_text.xsl` / `odf2uof_text.xsl` stylesheets (`filters/UOF_text.xcu:23-24`). Flags `IMPORT EXPORT ALIEN 3RDPARTYFILTER` |
| `.uos`, `.uof` | `Unified_Office_Format_spreadsheet` | `UOF spreadsheet` | C | same mechanism |
| `.uop`, `.uof` | `Unified_Office_Format_presentation` | `UOF presentation` | I | same mechanism |

Confirmed present in this tree: **yes**, `uot`/`uos`/`uop`/`uof` all exist
(`types/Unified_Office_Format_text.xcu:21`, `..._spreadsheet.xcu:21`,
`..._presentation.xcu:21`).

### 3.10 Other alien import formats (Writer)

| Ext | Type name | DetectService | Notes |
|---|---|---|---|
| `.wpd` | `writer_WordPerfect_Document` | `com.sun.star.comp.Writer.WordPerfectImportFilter` | MIME `application/vnd.wordperfect` |
| `.602` | `writer_T602_Document` | `com.sun.star.comp.Writer.T602ImportFilter` | Czech T602 format |
| `.lwp` | `writer_LotusWordPro_Document` | `com.sun.star.comp.Writer.LotusWordProImportFilter` | |
| `.hwp` | `writer_MIZI_Hwp_97` | `com.sun.star.comp.hwpimport.HwpImportFilter` | Hangul Word Processor 97 |
| `.wps` | `writer_MS_Works_Document` | `com.sun.star.comp.Writer.MSWorksImportFilter` | MS Works word processor |
| `.abw`/`.zabw` | `writer_AbiWord_Document` | `com.sun.star.comp.Writer.AbiWordImportFilter` | |
| `.pages` | `writer_ApplePages` | (iWork import) | `Preferred=true`, MIME `application/x-iwork-pages-sffpages` |
| `.xml` | `writer_DocBook_File` | `XMLFilterDetect` | MIME `application/docbook+xml` |
| `.fb2`/`.zip` | `writer_FictionBook_2` | (import filter) | `Preferred=true` |
| `.epub` | `writer_EPUB_Document` | (empty `DetectService` — extension-only) | MIME `application/epub+zip` |
| `.pdb` | `writer_PalmDoc` **and** `writer_Plucker_eBook` | — | Same extension `.pdb` for two different Palm-OS ebook formats; disambiguated by content |
| `.psw` | `writer_PocketWord_File` | `MSWorksImportFilter` | Pocket Word |
| `.wri` | `writer_MS_Write` | `MSWorksImportFilter` | MS Write |
| `.mw`/`.mcw` | `writer_MacWrite` | `MWAWImportFilter` | |
| `.doc` | `writer_Mac_Word` | `MWAWImportFilter` | Mac Word via libmwaw, shares `.doc` |
| `.wps` | `writer_Mac_Works` | `MWAWImportFilter` | shares `.wps` |
| `.mwd` | `writer_Mariner_Write` | `MWAWImportFilter` | |
| `.wn`/`.nx^d` (sic, literal token in the XCU) | `writer_WriteNow` | `MWAWImportFilter` | |
| `.doc` | `writer_DosWord` | (own detector) | Word for DOS, shares `.doc` |
| `.lrf` | `writer_BroadBand_eBook` | — | Sony BBeB |

---

## 4. Detection algorithm (implementable, in precedence order)

This is the exact algorithm implemented by
`filter::config::TypeDetection::queryTypeByDescriptor`
(`filter/source/config/cache/typedetection.cxx:384-475`), which is what every load path
(UI "Open", API `loadComponentFromURL`, drag&drop, etc.) funnels through
(`sfx2/source/bastyp/fltfnc.cxx:494-568`). It runs in two phases:

### Phase A — "flat" (cheap, no or minimal I/O) candidate gathering

`impl_getAllFormatTypes` (`typedetection.cxx:772-843`):

1. **If the caller already specified an explicit `FilterName`** in the media descriptor,
   trust it outright — look it up, get its `Type`, done, skip everything else
   (`typedetection.cxx:409-417`).
2. Otherwise collect **every registered type that has at least one filter** as a
   baseline candidate list, each initially marked "no extension/pattern match"
   (`typedetection.cxx:791-802`). *(This is why a type registered with a literal `*`
   extension, like `MWAW_Text_Document`, still becomes a candidate for every file — it's
   in the baseline "every type" list, just ranked last unless nothing else matches; see
   §6.8.)*
3. Match the URL against every registered **`URLPattern`** wildcard
   (`filtercache.cxx:683-697`) — e.g. `private:factory/scalc*` for a brand-new blank Calc
   document. A pattern match short-circuits deep detection entirely later (step 6b).
4. Extract the **file-name extension**, lower-case it, and look it up in a
   precomputed extension→types hash map (`filtercache.cxx:700-716`, built at
   `filtercache.cxx:1037-1043`). All types registered for that literal extension string
   become extension-matched candidates, in the type-registration order (`Preferred`
   types are inserted first when the cache builds this map — see
   `filtercache.cxx:1024-1032`).
5. If the caller pre-supplied a `TypeName` or `DocumentService` hint, mark matching
   candidates as pre-selected (raises their sort priority) (`typedetection.cxx:834-842`).

Candidates are deduplicated by type name and sorted by `SortByPriority`
(`typedetection.cxx:317-339`), whose comparator, in order:

1. `bMatchByPattern` wins (URL-pattern matches beat everything)
2. `bMatchByExtension` wins (extension match beats no match)
3. **`getFlatTypeRank`** — a hand-maintained priority table hard-coded in
   `typedetection.cxx:141-297` (reproduced/explained in §5 below); types **not** in this
   table get the *highest* possible rank (`typedetection.cxx:307-309` — "unranked
   formats have higher priority... since they may be defined externally")
4. `bPreselectedByDocumentService`
5. alphabetical by type name (final tie-break, arbitrary but deterministic)

### Phase B — "deep" (content-based) verification, in that priority order

`impl_detectTypeFlatAndDeep` (`typedetection.cxx:915-1058`):

0. **Broken-ZIP pre-check**: if an interaction handler is available, peek the first two
   bytes; if they are `0x50 0x4B` (`"PK"`), try to open it as a ZIP package
   (`css::embed::StorageFormats::ZIP`) — if that throws `ZipIOException`, retry with
   `RepairPackage=true` and if that succeeds, ask the user whether to repair
   (`typedetection.cxx:846-912`, `927-987`). This runs **before** any type-specific
   detection so a slightly-corrupt `.docx`/`.odt`/`.xlsx` doesn't silently fall through
   to "unknown format".
1. For each candidate type, in the sorted order:
   a. If it matched by **URL pattern**, or deep detection is disallowed by the caller,
      **accept it immediately without further checks** (`typedetection.cxx:1013-1020`).
   b. Otherwise look up its `DetectService`. If empty, remember it as the
      "last chance" fallback (only the *first* such type is kept) and move on
      (`typedetection.cxx:1032-1043`).
   c. Otherwise instantiate the `DetectService` UNO component and call
      `XExtendedFilterDetection::detect()`, passing (and letting it mutate) the media
      descriptor. **The stream is opened and seeked to 0 before every call**
      (`impl_openStream` / `impl_seekStreamToZero`, `typedetection.cxx:1060-1082`,
      `1092-1096`).
   d. **The detect-service's returned type name is authoritative and is returned
      immediately** — it does not have to equal the candidate type that triggered the
      call (`typedetection.cxx:1045-1049`, `1163-1165`). This is the mechanism that lets
      one `DetectService` implementation (e.g. the OOXML `FormatDetector`) resolve
      *which one* of several duplicate type registrations is the true answer, based on
      file content, independent of which flat candidate happened to fire first.
2. If nothing matched and an `XInteractionHandler` is present, ask the user to pick a
   filter explicitly (`impl_askUserForTypeAndFilterIfAllowed`,
   `typedetection.cxx:1171-1227`).
3. If still nothing, fall back to the one "last chance" type recorded in step 1b, if any.

### 5. The hard-coded flat-type rank table (extract, Writer/Calc/Impress-relevant)

From `typedetection.cxx:141-297`, listed **from highest priority (top) to lowest**
(entries are listed array-first-to-last in source, which the code maps to
*highest-rank-first*, i.e. tried earliest):

```
writer8_template, writer8, calc8_template, calc8, impress8_template, impress8,
draw8_template, draw8, chart8, math8, writerglobal8_template, writerglobal8,
writerweb8_writer_template, StarBase,                       # ODF ZIP formats

writer_OOXML_Text_Template, writer_OOXML,
writer_MS_Word_2007_Template, writer_MS_Word_2007,
Office Open XML Spreadsheet Template, Office Open XML Spreadsheet,
MS Excel 2007 XML Template, MS Excel 2007 XML,
MS PowerPoint 2007 XML Template, MS PowerPoint 2007 XML AutoPlay,
MS PowerPoint 2007 XML,                                       # OOXML

Unified_Office_Format_text, Unified_Office_Format_spreadsheet,
Unified_Office_Format_presentation,                            # UOF

calc_StarOffice_XML_Calc, calc_StarOffice_XML_Calc_Template,
chart_StarOffice_XML_Chart, draw_StarOffice_XML_Draw, draw_StarOffice_XML_Draw_Template,
impress_StarOffice_XML_Impress, impress_StarOffice_XML_Impress_Template,
math_StarOffice_XML_Math, writer_StarOffice_XML_Writer,
writer_StarOffice_XML_Writer_Template,
writer_globaldocument_StarOffice_XML_Writer_GlobalDocument,
writer_web_StarOffice_XML_Writer_Web_Template,                 # StarOffice XML (sxw/sxc/sxi)

pdf_Portable_Document_Format,                                   # compressed text

writer_T602_Document, writer_WordPerfect_Document, writer_MS_Works_Document,
writer_MS_Word_97_Vorlage, writer_MS_Word_97,
writer_MS_Word_95_Vorlage, writer_MS_Word_95,
writer_MS_WinWord_60, writer_MS_WinWord_5,
MS Excel 2007 Binary,
calc_MS_Excel_97_VorlageTemplate, calc_MS_Excel_97,
calc_MS_Excel_95_VorlageTemplate, calc_MS_Excel_95,
calc_MS_Excel_5095_VorlageTemplate, calc_MS_Excel_5095,
calc_MS_Excel_40_VorlageTemplate, calc_MS_Excel_40,
calc_Pocket_Excel_File,
impress_MS_PowerPoint_97_Vorlage, impress_MS_PowerPoint_97_AutoPlay,
impress_MS_PowerPoint_97,
calc_Lotus, calc_QPro, calc_SYLK, calc_DIF, calc_dBase, Apache Parquet,
                                                                  # legacy binary

... (image/raster formats, out of scope) ...

writer_LotusWordPro_Document, writer_MIZI_Hwp_97,

writer_ODT_FlatXML, calc_ODS_FlatXML, impress_ODP_FlatXML, draw_ODG_FlatXML,
calc_ADO_rowset_XML, calc_MS_Excel_2003_XML, writer_MS_Word_2003_XML,
writer_DocBook_File, XHTML_File, svg_Scalable_Vector_Graphics,
math_MathML_XML_Math, generic_HTML, generic_XML,                # non-compressed XML

... (image ascii formats) ...
writer_Rich_Text_Format, writer_web_HTML_help, generic_JSON, generic_Markdown,

generic_Text,        # <-- catch-all; anything ranked below this line NEVER wins,
                      #     per the source comment at typedetection.cxx:285-286

writer_layout_dump_xml, writer_indexing_export, graphic_HTML,     # export-only
StarBaseReportChart, StarBaseReport, math_MathType_3x             # internal-only
```

Anything **not** in this table (e.g. `MWAW_Text_Document`, `Office Open XML
Presentation` and its Template/AutoPlay siblings, `writer_ApplePages`,
`writer_FictionBook_2`, most of the exotic import-only formats) is treated as *higher
priority than every ranked entry* (`typedetection.cxx:307-309`) — but this only matters
relative to other unranked types sharing the same extension-match/pattern-match bucket;
it never lets an unranked type jump ahead of a ranked type that also matched by
extension, because `bMatchByExtension` is compared **before** rank. Practically: this
rank table exists to arbitrate **between multiple types that all registered the same
extension** (see §3.6/§3.8), not to override extension matching itself.

> **Curiosity worth flagging for a reimplementation**: `Office Open XML Presentation`
> (the ISO/preferred pptx type) is conspicuously **absent** from the rank table, while
> its sibling family for Word (`writer_OOXML`) and Excel (`Office Open XML
> Spreadsheet`) **are** present. Since it's unranked, it is tried *before*
> `MS PowerPoint 2007 XML` for a `.pptx` file. This has no observable effect in
> practice because the shared OOXML `DetectService` returns a definitive, content-based
> answer regardless of which candidate triggered it (see §6.2) — but a from-scratch
> reimplementation that tries to replicate exact candidate-order semantics should be
> aware the table is not perfectly symmetric across the three modules.

---

## 6. Format-family deep dives

### 6.1 ODF (`.odt`/`.ods`/`.odp` and their `_template`/`sxw`-family cousins)

Type: `DetectService=com.sun.star.comp.filters.StorageFilterDetect`
(`filter/source/storagefilterdetect/filterdetect.cxx`).

Algorithm (`filterdetect.cxx:86-172`):

1. Open the input stream as a package storage:
   `comphelper::OStorageHelper::GetStorageFromInputStream()` (line 97). This internally
   requires the stream to parse as a **ZIP** archive.
2. Read the storage's `"MediaType"` UNO property (line 106) — this is populated by the
   package layer from **two places, cross-checked against each other**:
   * the **`mimetype`** entry in the ZIP (read as up to 1024 raw bytes, ASCII)
     (`package/source/zippackage/ZipPackage.cxx:473-494`)
   * the `MediaType` attribute of the `<manifest:file-entry manifest:full-path="/">`
     root entry inside **`META-INF/manifest.xml`** (`ZipPackage.cxx:517-529`)
   * If both exist and **disagree**, a `ZipIOException` is thrown ("mimetype conflicts
     with manifest.xml") (`ZipPackage.cxx:523-529`) — this is the "broken package"
     branch caught in `filterdetect.cxx:115-163`, which then falls back to whatever
     type was already flat-detected (extension-based) and offers the user a repair
     prompt.
   * If the manifest didn't parse (or its root media type is empty) but a `mimetype`
     stream is present and its value starts with `"application/vnd."`, that value is
     trusted directly (`ZipPackage.cxx:496-516`).
   * **Note**: per the ODF spec the `mimetype` entry must be the first ZIP entry and
     stored (uncompressed), but the code path above does not appear to hard-fail merely
     for compressed/non-first `mimetype` — it primarily relies on reading its bytes and
     cross-checking with the manifest. A from-scratch implementation aiming for
     bit-for-bit LO compatibility should still write it first/stored (interoperability
     with other readers depends on it) but the **read**-side tolerance is looser than
     the write-side requirement.
3. Map the resulting media-type string to an internal type name via a hard literal
   table, `getInternalFromMediaType()` (`filterdetect.cxx:43-77`):

   | MediaType | → internal type |
   |---|---|
   | `application/vnd.oasis.opendocument.text` | `writer8` |
   | `...text-template` | `writer8_template` |
   | `...text-web` | `writerweb8_writer_template` |
   | `...text-master` | `writerglobal8` |
   | `...text-master-template` | `writerglobal8_template` |
   | `...spreadsheet` | `calc8` |
   | `...spreadsheet-template` | `calc8_template` |
   | `...presentation` | `impress8` |
   | `...presentation-template` | `impress8_template` |
   | `application/vnd.sun.xml.writer` | `writer_StarOffice_XML_Writer` |
   | `application/vnd.sun.xml.writer.template` | `writer_StarOffice_XML_Writer_Template` |
   | `application/vnd.sun.xml.writer.web` | `writer_web_StarOffice_XML_Writer_Web_Template` |
   | `application/vnd.sun.xml.writer.global` | `writer_globaldocument_StarOffice_XML_Writer_GlobalDocument` |
   | `application/vnd.sun.xml.calc` | `calc_StarOffice_XML_Calc` |
   | `application/vnd.sun.xml.calc.template` | `calc_StarOffice_XML_Calc_Template` |
   | `application/vnd.sun.xml.impress` | `impress_StarOffice_XML_Impress` |
   | `application/vnd.sun.xml.impress.template` | `impress_StarOffice_XML_Impress_Template` |
   | anything unrecognized | empty string → no match |

   Note this table is the **sole authority**; if the manifest media type is a
   correctly-formed OASIS string that just isn't in this list (e.g. hypothetically a
   future ODF subtype) the detector returns nothing.
4. A quirk: under LibreOffice Online (`comphelper::LibreOfficeKit::isActive()`), a
   detected `draw8_template` is silently rewritten to `draw8` (`filterdetect.cxx:108-112`)
   — Draw-only, noted for completeness, not applicable to W/C/I.

So for ODF: **the `mimetype` ZIP-stream / manifest root `MediaType` is the single
ground truth; the file extension only participates in step 0 (whether this detector
even runs) and as a tie-break for `.stw` (writer template vs writer-web template,
§3.5) and `.xml` (flat variants).**

### 6.2 OOXML (`.docx`/`.dotx`/`.docm`, `.xlsx`/`.xltx`/`.xlsm`/`.xlsb`, `.pptx`/`.potx`/`.pptm`/`.ppsx`)

Type: `DetectService=com.sun.star.comp.oox.FormatDetector`
(`oox/source/core/filterdetect.cxx`, class `oox::core::FilterDetect`).

Algorithm (`FilterDetect::detect`, `filterdetect.cxx:493-570`):

1. Obtain an **unencrypted** package input stream
   (`extractUnencryptedPackage`, lines 396-472): try the raw stream as a ZIP first
   (`lclIsZipPackage` — opens `oox::ZipStorage` and checks `isStorage()`); if that
   fails, check whether it's an **OLE2 compound file** instead
   (`oox::ole::OleStorage`) — if so, this is an **encrypted OOXML package**
   (MS-OFFCRYPTO), and it attempts to decrypt it (see §8).
2. Confirm the (now-plaintext) stream is a ZIP (`ZipStorage::isStorage()`,
   line 511).
3. Parse, **in this exact fixed order** (comment at `filterdetect.cxx:547`: "Order is
   critical"):
   a. **`_rels/.rels`** — find the relationship whose `Type` is
      `http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument`
      (ECMA/transitional) or
      `http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument` (ISO
      strict — sets `maOOXMLVariant=ISO_Strict`, but that is then always treated the
      same as `ISO_Transitional`, `filterdetect.cxx:197-198`, `230-231` etc. — **strict
      OOXML has no distinct behavior, it degrades to transitional**). The `Target` of
      that relationship is resolved to an absolute in-package path (`maTargetPath`),
      e.g. `/word/document.xml` (`filterdetect.cxx:189-220`).
   b. **`word/settings.xml`** (Writer only) — look for
      `<w:compatSetting w:name="compatibilityMode" w:val="N"/>`; if `N > 12`, mark
      `maOOXMLVariant = ISO_Transitional` ("Word 2010+"), else it stays
      `ECMA_Transitional` ("Word 2007") (`filterdetect.cxx:164-174`, `529-532`). Parse
      failure (file absent = not a Writer doc) is caught and ignored, and then:
   c. **`xl/workbook.xml`** (Calc only, tried only if (b) failed/didn't apply) — look at
      `<fileVersion lowestEdited="N">`; if `N > 4`, mark `ISO_Transitional` ("Excel
      2010+") (`filterdetect.cxx:176-187`, `533-545`).
   d. **`[Content_Types].xml`** — for the part at `maTargetPath`, find its content type
      either via an `<Override PartName="...">` element or a `<Default
      Extension="...">` fallback (`filterdetect.cxx:320-337`), then map that
      content-type string (**plus** the already-known `maOOXMLVariant`, **plus** — for
      Word only — whether the file name ends in `.docm`) to a final type name via
      `getFilterNameFromContentType()` (lines 222-318). This call *is* the return
      value of `detect()`.

4. The content-type → type-name table (verbatim from source,
   `oox/source/core/filterdetect.cxx:222-318`):

   | Content-Type of the main part | + condition | → type name |
   |---|---|---|
   | `application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml` | not `.docm`, ISO | `writer_OOXML` |
   | (same) | not `.docm`, ECMA | `writer_MS_Word_2007` |
   | `application/vnd.ms-word.document.macroEnabled.main+xml` **or filename ends `.docm`** | ISO | `writer_OOXML_VBA` |
   | (same) | ECMA | `writer_MS_Word_2007_VBA` |
   | `...wordprocessingml.template.main+xml` or `...ms-word.template.macroEnabledTemplate.main+xml` | ISO | `writer_OOXML_Text_Template` |
   | (same) | ECMA | `writer_MS_Word_2007_Template` |
   | `...spreadsheetml.sheet.main+xml` | ISO | `Office Open XML Spreadsheet` |
   | (same) | ECMA | `MS Excel 2007 XML` |
   | `application/vnd.ms-excel.sheet.macroEnabled.main+xml` | ISO | `Office Open XML Spreadsheet VBA` |
   | (same) | ECMA | `MS Excel 2007 VBA XML` |
   | `...spreadsheetml.template.main+xml` or `...ms-excel.template.macroEnabled.main+xml` | ISO | `Office Open XML Spreadsheet Template` |
   | (same) | ECMA | `MS Excel 2007 XML Template` |
   | `application/vnd.ms-excel.sheet.binary.macroEnabled.main` | (either) | `MS Excel 2007 Binary` |
   | `...presentationml.presentation.main+xml` | (either — **no ISO/ECMA branch for Impress!**) | `MS PowerPoint 2007 XML` |
   | `application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml` | (either) | `MS PowerPoint 2007 XML VBA` |
   | `...presentationml.slideshow.main+xml` or `...ms-powerpoint.slideshow.macroEnabled.main+xml` | (either) | `MS PowerPoint 2007 XML AutoPlay` |
   | `...presentationml.template.main+xml` or `...ms-powerpoint.template.macroEnabled.main+xml` | (either) | `MS PowerPoint 2007 XML Template` |

   **Important asymmetry, verified in source**: Word and Excel each branch on
   `maOOXMLVariant` to choose between the "ISO-named" and "ECMA-named" type strings;
   **PowerPoint does not** — every `.pptx`/`.pptm`/`.ppsx`/`.potx` always maps to the
   *legacy-named* type (`MS PowerPoint 2007 XML*`) regardless of compatibility mode.
   Since the legacy type's filter and the ISO type's filter are functionally identical
   (same `FilterService`), this has no user-visible effect, but a reimplementation
   should not expect Impress to ever report `Office Open XML Presentation*` from
   *content* detection — that type name is reachable in this codebase only via
   extension-flat-matching before deep detection runs, or when a caller explicitly
   requests it.

5. **`.docx` vs `.docm` is decided by two independent signals that must agree**: the
   content-type string of the main document part, **and** (for Word specifically) a
   literal check of whether the file name ends in `.docm`
   (`bDocm = o3tl::endsWithIgnoreAsciiCase(rFileName, ".docm")`,
   `filterdetect.cxx:224`). The `bDocm` check is used to *force* the VBA/macro-enabled
   branch even if the content type is the plain (non-macro) one, and to *suppress* the
   plain-document branch when the name says `.docm`. **So for Writer, extension is not
   entirely irrelevant to OOXML type resolution — it participates as a tie-breaker
   alongside the content type.** For Calc/PowerPoint no equivalent filename check
   exists; those are purely content-type driven.

### 6.3 Legacy binary Word (`.doc`) — WW8/WW6/WW5/WW2/WW1, and RTF

Detector: `com.sun.star.text.FormatDetector`
(`sw/source/ui/uno/swdetect.cxx`, class `SwFilterDetect`), invoked once per surviving
candidate type in rank order (§5).

* **`writer_Rich_Text_Format`**: read first 5 bytes, require literal `{\rtf`
  (`swdetect.cxx:65-68`).
* **`writer_MS_WinWord_5`**: read first 3 bytes, match one of 4 exact byte triples
  (`swdetect.cxx:70-81`):

  | Bytes (hex) | Meaning |
  |---|---|
  | `9B A5 21` | WinWord 1 |
  | `9C A5 21` | PmWord 1 |
  | `DB A5 2D` | WinWord 2 |
  | `DC A5 65` | WinWord 6.0/95 as a **single-stream** (non-OLE2) file |

* **All other `.doc` candidates** (`writer_MS_Word_97`, `writer_MS_Word_97_Vorlage`,
  `writer_MS_Word_95`, `writer_MS_WinWord_60`): open as an **OLE2/CFB compound file**
  (`SotStorage`), then (`swdetect.cxx:93-138`):
  1. Require a stream named **`WordDocument`** to exist at the storage root
     (`aStorage->IsContained(u"WordDocument")`).
  2. If the candidate type name starts with `"writer_MS_Word_97"` (i.e. testing for
     WW8), additionally require the presence of a stream named **`0Table`** *or*
     **`1Table`** (`swdetect.cxx:101`). This is the WW8-vs-WW6/95 discriminator: WW8
     stores its formatted text piece table in `0Table`/`1Table`; WW6/95 does not.
  3. If specifically testing the **template** type `writer_MS_Word_97_Vorlage` and the
     file's extension isn't literally `.dot`, an extra check reads byte offset **10**
     of the `WordDocument` stream (the FIB's `Bits1`/flags byte) and requires bit
     `0x01` (the **`fDot`** flag) to be set (`swdetect.cxx:105-131`) — i.e. a `.doc`-named
     WW8 file is only accepted as a *template* type if the FIB explicitly marks it as a
     template. Files actually named `.dot` are always accepted regardless of this bit
     ("common practice to rename a .doc to .dot", comment at `swdetect.cxx:107`).
  4. `writer_MS_Word_95` uses the **same** "just needs `WordDocument`" check with *no*
     `0Table`/`1Table` requirement (falls into the generic `else` branch at
     `swdetect.cxx:83-138` since its name doesn't start with `writer_MS_Word_97`) — so
     in practice, **because `writer_MS_Word_97*` is ranked ahead of `writer_MS_Word_95`
     (§5)**, any WW8 file is claimed by the 97 detector first, and any pre-WW8
     OLE2-based `WordDocument` file that fails the 97 test (no table stream) falls
     through and is claimed by `writer_MS_Word_95`, never actually reaching
     `writer_MS_WinWord_60` in practice (it's ranked lower and `writer_MS_Word_95`'s
     bare-`WordDocument` test already always succeeds for such a file). **In this
     source, `writer_MS_WinWord_60` is therefore effectively dead code on the read
     path** — worth flagging (§10).

Additionally, `sw/source/filter/basflt/iodetect.cxx` implements a second, related check
used when *browsing* filters programmatically rather than during URL-open detection
(`SwIoSystem::IsValidStgFilter`, lines 97-146): it compares the storage's OLE
`SotClipboardFormatId` (from the CLSID→format mapping, see §6.5) against the filter's
expected format, with a special-case override for WW8/WW6 ("we cannot trust the
clipboard id anymore", line 117) that instead re-derives validity from
`0Table`/`1Table` presence exactly as above (`iodetect.cxx:128-142`).

### 6.4 Legacy binary Excel (BIFF) — `.xls`/`.xlt` family

Two different detectors are involved depending on BIFF generation:

* **BIFF8 / BIFF5 / BIFF5-or-95** (`calc_MS_Excel_97[_VorlageTemplate]`,
  `calc_MS_Excel_95[_VorlageTemplate]`, `calc_MS_Excel_5095[_VorlageTemplate]`):
  detector `com.sun.star.comp.calc.ExcelBiffFormatDetector`
  (`sc/source/ui/unoobj/exceldetect.cxx`, class `ScExcelBiffDetect`). Opens the file as
  an OLE2 storage and just checks for a named stream (`exceldetect.cxx:45-78`,
  `132-190`):
  * BIFF8 (`calc_MS_Excel_97*`) requires a stream named **`Workbook`**
    (`exceldetect.cxx:150`).
  * BIFF5 (`calc_MS_Excel_95*` and `calc_MS_Excel_5095*`) requires a stream named
    **`Book`** (`exceldetect.cxx:161`, `169`) — note **both** the "95" and "5095" type
    variants use the exact same `Book`-stream test; they are not actually
    distinguished by content at all in this code path (only by which candidate the
    rank table happens to try — effectively another dead branch, see §10).
* **BIFF2/3/4** (`calc_MS_Excel_40[_VorlageTemplate]`): **not** OLE2 — these are bare
  BIFF streams. `isExcel40()` (`exceldetect.cxx:84-123`) reads the first 4 bytes as a
  little-endian `(BOF-id: u16, BOF-recordSize: u16)` pair and requires:
  * `BOF-id` ∈ `{0x0009 (BIFF2), 0x0209 (BIFF3), 0x0409 (BIFF4), 0x0809 (BIFF5,
    accepted here too "some apps create such files")}`
  * `4 <= BOF-recordSize <= 16`
  * the stream must actually contain that many bytes after the header

* **Non-Excel spreadsheet legacy formats**, detector `com.sun.star.comp.calc.FormatDetector`
  (`sc/source/ui/unoobj/scdetect.cxx`, class `ScFilterDetect`), pure byte-pattern
  sniffing via a small custom bytecode interpreted by `detectThisFormat()`
  (`scdetect.cxx:50-143`):
  * `calc_Lotus` (`.wk1`/`.wks`/`.123`): three alternative byte patterns for Lotus
    1/1A/2, Lotus ≥9.7, Lotus >3 (`scdetect.cxx:54-70`).
  * `calc_QPro` (`.wb2`): pattern at `scdetect.cxx:72-77`.
  * `calc_SYLK` (`.slk`/`.sylk`): literal `ID;` followed by one of `P`/`N`/`E`
    (`scdetect.cxx:97-101`).
  * `calc_DIF` (`.dif`): literal `TABLE` then `0,1` then a `"` with CR/LF variations
    (`scdetect.cxx:79-95`).
  * `calc_dBase` (`.dbf`): first byte must be one of a fixed set of dBase version
    markers (`0x03,0x04,0x05,0x30,0x31,0x43,0xB3,0x83,0x8B,0x8E,0xF5`), then header
    length/record-count/record-size sanity checks, then the header must end on a
    32-byte boundary with byte `0x0D` (`scdetect.cxx:194-261`).

### 6.5 OLE2/CFB container basics (applies to all legacy `.doc`/`.xls`/`.ppt`)

* **Magic bytes for the OLE2 Compound File Binary format itself** (used by
  `Storage::IsStorageFile`, `sot/source/sdstor/stg.cxx:304-`):
  `cStgSignature[8] = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 }`
  (`sot/source/sdstor/stgelem.cxx:33`) — the well-known universal OLE2/CFB header
  signature.
* **CLSIDs** used by LibreOffice's own *export/embedding* code to stamp the root
  storage's class ID (`include/filter/msfilter/classids.hxx`) — these are written on
  export but **not** used as the primary read-side detection signal (detection instead
  uses stream-name sniffing, §6.3/§6.4):

  | Format | CLSID (first 4 dashed groups omitted where standard) |
  |---|---|
  | Word 8/97 document (`MSO_WW8_CLASSID`) | `00020906-0000-0000-C000-000000000046` |
  | Excel 5 (`MSO_EXCEL5_CLASSID`) | `00020810-0000-0000-C000-000000000046` |
  | Excel 8/97 (`MSO_EXCEL8_CLASSID`) | `00020820-0000-0000-C000-000000000046` |
  | Excel 8/97 chart (`MSO_EXCEL8_CHART_CLASSID`) | `00020821-0000-0000-C000-000000000046` |
  | PowerPoint 8/97 (`MSO_PPT8_CLASSID`) | `64818D10-4F9B-11CF-86EA-00AA00B929E8` |
  | PowerPoint 8/97 slide (`MSO_PPT8_SLIDE_CLASSID`) | `64818D11-4F9B-11CF-86EA-00AA00B929E8` |
  | MS Equation 2 | `00021700-0000-0000-C000-000000000046` |
  | MS Equation 3 | `0002CE02-0000-0000-C000-000000000046` |

  (`include/filter/msfilter/classids.hxx:12-42`.) A from-scratch reimplementation that
  wants to detect legacy binary formats **should** still check these CLSIDs as a fast
  first pass (they are the semantically "correct" signal per the OLE2 spec) even though
  LibreOffice's own read path prefers the more failure-tolerant stream-name checks — LO
  falls back to stream-name checks specifically *because* real-world files are known to
  have missing/wrong CLSIDs (see comment "there are Excel Docs w/o ClipBoardId!... and
  also WinWord Docs w/o ClipBoardId!" at `sw/source/filter/basflt/iodetect.cxx:126-127`).
* `SotStorage::GetFormatID()` (`sot/source/sdstor/storage.cxx:653-677`) converts a
  storage's `MediaType` UNO property (not the raw CLSID bytes directly — this is for
  the **UCB storage wrapper**, which itself derived that MediaType from the CLSID via
  `GetFormatId_Impl`/`GetClassId_Impl` in `sot/source/sdstor/ucbstorage.cxx:305-408`,
  though only for LibreOffice's own SO5/6/7/8 class IDs — MS class IDs are not present
  in that particular switch, consistent with §6.3's stream-name-based approach being
  primary for MS formats).

### 6.6 ODF flat-XML (`.fodt`/`.fods`/`.fodp`) and other content-sniffed XML types

Detector: `com.sun.star.comp.filters.XMLFilterDetect`
(`filter/source/xmlfilterdetect/filterdetect.cxx`).

Algorithm (`filterdetect.cxx:75-207`):

1. Read up to 4000 characters of text from the start of the stream, auto-sensing
   UTF-8/UTF-16 BOM via `SvStream::StartReadingUnicodeText` (lines 111-141).
2. If the text does not start with `<?xml`, fall back to checking the content's
   `MediaType`/`Content-Type` UCB property — accept only `application/xml` or anything
   ending `+xml` per RFC 3023 (`IsMediaTypeXML`, lines 54-71); otherwise bail out (not
   XML at all).
3. Enumerate **every registered type** via the `com.sun.star.document.TypeDetection`
   service, filter down to those whose `DetectService` equals this detector's own
   implementation name (`com.sun.star.comp.filters.XMLFilterDetect`) **and** whose
   `ClipboardFormat` starts with the literal prefix `"doctype:"` (lines 170-199).
4. For each such candidate, strip the `doctype:` prefix from its `ClipboardFormat` and
   do a plain **substring search** for that string inside the 4000-character prefix
   read in step 1 (`supportedByType`, lines 40-52). First match wins.

Concretely, the substrings being searched for (all pulled straight from the
`ClipboardFormat` properties of the corresponding type XCUs):

| Type | Substring searched for |
|---|---|
| `writer_ODT_FlatXML` | `office:mimetype="application/vnd.oasis.opendocument.text"` |
| `calc_ODS_FlatXML` | `office:mimetype="application/vnd.oasis.opendocument.spreadsheet"` |
| `impress_ODP_FlatXML` | `office:mimetype="application/vnd.oasis.opendocument.presentation"` |
| `Unified_Office_Format_text` | `vnd.uof.text` |
| `Unified_Office_Format_spreadsheet` | `vnd.uof.spreadsheet` |
| `Unified_Office_Format_presentation` | `vnd.uof.presentation` |

This is a **plain substring search over raw decoded text**, not real XML parsing —
important for a reimplementation: it will match the string anywhere in the first ~4000
characters, e.g. even inside a comment, and is not attribute-order- or
whitespace-sensitive as an XML parser would be, but it also isn't a validating parse (a
non-well-formed file containing the right substring would still "match").

### 6.7 HTML / plain text / CSV / Markdown

Detector: `com.sun.star.comp.filters.PlainTextFilterDetect`
(`filter/source/textfilterdetect/filterdetect.cxx`).

* **HTML** (`generic_HTML`/`calc_HTML`): a small hand-rolled state machine
  (`IsHTMLStream`, lines 42-132) that requires the stream, after BOM-stripping, to
  start with `<` immediately followed by either `!` (DOCTYPE/comment) or a token that
  `GetHTMLToken()` (from `svtools/htmltokn.h`) recognizes as a real HTML tag name (e.g.
  `html`, `head`, `body`, …) or a `<?...?>` processing instruction. Whitespace before
  the tag name is *not* tolerated as leading content — the very first character must be
  `<`. Once accepted, the concrete filter chosen is: `calc_HTML_WebQuery` if
  `DocumentService` is Calc's or `TypeName=="calc_HTML"`; `HTML (StarWriter)` if
  Writer's; else generic `HTML` (lines 146-158).
* **Plain text** (`generic_Text`): if the stream is gzip-compressed (checked via
  `ZCodec::AttemptDecompression`), it is transparently decompressed first and the `.gz`
  suffix stripped from the tracked URL (lines 182-198). Filter choice: Calc's text
  filter if `DocumentService` is Calc's, Writer's if Writer's, else by extension —
  `csv`/`tsv`/`tab`/`xls`/`*.csv.gz` → Calc, everything else → Writer (lines 209-216).
  **Note `xls` is in the extension list here** — this is the generic-text fallback path
  used when a `.xls` file fails *all* BIFF/OLE2 detection (i.e. it's actually a
  CSV/tab-delimited file someone renamed to `.xls`).
* **Markdown** (`generic_Markdown`): Writer's markdown filter is used if
  `DocumentService` is Writer's, or if the extension is `md`/`markdown`; otherwise no
  match (lines 160-174). No content sniffing at all — purely doc-service/extension
  driven despite being reached through the "deep detection" mechanism.

### 6.8 The literal `*` (wildcard-as-extension) trick

`MWAW_Text_Document`, `MWAW_Spreadsheet`, `MWAW_Presentation` register `Extensions`
containing the single literal character `*` (`filter/source/config/fragments/types/MWAW_Text_Document.xcu:14-16`,
similarly for the other two). Because the extension-registration map is a plain
hash-map keyed by the literal (lower-cased) extension string
(`filtercache.cxx:1037-1043`) and lookup is `m_lExtensions2Types.find(sExtension)`
(`filtercache.cxx:704`) with no wildcard expansion, **a real file whose extension is the
single character `*` would be needed for this to flat-match by extension** — which
essentially never happens. Instead, these types become detection candidates purely
through the **"every type with a filter" baseline pass**
(`typedetection.cxx:791-802`), where they get `bMatchByExtension=false`. Since they are
also **absent from the rank table** (§5), they receive the *maximum* unranked priority
— but that only elevates them above other *unranked, no-extension-match* types; they
still sort behind every type that did match by extension or pattern
(`SortByPriority`, `typedetection.cxx:317-339`, extension-match is compared before
rank). Net effect: these three MWAW catch-all types are tried, deep, for **any** file
whose real extension didn't match anything more specific — a legacy-Mac-format sniffer
of last resort. Their `DetectService`s (`com.sun.star.comp.Writer.MWAWImportFilter` /
`...Calc.MWAWCalcImportFilter` / `...Impress.MWAWPresentationImportFilter`, all backed
by the `libmwaw` library) presumably do their own internal magic-byte sniffing across
many legacy Mac formats; this investigation did not descend into `libmwaw`'s own format
sniffing (out of tree / external library, not in this repository).

### 6.9 Summary table: concrete magic-byte / stream-name signatures found

| Format | Signal | Value | Citation |
|---|---|---|---|
| Any ZIP-based container (ODF, OOXML, sxw/sxc/sxi, epub, …) | first 2 bytes | `0x50 0x4B` (`"PK"`) | `typedetection.cxx:866` (used only for the pre-check; real ZIP validity is then verified by actually opening it as a `ZipPackage`) |
| OLE2/CFB compound file (doc/xls/ppt) | first 8 bytes | `D0 CF 11 E0 A1 B1 1A E1` | `sot/source/sdstor/stgelem.cxx:33` |
| RTF | first 5 bytes | `{\rtf` (ASCII `7B 5C 72 74 66`) | `sw/source/ui/uno/swdetect.cxx:68` |
| WinWord 1 | first 3 bytes | `9B A5 21` | `swdetect.cxx:78` |
| PmWord 1 | first 3 bytes | `9C A5 21` | `swdetect.cxx:79` |
| WinWord 2 | first 3 bytes | `DB A5 2D` | `swdetect.cxx:80` |
| WinWord 6.0/95 (single-stream) | first 3 bytes | `DC A5 65` | `swdetect.cxx:81` |
| Word 6/95/97 (OLE2) | OLE stream name | `WordDocument` present | `swdetect.cxx:98` |
| Word 97/2000/XP/2003 (WW8) | OLE stream name | `WordDocument` **and** (`0Table` **or** `1Table`) | `swdetect.cxx:101` |
| `.dot` template detection (WW8) | byte at offset 10 in `WordDocument` stream, bit `0x01` | `fDot` FIB flag | `swdetect.cxx:118-129` |
| Excel 97/2000/XP/2003 (BIFF8) | OLE stream name | `Workbook` present | `sc/source/ui/unoobj/exceldetect.cxx:150` |
| Excel 5.0/95 (BIFF5) | OLE stream name | `Book` present | `exceldetect.cxx:161,169` |
| Excel 2/3/4 (BIFF2-4) | first 4 bytes, `u16 BOF-id` | `0x0009`/`0x0209`/`0x0409`/(`0x0809`) | `exceldetect.cxx:104-107` |
| PowerPoint 97-2003 | OLE stream name | `PowerPoint Document` present | `sd/source/ui/unoidl/sddetect.cxx:82` |
| Lotus 1/1A/2, ≥9.7, >3 | custom byte-pattern bytecode | see `scdetect.cxx:54-70` | |
| Quattro Pro | custom byte-pattern | `scdetect.cxx:72-77` | |
| SYLK | literal | `ID;` + one of `P`/`N`/`E` | `scdetect.cxx:97-101` |
| DIF | literal | `TABLE` … `0,1` … `"` | `scdetect.cxx:79-95` |
| dBase | first byte | one of `03,04,05,30,31,43,B3,83,8B,8E,F5` + header sanity | `scdetect.cxx:194-261` |
| ODF (any) | ZIP `mimetype` entry content, cross-checked with `META-INF/manifest.xml` root `MediaType` | literal MIME string | `package/source/zippackage/ZipPackage.cxx:473-533` |
| OOXML (any) | `[Content_Types].xml` content-type of the part named by `_rels/.rels`'s officeDocument relationship | literal content-type string | `oox/source/core/filterdetect.cxx:222-337` |
| OOXML `.docm` disambiguation | file name suffix | `.docm` (case-insensitive) | `oox/source/core/filterdetect.cxx:224` |
| Flat ODF / UOF / DocBook | raw-text substring search, first ~4000 chars | `doctype:`-prefixed `ClipboardFormat` string | `filter/source/xmlfilterdetect/filterdetect.cxx:170-199` |
| HTML | tag-name state machine on first ~4096 chars | must start `<` + known HTML token or `<!`/`<?` | `filter/source/textfilterdetect/filterdetect.cxx:42-132` |

---

## 7. Macro-enabled / template variants — confirmed existence & detection summary

| Variant | Exists in this tree? | Detected via |
|---|---|---|
| `.docx` / `.dotx` / `.docm` | Yes | OOXML content-type + `.docm` filename check (§6.2) |
| `.xlsx` / `.xltx` / `.xlsm` / `.xlsb` | Yes | OOXML content-type (xlsb has its own distinct content-type, `application/vnd.ms-excel.sheet.binary.macroEnabled.main`, no filename check needed) |
| `.pptx` / `.potx` / `.potm` / `.ppsx` / `.ppsm` / `.pps` / `.pot` / `.ppa` | `.pptx/.potx/.potm/.ppsx` confirmed as OOXML types (§3.4); `.pps`/`.pot` confirmed as **legacy binary** types (§3.6, `impress_MS_PowerPoint_97_AutoPlay`/`_Vorlage`); **`.ppsm` and `.ppa` were not found as registered extensions anywhere in `filter/source/config/fragments/{types,filters}`** — see §10 |
| ODF templates: `.ott`/`.ots`/`.otp` | Yes (§3.1) | ZIP `mimetype`/manifest media type ending in `-template` |
| ODF flat: `.fodt`/`.fods`/`.fodp` | Yes (§3.1, §6.6) | raw-text `doctype:` substring match; note these types' `Extensions` list is actually `fodt odt xml` / `fods ods xml` / `fodp odp xml` (i.e. the *non-flat* extension is also listed, purely to give the flat-XML type a chance to be tried as a fallback candidate for those extensions too) |
| Legacy StarOffice ZIP/XML: `.sxw`/`.sxc`/`.sxi` (+ `.stw`/`.stc`/`.sti` templates, `.sxg` global) | Yes (§3.5) | same `StorageFilterDetect` mechanism as ODF, different media-type table |
| UOF: `.uot`/`.uos`/`.uop` (+ shared `.uof`) | Yes (§3.9) | confirmed present, contra the prompt's "if present" hedge |

---

## 8. Password / encryption detection

This investigation identified **where** encryption is detected/handled for each
container family; it deliberately did not descend into the cryptographic
implementations themselves (out of scope per the task).

* **Filter-flag declaration**: a filter's `Flags` include `ENCRYPTION` (password-to-open
  supported), `PASSWORDTOMODIFY` (password-to-modify supported), and/or
  `GPGENCRYPTION` (OpenPGP-based encryption supported, ODF-only). These are static
  capability declarations in the XCU, not detection logic — see the flag table in §2 and
  per-format notes in §3 (e.g. ODF formats support all three; OOXML formats support
  `ENCRYPTION`+`PASSWORDTOMODIFY`+`SUPPORTSSIGNING` but not `GPGENCRYPTION`; legacy
  binary Word/Excel 97 support `ENCRYPTION`+`PASSWORDTOMODIFY`; `.docx`/`.xlsx`
  *template* filters conspicuously drop `ENCRYPTION` entirely,
  `filters/MS_Word_2007_XML_Template.xcu:19`).
* **OOXML (MS-OFFCRYPTO) encrypted packages**: an "encrypted `.docx`/`.xlsx`/`.pptx`"
  is, at the container level, actually an **OLE2/CFB compound file** (not directly a
  ZIP) holding the real ZIP payload encrypted inside a stream named
  **`EncryptedPackage`**, alongside encryption metadata. Detected/handled in
  `oox::core::FilterDetect::extractUnencryptedPackage()`
  (`oox/source/core/filterdetect.cxx:396-472`): if the raw stream doesn't parse as a
  ZIP, it's tried as an OLE2 storage (`oox::ole::OleStorage`); if that succeeds,
  `oox::crypto::DocumentDecryption::readEncryptionInfo()`
  (`oox/source/crypto/DocumentDecryption.cxx:95-`) reads a stream named
  **`\006DataSpaces/DataSpaceMap`** (MS-OFFCRYPTO §2.1.6.1) to locate the encryption
  data-space, and the actual ciphertext is read from the **`EncryptedPackage`** stream
  (`DocumentDecryption.cxx:204-205`). Password recovery tries the well-known Excel
  "workbook protection" default password **`"VelvetSweatshop"`** before prompting the
  user (`filterdetect.cxx:427-428`).
* **ODF encryption**: declared per-stream in **`META-INF/manifest.xml`** via
  `<manifest:encryption-data>` / `<manifest:algorithm>` elements
  (`package/source/manifest/ManifestImport.cxx:340-423`). Recognized algorithms include
  Blowfish-CFB (`BLOWFISH_NAME`/`BLOWFISH_URL` →
  `xml::crypto::CipherID::BLOWFISH_CFB_8`) and several AES variants — AES-128/192/256-CBC
  and AES-GCM-128/192/256 (`ManifestImport.cxx:186-219`). **GPG/OpenPGP** encryption
  (the `GPGENCRYPTION` filter flag) is declared via `<loext:PGPData>`/
  `<manifest:PGPData>` (and `PGPKeyID`/`PGPKeyPacket`) elements in the same manifest
  (`ManifestImport.cxx:296-298`, `384-423`). Whether a package is encrypted at all is
  therefore something the ODF `StorageFilterDetect` path (§6.1) would only learn about
  indirectly (via the manifest, which the underlying package/storage layer parses); the
  `StorageFilterDetect::detect()` code itself does not special-case encryption beyond
  the generic "broken package" repair-prompt path shared with all ZIP corruption.
* **Legacy binary (BIFF8 `.xls`, WW8 `.doc`) encryption**: not investigated in depth;
  the `ENCRYPTION`/`PASSWORDTOMODIFY` flags are declared on `MS Word 97`
  (`filters/MS_Word_97.xcu:19`) and `MS Excel 97` filters, implying RC4/CryptoAPI
  encryption support lives inside the WW8/BIFF import filters themselves (in
  `sw/source/filter/ww8/` and `sc/source/filter/excel/`), not in the type-detection
  layer — out of scope for this document.

---

## 9. Notable inconsistencies in the `Preferred` flag across the three modules

Both a **type**-level `Preferred` boolean property and a **filter**-level `PREFERRED`
flag bit exist and are set independently; they usually agree but not always:

* `writer8` type: `Preferred=true` (`types/writer8.xcu:23`); `writer8` **filter**'s
  `Flags` also includes the `PREFERRED` token
  (`filters/writer8.xcu:19` — full list: `IMPORT EXPORT TEMPLATE OWN DEFAULT PREFERRED
  ENCRYPTION PASSWORDTOMODIFY GPGENCRYPTION`). Both properties agree here.
* `calc8` **filter**'s `Flags = IMPORT EXPORT TEMPLATE OWN DEFAULT ENCRYPTION
  PASSWORDTOMODIFY GPGENCRYPTION` (`filters/calc8.xcu:19`) — **no `PREFERRED` token**,
  unlike `writer8`'s filter. This looks like either an intentional omission (Calc's
  `.ods` format has no competing filter needing disambiguation, so the flag is moot) or
  a copy-paste inconsistency between the three modules' XCU fragments — flagged here
  as-is since it is directly observable in the source rather than inferred.
* `writer_Rich_Text_Format` **type**: `Preferred=false` (`types/writer_Rich_Text_Format.xcu:23`)
  yet its **filter**'s `Flags` includes the `PREFERRED` token
  (`filters/Rich_Text_Format.xcu:19`). These two "preferred" concepts are read by
  different code paths (type-level `Preferred` feeds `detectFlatForURL`'s
  extension-map ordering, `filtercache.cxx:1024-1032`; filter-level `PREFERRED` flag is
  read directly as a bitmask by consumers such as
  `TypeDetection::impl_checkResultsAndAddBestFilter`,
  `typedetection.cxx:511-537`, `typedetection.cxx:531`) — a reimplementation must keep
  these two properties **separate**, they are not redundant encodings of the same fact.

---

## 10. Open questions / ambiguities (could not fully resolve from static reading)

1. **`.ppsm` and `.ppa` (PowerPoint macro-enabled slideshow / add-in)**: not found as
   registered `Extensions` values anywhere under
   `filter/source/config/fragments/{types,filters}` in this checkout. `.ppsm` may be
   handled generically by the `.pptm`/OOXML VBA content-type path once opened (since
   OOXML detection is largely content-type driven for Impress, an actual `.ppsm` file
   might still be *readable* if double-clicked, arriving at
   `MS PowerPoint 2007 XML VBA` via content sniffing even without an extension
   registration) but there is no dedicated extension→type mapping, so a `.ppsm` file
   would not flat-match anything by extension and would only be caught by the
   MWAW-style "every registered type" fallback bucket, or fail outright depending on
   how the OS/shell association is configured. Not confirmed either way by static
   reading; would need a running instance or the full `officecfg` merge output to be
   sure. `.ppa`/`.ppam` (PowerPoint add-ins) appear entirely unhandled by
   Impress — plausible, since add-ins aren't really "presentations" in LO's model, but
   not explicitly confirmed absent-on-purpose vs. simply out of scope for this LO
   version.
2. **`writer_MS_WinWord_60` and one of `calc_MS_Excel_95`/`calc_MS_Excel_5095`
   appear to be dead code on the detection path** (§6.3, §6.4) — their registered
   `DetectService` never actually distinguishes them from a higher-ranked sibling type
   that is tried first and whose test is a strict superset/independent match. This is
   an observation about *this specific source snapshot*'s detector implementations,
   not a guess — but whether this is intentional (the types exist mainly for
   *export*/Save-As purposes, where the user explicitly picks the filter and detection
   is irrelevant) or a genuine latent bug was not something I could determine from
   static reading alone.
3. **ODF `mimetype`-must-be-first-and-stored requirement**: confirmed this is required
   by the ODF spec and that LibreOffice's *writer* honors it
   (`package/source/zipapi/ZipFile.cxx` references to `STORED` and to ODF's requirement
   near line 1051-1056), but I did not find an explicit hard-fail in the *reader* path
   solely for a compressed or non-first `mimetype` entry — the reader instead relies on
   the manifest cross-check (§6.1). A fully faithful C# reimplementation should
   probably enforce the spec on write and be lenient on read, mirroring what the code
   appears to do, but this is inferred from absence-of-evidence rather than an explicit
   confirmed guard, so treat with appropriate caution.
4. **OOXML "ISO Strict" (`http://purl.oclc.org/ooxml/officeDocument`) handling**: the
   source explicitly always downgrades `ISO_Strict` to the same handling as
   `ISO_Transitional` (`oox/source/core/filterdetect.cxx:231`, `243`, `256`, etc., each
   with a comment "Not supported, map to ISO transitional"). Strict OOXML is therefore
   detected as a distinct enum value internally but produces **identical** output type
   names to transitional-ISO — i.e., there is no way to tell, from the detected *type
   name* alone, whether a `.docx` was Strict or Transitional ISO OOXML. Only the
   relationship namespace at parse time (`_rels/.rels`) distinguishes them, and that
   information is discarded once `getFilterNameFromContentType()` returns.
5. **Exact list of file extensions is config-merge-dependent**: this document reflects
   the fragments as authored under `filter/source/config/fragments/`. LibreOffice's
   build merges these `.xcu` fragments (plus any product/branding overlays under
   `officecfg/registry/data` or vendor patches) into the final runtime configuration; I
   did not find evidence of W/C/I-relevant overlays altering these specific
   extensions/types in this checkout's `officecfg/registry/data/org/openoffice/TypeDetection/`
   (only `UISort.xcu`, a display-order file, exists there), so I'm confident the
   fragment-level facts above are the effective runtime facts for this build, but a
   downstream vendor build could in principle patch them.

---

## 11. Key source files for a C# reimplementation (index)

| Concern | File(s) |
|---|---|
| Flag enum & meaning | `include/comphelper/documentconstants.hxx` |
| Flag string↔enum table | `filter/source/config/cache/constant.hxx`, `filter/source/config/cache/filtercache.cxx:1821-1849` |
| Type/filter XCU sources (ground truth for the big table) | `filter/source/config/fragments/types/*.xcu`, `filter/source/config/fragments/filters/*.xcu` |
| Core detection algorithm | `filter/source/config/cache/typedetection.cxx` |
| Flat (extension/pattern) matching | `filter/source/config/cache/filtercache.cxx:662-719`, `:980-1050` |
| SfxFilterMatcher bridge (UI open path) | `sfx2/source/bastyp/fltfnc.cxx:494-568` |
| ODF media-type detection | `filter/source/storagefilterdetect/filterdetect.cxx` |
| ODF ZIP mimetype/manifest cross-check | `package/source/zippackage/ZipPackage.cxx:460-533` |
| OOXML content-type detection | `oox/source/core/filterdetect.cxx` |
| OOXML encryption (MS-OFFCRYPTO) | `oox/source/crypto/DocumentDecryption.cxx` |
| ODF encryption/manifest parsing | `package/source/manifest/ManifestImport.cxx` |
| Legacy Word (.doc/.rtf) detection | `sw/source/ui/uno/swdetect.cxx`, `sw/source/filter/basflt/iodetect.cxx` |
| Legacy Excel (BIFF) detection | `sc/source/ui/unoobj/exceldetect.cxx`, `sc/source/ui/unoobj/scdetect.cxx` |
| Legacy PowerPoint / image / CGM detection | `sd/source/ui/unoidl/sddetect.cxx` |
| Flat-XML / doctype-substring detection | `filter/source/xmlfilterdetect/filterdetect.cxx` |
| Plain-text/HTML/CSV/Markdown detection | `filter/source/textfilterdetect/filterdetect.cxx` |
| OLE2/CFB signature & CLSIDs | `sot/source/sdstor/stgelem.cxx:33`, `include/filter/msfilter/classids.hxx`, `sot/source/sdstor/storage.cxx:653-677`, `sot/source/sdstor/ucbstorage.cxx:305-408` |
