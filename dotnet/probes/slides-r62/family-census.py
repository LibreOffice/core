#!/usr/bin/env python3
"""Per-object chart text faces against the one face `FamilyOf` currently gives the whole chart.

`DrawingChartPlot.FamilyOf` is chartSpace/c:txPr's literal a:latin, then the first literal
a:latin *anywhere in the part*, then the theme's minor Latin face.  The second term is what
leaks an axis' stated face onto a legend that states none: 001_advanced_powerpoint_bar states
Arial on both c:catAx/c:txPr and c:valAx/c:txPr and nothing on c:legend, and 26.2.4.2 draws its
axis labels in LiberationSans (Arial) and its legend in Carlito (the theme's Calibri).

Prints one row per chart part: the current answer, and the per-object answers for legend,
axis labels, data labels, axis titles and the main title, under the rule
    object's own c:txPr  ->  chartSpace/c:txPr  ->  theme minor
"""
import os, re, sys, zipfile
import xml.etree.ElementTree as ET

C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"
REL = "{http://schemas.openxmlformats.org/package/2006/relationships}"


def face(el):
    """The first literal a:latin/@typeface under el: run properties first, then any descendant."""
    if el is None:
        return None
    for rpr in list(el.iter(A + "defRPr")) + list(el.iter(A + "rPr")) + list(el.iter(A + "endParaRPr")):
        lat = rpr.find(A + "latin")
        if lat is not None:
            t = (lat.get("typeface") or "").strip()
            if t and t[0] != "+":
                return t
    for lat in el.iter(A + "latin"):
        t = (lat.get("typeface") or "").strip()
        if t and t[0] != "+":
            return t
    return None


def child(el, name):
    return None if el is None else el.find(C + name)


def theme_minor(z, chartname):
    """The theme's minor Latin face for the part that owns this chart, best effort."""
    for n in z.namelist():
        if re.match(r"(ppt|word|xl)/theme/theme1\.xml$", n):
            r = ET.fromstring(z.read(n))
            mn = r.find(f".//{A}fontScheme/{A}minorFont/{A}latin")
            if mn is not None:
                return (mn.get("typeface") or "").strip() or None
    return None


def axes(plot):
    return [e for e in plot if e.tag.startswith(C) and e.tag.endswith("Ax")]


def groups(plot):
    return [e for e in plot if e.tag.startswith(C) and e.tag.endswith("Chart")]


def rowfor(z, name):
    root = ET.fromstring(z.read(name))
    chart = child(root, "chart")
    plot = child(chart, "plotArea")
    if plot is None:
        return None
    space_txpr = face(child(root, "txPr"))
    anywhere = face(root)
    theme = theme_minor(z, name)
    current = space_txpr or anywhere or theme

    def under(el):
        return face(child(el, "txPr"))

    legend = under(child(chart, "legend"))
    labels = next((f for ax in axes(plot) if (f := under(ax))), None)
    axtitle = next((f for ax in axes(plot) if (f := face(child(ax, "title")))), None)
    dlbl = face(child(plot, "dLbls")) or next((f for g in groups(plot) if (f := face(child(g, "dLbls")))), None)
    title = face(child(chart, "title"))

    def resolve(x):
        return x or space_txpr or theme

    return dict(part=name, spaceTxPr=space_txpr, anywhere=anywhere, theme=theme,
                current=current,
                legend=resolve(legend), labels=resolve(labels),
                dlbl=resolve(dlbl), axtitle=resolve(axtitle), title=resolve(title),
                legendStated=legend, labelsStated=labels)


if __name__ == "__main__":
    root = sys.argv[1]
    print("doc\tpart\tcurrent\tlegend\tlabels\tdlbl\taxtitle\ttitle\tlegendDiffers\tlabelsDiffers")
    for dirpath, _, names in os.walk(root):
        for n in sorted(names):
            if not n.lower().endswith((".pptx", ".pptm", ".potx", ".ppsx",
                                       ".xlsx", ".xlsm", ".xltx",
                                       ".docx", ".docm", ".dotx")):
                continue
            p = os.path.join(dirpath, n)
            try:
                z = zipfile.ZipFile(p)
            except Exception:
                continue
            for member in sorted(z.namelist()):
                if not re.match(r"(ppt|word|xl)/charts/chart\d+\.xml$", member):
                    continue
                try:
                    r = rowfor(z, member)
                except Exception as e:
                    print(f"{n}\t{member}\tERROR\t{e}")
                    continue
                if r is None:
                    continue
                print("\t".join([n, member, str(r["current"]), str(r["legend"]), str(r["labels"]),
                                 str(r["dlbl"]), str(r["axtitle"]), str(r["title"]),
                                 "Y" if r["legend"] != r["current"] else "",
                                 "Y" if r["labels"] != r["current"] else ""]))
