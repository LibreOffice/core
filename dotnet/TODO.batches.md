# Corpus batch status

Progress driving the [sample-files](https://github.com/theolivenbaum/sample-files) corpus —
541 real-world documents ordered by rendering complexity — to parity with LibreOffice.

The method, the reasoning behind it and the dispatch rules live in the
[`corpus-batches`](../.claude/skills/corpus-batches/SKILL.md) skill. This file is only the
scoreboard.

## The rule

A batch is **done** when it matches *and* every earlier batch in its track still matches.

```sh
S=.claude/skills/corpus-batches/scripts
$S/batch-check.sh /workspace/sample-files 'words/batch-007' out 3      # the batch you are on
$S/batch-check.sh /workspace/sample-files 'words/batch-00[1-6]' out 3  # the gate for moving on
```

Advancing on the first condition alone is how a corpus rots from the front: rendering
errors cascade, so a fix aimed at batch *n* routinely breaks batch *n−4* in a way that looks
nothing like the change.

## Priority

`words` and `slides` are worked first, in parallel. `sheets` is ordered and ready but comes
last — a spreadsheet's value is in its cells rather than its pagination, so faithful PDF
output matters least there.

## Status key

| Mark | Means |
|---|---|
| `—` | not started |
| `n/10` | measured, that many documents at parity |
| `WIP` | an agent is working it now |
| `✅` | full parity, **and** every earlier batch in the track re-proved after the last change |

Record measured numbers, never expected ones. A number here that was not produced by
`batch-check.sh` is worse than a blank.

## Baseline: the whole words track, measured

All 202 documents at `1d13c1e0a`, before any fix: **84 match, 118 do not (42%).**

Sorting the failures by page delta — ours minus LibreOffice — is what turns 118 problems
into three:

| Cluster | Documents | Shape |
|---|---|---|
| **Under-paginate** | 70 | We produce *fewer* pages. Page deltas run 5–43% while word deltas are only 1–6%: `A_320.doc` is 64 pages short and still carries 94% of the text. We are not losing content, we are **fitting too much of it onto each page** — a vertical-budget error (line height, text-area height, or the line-fits rule) manufacturing seventy unrelated-looking failures. |
| **Over-paginate** | 32 | We produce *more* pages, with word counts near-identical. About *where* content breaks, not what it contains. Opposite sign, so probably a different rule. |
| **Text loss** | 16 | Pagination already correct and text still short — 2%–22%, always short, never long. No reflow cascade confusing the picture, so whatever is missing never reached the page at all. |

Failure rates were 42% for `doc` and 43% for `docx`. Near-equal rates say the cause is
**downstream of both readers, in the layout they share** — which is why the work is split by
cluster and not by format.

Cluster lists live in the scratch directory as `c1-underpaginate.tsv`,
`c2-overpaginate.tsv`, `c3-textloss.tsv` (path, delta, pages ours/ref, words ours/ref).

## Baseline: slides

`slides/batch-001` at `84e7fe976` — **2/10**. Every page count already correct, so the
failures are about what reaches the page rather than pagination. Text is lost on six
documents and *over*-emitted on two — different bugs — and one document produces no PDF at
all. A full-track sweep is running.

### `words` — 202 documents, 21 batches

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 43–59 | doc:5 docx:5 | 9/10 |
| `batch-002` | 10 | 59–81 | doc:3 docx:7 | 6/10 |
| `batch-003` | 10 | 87–102 | doc:5 docx:5 | 6/10 |
| `batch-004` | 10 | 102–123 | doc:4 docx:6 | 8/10 |
| `batch-005` | 10 | 124–141 | doc:5 docx:5 | 4/10 |
| `batch-006` | 10 | 141–158 | doc:4 docx:6 | 7/10 |
| `batch-007` | 10 | 160–185 | doc:4 docx:6 | 4/10 |
| `batch-008` | 10 | 186–204 | doc:4 docx:6 | 8/10 |
| `batch-009` | 10 | 208–226 | doc:5 docx:5 | 3/10 |
| `batch-010` | 10 | 228–260 | doc:2 docx:8 | 4/10 |
| `batch-011` | 10 | 260–296 | doc:2 docx:8 | 4/10 |
| `batch-012` | 10 | 306–333 | doc:4 docx:6 | 3/10 |
| `batch-013` | 10 | 338–370 | docx:10 | 3/10 |
| `batch-014` | 10 | 372–422 | doc:4 docx:6 | 1/10 |
| `batch-015` | 10 | 424–471 | doc:3 docx:7 | 3/10 |
| `batch-016` | 10 | 473–537 | doc:5 docx:5 | 4/10 |
| `batch-017` | 10 | 537–602 | doc:2 docx:8 | 3/10 |
| `batch-018` | 10 | 620–859 | doc:2 docx:8 | 3/10 |
| `batch-019` | 10 | 956–1521 | doc:1 docx:9 | 0/10 |
| `batch-020` | 10 | 1523–3818 | doc:2 docx:8 | 1/10 |
| `batch-021` | 2 | 4417–4676 | docx:2 | 0/2 |

### `slides` — 165 documents, 17 batches

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 14–282 | ppt:3 pptx:7 | WIP (2/10) |
| `batch-002` | 10 | 312–410 | ppt:6 pptx:4 | — |
| `batch-003` | 10 | 411–482 | ppt:5 pptx:5 | — |
| `batch-004` | 10 | 488–560 | ppt:3 pptx:7 | — |
| `batch-005` | 10 | 587–668 | ppt:3 pptx:7 | — |
| `batch-006` | 10 | 671–903 | ppt:4 pptx:6 | — |
| `batch-007` | 10 | 941–1129 | ppt:3 pptx:7 | — |
| `batch-008` | 10 | 1130–1437 | ppt:5 pptx:5 | — |
| `batch-009` | 10 | 1510–1711 | ppt:4 pptx:6 | — |
| `batch-010` | 10 | 1748–1935 | ppt:3 pptx:7 | — |
| `batch-011` | 10 | 1980–2294 | ppt:1 pptx:9 | — |
| `batch-012` | 10 | 2403–3036 | pptx:10 | — |
| `batch-013` | 10 | 3054–3633 | ppt:3 pptx:7 | — |
| `batch-014` | 10 | 3638–4498 | ppt:2 pptx:8 | — |
| `batch-015` | 10 | 4626–7249 | ppt:4 pptx:6 | — |
| `batch-016` | 10 | 7428–13730 | ppt:1 pptx:9 | — |
| `batch-017` | 5 | 14810–32582 | ppt:1 pptx:4 | — |

### `sheets` — 174 documents, 18 batches

| Batch | Files | Score | Mix | Status |
|---|---|---|---|---|
| `batch-001` | 10 | 47–69 | xls:3 xlsx:7 | — |
| `batch-002` | 10 | 69–86 | xls:4 xlsx:6 | — |
| `batch-003` | 10 | 87–116 | xls:5 xlsx:5 | — |
| `batch-004` | 10 | 118–173 | xls:3 xlsx:7 | — |
| `batch-005` | 10 | 173–217 | xls:5 xlsx:5 | — |
| `batch-006` | 10 | 223–249 | xls:3 xlsx:7 | — |
| `batch-007` | 10 | 253–325 | xls:1 xlsx:9 | — |
| `batch-008` | 10 | 328–420 | xls:3 xlsx:7 | — |
| `batch-009` | 10 | 421–540 | xls:2 xlsx:8 | — |
| `batch-010` | 10 | 560–691 | xls:7 xlsx:3 | — |
| `batch-011` | 10 | 702–799 | xls:4 xlsx:6 | — |
| `batch-012` | 10 | 825–995 | xls:1 xlsx:9 | — |
| `batch-013` | 10 | 1039–1250 | xls:4 xlsx:6 | — |
| `batch-014` | 10 | 1276–1765 | xls:6 xlsx:4 | — |
| `batch-015` | 10 | 1773–2264 | xls:4 xlsx:6 | — |
| `batch-016` | 10 | 2286–4300 | xls:6 xlsx:4 | — |
| `batch-017` | 10 | 4468–14431 | xls:4 xlsx:6 | — |
| `batch-018` | 4 | 19384–48127 | xlsx:4 | — |
