#!/usr/bin/env python3
"""Which faces each side chooses, per document — the fallback-*order* census.

    facechoice.py <ours-dir> <ref-dir> <label> [--min N]

Round 58's slant census read a face-*selection* divergence as a lean defect because it summed
several fallback faces per document. This asks the other question and asks it per face: for each
document, which `/BaseFont`s does the reference draw that we draw **none** of, and which do we
draw that it draws none of? A pair of those in one document is one substitution decision made two
ways, and it is the shape no slant or metric fix can reach.

Prints three sections and never nets them against each other:

  * UNIQUE TO THE REFERENCE — glyphs in a face we never open in that document.
  * UNIQUE TO US — the same, the other way round. Usually the *other half* of the same
    substitution, and the two lists should be read together per document.
  * PAIRED — documents appearing in both, with the two faces named side by side. That is the
    list a fallback-order fix is aimed at.

Refuses to print unless every reference rendering has an `ours` counterpart.
"""
import collections
import glob
import importlib.util
import os
import sys

_spec = importlib.util.spec_from_file_location(
    "sf", "/c/sandbox/workdir/wt-words-r50/dotnet/probes/words-r56/shear-faces.py")
sf = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(sf)


def faces(pdf):
    lean, flat = sf.census(pdf)
    out = collections.Counter()
    for source in (lean, flat):
        for key, value in source.items():
            out[sf.strip(key)] += value
    return out


if __name__ == '__main__':
    ours_dir, ref_dir, label = sys.argv[1], sys.argv[2], sys.argv[3]
    floor = int(sys.argv[sys.argv.index('--min') + 1]) if '--min' in sys.argv else 1

    refs = sorted(glob.glob(os.path.join(ref_dir, '*.pdf')))
    missing = [os.path.basename(p) for p in refs
               if not os.path.exists(os.path.join(ours_dir, os.path.basename(p)))]
    if missing:
        print('REFUSING (%s): %d of %d reference renderings have no ours'
              % (label, len(missing), len(refs)))
        for m in missing[:10]:
            print('   ', m)
        sys.exit(2)

    onlyRef = []
    onlyOurs = []
    for p in refs:
        ident = os.path.basename(p)[:-4]
        try:
            ours = faces(os.path.join(ours_dir, ident + '.pdf'))
            ref = faces(p)
        except Exception as exc:
            print('  !! %s: %s' % (ident, exc))
            continue
        for name, n in ref.items():
            if n >= floor and ours.get(name, 0) == 0:
                onlyRef.append((n, ident, name))
        for name, n in ours.items():
            if n >= floor and ref.get(name, 0) == 0:
                onlyOurs.append((n, ident, name))

    print('=== %s: %d documents compared, floor %d glyphs' % (label, len(refs), floor))
    print('\nUNIQUE TO THE REFERENCE — %d glyphs over %d document/face pairs'
          % (sum(r[0] for r in onlyRef), len(onlyRef)))
    for n, ident, name in sorted(onlyRef, reverse=True)[:30]:
        print('   %7d  %-26s %s' % (n, name, ident[:70]))
    print('\nUNIQUE TO US — %d glyphs over %d document/face pairs'
          % (sum(r[0] for r in onlyOurs), len(onlyOurs)))
    for n, ident, name in sorted(onlyOurs, reverse=True)[:30]:
        print('   %7d  %-26s %s' % (n, name, ident[:70]))

    byDocRef = collections.defaultdict(list)
    byDocOurs = collections.defaultdict(list)
    for n, ident, name in onlyRef:
        byDocRef[ident].append((n, name))
    for n, ident, name in onlyOurs:
        byDocOurs[ident].append((n, name))
    paired = sorted(
        (max(x[0] for x in byDocRef[d]), d) for d in byDocRef if d in byDocOurs)
    print('\nPAIRED — %d documents where each side draws a face the other never opens' % len(paired))
    for size, ident in sorted(paired, reverse=True)[:25]:
        r = ','.join('%s:%d' % (name, n) for n, name in sorted(byDocRef[ident], reverse=True)[:3])
        o = ','.join('%s:%d' % (name, n) for n, name in sorted(byDocOurs[ident], reverse=True)[:3])
        print('   %-58s\n        ref-only  %s\n        our-only  %s' % (ident[:58], r, o))
