#!/usr/bin/env python3
"""What a workbook's charts themselves draw, on each side.

The chart's own text is the difference between the whole rendering and the same
workbook with its `xdr:graphicFrame` anchors removed, which is exact rather than
geometric: it needs no guess about where the chart sits on the page.
"""
import collections, hashlib, pathlib, re, subprocess, sys

TOK = re.compile(r'\S+')


def toks(p):
    if not pathlib.Path(p).exists():
        return collections.Counter()
    return collections.Counter(
        TOK.findall(subprocess.run(['pdftotext', p, '-'], capture_output=True, text=True).stdout))


for f in sorted(pathlib.Path('frozen').glob('*.xlsx')):
    dw = hashlib.md5(f'frozen/{f.name}'.encode()).hexdigest()
    dn = hashlib.md5(f'nochart/{f.name}'.encode()).hexdigest()
    ours = toks(f'out-frozen/ours/{dw}.pdf') - toks(f'out-nochart/ours/{dn}.pdf')
    ref = toks(f'out-frozen/ref/{dw}.pdf') - toks(f'out-nochart/ref/{dn}.pdf')
    if not ours and not ref:
        continue
    print('#' * 6, f.name[:50])
    print('  OURS-ONLY:', sorted((ours - ref).elements())[:40])
    print('  REF-ONLY :', sorted((ref - ours).elements())[:40])
