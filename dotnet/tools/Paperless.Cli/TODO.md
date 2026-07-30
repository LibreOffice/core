# Paperless.Cli — TODO

The `paperless` command. Two audiences: humans inspecting documents, and the comparison
skills using it as the Paperless side of a side-by-side against LibreOffice.

`identify` works; the rest print usage and are not wired up.

Argument parsing is hand-rolled and `System.CommandLine` has been dropped. The surface is a
handful of subcommands with two or three options each, and hand-rolling keeps the tool
dependency-free — worth more here than what a parser library would add. Revisit only if the
option surface grows enough to make this awkward.

## Subcommands

- [x] `identify FILE...` — format, family, container, confidence, media type, flags, and the
      basis for the decision. Text and `--json` output; correct on all 17 corpus formats.
- [ ] `metadata FILE` — document properties as JSON
- [ ] `extract FILE` — text, or the content tree as JSON (`--format text|json`)
- [ ] `render FILE` — pages to PNG. `--dpi`, `--pages`, `--outdir`.
      **Page naming must match what the comparison scripts expect** (`page-1.png`, ...) or
      the harness cannot pair pages up.
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
- [ ] Deterministic output: a fixed PDF creation date option, so output is byte-comparable
- [ ] Package as a dotnet tool (`PackAsTool` is already set; `ToolCommandName` is
      `paperless`). Note the assembly is deliberately *not* named `paperless` — a lowercase
      assembly of that name collides case-insensitively with the `Paperless` library project
      and breaks restore.
