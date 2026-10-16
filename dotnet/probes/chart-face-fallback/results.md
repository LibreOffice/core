# The two references do not agree on what an unknown family resolves to, and 26.2 is the odd one

Round `agent/chart`, 2026-09-05.

## Why this was asked

Two of the nine documents catalogued under *charts drawn wrongly* have a residual against
26.2.4.2 that is not a chart defect at all:

* `Demick_JetBlue.pptx` page 4 — the reference's chart text is **Noto Serif** and ours is
  **DejaVu Serif**, which is 12% wider, so our value-axis labels reserve more of the plot's left
  edge and every gridline in a dense minor grid lands a pixel or two out. `pdftohtml -xml`
  reports the same two em sizes on both sides (15 and 27), so it is the face and not the size.
* `057_Simple_balance_sheet_…xlsx` page 3 — the reference's is **Noto Sans** and ours **DejaVu
  Sans**, and the difference is what makes our twenty rotated category labels collide where the
  reference's do not.

Neither deck names those faces. `Demick`'s theme minor is `Constantia`, which LibreOffice
classifies `Normal,Serif` and nothing else
(`officecfg/registry/data/org/openoffice/VCL.xcu:1385-1389`); `057`'s chart names
`Franklin Gothic Book`. Both are families this container does not have.

## The probe

`make-face-probe.py` rewrites one attribute of one authored deck: the theme's minor latin
typeface becomes `ZzzNoSuchFamily`. Nothing can resolve it, so what comes out is each engine's
last-resort answer.

| engine | face in the PDF |
|---|---|
| `fc-match ZzzNoSuchFamily` | DejaVu Sans |
| `soffice` 24.2.7.2 (distro) | **DejaVu Sans** |
| `/opt/libreoffice26.2/program/soffice` 26.2.4.2 (TDF tarball) | **Noto Sans** |
| X-Ray.Office | DejaVu Sans |

## What it means

**We agree with the distro binary and disagree with the tarball, and the tarball is the outlier
because it ships fonts the system has not got.** `/opt/libreoffice26.2/share/fonts/truetype`
carries `NotoSans-Regular.ttf`; the system carries exactly one Noto file, `NotoColorEmoji.ttf`.
So 26.2.4.2 has a Noto Sans to fall back to and nothing else on this machine does.

This is `dotnet/CLAUDE.md`'s "a TDF tarball 26.2.4.2 is NOT the distro-packaged 26.2.4.2 this
tree is calibrated against" arriving with a named mechanism and a measurement. The existing note
covers the *metric-compatible* duplicates — Carlito, Caladea, Liberation, DejaVu — and says
moving those aside takes 36 fidelity failures to 31. Moving them aside does **not** cover this
one: the tarball's Noto is not a duplicate of anything installed, so it is still there after the
recommended `mkdir .duplicates-aside` step, and it is still what an unresolvable family lands on.

Practical consequences, in the order they matter:

1. **An ink figure against 26.2.4.2 for a document naming a family this container lacks is
   inflated, and the inflation is not a defect in the tree.** Both chart documents above are in
   that class.
2. Scoring the same documents against 24.2.7.2 measures the same font set on both sides and is
   the sounder number *for those two*, which inverts the usual advice in the notes.
3. Nothing here argues for changing our own fallback. Ours already equals the distro binary's,
   which is the environment the tree is calibrated to.
