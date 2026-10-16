#!/usr/bin/env python3
"""How many corpus documents hold a 3D chart.

OOXML: any chart part naming a 3D plot type (bar3DChart, pie3DChart, line3DChart, area3DChart,
surface3DChart) or carrying c:view3D. BIFF: a CH3D record (0x103A) inside a chart substream.
"""
import re, struct, sys, zipfile, pathlib, olefile

THREE_D = re.compile(rb'<c:(bar3DChart|pie3DChart|line3DChart|area3DChart|surface3DChart|surfaceChart)[ />]')
VIEW3D = re.compile(rb'<c:view3D[ />]')

def ooxml(path):
    kinds = set()
    try:
        with zipfile.ZipFile(path) as z:
            for n in z.namelist():
                if '/charts/' not in n or not n.endswith('.xml'):
                    continue
                data = z.read(n)
                for m in THREE_D.finditer(data):
                    kinds.add(m.group(1).decode())
                if VIEW3D.search(data):
                    kinds.add('view3D')
    except Exception:
        return None
    return kinds

def biff(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return None
    names = [n for n in ole.listdir() if n[-1] in ('Workbook', 'Book')]
    if not names:
        return set()
    data = ole.openstream(names[0]).read()
    off, kinds = 0, set()
    while off + 4 <= len(data):
        rid, rlen = struct.unpack_from('<HH', data, off)
        if off + 4 + rlen > len(data):
            break
        if rid == 0x103A:
            kinds.add('CH3D')
        off += 4 + rlen
    return kinds

def main(root, manifest):
    rows = [l.split('\t')[0] for l in pathlib.Path(manifest).read_text().splitlines()[1:] if l]
    hits = 0
    for rel in rows:
        p = pathlib.Path(root) / rel
        suffix = p.suffix.lower()
        kinds = (biff(p) if suffix in ('.xls', '.xlt')
                 else ooxml(p) if suffix in ('.docx', '.xlsx', '.xlsm', '.pptx', '.xltx', '.dotx', '.potx', '.xlsb')
                 else None)
        if kinds:
            hits += 1
            print(f"{rel}\t{','.join(sorted(kinds))}")
    print(f"# {hits} of {len(rows)} documents hold a 3D chart", file=sys.stderr)

main(sys.argv[1], sys.argv[2])
