#!/usr/bin/env python3
"""Where does proportional line spacing put its gap, and does a list label feed it?

    pitch.py /abs/scratch/dir

One paragraph of five lines, broken by explicit <w:br/> so the line composition is fixed
rather than negotiated, at p in {100, 150, 200}%.  Every line begins with a 12 pt marker
word, so a pitch is the difference of two boxes of identical metrics and no font
substitution can be mistaken for a spacing change.

Five families, varying one thing at a time:

    flat     nothing on any line is taller than 12 pt          — the control
    tall1    a 28 pt word on line 1
    tall3    a 28 pt word on line 3
    tall5    a 28 pt word on line 5
    labelL   the paragraph is numbered, level size L, no tall word anywhere

The two models the readings separate:

    gap ABOVE the line   pitch(n-1 -> n) grows by (p-100)% x base(line n)
    gap BELOW the line   pitch(n -> n+1) grows by (p-100)% x base(line n)

With the tall word on line 3 those put a 32 pt difference in two different pitches, so a
single render decides it.  The label rows then ask the round's own question: does the
label enter base(line 1) at all?
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probelib as P  # noqa: E402

MARKS = ["MAA", "MBB", "MCC", "MDD", "MEE"]
PERCENTS = [100, 150, 200]
LEVELS = [12.0, 20.0, 28.0]
TALL_POINTS = 28.0


def body(percent: int, tall_line: int | None, labelled: bool) -> str:
    spacing = f'<w:spacing w:line="{percent * 240 // 100}" w:lineRule="auto"/>'
    num = '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>' if labelled else ""
    half = int(round(TALL_POINTS * 2))
    lines = []
    for index, mark in enumerate(MARKS, start=1):
        tall = (f'<w:r><w:rPr><w:sz w:val="{half}"/></w:rPr>'
                f'<w:t xml:space="preserve"> tall</w:t></w:r>') if index == tall_line else ""
        brk = "<w:r><w:br/></w:r>" if index < len(MARKS) else ""
        lines.append(f'<w:r><w:t xml:space="preserve">{mark} line </w:t></w:r>{tall}{brk}')
    return ("<w:p><w:r><w:t>MZZ above</w:t></w:r></w:p>"
            f'<w:p><w:pPr>{num}{spacing}</w:pPr>{"".join(lines)}</w:p>'
            "<w:p><w:r><w:t>MYY below</w:t></w:r></w:p>")


def cases() -> list[tuple[str, int, int | None, bool, float]]:
    out: list[tuple[str, int, int | None, bool, float]] = []
    for p in PERCENTS:
        out.append((f"flat-{p}", p, None, False, 12.0))
        for line in (1, 3, 5):
            out.append((f"tall{line}-{p}", p, line, False, 12.0))
        for level in LEVELS:
            out.append((f"label{level:g}-{p}", p, None, True, level))
    return out


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)

    built = []
    for name, percent, tall, labelled, level in cases():
        src = work / f"{name}.docx"
        P.build(src, body(percent, tall, labelled), level_points=level)
        built.append(src)

    ref = P.render(built, work / "ref")
    ours = P.render_ours(built, work / "ours")

    order = ["MZZ"] + MARKS + ["MYY"]

    def pitches(pdf):
        if not pdf.exists():
            return None
        m = P.marks(pdf)
        if any(k not in m for k in order):
            return None
        tops = [m[k][0] for k in order]
        return [tops[i + 1] - tops[i] for i in range(len(tops) - 1)]

    head = (f"{'case':>12} {'side':>4} {'MZZ->A':>7} {'A->B':>7} {'B->C':>7} "
            f"{'C->D':>7} {'D->E':>7} {'E->MYY':>7}")
    print(head)
    rows: dict[tuple[str, str], list[float]] = {}
    for (name, *_), rp, op in zip(cases(), ref, ours):
        for side, pdf in (("LO", rp), ("ours", op)):
            got = pitches(pdf)
            if got is None:
                print(f"{name:>12} {side:>4}  FAILED/MISSING {pdf.name}")
                continue
            rows[(name, side)] = got
            print(f"{name:>12} {side:>4} " + " ".join(f"{v:7.2f}" for v in got))

    print()
    print("difference from the same family's 100% row — where the gap went:")
    print(head)
    for (name, side), got in rows.items():
        family, p = name.rsplit("-", 1)
        base = rows.get((f"{family}-100", side))
        if base is None or p == "100":
            continue
        print(f"{name:>12} {side:>4} " + " ".join(f"{v - b:+7.2f}" for v, b in zip(got, base)))

    print()
    print("ours minus LibreOffice, per pitch:")
    print(head)
    for name, *_ in cases():
        a, b = rows.get((name, "ours")), rows.get((name, "LO"))
        if a and b:
            print(f"{name:>12} {'d':>4} " + " ".join(f"{x - y:+7.2f}" for x, y in zip(a, b)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
