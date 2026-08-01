---
name: corpus-batches
description: Drive a large real-world document corpus to rendering parity with LibreOffice, batch by batch. Use when working through the sample-files corpus, when deciding which documents to fix next, when checking whether a fix regressed earlier batches, when dispatching parallel agents across the words/slides/sheets tracks, or when building or re-scoring a batched corpus from scratch.
---

# Working a corpus to parity, batch by batch

Proving a renderer correct on one document is a test. Proving it on 541 real documents
collected from the open web is a different problem, and the difference is not scale — it is
that you cannot hold the state in your head, so the *process* has to hold it for you.

This skill is that process. It covers the ordering, the unit of work, the rule that keeps
progress from unwinding, and how to run several workers at once without them colliding.

## The corpus

`theolivenbaum/sample-files` — real documents kept as found, mislabelled extensions,
version quirks and malformed markup included. That is the point: a corpus of clean
documents proves only that the clean path works.

```
words/batch-001 … batch-021     doc  docx     202 documents
slides/batch-001 … batch-017    ppt  pptx     165 documents
sheets/batch-001 … batch-018    xls  xlsx     174 documents
```

`MANIFEST.tsv` records the score that placed every document; `DUPLICATES.tsv` records every
byte-identical copy removed and where its survivor now lives.

## The four decisions, and why each one is the way it is

Each of these was arrived at by getting it wrong first.

### 1. Order by what a document demands of a renderer, not by size

Every document is rendered with LibreOffice and the resulting PDF scored on the distinct
capabilities it forces a renderer to have: embedded faces, images, shadings, transparency
groups, clipping paths, vector density, text operations, page count. Weights are ratios of
"how visibly wrong does the page go when this is missing", **not** of implementation effort.

A fifty-page memo set in one face is easier than a single slide carrying a gradient, a
clipped picture and a rotated table. Ordering by file size or page count puts the memo last
and teaches you nothing for forty batches.

Calibration note: on real-world documents this score runs far heavier than on synthetic
test files. The absolute bands in `pdf-complexity.py` (`01-low` under 25, and so on) were
set against generated documents; measured across this corpus the 10th percentile is already
95 and the median is 347. **Ordering is by rank, so the bands do not affect batching** — but
do not read a band label as a difficulty claim on real files.

**The ordering has since validated itself.** Sweeping all 21 words batches produced a match
rate that falls almost monotonically with batch number — 9/10 and 8/10 in the first
batches, 0/10 and 1/10 in the last three. The score is computed from LibreOffice's own
output and knows nothing about Paperless, so that gradient is evidence the metric measures
real difficulty rather than an artefact of how the corpus was assembled. It also means a
batch number is a usable progress estimate: reaching batch *n* is worth more than *n*/21.

### 2. Ten documents per batch

Small enough that one session can take a batch, find what is wrong, fix it and prove the
fix. Large enough that a batch is not noise. Fifty per batch turns each batch into a
project with no checkpoint; one per batch buries the signal in overhead.

### 3. Cut batches per family, not across the whole corpus

A single global ordering front-loads the easy end almost entirely with word processing, so
the slides and sheets work has nothing to do until batch forty. Three tracks let three
workers run in parallel and **never touch the same file**, which is what makes parallel
dispatch safe. Within a track the ordering is still strictly easiest-first.

### 4. Remove what cannot teach you anything

- **Byte-identical duplicates.** 39% of the original corpus — mostly the same document
  collected from two sites. Rendering one twice proves nothing and costs a batch slot a
  different document should have had. Deduplicate by content hash, not by name: the copies
  rarely share one.
- **Documents the reference cannot render.** Retry once with a generous ceiling, then drop.
  Without a reference rendering there is nothing to compare against, so the file would sit
  in a batch that can never be marked passing.

Record both removals rather than merely doing them. A deletion with no audit trail is
indistinguishable from a mistake six months later.

## The rule

**Make the current batch match, then re-prove every earlier batch in the track.**

The second half is the one that gets skipped, and it is the one that matters. Rendering
errors cascade: one wrong measurement — a font metric, a margin, a line break, an autofit
scale — shifts everything after it. So a fix aimed at batch *n* routinely breaks batch *n−4*
in a way that looks nothing like the change that caused it.

A batch is finished when it matches **and** everything before it still matches. Advancing on
the first condition alone is how a corpus rots from the front: you end up with batch 12
green, batches 1–11 quietly broken, and no idea which change did it.

```sh
S=.claude/skills/corpus-batches/scripts

$S/batch-check.sh /workspace/sample-files 'words/batch-007' out 3     # the batch you are on
$S/batch-check.sh /workspace/sample-files 'words/batch-00[1-6]' out 3 # the gate for moving on
```

Both exit non-zero on any mismatch, so either can gate a commit.

## What parity means here

Three checks, and the order matters — each is cheap and rules out a whole class:

| Check | Catches |
|---|---|
| **page count** | pagination. A wrong count means every later comparison is comparing different pages, so nothing after it is meaningful. |
| **extractable words** | text that lands perfectly but cannot be selected. Invisible to any geometric comparison — this project has shipped that defect twice, once from zero `/Widths` producing per-glyph TJ kerning of −700. |
| **font embedding** | a PDF that embeds nothing renders as tofu in a viewer while extracting flawlessly, so neither check above sees it. |

Word counts get a 2% tolerance: extraction drifts slightly on hyphenation and soft breaks,
and that band separates "the same text" from "text is missing" on this corpus.

**This is a gate, not a proof.** It says a document is worth measuring precisely, not that
it is correct. Where the ink actually lands is the fidelity suite's job — it compares PDF
operators directly. Use `render-comparison` to diagnose *why* two renderings differ.

## Priority

`words` and `slides` first; `sheets` last. A spreadsheet's value is in its cells rather
than its pagination, so faithful PDF output matters least there. The track is ordered and
ready, but it is worked after the other two.

## Dispatching parallel agents

One agent per track, each in its own git worktree. The tracks touch disjoint corpus files
and largely disjoint libraries (`Paperless.WordProcessing` vs `Paperless.Presentations`),
so conflicts are confined to the shared layers below them — merge those by *combining*
both changes, never by picking one side.

A brief that works contains all of:

1. **The track and the batch order.** Explicitly: do not touch the other tracks.
2. **The rule, both halves**, with the cascade reasoning — otherwise the regression sweep
   is the first thing dropped under time pressure.
3. **The baseline you measured, told as something to verify rather than trust.** Every
   time this has been checked, the predecessor's numbers were wrong in some way: describing
   already-fixed work, counting 27 exact matches that were really 10, a headline claim with
   no harness behind it.
4. **Known-good test counts, per project, with the instruction to compare counts.** A
   truncated run prints `Passed! - Failed: 0` while silently dropping the tests it never
   reached. Measured: 470 passed on one run and 353 on the next, both green, nothing
   changed between them. The colour means nothing; only the count does.
5. **No special-casing.** A fix that helps exactly one document in the corpus is not a fix.
6. **Commit, do not push.** The parent session merges and re-verifies.

Ask for honesty about what is unproven. An overstated claim costs more than an admitted gap,
because the next worker builds on it.

## Putting more than one agent on a single track

Worth doing, but **not by handing each agent a different batch range**. Batches are ordered
by complexity, not by cause; two agents on batches 1–10 and 11–20 will meet in the same
layout code and fix the same bug twice, differently.

### Sweep the whole track first, then split by symptom cluster

Measure every batch in the track at the current commit before dispatching anyone. It costs
one background run and it changes the answer, because a sweep turns "38 failures" into "two
clusters and a tail" — and the clusters, not the batches, are the work units that do not
collide.

The cheapest instrument is a **page-delta histogram**: for each failing document, ours minus
the reference. Measured on the words track at `84e7fe976`:

```
-1 page   most common by far    we under-paginate: content that should overflow does not
+1 page   second                we over-paginate
±2 or more                      a genuinely different layout, not an off-by-one
0, words differ                 pagination is right; what reaches the page is not
```

Documents sharing a delta usually share a root cause, so each bucket is one agent's brief.
A bucket also gives that agent something a batch never does: dozens of instances of the same
bug, which is what distinguishes a real fix from a special case.

### Check the split isolates the work before you commit to it

The obvious split is by format — one agent on the binary reader, one on the OOXML reader.
Verify it first. On the words track the failure rate was 42% for `doc` and 43% for `docx`,
which says the failures are **not** in the readers at all; they are in the layout both
formats share, and that split would have put two agents in one file.

One line tells you:

```sh
awk -F'\t' '{k=$2"\t"($7=="match"?"match":"fail"); c[k]++} END{for(x in c) print c[x], x}' rows.tsv
```

Near-equal failure rates across formats mean the cause is downstream of the readers. Split
by symptom. Sharply unequal rates mean the readers really are the problem, and splitting by
format is both safe and natural.

### How many agents actually fit

Each agent worktree costs **about 2.8 GB once built**. Measured breakdown, because the
obvious suspect is the wrong one:

| Part | Size | |
|---|---|---|
| LibreOffice C++ source checkout | 1.5 GB | inherent — a worktree copies the whole tree, and the C++ tree dwarfs `dotnet/` |
| Pinned build output, whole solution | 713 MB | working as designed |
| Unpinned `runtimes/` under `Paperless.Cli` | 550 MB | should not be there — see below |

**This is not the RID pin failing.** Verified on this commit: a clean `dotnet build` of the
CLI, a bare solution `dotnet build`, and a `dotnet publish` each produce **31 MB** with only
`bin/Debug/net10.0/linux-x64/` — no `runtimes/` at all. The pin does exactly what
`Directory.Build.props` claims.

The 550 MB is a `bin/Debug/net10.0/runtimes/` carrying all sixteen RIDs, present in every
agent worktree and absent from the main checkout. That is the layout MSBuild emits when
`RuntimeIdentifier` is empty, so some command run inside those worktrees built the CLI
without the pin applying. **I could not reproduce it** with the three builds above, so treat
this as an open question rather than a known cause — but check for it before blaming the
pin, and delete it if disk is tight (nothing consumes it; the pinned output beside it is
what runs).

Four concurrent agents is roughly the ceiling on a container with a 12–20 GB writable
allowance, and the failure mode is not a warning but `No space left on device` in the middle
of somebody's build.

Check before dispatching a fifth, and free the rendered PDFs from finished comparison runs
first — those are large and disposable, while the TSV they produced is small and is the
part worth keeping:

```sh
du -sh .claude/worktrees/* ; df -h /
rm -rf <outdir>/ours <outdir>/ref <outdir>/prof* <outdir>/t?   # keep rows.tsv
```

CPU is the softer limit: agents are mostly waiting on model calls, and the bursts are
builds and renders. On four cores, four agents plus a background sweep pushed load average
to ~8 and roughly halved sweep throughput, but nothing failed.

### Merging several agents into one branch

Merge one agent at a time, and **re-verify after each merge, not once at the end**. The
whole reason to sweep and cluster is that these changes interact; a merged result that
neither agent ever measured is not evidence of anything.

- **Combine conflicting hunks, never pick a side.** Every conflict here has been two correct
  fixes to one function — a stretch rule and a leading rule, an ascent computation and a
  height parameter. Taking one side silently reverts the other agent's work, and the tests
  will not catch it because each agent's tests pass in isolation.
- **Commit the resolved merge before starting the next one.** A `git merge` with an
  unfinished merge in the tree refuses silently; the symptom appears much later as test
  totals that do not add up.
- **Re-run the full track sweep after the last merge.** Per-agent numbers are measured
  against their own worktree. Only the sweep on the merged branch describes what you have.

## Building or re-scoring a corpus from scratch

```sh
S=.claude/skills/corpus-batches/scripts

# 1. Render everything. Parallel, resumable — a whole corpus outlives whatever starts it.
for i in 0 1 2; do $S/render-corpus.sh /workspace/sample-files /tmp/triage $i 3 & done; wait

# 2. Retry the failures once with a generous ceiling before writing any of them off.

# 3. Score.
python3 $S/pdf-complexity.py /tmp/triage/pdf > /tmp/triage/complexity.tsv

# 4. Preview the plan, then apply it on a clean working tree.
python3 $S/make-batches.py /workspace/sample-files /tmp/triage/complexity.tsv
python3 $S/make-batches.py /workspace/sample-files /tmp/triage/complexity.tsv --apply
```

## Traps

**`soffice` exits 0 having converted nothing.** Decide success by the output file existing,
never by the exit code. Every script here does.

**Parallel `soffice` instances need separate profiles.** Two headless instances sharing
`~/.config/libreoffice` block on the profile lock, and the loser converts nothing —
silently, with exit status 0. Pass `-env:UserInstallation=file:///distinct/path` per worker.

**PDFs need per-format names.** `report.doc` and `report.docx` both convert to `report.pdf`
and one overwrites the other. The failure surfaces as a mysterious parity mismatch on
whichever lost. Use `<stem>__<ext>.pdf` throughout.

**A whole-corpus render takes tens of minutes.** Three workers on four cores did 890
documents in about 25. Make it resumable and run it in the background; do not hold a
foreground shell for it.
