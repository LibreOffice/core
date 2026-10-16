#!/usr/bin/env python3
"""Dump a .ppt's TextChars/TextBytes + StyleTextPropAtom runs, per slide, per shape.

Independent of the C# reader on purpose: the question this round asks is what character
height an EMPTY paragraph inherits, and an instrument that shares a parser with the thing
under test cannot answer it.

    ppt-style-dump.py <file.ppt> [--slide N]

MS-PPT StyleTextPropAtom layout: a paragraph-run array then a character-run array, back to
back, with nothing between them.  The boundary is found by summing the paragraph counts
until they cover the text -- exactly as PptTextReader.ReadStyle does, and the one place a
misread optional field silently shifts the second array.
"""
import argparse, struct, sys
import olefile

TEXT_HEADER_ATOM = 3999
TEXT_CHARS_ATOM  = 4000
TEXT_BYTES_ATOM  = 4008
STYLE_TEXT_PROP  = 4001
SLIDE            = 1006
MAIN_MASTER      = 1016


def records(data, start=0, end=None):
    end = len(data) if end is None else end
    pos = start
    while pos + 8 <= end:
        ver_inst, rtype, length = struct.unpack_from("<HHI", data, pos)
        body = pos + 8
        stop = min(body + length, end)
        yield (ver_inst & 0x0F, ver_inst >> 4, rtype, body, stop)
        pos = stop
        if length == 0 and rtype == 0:
            break


def walk(data, start, end, depth=0):
    for ver, inst, rtype, body, stop in records(data, start, end):
        yield (ver, inst, rtype, body, stop, depth)
        if ver == 0x0F:
            yield from walk(data, body, stop, depth + 1)


def read_paras(b, textlen):
    pos = 0
    paras = []
    covered = 0
    while covered <= textlen and pos + 10 <= len(b):
        count, = struct.unpack_from("<I", b, pos)
        depth, = struct.unpack_from("<H", b, pos + 4)
        pos += 6
        mask, = struct.unpack_from("<I", b, pos)
        pos += 4
        f = {}
        def take16(name):
            nonlocal pos
            v, = struct.unpack_from("<H", b, pos); pos += 2; f[name] = v
        def take32(name):
            nonlocal pos
            v, = struct.unpack_from("<I", b, pos); pos += 4; f[name] = v
        def skip(n):
            nonlocal pos
            pos += n
        if mask & 0x0000000F: take16("bulletFlags")
        if mask & 0x00000080: take16("bulletChar")
        if mask & 0x00000010: take16("bulletFont")
        if mask & 0x00000040: take16("bulletHeight")
        if mask & 0x00000020: take32("bulletColour")
        if mask & 0x00000800: take16("align")
        if mask & 0x00001000: take16("lineFeed")
        if mask & 0x00002000: take16("spaceBefore")
        if mask & 0x00004000: take16("spaceAfter")
        if mask & 0x00000100: take16("textOfs")
        if mask & 0x00000400: take16("bulletOfs")
        if mask & 0x00008000: skip(2)
        if mask & 0x00100000:
            stops, = struct.unpack_from("<H", b, pos); pos += 2; skip(stops * 4)
        if mask & 0x00010000: skip(2)
        if mask & 0x000E0000: skip(2)
        if mask & 0x00200000: skip(2)
        if pos > len(b): break
        f["count"], f["depth"], f["mask"] = count, depth, mask
        paras.append(f)
        if count <= 0: break
        covered += count
    return paras, pos


def read_chars(b, pos, textlen):
    chars = []
    covered = 0
    while covered < textlen and pos + 8 <= len(b):
        count, = struct.unpack_from("<I", b, pos); pos += 4
        mask, = struct.unpack_from("<I", b, pos); pos += 4
        f = {"count": count, "mask": mask}
        if mask & 0xFFFF:
            v, = struct.unpack_from("<H", b, pos); pos += 2; f["flags"] = v
        if mask & 0x00010000:
            v, = struct.unpack_from("<H", b, pos); pos += 2; f["fontIndex"] = v
        if mask & 0x00200000: pos += 2
        if mask & 0x00400000: pos += 2
        if mask & 0x00800000: pos += 2
        if mask & 0x00020000:
            v, = struct.unpack_from("<H", b, pos); pos += 2; f["fontHeight"] = v
        if mask & 0x00040000:
            v, = struct.unpack_from("<I", b, pos); pos += 4; f["colour"] = v
        if mask & 0x00080000:
            v, = struct.unpack_from("<h", b, pos); pos += 2; f["escapement"] = v
        if pos > len(b): break
        chars.append(f)
        if count <= 0: break
        covered += count
    return chars


def slides(data):
    """Top-level slide containers, in stream order."""
    out = []
    for ver, inst, rtype, body, stop, depth in walk(data, 0, len(data)):
        if rtype == SLIDE and ver == 0x0F:
            out.append((body, stop))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--slide", type=int, default=None, help="1-based slide index")
    args = ap.parse_args()

    ole = olefile.OleFileIO(args.path)
    data = ole.openstream("PowerPoint Document").read()

    for n, (body, stop) in enumerate(slides(data), start=1):
        if args.slide and n != args.slide:
            continue
        print(f"===== slide {n}  [{body}:{stop}]")
        kind = None
        text = None
        for ver, inst, rtype, b, e, depth in walk(data, body, stop):
            if rtype == TEXT_HEADER_ATOM:
                kind, = struct.unpack_from("<I", data, b)
                text = None
            elif rtype == TEXT_CHARS_ATOM:
                text = data[b:e].decode("utf-16-le", "replace")
            elif rtype == TEXT_BYTES_ATOM:
                text = data[b:e].decode("latin-1", "replace")
            elif rtype == STYLE_TEXT_PROP:
                if text is None:
                    print("  style atom with no text")
                    continue
                paras, pos = read_paras(data[b:e], len(text))
                chars = read_chars(data[b:e], pos, len(text))
                print(f"  shape kind={kind} textlen={len(text)}")
                off = 0
                for i, p in enumerate(paras):
                    seg = text[off:off + max(p['count'], 0)]
                    print(f"    para[{i}] start={off} count={p['count']} depth={p['depth']} "
                          f"before={p.get('spaceBefore')} after={p.get('spaceAfter')} "
                          f"lf={p.get('lineFeed')} text={seg[:48]!r}")
                    off += max(p["count"], 0)
                off = 0
                for i, c in enumerate(chars):
                    seg = text[off:off + max(c['count'], 0)]
                    print(f"    char[{i}] start={off} count={c['count']} "
                          f"height={c.get('fontHeight')} font={c.get('fontIndex')} "
                          f"text={seg[:32]!r}")
                    off += max(c["count"], 0)


if __name__ == "__main__":
    main()
