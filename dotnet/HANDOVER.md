# XRay.Office — parity work handover

Hand this whole file to the new session as its opening prompt. It is written to be read once,
top to bottom, before anything is run.

---

## 1. What the project is

**XRay.Office** is a pure C#/.NET library set for content extraction and headless rendering of
office documents. It lives in `dotnet/` inside a LibreOffice checkout at
`/c/sandbox/workdir/libreoffice-core`.

Two things about that arrangement matter and are easy to get wrong:

- **The C++ tree is reference material, never a build dependency.** You read it to understand
  what LibreOffice intends. You do not link it, port it wholesale, or trust it over measurement.
- **The C++ in this checkout is not the reference binary.** Ground truth is the *installed*
  `soffice` — **LibreOffice 26.2.4.2 620(Build:2)**, where the checked-out tree is
  **27.2.0.0.alpha0+**. Several rounds have found the checked-out source contradicting the
  installed binary's actual behaviour. When they disagree, the binary wins, and an authored probe
  read against the binary is how you settle it.

  **Five rounds have now burned predictions on forgetting this**, most expensively one that
  believed line-spacing machinery which does not exist in 26.2.4.2 at all, and one that predicted
  from the 27.2 source that LibreOffice never overlaps a border at a joint when it overlaps by the
  crossing border's full width. Cite the C++ for *intent*; measure `soffice` for *truth*.

### Layering

```
XRay.Office.Core (zero deps)
  → Containers · Text · Vector · Rendering · Markup
    → Ooxml · OpenDocument · MsBinary
      → WordProcessing · Spreadsheets · Presentations
        → XRay.Office → Cli
```

`Core`, `Containers`, `Text`, `Vector`, `Rendering`, `Markup` are **shared layers**. Note
particularly that **`XRay.Office.Text` reaches slides**, which has surprised people.

Stack: .NET 10, `TreatWarningsAsErrors`, host-RID pinned in `Directory.Build.props`, SkiaSharp
for raster, HarfBuzzSharp for shaping, a hand-rolled OpenType reader in `XRay.Office.Text`.

---

## 2. The standing goal

> Implement and validate the XRay.Office library by testing against each of the respective batches
> of test files, batch by batch. When done with a batch always retest the previous batch to
> validate no regressions. Use agents to parallelise the work according to the strategy for
> parallel work. Each agent should focus on a library (word, slides, sheets) and could also focus
> on specific identified problems.

And a standing operational instruction from the user:

> **Keep always at least one agent per track (Slides, Word, Sheets) running.**

That is the loop. The session's own job is not to fix documents directly — it is to **brief,
merge, verify, record and re-dispatch**, one agent per track, indefinitely.

---

## 3. Where things stand right now

> **This section was rewritten after a container move. Read `dotnet/CLAUDE.md` § "This container"
> before reproducing any figure below — three of the inputs a measurement depends on changed, and
> none of them is our code.**

Repository on `claude/paperless-odf-phase-1-rnyzcu`, pushed. Scoreboards measured at HEAD, not
inherited from a round's report.

> **Superseded by the 2026-08-19 sweep — read § 3a below before quoting any figure in this
> section.** The corpus grew from 534 to 946 documents and the whole scoreboard was re-measured.
> The per-track tables in § 3 and § 8 are the *pre-growth* record and are kept because the
> refutations and mechanisms attached to them are still live; the counts are not.

| track | verdicts |
|---|---:|
| words | **155 / 200** |
| slides | **144 / 163** |
| sheets | **146 / 171** |
| **total** | **445 of 534** |

**That total is not comparable with the 465 recorded before it, and neither is any per-track
figure.** Four things moved underneath the number, in order of size:

1. **The reference binary is LibreOffice 26.2.4.2**, where every stored figure was taken against
   **24.2.7.2**. Net cost on words alone: 5 verdicts.
2. **`fonts-dejavu-core` was missing from the container and is now installed.** It sits ahead of
   WenQuanYi Zen Hei in the fallback chain and **267 of the 534 reference PDFs resolve a fallback**.
   Cost on words: **25 verdicts** — larger than the version change. See `MISSING_PACKAGES.md`.
3. **The gate's word check changed**: a token is a word iff it carries a Unicode letter or digit,
   because LibreOffice now emits bullet glyphs into the text layer and `wc -w` counted each as a
   word. Worth **+14 across the corpus** and, on slides, it un-hid a document that had been passing
   by the arithmetic of two opposite errors.
4. **poppler is 26.01.0**, and it was an *undeclared* input: with our source provably unchanged, our
   own word counts moved on 169 of 200 documents and 86 of them moved by exactly the amount the
   reference moved. `xray-office analyze` now reads PDFs in process to remove that dependency —
   though `batch-check.sh` still calls poppler, deliberately, because rewiring it restates every
   scoreboard again.

Test counts at HEAD, **0 failed**:

```
Core 313   Containers 109   Text 289   Vector 295   Rendering 149(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 792   Spreadsheets 676   Presentations 631      = 3638
Fidelity 519 passed / 31 failed of 550
```

**Fidelity's 31 are classified, not mysterious** (`dotnet/probes/fidelity-01/results.md`): it
rebuilds the reference live via `soffice` rather than pinning stored figures, the same 40 reproduce
byte-identically at the handover's own green commit, and of them **12 were genuinely ours** — nine
of which shipped. Three of the remainder are places where **we are right and LibreOffice is wrong**.
The single `Rendering` skip is an environment gap: the container has zero `.otf` files.

### The method changed, at the user's instruction

**Look at the rendering. Do not chase it through metrics alone.** The gate is blind to most real
defects, and this is now written into `dotnet/CLAUDE.md` § "Fidelity" and
`.claude/skills/render-comparison/SKILL.md` § "Look at the page", with a tool:

```sh
python3 .claude/skills/render-comparison/scripts/look.py "<doc>__pptx" --worst
```

Rank **all** documents by `|ink|%` — not the failing ones, which are picked over — open the worst
pages, and describe them before checking the record. The first three *passing* slide decks opened
this way produced three findings, two previously unrecorded. See §9: the user's own review of 30
decks found **17 in a single class no gate column can see**.

---

## 3a. The 2026-08-19 sweep, and the corpus growth that preceded it

The corpus grew to **946 documents** (words 337, slides 302, sheets 307) with the addition of the
`chartset-*` batches — 300 documents of chart-bearing and template material across all three
tracks. Every batch was then swept against **26.2.4.2** and `MANIFEST.tsv` refreshed from it.

| track | passing | of |
|---|---:|---:|
| words | 300 | 337 |
| sheets | 265 | 307 |
| slides | 198 | 302 |
| **total** | **763** | **946** |

What landed in that round: `batch-check.sh` measuring every in-scope extension rather than
thirteen of thirty-four (two `.xlsm` were silently unmeasured); DrawingML `a:rPr/@cap`;
`headerFooter/@differentFirst`; floating VML shapes and their text boxes.

### Three findings from it that will mislead a round that does not know them

**A real fix that moves no verdict.** `a:rPr/@cap` was a genuine defect — the reference drew
`LOREM IPSUM` where we drew `Lorem Ipsum` — and fixing it moved **zero** gate verdicts, because
the gate counts *words* and upper-casing a word does not change how many there are. The round
predicted 20 documents and got none. **Do not go looking for the missing verdicts.** Character
identity over the 100 new decks went 50/100 → 67/100, which is the only measurement that can see
it. This is § 4's blind-spot rule arriving in its purest form yet.

**Most of the slides failure pool is a measurement ceiling.** Those decks carry `spc="150"`
letter spacing from their masters; LibreOffice positions each glyph separately enough that
`pdftotext` splits *inside* words — `2-Way` extracts as `2` + `-W` + `ay` — so the reference is
credited three tokens where we get one. 1585 phantom words. **Our output is the better one.**
They are filed `kind=ceiling` in the manifest. No code change wins them; the honest reading of
the new decks is 72 of 100 correct in content, not 12. The standing risk is the mirror of it:
a *misfiled* ceiling hides a real defect behind a label that tells every future round not to
look, so sample the class and re-check it rather than inheriting it.

**Stored status decays.** `MANIFEST.tsv` was 41 rows behind a fresh sweep, all of them
*understating* progress, and four were named open problems in the task list that earlier commits
had already closed (`orbus_togaf_tool_csq.xls` 75/75, `sectors-defense-and-aerospace.xlsx`
449/449, `grants-2005.xls` 201/201, `SIL_TDB648.xlsx` 90/90). **Re-sweep before trusting the
manifest** — this is § 7's "stored evidence decays silently" with the manifest itself as the
victim.

### The method that found every fix in that round

Not the gate. For each failing document, strip **all** whitespace from both `pdftotext`
extractions and compare the characters that remain. That separates "we drew different text" from
"we drew the same text and `pdftotext` tokenised it differently", and it is what turned an
undifferentiated pile of 162 failures into four named mechanisms. It is sixty lines of shell and
it is the first thing to run on any word-count failure.

Then look for clusters in the **size** of the gap. Six sheets workbooks failing by exactly four
words was `Page 1 of 1`. Seven words documents at exactly zero words was floating VML.

### Two operational rules the round added

- **Never `git add -A`.** This mount reports symlink size as 0, so git sees 57 symlinks as
  emptied and staging everything replaces them with empty files. Stage explicit paths, then
  verify `git ls-files -s <paths> | awk '$1=="120000"'` prints nothing.
- **A session has a subagent cap** (200 was hit). Three parallel tracks is the intended shape;
  budget the reviewers accordingly rather than discovering the ceiling mid-round.

## 4. The corpus and the gate

Corpus: `theolivenbaum/sample-files`, checked out at **`/c/sandbox/workdir/sample-files`**. Real
documents kept as found — mislabelled extensions, version quirks, malformed markup included.

```
words/batch-001 … batch-021     doc  docx     200 documents
slides/batch-001 … batch-017    ppt  pptx     163 documents
sheets/batch-001 … batch-018    xls  xlsx     171 documents
```

`MANIFEST.tsv` records the score that placed every document; `DUPLICATES.tsv` records every
byte-identical copy removed.

### The gate, in order

`batch-check.sh` decides a verdict on three checks, cheapest first, each ruling out a class:

1. **page count** — a wrong page count makes everything after it meaningless
2. **extractable words** (2% band)
3. **font embedding** (unembedded faces)

Then, for diagnosis rather than verdict: image diff → operator diff → source attribution.

### The gate's blind spot is the central fact of this project

**Most real fixes move zero verdicts.** A face name, a sub-pixel stroke, a 0.4% font size, a
spacing scale, chart geometry, merged-range borders — all real, all measured by rendering, none
visible to page count / words / unembedded fonts. This is normal and expected. A round that
moves no verdict is not a failed round; a round that *predicts* it will move no verdict and is
right is a well-run one.

Two consequences:

- **Predict verdict movement in a committed file before rendering anything post-change.** Every
  recent round does this and it is what keeps the reporting honest.
- **Rank by `|ink|%` (unsigned), decide by `ink%` (signed).** Two different measurements
  circulated under one name for several rounds — the slides track's signed sum was 1181.39 while
  the unsigned was 1493.00, and a figure quoted as `|ink|%` was actually the signed column.
  `probes/slides-r39/ink-ranking.py` prints both and asserts `|signed| ≤ |ink|`.

---

## 5. The merge routine — follow it exactly

**Pass `-C <primary>` to every `git` invocation in this routine.** The shell's working directory is
not reliably where the last `cd` put it, and a merge that lands on the wrong branch reports success
(§7).

When an agent reports:

0. **Read the agent's uncommitted diff before anything else** — `git -C <worktree> status --short`
   and `git -C <worktree> diff`. It is one of three things and only one of them is safe to merge as
   found (§7).
1. `git -C <primary> rev-parse --abbrev-ref HEAD` and `git -C <primary> status --short` — the right
   branch, and clean.
2. `git -C <primary> merge --no-edit worktree-<track>-r<N>`, then
   `git -C <primary> log --oneline --graph -3` — **the merge commit must be there.** A fast-forward
   of something else means it went somewhere else.
3. **`dotnet/TODO.batches.md` conflicts almost every time.** Resolve by **stripping the markers
   and keeping both sides**, in order, with a blank line between sections. Never pick a side —
   both are records of real rounds.
4. `git diff <base>..HEAD --stat -- dotnet/src` — **a round that shipped code must show a diff
   here.** See §7 for why this check exists.
5. `cd dotnet && dotnet build -v q -nologo` — expect **0 warnings, 0 errors**.
6. Run the ten non-Fidelity projects individually and compare each count against the table in §3
   plus whatever the round says it added. Then run Fidelity (2–7 minutes).
7. Append a **merge note** to `dotnet/TODO.batches.md`: the combined test counts (a round measured
   on an older base will quote stale counts for the other tracks), the verdict movement, whether a
   cross-track sweep was owed and what it measured, and anything a future round reading only the
   scoreboard would misread.
8. Commit with the project's style — findings and refutations in the message body,
   `Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>`.
9. `git worktree remove .claude/worktrees/<name> && git branch -d worktree-<name>`
10. `git push -u origin claude/paperless-odf-phase-1-rnyzcu`
11. **Immediately dispatch the next agent for that track**, so the one-per-track rule holds.

### A shared-layer merge owes a measurement, not an argument

If the diff touches `Core`, `Containers`, `Text`, `Vector`, `Rendering` or `Markup`, the other two
tracks must be *measured*, not reasoned about. Recent examples of both shapes:

- Round 45 changed `XRay.Office.Text` and swept slides + sheets whole: **334 of 334 byte-identical**.
- Round 44 changed `PdfImages` in `XRay.Office.Rendering`. A census named the affected documents
  exactly (words 5, slides 7, sheets 0), and all seven slides decks were rendered at both commits:
  **all seven renderings changed, not one gate column did.**

---

## 6. Tools

### `.claude/skills/corpus-batches/scripts/`

- **`batch-check.sh <corpus-root> <batch-glob> [outdir] [workers]`** — the gate. Parallel workers
  each with their own `soffice` profile (two headless instances sharing `~/.config/libreoffice`
  block on the profile lock and one converts nothing, silently, with exit 0). Per-format identity
  (`report__docx`, not `report`) because two documents differing only by extension both convert
  to `report.pdf` and one overwrites the other.
- **`verify-test.sh <project> '<mutation>' [filter]`** — the mutation cycle, made safe by
  construction. **Refuses to start unless `dotnet/` is clean**, builds explicitly on both legs,
  names the failing tests, restores, rebuilds. Exit 0 = detected, 1 = drift guard, 2 = refused.
  Use it for every new test and report which tests were verified *by reintroduction* and which are
  drift guards or preconditions that were not.
- `render-corpus.sh`, `track-ink-sweep.sh`, `make-batches.py`, `pdf-complexity.py`,
  `raster-ceiling-pages.py`

### `.claude/skills/render-comparison/scripts/`

- **`pdf-ops.py`** — names the element that differs. Reports `dump` and `diff`. Knows about
  horizontal scale (`13.59ptx14.00w`) because 4881 of 7078 `Tm`+`Tf` shows across 21 slide decks
  carry a non-unit horizontal scale and collapsing them into one number hid it.
  **Repaired in round 43**: same-orientation stroke pairing, an extent tie-break for the identical
  anchors at a table corner, and a `hairline` note class. A `box` count taken before `cce1cc314`
  is not comparable with one taken after.
- **`first-divergence.py`** — walks forward to the first materially differing page and diffs that
  page alone, *because errors cascade and comparing page N against page N measures the cascade
  rather than the fault*. `--corpus rows.tsv --out div.tsv` sweeps a track. A page the image diff
  refuses to compare (`page size differs`) is the **strongest** divergence signal, not agreement.
- **`pdf-image-diff.py`**, **`compare-images.py`** — `ink%` is the column that decides.
- **`trace-text.py`** — rewrites a document's ASCII words with tokens unique across the file so a
  PDF mark can be traced to its source run. Works on OOXML and ODF; **not** on binary `.doc`/
  `.xls`/`.ppt`. Word count is preserved at the *model* level; `pdftotext` counts drift ~1%
  because PDF extraction re-infers word boundaries from geometry, so it is **never usable as a
  fidelity reference**.
- `line-anatomy.py`, `corpus-parity.sh`

### The CLI

```
dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
xray-office render FILE --format pdf --outdir DIR
xray-office extract FILE
```

Pin `SOURCE_DATE_EPOCH` and `TZ=UTC` for comparable renderings.

---

## 7. Rules that have each cost this project a round or more

### Every predecessor claim reproduces to the digit and the sentence attached to it is wrong

This is the **dominant pattern**. Rounds 42 through 46 each refuted their brief's central item and
each shipped a real fix found underneath it. Write briefs that say so explicitly, and treat a
refutation backed by two independent measurements as a full result worth publishing.

A partial list of what has been refuted after being believed:

- "We never draw bold" — reproduced on two documents; the embedded font programs said
  `LiberationSans-Bold`. The real defect was `/BaseFont` named from family rather than PostScript
  name. Zero pixels moved.
- "The 1/100 mm grid explains the drawn font size" — the real law is a **720 dpi device round
  trip**, exact on 178/178.
- "The column-fit predicate" (sheets) — zero column widths and zero page terms differ over 55
  sheets of seven documents. The real axis was **row heights**.
- "`PlotAreaOf` reserves per edge where `adjustInnerSize` reserves a bounding box" — it *is*
  `adjustInnerSize`, within 0.09 pt on every edge.
- "`.doc` `ilfo` is nought → lost `sprmPAnld` numbering" — the document contains **zero**
  `sprmPAnld` bytes and LibreOffice's own export has **0** `<text:list>`.
- "There is a table row-height law" — 3410 of 4264 paired cell edges agree within 1 pt, the 854
  that differ have **no direction** (383 ours-taller, 471 ours-shorter), and 35 of 76 *matching*
  page-exact documents differ by ≥10 pt.
- "The ±1 page cluster has a shared cause" — not page capacity, not empty pages; its divergence
  pages run from page 1 to page 91.
- "The document-level `w:settings/w:widowControl` does something" — inert in 24.2.7.2.
- "The AIRBUS table review was probably made against PowerPoint" — **wrong; the user was right.**
  Its `a:tblPr` does name a style id and the reference draws 30 `#FBECE7` + 25 `#F8D7CD` fills.

### Run every classifier over the documents that already match

**Five instrument clusters have died to this one control.** On words, run over all 200 rather than
the 46 failing, the divergence classes came out:

| dominant kind | matching (154) | failing (46) |
|---|---:|---:|
| no divergent page at all | **71** | **0** |
| `glyphs` | 54 | 29 |
| `one-sided` | 13 | 5 |
| `box` | 5 | 8 |
| `face` | 5 | **0** |
| `size` | 4 | 3 |

What separates a failure from a pass is **whether a divergent page exists at all**, not what kind
it is.

But know the limit: the test is **"could this observable be produced by anything other than the
defect?"** A direct comparison of two font lists fired on 45% of matching documents and was a
*real* defect on half the corpus. A high rate on matching documents means the gate is blind to the
class, which is worth knowing and is the opposite of a reason to drop it.

### Two blind readers agreeing is weaker evidence than this project has been treating it as

`page-vision` says a class showing up in several unrelated readings is worth more than one
showing up in three pages a single reader looked at, and § 9 records the user's visual reports
outperforming every metric. Both are true. But a round dispatched on the strength of *two blind
reviewers, on unrelated documents, independently reporting "the reference draws a legend and we
draw none"* found that neither observation meant what the agreement implied:

- On `003_advanced_excel_pie` **both sides draw a five-entry legend** — on page 2. `--worst`
  selects page 1, and the "legend swatch" the reviewer described there is the reference's M1 data
  label. The round's own fresh reviewer, given the same image, **reproduced the misreading**, which
  is what identifies it as the instrument rather than the reader.
- On `057_Simple_balance_sheet` the observation is real but the mechanism is not selection at all:
  the chart *declares* `<c:legend legendPos="b">`. Its series carry no `c:tx`, so the entries are
  LibreOffice-synthesised `Column C` / `Column D` names — a naming hypothesis, not a selection one.

**The agreement was on a description, not on a mechanism.** Two readers can produce the same
sentence about two different causes, and one of them can be an artefact of which page the tooling
chose to show. So:

- Ask reviewers for **direction and location**, then check that the two reports are about the same
  *object* before treating them as corroboration.
- **A reading is an observation, never a diagnosis** — that is already in `page-vision`, and this
  is what it costs when the step from one to the other is skipped.
- When a reading is going to launch a round, **re-derive it from a page you chose for a stated
  reason**, not from `--worst`. `--worst` ranks by ink and the largest ink difference on a document
  is frequently not the defect being discussed.

**And the positive control, so this is not read as "distrust blind readers".** One round later,
two reviewers on unrelated documents *and unrelated pages, neither chosen by `--worst`*, named the
same object — the reference's green slicer advisories, absent from ours — and `pdftotext` confirmed
it independently at 3/2/1 against 0. Real, and it became one of that round's two shipped fixes.

**And the mirror, found by the parent looking at a page directly on 2026-08-21 after the user asked
whether vision was actually being used.** Three reviewers, over three rounds, reported *"the
reference draws a legend and we draw none"* on one page. Three rounds refuted it — correctly, on
the words used: **both sides draw the five-entry legend, on page 2.** The item was closed each time.

**The readers were right about the observation and wrong about the name.** The reference draws a
**legend-key swatch beside every data label** — `pdf-ops.py` counts 10 small filled rects on the
reference's page 1 against our 5, the extra six sitting at the label positions in their series
colours — from `c:showLegendKey val="1"`, which our source consults **only inside an existence
test** and never draws. **62** elements in 5 documents, all sheets.

**And the parent's own census of that figure was wrong, by the trap the parent had been warning
every round about.** It first reported **68**, because the walk counted
`003_advanced_excel_pie.XLSX` and `.xlsx` — one inode, two names — as two documents. The
per-document listing beside it *had* case-folded and so read correctly at 5 documents; only the
total summed over raw walked paths. The sheets round caught it within the hour. **Case-fold at the
point you accumulate, not only at the point you print.**

**A refutation of the description is not a refutation of the observation.** And the *recurrence* was
itself the signal: one sentence from three independent readers across three rounds is evidence that
something real is there, not evidence of a reliably-wrong reviewer. When a reading keeps coming back
after being refuted, re-read the page rather than the refutation.

**The discriminator is not the number of readers.** It is: are the reports about the *same object*;
was the page chosen for a stated reason rather than by maximum ink; and does a *different
instrument* confirm it. All three held there and none held in the false case. `--worst` ranks by ink and the largest ink difference on a document
  is frequently not the defect being discussed.

### An instrument can manufacture a defect out of nothing

`pdf-ops.py` anchored a stroke at its top-left corner, and the left and top edges of a rectangle
share that corner exactly, so greedy nearest-neighbour paired our vertical rule against the
reference's horizontal one at every table corner. Of 439 box notes: 142 cross-orientation, 146
hairline, 151 genuine. A whole round was dispatched to chase the class.

Before believing an instrument's first big result, run it on a case whose answer you already know
and check it returns near zero.

### Estimate reach from what a shape *resolves to*, not what a part *declares*

A slides round predicted 35–55 changed renderings from a grep and measured **2**: 87 decks state a
circle-path gradient, 81 with an out-of-box focus, and **not one changed a pixel**, because they
all state it in a *theme* part and a theme's third fill style is almost never what a drawn shape
resolves to.

**And the mirror, which is worse.** A census that searched each `w:p` for its own `w:spacing`
missed style inheritance, gave a ceiling of 2–4, and the round predicted 2–4 and measured 11. A
low prediction that comes true reads as well-calibrated, so an under-reaching census conceals
itself. **Write down what the census cannot see** — inheritance, defaults, the other reader, the
other format — in the prediction file, before the sweep.

### A measurement of the wrong tree announces nothing

Two related traps, and the second was hit while merging the last three rounds.

**`git status` compares the tree to HEAD, so it cannot see a bad HEAD.** A round ran its
cross-track sweep by checking `dotnet/src` out at the base commit, and a following `git add -A`
committed the revert. Twice. The branch held the round's tests and results with **none of its
code**, and `git status` was clean throughout.

**`git checkout <commit> -- <path>` writes the index too, so an A/B measurement leaves a staged
revert behind.** This is a third route to the same destination and it was hit on 2026-08-20 while
measuring a shared-layer change's cross-track reach. The before-leg checks the base version of the
changed files out; the restore afterwards — done correctly, `cp` + `touch`, exactly as prescribed —
makes `git diff HEAD` empty and rebuilds the right binary, while the **index still holds the base
blob**. `git status --short` prints `MM`, which reads as ordinary local edits. Commit there and the
round's own fix is reverted into the branch, with the full test suite green because it ran against
the correct binary. Assert it away in the script rather than hoping to notice:

```sh
git diff --cached --stat -- <files>    # must be empty; `git reset HEAD -- <files>` if not
```

**The shell's working directory is not where the last `cd` put it.** A merge session ran
`cd <primary> && git merge …` and then issued the next two merges with no `cd`. Both landed on the
**agent worktree's** branch. `git log` in the primary showed a clean fast-forward of the first
round only, and the build and full test suite that followed measured **the worktree, not the tree
being merged into** — reporting counts that were correct for the wrong checkout.

What makes that one worth internalising is that **every check passed**: the merges reported
success, the test counts matched what the rounds had predicted, and the tree was clean. The
numbers being right is not evidence you measured the right thing.

```sh
git diff <base>..HEAD --stat -- dotnet/src        # a round that shipped code must show a diff
git -C "$PRIMARY" rev-parse --abbrev-ref HEAD     # before merging
git -C "$PRIMARY" log --oneline --graph -3        # after: the merge commit must be there
```

### A dead agent leaves three kinds of uncommitted work, and they are not the same

All three rounds finished by the parent had committed as they went and had uncommitted working
trees. **Never merge a dead agent's branch on the strength of its commit log** — in two of the
three cases the log read like a finished fix and the working tree said otherwise:

- **A stray probe script.** Merge as found.
- **A broken mid-edit.** One removed a `MoveTo` from an arm its committed fix did not touch and
  **failed the round's own test**. Running the round's own tests against the uncommitted tree
  settles this in seconds and is the first thing to try.
- **A refutation of the round's own last commit**, with the counter-measurement already written
  into the comment. Merging as found would have landed a change its author had measured and
  rejected. Commit the revert; the round's result *is* the refutation.

### A full disk looks exactly like a catastrophic regression

A fidelity run reported `Failed: 357, Passed: 193`. It was `No space left on device`. Two tells,
either alone sufficient: every test in a class dies at **< 1 ms**, and the failure list **spans
families the change cannot reach** (spreadsheet and ODF tests failing on a word-processing
change). `LibreOffice produced no output converting …` across unrelated families is the same
thing. `df -h /`, delete rendered PDFs, re-run.

**Disk is the binding constraint.** Each agent worktree is ~2.3 GB (checkout plus build
artifacts); three live worktrees plus the LibreOffice tree leaves roughly 5 GB free. Delete merged
rounds' scratch as you go.

### Other traps, each learned the hard way

- **Byte-comparing renderings does not measure reach even with `SOURCE_DATE_EPOCH` pinned** — all
  200 words renderings differ on `/CreationDate`. Normalise it out first.
- **A byte-reach outlier measured under load is not a finding** until re-run alone: a render
  truncated under load leaves a file that exists and differs.
- **Never `pkill -f <toolname>`** — agents share the process table, and `pkill -f pdf-image-diff`
  would have killed another agent's comparison. Match on your own script's full path.
- **`nohup … &` inside a backgrounded tool call orphans the sweep** to ppid 1; a second run then
  appends to the same `rows.tsv`, giving duplicate paths and spurious timeouts.
- **The container suspends between tool calls**, so a backgrounded `dotnet test` makes almost no
  progress. Run long test work in a **foreground** call, one at a time — two runs writing to one
  output file has produced garbage three times.
- **A relative `scratchpad/` path lands inside the repository** — one agent wrote 406 MB into the
  repo root. Absolute paths only. `scratchpad/` is in `.git/info/exclude`.
- **Check the worktree's base commit before measuring.** Three agents reported bases 247, 249 and
  279 commits behind. The tell that it worked: your baseline sweep reproduces the briefed numbers
  exactly. If it does not, stop.
- `mv backup original` restores an older mtime, so even a plain `dotnet build` can measure a stale
  binary.
- An unquoted `$id` makes `pdffonts` fail on filenames with spaces; a census globbing `words/*/*`
  reads directories, not files.

### The method that works

Every productive round in the last stretch used the same one: **authored variants read against the
installed binary, varying one thing at a time, with at least two points so a slope is fixed, and a
probe that measures the thing you would otherwise assume.** Then pin the refuted alternatives with
tests. Round 46's nine variants at five straddle positions, one of which measured the room at the
foot of the page rather than assuming it, is the model.

---

## 8. Per-track open items

> Scoreboards below are at HEAD under the **corrected** word check and the **current** environment.
> Items closed since the last handover are struck through where the closure is itself instructive;
> `dotnet/TODO.batches.md` carries a merge note per round with the measurements.

**The live fronts, largest first:**

1. **Slides text metrics — 17 of the user's 30 observations, one class.** Nine used the identical
   words "text sizes are different". Every affected deck is page-exact and passes the word gate, so
   no gate column can see it. `dotnet/probes/user-review-slides-02/review.md`.
2. **The `.ppt` text shadow — 36 of 51 decks, 843 runs.** Character bit `0x0010`, unread and with
   nowhere to hold it. LibreOffice's rule reproduces to the digit at three sizes; the blocker is
   that those three points do not separate Liberation Sans's `hhea` sum from its `OS/2` typo sum,
   and shipping on that would be a rounding rule resting on an unresolved metric.
3. **Hyperlink underline and colour — 41 of 112 `.pptx`, 297 runs.** A hyperlink run stating
   neither gets both from `textrun.cxx:161-166`.
4. **The ~0.1% advance divergence — withdrawn 2026-09-06; it was the instrument.** Measured
   through the reference's own stated pen instead of through a PDF's glyph positioning, over 5
   faces x 6 units x up to 11 sizes, the two stacks agree to 0.011%. What accumulated *between*
   the tab stops is LibreOffice's PDF writer declaring every glyph width as
   `floor(hmtx x 1000 / upem)` and correcting only per-gap, so a reconstructed pen falls half a
   thousandth of an em behind per glyph. The 8 Fidelity failures are left failing on purpose;
   only writing our own PDF with those truncated widths would close them. `probes/advance-ppem/`,
   and `CLAUDE.md` rule 3, which is rewritten.
5. **`apron-area.xls` page 1 draws no grid at all on our side** — 70 vertical and 56 horizontal
   reference hairlines, three missing border classes — **while matching the gate exactly**.


### Words — 155/200

- **The list-label rule is done and its reach was 1 of 200.** Round 47 established, against round
  46's citation-derived claim, that a list label **does** raise the line-spacing base height —
  31 authored probe rows, 12 wrong before and 0 after. The 51-of-134 ceiling round 46 quoted was
  badly loose: refined to levels stating a `w:sz` *larger than the document default run size* with
  proportional spacing above 100%, it is 17 of 134, and the one document that actually moved is not
  among them — it is taller through its **face** rather than its `w:sz`, which the prediction had
  named as a blind spot in advance. **Faces, not sizes, is the unworked half.**
- **The three large page outliers**, holding most of the remaining page error:
  `AC-150-5370-10G-updated-201604.docx` 687/697 and `150-5370-10H.docx` 714/721 (two revisions of
  one document), and `A_320.doc` 141/150 (the printer-device document).
- **`template---tpr…docx` is fixed** — it was not a table metric. Sixteen of its fifty-three styles
  carry no `w:name`, and `StyleSheetTable::sprm` appends a nameless entry to neither the style table
  nor its identifier map on an OOXML import (`StyleSheetTable.cxx:774`), so it cannot be referenced
  at all. We were honouring a table style the reference never resolved, setting cell text on a
  13.45 pt pitch against the reference's 15.45. **7/8 → match.** Also refuted there: that
  LibreOffice puts `w:docDefaults` *above* a table style — six authored variants say it does not.
- ~~**249 legacy `FORMCHECKBOX` fields across 16 documents** — established, deliberately not
  implemented: the drawn square's size would not pin (9.0…15.9 pt, not following
  `w:checkBox/w:size`).~~ **Both halves refuted and implemented, round 56.** The census is **675
  boxes in 12 documents**, all `.docx`, counted over every part of every package. And the size pins
  exactly: the portion is a square of `rInf.GetTextHeight()` with the line's own ascent
  (`portxt.cxx`:1492) and the drawn rectangle is that square deflated by a hard **25 twips a side**
  (`inftxt.cxx`:1247), crossed when ticked. **9.0…15.9 pt was a range of font sizes read as a
  failure to pin**, and `w:checkBox/w:size` — which 109 of the 675 state — is inert on four values
  from 5 to 40 pt. Drawn and, more to the point, *charged to the line*: 249/249, 152/152 and 48/48
  squares against the reference on the three densest documents, sides identical to 0.000 pt, with
  zero verdicts and zero page counts moved. **Censused in round 58**: the `.doc` arm is 103 boxes
  in 4 documents and the `.rtf` arm has no witness in this corpus. Neither is implemented.
- **Synthetic oblique was lost a *second* time, at glyph fallback — fixed, round 58, in
  `XRay.Office.Text`.** `IGlyphFallbackResolver.ReferenceFor` is a reverse lookup from a face with no
  request to compare against, so a run whose glyph came from a fallback face was drawn upright
  however italic it was. Measured over **six** filters (`.docx`, `.fodt`, `.fodp`, `.fods`,
  `.pptx`, `.xlsx`) on 41 authored two-run packages, four negative controls at nought on both sides
  in all six. **The reference does not go looking for an italic face**: Hebrew from an italic
  Carlito run is drawn in DejaVu Sans *sheared*, not in Liberation Sans Italic upright — so the
  fallback order was already right and only the lean was missing. Reach measured, not argued:
  words 2 renderings, slides 0, sheets 1; zero verdicts on all three tracks.
- **The rest of the fallback-lean gap is two different things, and a summing census hid it.**
  Per face (`probes/words-r58/fallbackfaces.py`): of the reference's 289 leaning glyphs in faces no
  words document names, **206 are reachable** (we draw the same face, upright) and **83 are not**
  (we do not draw that face at all — `1228841571067…doc` 74, `1257259179492…doc` 9, a
  fallback-*order* divergence). On slides the split is **0 reachable and 341 unreachable**:
  `outlook_of_nigerian_pension_sector.ppt` draws 355 WenQuanYi Zen Hei glyphs on the reference and
  **none at all** on ours. A census that summed the fallback faces together read that as a lean
  defect and predicted 345 glyphs of movement where the true answer was nought.
- **164 of the 206 reachable glyphs are list bullets, and the label is a different seat.**
  OpenSymbol's whole 112-glyph column across ten documents is single `<01>` draws one per line at
  the left margin, which reach the page through `PageDrawing`'s label branch and never through
  `ByFace`. The rule is pinned (`probes/words-r58/label-and-autocolour.py`, five authored
  packages with a control): **the level's own `w:rPr` leans the bullet, the paragraph mark's
  `w:rPr` leans it, a run's `w:rPr` does not.** We lean it in none of the three. Not implemented.
- **`AFS-050-004-F2_0i.docx` page 2's banner rows are present, painted, and the wrong colour.**
  The question the standing record could not answer — present-but-unpainted or never-read — is
  answered: **neither.** Both strings are in our text layer at the reference's positions; the
  reference draws **305 glyphs `1 1 1 rg`** on that page and we draw **none**, because we do not
  resolve an automatic font colour against a dark cell fill. The rule is pinned exactly over 22
  fills: **white when `Color::IsDark()`, which is `GetWCAGLuminance() <= 87`** — confirmed to the
  single sRGB step, grey `0x9E` white and `0x9F` black, and correct on all seven primaries too.
  Not implemented: it needs the *background behind a run* at the drawing pass, which the table
  renderer owns and `PageDrawing.RunsIn` cannot see. The same page also shows three shaded header
  cells the reference fills and we do not (8 filled rectangles against our 5).
- **The `FORMCHECKBOX` corpus census is now complete: 778 boxes in 16 documents.** Round 56's
  "675 in 12, a floor" is closed. The `.doc` arm is **103 boxes in 4 documents** — `f111.doc` 58,
  `1528364855.doc` 37, `foca_form_1.doc` 4, `LHD-230-…-aircraftr.doc` 4 — by **two independent
  instruments that agree to the digit**: a byte search for `FORMCHECKBOX` in both WW8 text
  encodings, and LibreOffice's own reader via `.doc` → `.docx` counted by round 56's rule. **The
  `.rtf` arm needs no probe at all: the words corpus holds no `.rtf`** — 271 `.docx` and 66
  `.doc` — so it has zero witnesses. `probes/words-r58/doc-checkbox-census.py`.
- **An ODF end-of-paragraph inline object contributes no height** — measured and reproducing, not
  fixed; the fix needs a line list `MeasureLine` does not have, and no ODF document is in this
  track.
- **The table-only-header import defect is the reference's, not ours** — LibreOffice copies a
  section's header forward only when the source header holds at least one top-level `w:p`.
  Established by bisection, costed at exactly **one** verdict, unimplemented per CLAUDE.md.
- Untouched: the `.doc` reader-split clusters; two round-38 leads (two `sprmCSymbol` slots in one
  paragraph both emitting the first one's code point — it is in whatever resolves a character's
  CHPX per position, not that sprm; and `absrc-pac-01-info-note-en.doc` page 1, a `one-sided`
  note, to be treated with the suspicion that class earns); `手机免提系统TSB.doc`, under-drawing
  4 words and embedding one face the reference does not; Escher picture cropping, implemented
  nowhere in the word path.
- `dotnet/TODO.raster-ceiling.md` lists 37 pages the word gate cannot win.

### Slides — 144/163

Round 41 closed most of the chart cluster. **Its whole-track sweep was never run by the agent** —
the parent measured 151/163 at the merge, so the chart work is known not to have cost a verdict,
but the per-document ink movement across the track is unmeasured beyond the three review documents.
That sweep is cheap and worth doing before the next slides round picks a target.

What round 41 settled, so it is not re-derived:

- **The chart blocker was a route, not a rule.** `DrawingTheme` and `DrawingStyleMatrix` were both
  read, both correct, and neither reached `DrawingChartPlot`; `PptxSlideLayout` had held the matrix
  for rounds and passed only the colour scheme. Sixth instance of that shape here. Ported from
  `objectformatter.cxx`: three automatic format tables over all 48 chart styles, the four colour
  patterns, `getPhColor`'s shade/tint, `LineFormatter`'s relative line width (theme
  `a:lnStyleLst[1]` × 300% at the default style, so 2.25 pt rather than our hairline), and the
  pie/`c:varyColors` point cycle. `Demick_JetBlue` ink 35.97 → 29.13, `N2_E_Maestroni` 2.36 → 1.72.
- Three chart readings replaced, each pinned by tests the refuted reading fails: `c:marker` absent
  means an **automatic** marker, not none; a chart space's **stated** `c:spPr/a:ln` is drawn; a
  series' `a:gradFill` is read as its **middle stop** rather than as no fill — reading it as no fill
  drew none of `N2_E_Maestroni`'s 111 bars. `TypeGroupModel::mbShowMarker` is parsed and read by
  nothing in all of `oox` and `chart2`.
- **`16 - UTM - (NASA).pptx`'s notdef boxes were never a font problem.** `Bezier(continueFrom:
  true)` emitted `MoveTo(whole[0])` unconditionally while a path was recording, so every
  `EMR_POLYBEZIERTO` started a fresh subpath; one glyph outline cut into per-record fragments and
  filled even-odd draws a solid blot. 15 of 163 documents carry such a record. **The
  glyph-fallback-in-chart-text hypothesis is refuted** — do not spend a round on it.

Still open on the track:

- **`Demick_JetBlue.pptx`'s missing subgrid** was not addressed; the legend and line colours were.
- ~~`Fundamentals_Module_1_basics.ppt` and `W3_Case_Study…` — arrow shapes drawn as
  rectangles.~~ **Stale — closed by round 21, and this entry outlived it by twenty-six rounds.**
  The shapes do reach `PptShapeGeometry.PresetOf`: `Fundamentals` carries 7× type 69, 2× type
  104 and 1× type 13, `W3` 3× type 103, none with `pVertices`, so the preset branch is taken at
  `PptSlideLayout.cs:939`, and all 148 names resolve against `PresetShapeGeometry.txt` with none
  missing. Both decks render proper arrows; max `|ink|%` on any page is 0.89. Round 21 measured
  the fix at the time (`TODO.batches.md:9927`, ink 7.18 → 3.37 and 7.61 → 3.78).
  **The instruction attached to it was sound and is what caught it** — "establish whether these
  decks' shapes reach that table before assuming an entry is missing" — so the lesson is not
  about arrows. A defect list is not self-expiring: an item that no round re-measures survives
  every handover it appears in, and the two documents named here were carried forward by three.
- `8_P-Pavese_AIRBUS…pptx` — missing orange table backgrounds and borders. Its `a:tblPr` **does**
  name a style id and the reference draws 30 `#FBECE7` + 25 `#F8D7CD` fills. PowerPoint's 74
  built-in table styles were ported from `predefined-table-styles.cxx`; find out whether this
  deck's style id resolves against that table.
- `OnTrac…` — a large background page number the reference draws grey and we draw black, plus a
  position shift. `Thailand17.ppt` — image scaling, reference renders it taller.
- `Wildlife for REDAC September 11.pptx` page 3 (picture drawn unrotated) and page 13 (two blocks
  we draw and the reference does not). Its `path="rect"`/`"shape"` gradient geometry is still
  wrong but **no corpus deck states one**, so its reach is zero.
- **Three the user reviewed as false positives, where we render better than the reference — do not
  work them**: `NAS-Infrastructure-Roadmaps-v16.0.pptx`, `WiGr_2021W…`, `FAAAIandtheArt…`. Also
  `NWD-GLA…pptx`, "looks exactly the same, check if the word count is not misleading" — worth one
  measurement, not a round.
- `glyphs` is dead as a slides class too: dominant on 66 of the 151 that pass (44%). `size` is
  dominant on 22 matching and **0** failing.

### Sheets — 146/171

Round 40 worked the page-split cluster and **its result was a refutation** — read it before
starting, because the obvious repair is measured and rejected.

Ten of the sixteen sheets failures have a wrong page count. Ours minus the reference:

| document | pages | Δ | | document | pages | Δ |
|---|---|---:|---|---|---|---:|
| `orbus_togaf_tool_csq.xls` | 33/75 | −42 | | `grants-2005.xls` | 219/220 | −1 |
| `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` | 154/175 | −21 | | `ans_mappings_of_eccairs_terms.xlsx` | 190/191 | −1 |
| `aircraft_analysis_2016-04-27.xls` | 44/46 | −2 | | `7-memento-2015-transports-aeriens-b.xls` | 190/191 | −1 |
| `FY2018_Q4_UAS_Sightings.xlsx` | 304/302 | +2 | | `SIL_TDB648.xlsx` | 89/88 | +1 |
| `FAA-2019-0995-0002_attachment_2.xlsx` | 32/33 | −1 | | `CSJU List of Recipients…xlsx` | 97/96 | +1 |

**The two large ones, −42 and −21, are untouched and are most of the track's page error.** The rest
sit at ±1 or ±2.

- **Refuted: the print band's right edge does not follow the allocated columns.** Extending to the
  last allocated column fixes `CSJU` (97 → 96) and breaks two others — `fy20-may20-sep20.xlsx`, a
  sheet with no data at all, allocated to column E, which then prints a page it should not, and
  `fm-provider-service-measures.xlsx`, whose data stops at column C and whose closed run reaches T,
  which then fits to a smaller zoom and loses two pages. `bFound` in `ScTable::GetPrintArea` is set
  by the data loop and `GetLastVisibleAttr`, and by nothing else.
- What survives from that round is the **scan rewrite** — a column's attribute array walked as
  runs, so a whole-column or sheet-wide format participates. Its only measured effect on the gate
  is one word: 27163 → 27162.
- **`7-memento-2015-transports-aeriens-b.xls`** — on page 2 the reference draws 115.28 pt verticals
  at x = 119.99 and 512.39 in `#0066CC`; we draw a **single 13.1 pt segment** and nothing below it.
  A merged block the decoration path knows about emits its left edge once per covered row. One
  segment says **the block is not in our model as a merge** — find why that `MERGEDCELLS` range
  never reaches `StatedMerges`.
- **3.4 twips on a non-wrapping multi-line row** (`bStdAllowed = false` for an edit-cell row):
  confirmed, unimplemented, corpus reach unmeasured.
- The narrower leg of the solidus line-break rule needs the fitting limit to the character.
- We do not pick the reference's fallback face (`IPAGothic` + `WenQuanYiZenHei` against
  `WenQuanYiZenHei`), and every affected row height is still exact.
- User review items not yet addressed: `Keywords_Mapping…` (table border, chart border, "chart
  vertical scale is very different"); `Template Pilot Logbook…` (angled horizontal axis rendered
  horizontally; chart area drawn as a rectangle instead of polygons); `T0A0D0000090006XLSE.xls`
  (text sizing causing different wrapping); `sectors-defense-and-aerospace.xlsx` (empty cells
  missing shading); `ans_mappings_of_eccairs_terms.xlsx` (link colour); `grants-2005.xls` (header
  text not cropped to cell size); `Capability_List…` ("some cells are taller" — this observation
  identified the real axis of a 14-document cluster after a column-width hypothesis had been
  refuted).
- **Sixteen of the fifty documents that changed in round 37 are measurably further from the
  reference**, by 0.14 to 2.07 of `|ink|%` against gains of up to 21.80. Small, real, unfollowed.

---

## 9. The user's review feedback, and how much it has been worth

The user reviewed rendered pages side by side and produced **24 defects, none visible to any
gate**. Two of their observations outperformed every metric on the project: "some cells are
taller" identified the real axis of a 14-document cluster, and their page-split calls prompted a
cross-document filter that turned a 3-document lead into 14. **Take their visual reports as
primary evidence.** When one has been contradicted by a brief, the brief has been wrong.

Status of the words items they raised — all resolved or costed:

| document | reported | now |
|---|---|---|
| `omrIMInterpretiveGuideLine.doc` | missing header frame / image | **fixed, matches** — group members never placed, envelope painting an opaque white box over body text, greyscale JPEG declared `/DeviceRGB` |
| `UG.CAO.00133 …docx` | page numbering and a section break | **half fixed** — numbering now counts 1 to 18; the header on 18 pages against 5 is LibreOffice's own import defect |
| `手机免提系统TSB.doc` | missing all Asian characters | **fixed** — not missing text; a font tie-break that was alphabetical by its own admission sent 138 records to a CFF face the PDF writer can name but not embed |
| `FO.FCTOA.00010 …docx` | missing image | **diagnosed, open** — not an image at all; 249 legacy `FORMCHECKBOX` fields |

And one slides item, which is the clearest example on the project of a visual report beating the
diagnosis attached to it:

| `16 - UTM - (NASA).pptx` | "some Unicode unknown character rendering… in the chart" | **fixed** — read as a glyph-fallback hole in the chart text path, and it was not: an EMF `BezierTo` inside a recorded path started a fresh subpath per record, so one glyph outline filled even-odd drew a solid blot |

Their methodological request, which produced `first-divergence.py`, in their own words:

> "It would be great to approach this more methodologically moving forward. Maybe split the
> documents into increasingly longer pages (1, 1-2, etc) and compare them to find where they
> break, then it should be possible to identify what's in that page that changed that is
> different."

---

## 10. The published artifacts

Three HTML proof sheets — one per track — showing only the non-matching documents, our rendering
and the reference side by side, organised by batch, with the user's review notes as an annotation
layer and a first-divergence line per document. Built by:

```
<scratchpad>/artifacts/collect.py    renders each failing document both ways, picks the page with
                                     max |ink|% over the common prefix, emits base64 JPEG data URIs
<scratchpad>/artifacts/build.py      emits the HTML with inlined subset WOFF2 fonts (Carlito
                                     Regular/Bold + Caladea Bold, 74 KB via pyftsubset --flavor=woff2)
<scratchpad>/artifacts/notes-*.json  the review-note layer
```

`build.py` now supports note kinds `fixed`, `part-fixed` and `diagnosed` alongside `defect`,
`ceiling` and `open`, with a tally line, so a resolved item keeps its record instead of falling
out of every count. `notes-words.json` is current.

**The words artifact has not been republished since those fixes landed**, because the collected
page images predate them and a page captioned "fixed" showing the old rendering would be worse
than not republishing. Re-run `collect.py` for the track before rebuilding. Republishing the same
file path keeps the URL.

---

## 11. Security and privacy constraint, still in force

One document used in an earlier round is a **real person's CV** — a `.docx` restored to the scratch
directory for diagnosis only. It is not in the corpus and not in this repository, and it stays that
way. Its filename contains the person's surname, which is why it is not written here.

- Do **not** copy it into the repository or anywhere under version control.
- Do **not** quote the person's name, contact details, employers or any other personal content in
  reports, commit messages, code comments or test names. Refer to it as "the CV".
- The phrase `"Functional Avionics, Flight Software, Satellite Database, Fault Detection"` is a
  skills list and is fine to quote. Nothing else from the document is.
- Any regression fixture must be a **newly authored minimal document**, never a copy or excerpt.

---

## 12. Suggested first moves

1. `git log --oneline -1` and `git status --short` — confirm **`0fb6d41e0`** or later, clean.
   `git worktree list` should show only the primary checkout.
2. `df -h /` — the binding constraint. Each agent worktree costs ~2.3 GB and a whole-track sweep
   costs a few more; clear merged rounds' scratch before starting anything that renders.
3. **The user's last instruction was not to start new agents.** Do not re-establish the
   one-per-track loop until they ask for it. When they do, §2 is the standing goal and §8 has the
   targets.
4. If a round is dispatched, the first cheap thing worth having is the **slides whole-track ink
   sweep** round 41 never ran — its verdicts are verified, its per-document movement is not.

When briefing an agent, give it: the exact baseline figures to reproduce before it believes
anything, the worktree command, the refuted list for its track so it does not re-derive, the
rules in §7 that bear on its task, the standing requirement to commit a prediction first, and the
expected test counts for its final tree. Tell it to commit but **not** to merge or push — merging
is the parent's job, and combining is how the scoreboard stays trustworthy.
