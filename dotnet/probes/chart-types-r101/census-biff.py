#!/usr/bin/env python3
"""Census BIFF chart-type records in .xls (and OLE-embedded charts in .doc/.ppt).
Walks every stream of the compound file, scans BIFF record headers, and reports
the CHTYPEGROUP-child type records that decide the chart type."""
import csv, os, struct, sys, olefile

ROOT="/home/user/sample-files"
TYPE_RECS = {
    0x1017: "CHBAR", 0x1018: "CHLINE", 0x1019: "CHPIE", 0x101A: "CHAREA",
    0x101B: "CHSCATTER", 0x103E: "CHRADARLINE", 0x103F: "CHSURFACE",
    0x1040: "CHRADARAREA", 0x1061: "CHPIEEXT",
}
OTHER = {0x1014: "CHTYPEGROUP", 0x1002: "CHCHART", 0x103D: "CHDROPBAR",
         0x101C: "CHCHARTLINE"}

def scan(data):
    """Yield (recid, payload) for a plausible BIFF record stream."""
    n = len(data); off = 0
    while off + 4 <= n:
        rid, rlen = struct.unpack_from("<HH", data, off)
        if off + 4 + rlen > n: break
        yield rid, data[off+4:off+4+rlen]
        off += 4 + rlen

def feats_for(data):
    out = set()
    seen_group = False
    for rid, pay in scan(data):
        if rid in OTHER:
            out.add(OTHER[rid]); continue
        if rid not in TYPE_RECS: continue
        name = TYPE_RECS[rid]
        out.add(name)
        if rid == 0x1017 and len(pay) >= 6:
            flags = struct.unpack_from("<H", pay, 4)[0]
            out.add("CHBAR.horizontal" if flags & 0x0001 else "CHBAR.vertical")
            if flags & 0x0002: out.add("CHBAR.stacked")
            if flags & 0x0004: out.add("CHBAR.percent")
            if flags & 0x0008: out.add("CHBAR.3d")
        if rid in (0x1018, 0x101A) and len(pay) >= 2:
            f = struct.unpack_from("<H", pay, 0)[0]
            if f & 0x0001: out.add(name + ".stacked")
            if f & 0x0002: out.add(name + ".percent")
            if f & 0x0004: out.add(name + ".3d")
        if rid == 0x1019 and len(pay) >= 4:
            hole = struct.unpack_from("<H", pay, 2)[0]
            out.add("CHPIE.donut" if hole > 0 else "CHPIE.solid")
            if len(pay) >= 6 and struct.unpack_from("<H", pay, 4)[0] & 0x0001:
                out.add("CHPIE.3d")
        if rid == 0x101B and len(pay) >= 6:
            f = struct.unpack_from("<H", pay, 4)[0]
            out.add("CHSCATTER.bubbles" if f & 0x0001 else "CHSCATTER.plain")
        if rid in (0x103E, 0x1040) and len(pay) >= 2:
            pass
    return out

w = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
w.writerow(["path","ext","stream","feature"])
for r in csv.DictReader(open(os.path.join(ROOT,"MANIFEST.tsv")), delimiter="\t"):
    ext = r["ext"].lower()
    if ext not in ("xls","doc","ppt"): continue
    p = os.path.join(ROOT, r["path"])
    try:
        if not olefile.isOleFile(p):
            sys.stderr.write("NOTOLE %s\n" % r["path"]); continue
        ole = olefile.OleFileIO(p)
    except Exception as e:
        sys.stderr.write("BADOLE %s %s\n" % (r["path"], e)); continue
    for entry in ole.listdir(streams=True, storages=False):
        name = "/".join(entry)
        try: d = ole.openstream(entry).read()
        except Exception: continue
        if len(d) < 8: continue
        fs = feats_for(d)
        fs = {f for f in fs if not f.startswith("CHCHART") or f in ("CHCHART","CHTYPEGROUP")}
        if not ({"CHTYPEGROUP"} & fs): continue
        for f in sorted(fs):
            w.writerow([r["path"], ext, name, f])
    ole.close()
