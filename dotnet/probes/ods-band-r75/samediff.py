#!/usr/bin/env python3
"""Render one corpus tree with two binaries and report which documents' output moved.

    samediff.py <before-cli> <after-cli> <root> <glob-dir-regex> <outdir> [workers]

`SOURCE_DATE_EPOCH` is set for both legs, so a header holding `&D` prints the same date on
both and two runs of one binary are byte-equal with nothing masked. Anything that differs is
the diff under test and nothing else.

This is the check that a gain on one column did not cost a row on another, and it is not the
circular one: both halves are rendered afresh rather than one being seeded from the other.
"""
import concurrent.futures, hashlib, os, re, shutil, subprocess, sys

before, after, root, pattern, outdir = sys.argv[1:6]
workers = int(sys.argv[6]) if len(sys.argv) > 6 else 2
os.makedirs(outdir, exist_ok=True)
env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000")

EXTS = (".xls", ".xlsx", ".xlsm", ".xlsb", ".xlt", ".xltx", ".xltm", ".csv",
        ".ods", ".ots", ".fods", ".sxc")

files = []
for dirpath, _, names in os.walk(root):
    if not re.search(pattern, os.path.relpath(dirpath, root)):
        continue
    for n in names:
        if n.lower().endswith(EXTS):
            files.append(os.path.join(dirpath, n))
# One inode, one render: this mount is case-insensitive and carries alias directory entries.
seen, unique = set(), []
for f in sorted(files):
    key = os.stat(f).st_ino
    if key not in seen:
        seen.add(key)
        unique.append(f)
print(f"{len(unique)} documents", file=sys.stderr)


def one(src):
    d = os.path.join(outdir, hashlib.md5(src.encode()).hexdigest())
    digests = []
    for cli in (before, after):
        leg = os.path.join(d, hashlib.md5(cli.encode()).hexdigest()[:8])
        shutil.rmtree(leg, ignore_errors=True)
        os.makedirs(leg, exist_ok=True)
        try:
            subprocess.run([cli, "render", src, "--format", "pdf", "--outdir", leg],
                           capture_output=True, timeout=600, env=env)
        except subprocess.TimeoutExpired:
            pass
        pdfs = [x for x in os.listdir(leg) if x.lower().endswith(".pdf")]
        digests.append(
            hashlib.md5(open(os.path.join(leg, pdfs[0]), "rb").read()).hexdigest()
            if pdfs else "-")
    shutil.rmtree(d, ignore_errors=True)
    return os.path.relpath(src, root), digests[0], digests[1]


moved, failed = [], []
with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
    for rel, a, b in pool.map(one, unique):
        if a == "-" or b == "-":
            failed.append((rel, a, b))
        elif a != b:
            moved.append(rel)

print(f"documents {len(unique)}  moved {len(moved)}  no-output {len(failed)}")
for rel in moved:
    print("MOVED", rel)
for rel, a, b in failed:
    print("NO-OUTPUT", rel, a, b)
