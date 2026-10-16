# sheets-rest-01 — prediction

Written before the final sweep, before `done-*` is re-proved, and before a single test has been
run in this worktree. Scored honestly in `results.md`.

**One thing about the order, stated up front because the brief asked for this file first.** The
five documents were diagnosed one at a time and each diagnosis needed a measurement, so the
per-defect probe figures below are *already measured* and are not predictions — they are stated
here only so the predictions that follow are legible. What is genuinely unmeasured at the moment
this is committed is everything in §2, §3 and §4: the whole-track sweep after the fourth change,
the reach of each change across the corpus, the `done-*` verdict, and every test count.

## 1. What was changed, and what is already measured about it

| # | seat | measured already |
|---|---|---|
| 1 | `LineFiller` breaks inside a run of blanks that overflows the line (EditEngine, not Writer) | 17 of 17 probe rows exact; `FAA-2019-0995-0002_attachment_2.xlsx` 33/33 pages, 9995 words against 9994 |
| 2 | — declined, see §5 | — |
| 3 | a `<font>` naming no face is Cambria 11, not `fonts[0]` at 10 pt | 7 of 7 probe rows exact; `ans_mappings_of_eccairs_terms.xlsx` 191/191 pages, 27893 against 27896, 7 faces against 7 |
| 4a | a rich or hard-broken cell is `CELLTYPE_EDIT` and its clipped string is not shortened | 4 of 4 probe rows exact; `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` 9021 words against 8981, inside the 179-word band |
| 4b | a rich cell that does not wrap takes one EditEngine line, not the arithmetic height | `SIL_TDB648.xlsx` page blocks now align with the reference's through page 16 and from page 18 |

## 2. The sweep — predicted

The three sweeps already run went 160 → 161 → 162 → 163 of 171. Change 4b is the only one not yet
swept.

1. **`sheets/*` finishes at 164 of 171 match.** Medium confidence. 163 is measured; SIL_TDB648
   stays a `pages` failure at 89/90, so the gain has to come from somewhere else, and the only
   candidate is a document whose rich rows were short. Call it **163 or 164, and not less than
   163** — high confidence on the floor, medium on the gain.
2. **`done-*` (159 documents) stays at 159 of 159.** Medium confidence, and this is the one I
   would least like to be wrong about: change 4b moves a *row height*, which moves pagination,
   and three of the four changes before it moved only a word count. If anything regresses it will
   be here and it will be a page count.
3. **SIL_TDB648 remains 89/90.** High confidence — its residual page carries nothing but a
   picture and no picture was added.
4. **`orbus_togaf_tool_csq.xls` remains 33/75.** Certain; declined deliberately.
5. **The five `ceiling-001` documents do not move.** High confidence. None of the four changes
   touches a raster.
6. **`fse_identification_form.xlsx` is a coin toss** and its verdict must not be read as
   movement either way — it is the recorded unstable document and it read `words` on the
   baseline sweep here.

## 3. Reach — predicted

Measured from what resolves in the sweep, never from a grep.

| change | documents moved on the sheets track | confidence |
|---|---|---|
| 1 — overflowing blanks | 7 (measured: 7) | measured |
| 3 — Cambria 11 | 1 (measured: 1) | measured |
| 4a — edit cell not shortened | 5 (measured: 5) | measured |
| **4b — rich row height** | **5 to 15, all improving or neutral** | low |

4b is the one I cannot bound from a probe: every `.xlsx` and `.ods` with automatic row heights
and a rich cell is in range, and the sheets track holds no `.ods`. **If it moves more than about
twenty documents I have got the rule too wide and it should come out**, because Calc reaches the
EditEngine branch for a rich cell and I have measured that at exactly one font size.

**No document outside the sheets track can move.** Changes 3, 4a and 4b are in
`XRay.Office.Spreadsheets`; change 1 is in `XRay.Office.Text` but behind a flag only the four sheet
layouters set. High confidence.

## 4. Tests — predicted

- **The fidelity baseline is 30 failed of 550** and I have not yet reproduced it. I predict
  **exactly 30 failed, 520 passed, 0 skipped, 550 total**, and if it is not I stop and find out
  why before believing anything above. Medium confidence — the number is a brief's claim about a
  binary I have changed.
- Every other project passes with **0 failed and 0 skipped**, at the counts recorded in
  `sheets-pagination-01/results.md` plus the new tests. Medium-high.
- The new tests, added to `XRay.Office.Spreadsheets.Tests` and `XRay.Office.Text.Tests`, **fail on the
  unfixed tree** — checked by copying the four changed files aside, reverting, building, running,
  and restoring. High confidence for three of the four; for 4b the arithmetic height and the
  EditEngine line may collide at some font size and I will pick one where they do not.
- `XRay.Office.Vector.Tests` may report a phantom failure under load. If it does I will re-run it
  alone and say so rather than reporting the first number.

## 5. The DPCache decision — predicted verdict

I predict I will **decline to synthesise the sheet**, and that the strongest argument for
declining will turn out to be LibreOffice's own: the sheet is created by
`XclImpPivotCache::ReadPivotCacheStream` for a pivot table whose source is external or deleted,
and upstream commit `6bc8bae7047` (2026-05-09, "sc: hide helper sheet for external pivot sources
in XLS") adds `rDoc.SetVisible(nScTab, false)` to that very block — after 26.2.4.2 branched. So
the 42 pages the banked reference prints are a defect LibreOffice has already fixed, and matching
them would mean writing a PTCACHE reader to reproduce output that no longer exists upstream.

**Prediction: the flat-ODF export of the file under the installed binary shows the DPCache sheet
with `table:display="true"`, and would show `false` under 27.2.** The first half is measured; the
second is a prediction I cannot test here, since the LibreOffice download hosts are unreachable
from this container.

## 6. What I expect to be wrong about

- The 164. I would not be surprised by 163.
- The 4b reach. "5 to 15" is a guess with no measurement behind it.
- Whether the `done-*` re-prove is clean. This is the round's real risk and the prediction above
  is the least evidenced thing in this file.
