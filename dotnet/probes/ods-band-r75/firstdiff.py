#!/usr/bin/env python3
"""The first page on which two renderings stop agreeing, and by what.

    firstdiff.py <ours.pdf> <ref.pdf> [--show N]

Prints a per-page line of alphanumeric-character counts until they diverge, then the first
few spans of both sides of the page they diverge on. Page counts alone say a document is
wrong; this says *where* it went wrong, which is what decides whether the cause is on page 1
or forty pages in.
"""
import sys, pymupdf


def glyphs(page):
    return sum(1 for c in page.get_text() if c.isalnum())


def main():
    ours, ref = sys.argv[1], sys.argv[2]
    show = int(sys.argv[sys.argv.index("--show") + 1]) if "--show" in sys.argv else 8
    a, b = pymupdf.open(ours), pymupdf.open(ref)
    print(f"# ours {a.page_count} pages, ref {b.page_count}")
    first = None
    for i in range(min(a.page_count, b.page_count)):
        ga, gb = glyphs(a[i]), glyphs(b[i])
        flag = "" if ga == gb else "  <-- differs"
        if i < 40 or flag:
            print(f"page {i + 1:5d}  ours {ga:7d}  ref {gb:7d}{flag}")
        if flag and first is None:
            first = i
            break
    if first is None:
        print("# every common page agrees on its character count")
        return
    for name, doc in (("ours", a), ("ref", b)):
        print(f"--- {name} page {first + 1}")
        n = 0
        for block in doc[first].get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                for span in line["spans"]:
                    x0, y0, x1, y1 = span["bbox"]
                    print(f"  {y0:8.2f} {x0:8.2f} {x1:8.2f} {span['text'][:60]!r}")
                    n += 1
                    if n >= show:
                        break
                if n >= show:
                    break
            if n >= show:
                break


if __name__ == "__main__":
    main()
