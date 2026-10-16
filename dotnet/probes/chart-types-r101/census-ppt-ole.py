#!/usr/bin/env python3
"""Census charts inside .ppt: walk PowerPoint Document records for ExOleObjStg
(0x1011), inflate each, and scan the embedded compound file's BIFF streams for
CHTYPEGROUP-child chart-type records. Also records the storage CLSID, which is
what decides whether LibreOffice routes the object to the BIFF chart reader."""
import csv, io, os, struct, sys, zlib, olefile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from importlib import import_module
biff = import_module("census_biff_lib")

ROOT="/home/user/sample-files"

def ppt_records(d):
    off=0; n=len(d)
    while off+8<=n:
        vi, typ, ln = struct.unpack_from("<HHI", d, off)
        if off+8+ln > n: break
        yield vi, typ, d[off+8:off+8+ln]
        if (vi & 0x000F) == 0xF:
            off += 8              # container: descend
        else:
            off += 8+ln

w=csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
w.writerow(["path","ext","obj","feature"])
for r in csv.DictReader(open(os.path.join(ROOT,"MANIFEST.tsv")), delimiter="\t"):
    if r["ext"].lower()!="ppt": continue
    p=os.path.join(ROOT,r["path"])
    try:
        if not olefile.isOleFile(p): continue
        o=olefile.OleFileIO(p)
        d=o.openstream("PowerPoint Document").read()
        o.close()
    except Exception as e:
        sys.stderr.write("BAD %s %s\n"%(r["path"],e)); continue
    k=0
    for vi,typ,pay in ppt_records(d):
        if typ != 0x1011 or len(pay) < 8: continue
        k+=1
        raw=None
        for cand in (pay[4:], pay):
            for dec in (lambda b: zlib.decompress(b),
                        lambda b: zlib.decompressobj().decompress(b),
                        lambda b: zlib.decompressobj(-15).decompress(b)):
                try:
                    x=dec(cand)
                    if len(x)>=512 and x[:8]==b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1": raw=x; break
                except Exception: pass
            if raw: break
        if raw is None and pay[4:12]==b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1": raw=pay[4:]
        if raw is None and pay[:8]==b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1": raw=pay
        if raw is None:
            sys.stderr.write("INFLATE %s obj%d\n"%(r["path"],k)); continue
        try:
            if not olefile.isOleFile(io.BytesIO(raw)): 
                w.writerow([r["path"],"ppt","obj%d"%k,"!notOLE"]); continue
            eo=olefile.OleFileIO(io.BytesIO(raw))
        except Exception as e:
            sys.stderr.write("EMBOLE %s obj%d %s\n"%(r["path"],k,e)); continue
        clsid = eo.root.clsid if eo.root is not None else ""
        feats=set(["clsid:"+str(clsid)])
        for e in eo.listdir(streams=True, storages=False):
            try: sd = eo.openstream(e).read()
            except Exception: continue
            if len(sd) < 8: continue
            f = biff.feats_for(sd)
            if "CHTYPEGROUP" in f: feats |= f
        eo.close()
        for f in sorted(feats): w.writerow([r["path"],"ppt","obj%d"%k,f])
