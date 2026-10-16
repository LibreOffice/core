#!/usr/bin/env python3
"""One DOCX per (c:rotX, c:depthPercent), everything else left alone.

    ./variants.py <source.docx> <outdir>

The source is a real corpus document holding a `c:pie3DChart`, so the chart, its data, its
labels and the page around it are a writer's own and not an author's guess. Only the two
attributes under test are rewritten, in the chart part alone.
"""
import pathlib
import re
import sys
import zipfile

ROT = [10, 20, 30, 40, 50, 60, 70, 80, 90]
DEPTH = [20, 50, 100, 200]


def main() -> int:
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)

    # Read every entry up front. Reading from an open ZipFile while writing another one
    # corrupts the output -- "Bad magic number" -- and the failure is silent until the reader
    # chokes on it.
    with zipfile.ZipFile(src) as package:
        entries = [(i, package.read(i.filename)) for i in package.infolist()]

    chart = [i for i, (info, _) in enumerate(entries)
             if "/charts/chart" in info.filename and info.filename.endswith(".xml")]
    if not chart:
        print("no chart part in %s" % src, file=sys.stderr)
        return 1
    at = chart[0]

    for rot in ROT:
        for depth in DEPTH:
            body = entries[at][1]
            body = re.sub(rb'<c:rotX val="[^"]*"', b'<c:rotX val="%d"' % rot, body, count=1)
            body = re.sub(rb'<c:depthPercent val="[^"]*"',
                          b'<c:depthPercent val="%d"' % depth, body, count=1)
            name = out / ("rot%02d-depth%03d.docx" % (rot, depth))
            with zipfile.ZipFile(name, "w", zipfile.ZIP_DEFLATED) as w:
                for i, (info, data) in enumerate(entries):
                    w.writestr(info, body if i == at else data)

    print("%d variants in %s" % (len(ROT) * len(DEPTH), out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
