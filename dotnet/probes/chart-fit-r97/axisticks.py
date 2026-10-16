#!/usr/bin/env python3
"""Read the drawn value-axis label sets out of a PDF.

The gate's glyph column cannot score this cluster (`dotnet/CLAUDE.md`'s sixth
confound: ten of the fourteen `chartset` rows recalculate `TODAY()`), so the
instrument this round scores on is the axis itself: the *set of tick labels a
value axis draws*, read out of both renderings' text layers and compared.

A value axis' labels are a column of numeric spans that share an edge -- right
for a left-hand axis, left for a right-hand one -- and step monotonically in y
at a near-constant pitch.  That is enough to find them without knowing anything
about the chart: cluster the numeric spans by rounded x-edge, keep clusters of
three or more whose y values are strictly monotonic, and report the strings in
drawn order together with the pitch and the numeric step.

Usage:  axisticks.py FILE.pdf [--json]
"""
import json
import re
import sys

import pymupdf

NUMBER = re.compile(r'^-?[\d,]+(?:\.\d+)?%?$')


def numeric(text):
    t = text.strip()
    if not NUMBER.match(t):
        return None
    try:
        return float(t.rstrip('%').replace(',', ''))
    except ValueError:
        return None


def spans(path):
    doc = pymupdf.open(path)
    for pno, page in enumerate(doc):
        for block in page.get_text("dict")["blocks"]:
            if block["type"] != 0:
                continue
            for line in block["lines"]:
                for span in line["spans"]:
                    value = numeric(span["text"])
                    if value is None:
                        continue
                    x0, y0, x1, y1 = span["bbox"]
                    yield {
                        "page": pno, "text": span["text"].strip(), "value": value,
                        "x0": x0, "x1": x1, "y": (y0 + y1) / 2.0,
                        "height": y1 - y0, "size": span["size"],
                        "dir": line["dir"],
                    }


def axes(path, tolerance=2.0):
    """Every column of numeric spans that reads as a vertical value axis.

    `tolerance` is the width of the band an axis' labels are taken to share, in
    points, and it is single-linkage rather than a bucket: rounding to a grid
    splits neighbours that straddle a boundary, and the reference's own right
    edges wander by hundredths of a point between a one-digit label and a
    three-digit one -- which read as a whole missing tick on
    `002_advanced_excel_line`, where both renderings in fact draw the same nine.
    """
    found = []
    by_page = {}
    for s in spans(path):
        if abs(s["dir"][0] - 1.0) > 1e-6:      # a turned label is not this
            continue
        by_page.setdefault(s["page"], []).append(s)

    for page, items in sorted(by_page.items()):
        for edge in ("x1", "x0"):
            clusters = {}
            for s in sorted(items, key=lambda s: s[edge]):
                for key in clusters:
                    if abs(s[edge] - clusters[key][-1][edge]) <= tolerance:
                        clusters[key].append(s)
                        break
                else:
                    clusters[len(clusters)] = [s]
            for _, group in sorted(clusters.items()):
                if len(group) < 3:
                    continue
                group.sort(key=lambda s: s["y"])
                ys = [s["y"] for s in group]
                pitches = [b - a for a, b in zip(ys, ys[1:])]
                if min(pitches) <= 0.0:
                    continue
                if max(pitches) - min(pitches) > 0.75:   # not evenly spaced
                    continue
                values = [s["value"] for s in group]
                steps = {round(b - a, 6) for a, b in zip(values, values[1:])}
                if len(steps) != 1:                      # not an arithmetic run
                    continue
                found.append({
                    "page": page,
                    "edge": edge,
                    "x": round(group[0][edge], 2),
                    "labels": [s["text"] for s in group],
                    "values": values,
                    "step": abs(steps.pop()),
                    "pitch": round(sum(pitches) / len(pitches), 3),
                    "length": round(ys[-1] - ys[0], 3),
                    "height": round(max(s["height"] for s in group), 3),
                    "size": round(group[0]["size"], 3),
                })

    # A left-edge cluster and a right-edge cluster can be the same axis; keep
    # the one that holds the most labels at a given x-band.
    found.sort(key=lambda a: (a["page"], -len(a["labels"]), a["x"]))
    kept = []
    for axis in found:
        if any(k["page"] == axis["page"] and abs(k["x"] - axis["x"]) < 30.0
               and set(axis["labels"]) <= set(k["labels"]) for k in kept):
            continue
        kept.append(axis)
    return kept


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    as_json = "--json" in sys.argv
    out = {}
    for path in args:
        out[path] = axes(path)
    if as_json:
        print(json.dumps(out, indent=1))
        return
    for path, found in out.items():
        print(path)
        for a in found:
            print(f"  p{a['page']} x={a['x']:7.2f} step={a['step']:<8g} "
                  f"pitch={a['pitch']:7.3f} span={a['length']:7.3f} "
                  f"h={a['height']:.2f} sz={a['size']:.2f}  "
                  f"{' '.join(a['labels'])}")


if __name__ == "__main__":
    main()
