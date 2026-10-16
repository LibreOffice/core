#!/usr/bin/env python3
"""Why does our token count differ from poppler's on one document?

Compares the two token multisets and attributes the difference to a cause:
  * split      — a poppler token equals several of ours concatenated (we split it)
  * joined     — one of our tokens equals several of poppler's concatenated (we joined)
  * ours-only / poppler-only — text one extractor found and the other did not
"""
import subprocess, sys, collections

CLI = "/c/sandbox/workdir/wt-tooling/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"

def ours(p):
    return subprocess.run([CLI, "analyze", "--text", p], capture_output=True).stdout.decode("utf-8", "replace")

def pop(p):
    return subprocess.run(["pdftotext", p, "-"], capture_output=True).stdout.decode("utf-8", "replace")

def report(path, show=14):
    a, b = ours(path).split(), pop(path).split()
    print(f"\n=== {path.split('/')[-1]}  ours={len(a)} poppler={len(b)} delta={len(a)-len(b):+}")
    ca, cb = collections.Counter(a), collections.Counter(b)
    only_a, only_b = ca - cb, cb - ca
    print(f"    tokens in common (multiset): {sum((ca & cb).values())}")
    print(f"    ours-only {sum(only_a.values())} distinct {len(only_a)} | poppler-only {sum(only_b.values())} distinct {len(only_b)}")

    # A poppler token that is the concatenation of a run of our tokens => we split it.
    # Detect by scanning both streams in parallel and greedily matching.
    i = j = 0
    splits, joins, misa, misb = collections.Counter(), collections.Counter(), collections.Counter(), collections.Counter()
    while i < len(a) and j < len(b):
        if a[i] == b[j]:
            i += 1; j += 1; continue
        # try: b[j] == a[i] + a[i+1] + ...
        acc = ""; k = i
        while k < len(a) and len(acc) < len(b[j]):
            acc += a[k]; k += 1
            if acc == b[j]: break
        if acc == b[j] and k > i + 1:
            splits[b[j]] += 1; i = k; j += 1; continue
        acc = ""; k = j
        while k < len(b) and len(acc) < len(a[i]):
            acc += b[k]; k += 1
            if acc == a[i]: break
        if acc == a[i] and k > j + 1:
            joins[a[i]] += 1; j = k; i += 1; continue
        # resync: look ahead for the next common token
        found = False
        for w in range(1, 60):
            if i + w < len(a) and a[i + w] == b[j]:
                for x in range(i, i + w): misa[a[x]] += 1
                i += w; found = True; break
            if j + w < len(b) and a[i] == b[j + w]:
                for x in range(j, j + w): misb[b[x]] += 1
                j += w; found = True; break
        if not found:
            misa[a[i]] += 1; misb[b[j]] += 1; i += 1; j += 1
    for x in range(i, len(a)): misa[a[x]] += 1
    for x in range(j, len(b)): misb[b[x]] += 1

    print(f"    we SPLIT   {sum(splits.values())} poppler tokens -> extra {sum(splits.values())}+ ours")
    print(f"      {splits.most_common(show)}")
    print(f"    we JOINED  {sum(joins.values())} of our tokens covering more of poppler's")
    print(f"      {joins.most_common(show)}")
    print(f"    ours-only  {sum(misa.values())}: {misa.most_common(show)}")
    print(f"    poppler-only {sum(misb.values())}: {misb.most_common(show)}")

for p in sys.argv[1:]:
    report(p)
