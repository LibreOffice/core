#!/usr/bin/env python3
"""Join every `.xls` shape's raw Escher fill/line against what 26.2.4.2 resolves them to.

The reference's own flat ODF (`soffice --convert-to fods`) states each shape's graphic
style, so a shape whose `draw:name` is in both halves gives an exact answer for one
MSO_CLR value with no rendering at all. Prints one row per joined shape.

Usage: expected-ink.py <fods-dir> <corpus-root>
"""
import pathlib
import re
import struct
import sys

import olefile

MSODRAWING = 0x00EC
MSODRAWINGGROUP = 0x00EB
CONTINUE = 0x003C
BOF = 0x0809


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        at += 4
        yield rid, data[at:at + size]
        at += size


def flat(buf, out):
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        body = buf[at + 8:at + 8 + size]
        if (ver_inst & 0x0F) == 0x0F:
            flat(body, out)
        else:
            out.append((rtype, ver_inst >> 4, body))
        at += 8 + size


def walk(buf, out):
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        body = buf[at + 8:at + 8 + size]
        if rtype == 0xF004:
            recs = []
            flat(body, recs)
            out.append(recs)
            walk(body, out)
        elif (ver_inst & 0x0F) == 0x0F:
            walk(body, out)
        at += 8 + size


def properties(body, inst):
    props = {}
    complex_props = {}
    at = 0
    complex_at = 6 * inst
    for _ in range(inst):
        if at + 6 > len(body):
            break
        pid_raw, value = struct.unpack_from("<HI", body, at)
        at += 6
        pid = pid_raw & 0x3FFF
        props[pid] = value
        if pid_raw & 0x8000:
            complex_props[pid] = body[complex_at:complex_at + value]
            complex_at += value
    return props, complex_props


def biff_shapes(path):
    ole = olefile.OleFileIO(str(path))
    name = "Workbook" if ole.exists("Workbook") else "Book"
    data = ole.openstream(name).read()
    subs, cur = [], None
    for rid, body in records(data):
        if rid == BOF:
            cur = []
            subs.append(cur)
        if cur is not None:
            cur.append((rid, body))

    found = {}
    for sub in subs:
        dff = bytearray()
        last = None
        for rid, body in sub:
            if rid in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
                last = rid
            elif rid == CONTINUE and last in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
            else:
                last = None
        if not dff:
            continue
        shapes = []
        walk(bytes(dff), shapes)
        for recs in shapes:
            props, cx = {}, {}
            sptype = None
            for rt, inst, body in recs:
                if rt == 0xF00B:
                    p, c = properties(body, inst)
                    props.update(p)
                    cx.update(c)
                elif rt == 0xF00A:
                    sptype = inst
            nm = cx.get(896, b"").decode("utf-16-le", "replace").rstrip("\x00")
            if nm:
                found[nm] = (sptype, props)
    return found


def fods_shapes(path):
    s = path.read_text(encoding="utf-8")
    styles = {}
    for m in re.finditer(r'<style:style style:name="(gr\d+)"[^>]*>(.*?)</style:style>', s, re.S):
        gp = re.search(r"<style:graphic-properties([^>]*?)/?>", m.group(2))
        styles[m.group(1)] = dict(re.findall(r'(draw:fill|draw:fill-color|draw:stroke|svg:stroke-color|svg:stroke-width)="([^"]*)"', gp.group(1))) if gp else {}
    out = {}
    for m in re.finditer(r"<draw:(custom-shape|frame|control|line|connector|g)\b([^>]*)>", s):
        a = m.group(2)
        st = re.search(r'draw:style-name="([^"]*)"', a)
        nm = re.search(r'draw:name="([^"]*)"', a)
        if nm and st:
            out[nm.group(1)] = (m.group(1), styles.get(st.group(1), {}))
    return out


def main():
    fods_dir = pathlib.Path(sys.argv[1])
    corpus = pathlib.Path(sys.argv[2])
    print("document\tshape\tspType\telement\tfillRaw\tlineRaw\trefFill\trefStroke\trefWidth")
    for fods in sorted(fods_dir.glob("*.fods")):
        matches = [p for p in corpus.rglob("*") if p.is_file()
                   and p.suffix.lower() == ".xls" and p.stem == fods.stem]
        if not matches:
            print(f"# no .xls for {fods.stem}", file=sys.stderr)
            continue
        biff = biff_shapes(matches[0])
        odf = fods_shapes(fods)
        for nm, (sptype, props) in sorted(biff.items()):
            if nm not in odf:
                continue
            kind, style = odf[nm]
            fill = props.get(385)
            line = props.get(448)
            if fill is None and line is None:
                continue
            ref_fill = style.get("draw:fill-color", "-") if style.get("draw:fill") == "solid" else "none"
            ref_stroke = style.get("svg:stroke-color", "-") if style.get("draw:stroke") == "solid" else "none"
            print(f"{fods.stem}\t{nm}\t{sptype}\t{kind}\t"
                  f"{'-' if fill is None else f'0x{fill:08x}'}\t"
                  f"{'-' if line is None else f'0x{line:08x}'}\t"
                  f"{ref_fill}\t{ref_stroke}\t{style.get('svg:stroke-width', '-')}")


if __name__ == "__main__":
    main()
