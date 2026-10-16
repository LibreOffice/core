#!/usr/bin/env python3
"""The three things round 47 could not separate, and named as blind spots.

    labelshape.py /abs/scratch/dir

Same three-paragraph geometry as `lastline.py` — MZZ / one-line item / MYY — so the two
readings are the raw line box (MZZ->MAA) and the share proportional spacing takes
(MAA->MYY, at p = 200%).

    face-F     the level names face F at the *same* 12 pt as the item.  Round 47's rows
               all scaled one family, so a label taller through its FACE was untested and
               was exactly the case its one moving document turned out to be.
    sym-F      the level's text is a private-use slot in a symbol face, which Linux does
               not have and LibreOffice recodes into OpenSymbol.  The bullet case.
    comp       a 30 pt SUBSCRIPT run in the item, which pushes the line's descent far
               below anything the label has, beside a 40 pt label.  This separates
                   base = the tallest single portion's own box        -> +45.98
                   base = max(ascent) + max(descent) over portions    -> more than that
               which every one-family row makes identical by construction.
    rule-R-V   w:lineRule atLeast/exact at V twips, with and without a 28 pt label — the
               two branches nothing on this track has ever measured with a label on them.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probelib as P  # noqa: E402

FACES = ["Liberation Serif", "Liberation Sans", "Carlito", "Caladea", "OpenSymbol",
         "Liberation Mono", "IPAGothic", "WenQuanYi Zen Hei"]
SYMBOL_FACES = ["Symbol", "Wingdings", "OpenSymbol", "Courier New"]
PUA = ""


def body(percent: int, *, labelled: bool, subscript: bool = False,
         rule: str = "auto", line: int | None = None) -> str:
    if rule == "auto":
        value = percent * 240 // 100
        spacing = f'<w:spacing w:line="{value}" w:lineRule="auto"/>'
    else:
        spacing = f'<w:spacing w:line="{line}" w:lineRule="{rule}"/>'
    num = '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>' if labelled else ""
    sub = ('<w:r><w:rPr><w:sz w:val="60"/><w:vertAlign w:val="subscript"/></w:rPr>'
           '<w:t xml:space="preserve"> deep</w:t></w:r>') if subscript else ""
    return ("<w:p><w:r><w:t>MZZ above</w:t></w:r></w:p>"
            f'<w:p><w:pPr>{num}{spacing}</w:pPr>'
            f'<w:r><w:t xml:space="preserve">MAA item </w:t></w:r>{sub}</w:p>'
            "<w:p><w:r><w:t>MYY below</w:t></w:r></w:p>")


def cases():
    out = []
    for p in (100, 200):
        out.append((f"plain-{p}", dict(percent=p, labelled=False), {}))
        for face in FACES:
            out.append((f"face-{face.replace(' ', '')}-{p}",
                        dict(percent=p, labelled=True),
                        dict(level_points=12.0, level_family=face)))
        for face in SYMBOL_FACES:
            out.append((f"sym-{face.replace(' ', '')}-{p}",
                        dict(percent=p, labelled=True),
                        dict(level_points=12.0, level_family=face, level_text=PUA)))
        # the composition family
        out.append((f"comp-none-{p}", dict(percent=p, labelled=False, subscript=True), {}))
        out.append((f"comp-label-{p}", dict(percent=p, labelled=True, subscript=True),
                    dict(level_points=40.0)))
        out.append((f"comp-plainlabel-{p}", dict(percent=p, labelled=True),
                    dict(level_points=40.0)))
    for rule, line in (("atLeast", 480), ("atLeast", 240), ("exact", 480), ("exact", 240)):
        for labelled in (False, True):
            tag = "label" if labelled else "none"
            out.append((f"rule-{rule}{line}-{tag}",
                        dict(percent=100, labelled=labelled, rule=rule, line=line),
                        dict(level_points=28.0)))
    return out


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)

    built = []
    for name, kwargs, buildargs in cases():
        src = work / f"{name}.docx"
        P.build(src, body(**kwargs), **buildargs)
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

    rows: dict[tuple[str, str], tuple[float, float]] = {}
    print(f"{'case':>28} {'LO above':>9} {'LO below':>9} {'we above':>9} {'we below':>9} "
          f"{'d above':>8} {'d below':>8}")
    for (name, *_), rp, op in zip(cases(), ref, ours):
        a, b = gaps(rp), gaps(op)
        if a is None or b is None:
            print(f"{name:>28}  FAILED  ref={a is not None} ours={b is not None}")
            continue
        rows[(name, "LO")], rows[(name, "ours")] = a, b
        print(f"{name:>28} {a[0]:9.2f} {a[1]:9.2f} {b[0]:9.2f} {b[1]:9.2f} "
              f"{b[0] - a[0]:+8.2f} {b[1] - a[1]:+8.2f}")

    print()
    print("the share proportional spacing took — below(200) - below(100), per family:")
    print(f"{'case':>28} {'LO':>9} {'ours':>9} {'ours-LO':>9}")
    for name, *_ in cases():
        if not name.endswith("-200"):
            continue
        stem = name[:-4]
        for side in ("LO",):
            hi, lo = rows.get((name, side)), rows.get((f"{stem}-100", side))
            hi2 = rows.get((name, "ours"))
            lo2 = rows.get((f"{stem}-100", "ours"))
            if hi and lo and hi2 and lo2:
                a, b = hi[1] - lo[1], hi2[1] - lo2[1]
                print(f"{stem:>28} {a:9.2f} {b:9.2f} {b - a:+9.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
