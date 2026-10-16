# r170 — a positioned table's wrap, scoped before it was implemented

`Paginator.PlaceFloatedTable` floats a `w:tblpPr` table only when it fills its column, and says
so in its own comment (`Layout/Paginator.cs`:3602-3645):

> A fly with room beside it is one this cannot place: Writer wraps the flow into that room and
> nothing here can. A fly that fills the column has no such room, and Writer puts the flow
> **under** it — which is a position rather than a wrap, and is reproducible.

Round 169 saw the reference wrap prose level with such a table's first row and queued the wrap as
task 13, noting that it is an architectural change rather than a fix: `WrapObstacle` records are
built in `FrameResolution` from paragraph-anchored `PageFrame`s and keyed by page **before**
pagination places a floated table, so registering one as an obstacle is a fixed point.

**It is worth one corpus document, and no gate verdict anywhere. Not implemented.**

## The instrument

The prize is measurable at the reference alone. Strip every `w:tblpPr` and 26.2.4.2 lays that
table in the flow — which is exactly what this tree does today — so the difference between the
reference's two renderings of one document is the whole of what a wrap could buy, with no
renderer of ours in the loop and no tolerance in it. `unfloat.py` builds the pair and scores it;
`wraponly.py` pairs their spans; `whichleg.py` puts our own rendering against both.

All 42 corpus DOCX stating a positioned table were measured, not a selection.

## The census that briefed the task is refuted

`census.py` marked the ten documents whose table is narrower than 90 % of its text column, on the
reasoning that only those leave room to wrap into. Measured at the reference:

| | reference's counts move when the float is removed |
|---|---:|
| the 10 "narrow" documents | **3** |
| the other 32 | **14** |

So the heuristic selected against the truth. The seventeen that move are mostly *wide* tables —
the ones this tree already floats. `movers.txt` should not be reused.

## We are on the float leg, not the flow leg

`whichleg.py` pairs each of our spans with the reference's by page and string and reports the mean
`|dx| + |dy|` against each leg. **34 of 42 are nearer the wrapped rendering.** Of the eight marked
`FLOW`, seven are inside the noise — 1.35 against 0.86, 52.98 against 52.93, 0.17 against 0.10 —
on documents whose wrap moves nothing to begin with.

The exception is one document:

| | vs float | vs flow |
|---|---:|---:|
| `slcc-architecture-uu-architecture.docx` | 78.47 | **0.21** |

That is unambiguous: we reproduce the reference's *unfloated* rendering of it exactly, and its
wrap displaces 301 paired spans by a mean 61.56 pt vertically, 518.50 at worst. A second candidate,
`085_Printable_Graph_Paper_Template_Excellent_Format`, reads 16.70 against 0.10 on **two** spans.

**Both are page- and glyph-exact against the reference on all three legs**, so no gate column can
see the wrap on either, and none of the other 40 offers a verdict either.

## What that leaves

A fixed-point change to obstacle resolution, to move one document's prose 78 pt. Banked and not
taken. What the round does leave behind is the instrument: `unfloat.py` generalises to any
attribute whose effect can be removed from the file, and it answered this in twenty minutes of
rendering where implementing the feature would have cost a round.

The round-169 observation itself stands — the reference does wrap, and the authored fixtures in
`probes/indexwrap-r169` show it. What was never measured is how often a corpus document depends
on it, and the answer is once.
