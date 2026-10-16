#!/usr/bin/env python3
"""Every EMF/WMF font object in the corpus whose lfCharSet is SYMBOL_CHARSET.

A symbol-charset metafile font addresses glyphs by byte, not by character: LibreOffice moves
the byte into the Private Use Area and recodes it into OpenSymbol, and a reader that decodes
the byte through Windows-1252 instead draws a Latin letter and puts one into extracted text.
Counts records and documents, and separates the faces this stack has a recode table for from
the ones it does not.
"""
import collections, os, re, struct, sys, zipfile

RECODEABLE = {
    "starbats", "starmath", "symbol", "standardsymbols", "standardsymbolsl",
    "monotypesorts", "zapfdingbats", "itczapfdingbats", "dingbats",
    "webdings", "wingdings", "wingdings2", "wingdings3", "mtextra",
}


def norm(s):
    return re.sub(r"[^a-z0-9]", "", (s or "").lower())


def emf_fonts(d):
    q, out = 0, []
    while q + 8 <= len(d):
        t, sz = struct.unpack_from("<II", d, q)
        if sz < 8 or q + sz > len(d):
            break
        if t == 82 and sz >= 12 + 92:
            b = d[q + 12:q + sz]
            name = b[28:28 + 64].decode("utf-16-le", "replace").split("\0")[0]
            out.append((name, b[23]))
        q += sz
    return out


def wmf_fonts(d):
    at = 0
    if len(d) >= 22 and d[:4] == b"\xd7\xcd\xc6\x9a":
        at = 22
    if len(d) < at + 18:
        return []
    at += 18
    out = []
    while at + 6 <= len(d):
        size, fn = struct.unpack_from("<IH", d, at)
        if size < 3:
            break
        nxt = at + size * 2
        if nxt > len(d) or nxt <= at:
            break
        if fn == 0x02FB and nxt - (at + 6) >= 19:
            b = d[at + 6:nxt]
            out.append((b[18:].split(b"\0")[0].decode("latin1"), b[13]))
        if fn == 0:
            break
        at = nxt
    return out


def blobs(path):
    low = path.lower()
    if low.endswith((".pptx", ".docx", ".xlsx", ".xlsm", ".pptm", ".docm")):
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            return
        for n in z.namelist():
            if n.lower().endswith(".emf"):
                yield "emf", z.read(n)
            elif n.lower().endswith(".wmf"):
                yield "wmf", z.read(n)
        return
    if low.endswith((".doc", ".ppt", ".xls")):
        import olefile, zlib
        try:
            o = olefile.OleFileIO(path)
        except Exception:
            return
        for entry in o.listdir():
            try:
                d = o.openstream(entry).read()
            except Exception:
                continue
            # Escher blips: an EMF/WMF payload, deflate-compressed
            for m in re.finditer(rb"\x78\x9c|\x78\x01|\x78\xda", d):
                try:
                    raw = zlib.decompressobj().decompress(d[m.start():])
                except Exception:
                    continue
                if len(raw) < 64:
                    continue
                if raw[:4] == b"\x01\x00\x00\x00":
                    yield "emf", raw
                elif raw[:4] == b"\xd7\xcd\xc6\x9a" or struct.unpack_from("<H", raw, 0)[0] in (1, 2):
                    yield "wmf", raw
        return


if __name__ == "__main__":
    docs = collections.Counter()
    faces = collections.Counter()
    perdoc = collections.Counter()
    for root in sys.argv[1:]:
        for dirpath, _, names in os.walk(root):
            for n in sorted(names):
                if not n.lower().endswith((".pptx", ".docx", ".xlsx", ".xlsm", ".pptm",
                                           ".docm", ".doc", ".ppt", ".xls")):
                    continue
                p = os.path.join(dirpath, n)
                hits = 0
                try:
                    for kind, blob in blobs(p):
                        got = emf_fonts(blob) if kind == "emf" else wmf_fonts(blob)
                        for name, charset in got:
                            if charset == 2:
                                hits += 1
                                faces[(name, norm(name) in RECODEABLE)] += 1
                except Exception:
                    pass
                if hits:
                    track = os.path.basename(os.path.dirname(os.path.dirname(dirpath)))
                    docs[track] += 1
                    perdoc[p] = hits
    print("faces with lfCharSet = SYMBOL:")
    for (name, known), n in faces.most_common():
        print(f"   x{n:5d}  {name!r:26s} recode table: {'yes' if known else 'NO'}")
    print(f"\ndocuments: {sum(docs.values())}  by track: {dict(docs)}")
    for p, n in sorted(perdoc.items(), key=lambda kv: -kv[1]):
        print(f"   {n:5d}  {p}")
