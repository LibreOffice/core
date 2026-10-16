#!/usr/bin/env python3
"""Read each probe PDF and report what the frame's height cost the body below it.

The frame is `style:wrap="none"` (Writer's *top and bottom*), so the paragraph that anchors it
is pushed below it and `Body paragraph 1` follows that.  The frame's height is therefore read
as a *difference* between probes rather than absolutely: `a-2para` is the control and every
other probe differs from it by exactly one thing.

The frame's own border rectangle is deliberately not read.  A Writer fly's border is emitted as
a stroked polygon whose shape depends on which edges are drawn, and a probe stating
`fo:border="none"` emits none at all — so the border is not a channel that works across the
whole set, while the body's own baseline is.
"""
import os, re, subprocess, sys

D = sys.argv[1] if len(sys.argv) > 1 else "/tmp/odtframe-probes/pdf"
CONTROL = "a-2para"


def words(pdf):
    out = subprocess.run(["pdftotext", "-bbox", pdf, "-"],
                         capture_output=True, text=True).stdout
    res, page = [], 0
    for m in re.finditer(
            r'<page |<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>',
            out):
        if m.group(1) is None:
            page += 1
            continue
        res.append((page, float(m.group(2)), m.group(5)))
    return res


rows = {}
for name in sorted(os.listdir(D)):
    if not name.endswith(".pdf"):
        continue
    ws = words(os.path.join(D, name))
    first = next((w for w in ws if w[2] == "Line"), None)
    body = next((w for w in ws if w[2] == "Body"), None)
    rows[name[:-4]] = (max((w[0] for w in ws), default=0),
                       first[1] if first else None,
                       body[1] if body else None,
                       body[0] if body else None)

base = rows[CONTROL][2]
print(f"{'probe':24s} {'pages':>5s} {'frame line 1':>12s} {'body 1':>9s} {'pg':>3s} "
      f"{'vs ' + CONTROL:>12s}")
for name, (pages, first, body, bpage) in rows.items():
    d = "" if body is None else f"{body - base:+.3f}"
    print(f"{name:24s} {pages:5d} "
          f"{'-' if first is None else format(first, '.3f'):>12s} "
          f"{'-' if body is None else format(body, '.3f'):>9s} "
          f"{'-' if bpage is None else bpage:>3} {d:>12s}")
