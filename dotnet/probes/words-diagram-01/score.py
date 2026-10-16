import sys, csv, os
banked="/home/user/gate-2f47/parity.tsv"
now=sys.argv[1]
def load(path):
    rows={}
    with open(path, newline="", encoding="utf-8") as f:
        for line in f:
            if line.startswith("#"): continue
            p=line.rstrip("\n").split("\t")
            if not p or p[0]=="path": continue
            rows[p[0]]=p
    return rows
b=load(banked); n=load(now)
words_b={k:v for k,v in b.items() if k.startswith("words/")}
missing=[k for k in words_b if k not in n]
extra=[k for k in n if k not in words_b]
print("banked words rows:", len(words_b), " swept rows:", len(n))
if missing: print("MISSING from sweep (%d):"%len(missing)); [print("  ", m) for m in missing[:20]]
if extra: print("EXTRA in sweep (%d):"%len(extra)); [print("  ", m) for m in extra[:20]]
if missing:
    print("REFUSING to score: every banked words path must have a row.")
moved=[]
for k,v in sorted(words_b.items()):
    if k not in n: continue
    if v[6]!=n[k][6]: moved.append((k, v[6], n[k][6], v[2], n[k][2], v[3], n[k][3], v[7] if len(v)>7 else "", n[k][7] if len(n[k])>7 else ""))
import collections
print("banked verdicts:", dict(collections.Counter(v[6] for v in words_b.values())))
print("swept  verdicts:", dict(collections.Counter(n[k][6] for k in words_b if k in n)))
print("verdicts moved:", len(moved))
for m in moved:
    print("  %s\n     %s -> %s   pages %s -> %s   words %s -> %s"%(m[0],m[1],m[2],m[3],m[4],m[5],m[6]))
# also report rows whose numbers moved without the verdict moving
num=[]
for k,v in sorted(words_b.items()):
    if k not in n: continue
    if v[6]==n[k][6] and (v[2]!=n[k][2] or v[3]!=n[k][3]):
        num.append((k, v[2],n[k][2], v[3],n[k][3]))
print("same verdict, numbers moved:", len(num))
for x in num: print("   %s  pages %s->%s  words %s->%s"%x)
