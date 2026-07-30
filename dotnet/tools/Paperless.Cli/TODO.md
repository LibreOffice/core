# Paperless.Cli — TODO

The `paperless` command. Two audiences: humans inspecting documents, and the comparison
skills using it as the Paperless side of a side-by-side against LibreOffice.

Currently inert — it prints a "not implemented" message and exits 70. Deliberately: a CLI
that accepts commands and then throws is worse than one that says plainly it is not ready.

## Subcommands

- [ ] `identify FILE...` — detected format, container, encryption state, confidence, and how
      the decision was reached. **Build this first**: it is the first externally visible
      behaviour and makes everything after it debuggable.
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

- [ ] `System.CommandLine` wiring
- [ ] Exit codes: 0 success, 1 comparison/processing failure, 2 usage, 3 environment,
      65 unsupported format, 70 internal error
- [ ] `--json` for machine-readable output everywhere, so scripts do not parse prose
- [ ] `--password` for encrypted documents; also read from an environment variable so
      passwords stay out of shell history
- [ ] Deterministic output: a fixed PDF creation date option, so output is byte-comparable
- [ ] Package as a dotnet tool (`PackAsTool` is already set; `ToolCommandName` is
      `paperless`). Note the assembly is deliberately *not* named `paperless` — a lowercase
      assembly of that name collides case-insensitively with the `Paperless` library project
      and breaks restore.
