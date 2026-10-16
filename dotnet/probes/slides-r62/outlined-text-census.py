#!/usr/bin/env python3
"""Pages where the reference draws text as outlines and we draw it as text.

`Demick_JetBlue.pptx` fails the word gate at 812 against 608, and the whole 204-word surplus is
on its three chart pages carrying 45-degree category labels: 63, 78 and 63 words, which are
exactly three times the 21, 26 and 21 labels those pages hold.  26.2.4.2 emits those labels as
*filled outlines* -- 126 glyph-sized black paths on page 4, which is 21 labels of six characters
-- so they carry no extractable text at all, while we emit them as real text runs.

Both stacks draw the same glyphs; only the reference's PDF fails to carry them as characters.
So the gate's word count is measuring the reference's export, not our layout, and our output is
the better one.  This finds every page with that signature.
"""
import os, subprocess, sys
sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r62-slides')
from fills import fills
from pg import page_stream, npages


def words(path, page):
    out = subprocess.run(['pdftotext', '-f', str(page), '-l', str(page), path, '-'],
                         capture_output=True).stdout.decode('utf-8', 'replace')
    return sum(1 for t in out.split() if any(c.isalnum() for c in t))


def glyphpaths(path, page):
    try:
        rows = fills(page_stream(path, page - 1))
    except Exception:
        return -1
    return sum(1 for c, x0, y0, x1, y1, n in rows
               if 0 < (x1 - x0) <= 30 and 0 < (y1 - y0) <= 30 and n >= 4)


if __name__ == '__main__':
    ours, ref = sys.argv[1], sys.argv[2]
    names = sys.argv[3:]
    print("doc\tpage\tourWords\trefWords\tdWords\toursPaths\trefPaths")
    for name in names:
        o, r = f"{ours}/{name}.pdf", f"{ref}/{name}.pdf"
        if not (os.path.exists(o) and os.path.exists(r)):
            continue
        try:
            n = min(npages(o), npages(r))
        except Exception:
            continue
        for p in range(1, n + 1):
            ow, rw = words(o, p), words(r, p)
            if ow - rw <= 3:
                continue
            print(f"{name}\t{p}\t{ow}\t{rw}\t{ow - rw}\t{glyphpaths(o, p)}\t{glyphpaths(r, p)}")
