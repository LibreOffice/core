#!/usr/bin/env python3
"""For every corpus OOXML document: per line/scatter plot group, the c:smooth flags."""
import zipfile, re, pathlib, sys, csv
ROOT = pathlib.Path('/home/user/sample-files')
man = ROOT/'MANIFEST.tsv'
rows = [l.split("\t")[2] for l in man.read_text().splitlines()[1:] if l]
GRP = re.compile(r'<(?:\w+:)?(lineChart|scatterChart|line3DChart)\b(.*?)</(?:\w+:)?\1>', re.S)
SER = re.compile(r'<(?:\w+:)?ser\b')
SM  = re.compile(r'<(?:\w+:)?smooth val="(\d)"\s*/>')
out = csv.writer(sys.stdout, delimiter='\t')
out.writerow(['path','part','group','nser','smooth_flags'])
for rel in rows:
    p = ROOT/rel
    if p.suffix.lower() not in ('.xlsx','.xlsm','.pptx','.docx','.xltx','.dotx','.potx','.ppsx','.pptm','.docm','.xlsb'): continue
    try: z = zipfile.ZipFile(p)
    except Exception: continue
    for n in z.namelist():
        if '/charts/' not in n or not n.endswith('.xml'): continue
        try: d = z.read(n).decode('utf8','replace')
        except Exception: continue
        for m in GRP.finditer(d):
            body = m.group(2)
            pieces = re.split(r'<(?:\w+:)?ser\b', body)[1:]
            flags = []
            for s in pieces:
                sm = SM.search(s)
                flags.append(sm.group(1) if sm else '-')
            if not flags: continue
            out.writerow([rel, n, m.group(1), len(flags), ','.join(flags)])
