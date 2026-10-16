"""The fixed-inner-size half of the vertical-axis probe.

`mkprobe2.py` builds a bar chart whose plot rectangle is computed, and on such a chart the plot
gives up exactly the width the widest category label needs — so the wrap limit is its own fixed
point and never binds. This adds a `c:plotArea/c:layout/c:manualLayout` with
`layoutTarget="inner"`, which makes `CreateShapeParam2D::mbUseFixedInnerSize` true and pins the
rectangle, and *then* the labels break.

Three label shapes are built at the same fixed rectangle to test what turns line breaking off:
plain words, a run whose second line starts with a hyphen, and one whose last line starts with a
bracket. All three wrap, so `lcl_hasWordBreak`'s `nWordStart != nLineStart` does not fire on
punctuation beginning a line.

    mkprobe3.py <out-dir>
"""
import re
import sys
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from mkprobe2 import BASE, cat_block, val_block  # noqa: E402

LAYOUT = ('<c:layout><c:manualLayout><c:layoutTarget val="inner"/><c:xMode val="edge"/>'
          '<c:yMode val="edge"/><c:x val="{x}"/><c:y val="0.05"/><c:w val="{w}"/>'
          '<c:h val="0.85"/></c:manualLayout></c:layout>')


def build(out: Path, labels, x="0.10", w="0.85"):
    """One bar chart with a fixed inner plot rectangle and the given category names."""
    n = len(labels)
    out.parent.mkdir(parents=True, exist_ok=True)
    zin = zipfile.ZipFile(BASE)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                s = data.decode("utf-8")
                s = re.sub(r"<c:cat>.*?</c:cat>", lambda m: cat_block(labels), s, flags=re.S)
                s = re.sub(r"<c:val>.*?</c:val>", lambda m: val_block(n), s, flags=re.S)
                s = re.sub(r"<c:ser>(?:(?!</c:ser>).)*</c:ser>\s*(?=<c:ser>)", "", s, flags=re.S)
                # The base deck writes an empty <c:layout/>; a second one is ignored, so the
                # existing element has to be replaced rather than preceded.
                s = s.replace("<c:plotArea><c:layout/>",
                              "<c:plotArea>" + LAYOUT.format(x=x, w=w), 1)
                data = s.encode("utf-8")
            zout.writestr(item, data)
    zin.close()


LONG = "Alpha Bravo Charlie Delta Echo Foxtrot"

if __name__ == "__main__":
    out = Path(sys.argv[1])
    for n in (8, 16, 32):
        build(out / f"M{n}.pptx", [f"{LONG} {i:02d}" for i in range(n)])
    build(out / "PLAIN.pptx", [f"Alpha Bravo Charlie Delta {i:02d}" for i in range(8)])
    build(out / "HYPH.pptx", [f"Alpha Bravo - Charlie Delta {i:02d}" for i in range(8)])
    build(out / "BRACK.pptx", [f"Alpha Bravo Charlie Delta [{i:04d}]" for i in range(8)])
    # x + w = 1.15: the overrun that DiagramHelper::setDiagramPositioning corrects by moving x.
    build(out / "OVER.pptx", [f"C{i:02d}" for i in range(8)], x="0.30", w="0.85")
    print(f"built into {out}")
