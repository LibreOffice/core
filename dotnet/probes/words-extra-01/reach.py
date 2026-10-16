#!/usr/bin/env python3
"""Reach: which of the 200 words documents changed, and how the gate verdict moved."""
import os, subprocess, glob, hashlib, sys, concurrent.futures

SC = '/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad'
OLD, NEW = SC + '/sweep-old', SC + '/sweep-new'
REF = '/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words'


def pdf_of(root, key):
    files = glob.glob(os.path.join(root, key, '*.pdf'))
    return files[0] if files else None


def npages(p):
    o = subprocess.run(['pdfinfo', p], capture_output=True, text=True).stdout
    for l in o.splitlines():
        if l.startswith('Pages:'):
            return int(l.split()[1])
    return 0


def words(p):
    o = subprocess.run(['pdftotext', p, '-'], capture_output=True, text=True).stdout
    return len([t for t in o.split() if any(c.isalnum() for c in t)])


def digest(p):
    return hashlib.sha256(open(p, 'rb').read()).hexdigest()


keys = sorted(os.path.basename(d) for d in glob.glob(OLD + '/*') if os.path.isdir(d))


def verdict(op, rp, ow, rw):
    if op != rp:
        return 'PAGES'
    return 'WORDS' if abs(ow - rw) > rw * 0.02 + 3 else 'pass'


def one(key):
    o, n = pdf_of(OLD, key), pdf_of(NEW, key)
    if not o or not n:
        return (key, 'MISSING', None)
    if digest(o) == digest(n):
        return (key, 'same', None)
    # find the banked reference
    cand = [c for c in os.listdir(REF) if c[:-4] == key]
    if not cand:
        return (key, 'changed-noref', None)
    r = os.path.join(REF, cand[0])
    rp, rw = npages(r), words(r)
    op_, ow = npages(o), words(o)
    np_, nw = npages(n), words(n)
    return (key, 'changed', (op_, ow, np_, nw, rp, rw,
                             verdict(op_, rp, ow, rw), verdict(np_, rp, nw, rw)))


with concurrent.futures.ThreadPoolExecutor(max_workers=8) as ex:
    rows = list(ex.map(one, keys))

same = [r for r in rows if r[1] == 'same']
changed = [r for r in rows if r[1] == 'changed']
other = [r for r in rows if r[1] not in ('same', 'changed')]

print(f'documents: {len(rows)}   byte-identical: {len(same)}   changed: {len(changed)}   other: {len(other)}')
for r in other:
    print('  ', r[1], r[0])
print()
print(f"{'document':56}{'oldP':>5}{'newP':>5}{'refP':>5}{'oldW':>7}{'newW':>7}{'refW':>7}  before -> after")
better = worse = neutral = 0
for key, _, d in changed:
    op_, ow, np_, nw, rp, rw, vb, va = d
    move = ''
    if vb != va:
        move = '  ***'
        if va == 'pass':
            better += 1
        elif vb == 'pass':
            worse += 1
    else:
        neutral += 1
    print(f'{key[:56]:56}{op_:>5}{np_:>5}{rp:>5}{ow:>7}{nw:>7}{rw:>7}  {vb} -> {va}{move}')
print()
print(f'verdict flips: {better} to pass, {worse} to fail, {neutral} unchanged verdict')
# word-error movement over the changed set
tot_b = sum(abs(d[1] - d[5]) for _, _, d in changed)
tot_a = sum(abs(d[3] - d[5]) for _, _, d in changed)
print(f'summed |word error| over changed documents: {tot_b} -> {tot_a}')
