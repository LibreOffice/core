# Test corpus

See the `paperless-corpus` skill for how to generate and curate these files.

## Layout

| Directory | Committed? | Contents |
|---|---|---|
| `minimal/` | yes | One small file per format; the smoke-test set |
| `features/` | yes | One document per feature under test |
| `regression/` | yes | One file per fixed bug, named for the issue |
| `reference/` | yes | LibreOffice reference output for the above |
| `generated/` | **no** | Produced by `make-corpus.sh`; reproducible |
| `external/` | **no** | Large or licence-encumbered files |

## Rules for committing a file

Small (tens of kB), freely licensed, and reduced to the minimum that shows the behaviour.

Never commit a document from a customer, from the web, or whose licence you cannot state —
git history is not easily rewritten. Never commit real personal data; test files get shared
and attached to issues.

## Generating

```bash
.claude/skills/paperless-corpus/scripts/make-corpus.sh --outdir generated
```

Every file must have a **unique stem**: `soffice --convert-to` names output after the input
stem alone, so `a.docx` and `a.xlsx` both become `a.pdf` and one silently disappears. Name
files for what they contain and which format they are — `table-merged.docx`.

## Reference output

```bash
.claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --png --dpi 150 \
    --outdir reference minimal/*
```

Record the LibreOffice version alongside. Rasterised output is byte-deterministic, so a
changed checksum means the reference genuinely changed — usually a LibreOffice upgrade.
Regenerate deliberately.

## Inventory

_Empty. Add a row per committed file: what it exercises, and what correct output looks like._

| File | Exercises | Notes |
|---|---|---|
