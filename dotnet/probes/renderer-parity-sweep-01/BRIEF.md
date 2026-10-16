# Shared brief — X-Ray.Office fidelity fix investigation

You are one of eight agents working in parallel on the same finding: a sweep of all
946 documents in `theolivenbaum/sample-files` rendered by X-Ray.Office and by headless
LibreOffice found **192 documents that do not match**. Each of you owns one lane.

## The one rule that keeps eight agents from colliding

**Do not modify, build, or test anything in the checkout.** Eight agents share one
working tree and four cores. `dotnet/CLAUDE.md` records what happens when a build
lands under someone else's measurement (a silently swapped binary) and what a test
run under load reports (a truncated run that prints `Failed: 0`, and failures that
are not there). So:

- The checkout at `/home/user/libreoffice-core` is **read-only** to you.
- Write **only** inside your own lane directory. Nothing else writes there, so your
  output can never conflict with another lane's.
- Your deliverable is an investigation and a **patch as a diff file**, not an applied
  change. A later serialized pass applies, builds and tests them one lane at a time.

## What you are producing

`<your lane dir>/findings.md`, organised **by root cause, not by document**. Thirty-
seven documents are not thirty-seven bugs; the value you add is collapsing them. For
each root cause:

1. **What the pages show.** Which of your documents exhibit it, and the evidence from
   the rendered pair that says they are the same fault.
2. **What the document actually contains.** Unzip the OOXML part (or dump the record
   stream) and quote the markup that drives it. `dotnet/CLAUDE.md` records three
   diagnoses that named a mechanism the file does not use — grepping the part for the
   attribute you are about to blame costs a minute and refutes a plausible story
   before it costs a day.
3. **Where it lives in the source**, with `file:line` citations, and whether the
   property is read but never consumed. That last pattern has been the cause four
   times in this project; grep for it on purpose.
4. **The proposed change**, as a unified diff written to
   `<your lane dir>/patches/<short-name>.diff` (`git diff` format, paths relative to
   the repo root). Keep each patch to one root cause so it can be applied alone.
5. **The probe that would refute you** — the smallest document or measurement that
   distinguishes your explanation from the next most likely one. State it even when
   you cannot run it.
6. **Confidence**, and what you did not establish.

Also write `<your lane dir>/summary.md`: five lines or fewer per root cause — the
fault, the file, the document count, and confidence. That is what gets read first.

## Ownership

Your `cases.md` names the source directories your lane owns exclusively. No other
lane will propose an edit inside them. **Do not propose an edit outside them.** If a
fix genuinely needs a shared file — `XRay.Office.Core` outside `Charts/` and
`Graphics/`, `XRay.Office.Containers`, `XRay.Office.Text` (unless you are L1),
`XRay.Office.Vector`, `XRay.Office.MsBinary`, `XRay.Office.OpenDocument` — do not write it.
Record it under a `## Cross-lane dependencies` heading naming the file and what it
needs, and I will sequence it.

## The environment, already set up

- Corpus: `/home/user/sample-files`. Repo: `/home/user/libreoffice-core`.
- Reference: `soffice` **24.2.7.2**, with Carlito, Caladea and Liberation installed
  (`fc-match Calibri` → Carlito). Do not re-render the reference — every document's
  reference PDF is already at `/data/bench/lo/<id>/out.pdf` and ours at
  `/data/bench/pl/<id>/out.pdf`.
- Prebuilt CLI, if you need to render a *probe* document you construct yourself:
  `/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli`
  Use `--outdir` (not `-o`). Assert it produced output before comparing anything —
  a glob that silently picked up the reference once reported a fabricated match.
- Your rendered pairs are at `/data/bench/pairs-view/NNN.jpg`, LibreOffice left,
  X-Ray.Office right, at reading resolution. **Look at them.** The numbers say a page is
  wrong; only the image says what is wrong.

## Read before you start

`dotnet/CLAUDE.md` — it is long and it is worth it. It records refuted hypotheses you
must not re-derive, in particular:

- The advance-width divergence is **not** kerning and **not** a quantisation grid.
  Both were settled by probe; the seat is grid-fitted vs. unhinted advances.
- `w:trHeight` is exact, and a `nextPage` section break after a table is honoured.
- 37 pages are a "raster ceiling" the word-count gate cannot win — ours is the better
  output. `dotnet/TODO.raster-ceiling.md`.

Six documents in the sweep are tagged `lo-broken`: the **reference** is the broken
rendering and X-Ray.Office is correct. If one is in your lane, confirm it and file it —
do not chase it.

## Scope

Investigate; propose. Do not widen into refactors, and do not propose a heuristic
tuned to one sample — `AGENTS.md`'s OOXML policy forbids sample-specific branches,
magic thresholds and empirical scale factors. If Office behaviour is genuinely
unspecified, say so and stop rather than guessing.
