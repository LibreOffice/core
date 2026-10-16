#!/usr/bin/env python3
"""Re-derive the axis wrap limit against the reference, with the corrected ruler.

Round 30 bracketed the limit at [0.990, 1.056] of the tick spacing by comparing LibreOffice's
own rotation decision against *our* word widths — measured on an unquantised ruler.  Round 62
showed chart2 instantiates the em at a whole number of 96 dpi device pixels, so those widths
were 2.5% too wide at 10 pt and the fitted 1.000 is a measurement of `true / 0.975`.

Rescaling one bracket is arithmetic.  What is not arithmetic is that the correction is a
**sawtooth**: 0.975 at 10 pt, 1.023 at 11 pt, 1.031 at 8 pt, 0.981 at 13 pt.  Round 30's three
boundaries were all at 10 pt and so cannot see it.  This reads the reference's decision at six
sizes, and asks of each candidate ruler whether **one** limit fits every boundary at once.

  * a ruler that is right makes the six per-size brackets overlap;
  * a ruler that is wrong makes them disjoint, in the sawtooth's own pattern.

The reference's decision is read two ways that can disagree:

  * the depth of the bottom band in its own `chart:coordinate-region`;
  * whether the category labels are in the exported PDF's text layer at all — 26.2.4.2 draws a
    45-degree chart label as outlines and an upright one as text.

Nothing of ours runs.  The only thing taken from our side is the *font*'s advance for `W`,
1229/2048 em, read out of `LiberationMono-Regular.ttf` itself.
"""
import os, re, subprocess, sys, collections

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "..", "..", "research", "probes", "slides-r30"))
from region import regions  # noqa: E402

ADV_W = 1229 / 2048.0          # Liberation Mono, every glyph, from the font's own hmtx
DPI = 96.0                     # MetricGrid.Chart

NAME = re.compile(r"^rot-(?:z(?P<size>\d+)-n(?P<n>\d+)|n(?P<n2>\d+)c(?P<c>\d+))$")
FINE = re.compile(r"^fine-z(?P<size>\d+)-n(?P<n>\d+)-w(?P<w>\d+)$")


def scale(size_pt):
    px = size_pt * DPI / 72.0
    return round(px) / px


def variant(stem):
    f = FINE.match(stem)
    if f:
        return dict(size=int(f.group("size")) / 100.0, count=int(f.group("n")), chars=6,
                    series="fine-z%s" % f.group("size"))
    m = NAME.match(stem)
    if not m:
        return None
    if m.group("size"):
        return dict(size=int(m.group("size")) / 100.0, count=int(m.group("n")), chars=6,
                    series="z%s" % m.group("size"))
    return dict(size=10.0, count=int(m.group("n2")), chars=int(m.group("c")),
                series="n%sc" % m.group("n2"))


def text_has(pdf, token):
    out = subprocess.run(["pdftotext", "-layout", pdf, "-"],
                         capture_output=True, text=True).stdout
    return out.count(token)


def main(root):
    odp_dir, ref_dir = os.path.join(root, "odp"), os.path.join(root, "ref")
    rows = []
    for f in sorted(os.listdir(odp_dir)):
        if not f.endswith(".odp"):
            continue
        stem = f[:-4]
        v = variant(stem)
        if v is None:
            continue
        rs = regions(os.path.join(odp_dir, f))
        if len(rs) != 1:
            print("SKIP %s: %d regions" % (stem, len(rs)), file=sys.stderr)
            continue
        r = rs[0]
        bottom = r["inset"][3]
        pdf = os.path.join(ref_dir, stem + ".pdf")
        shows = text_has(pdf, "W" * v["chars"]) if os.path.exists(pdf) else -1
        rows.append(dict(stem=stem, bottom=bottom, width=r["region"][2],
                         shows=shows, **v))

    # The upright bottom band is the same for every count at one size; a rotated one is deeper
    # and grows with the label.  Classify by the text layer, and report where the two disagree.
    for r in rows:
        r["rot_text"] = r["shows"] == 0
    per_size = collections.defaultdict(list)
    for r in rows:
        per_size[r["size"]].append(r)
    for size, rs in per_size.items():
        floor = min(x["bottom"] for x in rs)
        for r in rs:
            r["rot_band"] = r["bottom"] > floor + 2.0

    print("%-16s %5s %3s %2s %8s %8s %6s %5s %5s  %s" %
          ("deck", "size", "n", "c", "region.w", "spacing", "bottom", "shows", "rot?", "agree"))
    for r in sorted(rows, key=lambda x: (x["series"], x["count"], x["chars"])):
        s = r["width"] / r["count"]
        print("%-16s %5.1f %3d %2d %8.2f %8.3f %6.2f %5d %5s  %s" %
              (r["stem"], r["size"], r["count"], r["chars"], r["width"], s, r["bottom"],
               r["shows"], r["rot_text"], "ok" if r["rot_text"] == r["rot_band"] else "DISAGREE"))

    print("\nboundaries, and the limit each implies under two rulers")
    print("%-8s %5s %2s %9s %9s | %-19s | %-19s" %
          ("series", "size", "c", "s(upright)", "s(rotated)",
           "quantised L in", "unquantised L in"))
    brackets = {}
    for series in sorted({r["series"] for r in rows}):
        rs = [r for r in rows if r["series"] == series]
        up = [r for r in rs if not r["rot_text"]]
        ro = [r for r in rs if r["rot_text"]]
        if not up or not ro:
            print("%-8s no boundary in window" % series)
            continue
        # For a count series the label is fixed and the spacing varies; for a character series
        # the spacing is fixed and the label varies.  Both reduce to: the largest width/spacing
        # ratio that stayed upright, and the smallest that turned.
        def ratio(r, q):
            w = r["chars"] * ADV_W * r["size"]
            if q:
                w *= scale(r["size"])
            return w / (r["width"] / r["count"])
        lo_q = max(ratio(r, True) for r in up)
        hi_q = min(ratio(r, True) for r in ro)
        lo_u = max(ratio(r, False) for r in up)
        hi_u = min(ratio(r, False) for r in ro)
        u = max(up, key=lambda r: ratio(r, True))
        o = min(ro, key=lambda r: ratio(r, True))
        brackets[series] = (lo_q, hi_q, lo_u, hi_u)
        print("%-8s %5.1f %2d %9.3f %9.3f | [%7.4f, %7.4f) | [%7.4f, %7.4f)" %
              (series, u["size"], u["chars"], u["width"] / u["count"], o["width"] / o["count"],
               lo_q, hi_q, lo_u, hi_u))

    for label, i in (("quantised", 0), ("unquantised", 2)):
        lo = max(b[i] for b in brackets.values())
        hi = min(b[i + 1] for b in brackets.values())
        print("\n%-12s intersection over %d series: [%.4f, %.4f)  %s"
              % (label, len(brackets), lo, hi, "EMPTY" if lo >= hi else "non-empty"))


if __name__ == "__main__":
    main(sys.argv[1])
