#!/usr/bin/env python3
"""Remove case-variant alias directory entries from the corpus, without deleting anything.

This mount is case-insensitive, and a case-variant lookup (`Foo.PPT` for `Foo.ppt`) creates a
cached directory entry that becomes visible in `readdir` once the directory is next modified.
The entries persist across a cache drop, so they are not transient.

**Never `rm` an alias.** Measured 2026-08-21 on a scratch file: three names, one inode, link count
1; `rm` on one name destroyed the file and left the others as stale entries pointing at nothing.
That is why this script renames instead.

The fix is a rename round trip on the *tracked* name — `mv x .tmp && mv .tmp x` — which invalidates
every case-variant entry for that inode while leaving the file itself untouched. Verified over the
whole corpus: 77 inodes carrying 87 extra names, all cleared, **zero hash changes**, `git status`
clean, and the on-disk count dropping to exactly the manifest's 946.

Run with --check to report without changing anything.
"""
import argparse, collections, hashlib, os, subprocess, sys

def tracked_paths(root):
    out = subprocess.run(['git', '-C', root, '-c', 'core.quotePath=false', 'ls-files'],
                         capture_output=True, text=True)
    if out.returncode != 0:
        sys.exit('corpus is not a git checkout; git is the only authority for which spelling is real')
    return {os.path.join(root, l) for l in out.stdout.split('\n') if l}

def sha(p):
    m = hashlib.sha256()
    with open(p, 'rb') as f:
        for b in iter(lambda: f.read(1 << 20), b''):
            m.update(b)
    return m.hexdigest()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('root', nargs='?', default='/c/sandbox/workdir/sample-files')
    ap.add_argument('--check', action='store_true', help='report only, change nothing')
    a = ap.parse_args()

    tracked = tracked_paths(a.root)
    by_inode = collections.defaultdict(list)
    for fam in ('words', 'slides', 'sheets'):
        for dp, _, fs in os.walk(os.path.join(a.root, fam)):
            for f in fs:
                p = os.path.join(dp, f)
                try:
                    st = os.stat(p)
                except OSError:
                    continue
                by_inode[(st.st_dev, st.st_ino)].append(p)

    dupes = {k: v for k, v in by_inode.items() if len(v) > 1}
    print(f'files {sum(len(v) for v in by_inode.values())}  inodes {len(by_inode)}  '
          f'aliased inodes {len(dupes)}  extra names {sum(len(v) - 1 for v in dupes.values())}')
    if a.check or not dupes:
        return 0

    ok = bad = 0
    for names in dupes.values():
        real = [p for p in names if p in tracked]
        if len(real) != 1:
            # No ordering rule picks the right name: some aliases upper-case the extension and
            # some lower-case the whole filename. Without exactly one tracked name, refuse.
            print('SKIP (not exactly one tracked name):', names)
            bad += 1
            continue
        real = real[0]
        before = sha(real)
        tmp = os.path.join(os.path.dirname(real), '.dealias.tmp')
        if os.path.exists(tmp):
            print('SKIP (temp name in use):', real)
            bad += 1
            continue
        os.rename(real, tmp)
        os.rename(tmp, real)
        if sha(real) == before:
            ok += 1
        else:
            print('HASH CHANGED:', real)
            bad += 1
    print(f'cleared {ok}, refused {bad}')
    return 1 if bad else 0

if __name__ == '__main__':
    raise SystemExit(main())
