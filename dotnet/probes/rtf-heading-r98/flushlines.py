#!/usr/bin/env python3
r"""Which lines each rendering draws flush to its own right margin — a justification census that
does not depend on the two sides paginating alike.

`mover-ink.py` compares page N against page N, so on a document whose pagination already differs
it scores our page 10 against the reference's page 10 and reports noise. `PES-Technical-Report-
Template_Jan_2019` is 15 pages here and 17 at 26.2.4.2, so that instrument cannot say whether this
round's change moved the document towards the reference or away from it.

What can is the line itself. A justified line ends flush at the text area's right edge and a
ragged one does not, so:

  * take every text line of a rendering, with its right edge;
  * take that rendering's own right margin as the **most common** right edge, bucketed to a
    twentieth of a point (each side has its own -- ours is 522.0 pt on this document and
    26.2.4.2's is 525.1). The *largest* right edge is the wrong statistic and the first cut of
    this script used it: a running head, a page number or a table rule reaches past the body's
    measure, and on this document that put the margin at 588.66 and called one line of 340 flush;
  * call a line *flush* when its right edge is within `tol` of that margin;
  * pair a line with the reference's by its text, so pagination drops out entirely.

A line that is flush on both sides or ragged on both sides agrees. The counts alone are not
enough — a document can gain flush lines in the wrong paragraphs — which is why the pairing is
by text and the disagreements are printed.

  flushlines.py <reference.pdf> <ours.pdf> [<ours-other-leg.pdf>]
"""
import collections
import re
import sys

import pymupdf

TOL = 1.0


def lines(path):
    """[(normalised text, right edge)] over every non-empty text line."""
    out = []
    with pymupdf.open(path) as doc:
        for page in doc:
            for block in page.get_text('dict')['blocks']:
                for line in block.get('lines', []):
                    text = ''.join(s['text'] for s in line['spans'])
                    key = re.sub(r'\s+', ' ', text).strip()
                    if key:
                        out.append((key, line['bbox'][2]))
    return out


def flushed(rows, tol=TOL):
    """{text -> is the line flush to this rendering's own right margin}, first occurrence wins."""
    if not rows:
        return {}, 0.0
    counts = collections.Counter(round(right * 20) / 20 for _, right in rows)
    margin = max(counts, key=lambda edge: (counts[edge], edge))
    seen = {}
    for key, right in rows:
        seen.setdefault(key, right >= margin - tol)
    return seen, margin


def report(label, ref, ours):
    refflush, refmargin = flushed(lines(ref))
    ourflush, ourmargin = flushed(lines(ours))
    shared = [k for k in ourflush if k in refflush]
    agree = sum(1 for k in shared if ourflush[k] == refflush[k])
    print(f'{label:6} margin {ourmargin:7.2f} (reference {refmargin:7.2f})  '
          f'lines {len(ourflush):5} paired {len(shared):5}  '
          f'flush {sum(ourflush.values()):4} (reference {sum(refflush[k] for k in shared):4} '
          f'over the paired)  agree {agree}/{len(shared)}')
    return {k: (refflush[k], ourflush[k]) for k in shared}


ref = sys.argv[1]
first = report('ours', ref, sys.argv[2])
if len(sys.argv) > 3:
    second = report('other', ref, sys.argv[3])
    moved = [k for k in first if k in second and first[k][1] != second[k][1]]
    print(f'\n{len(moved)} paired lines change flushness between the two legs; '
          f'{sum(1 for k in moved if second[k][1] == second[k][0])} of them end up agreeing '
          f'with the reference and {sum(1 for k in moved if second[k][1] != second[k][0])} do not')
    for k in moved[:10]:
        print(f'  ref={first[k][0]!s:5} leg1={first[k][1]!s:5} leg2={second[k][1]!s:5}  {k[:70]!r}')
