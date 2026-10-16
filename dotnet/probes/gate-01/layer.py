"""Is the bullet a *text-showing operator* in the reference PDF, or something poppler invents?

This is the producer-side half of task 1, and it is the half that IS answerable here.  It does
not need an older LibreOffice or an older poppler: it reads the PDF bytes directly.

A glyph reaches an extractor as text only if the producer wrote (a) a Tj/TJ operator selecting
it and (b) a ToUnicode CMap mapping its code to a Unicode scalar.  If both are present, every
conformant extractor of any vintage has everything it needs -- so a bullet in the text layer is
a statement about LibreOffice's PDF export, not about poppler's tokeniser.  (The converse is not
established: poppler could still have changed how it *joins* those glyphs into tokens.  Said in
the write-up, not hidden here.)

Usage: layer.py <pdf> [<codepoint-hex> ...]
"""
import re, sys, zlib


def objects(buf):
    """Crude but sufficient: every `N G obj … endobj` span in the file, uncompressed only."""
    out = {}
    for m in re.finditer(rb"(\d+)\s+(\d+)\s+obj\b", buf):
        num = int(m.group(1))
        end = buf.find(b"endobj", m.end())
        if end > 0:
            out[num] = buf[m.end():end]
    return out


def stream_of(body):
    m = re.search(rb"stream\r?\n", body)
    if not m:
        return None
    raw = body[m.end():body.rfind(b"endstream")]
    if b"FlateDecode" in body:
        try:
            return zlib.decompress(raw)
        except zlib.error:
            try:
                return zlib.decompressobj().decompress(raw)
            except zlib.error:
                return None
    return raw


def tounicode_maps(objs):
    """code -> unicode, per ToUnicode stream, tagged with the object number."""
    maps = {}
    for num, body in objs.items():
        if b"/CMapType" not in body and b"beginbfchar" not in body and b"beginbfrange" not in body:
            data = stream_of(body) if b"stream" in body else None
            if not data or b"beginbf" not in data:
                continue
        else:
            data = stream_of(body) if b"stream" in body else body
        if not data or b"beginbf" not in data:
            continue
        m = {}
        for blk in re.findall(rb"beginbfchar(.*?)endbfchar", data, re.S):
            for src, dst in re.findall(rb"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", blk):
                m[int(src, 16)] = int(dst[:4], 16)
        for blk in re.findall(rb"beginbfrange(.*?)endbfrange", data, re.S):
            for lo, hi, dst in re.findall(rb"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", blk):
                lo, hi, dst = int(lo, 16), int(hi, 16), int(dst[:4], 16)
                for i in range(lo, min(hi, lo + 512) + 1):
                    m[i] = dst + (i - lo)
        if m:
            maps[num] = m
    return maps


def main():
    pdf = sys.argv[1]
    wanted = {int(x, 16) for x in sys.argv[2:]} or {0x2022, 0xF0B7, 0xF0A7, 0x25CF, 0xF076, 0xF0D8}
    buf = open(pdf, "rb").read()
    objs = objects(buf)
    maps = tounicode_maps(objs)
    print(f"{pdf}\n  {len(objs)} top-level objects, {len(maps)} ToUnicode CMaps")
    hits = {}
    for num, m in maps.items():
        for code, uni in m.items():
            if uni in wanted:
                hits.setdefault(uni, []).append((num, code))
    if not hits:
        print("  no ToUnicode entry maps to any wanted code point")
    for uni, where in sorted(hits.items()):
        print(f"  U+{uni:04X}  mapped by {len(where)} CMap entr(ies): "
              + ", ".join(f"obj {n} code {c:#04x}" for n, c in where[:6]))

    # And the other half: is that code actually shown by a text operator anywhere?
    shown = 0
    for num, body in objs.items():
        if b"stream" not in body:
            continue
        data = stream_of(body)
        if not data or (b"Tj" not in data and b"TJ" not in data):
            continue
        for uni, where in hits.items():
            for _, code in where:
                pat = re.escape(bytes([code])) if code < 256 else re.escape(code.to_bytes(2, "big"))
                for m in re.finditer(rb"\((?:[^()\\]|\\.)*\)|<[0-9A-Fa-f\s]+>", data):
                    tok = m.group(0)
                    body_after = data[m.end():m.end() + 40]
                    if b"Tj" not in body_after[:8] and b"TJ" not in body_after[:40]:
                        continue
                    if tok.startswith(b"<"):
                        hexs = bytes.fromhex(re.sub(rb"\s", b"", tok[1:-1]).decode())
                        if pat in re.escape(hexs):
                            shown += 1
                    elif pat in re.escape(tok):
                        shown += 1
    print(f"  text-showing operators containing a wanted code: {shown}")


main()
