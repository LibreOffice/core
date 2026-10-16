#!/usr/bin/env python3
"""Which swept documents moved, and how each mover scores against a banked 26.2.4.2 reference.

    ./confine.py <targets.txt> <before-dir> <after-dir> [ref-dir]

`before` and `after` are two runs of `sweep.py` over the same target list. A document whose bytes are
equal did not move; for the rest the worst page's `diff%` against the reference is printed on both
sides, which is the direction of the change rather than its size. The reference directory holds one
`<ident>.pdf` per document, keyed the same way `sweep.py` keys its output directories.
"""
import hashlib
import pathlib
import re
import subprocess
import sys

TARGETS = [line.strip() for line in open(sys.argv[1]) if line.strip()]
BEFORE = pathlib.Path(sys.argv[2])
AFTER = pathlib.Path(sys.argv[3])
REF = pathlib.Path(sys.argv[4]) if len(sys.argv) > 4 else None
DIFF = ('/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py')


def only(directory: pathlib.Path) -> pathlib.Path | None:
    if not directory.is_dir():
        return None
    pdfs = sorted(directory.glob('*.pdf'))
    return pdfs[0] if pdfs else None


def digest(path: pathlib.Path | None) -> str | None:
    return hashlib.md5(path.read_bytes()).hexdigest() if path else None


movers = []
missing = 0
for rel in TARGETS:
    ident = hashlib.md5(rel.encode()).hexdigest()[:12]
    before, after = only(BEFORE / ident), only(AFTER / ident)
    if before is None or after is None:
        missing += 1
        continue
    if digest(before) != digest(after):
        movers.append((rel, ident, before, after))

print(f'{len(TARGETS)} documents, {missing} unrendered, {len(movers)} moved, '
      f'{len(TARGETS) - missing - len(movers)} byte-identical')

if REF is None:
    for rel, _, _, _ in movers:
        print('  moved', rel)
    raise SystemExit(0)


def score(pdf: pathlib.Path, ident: str) -> tuple[float, float] | None:
    """The worst page's diff% and the sum over the pages, against the banked reference."""
    ref = REF / f'{ident}.pdf'
    if not ref.exists():
        return None
    out = subprocess.run(
        ['python3', DIFF, str(pdf), str(ref), '--outdir', f'/tmp/pagev-{ident}'],
        capture_output=True, text=True, timeout=900).stdout
    worst = total = 0.0
    for line in out.splitlines():
        fields = line.split('\t')
        if len(fields) >= 3 and re.fullmatch(r'\d+', fields[0]):
            worst = max(worst, float(fields[1]))
            total += float(fields[1])
    subprocess.run(['rm', '-rf', f'/tmp/pagev-{ident}'], check=False)
    return worst, total


for rel, ident, before, after in movers:
    a, b = score(before, ident), score(after, ident)
    name = rel.rsplit('/', 1)[-1]
    if a is None or b is None:
        print(f'{"  unscored":>28}   {name}')
        continue
    print(f'worst {a[0]:6.2f} -> {b[0]:6.2f}   sum {a[1]:7.2f} -> {b[1]:7.2f}   {name}')
