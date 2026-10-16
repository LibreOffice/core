#!/usr/bin/env python3
"""Escher fill types, per shape container, over a list of BIFF workbooks.

Assembles each substream's drawing exactly as `autotm.py` did — MSODRAWING records and the
CONTINUEs directly behind one, at that substream's own BOF depth, so an embedded chart's
drawing is a stream of its own — and walks it as a record tree. For each msofbtSpContainer
it reports the shape type, the group flag, `fillType` (property 384), whether `fFilled`
(447 bit 4) is stated hard and how, and whether `fillBlip` (390) is present and in what
form.

Does NOT see a shape container that arrived as a bare CONTINUE behind an OBJ or a TXO
(round 103 §1.1), so every count here is a floor.
"""
import sys, struct
import olefile

MSO, CONT, BOF, EOFR = 0x00EC, 0x003C, 0x0809, 0x000A
SP_CONTAINER, SP, OPT, TERTIARY_OPT, SECONDARY_OPT = 0xF004, 0xF00A, 0xF00B, 0xF122, 0xF121

P_FILLTYPE, P_FILLCOLOR, P_FILLBLIP, P_FILLBACKCOLOR, P_FILLOPACITY = 384, 385, 390, 387, 386
P_FILLBOOLS = 447                      # fNoFillHitTest group; fFilled is bit 4

FILLNAME = {0: "solid", 1: "pattern", 2: "texture", 3: "picture", 4: "shadeLinear",
            5: "shade", 6: "shadeShape", 7: "shadeScale", 8: "shadeTitle", 9: "background"}


def streams(path):
    ole = olefile.OleFileIO(path)
    ent = [e for e in ole.listdir() if e[-1].lower() in ("workbook", "book")]
    if not ent:
        return []
    data = ole.openstream(ent[0]).read()
    pos, recs = 0, []
    while pos + 4 <= len(data):
        rid, rlen = struct.unpack_from("<HH", data, pos)
        recs.append((pos, rid, rlen))
        pos += 4 + rlen
    out, cur, depth, prev = [], None, 0, None
    for (pos, rid, rlen) in recs:
        if rid == BOF:
            depth += 1
            cur = {"dff": bytearray(), "depth": depth}
            out.append(cur)
            prev = rid
            continue
        if rid == EOFR:
            depth -= 1
            prev = rid
            continue
        if cur is None:
            continue
        if rid == MSO and depth == cur["depth"]:
            cur["dff"] += data[pos + 4:pos + 4 + rlen]
            prev = MSO
        elif rid == CONT and prev == MSO and depth == cur["depth"]:
            cur["dff"] += data[pos + 4:pos + 4 + rlen]
        elif rid != CONT:
            prev = rid
    return [bytes(s["dff"]) for s in out if s["dff"]]


def walk(buf, start, end, depth, out):
    p = start
    while p + 8 <= end:
        vi, typ, ln = struct.unpack_from("<HHI", buf, p)
        body = p + 8
        be = min(body + ln, end)
        out.append((depth, typ, ln, body, be, vi >> 4))
        if (vi & 0xF) == 0xF:
            walk(buf, body, be, depth + 1, out)
        p = body + ln
        if ln == 0 and typ == 0:
            break
    return out


def props(buf, s, e, inst):
    """id -> (value, isBlip, isComplex) from one OPT-family record."""
    got = {}
    p = s
    for _ in range(inst):
        if p + 6 > e:
            break
        raw, val = struct.unpack_from("<HI", buf, p)
        p += 6
        got.setdefault(raw & 0x3FFF, (val, bool(raw & 0x4000), bool(raw & 0x8000)))
    return got


def main(paths):
    rows = []
    for path in paths:
        try:
            strms = streams(path)
        except Exception as exc:                                     # noqa: BLE001
            print(f"# {path}: {exc}", file=sys.stderr)
            continue
        name = path.split("/")[-1]
        for si, buf in enumerate(strms):
            recs = walk(buf, 0, len(buf), 0, [])
            for (d, t, ln, s, e, inst) in recs:
                if t != SP_CONTAINER:
                    continue
                kids = [x for x in recs if x[0] == d + 1 and s <= x[3] < e]
                pr, flags, shapetype = {}, 0, 0
                for (dd, tt, ll, ss, ee, ii) in kids:
                    if tt in (OPT, SECONDARY_OPT, TERTIARY_OPT):
                        for k, v in props(buf, ss, ee, ii).items():
                            pr.setdefault(k, v)
                    elif tt == SP and ee - ss >= 8:
                        flags = struct.unpack_from("<I", buf, ss + 4)[0]
                        shapetype = ii
                ft = pr.get(P_FILLTYPE, (0, False, False))[0]
                bools = pr.get(P_FILLBOOLS, (None, False, False))[0]
                blip = pr.get(P_FILLBLIP)
                rows.append({
                    "doc": name, "stream": si, "type": shapetype,
                    "group": bool(flags & 0x0001),
                    "fill": ft, "fillname": FILLNAME.get(ft, f"?{ft}"),
                    "statesFillType": P_FILLTYPE in pr,
                    "fFilledStated": bools is not None and bool(bools & (0x10 << 16)),
                    "fFilled": bools is not None and bool(bools & 0x10),
                    "fillColor": pr.get(P_FILLCOLOR, (None,))[0],
                    "fillBackColor": pr.get(P_FILLBACKCOLOR, (None,))[0],
                    "fillOpacity": pr.get(P_FILLOPACITY, (None,))[0],
                    "blip": None if blip is None else ("complex" if blip[2] else
                                                       ("index" if blip[1] else "plain")),
                    "blipValue": None if blip is None else blip[0],
                })

    print("doc\tstream\tshapeType\tgroup\tfillType\tstatesFillType\tfFilledStated\tfFilled"
          "\tfillColor\tfillBackColor\tfillOpacity\tblipForm\tblipValue")
    for r in rows:
        print("\t".join(str(r[k]) for k in
                        ("doc", "stream", "type", "group", "fillname", "statesFillType",
                         "fFilledStated", "fFilled", "fillColor", "fillBackColor",
                         "fillOpacity", "blip", "blipValue")))

    pat = [r for r in rows if r["fill"] in (1, 2, 3)]
    docs = sorted({r["doc"] for r in pat})
    print(f"# shape containers: {len(rows)}", file=sys.stderr)
    print(f"# stating fillType pattern/texture/picture: {len(pat)} in {len(docs)} documents",
          file=sys.stderr)
    for d in docs:
        here = [r for r in pat if r["doc"] == d]
        kinds = {}
        for r in here:
            kinds[r["fillname"]] = kinds.get(r["fillname"], 0) + 1
        print(f"#   {d}: {kinds}", file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1:])
