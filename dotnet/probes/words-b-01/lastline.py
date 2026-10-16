#!/usr/bin/env python3
"""Does a list label feed the height proportional line spacing takes its share of?

    lastline.py /abs/scratch/dir

`pitch.py` showed the gap is applied ABOVE each line and that the paragraph's own first
line never gets one (`IsParaLine`).  A label only ever sits on line 1, so the only place
its share can show is the paragraph boundary: the last line's height is what the *next*
paragraph's upper space is built from.  For a one-line paragraph, line 1 is the last line.

That is round 47's geometry, and this re-measures it against the installed 26.2.4.2 and
against our own tree, with one control round 47 did not have:

    none      a plain 12 pt one-line paragraph                     — the flat control
    tallL     the same paragraph with an extra run at L pt         — tall *text*
    labelL    the same paragraph numbered, level size L, no run    — tall *label*

`tallL` and `labelL` produce the *same line box* (pitch.py measured both at 28 pt giving
MZZ->A 28.75 and A->B 17.25 to the hundredth).  So if the two rows separate at p > 100%,
the difference is the portion's kind and nothing else — which is the whole question.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probelib as P  # noqa: E402

LEVELS = [14.0, 20.0, 28.0]
PERCENTS = [50, 100, 150, 200]


def body(percent: int, kind: str, points: float) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    num = '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>' if kind == "label" else ""
    half = int(round(points * 2))
    tall = (f'<w:r><w:rPr><w:sz w:val="{half}"/></w:rPr>'
            f'<w:t xml:space="preserve"> tall</w:t></w:r>') if kind == "tall" else ""
    return ("<w:p><w:r><w:t>MZZ above</w:t></w:r></w:p>"
            f'<w:p><w:pPr>{num}{spacing}</w:pPr>'
            f'<w:r><w:t xml:space="preserve">MAA item </w:t></w:r>{tall}</w:p>'
            "<w:p><w:r><w:t>MYY below</w:t></w:r></w:p>")


def cases():
    out = []
    for p in PERCENTS:
        out.append((f"none-{p}", p, "none", 12.0))
        for level in LEVELS:
            out.append((f"tall{level:g}-{p}", p, "tall", level))
            out.append((f"label{level:g}-{p}", p, "label", level))
    return out


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)

    built = []
    for name, percent, kind, points in cases():
        src = work / f"{name}.docx"
        P.build(src, body(percent, kind, points), level_points=points)
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
    print(f"{'case':>12} {'side':>4} {'MZZ->MAA':>9} {'MAA->MYY':>9} {'total':>9}")
    for (name, *_), rp, op in zip(cases(), ref, ours):
        for side, pdf in (("LO", rp), ("ours", op)):
            got = gaps(pdf)
            if got is None:
                print(f"{name:>12} {side:>4}  FAILED {pdf.name}")
                continue
            rows[(name, side)] = got
            print(f"{name:>12} {side:>4} {got[0]:9.2f} {got[1]:9.2f} {sum(got):9.2f}")

    print()
    print("gap(p) - gap(100) for the same family — what the percentage is taken of:")
    print(f"{'case':>12} {'side':>4} {'MZZ->MAA':>9} {'MAA->MYY':>9} {'total':>9}")
    for (name, side), got in rows.items():
        family, p = name.rsplit("-", 1)
        base = rows.get((f"{family}-100", side))
        if base is None or p == "100":
            continue
        print(f"{name:>12} {side:>4} {got[0] - base[0]:+9.2f} {got[1] - base[1]:+9.2f} "
              f"{sum(got) - sum(base):+9.2f}")

    print()
    print("ours minus LibreOffice:")
    for name, *_ in cases():
        a, b = rows.get((name, "ours")), rows.get((name, "LO"))
        if a and b:
            print(f"{name:>12}      {a[0] - b[0]:+9.2f} {a[1] - b[1]:+9.2f} "
                  f"{sum(a) - sum(b):+9.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
