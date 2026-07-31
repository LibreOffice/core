# Paperless.Cli — TODO

The `paperless` command. Two audiences: humans inspecting documents, and the comparison
skills using it as the Paperless side of a side-by-side against LibreOffice.

`identify`, `extract`, `metadata` and `render` work; `convert` and `diagnose` print usage
and are not wired up.

Argument parsing is hand-rolled and `System.CommandLine` has been dropped. The surface is a
handful of subcommands with two or three options each, and hand-rolling keeps the tool
dependency-free — worth more here than what a parser library would add. Revisit only if the
option surface grows enough to make this awkward.

## Subcommands

- [x] `identify FILE...` — format, family, container, confidence, media type, flags, and the
      basis for the decision. Text and `--json` output; correct on all 17 corpus formats.
- [x] `metadata FILE` — document properties as JSON
- [x] `extract FILE` — text, or the content tree as JSON (`--format text|json`)
- [x] `render FILE` — pages to PDF, PNG or JPEG. `--format`, `--dpi`, `--pages`, `--outdir`.
      Page naming matches what the comparison scripts expect: one input writes `page-1.png`,
      `page-2.png` … straight into `--outdir`, which is what the `render-comparison` skill's
      own example points `compare-images.py` at, and the same names `pdftoppm` gives the
      reference side. Several inputs would collide on those names, so each gets a
      subdirectory named after the file — the rule `lo-convert.sh` uses for the reference.
      PDF output is `<stem>.pdf` in the same place.
      `--format jpeg` exists for thumbnails and the usage text says so: a JPEG page image
      turns a fidelity question into a question about quantisation.
- [ ] `convert FILE` — PDF or SVG output
- [ ] `diagnose FILE` — dump diagnostics, resolved fonts and any substitutions. Most
      valuable command for debugging fidelity: a silent font substitution explains most
      unexplained reflows.

## Cross-cutting

- [x] Exit codes, sysexits-style: 0 success, 1 read failure, 2 usage, 65 unsupported format.
      Add 3 (environment) and 70 (internal) as the commands that need them land.
- [x] `--json` on `identify`. **The JSON property names are effectively public API** — the
      comparison scripts parse them — so renaming one is a breaking change. camelCase.
- [ ] `--json` on the remaining subcommands as they land
- [ ] `--password` for encrypted documents; also read from an environment variable so
      passwords stay out of shell history
- [ ] Deterministic output: a fixed PDF creation date option, so output is byte-comparable.
      `PdfRenderOptions.CreationDate` does this and the tests use it; the CLI does not expose
      it yet, so a PDF written by the tool differs from run to run in exactly one field.
- [ ] Package as a dotnet tool (`PackAsTool` is already set; `ToolCommandName` is
      `paperless`). Note the assembly is deliberately *not* named `paperless` — a lowercase
      assembly of that name collides case-insensitively with the `Paperless` library project
      and breaks restore.
