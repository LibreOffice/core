# Round 157 — O107's second candidate refuted: a merge across a page boundary

## 0. What this round is

Round 156 closed O105 and left **O107**: one `.doc` of its four movers,
`P200904290238_0238_51880.doc`, whose pages 2 and 3 shift by about 18 pt while page 1 improves. Its
first seat — *a deep merge whose master states a bottom rule* — was **withdrawn** in that round when
the rendering it rested on turned out to have been made with the base binary.

This round tests the next candidate the document suggests: **a merge that crosses a page boundary**,
which none of round 154's or 156's fixtures hold. It is refuted.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, `b98b02b94` |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 `0229ac93…`** |
| date | 2026-09-19 |

---

## 1. [bin] Where the corpus document diverges

The three renderings' grid lines, page by page (`probes/ww8covered-r156/`'s own sweep output):

| page | 26.2.4.2 | base | head |
|---|---|---|---|
| 1 | 86.3, 184.7, 208.9, **247.4**, 285.9, … | 86.3, 184.7, 208.9, **246.9**, 284.9, … | 86.3, 184.7, 208.9, **247.4**, 285.9, … |
| 2 | 72.0, 96.2, **120.3**, 158.8, … | 72.0, 96.2, **120.3**, 158.3, … | 72.0, 96.2, **112.8**, 136.9, … |
| 3 | 7 lines | 7 lines | **8 lines** |

**Page 1 is a clean win** — the head reproduces 26.2.4.2's boundaries exactly where the base was
0.5 to 2.5 pt out. Page 2's third boundary is then **7.5 pt high** in the head, and page 3 gains a
line. So the head is *short* from page 2 on, and the first rows of page 2 are the continuation of a
table the page boundary cut.

## 2. [bin] A merge that straddles the break: 24 of 24 cells exact

`make-split.py` is six arms, each a spacer of N empty lines followed by a two-column six-row table
whose left column is merged over all six rows — three merged arms and three unmerged controls, with
N chosen so that the break falls at a different row of the merge in each. Every cell of the right
column is labelled, so which row landed on which page is read rather than inferred.

The break lands inside the merge in all three merged arms — `M5` on page 2, `N4`/`N5` on page 4,
`P4`/`P5` on page 6 — and **every one of the 24 cells of the five scored arms agrees with 26.2.4.2
on both its page and its baseline**, to a tenth of a point. `data/split.txt`.

So a merge crossing a page boundary is **not** the mechanism, and O107 keeps its two refutations:
not depth, not the master's bottom rule, and not the page break.

## 3. What is left, and the one thing this fixture did show

The sixth arm, `S` — an **unmerged** control with the longest spacer — is the only disagreement:
26.2.4.2 draws 11 pages and this tree 12, fitting the table where we push it over. That is a
capacity difference on a column of 66 empty lines with no merge in it, so it is not O107, and this
round did not attribute it to any change.

**The next instrument for O107 is the document itself rather than another synthetic.** Its page 2
begins with the tail of a cut table: pair that table's rows between the three renderings and find
which row grows, then read that row's cells out of the `.doc`.
