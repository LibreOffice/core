# hyphen-r163 — what is here

- `results.md` — the write-up.
- `dumplines.py` — one PDF page's lines, assembled from `pdftotext -bbox` word boxes:
  `python3 dumplines.py <pdf> <page>` prints `y  x0  x1 | the line`.
- `fixture/build.py` — builds the minimal reproducing DOCX; `python3 build.py [fillers]`,
  15 is the threshold. `fixture/words-reference-field-wraps-a-page.docx` is the built file.

Renders are not kept (28 MB). To reproduce:

```bash
PATH=/opt/libreoffice26.2/program:$PATH \
  .claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --outdir ref \
  "/home/user/sample-files/words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx"
dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli render \
  --format pdf --outdir ours \
  "/home/user/sample-files/words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx"
```

**`lo-convert.sh` takes `soffice` from `PATH`, and `/usr/bin/soffice` is 24.2.7.2.**
Without the `PATH` prefix the reference comes out at 154 pages instead of 167.
