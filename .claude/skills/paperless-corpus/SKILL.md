---
name: paperless-corpus
description: Build and manage the Paperless test corpus of office documents. Use when you need test files in specific formats (docx/doc/xlsx/xls/pptx/ppt/odt/ods/odp and variants), when adding a regression case for a bug, when a fidelity test needs a document exercising a particular feature, or when deciding whether a document belongs in the repository.
---

# Building the Paperless test corpus

Paperless must read every format LibreOffice's Writer, Calc and Impress do, so testing
needs documents in all of them. This skill covers producing them, organising them, and
knowing what not to commit.

## Generating files in every format

You do not need Microsoft Office. LibreOffice converts *into* all the target formats, so
one source document can be fanned out across a whole family:

```bash
.claude/skills/paperless-corpus/scripts/make-corpus.sh --outdir dotnet/tests/corpus/generated
```

The script takes a small set of source documents and emits each in every format of its
family. Verified working for: `odt doc docx rtf fodt` (Writer),
`ods xls xlsx fods` (Calc), `odp ppt pptx fodp` (Impress).

### The one thing to be careful about

**Give every file a unique stem.** `soffice --convert-to` names output after the input
stem alone, so `t.docx` and `t.xlsx` both become `t.pdf` and one silently vanishes. The
corpus therefore names files by what they contain and which format they are —
`table-merged.docx`, not `test.docx` — which is better documentation anyway.

## Corpus layout

```
dotnet/tests/corpus/
  minimal/      one small file per format; the smoke-test set
  features/     one document per feature under test
  regression/   a file per fixed bug, named for the issue
  generated/    produced by make-corpus.sh; NOT committed
  external/     large or licence-encumbered files; NOT committed
```

Only `minimal/`, `features/` and `regression/` belong in git. `generated/` is
reproducible and `external/` is not ours to redistribute; both are gitignored.

## What to commit, and what not to

Commit a file when it is **small, freely licensed, and minimal for its purpose.**

- **Small.** Tens of kilobytes. A repository of multi-megabyte decks is painful forever,
  because git keeps every version of every binary.
- **Freely licensed.** Do not commit documents from customers, from the web, or anything
  whose licence you cannot state. This is the rule that gets broken accidentally and is
  hardest to undo — git history is not easily rewritten.
- **Minimal.** Reduce to the smallest file that still shows the behaviour. A regression
  test containing one table is far more useful than a fifty-page report that happens to
  contain one, because when it fails you immediately know why.

Never commit anything containing real personal data. Test documents get shared, attached
to issues, and copied into reproductions.

## Adding a regression case

When you fix a bug:

1. **Reduce the file** until removing anything more makes the bug disappear.
2. Name it for the symptom or issue: `regression/issue-42-merged-cell-border.xlsx`.
3. Note in `dotnet/tests/corpus/README.md` what it exercises and what correct output is.
4. Generate its reference output and commit that too, so the test does not require
   LibreOffice to be installed just to run:
   ```bash
   .claude/skills/libreoffice-reference/scripts/lo-convert.sh --png --dpi 150 \
       --outdir dotnet/tests/corpus/reference regression/issue-42-merged-cell-border.xlsx
   ```
   Reference PNGs are byte-deterministic, so they are safe to commit and checksum.
   Record the LibreOffice version alongside them — a version upgrade legitimately changes
   them, and you want that to be an explicit decision rather than a mystery.

## Coverage worth having

Ranked by how often each breaks real documents. The first group causes the most damage
because it makes *everything downstream* wrong:

**Layout foundations** — these cause cascading failures
- Page size and margins; mixed page sizes in one document
- Fonts: an embedded font; a font that is definitely absent, to test substitution
- Line breaking: long unbroken words, CJK text, mixed-script runs
- Explicit page and column breaks

**Structure**
- Tables: merged cells, nested tables, spanning page breaks, borders
- Lists: multi-level, restarting numbering, custom bullets
- Headers/footers, including differing first page; footnotes; sections
- Floating frames with text wrap

**Spreadsheet-specific**
- Multiple sheets, including hidden ones
- Number formats: dates, currency, custom codes, `General` at narrow widths
- Print setup: print areas, scale-to-pages, repeated rows — a spreadsheet has no
  intrinsic pagination, so this *is* its page geometry
- Formulas including error values; conditional formatting

**Presentation-specific**
- Master and layout inheritance, and a placeholder overriding the master
- Theme colours with transforms (`lumMod`/`shade`/`tint`) — the most common source of
  wrong-colour slides
- Preset geometries beyond rectangles; grouped and rotated shapes
- Autofit/shrink-on-overflow text

**Awkward but common**
- An embedded EMF or WMF image (very common in real files; a known high-risk area)
- A password-protected file, one per encryption scheme
- A file with a wrong extension, to test content-based detection
- A macro-enabled file, to confirm macros are ignored, never executed
- A deliberately truncated or corrupt file, to confirm lenient recovery

## Third-party corpora

Do not vendor these; fetch them locally into `external/` when needed.

- **LibreOffice's own test files** — this repository, under `sw/qa/`, `sc/qa/`, `sd/qa/`.
  Thousands of real regression documents, each tied to a fixed bug. The most valuable
  resource available, and already on disk. Note they are MPL-2.0/LGPL-3.0, so check
  before redistributing any of them.
- **`ooxml-strict` and ISO conformance samples** — useful for the strict-namespace variant
  that real files occasionally use.

Point tests at `external/` through an environment variable so they skip cleanly when it
is absent, rather than failing on a machine that never downloaded it.
