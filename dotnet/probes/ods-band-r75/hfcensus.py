#!/usr/bin/env python3
"""How many .ods declare a header or footer band that Calc's rule will grow.

    hfcensus.py <corpus-root>

Two counts, because one of them needs no assumption at all. A band whose gap alone exceeds its
declared height *must* grow, whatever its text is; a band whose declared height is under
`gap + one line` grows for any ordinary one-line header, and one line is taken as 11.5 pt —
Liberation Sans at 10 pt is 11.16 and Calibri at 11 is 13.4, so the figure is a middling one
and the count is quoted as an estimate rather than as a measurement.
"""
import os, re, sys, zipfile

LEN = re.compile(r'^\s*(-?[0-9.]+)\s*(cm|mm|in|pt|pc|px)?\s*$')
UNIT = {"cm": 28.3465, "mm": 2.83465, "in": 72.0, "pt": 1.0, "pc": 12.0, "px": 0.75}
BAND = re.compile(r"<style:(header|footer)-style>(.*?)</style:\1-style>", re.S)
ATTR = re.compile(r'\b(fo:min-height|svg:height|fo:margin-bottom|fo:margin-top)="([^"]*)"')


def points(text):
    m = LEN.match(text or "")
    return float(m.group(1)) * UNIT.get(m.group(2) or "pt", 1.0) if m else None


def main():
    root = sys.argv[1]
    line = 11.5
    docs = grows = certainly = 0
    bands = bgrows = 0
    for dirpath, _, names in os.walk(root):
        for n in sorted(names):
            if not n.lower().endswith((".ods", ".fods", ".ots")):
                continue
            path = os.path.join(dirpath, n)
            try:
                with zipfile.ZipFile(path) as z:
                    styles = z.read("styles.xml").decode("utf-8", "replace")
            except Exception:
                continue
            docs += 1
            g = c = False
            for kind, body in BAND.findall(styles):
                a = dict(ATTR.findall(body))
                if "svg:height" in a:
                    continue  # not dynamic: it cannot grow
                declared = points(a.get("fo:min-height"))
                gap = points(a.get("fo:margin-bottom" if kind == "header" else "fo:margin-top"))
                if declared is None:
                    continue
                bands += 1
                if gap is not None and gap > declared:
                    c = True
                if (gap or 0) + line > declared:
                    g = True
                    bgrows += 1
            grows += g
            certainly += c
    print(f"documents {docs}")
    print(f"  a band whose gap alone exceeds its declared height: {certainly}")
    print(f"  a band under gap + one 11.5 pt line:                {grows}")
    print(f"  dynamic bands {bands}, of them under gap + a line {bgrows}")


if __name__ == "__main__":
    main()
