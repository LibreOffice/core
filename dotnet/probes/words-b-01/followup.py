#!/usr/bin/env python3
"""Controls and slopes for what labelshape.py turned up.

    followup.py /abs/scratch/dir

Three families, all in the MZZ / one-line item / MYY geometry:

    run-F      an UNLABELLED paragraph whose own run is 12 pt face F.  This is the
               control the face rows are not allowed to fail: if our line box for a
               plain run in F is already wrong, then the label rows measured the font
               metrics and not the label, and the whole face finding dies to it.
    least-V    w:lineRule="atLeast" at V twips over 12 pt text, four values, so that
               "the extra room goes above the baseline" is a slope and not one point.
    prop-p     w:lineRule="auto" below 100% at four percentages over two text heights,
               so that "the shrunk line's ascent is 80% of its height" is a slope too.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probelib as P  # noqa: E402

FACES = ["Liberation Serif", "Liberation Sans", "Carlito", "Caladea", "OpenSymbol",
         "Liberation Mono", "IPAGothic", "WenQuanYi Zen Hei"]
LEAST = [300, 360, 480, 600]
SHRINK = [50, 60, 75, 90]


def body(*, spacing: str = "", run_family: str | None = None, points: float = 12.0,
         extra_points: float | None = None) -> str:
    half = int(round(points * 2))
    face = (f'<w:rPr><w:rFonts w:ascii="{run_family}" w:hAnsi="{run_family}"/>'
            f'<w:sz w:val="{half}"/></w:rPr>') if run_family else ""
    extra = ""
    if extra_points:
        eh = int(round(extra_points * 2))
        extra = (f'<w:r><w:rPr><w:sz w:val="{eh}"/></w:rPr>'
                 f'<w:t xml:space="preserve"> tall</w:t></w:r>')
    return ("<w:p><w:r><w:t>MZZ above</w:t></w:r></w:p>"
            f'<w:p><w:pPr>{spacing}</w:pPr>'
            f'<w:r>{face}<w:t xml:space="preserve">MAA item </w:t></w:r>{extra}</w:p>'
            "<w:p><w:r><w:t>MYY below</w:t></w:r></w:p>")


def cases():
    out = []
    for face in FACES:
        out.append((f"run-{face.replace(' ', '')}", dict(run_family=face)))
    for value in LEAST:
        out.append((f"least-{value}",
                    dict(spacing=f'<w:spacing w:line="{value}" w:lineRule="atLeast"/>')))
        out.append((f"least-{value}-tall20",
                    dict(spacing=f'<w:spacing w:line="{value}" w:lineRule="atLeast"/>',
                         extra_points=20.0)))
    for percent in SHRINK + [100]:
        line = percent * 240 // 100
        out.append((f"prop-{percent}",
                    dict(spacing=f'<w:spacing w:line="{line}" w:lineRule="auto"/>')))
        out.append((f"prop-{percent}-tall28",
                    dict(spacing=f'<w:spacing w:line="{line}" w:lineRule="auto"/>',
                         extra_points=28.0)))
    return out


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)

    built = []
    for name, kwargs in cases():
        src = work / f"{name}.docx"
        P.build(src, body(**kwargs))
        built.append(src)

    ref = P.render(built, work / "ref")
    ours = P.render_ours(built, work / "ours")

    def gaps(pdf):
        if not pdf.exists():
            return None
        m = P.marks(pdf)
        if any(k not in m for k in ("MZZ", "MAA", "MYY")):
            return None
        return (m["MAA"][0] - m["MZZ"][0], m["MYY"][0] - m["MAA"][0])

    print(f"{'case':>22} {'LO above':>9} {'LO below':>9} {'LO total':>9} "
          f"{'we above':>9} {'we below':>9} {'we total':>9} {'d above':>8} {'d total':>8}")
    for (name, _), rp, op in zip(cases(), ref, ours):
        a, b = gaps(rp), gaps(op)
        if a is None or b is None:
            print(f"{name:>22}  FAILED")
            continue
        print(f"{name:>22} {a[0]:9.2f} {a[1]:9.2f} {sum(a):9.2f} "
              f"{b[0]:9.2f} {b[1]:9.2f} {sum(b):9.2f} "
              f"{b[0] - a[0]:+8.2f} {sum(b) - sum(a):+8.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
