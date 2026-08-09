#!/usr/bin/env python3
"""Census which shape geometries the slides track actually references.

.ppt: msofbtSp (0xF00A) records in every Escher-bearing stream; the record instance is the
      MSO_SPT shape type.
.pptx: a:prstGeom/@prst over every slide/layout/master part.
"""
import collections, glob, json, os, re, struct, sys, zipfile
import olefile

ROOT = "/workspace/sample-files/slides"
NAMES = json.load(open(os.path.join(os.path.dirname(__file__), "msospt.json")))

def ppt_types(path):
    c = collections.Counter()
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return c, 0
    for entry in ole.listdir(streams=True):
        name = entry[-1]
        if name.startswith("\x05") or "Ole" in name:
            continue
        try:
            data = ole.openstream(entry).read()
        except Exception:
            continue
        i = 0
        n = len(data)
        while True:
            i = data.find(b"\x0a\xf0\x08\x00\x00\x00", i)
            if i < 2:
                break
            hdr = struct.unpack_from("<H", data, i - 2)[0]
            if (hdr & 0x0F) == 2:
                c[hdr >> 4] += 1
            i += 6
    ole.close()
    return c, 1

def pptx_types(path):
    c = collections.Counter()
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return c, 0
    for n in z.namelist():
        if not n.endswith(".xml"):
            continue
        if not any(k in n for k in ("slide", "notes", "chart", "drawing")):
            continue
        try:
            body = z.read(n)
        except Exception:
            continue
        for m in re.finditer(rb'<a:prstGeom[^>]*\sprst="([^"]+)"', body):
            c[m.group(1).decode()] += 1
    return c, 1

def main():
    ppt = collections.Counter(); pptx = collections.Counter()
    ppt_docs = collections.defaultdict(set); pptx_docs = collections.defaultdict(set)
    nppt = npptx = 0
    for path in sorted(glob.glob(ROOT + "/batch-*/*/*")):
        ext = path.rsplit(".", 1)[-1].lower()
        if ext == "ppt":
            c, ok = ppt_types(path); nppt += ok
            ppt.update(c)
            for k in c: ppt_docs[k].add(path)
        elif ext == "pptx":
            c, ok = pptx_types(path); npptx += ok
            pptx.update(c)
            for k in c: pptx_docs[k].add(path)
    print(f"{nppt} .ppt, {npptx} .pptx\n")
    print("=== .ppt MSO_SPT shape types (shapes, documents)")
    for k, v in ppt.most_common():
        print(f"{k:5d} {NAMES.get(str(k),'?'):28s} {v:7d} {len(ppt_docs[k]):4d}")
    print("\n=== .pptx a:prstGeom (shapes, documents)")
    for k, v in pptx.most_common(60):
        print(f"      {k:28s} {v:7d} {len(pptx_docs[k]):4d}")

main()
