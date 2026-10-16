#!/usr/bin/env python3
"""Census the Escher wrap/fit properties of every .ppt in the slides corpus.

PptSlideLayout.Autofits suppresses autofit when the shape grows to its text (fFitShapeToText)
or when the text does not wrap -- and the comment beside it says the wrap half is an
approximation that errs by NOT shrinking a non-wrapping outline placeholder, that no corpus
deck holds that combination, and that it is "the first place to look if one turns up".

A blind reviewer has now found a page where our text is ~9% larger than the reference and
overruns its frame while the reference fits, which is exactly that symptom. This checks the
claim rather than believing it.
"""
import collections, glob, os, struct, sys
import olefile

WRAP_TEXT = 133          # DFF_Prop_WrapText
FIT_TEXT_TO_SHAPE = 191  # DFF_Prop_FitTextToShape
WRAP_NONE = 2
FIT_SHAPE_TO_TEXT = 2

def opt_records(data):
    """Yield every msofbtOPT property table in an Escher stream."""
    pos = 0
    n = len(data)
    while pos + 8 <= n:
        ver_inst, rectype, size = struct.unpack_from("<HHI", data, pos)
        pos += 8
        if size > n - pos:
            return
        if rectype in (0xF00B, 0xF121):        # OPT and TERTIARY OPT
            yield ver_inst >> 4, data[pos:pos + size]
        if (ver_inst & 0x0F) == 0x0F:          # container: descend
            continue
        pos += size

def props(count, body):
    out = {}
    off = 0
    for _ in range(count):
        if off + 6 > len(body):
            break
        pid, value = struct.unpack_from("<HI", body, off)
        off += 6
        out[pid & 0x3FFF] = value
    return out

rows = []
for path in sorted(glob.glob("/c/sandbox/workdir/sample-files/slides/*/ppt/*.ppt")):
    try:
        ole = olefile.OleFileIO(path)
    except Exception as exc:
        print("SKIP", os.path.basename(path), exc, file=sys.stderr); continue
    if not ole.exists("PowerPoint Document"):
        ole.close(); continue
    data = ole.openstream("PowerPoint Document").read()
    ole.close()

    wrapnone = fitshape = both = total = 0
    for count, body in opt_records(data):
        p = props(count, body)
        if WRAP_TEXT not in p and FIT_TEXT_TO_SHAPE not in p:
            continue
        total += 1
        w = p.get(WRAP_TEXT, 0) == WRAP_NONE
        f = (p.get(FIT_TEXT_TO_SHAPE, 0) & FIT_SHAPE_TO_TEXT) != 0
        wrapnone += w
        fitshape += f
        both += (w and not f)
    rows.append((os.path.basename(path), total, wrapnone, fitshape, both))

print(f"{'document':62} {'opt':>5} {'wrapNone':>9} {'fitShape':>9} {'wrapNone&!fit':>13}")
print("-" * 103)
for name, total, w, f, b in sorted(rows, key=lambda r: -r[3]):
    flag = "   <-- autofit suppressed by wrap alone" if b else ""
    print(f"{name[:61]:62} {total:5d} {w:9d} {f:9d} {b:13d}{flag}")
print(f"\ndocuments: {len(rows)}")
print(f"documents with at least one wrapNone-and-not-fitShape text shape: "
      f"{sum(1 for r in rows if r[4])}")
