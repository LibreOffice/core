#!/usr/bin/env python3
"""What 26.2.4.2's own ODF export of the corpus says about smoothing and of-pie.

/home/user/corpus-odf holds the reference's `--convert-to` of all 947 corpus documents, so this
is the reference's *resolved model* rather than the source markup — the second leg every claim
in this round gets.

Run: python3 census-odf-chart.py > odf-chart.tsv
"""
import pathlib, re, zipfile

ROOT = pathlib.Path('/home/user/corpus-odf')
WANTED = re.compile(r'chart:interpolation="([^"]*)"|chart:spline-resolution="([^"]*)"'
                    r'|chart:spline-order="([^"]*)"'
                    r'|loext:sub-(bar|pie)="([^"]*)"|loext:split-position="([^"]*)"')

print('rendering\tobject\tstatement')
seen = {'interpolation': set(), 'ofpie': set()}

for f in sorted(ROOT.rglob('*')):
    if f.suffix.lower() not in ('.odt', '.ods', '.odp'):
        continue
    try:
        z = zipfile.ZipFile(f)
    except Exception:
        continue
    for name in z.namelist():
        if not name.endswith('content.xml'):
            continue
        try:
            text = z.read(name).decode('utf8', 'replace')
        except Exception:
            continue
        hits = sorted({m.group(0) for m in WANTED.finditer(text)})
        if not hits:
            continue
        rel = str(f.relative_to(ROOT))
        print(f'{rel}\t{name}\t{" ".join(hits)}')
        if any('interpolation' in h for h in hits):
            seen['interpolation'].add(rel)
        if any('sub-' in h for h in hits):
            seen['ofpie'].add(rel)

print(f'# {len(seen["interpolation"])} of 947 renderings state chart:interpolation')
print(f'# {len(seen["ofpie"])} of 947 renderings state an of-pie extension attribute')
