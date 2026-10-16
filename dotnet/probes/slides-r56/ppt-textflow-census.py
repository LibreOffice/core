#!/usr/bin/env python3
"""Census of Escher `txflTextFlow` (136) and `cdirFont` (137) over the .ppt corpus.

Both decide whether a shape's text is turned on the binary path
(`filter/source/msfilter/svdfppt.cxx:815-846`), and neither is read today.

What this CAN see: the value written on a shape's own `msofbtOPT` (0xF00B), anywhere in the
PowerPoint Document stream -- slides, masters, notes, and shapes inside groups.

What it CANNOT see, stated so a prediction built on it declares its blind spots:
  * inheritance through `DFF_Prop_MasterShape` (769) -- a shape can take the property from
    the master shape it points at, and this counts only the literal writer;
  * whether the shape actually carries any TEXT (no `TextId`/no text record => no turn to
    draw), which is checked separately here via prop 128 but that too can be inherited;
  * whether the shape is on a page that is RENDERED (a master's shape may never be drawn);
  * `msofbtSecondaryOPT` (0xF121) / `msofbtTertiaryOPT` (0xF122), which can also carry 136.

    ppt-textflow-census.py <corpus-root-or-file>...
"""
import collections, os, struct, sys
import olefile

TXFL = 136
CDIR = 137
TEXTID = 128
NAMES = {0: "HorzN", 1: "TtoBA", 2: "BtoT", 3: "TtoBN", 4: "HorzA", 5: "VertN"}


def stream(path, name="PowerPoint Document"):
    ole = olefile.OleFileIO(path)
    for e in ole.listdir():
        if e[-1].lower() == name.lower():
            return ole.openstream(e).read()
    return None


def children(buf, off, end):
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        stop = min(body + rl, end)
        if rl > len(buf):
            return
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


def walk(buf, off, end, out, page):
    for ver, inst, rt, b, s in children(buf, off, end):
        if rt == 0xF004:  # SpContainer
            props = {}
            for v2, i2, r2, b2, s2 in children(buf, b, s):
                if r2 in (0xF00B, 0xF121, 0xF122):
                    props.update(opt(buf, b2, s2, i2))
            if TXFL in props or CDIR in props:
                out.append((page, props.get(TXFL, 0) & 0xFFFF,
                            props.get(CDIR, 0) & 0xFFFF, TEXTID in props))
        elif rt in (0xF003, 0xF002):  # Spgr / Dg container
            walk(buf, b, s, out, page)
        elif ver == 0x0F:
            page2 = page
            if rt == 1006:
                page2 = ("slide", page[1] + 1 if page[0] == "slide" else 1)
            elif rt == 1016:
                page2 = ("master", 0)
            elif rt == 1008:
                page2 = ("notes", 0)
            walk(buf, b, s, out, page2)


def census(path):
    buf = stream(path)
    if buf is None:
        return []
    out = []
    walk(buf, 0, len(buf), out, ("?", 0))
    return out


def files(args):
    for a in args:
        if os.path.isfile(a):
            yield a
        else:
            for root, _, names in os.walk(a):
                for n in sorted(names):
                    if n.lower().endswith(".ppt"):
                        yield os.path.join(root, n)


if __name__ == "__main__":
    totals = collections.Counter()
    for path in sorted(set(files(sys.argv[1:]))):
        rows = census(path)
        nz = [r for r in rows if r[1] not in (0, 4) or r[2] != 0]
        if not nz:
            continue
        c = collections.Counter()
        for page, txfl, cdir, hastext in nz:
            c[(NAMES.get(txfl, txfl), cdir, hastext)] += 1
            totals[NAMES.get(txfl, txfl)] += 1
        print(os.path.basename(path))
        for k, v in sorted(c.items(), key=lambda kv: str(kv[0])):
            print(f"    txfl={k[0]:<6} cdir={k[1]}  textid={k[2]}  x{v}"
                  f"   pages={sorted({r[0] for r in nz})}")
    print("TOTAL", dict(totals))
