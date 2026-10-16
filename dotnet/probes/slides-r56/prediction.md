# slides-r56 — prediction

Committed before anything is rendered post-change. Environment: LibreOffice **26.2.4.2
620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, base `e64f743dbff`, branch
`wt-slides-r56`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline reproduced

| | briefed | measured |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements** |
| `abs_ink` | 1136.53 | **1131.78** |
| major pages | 395 | **390** |
| `tf-agreement` | 0.77061 | see note |
| turned blocks (ref 1307) | 1385 | **1374** |
| sheared glyphs (ref 16008) | 16740 | **16740** |

**The 4.75 of `abs_ink` is the merge and it is five named rows, not a different tree.**
`ink.tsv` is byte-identical to round 55's final sweep on 306 of 311 rows. The five that differ
all improved, and one changed verdict — which is the parent's own 199 → 200:

| document | r55 → r56 |
|---|---|
| `WiGr_2021W_1_Angebot-Nachfrage-Elastizität…pptx` | 8.12 → **5.08**, major 4 → 1, **`words` → `match`** |
| `Structural Testing.pptx` | 6.56 → 5.37, major 4 → 3 |
| `RPA P4 - Advanced Material.pptx` | 5.68 → 5.20, major 3 → 2 |
| `dhs-293364.pptx` | 2.51 → 2.47 |
| `Ensemble-pour-l-amelioration…AIRBUS.pptx` | 0.43 → 0.43 |

That is round 55 *sheets*' `mc:AlternateContent` change arriving through the merge, exactly as
the parent's verification note describes. The briefed 1136.53 is the pre-merge figure.

`tf-agreement`'s briefed 0.77061 is **not** the number this script prints at this commit: it
prints a per-document mean of **0.85188** over 168 documents and **1678 of 4184** exact pages.
The briefed pair (0.77061 / 1709 of 4515) is a different reduction of the same script's output
and is carried forward from round 52. Treated here as a **control that must not move**, on its
own reading, rather than as a level to reconcile.

## What ships

### 1. `.ppt` `txflTextFlow` (Escher 136) — the brief's item 1

Six values, three answers, measured on 26.2.4.2 with a fixture built to **discriminate**:
`patch-textflow.py` rewrites the four value bytes of this one property in a real corpus `.ppt`
and leaves every other byte alone, so no arm states the reference's own default and the six
differ in nothing else. On `concepts-surrounding-cloud-computing…ppt` page 11:

```
0 HorzN upright     1 TtoBA 0 1 -1 0     2 BtoT 0 -1 1 0
4 HorzA upright     3 TtoBN 0 1 -1 0     5 VertN 0 1 -1 0
```

24 blocks apiece, identical pens across 1/3/5. **6 of 6 for "1,3,5 vertical; 2 the other
quarter"**; refutes "any non-zero turns the same way" (arm 2) and "only `TtoBA`" (arms 3, 5).
`0 1 -1 0` is the matrix the reference writes for `a:bodyPr/@vert`, so the two spellings are one
rendering and this reuses round 55's transposed-frame path and its rotated insets.

### 2. `c:minorGridlines` — the *actual* content of the brief's item 2, and item 2 as briefed is refuted

The brief says `Demick_JetBlue.pptx`'s 68 turned blocks are "a chart category axis the reference
does not draw at all". **It draws all 21 of them, at the same 45°, as glyph outlines** — page 4's
reference stream is 1.9 MB with 1502 curves and 200 subpath moves in the label band, against our
124 subpaths in total. That is the case `ChartAxisLabels`' own remarks already record for
`tdf106217.pptx`: an unequally scaled chart plus a 45° turn is a shear the PDF text state cannot
carry, so LibreOffice writes outlines and `pdftotext` reads nothing. So `52 BT` to `31` and
`163 words` to `79` are a **representation** difference, not a placement one, and the turn census
counting *blocks* is the same granularity artefact that nearly shipped as "we over-shear".

What page 4 actually differs by — and what the `cmp` report's *"a solid area drawn differently
(31.18% of page, x 0.10–0.87, y 0.28–0.74)"* is — is **minor gridlines**. The reference draws a
dense mesh over the whole plot area; we draw the major grid only. `c:minorGridlines` is read
nowhere in `Core/Charts`.

## Predictions

1. **Verdict movement 0**, band **−2 … +1**. `concepts-surrounding-cloud-computing…ppt` is
   already `match`; the four other `.ppt` carrying a non-zero flow are `match`, `match`, `match`
   and `words`, and none of them can gain the word gate from a turn — the reference draws the
   same glyphs either way. The −2 arm is the real risk: a shape whose flow we now honour and
   whose text the reference draws upright would move ink and words on a passing document.
2. **Page counts change on 0 of 302.**
3. `.ppt` flow — **the column is turned blocks and differing pixels, not ink.** A turned label is
   the same glyphs in a different place, so its ink can move either way. Expect
   `concepts-surrounding-cloud-computing…ppt` to go from **0 turned blocks to 4** (the reference's
   own figure) and its page-11 differing pixels to fall. Expect at most **5 documents** to move on
   any instrument, and **1 to 3** to move on ink by more than 0.005.
4. `c:minorGridlines` — **the column is ink and differing pixels**, because a gridline mesh is
   real ink over a third of a page. Expect `Demick_JetBlue.pptx` **26.10 → below 20**, its major
   pages **6 → 4 or fewer**, and `N2_E_Maestroni_Swarm_COP.pptx` and `171128IPAP.pptx` to move by
   less than 0.5 each. Expect `abs_ink` **down** overall, between **−5 and −10**.
5. `tf-agreement` and the sheared-glyph count are **controls**: 0.85188 ± 0.0005 and 16740 exactly.
   Neither change touches a font size or an oblique.
6. **Documents moved on ink: 4 to 8.** Named: `concepts-surrounding-cloud-computing…ppt`,
   `Thailand17.ppt`, `introduction_to_bea_tuxedo.ppt`, `hofman.ppt`,
   `ws_prod…Approval-of-Flight-Conditions.ppt`, `Demick_JetBlue.pptx`,
   `N2_E_Maestroni_Swarm_COP.pptx`, `171128IPAP.pptx`. This is the fourth round running that this
   quantity is at issue; the failure modes so far were extrapolating candidates (r53), censusing
   visible symptoms (r54) and censusing the reference's output in a column that could not resolve
   it (r55). **This round's exposure is the second one**: both censuses read what the *file*
   states, and a stated property on a shape or axis that draws nothing moves no pixels. The
   `.ppt` half is already known to over-count that way — 22 of the 33 non-zero flows are on
   shapes of one document that state no `TextId`.
7. Cross-track: the `.ppt` change is `XRay.Office.Presentations` and reaches slides only. The
   gridline change is **`XRay.Office.Core/Charts`, a shared layer**, and the census names its whole
   corpus reach: **4 sheets documents** (`033_Event_planning_tracker…`, `035_Project_plan_for_law_firms…`,
   `038_Baby_growth_chart…`, `039_Baby_growth_tracker…`) and **1 words document**
   (`ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx`). Predicted verdict movement there:
   **0 on both**, band −1 … 0.

## What these censuses cannot see

- **`txflTextFlow`**: inheritance through `DFF_Prop_MasterShape` (769), so a shape can take the
  property from a master shape and this counts only the literal writer; whether the shape carries
  any text at all (`TextId` is itself inheritable); whether the shape sits on a page that is
  drawn. It **can** see secondary and tertiary OPT records, which it reads.
  It also cannot see `cdirFont` (137), which interacts with the flow rather than composing with
  it — two documents and six shapes carry one, none of them with a non-zero flow, and it stays
  unread and recorded rather than guessed at.
- **`minorGridlines`**: only zip entries whose name contains `chart` or which are `content.xml`,
  so a BIFF chart embedded in a `.xls` or a `.ppt` is invisible to it, as is an ODF chart in an
  `.odp`. It cannot see whether the axis' minor interval differs from its major one, which is
  what decides whether a mesh is drawn at all.
- Neither census can see the **other reader**: the `.ppt` census says nothing about `.pptx`, and
  the gridline census says nothing about the two legacy chart readers.
