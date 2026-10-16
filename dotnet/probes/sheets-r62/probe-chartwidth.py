#!/usr/bin/env python3
"""How wide 26.2.4.2 draws a chart's text — read off the reference alone, our renderer never runs.

Round 60 put a chart's *vertical* metrics through `chart2`'s own 96 dpi device and left the
*advance width* on the face's unquantised metrics.  This asks the reference what it does to the
width, at nine sizes, without measuring a single absolute coordinate.

The trick that makes it scale-free: 26.2.4.2 writes a chart label as one `TJ` array with a
per-glyph adjustment, e.g.

    /F2 10.008 Tf[<1E>23<19>14<1F>7 ... ]TJ

and those adjustments are in **thousandths of the text space em**, so they are independent of the
chart frame's own scale, of the page, and of the font size the writer chose.  The drawn advance of
a glyph is therefore `Widths[code] - adj` exactly, in the same units as `Widths`, and

    drawn / natural = 1 - sum(adj) / sum(Widths[code])       over every glyph but the last

is a pure number that can be compared against a candidate law with nothing fitted.  The last glyph
of a show carries no adjustment and is excluded from both sums.

The candidate: `chart2` measures on a `VirtualDevice` at 96 dpi, so the em is instantiated at a
whole number of device pixels — `round(size_pt * 96 / 72)` — and every advance comes back scaled
by `ppem / (size_pt * 96 / 72)`.  At 10 pt that is 13/13.333 = 0.975 and at 18 pt it is 24/24 = 1.
**The law predicts a non-monotone sawtooth in the size, which nothing else does**, so a size series
either reproduces it or refutes it outright.
"""
import collections, os, re, shutil, subprocess, sys, zipfile, zlib

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx"
OUT = "/c/sandbox/workdir/scratch-r62-sheets/chartwidth"
SIZES = [600, 800, 900, 1000, 1100, 1200, 1300, 1400, 1600, 1800, 2000, 2200, 2800, 3600]


def build(sz):
    dst = os.path.join(OUT, "sz%04d.xlsx" % sz)
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "xl/charts/chart1.xml":
                x = data.decode("utf-8").replace('sz="1000"', 'sz="%d"' % sz)
                data = x.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
    return dst


def render(path, prof):
    d = os.path.join(OUT, "r-" + os.path.basename(path)[:-5])
    pdf = os.path.join(d, os.path.basename(path)[:-5] + ".pdf")
    if os.path.exists(pdf):
        return pdf                     # already rendered; a rerun must not re-render
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                    "--convert-to", "pdf", "--outdir", d, path],
                   capture_output=True, timeout=300)
    pdf = os.path.join(d, os.path.basename(path)[:-5] + ".pdf")
    return pdf if os.path.exists(pdf) else None


def fonts(pdf):
    """{resource name -> (basefont, firstchar, widths)} for the simple fonts on the page."""
    data = open(pdf, "rb").read()
    objs = {}
    for m in re.finditer(rb"(\d+) 0 obj(.{0,4000}?)endobj", data, re.S):
        objs[int(m.group(1))] = m.group(2)
    out = {}
    for num, body in objs.items():
        if b"/Type/Font" not in body or b"/Widths" not in body:
            continue
        base = re.search(rb"/BaseFont/([A-Za-z0-9+\-]+)", body).group(1).decode()
        first = int(re.search(rb"/FirstChar (\d+)", body).group(1))
        widths = [int(w) for w in
                  re.search(rb"/Widths\[([\d\s]+)\]", body).group(1).split()]
        out[num] = (base, first, widths)
    return out, data


def streams(data):
    got = []
    for m in re.finditer(rb"stream\r?\n", data):
        s = m.end()
        e = data.find(b"endstream", s)
        try:
            d = zlib.decompress(data[s:e])
        except Exception:
            continue
        if b"BT" in d:
            got.append(d)
    return got


def resources(data):
    """{/Fn -> object number} from the first page resource dictionary that names fonts."""
    out = {}
    for f, n in re.findall(rb"/(F\d+) (\d+) 0 R", data):
        out[f.decode()] = int(n)
    return out


def ratios(pdf, want_base):
    """The unkerned drawn/natural advance ratio for one face, and the sizes it was drawn at.

    A kern pair shifts one occurrence of a glyph and not the others, so the estimator takes the
    **modal** adjustment per glyph code rather than the mean: `7` before `%` reads 11 where every
    other `7` reads 14, and averaging that in would bias the ratio by the corpus's kerning rather
    than by the device.  Codes seen once are still counted — they simply have a mode of one.
    """
    fdict, data = fonts(pdf)
    res = resources(data)
    seen = {}                      # code -> [width, Counter(adj)]
    sizes = set()
    for st in streams(data):
        t = st.decode("latin-1")
        for m in re.finditer(r"/(F\d+) ([\d.]+) Tf\[(.*?)\]TJ", t, re.S):
            fname, size, body = m.group(1), float(m.group(2)), m.group(3)
            num = res.get(fname)
            if num is None or num not in fdict:
                continue
            base, first, widths = fdict[num]
            if want_base not in base:
                continue
            toks = re.findall(r"<([0-9A-Fa-f]+)>|(-?\d+)", body)
            seq = []
            for h, n in toks:
                if h:
                    for i in range(0, len(h), 2):
                        seq.append(("g", int(h[i:i + 2], 16)))
                else:
                    seq.append(("a", int(n)))
            i = 0
            while i < len(seq):
                code = seq[i][1]
                adj = 0
                if i + 1 < len(seq) and seq[i + 1][0] == "a":
                    adj = seq[i + 1][1]
                    i += 1
                i += 1
                if i >= len(seq):
                    break      # last glyph of a show carries no adjustment: it is not measurable
                w = widths[code - first] if 0 <= code - first < len(widths) else 0
                if w == 0:
                    continue
                e = seen.setdefault(code, [w, collections.Counter()])
                e[1][adj] += 1
            sizes.add(round(size, 3))
    if not seen:
        return None
    tw = ta = 0
    table = []
    for code, (w, c) in sorted(seen.items()):
        adj, _ = c.most_common(1)[0]
        n = sum(c.values())
        tw += w * n
        ta += adj * n
        table.append((code, w, adj, n, dict(c)))
    return 1.0 - ta / tw, sorted(sizes), table


def main():
    os.makedirs(OUT, exist_ok=True)
    prof = os.path.join(OUT, "prof")
    rows, failed = [], []
    for sz in SIZES:
        pdf = render(build(sz), prof)
        r = ratios(pdf, "Carlito-Regular") if pdf else None
        if r is None:
            failed.append(sz)
            print("  FAILED %d" % sz)
            continue
        rows.append((sz, r[0], r[1], r[2]))
    if failed:
        print("\nREFUSING TO SUMMARISE — %d of %d sizes produced no measurement: %s"
              % (len(failed), len(SIZES), failed), file=sys.stderr)
        sys.exit(2)
    print("\n%-7s %-8s %-9s %-11s %-9s %-9s"
          % ("stated", "drawn", "measured", "ppem/px", "predicted", "residual"))
    for sz, r, drawn, table in rows:
        # The prediction is made from the size the reference actually drew, not the size the file
        # states: `RelativeSizeHelper::adaptFontSizes` rescales a chart's text to the page
        # reference size, and at 16 pt stated the reference draws 15.888.  Using the stated size
        # would put a second, unrelated error inside the residual.
        pt = drawn[0] if drawn else sz / 100.0
        px = pt * 96.0 / 72.0
        pred = round(px) / px
        print("%-7.2f %-8.3f %-9.5f %2d/%-8.3f %-9.5f %+-9.5f"
              % (sz / 100.0, pt, r, round(px), px, pred, r - pred))
    print()
    print("per-glyph table at the shipped size, so the estimator can be audited:")
    for sz, r, drawn, table in rows:
        if sz != 1000:
            continue
        print("  %-6s %-8s %-6s %-5s %s" % ("code", "width", "adj", "n", "all adjustments"))
        for code, w, adj, n, c in table:
            print("  %-6d %-8d %-6d %-5d %s" % (code, w, adj, n, c))


if __name__ == "__main__":
    main()
