#!/usr/bin/env python3
"""Census every corpus document's prior mentions across the project's write-ups.

    mention-census.py <manifest.tsv> <docs-root> <out.tsv> [--rev REV]

`docs-root` is a directory tree scanned for `*.md`.  With `--rev` the same scan is
run against a git revision instead of the working tree, so the figure this round
quotes and the figure round 124 quoted can be produced by ONE instrument.

A mention is:

  * the document's stem as a whole token -- bounded on both sides by something
    that is not a letter, digit or underscore.  For a stem shorter than nine
    characters (there are 31 of them, `09`, `003`, `135`, `762` ...) that is not
    enough on its own, because `09` occurs inside every `2026-09-14`, so those
    must additionally be followed by their own extension; and

  * an ELIDED citation.  Write-ups routinely shorten a long name, `RMI_...GettingOffOil`
    or `ws_prod-...-European-Safety-Strategy-Initiative`.  Every inline-code span
    holding an ellipsis is split on it and matched against each stem in order,
    requiring the leading fragment to be at least six characters so that a span
    like `... Do` cannot match everything.

The point of the second rule is that without it a document a round DID open reads
as never examined, which is the one direction this census must not err in.
"""
import pathlib
import re
import subprocess
import sys

man, root, out = sys.argv[1:4]
rev = None
if '--rev' in sys.argv:
    rev = sys.argv[sys.argv.index('--rev') + 1]
SCOPE = 'dotnet'
if '--scope' in sys.argv:
    SCOPE = sys.argv[sys.argv.index('--scope') + 1]

rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 9:
        continue
    stem = p[2].rsplit('/', 1)[-1].rsplit('.', 1)[0]
    rows.append({'track': p[0], 'batch': p[1], 'path': p[2], 'ext': p[3],
                 'status': p[7], 'kind': p[8], 'stem': stem})

# ---- gather the write-ups -------------------------------------------------
docs = {}
if rev:
    names = subprocess.run(['git', '-C', root, 'ls-tree', '-r', '--name-only', rev, '--',
                            SCOPE], capture_output=True, text=True,
                           check=True).stdout.split('\n')
    for n in names:
        if n.endswith('.md'):
            docs[n] = subprocess.run(['git', '-C', root, 'show', f'{rev}:{n}'],
                                     capture_output=True, text=True).stdout
else:
    for f in sorted(pathlib.Path(root).rglob('*.md')):
        docs[str(f)] = f.read_text(encoding='utf-8', errors='replace')

# ---- elided citations, collected once -------------------------------------
span = re.compile(r'`([^`\n]{4,200})`')
elisions = []            # (parts, writeup-label)
for name, text in docs.items():
    label = pathlib.Path(name).parent.name
    for m in span.finditer(text):
        s = m.group(1)
        if '…' not in s and '...' not in s:
            continue
        parts = [x.strip() for x in re.split(r'…|\.\.\.', s)]
        parts = [x for x in parts if x]
        if len(parts) < 2 or len(parts[0]) < 6:
            continue
        elisions.append((parts, label))

EXTS = ('docx', 'doc', 'pptx', 'ppt', 'xlsx', 'xls', 'xlsm', 'odt', 'ods', 'odp', 'rtf', 'pdf')

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('mentions\tstem\text\ttrack\tbatch\tstatus\tkind\twhere\n')
    nz = 0
    for r in rows:
        stem = r['stem']
        where = []
        if len(stem) < 9:
            pat = re.compile(re.escape(stem) + r'\.(?:' + '|'.join(EXTS) + r')\b', re.I)
        else:
            # `__xlsx` is this project's own rendering-name suffix, so a single
            # trailing underscore must not end the token but a doubled one must.
            alts = [re.escape(stem)]
            h = re.match(r'^(.*)_[0-9a-f]{8}$', stem)
            if h and len(h.group(1)) >= 9:
                alts.append(re.escape(h.group(1)))
            pat = re.compile(r'(?<![A-Za-z0-9_])(?:' + '|'.join(alts) +
                             r')(?![A-Za-z0-9]|_(?!_))', re.I)
        for name, text in docs.items():
            if pat.search(text):
                where.append(pathlib.Path(name).parent.name)
        if not where:
            for parts, label in elisions:
                pos, ok = 0, True
                for part in parts:
                    j = stem.find(part, pos)
                    if j < 0:
                        ok = False
                        break
                    pos = j + len(part)
                if ok:
                    where.append(label)
        where = sorted(set(where))
        if where:
            nz += 1
        fh.write(f'{len(where)}\t{stem}\t{r["ext"]}\t{r["track"]}\t{r["batch"]}\t'
                 f'{r["status"]}\t{r["kind"]}\t{",".join(where)}\n')
print(f'{len(docs)} write-ups scanned; {nz} of {len(rows)} documents mentioned, '
      f'{len(rows) - nz} never named', file=sys.stderr)
