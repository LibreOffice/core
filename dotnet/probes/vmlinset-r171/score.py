#!/usr/bin/env python3
"""Reach and cost over the 131 corpus DOCX holding a VML text box.

Three renderings per document -- ours before, ours after, and 26.2.4.2 -- scored on the gate's own
two columns and, because neither can see an inset, on summed unsigned ink per page as well.  A
document whose bytes do not change is one the fix did not reach; a document whose bytes change and
whose ink falls is one it helped.
"""
import hashlib, pathlib, sys


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


def only(directory):
    return next(directory.glob("*.pdf"), None) if directory.is_dir() else None


def counts(pdf):
    import fitz
    with fitz.open(pdf) as doc:
        text = "".join(page.get_text() for page in doc)
        return doc.page_count, sum(c.isalnum() for c in text)


def ink(pdf, pages):
    """Per-page fraction of the page area covered by ink, as a list."""
    import fitz
    out = []
    with fitz.open(pdf) as doc:
        for index in range(pages):
            if index >= doc.page_count:
                out.append(0.0)
                continue
            page = doc[index]
            pixmap = page.get_pixmap(dpi=36, colorspace=fitz.csGRAY)
            data = pixmap.samples
            out.append(sum(255 - b for b in data) / (255.0 * len(data)))
    return out


def verdict(ours, reference):
    """The gate's rule: equal pages, and alphanumerics within max(2 %, 15)."""
    if ours[0] != reference[0]:
        return "pages"
    d = abs(ours[1] - reference[1])
    return "match" if not (d > reference[1] * 0.02 and d > 15) else "glyphs"


def main():
    root = pathlib.Path(sys.argv[1])
    paths = [pathlib.Path(p.strip()) for p in open(sys.argv[2]) if p.strip()]
    moved = improved = worsened = level = 0
    before_match = after_match = 0
    rows = []
    for path in paths:
        k = key(path)
        b, a, r = (only(root / leg / k) for leg in ("before", "after", "ref"))
        if not (b and a and r):
            print(f"  missing a leg\t{path.name}")
            continue
        bc, ac, rc = counts(b), counts(a), counts(r)
        bv, av = verdict(bc, rc), verdict(ac, rc)
        before_match += bv == "match"
        after_match += av == "match"
        changed = b.read_bytes() != a.read_bytes()
        bi = ai = float("nan")
        if changed:
            moved += 1
            n = rc[0]
            ri = ink(r, n)
            bi = sum(abs(x - y) for x, y in zip(ink(b, n), ri)) * 100
            ai = sum(abs(x - y) for x, y in zip(ink(a, n), ri)) * 100
            if ai < bi - 0.005:
                improved += 1
            elif ai > bi + 0.005:
                worsened += 1
            else:
                level += 1
        rows.append((changed, bc, ac, rc, bv, av, bi, ai, path.name))
    print("moved\tbPages\taPages\trPages\tbGlyphs\taGlyphs\trGlyphs\tbefore\tafter\tbInk\taInk\tdocument")
    for changed, bc, ac, rc, bv, av, bi, ai, name in rows:
        print(f"{'*' if changed else ' '}\t{bc[0]}\t{ac[0]}\t{rc[0]}\t{bc[1]}\t{ac[1]}\t{rc[1]}\t"
              f"{bv}\t{av}\t{bi:.2f}\t{ai:.2f}\t{name}")
    print(f"\n{len(rows)} documents; {moved} renderings moved")
    print(f"  of the movers: {improved} better against 26.2.4.2, {worsened} worse, {level} level")
    print(f"  gate: {before_match} match before, {after_match} after")
    total_b = sum(r[6] for r in rows if r[0])
    total_a = sum(r[7] for r in rows if r[0])
    print(f"  summed |ink| over the movers: {total_b:.2f} -> {total_a:.2f}")


if __name__ == "__main__":
    main()
