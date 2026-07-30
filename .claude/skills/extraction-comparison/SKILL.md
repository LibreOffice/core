---
name: extraction-comparison
description: Compare Paperless text/content extraction against headless LibreOffice's own text export, and interpret the differences. Use when validating extraction correctness for any office format, investigating missing or garbled extracted text, checking encoding handling in legacy binary formats, or deciding whether an extraction difference is a real bug. Also the right first step when a rendering differs, since parsing bugs are far easier to find in text than in pixels.
---

# Comparing extraction against LibreOffice

Generate reference text with the **libreoffice-reference** skill
(`scripts/lo-extract.sh`); this skill covers comparing against it.

## Do this before comparing renderings

When a rendered page looks wrong, check extraction first. If the extracted text already
differs, the bug is in **parsing**, not rendering — and a text diff points at it far more
directly than a pixel diff ever will. Only once the text matches is a visual difference
actually about layout or drawing.

## Running a comparison

```bash
# 1. Reference text from LibreOffice
.claude/skills/libreoffice-reference/scripts/lo-extract.sh --outdir /tmp/ref document.docx

# 2. Paperless extraction
dotnet run --project dotnet/tools/Paperless.Cli -- extract --outdir /tmp/actual document.docx

# 3. Compare
.claude/skills/extraction-comparison/scripts/compare-text.py \
    --expected /tmp/ref/document.docx/document.txt \
    --actual   /tmp/actual/document.txt
```

`compare-text.py` normalises before comparing (see below), reports similarity plus the
first divergence, and prints a unified diff of the differing region.

## What the reference can and cannot tell you

LibreOffice's text export is a genuine oracle for *content*, but it is lossy and
opinionated. Know the limits before treating a difference as a defect.

| Reference | Covers | Does not cover |
|---|---|---|
| `Text` filter (Writer) | Paragraph text in document order | Headers/footers, footnotes, comments, text inside shapes, table structure (cells become tab-separated) |
| `csv` filter (Calc) | Cell **displayed** values, first sheet | **Every sheet after the first.** Formulas, comments, shape text |
| `impress_html_Export` | Slide text as HTML | Speaker notes placement, shape z-order, exact reading order |

Two consequences worth stating plainly:

- **A Paperless extraction that finds *more* than the reference is usually correct, not
  wrong.** Headers, comments, notes and shape text are things Paperless deliberately
  extracts and these filters deliberately drop. Compare the intersection; do not chase
  parity on what the reference cannot express.
- **For multi-sheet spreadsheets the CSV reference is nearly useless.** Use the reference
  PDF's text layer instead (`pdftotext`), which covers all printed sheets.

## Normalisation, and why each step is justified

Comparing raw bytes produces noise that hides real bugs. `compare-text.py` normalises,
and each step exists for a specific reason rather than to make numbers look better:

| Step | Why |
|---|---|
| CRLF/CR → LF | The filters differ in line ending by platform; never a real difference |
| Strip BOM | LibreOffice writes one; it is an encoding artefact, not content |
| NFC Unicode normalisation | The same character can be composed or decomposed. Visually and semantically identical |
| Collapse runs of blank lines | The filters emit inconsistent blank-line padding around blocks |
| Strip trailing whitespace per line | Padding artefact |
| Normalise non-breaking space to space (`--fold-spaces`) | **Off by default.** NBSP versus space *is* a real difference and can be a genuine bug; only fold when you have established it is not what you are chasing |

Nothing is case-folded and no punctuation is stripped: those would hide real bugs.

## Reading the result

| Symptom | Likely cause |
|---|---|
| Similarity ~1.0, tiny diffs | Match. Check the diff is only whitespace/padding |
| Paperless has *extra* content | Usually correct — headers, comments, notes, shape text the filter drops. Verify by eye, do not "fix" |
| Paperless *missing* a contiguous block | An unhandled structure: a table, a text box, a section, a nested field |
| Text present but mangled into wrong characters | **Encoding.** Legacy DOC/XLS/PPT store text in a codepage chosen by the file; if the codepage was misread every non-ASCII character is wrong. Check the codepage record before suspecting the parser |
| Right characters, wrong order | Reading order: a table traversed column-major, or shapes in z-order rather than position order |
| Paragraphs merged or split | Paragraph-boundary detection. In DOC/WW8 this often means the piece table was mishandled |
| Empty output from Paperless, reference fine | Format detection failed, or the reader threw. Check `identify` first |

### Encoding problems specifically

The tell is *systematic* corruption of non-ASCII characters while ASCII is perfect. That
is a codepage mismatch, not a parsing bug:

- **DOC/WW8** — codepage derives from the language id in the FIB.
- **XLS/BIFF** — the `CODEPAGE` record; BIFF8 is mostly UTF-16 but not universally.
- **PPT** — per-run character properties can override.
- **CSV** — genuinely ambiguous; detection is heuristic and a mismatch here may be
  expected rather than a bug.

`compare-text.py --show-codepoints` prints the differing characters with their code
points, which usually makes the substitution pattern obvious at a glance.

## Metadata

Metadata comes from three unrelated systems — ODF `meta.xml`, OOXML `docProps/*`, and the
OLE summary-information property sets — so compare it separately and per-field. A missing
field is often "the format never recorded it", not a bug: Paperless leaves unrecorded
fields null precisely so that "absent" stays distinguishable from "empty".
