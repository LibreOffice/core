"""How far this round's five plot-area defects reach across the corpus.

Counts, per chart part:
  * `overrun`  — a `c:plotArea/c:layout/c:manualLayout` whose stated rectangle does not fit the
    chart page (`x + w > 1` or `y + h > 1`), which `DiagramHelper::setDiagramPositioning`
    resolves by moving the *position*, not by shrinking the size;
  * `vertcat`  — a category axis running down the side (a `barDir val="bar"` chart), whose labels
    `ChartAxisLabels.Resolve` never arranges;
  * `valrot`   — a `c:valAx/c:txPr/a:bodyPr` stating an in-range non-zero `rot`;
  * `catrot`   — the same on the category axis, for contrast;

`valrot` and `catrot` are the round's cautionary tale. `<c:valAx>` *contains* `<c:title>`, whose
own `a:bodyPr` states the quarter turn that stands an axis title on its side — so a regex looking
for `a:bodyPr` anywhere inside the axis counts 37 value axes across 19 documents where the axis'
own tick-label properties carry two. The title is stripped before `c:txPr` is looked for, and
values outside [-5400000, 5400000] are dropped because `ObjectFormatter::convertTextRotation`
drops them (`oox/source/drawingml/chart/objectformatter.cxx`:1085-1093).
  * `insidecat`— `c:catAx` with `tickLblPos="nextTo"`, its value axis `crosses="autoZero"`, and a
    negative value somewhere in a series, so the category axis crosses inside the plot.

OOXML parts are read straight out of the zip. A file that is not a zip, or has no chart part,
contributes a zero row and is still counted, so the denominator is the corpus.
"""
import re, sys, zipfile
from pathlib import Path

CORPUS = Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")

PLOT = re.compile(rb"<c:plotArea\b.*?</c:plotArea>", re.S)
LAYOUT = re.compile(rb"<c:plotArea>\s*<c:layout>(.*?)</c:layout>", re.S)
MANUAL = re.compile(rb"<c:manualLayout>(.*?)</c:manualLayout>", re.S)
VAL = re.compile(rb"<c:valAx\b.*?</c:valAx>", re.S)
CAT = re.compile(rb"<c:(?:cat|date|ser)Ax\b.*?</c:(?:cat|date|ser)Ax>", re.S)
ROT = re.compile(rb'<a:bodyPr[^>]*\brot="(-?\d+)"')
TXPR = re.compile(rb"<c:txPr>(.*?)</c:txPr>", re.S)
TITLE = re.compile(rb"<c:title>.*?</c:title>", re.S)


def axis_rotation(axis: bytes) -> int | None:
    """The rotation an axis states for its own tick labels, or None."""
    body = TITLE.sub(b"", axis)
    inner = TXPR.search(body)
    if not inner:
        return None
    found = ROT.search(inner.group(1))
    if not found:
        return None
    turns = int(found.group(1))
    return turns if -5400000 <= turns <= 5400000 and turns != 0 else None
BARDIR = re.compile(rb'<c:barDir val="bar"\s*/?>')
LBLPOS = re.compile(rb'<c:tickLblPos val="(\w+)"\s*/?>')
CROSSES = re.compile(rb'<c:crosses val="(\w+)"\s*/?>')
NEGVAL = re.compile(rb"<c:v>-[\d.]")


def frac(blob: bytes, tag: bytes) -> float | None:
    m = re.search(rb"<c:" + tag + rb' val="([^"]+)"\s*/?>', blob)
    return float(m.group(1)) if m else None


def overruns(part: bytes) -> tuple[int, int]:
    """(parts with a manual inner layout, parts whose stated rectangle does not fit)."""
    m = LAYOUT.search(part)
    if not m:
        return 0, 0
    inner = MANUAL.search(m.group(1))
    if not inner:
        return 0, 0
    body = inner.group(1)
    x, y = frac(body, b"x"), frac(body, b"y")
    w, h = frac(body, b"w"), frac(body, b"h")
    if x is None or y is None or w is None or h is None:
        return 1, 0
    return 1, int(x + w > 1.0 or y + h > 1.0)


print("path\tcharts\tmanual\toverrun\tvertcat\tvalrot\tcatrot\tinsidecat")
for src in sorted(CORPUS.rglob("*")):
    if not src.is_file():
        continue
    rel = src.relative_to(CORPUS).as_posix()
    if "/" not in rel:
        continue
    charts = manual = overrun = vertcat = valrot = catrot = inside = 0
    try:
        with zipfile.ZipFile(src) as z:
            parts = [n for n in z.namelist() if re.search(r"charts?/chart\d*\.xml$", n)]
            for n in parts:
                blob = z.read(n)
                charts += 1
                for p in PLOT.finditer(blob):
                    body = p.group(0)
                    m, o = overruns(body)
                    manual += m
                    overrun += o
                    if BARDIR.search(body):
                        vertcat += 1
                    for v in VAL.finditer(body):
                        if axis_rotation(v.group(0)) is not None:
                            valrot += 1
                    for c in CAT.finditer(body):
                        if axis_rotation(c.group(0)) is not None:
                            catrot += 1
                    # A category axis that crosses inside the plot: nextTo labels, autoZero
                    # crossing on the value axis, and a negative value in the data.
                    cats = [c.group(0) for c in CAT.finditer(body)]
                    vals = [v.group(0) for v in VAL.finditer(body)]
                    if cats and vals and NEGVAL.search(body):
                        cl = LBLPOS.search(cats[0])
                        vc = CROSSES.search(vals[0])
                        if (cl is None or cl.group(1) == b"nextTo") and (
                            vc is None or vc.group(1) == b"autoZero"
                        ):
                            inside += 1
    except (zipfile.BadZipFile, OSError, KeyError):
        pass
    print(f"{rel}\t{charts}\t{manual}\t{overrun}\t{vertcat}\t{valrot}\t{catrot}\t{inside}")
