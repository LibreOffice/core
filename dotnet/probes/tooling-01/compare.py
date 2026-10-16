#!/usr/bin/env python3
"""Compare `xray-office analyze` against poppler over the 534 canonical reference PDFs."""
import sys, os, collections

SP = "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/cmp"

# ours: id pages words wordsRaw wordsAlnum bullets symbols punct fonts unemb subset sizes w h rot err ms
O = {}
for line in open(f"{SP}/ours.tsv", encoding="utf-8"):
    f = line.rstrip("\n").split("\t")
    O[f[0]] = dict(pages=f[1], words=f[2], raw=f[3], alnum=f[4], bullets=f[5], sym=f[6],
                   punct=f[7], fonts=f[8], unemb=f[9], subset=f[10], sizes=f[11],
                   w=f[12], h=f[13], rot=f[14], err=f[15], ms=int(f[16]))

# poppler: id pages words fonts unemb subset ms
P = {}
for line in open(f"{SP}/poppler.tsv", encoding="utf-8"):
    f = line.rstrip("\n").split("\t")
    P[f[0]] = dict(pages=f[1], words=int(f[2]), fonts=int(f[3]), unemb=int(f[4]),
                   subset=int(f[5]), ms=int(f[6]))

ids = sorted(set(O) & set(P))
print(f"ids ours={len(O)} poppler={len(P)} joined={len(ids)}")
assert len(ids) == 534, "join lost documents"

# ---- the banked reference baseline, as a third independent column
BASE = {}
bl = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/ref-baseline-all.tsv"
for line in open(bl, encoding="utf-8"):
    if line.startswith("#") or line.startswith("track\t"):
        continue
    f = line.rstrip("\n").split("\t")
    stem = os.path.basename(f[1]); ext = f[2].lower()
    key = stem[: stem.rfind(".")] + "__" + ext
    BASE[key] = dict(pages=f[3], words=int(f[4]), fonts=int(f[5]), unemb=int(f[6]))

print(f"baseline rows={len(BASE)}  joined with ours={len(set(BASE) & set(ids))}")

errs = [i for i in ids if O[i]["err"]]
print(f"\nour read errors: {len(errs)}")
for i in errs: print("   ", i, O[i]["err"][:120])

# ---------------------------------------------------------------- 1. page count
pg_ours_vs_pop = [i for i in ids if O[i]["pages"] != P[i]["pages"]]
print(f"\n== PAGES ==\nours vs poppler: agree {len(ids)-len(pg_ours_vs_pop)}/{len(ids)}")
for i in pg_ours_vs_pop[:20]: print("   ", i, "ours", O[i]["pages"], "poppler", P[i]["pages"])

common = sorted(set(BASE) & set(ids))
pg_base = [i for i in common if O[i]["pages"] != BASE[i]["pages"]]
print(f"ours vs banked ref-baseline-all.tsv: agree {len(common)-len(pg_base)}/{len(common)}")
for i in pg_base[:20]: print("   ", i, "ours", O[i]["pages"], "banked", BASE[i]["pages"])

pop_base = [i for i in common if P[i]["pages"] != BASE[i]["pages"]]
print(f"poppler-now vs banked (control on the banked file): agree {len(common)-len(pop_base)}/{len(common)}")

# ---------------------------------------------------------------- 2. fonts
fd = [i for i in ids if int(O[i]["fonts"]) != P[i]["fonts"]]
ud = [i for i in ids if int(O[i]["unemb"]) != P[i]["unemb"]]
sd = [i for i in ids if int(O[i]["subset"]) != P[i]["subset"]]
print(f"\n== FONTS ==\nface count  agree {len(ids)-len(fd)}/{len(ids)}")
for i in fd: print("   ", i, "ours", O[i]["fonts"], "poppler", P[i]["fonts"])
print(f"unembedded  agree {len(ids)-len(ud)}/{len(ids)}")
for i in ud: print("   ", i, "ours", O[i]["unemb"], "poppler", P[i]["unemb"])
print(f"subset      agree {len(ids)-len(sd)}/{len(ids)}")
for i in sd[:20]: print("   ", i, "ours", O[i]["subset"], "poppler", P[i]["subset"])

# ---------------------------------------------------------------- 3. words, raw vs raw
print("\n== WORDS: our raw tokenisation vs poppler's raw tokenisation ==")
exact = 0; deltas = []
for i in ids:
    d = int(O[i]["raw"]) - P[i]["words"]
    deltas.append((d, i))
    if d == 0: exact += 1
absd = [abs(d) for d, _ in deltas]
tot_o = sum(int(O[i]["raw"]) for i in ids); tot_p = sum(P[i]["words"] for i in ids)
print(f"exact agreement: {exact}/{len(ids)}")
print(f"total tokens ours={tot_o} poppler={tot_p} net={tot_o-tot_p} sum|delta|={sum(absd)}")
within = lambda pct, add: sum(1 for d, i in deltas if abs(d) <= max(add, P[i]['words'] * pct))
print(f"within 2%+3 (the gate's band): {within(0.02,3)}/{len(ids)}")
print(f"within 1%:   {within(0.01,0)}/{len(ids)}")
print(f"within 5%:   {within(0.05,0)}/{len(ids)}")
deltas.sort()
print("largest negative (we find fewer):")
for d, i in deltas[:12]:
    print(f"   {d:+7d}  ours {O[i]['raw']:>7} poppler {P[i]['words']:>7}  {i}")
print("largest positive (we find more):")
for d, i in deltas[-12:]:
    print(f"   {d:+7d}  ours {O[i]['raw']:>7} poppler {P[i]['words']:>7}  {i}")

by_track = collections.defaultdict(list)
TRACK = {}
for line in open(f"{SP}/pdfs.txt"):
    line = line.strip()
    TRACK[os.path.basename(line)[:-4]] = line.split("/")[-2]
for d, i in deltas:
    by_track[TRACK.get(i, "?")].append(d)
print("\nby track (raw vs raw):")
for t, ds in sorted(by_track.items()):
    print(f"   {t:8} n={len(ds):4} exact={sum(1 for d in ds if d==0):4} "
          f"net={sum(ds):+8} sum|d|={sum(abs(d) for d in ds):8}")

# ---------------------------------------------------------------- 4. the excluded classes
print("\n== EXCLUDED CLASSES (ours) ==")
for k in ("bullets", "sym", "punct"):
    tot = sum(int(O[i][k]) for i in ids)
    nz = sum(1 for i in ids if int(O[i][k]) > 0)
    print(f"   {k:8} total={tot:8}  documents with any={nz}")
print(f"   alnum total={sum(int(O[i]['alnum']) for i in ids)}  raw total={tot_o}")
bad = [i for i in ids if int(O[i]['raw']) != int(O[i]['alnum']) + int(O[i]['bullets'])
       + int(O[i]['sym']) + int(O[i]['punct'])]
print(f"   partition holds on {len(ids)-len(bad)}/{len(ids)}")

# ---------------------------------------------------------------- 5. timing
print("\n== TIMING (serial, per document, ms) ==")
for name, src in (("ours", [O[i]['ms'] for i in ids]), ("poppler(5 procs)", [P[i]['ms'] for i in ids])):
    s = sorted(src)
    print(f"   {name:18} total={sum(s)/1000:8.1f}s  median={s[len(s)//2]:5}  p95={s[int(len(s)*.95)]:6}  max={s[-1]:6}")

# ---------------------------------------------------------------- 6. page sizes
multi = [i for i in ids if int(O[i]["sizes"]) > 1]
print(f"\n== PAGE SIZE ==\ndocuments with more than one visible page size: {len(multi)}")
rot = [i for i in ids if O[i]["rot"] != "0"]
print(f"documents whose first page carries /Rotate: {len(rot)}")
