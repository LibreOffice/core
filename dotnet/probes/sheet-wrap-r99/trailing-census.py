#!/usr/bin/env python3
"""Corpus census: cells whose text ends in a hard break — a trailing empty paragraph.

Reports, per document, how many such cells there are and how many of them are RICH
(more than one formatting run), because only the rich ones took the layouter path that
dropped the line. Handles .ods/.fods (ODF) and .xlsx/.xlsm (SpreadsheetML) by reading
the markup; .xls is left to the reader harness.

usage: trailing-census.py <root> [...]
"""
import sys, os, re, zipfile, collections
import xml.etree.ElementTree as ET

ODF = {'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
       'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
       'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0'}
def q(p, l, ns=ODF): return '{%s}%s' % (ns[p], l)

SS = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'

def ods_counts(path):
    """(cells with a trailing empty paragraph, of those how many carry a span)"""
    try:
        with zipfile.ZipFile(path) as z:
            data = z.read('content.xml')
    except Exception:
        return None
    total = rich = 0
    try:
        root = ET.fromstring(data)
    except Exception:
        return None
    for cell in root.iter(q('table','table-cell')):
        ps = cell.findall(q('text','p'))
        if len(ps) < 2: continue
        if ''.join(ps[-1].itertext()) != '': continue
        total += 1
        if sum(len(p.findall(q('text','span'))) for p in ps) > 0: rich += 1
    return total, rich

def xlsx_counts(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    total = rich = 0
    def scan_si(el):
        nonlocal total, rich
        rs = el.findall('{%s}r' % SS)
        if rs:
            texts = [(t.text or '') for r in rs for t in r.findall('{%s}t' % SS)]
            s = ''.join(texts)
            runs = len(rs)
        else:
            t = el.find('{%s}t' % SS)
            s = (t.text or '') if t is not None else ''
            runs = 1
        if s.endswith('\n') and s.strip('\n') != '':
            total += 1
            if runs > 1: rich += 1
    names = z.namelist()
    for n in names:
        if n == 'xl/sharedStrings.xml':
            try: root = ET.fromstring(z.read(n))
            except Exception: continue
            for si in root.iter('{%s}si' % SS): scan_si(si)
        elif n.startswith('xl/worksheets/sheet') and n.endswith('.xml'):
            try: root = ET.fromstring(z.read(n))
            except Exception: continue
            for isel in root.iter('{%s}is' % SS): scan_si(isel)
    z.close()
    return total, rich

roots = sys.argv[1:]
rows = []
for root in roots:
    for dirpath, _, files in os.walk(root):
        for f in sorted(files):
            p = os.path.join(dirpath, f)
            lo = f.lower()
            if lo.endswith(('.ods','.fods')):
                r = ods_counts(p)
            elif lo.endswith(('.xlsx','.xlsm')):
                r = xlsx_counts(p)
            else:
                continue
            if not r: continue
            total, rich = r
            if total: rows.append((f, total, rich))

print('document\tcells\tof which rich')
for f, t, r in sorted(rows, key=lambda x: -x[1]):
    print('%s\t%d\t%d' % (f, t, r))
print('TOTAL\t%d\t%d' % (sum(t for _,t,_ in rows), sum(r for _,_,r in rows)))
print('documents\t%d' % len(rows))
