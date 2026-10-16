import os, re, subprocess, sys, collections
from concurrent.futures import ThreadPoolExecutor
SW = sys.argv[1]
def sh(c, t=240):
    try: return subprocess.run(c, capture_output=True, text=True, timeout=t).stdout
    except Exception: return ""
def rasters(pdf):
    by = collections.defaultdict(collections.Counter)
    for line in sh(["pdfimages", "-list", pdf]).splitlines()[2:]:
        f = line.split()
        if len(f) < 5 or not f[0].isdigit(): continue
        by[int(f[0])][(f[3], f[4])] += 1
    return by
def words(pdf, n):
    return [len(sh(["pdftotext", "-f", str(p), "-l", str(p), pdf, "-"]).split()) for p in range(1, n + 1)]
def pages(pdf):
    m = re.search(r"^Pages:\s+(\d+)", sh(["pdfinfo", pdf]), re.M); return int(m.group(1)) if m else 0
docs = [n[:-4] for n in sorted(os.listdir(f"{SW}/cmp"))]
def examine(doc):
    o, r = f"{SW}/ours/{doc}.pdf", f"{SW}/ref/{doc}.pdf"
    if not (os.path.exists(o) and os.path.exists(r)): return []
    n = pages(r); ro, rr = rasters(o), rasters(r); wo, wr = words(o, n), words(r, n)
    out = []
    for p in range(1, n + 1):
        um = bool(rr[p] - ro[p]); extra = wo[p-1] - wr[p-1]
        out.append((doc, p, int(um and extra >= 8 and extra >= wr[p-1] * 0.25), int(um and extra >= 8)))
    return out
print("doc\tpage\tstrict\tloose")
with ThreadPoolExecutor(max_workers=4) as pool:
    for rs in pool.map(examine, docs):
        for d, p, s, l in rs:
            if s or l: print(f"{d}\t{p}\t{s}\t{l}")
