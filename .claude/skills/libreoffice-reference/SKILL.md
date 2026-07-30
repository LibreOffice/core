---
name: libreoffice-reference
description: Produce reference renderings and text extractions from documents using headless LibreOffice (soffice). Use whenever you need ground-truth output for a .docx/.doc/.xlsx/.xls/.pptx/.ppt/.odt/.ods/.odp file — reference PDFs, per-page PNG images, or reference text — or when a Paperless output needs something to be compared against. Also use when a soffice --convert-to invocation misbehaves (missing output, wrong page count, blank pages, silently overwritten files).
---

# Producing reference output with headless LibreOffice

LibreOffice is the oracle for Paperless. This skill covers driving an **installed**
`soffice` binary in headless mode to generate ground truth.

**Do not build LibreOffice from this repository's C++ sources.** A full build takes hours
and is unnecessary — a distribution package renders identically for comparison purposes.

## Before you start: verify the environment

Run `scripts/check-env.sh`. It fails loudly if anything is missing, which is much better
than discovering it three hundred diffs later.

```bash
.claude/skills/libreoffice-reference/scripts/check-env.sh
```

It checks four things, in order of how badly each will mislead you:

1. **`soffice` exists and reports a version.** Record the version in any results you
   save — LibreOffice's layout changes between releases, so a reference PDF is only
   meaningful alongside the version that produced it.

2. **The application modules are installed.** This is the trap. `libreoffice-core` alone
   installs `soffice` and it runs, but every conversion fails with the maximally
   unhelpful `Error: source file could not be loaded` — including for a plain `.txt`
   file. You need `libreoffice-writer`, `libreoffice-calc` and `libreoffice-impress`:

   ```bash
   apt-get install -y --no-install-recommends \
       libreoffice-writer libreoffice-calc libreoffice-impress
   ```

3. **Metric-compatible fonts are installed.** The single largest source of false
   differences. Office documents ask for Calibri, Cambria, Arial and Times New Roman.
   If those resolve to DejaVu faces, advance widths change, so line breaks change, so
   pagination changes, and *every page after the first diverges* — which looks exactly
   like a Paperless layout bug but is not one.

   ```bash
   apt-get install -y --no-install-recommends \
       fonts-crosextra-carlito fonts-crosextra-caladea
   ```

   Verify with `fc-match Calibri` → must print **Carlito**, and `fc-match Cambria` →
   **Caladea**. Arial → Liberation Sans and Times New Roman → Liberation Serif come from
   `fonts-liberation`, normally already present.

4. **A PDF rasteriser exists.** `pdftoppm` from `poppler-utils`. Only needed for image
   comparison, not for text.

## The four gotchas that will cost you an hour each

These are all verified behaviours of `soffice --convert-to`, not folklore.

### 1. Output files silently overwrite each other

`--convert-to` names its output after the input's **stem only**, discarding the
directory and the original extension. So this looks like it converts nine files:

```bash
# BROKEN: produces exactly one t.pdf. Eight results are silently lost.
soffice --headless --convert-to pdf --outdir out/ a/t.docx b/t.xlsx c/t.pptx ...
```

Every input named `t.*` collapses onto `out/t.pdf`. There is no warning.

**Always give each conversion its own output directory**, which is what
`scripts/lo-convert.sh` does. Renaming inputs to unique stems also works but is more
fragile.

### 2. Concurrent runs need separate user profiles

Two `soffice` processes sharing a profile directory interfere: the second detects the
first and delegates its work to it, so its output never appears where you asked. Always
pass a private profile:

```bash
soffice --headless -env:UserInstallation=file:///abs/path/to/private/profile ...
```

The path must be an absolute `file://` URL. This is also why the scripts here never
assume `$HOME` is writable or unshared.

### 3. `--convert-to png` gives you only the first page

For Writer and Calc it renders page 1; for Impress, slide 1. There is no option to make
it emit all of them.

**For multi-page reference images, always go via PDF**, then rasterise:

```bash
soffice --headless ... --convert-to pdf --outdir out/ input.docx
pdftoppm -r 150 -png out/input.pdf out/page   # -> out/page-1.png, page-2.png, ...
```

Single-slide PNG export is still useful when you want LibreOffice's own rasterisation
rather than poppler's, e.g. `png:impress_png_Export:{"PixelWidth":{"type":"long","value":"1280"}}`.

### 4. Rasterised output is deterministic — exploit it

Verified: converting the same input twice and rasterising both gives **byte-identical
PNGs** (same MD5). So reference images can be committed and checksummed, and a changed
checksum means something genuinely changed.

The PDFs themselves are *not* byte-identical — they embed a creation timestamp — so
checksum the PNGs, never the PDFs.

## Doing the work

### Reference PDF and page images

```bash
# One document -> reference PDF + per-page PNGs at 150 DPI
.claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --png \
    --dpi 150 --outdir /tmp/ref path/to/document.docx
```

### A whole corpus

```bash
# Batches into as few soffice invocations as correctness allows.
.claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --png \
    --outdir /tmp/ref dotnet/tests/corpus/*.{docx,xlsx,pptx,odt,ods,odp}
```

Batching matters: process startup dominates the cost of a conversion. Thirteen documents
converted in one invocation took ~1.4s total; one at a time it is roughly a second each.

### Reference text extraction

The right filter depends on the document family — and note Impress has no plain-text
filter at all, so presentations must go through HTML:

| Family | Flag | Filter | Output |
|---|---|---|---|
| Writer | `--txt` | `Text` | `.txt` |
| Calc | `--csv` | `Text - txt - csv (StarCalc)` | `.csv`, first sheet only |
| Impress | `--html` | `impress_html_Export` | `.html` |

```bash
.claude/skills/libreoffice-reference/scripts/lo-extract.sh --outdir /tmp/ref doc.docx
```

`--csv` exporting only the first sheet is a real limitation. For multi-sheet ground truth
prefer the PDF (print layout covers all sheets) or drive LibreOffice through its UNO API.

### Filter options

Anything after a second colon is a filter name, and a `{...}` third field is JSON filter
data:

```
--convert-to '<ext>:<FilterName>:<json>'
```

Each JSON value is an object with `type` and `value`, both strings:

```bash
soffice --headless ... --convert-to \
  'pdf:writer_pdf_Export:{"UseTaggedPDF":{"type":"boolean","value":"false"},"SelectPdfVersion":{"type":"long","value":"17"}}' \
  --outdir out/ in.docx
```

Useful keys — PDF: `UseTaggedPDF`, `SelectPdfVersion`, `ReduceImageResolution`,
`MaxImageResolution`, `Quality`, `ExportBookmarks`, `PageRange`. PNG:
`PixelWidth`, `PixelHeight`, `Translucent`.

Filter names for the formats in scope are listed in `reference/filter-names.md`.

## Interpreting failures

| Symptom | Cause |
|---|---|
| `Error: source file could not be loaded` on *any* input | Application modules not installed (gotcha above), **not** a corrupt file |
| `Warning: failed to launch javaldx` | Harmless. Java is only needed for Base and some extensions |
| No output file, exit code 0 | Usually the name collision (gotcha 1) — or the filter has no export support, e.g. **XLSB is import-only**. `soffice` exits 0 on conversion failure either way, so **check for the output file; never trust the exit code** |
| Output has fewer pages than expected | Fonts missing → text reflowed. Re-check `fc-match` |
| Blank first page in a presentation | Often a genuine LibreOffice import bug. Confirm by opening the reference PDF |

Because `soffice` exits 0 even when it converts nothing, every script here verifies the
output file exists and fails if it does not.

## What is *not* worth chasing

LibreOffice is the reference, but it is not correct by definition. It has its own import
bugs, and in a handful of areas Paperless may legitimately differ. Before treating a
difference as a Paperless defect, sanity-check the reference PDF: if LibreOffice itself
rendered the document wrongly, matching it exactly is not the goal. Record such cases as
known deviations rather than bugs.
