# Round 112 — a blind reading of one page, and what it found

**Why this exists.** Rounds 111 and 112 both reported that they could not delegate a blind page
reading: a subagent has no subagent tool in this container, so a round cannot spawn the fresh
reviewer `.claude/skills/page-vision/SKILL.md` asks for, and every visual conclusion in those
rounds is the round's own. Only the parent session can spawn one. Round 112 named the page worth
sending — `Thailand17` page 11, for the row-height residual now seated as O54 — so it was sent.

## Method

```sh
XRay.Office.Cli render Thailand17.ppt --format pdf --outdir .        # ours
soffice --headless --convert-to pdf --outdir .  Thailand17.ppt     # 26.2.4.2
pdftoppm -r 128 -f 11 -l 11 -png ours.pdf ours
pdftoppm -r 128 -f 11 -l 11 -png ref.pdf  ref
.claude/skills/page-vision/scripts/compose.py ours-11.png ref-11.png -o pair11.png
```

128 dpi rather than 150 deliberately: at 150 the composite reported `shown at 86% of composed`,
which is the skill's receipt for pixels paid for and not received. At 128 it is `100%`.

The reviewer was a fresh agent given the image path and nothing else — forbidden from reading any
documentation, source or results, forbidden from Grep/Glob/Bash, and **not told what was supposed
to be wrong**. It was asked to describe each half separately before comparing, to give direction
rather than "differs", to say what looked identical, and to name candidate causes rather than
diagnose.

## What it found — and it is not what it was sent for

**It did not see the row-height difference.** It measured the table bottom at about 775 px against
763 px, called that "under 2 % of the table's height, spread over nine rows", and said explicitly
it would "call this indistinguishable rather than a difference". That corroborates O54's own
figure — 0.33 pt per row over nine rows, about 3 pt on a ~560 pt table — and settles that **O54 is
a real but sub-visual seat**. Worth knowing before anyone spends a round on it.

**It found an unreported defect instead.** Our table's strokes are two to three times heavier than
the reference's. Measured out of the two PDFs afterwards, which is the discipline — the reading
says where to point the instrument, it is not the instrument:

| | interior rules | outer frame |
|---|---|---|
| ours | **1.0 pt** × 13 items | **2.25 pt** × 4 items |
| 26.2.4.2 | **0.40 pt** × 13 | **0.95 pt** × 4 |
| ratio | 2.50 | 2.37 |

`strokes-p11.tsv`. Same item counts on both sides and **both black** (0,0,0), so two of the
reviewer's four candidates are ruled out immediately: it is not colour, and it is not a
rasterisation minimum, because these are PDF points and not device pixels. The two remaining
candidates it named — a width conversion, or a default substituted where the reference resolves a
style — are separated by whether the frame and the interior differ by the *same* factor. They
differ by 2.50 and 2.37, which is close but not equal, so neither is clean; that is the next
measurement and it is seated as **O55**.

## The point about method

`Thailand17` page 11 had been worked by three rounds and had its character-size half closed
exactly the same day (548 at 11.99 against the reference's 548). A reviewer who had never seen
the document, given one image and no context, found a defect in the first thing it described. The
rounds were measuring characters and row heights; nobody was measuring stroke width, because
nobody had a reason to.

That is the case the skill argues for delegation, now with a worked example. It also gives the
practical rule this container needs: **a round cannot run this control, so it should name the page
and the parent should spawn the reader.**
