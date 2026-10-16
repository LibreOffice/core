#!/usr/bin/env python3
"""Where the reference leans a face no document names, and whether we could ever follow it.

    fallbackfaces.py <ours-dir> <ref-dir> <label>

A "pure-fallback face" is one that can only have arrived through `FontItemiser` on this machine,
because no corpus document names the family: WenQuanYi Zen Hei, OpenSymbol, IPA Gothic.  DejaVu
Sans and DejaVu Serif are deliberately **not** on the list -- they are also the substitution
answer for an unrecognised family, and a PDF cannot tell the two routes apart.

**The split into REACHABLE and UNREACHABLE is the whole point, and the first version of this
probe did not have it.**  Round 58 predicted that a fix carrying the italic request across glyph
fallback would move ~345 sheared glyphs on the slides track, almost all of them in one deck, and
it moved **nought**.  The reason is visible only per face: the reference draws 355 WenQuanYi Zen
Hei glyphs in `outlook_of_nigerian_pension_sector.ppt` and **we draw none at all** -- we use
DejaVu Sans Bold there.  That is a face-*selection* divergence, and no amount of getting the
slant right can reach it.  The first cut summed the fallback faces together, so a document where
we drew 80 glyphs of OpenSymbol and nought of WenQuanYi Zen Hei looked like a document we had
merely failed to shear.

  REACHABLE   -- the reference leans N glyphs of a face we also draw, upright.  A slant fix can
                 move these.
  UNREACHABLE -- the reference leans N glyphs of a face we do not draw at all.  Only a
                 fallback-*order* fix can move these.
"""
import collections
import glob
import importlib.util
import os
import sys

HERE = '/c/sandbox/workdir/wt-words-r50/dotnet/probes/words-r56'
_s = importlib.util.spec_from_file_location("sf", os.path.join(HERE, "shear-faces.py"))
sf = importlib.util.module_from_spec(_s)
_s.loader.exec_module(sf)

FB = {'WenQuanYiZenHei', 'OpenSymbol', 'IPAGothic', 'IPAPGothic', 'TakaoPGothic'}

ours_dir, ref_dir, label = sys.argv[1], sys.argv[2], sys.argv[3]
refs = sorted(glob.glob(os.path.join(ref_dir, '*.pdf')))
missing = [p for p in refs if not os.path.exists(os.path.join(ours_dir, os.path.basename(p)))]
if missing:
    print('REFUSING (%s): %d of %d reference renderings have no ours'
          % (label, len(missing), len(refs)))
    sys.exit(2)

ourl = ourf = refl = reff = 0
reachable = collections.Counter()
unreachable = collections.Counter()
docs = 0
for p in refs:
    ident = os.path.basename(p)[:-4]
    ol, of = sf.census(os.path.join(ours_dir, ident + '.pdf'))
    rl, rf = sf.census(p)
    drew = False
    for face in FB:
        a = sum(v for k, v in ol.items() if sf.strip(k) == face)
        b = sum(v for k, v in of.items() if sf.strip(k) == face)
        c = sum(v for k, v in rl.items() if sf.strip(k) == face)
        d = sum(v for k, v in rf.items() if sf.strip(k) == face)
        if a or b or c or d:
            drew = True
        ourl += a
        ourf += b
        refl += c
        reff += d
        if c > a:
            (reachable if (a + b) else unreachable)[(ident, face)] = c - a
    docs += 1 if drew else 0

print('%s: %d documents draw a pure-fallback face on one side or the other' % (label, docs))
print('  ours      lean %6d   flat %6d' % (ourl, ourf))
print('  reference lean %6d   flat %6d' % (refl, reff))
print('  REACHABLE   (we draw the same face, upright): %6d glyphs in %d document/face pairs'
      % (sum(reachable.values()), len(reachable)))
for (d, f), n in reachable.most_common(20):
    print('     %6d  %-18s %s' % (n, f, d))
print('  UNREACHABLE (we do not draw that face at all): %6d glyphs in %d document/face pairs'
      % (sum(unreachable.values()), len(unreachable)))
for (d, f), n in unreachable.most_common(20):
    print('     %6d  %-18s %s' % (n, f, d))
