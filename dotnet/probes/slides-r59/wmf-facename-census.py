#!/usr/bin/env python3
"""Every WMF CreateFontIndirect in the corpus, and whether its face-name field is short.

The mirror of round 56's EMF defect, asked in the other direction: `EmfReader` treated a
NUL-terminated field as NUL-padded; does `WmfReader`, which reads that field correctly, read
*past the end of the record* when a writer stores a short one?  A WMF record states its own
size in words, and CreateFontIndirect's payload is an 18-byte LOGFONT plus a face name of up to
32 bytes -- so a payload under 50 bytes means the name field is truncated in the file and a
reader taking a fixed 32 bytes is reading the next record.

Reads WMFs out of OOXML packages, out of `.doc`/`.ppt`/`.xls` by scanning for the placeable and
plain WMF headers, and any standalone `.wmf`.
"""
import collections, os, struct, sys, zipfile

CREATEFONTINDIRECT = 0x02FB


def records(d, start):
    at = start
    out = []
    while at + 6 <= len(d):
        size, fn = struct.unpack_from("<IH", d, at)
        if size < 3:
            break
        nxt = at + size * 2
        if nxt > len(d) or nxt <= at:
            break
        out.append((fn, at + 6, nxt))
        if fn == 0:
            break
        at = nxt
    return out


def scan(blob, name, tally, short):
    # placeable header, then the WMF header (18 bytes)
    at = 0
    if len(blob) >= 22 and blob[:4] == b"\xd7\xcd\xc6\x9a":
        at = 22
    if len(blob) < at + 18:
        return
    at += 18
    for fn, body, end in records(blob, at):
        if fn != CREATEFONTINDIRECT:
            continue
        payload = end - body
        tally[payload] += 1
        if payload < 50:
            face = blob[body + 18:end]
            short.append((name, payload, face.split(b"\0")[0][:32]))


def blobs(path):
    lower = path.lower()
    if lower.endswith(".wmf"):
        yield os.path.basename(path), open(path, "rb").read()
        return
    if lower.endswith((".pptx", ".docx", ".xlsx", ".xlsm", ".pptm", ".docm")):
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            return
        for n in z.namelist():
            if n.lower().endswith(".wmf"):
                yield f"{os.path.basename(path)}!{n}", z.read(n)
        return
    if lower.endswith((".doc", ".ppt", ".xls")):
        d = open(path, "rb").read()
        at = 0
        while True:
            at = d.find(b"\xd7\xcd\xc6\x9a", at)
            if at < 0:
                break
            yield f"{os.path.basename(path)}@{at}", d[at:]
            at += 4


if __name__ == "__main__":
    tally = collections.Counter()
    short = []
    files = 0
    for root in sys.argv[1:]:
        for dirpath, _, names in os.walk(root):
            for n in sorted(names):
                p = os.path.join(dirpath, n)
                if not n.lower().endswith((".wmf", ".pptx", ".docx", ".xlsx", ".xlsm",
                                           ".pptm", ".docm", ".doc", ".ppt", ".xls")):
                    continue
                files += 1
                try:
                    for name, blob in blobs(p):
                        scan(blob, name, tally, short)
                except Exception as exc:
                    print(f"  ! {p}: {exc}", file=sys.stderr)
    print(f"files scanned: {files}")
    print(f"CreateFontIndirect records: {sum(tally.values())}")
    for size in sorted(tally):
        print(f"   payload {size:3d} bytes  x{tally[size]}")
    print(f"records whose face-name field is short: {len(short)}")
    for name, payload, face in short[:40]:
        print(f"   {payload:3d}  {face!r}  {name}")
