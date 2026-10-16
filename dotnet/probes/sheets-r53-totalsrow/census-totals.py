#!/usr/bin/env python3
"""Census: charts whose c:f range ends on an Excel table's totals row.

Reach of the ScChart2DataSequence rule (sc/source/ui/unoobj/chart2uno.cxx:2616-2632):
the last row of a chart data range is dropped when a database range (an imported
Excel table) with a totals row covers that cell and ends on that row.
"""
import os, re, sys, zipfile, csv, collections

ROOT = "/c/sandbox/workdir/sample-files"

def colnum(s):
    n = 0
    for c in s: n = n*26 + (ord(c.upper())-64)
    return n

RANGE = re.compile(r'^\$?([A-Za-z]{1,3})\$?(\d+)(?::\$?([A-Za-z]{1,3})\$?(\d+))?$')

def parse_ref(r):
    m = RANGE.match(r.strip())
    if not m: return None
    c1, r1, c2, r2 = m.groups()
    if c2 is None: c2, r2 = c1, r1
    return (colnum(c1), int(r1), colnum(c2), int(r2))

def split_sheet(text):
    quoted = False; sep = -1
    for i, ch in enumerate(text):
        if ch == "'": quoted = not quoted
        elif ch == '!' and not quoted: sep = i
    if sep <= 0: return None
    name = text[:sep].strip()
    if len(name) >= 2 and name[0] == "'" and name[-1] == "'":
        name = name[1:-1].replace("''", "'")
    return (name, text[sep+1:])

def rels_for(z, part):
    d, b = os.path.split(part)
    rp = f"{d}/_rels/{b}.rels" if d else f"_rels/{b}.rels"
    out = {}
    try: data = z.read(rp)
    except KeyError: return out
    for m in re.finditer(rb'<Relationship\b[^>]*>', data):
        tag = m.group(0).decode('utf8', 'replace')
        rid = re.search(r'Id="([^"]+)"', tag)
        tgt = re.search(r'Target="([^"]+)"', tag)
        typ = re.search(r'Type="([^"]+)"', tag)
        mode = re.search(r'TargetMode="([^"]+)"', tag)
        if rid and tgt and typ:
            out[rid.group(1)] = (typ.group(1), tgt.group(1), mode.group(1) if mode else None)
    return out

def norm(base, target):
    if target.startswith('/'): return target[1:]
    return os.path.normpath(os.path.join(os.path.dirname(base), target)).replace('\\', '/')

def scan(path):
    try: z = zipfile.ZipFile(path)
    except Exception: return None
    names = set(z.namelist())
    wb = 'xl/workbook.xml'
    if wb not in names:
        for n in names:
            if n.endswith('workbook.xml'): wb = n; break
        else: return None
    try: wbx = z.read(wb).decode('utf8', 'replace')
    except Exception: return None
    wrels = rels_for(z, wb)
    sheets = {}   # sheet name -> part
    for m in re.finditer(r'<sheet\b[^>]*/?>', wbx):
        tag = m.group(0)
        nm = re.search(r'name="([^"]*)"', tag)
        rid = re.search(r'r:id="([^"]*)"', tag) or re.search(r'id="([^"]*)"', tag)
        if not nm or not rid or rid.group(1) not in wrels: continue
        sheets[nm.group(1)] = norm(wb, wrels[rid.group(1)][1])
    # tables per sheet
    tables = collections.defaultdict(list)   # sheet name -> [(c1,r1,c2,r2,totals)]
    for nm, part in sheets.items():
        for typ, tgt, mode in rels_for(z, part).values():
            if not typ.endswith('/table') or mode == 'External': continue
            tp = norm(part, tgt)
            try: tx = z.read(tp).decode('utf8', 'replace')
            except KeyError: continue
            ref = re.search(r'<table\b[^>]*\bref="([^"]+)"', tx)
            tot = re.search(r'<table\b[^>]*\btotalsRowCount="(\d+)"', tx)
            idm = re.search(r'<table\b[^>]*\bid="(\d+)"', tx)
            dsp = re.search(r'<table\b[^>]*\bdisplayName="([^"]*)"', tx)
            if not ref: continue
            pr = parse_ref(ref.group(1))
            if not pr: continue
            # LibreOffice's Table::finalizeImport bails without an id or a displayName
            if not idm or int(idm.group(1)) <= 0 or not dsp or not dsp.group(1): continue
            tables[nm].append(pr + (int(tot.group(1)) if tot else 0,))
    if not any(t[4] > 0 for v in tables.values() for t in v):
        return ('no-totals-table', 0, [])
    hits = []
    for n in names:
        if not re.search(r'charts?/chart\d*\.xml$', n): continue
        try: cx = z.read(n).decode('utf8', 'replace')
        except KeyError: continue
        for f in re.findall(r'<c:f>(.*?)</c:f>', cx, re.S):
            f = f.strip()
            if any(ch in f for ch in '(,['): continue
            sp = split_sheet(f)
            if not sp: continue
            sn, ref = sp
            pr = parse_ref(ref)
            if not pr: continue
            c1, r1, c2, r2 = pr
            for (tc1, tr1, tc2, tr2, tot) in tables.get(sn, ()):
                if tot <= 0 or tr2 != r2: continue
                if max(c1, tc1) > min(c2, tc2): continue
                lo, hi = max(c1, tc1), min(c2, tc2)
                whole = (lo == c1 and hi == c2)
                hits.append((n, f, r1, r2, 'ALL-COLS' if whole else 'SOME-COLS',
                             'WHOLE-RANGE' if r1 == r2 else 'LAST-ROW-ONLY'))
                break
    return ('has-totals-table', len(hits), hits)

def main():
    man = {}
    with open(os.path.join(ROOT, 'MANIFEST.tsv')) as fh:
        for row in csv.reader(fh, delimiter='\t'):
            if len(row) < 8 or row[0] == 'family': continue
            man[row[2]] = (row[0], row[7])
    tot_docs = 0; with_tables = 0; hit_docs = []
    for rel, (fam, status) in sorted(man.items()):
        p = os.path.join(ROOT, rel)
        if not os.path.exists(p): continue
        tot_docs += 1
        r = scan(p)
        if r is None: continue
        kind, nhits, hits = r
        if kind == 'has-totals-table': with_tables += 1
        if nhits:
            hit_docs.append((fam, status, rel, hits))
    print(f"documents scanned: {tot_docs}")
    print(f"documents with a totals-row table: {with_tables}")
    print(f"documents where a chart range ends on such a totals row: {len(hit_docs)}")
    for fam, status, rel, hits in hit_docs:
        print(f"  [{fam}/{status}] {rel}")
        for h in hits:
            print(f"      {h[0]}  {h[1]}  rows {h[2]}-{h[3]}  {h[4]}  {h[5]}")

main()
