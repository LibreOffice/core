#!/usr/bin/env python3
"""Which renderings differ between two sweeps, ignoring the one byte range that always does.

    pdfdiff.py <before-dir> <after-dir>

`/CreationDate` and `/ModDate` carry the wall clock even under a pinned SOURCE_DATE_EPOCH in
some producers, and an ID array is a hash of them, so a byte comparison reports every document
as changed. Those three are blanked before hashing; nothing else is.
"""
import sys, os, re, hashlib, collections

BLANK = re.compile(rb'/(CreationDate|ModDate)\s*\((?:[^()\\]|\\.)*\)|/ID\s*\[[^\]]*\]')

def digest(path):
    with open(path, 'rb') as handle:
        return hashlib.sha256(BLANK.sub(b'', handle.read())).hexdigest()

def sweep(root):
    out = {}
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.lower().endswith('.pdf'):
                continue
            key = os.path.relpath(os.path.join(dirpath, name), root)
            out[key] = digest(os.path.join(dirpath, name))
    return out

before, after = sweep(sys.argv[1]), sweep(sys.argv[2])
tracks = collections.Counter()
changed = collections.defaultdict(list)

for key in sorted(set(before) | set(after)):
    track = key.split(os.sep)[0]
    tracks[(track, 'total')] += 1
    if before.get(key) != after.get(key):
        tracks[(track, 'changed')] += 1
        changed[track].append(key)

for track in sorted({t for t, _ in tracks}):
    print(f"{track}: {tracks[(track,'changed')]} changed of {tracks[(track,'total')]}")
    for key in changed[track]:
        print(f"    {key}")
