# Skills for developing XRay.Office

Six skills covering development of the [XRay.Office](../../dotnet/) C# libraries against
headless LibreOffice as the reference implementation.

| Skill | Use it when |
|---|---|
| [`libreoffice-reference`](libreoffice-reference/) | You need ground truth: reference PDFs, per-page PNGs, or reference text from a document. Also when `soffice --convert-to` misbehaves. |
| [`render-comparison`](render-comparison/) | Comparing an XRay.Office rendering against the reference, and working out *why* it differs. |
| [`page-vision`](page-vision/) | Actually *looking* at a page: getting it in front of a reader at a resolution where the defect survives, delegating the reading to an uncontaminated reviewer, and knowing what a visual reading can and cannot establish. |
| [`extraction-comparison`](extraction-comparison/) | Comparing extracted text. Also the right first step for a visual bug — parsing bugs are far cheaper to find in text than in pixels. |
| [`office-corpus`](office-corpus/) | Building or curating test documents. |
| [`corpus-batches`](corpus-batches/) | Driving the 541-document sample corpus to parity, batch by batch — the ordering, the regression rule, and how to dispatch parallel agents across the three tracks. |

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
  the first differs for a reason that has nothing to do with XRay.Office.

## Scripts

All are dependency-free (bash, or Python using only the standard library) and all were
verified against LibreOffice 24.2.7.2 on real files.

| Script | Does |
|---|---|
| `libreoffice-reference/scripts/check-env.sh` | Verifies soffice, modules, fonts, rasteriser |
| `libreoffice-reference/scripts/lo-convert.sh` | Documents → reference PDFs and page PNGs |
| `libreoffice-reference/scripts/lo-extract.sh` | Documents → reference text (right filter per family) |
| `render-comparison/scripts/compare-images.py` | Page images → metrics, diagnosis, diff images |
| `render-comparison/scripts/pdf-image-diff.py` | Two PDFs → page PNGs, diff PNGs, and where each page differs |
| `extraction-comparison/scripts/compare-text.py` | Text → similarity, first divergence, codepoint diagnostics |
| `office-corpus/scripts/make-corpus.sh` | Generates a test corpus across all 17 formats |
| `corpus-batches/scripts/render-corpus.sh` | Renders a whole corpus with LibreOffice — parallel, resumable |
| `corpus-batches/scripts/pdf-complexity.py` | Scores what each rendered PDF demands of a renderer |
| `corpus-batches/scripts/make-batches.py` | Orders a corpus by that score and cuts it into per-family batches |
| `corpus-batches/scripts/batch-check.sh` | Renders a batch range both ways and gates on whether they agree |

## Two facts worth knowing before you start

**`soffice` exits 0 even when it converts nothing.** Never trust its exit code — always check
that the output file exists. The scripts here all do.

**Rasterised LibreOffice output is byte-deterministic — on the documents this was verified on,
and not universally.** The original check stands: the same input converted twice and rasterised
gave identical PNGs. But `fse_identification_form.xlsx` converted five times by one `soffice`
26.2.4.2 gives 430, 430, 430, 430 and **443** words, the difference being one cell's sentence
that LibreOffice draws in about one run in five. See the last section of
`dotnet/TODO.raster-ceiling.md`. **Render a reference more than once before trusting a
single-document figure**, and treat a golden image's checksum as a claim about that document
rather than about the renderer.

Within that limit the practice still holds: golden reference images can be committed and
checksummed, and a changed checksum is a real signal — but it is now a signal to re-render and
check whether the document is one of the unstable ones, not automatically evidence that
something in the tree changed. The PDFs themselves are *not* byte-identical either way, since
they embed a timestamp, so checksum the PNGs and never the PDFs.
