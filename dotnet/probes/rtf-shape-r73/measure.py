#!/usr/bin/env python3
"""Read the shape's own word out of both renderings and report the pairs that disagree.

    python3 measure.py <outdir>

`ZZQ` is the shape's text in every family but `wrap-*`, where the body paragraph's first line
is what separates the five wraps; both are printed.
"""
import sys, pathlib, pymupdf


def words(path):
    doc = pymupdf.open(path)
    out = {}
    for page in doc:
        for w in page.get_text("words"):
            out.setdefault(w[4], (round(w[0], 2), round(w[1], 2)))
    return out, doc.page_count


def first_line(path):
    doc = pymupdf.open(path)
    lines = []
    for page in doc:
        for b in page.get_text("dict")["blocks"]:
            for line in b.get("lines", []):
                text = "".join(s["text"] for s in line["spans"]).strip()
                if text.startswith("Alpha"):
                    lines.append((round(line["bbox"][0], 1), round(line["bbox"][1], 1)))
    return lines[0] if lines else None


def main():
    out = pathlib.Path(sys.argv[1])
    bad = 0
    for pdf in sorted((out / "ref").glob("*.pdf")):
        name = pdf.stem
        ours = out / "ours" / pdf.name
        rw, rp = words(pdf)
        ow, op = words(ours)
        r, o = rw.get("ZZQ"), ow.get("ZZQ")
        extra = ""
        if name.startswith("wrap-"):
            extra = f"  prose ref={first_line(pdf)} ours={first_line(ours)}"
        agree = r and o and abs(r[0] - o[0]) < 0.5 and abs(r[1] - o[1]) < 0.5
        if not agree:
            bad += 1
        print(f"{'OK ' if agree else 'DIFF'} {name:22s} pages {rp}/{op} ZZQ ref={r} ours={o}{extra}")
    print(f"disagreeing: {bad}")


if __name__ == "__main__":
    main()
