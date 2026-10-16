#!/usr/bin/env python3
"""Per-track census: glyphs the reference shears in a face where we shear none.

    facegap.py <ours-dir> <ref-dir> <label>

Refuses to print unless every reference PDF has an `ours` counterpart.
"""
import glob, os, re, sys, importlib.util, collections
HERE='/c/sandbox/workdir/wt-words-r50/dotnet/probes/words-r56'
_s=importlib.util.spec_from_file_location("sf", os.path.join(HERE,"shear-faces.py"))
sf=importlib.util.module_from_spec(_s); _s.loader.exec_module(sf)

ours_dir, ref_dir, label = sys.argv[1], sys.argv[2], sys.argv[3]
refs = sorted(glob.glob(os.path.join(ref_dir,"*.pdf")))
missing=[os.path.basename(p) for p in refs
         if not os.path.exists(os.path.join(ours_dir, os.path.basename(p)))]
if missing:
    print(f'REFUSING ({label}): {len(missing)} of {len(refs)} reference renderings have no ours')
    for m in missing[:10]: print('   ',m)
    sys.exit(2)
short=collections.Counter(); long=collections.Counter()
docs=collections.defaultdict(collections.Counter)
nd=0
for p in refs:
    ident=os.path.basename(p)[:-4]
    try:
        ol,of = sf.census(os.path.join(ours_dir, ident+'.pdf'))
        rl,rf = sf.census(p)
    except Exception as e:
        print(f'  !! {ident}: {e}'); continue
    nd+=1
    names={sf.strip(k) for k in list(ol)+list(rl)}
    for n in names:
        a=sum(v for k,v in ol.items() if sf.strip(k)==n)
        b=sum(v for k,v in rl.items() if sf.strip(k)==n)
        if b>a: short[n]+=b-a; docs[n][ident]+=b-a
        elif a>b: long[n]+=a-b
print(f'=== {label}: {nd} documents compared')
print('  SHORT by face (reference leans, we do not):')
for f,n in short.most_common(): print('    %-28s %6d' % (f,n))
print('  LONG by face:')
for f,n in long.most_common(): print('    %-28s %6d' % (f,n))
print('  documents with any short, top 25:')
tot=collections.Counter()
for f,c in docs.items():
    for d,n in c.items(): tot[d]+=n
print('  (%d documents short in total)' % len(tot))
for d,n in tot.most_common(25): print('    %6d  %s' % (n,d))
