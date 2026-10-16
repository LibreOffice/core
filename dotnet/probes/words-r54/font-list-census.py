#!/usr/bin/env python3
"""Which renderings disagree with the reference's *embedded font list*, and how.

    python3 font-list-census.py <sweep-outdir> [label]

`<sweep-outdir>` is a `batch-check.sh` output directory: it holds `ours/` and `ref/` with one
PDF per per-format identity (`report__docx.pdf`), so both halves were produced in the same run
against the same font set. `pdffonts` on each pair gives the two sets of base font names with
the six-letter subset tag stripped.

This re-derives round 53's "**86 of 337 words renderings disagree and 73 carry DejaVu Sans on
our side**" rather than quoting it — the project's standing rule after round 53 found round 52's
"469 paragraphs in 66 documents" to be 22 in 13.

What it cannot see, stated rather than left implicit:

  * a face that is *right* by name and wrong by which file — `pdffonts` reports the base font
    name, and two faces of one family share it;
  * a wrong face that happens to embed nothing (a page with no text embeds no font, so an
    empty document agrees trivially);
  * whether a disagreement costs anything on the page. Different advances make it a line-break
    difference in principle; the gate and the renderings say whether it is one in practice.
"""
import os
import re
import subprocess
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor


def faces(pdf):
    if not os.path.exists(pdf):
        return None
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    out = set()
    for line in text.splitlines()[2:]:
        if line.strip():
            out.add(re.sub(r'^[A-Z]{6}\+', '', line.split()[0]))
    return out


def main():
    root = os.path.abspath(sys.argv[1])
    label = sys.argv[2] if len(sys.argv) > 2 else os.path.basename(root)
    names = sorted(os.listdir(os.path.join(root, 'ours')))

    with ThreadPoolExecutor(max_workers=16) as pool:
        ours = list(pool.map(lambda n: faces(os.path.join(root, 'ours', n)), names))
        refs = list(pool.map(lambda n: faces(os.path.join(root, 'ref', n)), names))

    rows = []
    for name, mine, theirs in zip(names, ours, refs):
        if mine is None or theirs is None:
            continue
        if mine != theirs:
            rows.append((name[:-4], sorted(mine - theirs), sorted(theirs - mine)))

    dejavu_sans_ours = [r for r in rows if any(f.startswith('DejaVuSans') and 'Mono' not in f
                                              for f in r[1])]
    plain_pair = [r for r in rows
                  if [f for f in r[1] if f.startswith('DejaVuSans') and 'Mono' not in f]
                  and [f for f in r[2] if f.startswith('DejaVuSerif')]]

    print(f'{label}: {len(names)} renderings, {len(rows)} disagree with the reference font list')
    print(f'  {len(dejavu_sans_ours)} of those carry DejaVu Sans on our side')
    print(f'  {len(plain_pair)} are the plain pair ours=DejaVuSans, ref=DejaVuSerif')
    print()
    shapes = Counter((tuple(r[1]), tuple(r[2])) for r in rows)
    print('the ten commonest disagreement shapes (ours-only | ref-only | count):')
    for (mine, theirs), count in shapes.most_common(10):
        print(f'  {", ".join(mine) or "-":44s} | {", ".join(theirs) or "-":44s} | {count}')
    print()
    with open(os.path.join(root, 'font-census.tsv'), 'w') as handle:
        for name, mine, theirs in rows:
            handle.write(f'{name}\t{",".join(mine)}\t{",".join(theirs)}\n')
    print(f'wrote {os.path.join(root, "font-census.tsv")}')


if __name__ == '__main__':
    main()
