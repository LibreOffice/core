#!/usr/bin/env python3
"""Which of the swept documents changed, and how each mover scores against the banked reference."""
import hashlib, pathlib, subprocess, sys, re

TARGETS = [l.strip() for l in open('/home/user/r190/targets.txt') if l.strip()]
BEFORE = pathlib.Path('/home/user/r190/before')
AFTER = pathlib.Path('/home/user/r190/after')
REF = pathlib.Path('/home/user/r186/cs/ref')
DIFF = '/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py'

def only(d):
    if not d.is_dir(): return None
    pdfs = sorted(d.glob('*.pdf'))
    return pdfs[0] if pdfs else None

def digest(p):
    return hashlib.md5(p.read_bytes()).hexdigest() if p else None

movers = []
missing = 0
for path in TARGETS:
    ident = hashlib.md5(path.encode()).hexdigest()[:12]
    b, a = only(BEFORE / ident), only(AFTER / ident)
    if b is None or a is None:
        missing += 1; continue
    if digest(b) != digest(a):
        movers.append((path, ident, b, a))

print(f'{len(TARGETS)} documents, {missing} unrendered, {len(movers)} moved, '
      f'{len(TARGETS) - missing - len(movers)} byte-identical')

def score(pdf, ident):
    ref = REF / f'{ident}.pdf'
    if not ref.exists(): return None
    out = subprocess.run(['python3', DIFF, str(pdf), str(ref), '--outdir', f'/tmp/c-{ident}'],
                         capture_output=True, text=True, timeout=600).stdout
    worst = 0.0
    for line in out.splitlines():
        f = line.split('\t')
        if len(f) >= 3 and re.fullmatch(r'\d+', f[0]):
            worst = max(worst, float(f[1]))
    subprocess.run(['rm', '-rf', f'/tmp/c-{ident}'])
    return worst

for path, ident, b, a in movers:
    sb, sa = score(b, ident), score(a, ident)
    name = path.rsplit('/', 1)[-1]
    print(f'{sb if sb is None else f"{sb:6.2f}"} -> {sa if sa is None else f"{sa:6.2f}"}   {name}')
