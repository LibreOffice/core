import hashlib, os, sys
SP="/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/charts2"
def h(p):
    with open(p,'rb') as f: return hashlib.md5(f.read()).hexdigest()
before, after = SP+"/render-before", SP+"/render-after"
names = sorted(set(os.listdir(before)) | set(os.listdir(after)))
changed=[]; missing=[]
for n in names:
    if n.startswith('.'): continue
    db, da = os.path.join(before,n), os.path.join(after,n)
    fb = sorted(f for f in os.listdir(db)) if os.path.isdir(db) else []
    fa = sorted(f for f in os.listdir(da)) if os.path.isdir(da) else []
    if fb != fa: missing.append((n, fb, fa)); continue
    for f in fb:
        if not f.endswith('.pdf'): continue
        if h(os.path.join(db,f)) != h(os.path.join(da,f)):
            changed.append(n); break
print("documents:", len([n for n in names if not n.startswith('.')]))
print("changed:", len(changed))
for c in changed: print("  ", c)
if missing:
    print("mismatched output sets:", len(missing))
    for m in missing[:10]: print("  ", m)
