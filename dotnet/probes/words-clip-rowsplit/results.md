# The upper space a split row's follow part gives the paragraph that opens it

Round `clip`, 2026-09-05. Environment: this container, `/usr/bin/soffice` **24.2.7.2** and
`/opt/libreoffice26.2/program/soffice` **26.2.4.2** (TDF tarball, its 33 duplicate font files
moved aside), `fc-match "DejaVu Sans"` → DejaVu, Carlito and Caladea installed.
Both binaries are reported, because a difference against one is not evidence until it is
rescored against the other.

## The question

A table row is cut across a page break so the cut falls **between** two of the cell's
paragraphs. The paragraph that opens the follow part has no paragraph above it *on that page*
but does have one in the same cell on the page before. Two readings disagree there:

* it re-applies the gap the flow was laid out with — `max(before − after, 0)` under the
  collapsing rule a Word document uses; or
* it re-applies its own `w:spacing w:before` in full.

They differ exactly when `before <= after`, which is the commonest shape a real style has
(the same figure stated for both).

`probe.py` authors a one-cell, four-paragraph row, sweeps the filler above it until the row is
genuinely split (some of the cell on each page — asserted, not assumed), and reads the distance
from the follow part's own top rule to the first baseline on page 2.

`mkdocx.py` here is a separate copy of `probes/words-regress-01/mkdocx.py` **with a
`word/settings.xml`**. Without that part LibreOffice never applies its OOXML compatibility
defaults, and the collapsing rule this probe is about is one of the things that reverses.

## Result

Distance in points from the follow part's top rule to its first baseline, page 2.

| `w:before` | `w:after` | 24.2.7.2 | 26.2.4.2 | ours before | ours after |
|---:|---:|---:|---:|---:|---:|
| 240 |   0 | 12.76 | 12.76 | 12.51 | 12.51 |
|   0 | 240 |  0.76 |  0.76 |  0.51 |  0.51 |
| 240 | 240 | 12.76 | 12.76 | **0.51** | 12.51 |
| 240 | 120 | 12.76 | 12.76 | **6.51** | 12.51 |
| 120 | 240 |  6.76 |  6.76 | **0.51** |  6.51 |

**The two references agree on every row, and the answer is the paragraph's own `before` in all
five.** The old reading was `max(before − after, 0)`, which agrees only on the two rows where
nothing collapses.

The constant 0.25 pt is where the part's own top rule sits and not this spacing — it is there
on the agreeing rows too, so it is a separate (and much smaller) matter.

## Why

`SwFlowFrame::CalcUpperSpace` (`sw/source/core/layout/flowfrm.cxx:1589`) asks
`GetPrevFrameForUpperSpaceCalc_` for the frame above and takes the collapsing branch **only
when it finds one**. A follow part's first paragraph has none — the paragraph above it lives in
the master cell frame on the previous page — so control reaches the `else if` at `:1744-1755`,
where `nUpper = pAttrs->GetULSpace().GetUpper()` is the whole upper space, with neither the
collapse against the previous frame's lower space nor contextual spacing consulted at all.
`HasParaSpaceAtPages` (`:1438`) grants that branch unconditionally to anything `IsInTab`, and
`lcl_PartiallyCollapseUpper` (`:1541`) returns early for a frame in a table, so nothing takes it
away again.

## Reach

`mde087077~283.docx` (`words/done-003`), first-page-of-worst-page ink at 30 dpi grayscale:

| | 24.2.7.2 | 26.2.4.2 |
|---|---:|---:|
| before | 24.11 | 24.12 |
| after | **0.99** | **1.39** |

Per page, same document (page 1–4): `10.13 23.19 2.27 2.23` → `10.13 2.10 2.27 1.97` against
24.2, and `10.36 23.21 2.22 1.15` → `10.36 3.02 2.22 0.41` against 26.2. Page 1 holds a
separate, unrelated defect and is untouched.
