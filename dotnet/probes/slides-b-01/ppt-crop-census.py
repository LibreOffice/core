#!/usr/bin/env python3
"""Report every Escher picture frame carrying a crop, with its slide and anchor.

Slides are attributed by walking the PowerPoint Document stream's top-level
records: a 1006 (Slide) container's PPDrawing (1036) holds that slide's shapes.
Slide order is the persist order in SlideListWithText / the stream order, which
for these decks is the presentation order LibreOffice exports.
"""
import sys, struct
import olefile

CROP = {256: "top", 257: "bottom", 258: "left", 259: "right"}


def stream(path, name="PowerPoint Document"):
    ole = olefile.OleFileIO(path)
    for e in ole.listdir():
        if e[-1].lower() == name.lower():
            return ole.openstream(e).read()
    raise SystemExit("no stream")


def children(buf, off, end):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        stop = min(body + rl, end)
        yield (vi & 0x0F, vi >> 4, rt, body, stop)
        off = body + rl


def opt(buf, body, stop, n):
    props = {}
    p = body
    for _ in range(n):
        if p + 6 > stop:
            break
        pid, val = struct.unpack_from("<HI", buf, p)
        p += 6
        props[pid & 0x3FFF] = val
    return props


def shapes_in(buf, body, stop, out, depth=0):
    for ver, inst, rt, b, s in children(buf, body, stop):
        if rt == 0xF004:  # SpContainer
            sp = {"props": {}, "type": None, "anchor": None, "child": None,
                  "flags": 0, "depth": depth}
            for v2, i2, r2, b2, s2 in children(buf, b, s):
                if r2 == 0xF00A:
                    sp["type"] = i2
                    sp["spid"], sp["flags"] = struct.unpack_from("<II", buf, b2)
                elif r2 == 0xF00B:
                    sp["props"] = opt(buf, b2, s2, i2)
                elif r2 == 0xF010:
                    c = buf[b2:s2]
                    if len(c) >= 16:
                        sp["anchor"] = struct.unpack_from("<iiii", c, 0)
                    elif len(c) >= 8:
                        t, l, r, bo = struct.unpack_from("<hhhh", c, 0)
                        sp["anchor"] = (l, t, r, bo)
                elif r2 == 0xF00F:
                    sp["child"] = struct.unpack_from("<iiii", buf, b2)
            out.append(sp)
        elif rt == 0xF003:  # SpgrContainer
            shapes_in(buf, b, s, out, depth + 1)
        elif ver == 0x0F:
            shapes_in(buf, b, s, out, depth)


def main(path):
    buf = stream(path)
    slideno = 0
    # top-level walk: find Slide (1006) containers in stream order
    todo = [(0, len(buf), None)]
    results = []

    def walk(off, end, slide):
        nonlocal slideno
        for ver, inst, rt, b, s in children(buf, off, end):
            if rt == 1006:
                slideno_local = None
                walk(b, s, ("slide", inst))
            elif rt == 1016:
                walk(b, s, ("master", inst))
            elif rt == 1008:
                walk(b, s, ("notes", inst))
            elif rt == 1036:  # PPDrawing
                out = []
                shapes_in(buf, b, s, out)
                results.append((slide, b, out))
            elif ver == 0x0F:
                walk(b, s, slide)

    walk(0, len(buf), None)

    n = 0
    for i, (slide, off, out) in enumerate(results):
        for sp in out:
            crops = {CROP[k]: v for k, v in sp["props"].items() if k in CROP and v}
            if not crops:
                continue
            n += 1
            a = sp["anchor"] or sp["child"]
            print("drawing#%d %s off=%#x type=%s spid=%s flags=%#x anchor=%s pib=%s"
                  % (i, slide, off, sp["type"], sp.get("spid"), sp["flags"], a,
                     sp["props"].get(260)))
            print("   crops(fraction) " + "  ".join(
                "%s=%d (%.4f)" % (k, v, (v if v < 2**31 else v - 2**32) / 65536.0)
                for k, v in crops.items()))
            if a:
                w, h = a[2] - a[0], a[3] - a[1]
                print("   anchor w=%d h=%d master-units" % (w, h))
    print("== %d drawings, %d cropped shapes" % (len(results), n))


if __name__ == "__main__":
    main(sys.argv[1])
