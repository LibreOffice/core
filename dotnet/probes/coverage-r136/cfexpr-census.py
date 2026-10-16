#!/usr/bin/env python3
"""`cfRule type="expression"` whose formula this tree cannot evaluate.

    cfexpr-census.py <corpus-root> <manifest.tsv> <out.tsv>

`XlsxConditionalStyles.ConditionOf` sends an `expression` rule to `Comparison.Parse`, which
accepts exactly `<operand> <op> <operand>` where each operand is a cell reference or a
literal (`SheetConditions.cs`:207-230).  Anything else -- a bare defined name, a function
call, a comparison against a name -- returns null and the rule paints nothing.  This
reimplements that acceptance test and counts, per workbook, the expression rules it accepts
and the ones it refuses, with the cells each rule's `sqref` covers beside them so the refusal
can be weighed rather than counted.
"""
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
OPS = ['<>', '<=', '>=', '=', '<', '>']
REF = re.compile(r'^\$?[A-Za-z]{1,3}\$?\d{1,7}$')
LIT = re.compile(r'^(-?\d+(\.\d+)?([eE][-+]?\d+)?|"[^"]*"|TRUE|FALSE)$', re.I)


def operand(text):
    t = text.strip()
    return bool(t) and (bool(REF.match(t)) or bool(LIT.match(t)))


def split_at(text, op):
    depth, quoted = 0, False
    i = 0
    while i < len(text):
        c = text[i]
        if c == '"':
            quoted = not quoted
        elif not quoted:
            if c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            elif depth == 0 and text.startswith(op, i):
                return i
        i += 1
    return -1


def accepted(formula):
    t = (formula or '').strip()
    if not t:
        return False
    if t.startswith('='):
        t = t[1:]
    for op in OPS:
        at = split_at(t, op)
        if at < 0:
            continue
        return operand(t[:at]) and operand(t[at + len(op):])
    return False


def cells(sqref):
    n = 0
    for part in (sqref or '').split():
        m = re.match(r'^\$?([A-Za-z]{1,3})\$?(\d+)(?::\$?([A-Za-z]{1,3})\$?(\d+))?$', part)
        if not m:
            continue

        def col(s):
            v = 0
            for ch in s.upper():
                v = v * 26 + ord(ch) - 64
            return v
        c1, r1 = col(m.group(1)), int(m.group(2))
        c2, r2 = (col(m.group(3)), int(m.group(4))) if m.group(3) else (c1, r1)
        n += (abs(c2 - c1) + 1) * (abs(r2 - r1) + 1)
    return n


root, man, out = sys.argv[1:4]
rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4 or p[3] not in ('xlsx', 'xlsm', 'xltx'):
        continue
    ok = bad = badcells = 0
    shapes = set()
    try:
        z = zipfile.ZipFile(pathlib.Path(root) / p[2])
    except (zipfile.BadZipFile, OSError):
        continue
    with z:
        for name in z.namelist():
            if not re.match(r'^xl/worksheets/sheet\d+\.xml$', name):
                continue
            try:
                sh = ET.fromstring(z.read(name))
            except ET.ParseError:
                continue
            for block in sh.iter(NS + 'conditionalFormatting'):
                sq = block.get('sqref')
                for rule in block.findall(NS + 'cfRule'):
                    if rule.get('type') != 'expression':
                        continue
                    f = rule.find(NS + 'formula')
                    text = f.text if f is not None else ''
                    if accepted(text):
                        ok += 1
                    else:
                        bad += 1
                        badcells += cells(sq)
                        shapes.add((text or '')[:40])
    if ok or bad:
        rows.append((p[2], ok, bad, badcells, ' | '.join(sorted(shapes))[:160]))

rows.sort(key=lambda r: -r[3])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\texpression_rules_accepted\trefused\tcells_under_a_refused_rule\tformulas\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print(f'{len(rows)} workbooks state a cfRule type="expression"; '
      f'{sum(r[1] for r in rows)} rules are of the shape this tree evaluates and '
      f'{sum(r[2] for r in rows)} are not, in {sum(1 for r in rows if r[2])} workbooks, '
      f'covering {sum(r[3] for r in rows)} cells')
