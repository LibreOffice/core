#!/usr/bin/env python3
"""The two bands of O100 against `style:table-centering`, with no free parameter.

Reads the page geometry and the Invoice sheet's column widths straight out of the
.ods, works out what a horizontally centred print range would be offset by on each
printed page, and prints it beside the band measured from the two PDFs.
"""
import re, zipfile, sys, collections, pymupdf

ODS = '/home/user/corpus-odf/ods/b7fde7cdac59-084_Service_invoice_Use_this_template_c92b43dc.ods'

def topt(s):
    for suf, f in (('in', 72.0), ('cm', 72/2.54), ('mm', 72/25.4), ('pt', 1.0)):
        if s.endswith(suf):
            return float(s[:-2]) * f
    return float(s)

z = zipfile.ZipFile(ODS)
styles = z.read('styles.xml').decode()
content = z.read('content.xml').decode()

lay = re.search(r'<style:page-layout style:name="Mpm4">.*?/>', styles, re.S).group(0)
g = lambda a: re.search(a + r'="([^"]+)"', lay).group(1)
pw, ml, mr = topt(g('fo:page-width')), topt(g('fo:margin-left')), topt(g('fo:margin-right'))
centring = re.search(r'style:table-centering="([^"]+)"', lay).group(1)
printable = pw - ml - mr

widths = {}
for m in re.finditer(r'<style:style style:name="(co\d+)" style:family="table-column".*?</style:style>',
                     content, re.S):
    w = re.search(r'style:column-width="([^"]+)"', m.group(0))
    widths[m.group(1)] = topt(w.group(1)) if w else None

inv = content.split('table:name="Invoice"')[1].split('</table:table>')[0]
cols = []
for c in re.findall(r'<table:table-column[^>]*>', inv):
    n = int(re.search(r'number-columns-repeated="(\d+)"', c).group(1)) if 'repeated' in c else 1
    cols += [widths[re.search(r'table:style-name="(co\d+)"', c).group(1)]] * n

rng = re.search(r'table:print-ranges="Invoice\.([A-Z]+)\d+:Invoice\.([A-Z]+)\d+"', inv).group(1, 2)
first, last = (ord(rng[0]) - 65), (ord(rng[1]) - 65)

# Split the print range into printed pages the way a printer does: greedily, a
# column at a time, until the next one would not fit the printable width.
pages, cur = [], []
for i in range(first, last + 1):
    if cur and sum(cols[c] for c in cur) + cols[i] > printable:
        pages.append(cur); cur = []
    cur.append(i)
pages.append(cur)

print(f'page {pw:.1f} pt wide, margins {ml:.1f}/{mr:.1f}, printable {printable:.2f} pt')
print(f'style:table-centering = {centring!r}   print range '
      f'{chr(65+first)}..{chr(65+last)}')
print()

# The bands, measured.
def spans(p):
    out = {}
    for pn, pg in enumerate(pymupdf.open(p)):
        for b in pg.get_text('dict')['blocks']:
            for l in b.get('lines', []):
                for s in l['spans']:
                    t = s['text'].strip()
                    if t:
                        out.setdefault((pn, round(s['bbox'][1], 1), t), []).append(s['bbox'][0])
    return out
import glob
ours = spans(glob.glob('render/ours/*.pdf')[0])
ref = spans(glob.glob('render/ref/*.pdf')[0])
per = collections.defaultdict(list)
for k in set(ours) & set(ref):
    for a, b in zip(ours[k], ref[k]):
        per[k[0]].append(a - b)

print(f'{"pdf page":>8}  {"cols":>6}  {"width pt":>9}  {"predicted":>9}  {"measured dx":>22}  {"n":>3}')
for pn in sorted(per):
    d = sorted(per[pn])
    if pn < len(pages):
        cs = pages[pn]
        w = sum(cols[c] for c in cs)
        pred = -(printable - w) / 2
        label = ''.join(chr(65 + c) for c in cs)
    else:
        w, pred, label = float('nan'), 0.0, '(About sheet, Mpm3, no centring)'
    print(f'{pn:>8}  {label:>6}  {w:9.3f}  {pred:9.3f}  '
          f'{d[0]:9.2f} .. {d[-1]:9.2f}  {len(d):>3}')
