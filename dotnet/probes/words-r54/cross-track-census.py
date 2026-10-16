#!/usr/bin/env python3
"""Which slides and sheets documents resolve a generic fallback, and would a serif default move them.

    python3 cross-track-census.py <slides-sweep-outdir> <sheets-sweep-outdir> [outdir]

The parent asked for this by name: *which slides and sheets documents resolve a generic fallback,
by the same method used for words — compare embedded font lists, ours against the reference.*

Same instrument as `font-list-census.py`, run over a `batch-check.sh` output directory whose
`ours/` and `ref/` halves were produced in one run against one font set. A document "resolves a
generic fallback" here iff its rendering embeds a **DejaVu** face on either side: DejaVu is what
this machine has installed for fontconfig's three generics and for nothing else, so a DejaVu face
in the output is a face nothing named was found for.

Three groups are separated, because they carry different risk:

  * **both** — both halves embed DejaVu. The fallback fired on both sides and the question is only
    *which* DejaVu.
  * **ours only** — we fell back where the reference did not. A different defect; a serif default
    would not touch it.
  * **reference only** — the reference fell back where we did not. Likewise.

And within those, the row that a serif default would actually move: **ours draws a DejaVu Sans
face**. That is the population a `GenericFallbacks` change would have reflowed, and naming it is
the point of the census.
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor


def faces(pdf):
    if not os.path.exists(pdf):
        return None
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    return {re.sub(r'^[A-Z]{6}\+', '', line.split()[0])
            for line in text.splitlines()[2:] if line.strip()}


def census(track, root, out):
    names = sorted(os.listdir(os.path.join(root, 'ours')))
    with ThreadPoolExecutor(max_workers=16) as pool:
        ours = list(pool.map(lambda n: faces(os.path.join(root, 'ours', n)), names))
        refs = list(pool.map(lambda n: faces(os.path.join(root, 'ref', n)), names))

    def dejavu(one):
        return one is not None and any(f.startswith('DejaVu') for f in one)

    def sans(one):
        return one is not None and any(f.startswith('DejaVuSans') and 'Mono' not in f for f in one)

    rows = [(n[:-4], o or set(), r or set()) for n, o, r in zip(names, ours, refs)
            if dejavu(o) or dejavu(r)]
    both = [x for x in rows if dejavu(x[1]) and dejavu(x[2])]
    ours_only = [x for x in rows if dejavu(x[1]) and not dejavu(x[2])]
    ref_only = [x for x in rows if not dejavu(x[1]) and dejavu(x[2])]
    at_risk = [x for x in rows if sans(x[1])]
    swap = [x for x in rows
            if any(f.startswith('DejaVuSans') and 'Mono' not in f for f in x[1] - x[2])
            and any(f.startswith('DejaVuSerif') for f in x[2] - x[1])]

    print(f'{track}: {len(names)} renderings')
    print(f'  {len(rows)} resolve a generic fallback on one side or the other'
          f'  (both {len(both)}, ours only {len(ours_only)}, reference only {len(ref_only)})')
    print(f'  {len(at_risk)} draw a DejaVu Sans face on our side — the population a serif '
          f'default would reflow')
    print(f'  {len(swap)} show ours = DejaVu Sans against reference = DejaVu Serif — '
          f'the words defect, on this track')

    path = os.path.join(out, f'{track}-generic-fallback.tsv')
    with open(path, 'w') as handle:
        handle.write('# document\tgroup\tours\treference\n')
        for name, mine, theirs in rows:
            group = ('both' if dejavu(mine) and dejavu(theirs)
                     else 'ours-only' if dejavu(mine) else 'reference-only')
            handle.write(f'{name}\t{group}\t{",".join(sorted(mine))}\t{",".join(sorted(theirs))}\n')
    print(f'  wrote {path}')

    path = os.path.join(out, f'{track}-at-risk.txt')
    with open(path, 'w') as handle:
        handle.write('\n'.join(name for name, _, _ in at_risk) + '\n')
    print(f'  wrote {path}\n')


def main():
    out = os.path.abspath(sys.argv[3]) if len(sys.argv) > 3 else os.getcwd()
    os.makedirs(out, exist_ok=True)
    census('slides', os.path.abspath(sys.argv[1]), out)
    census('sheets', os.path.abspath(sys.argv[2]), out)


if __name__ == '__main__':
    main()
