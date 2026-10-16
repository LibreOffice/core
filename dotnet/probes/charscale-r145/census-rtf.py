#!/usr/bin/env python3
r"""Census `\charscalex` over a directory of RTF files.

    census-rtf.py <dir-of-*.rtf>

THE TOKENISATION TRAP.  An RTF control word is a backslash, ASCII letters, then an optional
minus and digits, and it ENDS AT THE FIRST NON-DIGIT; a single space after it is a delimiter
and is swallowed.  So `\charscalex100` and `\charscalex1000` are different control words and a
census that matches the prefix alone conflates them, and `\charscalexyz` is not this control
word at all.  The regex below anchors on a letter boundary after `charscalex` and takes the
whole digit string.

A bare `\charscalex` with no parameter is `\charscalex100`: `rtftokenizer.cxx`:253 gives the
keyword a default of 100, so a parameterless occurrence costs nothing and is counted as 100.

Base rates per file: `\par` (paragraphs), `\fsN` (character-size statements, one per changed
character run), `\plain`, and `\expndtw` for comparison — the tracking twin, which this tree's
RTF reader does not read either.
"""
import collections, pathlib, re, sys

SCALE = re.compile(rb'\\charscalex(-?[0-9]*)(?![0-9])')
PAR = re.compile(rb'\\par(?![a-z0-9])')
FS = re.compile(rb'\\fs[0-9]+(?![0-9])')
PLAIN = re.compile(rb'\\plain(?![a-z0-9])')
EXPND = re.compile(rb'\\expndtw(-?[0-9]*)(?![0-9])')

root = pathlib.Path(sys.argv[1])
print('document\tvalue\tcount')
values = collections.Counter()
tot = collections.Counter()
base = collections.Counter()
docs_any, docs_nonunit = set(), set()
nfiles = 0
for f in sorted(root.glob('*.rtf')):
    nfiles += 1
    blob = f.read_bytes()
    base['par'] += len(PAR.findall(blob))
    base['fs'] += len(FS.findall(blob))
    base['plain'] += len(PLAIN.findall(blob))
    base['expndtw'] += len(EXPND.findall(blob))
    hits = collections.Counter()
    for m in SCALE.finditer(blob):
        raw = m.group(1).decode()
        v = int(raw) if raw not in ('', '-') else 100
        hits[v] += 1
        values[v] += 1
        tot['occurrences'] += 1
        docs_any.add(f.name)
        if v != 100:
            tot['non_unit_occurrences'] += 1
            docs_nonunit.add(f.name)
    for v, n in sorted(hits.items()):
        print('%s\t%d\t%d' % (f.name, v, n))

print('\n# files\t%d' % nfiles)
print('# occurrences\t%d' % tot['occurrences'])
print('# occurrences != 100\t%d' % tot['non_unit_occurrences'])
print('# documents stating it\t%d' % len(docs_any))
print('# documents stating != 100\t%d' % len(docs_nonunit))
print('# base \\par\t%d' % base['par'])
print('# base \\fsN\t%d' % base['fs'])
print('# base \\plain\t%d' % base['plain'])
print('# base \\expndtw\t%d' % base['expndtw'])
print('# values\t%s' % dict(sorted(values.items())))
