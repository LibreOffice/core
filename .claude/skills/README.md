# Skills for developing Paperless

Four skills covering development of the [Paperless](../../dotnet/) C# libraries against
headless LibreOffice as the reference implementation.

| Skill | Use it when |
|---|---|
| [`libreoffice-reference`](libreoffice-reference/) | You need ground truth: reference PDFs, per-page PNGs, or reference text from a document. Also when `soffice --convert-to` misbehaves. |
| [`render-comparison`](render-comparison/) | Comparing a Paperless rendering against the reference, and working out *why* it differs. |
| [`extraction-comparison`](extraction-comparison/) | Comparing extracted text. Also the right first step for a visual bug — parsing bugs are far cheaper to find in text than in pixels. |
| [`paperless-corpus`](paperless-corpus/) | Building or curating test documents. |

## Start here

```bash
.claude/skills/libreoffice-reference/scripts/check-env.sh
```

Nothing else is trustworthy until this passes. It catches the two failures that waste the
most time:

- **Application modules missing.** `libreoffice-core` alone installs a `soffice` that runs
  but cannot load *any* document, failing with a maximally unhelpful
  `Error: source file could not be loaded`.
- **Wrong font substitution.** If Calibri resolves to DejaVu Sans rather than Carlito,
  advance widths change, so line breaks change, so pagination changes — and every page after
  the first differs for a reason that has nothing to do with Paperless.

## Scripts

All are dependency-free (bash, or Python using only the standard library) and all were
verified against LibreOffice 24.2.7.2 on real files.

| Script | Does |
|---|---|
| `libreoffice-reference/scripts/check-env.sh` | Verifies soffice, modules, fonts, rasteriser |
| `libreoffice-reference/scripts/lo-convert.sh` | Documents → reference PDFs and page PNGs |
| `libreoffice-reference/scripts/lo-extract.sh` | Documents → reference text (right filter per family) |
| `render-comparison/scripts/compare-images.py` | Page images → metrics, diagnosis, diff images |
| `extraction-comparison/scripts/compare-text.py` | Text → similarity, first divergence, codepoint diagnostics |
| `paperless-corpus/scripts/make-corpus.sh` | Generates a test corpus across all 17 formats |

## Two facts worth knowing before you start

**`soffice` exits 0 even when it converts nothing.** Never trust its exit code — always check
that the output file exists. The scripts here all do.

**Rasterised LibreOffice output is byte-deterministic.** Verified: the same input converted
twice and rasterised gives identical PNGs. So golden reference images can be committed and
checksummed, and a changed checksum is a real signal. The PDFs themselves are *not*
byte-identical — they embed a timestamp — so checksum the PNGs, never the PDFs.
