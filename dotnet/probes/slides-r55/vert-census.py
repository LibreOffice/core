#!/usr/bin/env python3
"""How many slides documents state a vertical text direction, and where.

Counts `a:bodyPr/@vert` on the OOXML side and the Escher `txflTextFlow` property (0x0088,
id 136) on the binary side.  Deliberately reports **where** the attribute sits, because a
`vert` on a slideLayout or slideMaster placeholder is inherited by every slide shape that
resolves through it -- so a count of slide-part hits alone under-reaches, which is the trap
`HANDOVER.md` s7 names.  It still cannot see:

  * whether the shape carrying it draws any text at all;
  * `wordArtVert`, which LibreOffice renders as STACKED and not as a turn;
  * ODP's `style:writing-mode`, counted separately below.

    vert-census.py <corpus-root>
"""
import collections, glob, os, re, struct, sys, zipfile

VERT = re.compile(rb'vert="([A-Za-z0-9]+)"')
WMODE = re.compile(rb'writing-mode="([a-z\-]+)"')


def ooxml(path):
    """{(where, value): count} for one zip container."""
    out = collections.Counter()
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return out
    with z:
        for name in z.namelist():
            if not name.endswith(".xml"):
                continue
            where = ("slide" if "/slides/" in name else
                     "layout" if "slideLayout" in name else
                     "master" if "slideMaster" in name else
                     "notes" if "notes" in name else
                     "other")
            try:
                data = z.read(name)
            except Exception:
                continue
            for m in VERT.finditer(data):
                out[(where, m.group(1).decode())] += 1
            for m in WMODE.finditer(data):
                out[(where, "wm:" + m.group(1).decode())] += 1
    return out


def escher_textflow(path):
    """Count Escher opt entries with property id 136 (txflTextFlow) and their values.

    The property table is `<opt>` record 0xF00B: header (8 bytes) then nInstance
    six-byte entries, id/value.  Scanning the whole file for the record header is
    enough for a census -- a false positive needs four bytes to look like a header
    *and* to be followed by consistent entries, and the control below reports how
    many decks answer zero.
    """
    out = collections.Counter()
    data = open(path, "rb").read()
    i = 0
    n = len(data)
    while i + 8 <= n:
        ver_inst, fbt, clen = struct.unpack_from("<HHI", data, i)
        if fbt in (0xF00B, 0xF121) and clen == (ver_inst >> 4) * 6 and clen and i + 8 + clen <= n:
            for k in range(ver_inst >> 4):
                pid, val = struct.unpack_from("<HI", data, i + 8 + k * 6)
                if (pid & 0x3FFF) == 136:
                    out[val] += 1
            i += 8 + clen
            continue
        i += 1
    return out


if __name__ == "__main__":
    root = sys.argv[1] if len(sys.argv) > 1 else "/c/sandbox/workdir/sample-files"
    docs = sorted(glob.glob(os.path.join(root, "slides", "*", "*", "*")))
    grand = collections.Counter()
    perdoc = []
    for p in docs:
        ext = os.path.splitext(p)[1].lower().lstrip(".")
        if ext in ("pptx", "pptm", "potx", "ppsx", "odp", "otp", "fodp"):
            c = ooxml(p)
        elif ext in ("ppt", "pps", "pot"):
            c = collections.Counter(
                {("escher", f"txfl={k}"): v for k, v in escher_textflow(p).items()})
        else:
            continue
        interesting = {k: v for k, v in c.items()
                       if not k[1].startswith("horz")
                       and k[1] not in ("wm:lr-tb",)}
        for k, v in interesting.items():
            grand[k] += v
        if interesting:
            perdoc.append((sum(interesting.values()), os.path.basename(p), dict(interesting)))

    perdoc.sort(reverse=True)
    for total, name, d in perdoc:
        print(f"{total:6d}  {name}")
        for k in sorted(d):
            print(f"          {k[0]:8s} {k[1]:16s} {d[k]}")
    print(f"\ndocuments stating a non-horizontal text direction: {len(perdoc)} of {len(docs)}")
    for k in sorted(grand, key=lambda k: -grand[k]):
        print(f"  {k[0]:8s} {k[1]:16s} {grand[k]}")
