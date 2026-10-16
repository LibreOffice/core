# A fly filling its column: what moves, and what stays

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`, reference `soffice` the
distro's **24.2.7.2**. Every figure below is a fresh measurement in that environment.*

## Where this started

`HC-Bulletin-template.docx` was the worst tractable document on the words track by first-page ink —
**39.12** — and its defect is one a picture makes obvious: the HEALTH CLUSTER logo and the
photograph land at the *bottom* of page one where the reference has them at the top, half of each
hanging off the sheet. Its body begins with a positioned table
(`vertAnchor="page" tblpY="953"`, 10598 twips wide in a 9922-twip column, so it fills the column
and overflows it), and the paragraph directly after that fly carries both pictures as anchored
frames at `positionV relativeFrom="paragraph"`.

Round 50's rule was that such a fly pushes the whole flow under itself. That is why the pictures
moved: they hang off a paragraph, and the paragraph moved.

## The probes

`law.py` writes 21 one-page documents in one geometry — A4, the HC-Bulletin margins, a fly of a
single exact row at `tblpY="953"` and 10598 twips wide — and varies two things: the fly's height
(200, 600 and 710 pt) and the shape of the flow after it, written as

| | |
|---|---|
| `.` | an empty paragraph |
| `@` | an empty paragraph carrying a text box reading `MARK`, anchored at `posOffset` 0 from the paragraph |
| `A` | the inked paragraph, reading `AFTER` |

`MARK`'s y is the anchored frame's own position; `AFTER`'s y is where the flow reached. Both are
read out of the PDF text layer with `pdftotext -bbox`, so they are the drawn text rather than ink.
`clearance.py` and `anchored.py` are the two earlier, narrower cuts of the same question, kept
because each one alone gives an answer that the other refutes.

## What 24.2.7.2 does

| fly | shape | `MARK` | `AFTER` |
|---:|---|---|---|
| 200 pt | `A` | — | p1 247.7 |
| 200 pt | `.A` | — | p1 261.1 |
| 200 pt | `...A` | — | p1 288.0 |
| 200 pt | `@A` | **p1 60.3** | p1 261.1 |
| 200 pt | `@..A` | **p1 60.3** | p1 288.0 |
| 200 pt | `.@A` | **p1 264.7** | p1 274.6 |
| 200 pt | `..@A` | **p1 278.2** | p1 288.0 |
| 710 pt | `...A` | — | **p2** 70.2 |
| 710 pt | `.@A` | p1 774.7 | **p2** 56.7 |
| 710 pt | `..@A` | **p2** 60.3 | p2 70.2 |

The fly's bottom is 247.65 pt in the first block, and the line height is 13.45 pt. So:

1. **Every paragraph after the fly is displaced, an empty one included.** `AFTER` is at the fly's
   bottom plus 13.45 pt for each paragraph in front of it, and goes to page two when that runs off
   the sheet. There is no ink test: `.A` and `@A` put `AFTER` in exactly the same place.
2. **The frame anchored to the paragraph the displacement lands on does not move with it.** In
   `@A` the anchor paragraph's own line is at 247.65 and `MARK` is at 60.3 — the top of the body,
   190.95 pt above its own paragraph, which is the displacement exactly.
3. **Only that paragraph.** Put one empty paragraph in front of it and `MARK` is back at its
   paragraph's real position: 264.7 against a paragraph top of 261.1.

Which is Writer's formatting order rather than a rule about anchors. `SwObjectFormatter` positions
a text frame's anchored objects when the frame is first formatted; the fly's wrap then moves the
frame, and the objects are not positioned again. A paragraph with a displaced predecessor is
already being formatted below the fly when its turn comes, so nothing moves out from under its
objects.

## What we did, and what we do

`(1)` was already right and `(2)` was not, so the pictures followed the flow. The fix is
`PlacedLine.FlyDisplacement`: the paginator records how far the fly moved the block it landed on,
and `ParagraphTop` — the origin a floating frame measures from — takes it back off.

All 21 rows now agree with the reference, page for page:

| fly | shape | ref `MARK` / `AFTER` | ours |
|---:|---|---|---|
| 200 pt | `@A` | p1 60.3 / p1 261.1 | p1 60.7 / p1 259.6 |
| 200 pt | `.@A` | p1 264.7 / p1 274.6 | p1 263.2 / p1 271.2 |
| 710 pt | `...A` | — / p2 70.2 | — / p2 68.7 |
| 710 pt | `.@A` | p1 774.7 / p2 56.7 | p1 773.2 / p2 57.1 |
| 710 pt | `..@A` | p2 60.3 / p2 70.2 | p2 60.7 / p2 68.7 |

The residual is 1.5 pt per empty line, which is the standing line-height divergence and not this.

`HC-Bulletin-template.docx`: first-page ink **39.12 → 10.96**, its text unmoved.

## The wrong rule that also fixed HC-Bulletin, and why it was dropped

The first cut of this said *only a block with ink in it is displaced* — which puts the pictures in
the right place for the right-looking reason, and is refuted by `.A` and `...A` above, where the
reference moves paragraphs that draw nothing at all. It disagreed with the reference on **14 of
the 21 rows**.

It also gained a gate verdict, and the gain was a compensating error worth recording.
`087_Printable_Graph_Paper_Template_Green_Theme` is a fly followed by an empty paragraph and a
`Title: ___ Date: ___` line. Dropping the empty paragraph's line drew that line 20.8 pt above
where the reference draws it — and left it on page one, which the gate scores as a match. With the
line restored we draw it at 746.12 against the reference's 745.52, **0.6 pt out**, and it does not
fit: the two lines need 25.25 pt where the reference's need 23.3, so the paragraph overruns the
body by **1.62 pt** and goes to page two. That is the line-height divergence
(`dotnet/CLAUDE.md`, "Fidelity"), arriving on a document that has 1.62 pt of slack.

So the gate reads 310 rather than 311 with the correct rule, and the document it disagrees on is
one we now place to within 0.6 pt.
