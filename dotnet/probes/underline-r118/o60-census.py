#!/usr/bin/env python3
"""How many corpus presentations could show O60 at all.

O60 is the width of a *table cell's* inner measure, and the only quantity that separates the
two candidate arithmetics is whether the cell's own left and right borders come off it.  So a
document can only show the seat if it holds a table whose cells state a vertical border AND
whose text is long enough to wrap inside such a cell.  This counts both, over the OOXML half
of the slides track -- `.ppt` is not scanned, which is why the figure is a floor and why the
seat's own witness (`architecture6.ppt`) is not in it.
"""
import os, re, sys, zipfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/slides'
docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs
              if os.path.splitext(f)[1].lower() in ('.pptx', '.pptm', '.potx', '.ppsx'))

tables = borders = justified = both = 0
print('path\ttables\tcells_with_side_border\tjustified_paras_in_tables')
for p in docs:
    t = b = j = 0
    try:
        with zipfile.ZipFile(p) as z:
            for n in z.namelist():
                if not (n.startswith('ppt/slides/slide') and n.endswith('.xml')):
                    continue
                x = z.read(n).decode('utf-8', 'replace')
                for m in re.finditer(r'<a:tbl>.*?</a:tbl>', x, re.S):
                    body = m.group(0)
                    t += 1
                    b += len(re.findall(r'<a:ln[LR]\b', body))
                    j += len(re.findall(r'algn="just(?:Low)?"', body))
    except Exception:                                             # noqa: BLE001
        continue
    if t:
        print('%s\t%d\t%d\t%d' % (os.path.basename(p), t, b, j))
    tables += t
    borders += 1 if b else 0
    justified += 1 if j else 0
    both += 1 if (b and j) else 0
print('# %d presentations scanned, %d tables, %d with a cell side border, '
      '%d with justified table text, %d with both'
      % (len(docs), tables, borders, justified, both), file=sys.stderr)
