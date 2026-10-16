#!/usr/bin/env python3
"""How many cells a colorScale rule would actually paint, resolved rather than declared.

`sqref` is what the part *declares* — one rule in `036_Simple_to-do_list` declares
N18:Q1048576 — and what it *resolves to* is the numeric cells inside it, because
`ScColorScaleFormat::GetColor` returns nothing for a cell that is not numeric
(`sc/source/core/data/colorscale.cxx:679`, `if(!rCell.hasNumeric()) return {}`), which
the authored fixture `08-text-in-range` confirms at 0 fills of 11 cells.

It also predicts the exact colours, by the law measured in `probe-colorscale.py`, and
looks for them in the stored reference and our own renderings — so the census is
checkable against the page rather than only against the XML.

Refuses to report unless every input produced output.
"""
import collections, os, re, subprocess, sys, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
ROOT = "/c/sandbox/workdir/wt-sheets-r50"
PDFOPS = ROOT + "/.claude/skills/render-comparison/scripts/pdf-ops.py"
SWEEP = sys.argv[1] if len(sys.argv) > 1 else None

CELLRE = re.compile(r"^\$?([A-Za-z]{1,3})\$?(\d{1,7})$")


def colnum(s):
    n = 0
    for ch in s.upper():
        n = n * 26 + (ord(ch) - 64)
    return n


def ranges(sqref):
    for part in sqref.split():
        if ":" in part:
            a, b = part.split(":", 1)
            ma, mb = CELLRE.match(a), CELLRE.match(b)
            if ma and mb:
                yield (min(colnum(ma.group(1)), colnum(mb.group(1))),
                       min(int(ma.group(2)), int(mb.group(2))),
                       max(colnum(ma.group(1)), colnum(mb.group(1))),
                       max(int(ma.group(2)), int(mb.group(2))))
        else:
            m = CELLRE.match(part)
            if m:
                c, r = colnum(m.group(1)), int(m.group(2))
                yield (c, r, c, r)


def sheet_values(root):
    """{(col,row): float} for every numeric cell."""
    vals = {}
    data = root.find(NS + "sheetData")
    if data is None:
        return vals
    for row in data.findall(NS + "row"):
        for c in row.findall(NS + "c"):
            t = c.get("t")
            if t in ("s", "str", "inlineStr", "e", "b"):
                continue
            v = c.find(NS + "v")
            if v is None or v.text is None:
                continue
            try:
                vals[CELLREF(c.get("r"))] = float(v.text)
            except (TypeError, ValueError):
                continue
    return vals


def CELLREF(ref):
    m = CELLRE.match(ref or "")
    return (colnum(m.group(1)), int(m.group(2))) if m else (0, 0)


def percentile(sorted_vals, p):
    if not sorted_vals:
        return None
    p = min(1.0, max(0.0, p))
    n = len(sorted_vals)
    f = p * (n - 1)
    i = int(f)
    d = f - i
    if d == 0.0 or i == n - 1:
        return sorted_vals[i]
    return sorted_vals[i] + d * (sorted_vals[i + 1] - sorted_vals[i])


def cfvo_value(kind, val, lo, hi, sorted_vals):
    if kind == "min":
        return lo
    if kind == "max":
        return hi
    if kind == "percent":
        return lo + (hi - lo) * (val / 100.0)
    if kind == "percentile":
        return sorted_vals[0] if len(sorted_vals) == 1 else percentile(sorted_vals, val / 100.0)
    return val          # num, formula-as-literal


def chan(v, v1, c1, v2, c2):
    if v <= v1:
        return c1
    if v >= v2:
        return c2
    return int((v - v1) / (v2 - v1) * (c2 - c1)) + c1


def interp(v, stops):
    """stops: [(value, (r,g,b))] in order."""
    v1, c1 = stops[0]
    v2, c2 = stops[1]
    i = 2
    while i < len(stops) and v > v2:
        v1, c1 = v2, c2
        v2, c2 = stops[i]
        i += 1
    return tuple(chan(v, v1, c1[k], v2, c2[k]) for k in range(3))


def theme_colours(z):
    for n in z.namelist():
        if n.lower().endswith("theme/theme1.xml"):
            r = ET.fromstring(z.read(n))
            d = "{http://schemas.openxmlformats.org/drawingml/2006/main}"
            sch = r.find(d + "themeElements/" + d + "clrScheme")
            out = {}
            if sch is not None:
                for el in sch:
                    tag = el.tag.split("}")[1]
                    s = el.find(d + "srgbClr")
                    y = el.find(d + "sysClr")
                    if s is not None:
                        out[tag] = s.get("val")
                    elif y is not None:
                        out[tag] = y.get("lastClr")
            slots = ["lt1", "dk1", "lt2", "dk2", "accent1", "accent2", "accent3",
                     "accent4", "accent5", "accent6", "hlink", "folHlink"]
            return [out.get(s) for s in slots]
    return [None] * 12


def rgb_of(el, theme):
    if el.get("rgb"):
        s = el.get("rgb")
        s = s[-6:]
        return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4))
    if el.get("theme") is not None:
        t = theme[int(el.get("theme"))] if int(el.get("theme")) < len(theme) else None
        if not t:
            return None
        base = tuple(int(t[i:i + 2], 16) for i in (0, 2, 4))
        tint = float(el.get("tint") or 0)
        if abs(tint) < 1e-4:
            return base
        return None       # tinted: our XlsxTint owns that transform, not this census
    return None


docs, errors = [], {}
paths = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        if f[0] == "sheets" and f[3] in ("xlsx", "xlsm"):
            paths.append((f[2], f[7]))

for path, status in paths:
    full = os.path.join(CORPUS, path)
    try:
        with zipfile.ZipFile(full) as z:
            theme = theme_colours(z)
            per = {"path": path, "status": status, "cells": 0, "colours": collections.Counter(),
                   "rules": 0, "untinted": True, "sheets": 0}
            for n in z.namelist():
                if "/worksheets/" not in n.lower() or not n.lower().endswith(".xml"):
                    continue
                raw = z.read(n)
                if b"colorScale" not in raw:
                    continue
                root = ET.fromstring(raw)
                vals = sheet_values(root)
                for cf in root.iter(NS + "conditionalFormatting"):
                    for rule in cf.findall(NS + "cfRule"):
                        if rule.get("type") != "colorScale":
                            continue
                        cs = rule.find(NS + "colorScale")
                        if cs is None:
                            continue
                        per["rules"] += 1
                        cfvos = cs.findall(NS + "cfvo")
                        colours = [rgb_of(c, theme) for c in cs.findall(NS + "color")]
                        inrange = []
                        for c1, r1, c2, r2 in ranges(cf.get("sqref", "")):
                            for (c, r), v in vals.items():
                                if c1 <= c <= c2 and r1 <= r <= r2:
                                    inrange.append(v)
                        if not inrange or any(x is None for x in colours) \
                                or len(colours) != len(cfvos):
                            if any(x is None for x in colours):
                                per["untinted"] = False
                            per["cells"] += len(inrange)
                            continue
                        srt = sorted(inrange)
                        lo, hi = srt[0], srt[-1]
                        stops = []
                        for cv, col in zip(cfvos, colours):
                            sv = cfvo_value(cv.get("type"), float(cv.get("val") or 0),
                                            lo, hi, srt)
                            stops.append((sv, col))
                        for v in inrange:
                            per["cells"] += 1
                            per["colours"]["#%02X%02X%02X" % interp(v, stops)] += 1
            docs.append(per)
    except Exception as exc:                          # noqa: BLE001
        errors[path] = repr(exc)

if errors:
    print("REFUSING TO REPORT — %d of %d inputs failed:" % (len(errors), len(paths)),
          file=sys.stderr)
    for k, v in sorted(errors.items())[:10]:
        print("  ", k, v, file=sys.stderr)
    sys.exit(2)

assert len(docs) == len(paths)
hit = [d for d in docs if d["rules"]]
print("inputs: %d xlsx-family manifest rows, %d produced output, 0 failures" % (len(paths), len(docs)))
print("documents with a colorScale rule: %d;  rules %d;  cells that RESOLVE to a fill: %d"
      % (len(hit), sum(d["rules"] for d in hit), sum(d["cells"] for d in hit)))
print("documents whose predicted colours are all computable: %d"
      % sum(1 for d in hit if d["untinted"]))

if SWEEP:
    def fills(pdf):
        if not os.path.exists(pdf):
            return None
        out = subprocess.run([sys.executable, PDFOPS, "dump", pdf],
                             capture_output=True, text=True).stdout
        return collections.Counter(m.group(1) for m in
                                   re.finditer(r"^fill .*(#[0-9A-F]{6})", out, re.M))
    print("\n%-58s %5s %5s %6s %6s" % ("document", "cells", "cols", "in ref", "in ours"))
    nopdf = []
    for d in sorted(hit, key=lambda d: -d["cells"]):
        stem = os.path.basename(d["path"]).rsplit(".", 1)
        ident = "%s__%s" % (stem[0], stem[1])
        rf = fills(os.path.join(SWEEP, "ref", ident + ".pdf"))
        of = fills(os.path.join(SWEEP, "ours", ident + ".pdf"))
        if rf is None or of is None:
            nopdf.append(d["path"])
            continue
        want = set(d["colours"])
        print("%-58s %5d %5d %6d %6d"
              % (os.path.basename(d["path"])[:58], d["cells"], len(want),
                 len(want & set(rf)), len(want & set(of))))
    if nopdf:
        print("\nno rendering for %d documents (NOT scored): %s" % (len(nopdf), nopdf[:5]))
