#!/usr/bin/env python3
"""Cross-track census: which charts would LibreOffice give an automatic title that we draw none for?

Transcribes ChartSpaceConverter::convertFromModel (oox/source/drawingml/chart/chartspaceconverter.cxx
:177-208) together with PlotAreaConverter/AxesSetConverter::convertFromModel
(plotareaconverter.cxx:170-176, 465-495) and TypeGroupConverter::getSingleSeriesTitle
(typegroupconverter.cxx:272-289), and reports per chart part:

  enter        !autoTitleDeleted or a <c:title> exists at all
  own          the title model already carries text (rich, txPr paragraphs or a c:tx cache)
                -> we already draw it; nothing to gain
  auto         the single-series title the reference would substitute
  literal      the reference substitutes the localized STR_DIAGRAM_TITLE ("Chart Title")

The one thing this census CANNOT see is stated in prediction.md.
"""
import csv, os, re, sys, zipfile
import xml.etree.ElementTree as ET

C = "http://schemas.openxmlformats.org/drawingml/2006/chart"
A = "http://schemas.openxmlformats.org/drawingml/2006/main"
EXTPR = "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"

def q(ns, name): return "{%s}%s" % (ns, name)
def kid(el, name):
    if el is None: return None
    return el.find(q(C, name))
def kids(el, name):
    if el is None: return []
    return el.findall(q(C, name))
def boolval(el, default):
    if el is None: return default
    v = el.get("val")
    if v is None: return True          # <x/> with no @val is true
    return v not in ("0", "false")

# TypeGroupConverter's type table: which groups show exactly one series regardless of how many
# are stated (maTypeInfo.mbSingleSeriesVis).  From oox/source/drawingml/chart/typegroupconverter.cxx's
# spTypeInfos: pie, doughnut, ofPie and surface families.
# spTypeInfos' "1stvis" column is true for TYPEID_PIE and TYPEID_OFPIE only -- NOT doughnut and
# NOT surface, both of which a first draft of this census wrongly included.
# typegroupconverter.cxx:103-118 and :191-192 (pie3DChart and pieChart both map to TYPEID_PIE).
SINGLE_SERIES_VIS = {"pieChart", "pie3DChart", "ofPieChart"}
TYPE_GROUPS = {"areaChart","area3DChart","lineChart","line3DChart","stockChart","radarChart",
               "scatterChart","pieChart","pie3DChart","doughnutChart","barChart","bar3DChart",
               "ofPieChart","surfaceChart","surface3DChart","bubbleChart"}
AXIS_ELEMS = {"catAx","valAx","dateAx","serAx"}

def text_of_txbody(el):
    """a:t text under an a:p tree, LibreOffice's TextBody."""
    if el is None: return None
    out = []
    for t in el.iter(q(A, "t")):
        out.append(t.text or "")
    s = "".join(out)
    return s if s else None

def textbody_is_empty(el):
    """TextBody::isEmpty() -- no paragraph carries a run with text."""
    return text_of_txbody(el) is None

def cache_first(tx):
    """The first cached string of a c:tx/c:strRef/c:strCache (DataSequenceModel::maData.begin())."""
    if tx is None: return None
    for holder in ("strRef", "numRef", "multiLvlStrRef"):
        ref = kid(tx, holder)
        if ref is None: continue
        for cache in ref:
            for pt in cache.findall(q(C, "pt")):
                v = pt.find(q(C, "v"))
                if v is not None and (v.text or ""):
                    return v.text
    lit = kid(tx, "strLit") or kid(tx, "numLit")
    if lit is not None:
        for pt in lit.findall(q(C, "pt")):
            v = pt.find(q(C, "v"))
            if v is not None and (v.text or ""): return v.text
    v = kid(tx, "v")
    if v is not None and (v.text or ""): return v.text
    return None

def title_own_text(title):
    """What TextConverter::createStringSequence finds before it reaches the default string."""
    if title is None: return None
    tx = kid(title, "tx")
    rich = kid(tx, "rich")
    t = text_of_txbody(rich)
    if t: return t
    t = text_of_txbody(kid(title, "txPr"))
    if t: return t
    return cache_first(tx)

def axes_sets(plot_area):
    """AxesSetModel list.  oox groups type groups by the axis ids they name; a chart with a
    secondary axes set has two.  Reproduced by bucketing each type group on the *set* of axIds
    it states, then grouping the buckets by which value axis they hang off."""
    groups = [g for g in plot_area if g.tag.startswith("{%s}" % C)
              and g.tag.split("}")[1] in TYPE_GROUPS]
    axes = {}
    for ax in plot_area:
        if not ax.tag.startswith("{%s}" % C): continue
        n = ax.tag.split("}")[1]
        if n not in AXIS_ELEMS: continue
        idel = kid(ax, "axId")
        if idel is None: continue
        axes[idel.get("val")] = n
    # PlotAreaConverter::convertFromModel groups on exact, ORDER-SENSITIVE maAxisIds equality
    # and skips a type group with no series at all (plotareaconverter.cxx:415-440).
    buckets = []
    for g in groups:
        if not kids(g, "ser"): continue
        ids = tuple(a.get("val") for a in kids(g, "axId"))
        for b in buckets:
            if b[0] == ids:
                b[1].append(g); break
        else:
            buckets.append((ids, [g]))
    return [b[1] for b in buckets]

def single_series_title(group):
    sers = kids(group, "ser")
    if not sers: return None, False
    name = group.tag.split("}")[1]
    if not (name in SINGLE_SERIES_VIS or len(sers) == 1):
        return None, False
    tx = kid(sers[0], "tx")
    if tx is None: return None, False
    return cache_first(tx), True          # (title, isSingleSeriesTitle)

def analyse(chart_space, mso2007):
    chart = kid(chart_space, "chart")
    if chart is None: return None
    plot_area = kid(chart, "plotArea")
    if plot_area is None: return None

    title = kid(chart, "title")
    atd_el = kid(chart, "autoTitleDeleted")
    auto_title_del = boolval(atd_el, not mso2007) if atd_el is not None else (not mso2007)

    sets = axes_sets(plot_area)
    auto = None
    is_single = False
    if len(sets) == 1 and len(sets[0]) == 1:
        auto, is_single = single_series_title(sets[0][0])
    auto = auto or ""

    own = title_own_text(title)

    res = dict(has_title=title is not None, auto_title_del=auto_title_del,
               atd_stated=atd_el is not None,
               n_axes_sets=len(sets), n_groups=sum(len(s) for s in sets),
               own=own or "", auto=auto, drawn="", why="")

    # The two ways a chart gets no title at all, kept apart because they carry very different
    # risk. "no-title-atd" rests on mbAutoTitleDel defaulting to TRUE for a non-MSO2007 document
    # (chartspacemodel.cxx:29) -- a *default*, not a stated attribute, and therefore the census's
    # largest exposure if 26.2.4.2 differs. "no-title-nothing" needs no default: neither a title
    # element nor a single-series title exists, so the outer test cannot be entered whatever the
    # default is.
    if not (not auto_title_del or title is not None):
        res["why"] = "no-title-atd"
        res["drawn"] = auto or ("Chart Title" if title is not None else "")
        return res
    if title is None and not auto:
        res["why"] = "no-title-nothing"
        return res

    if own:
        res["drawn"] = own; res["why"] = "own-text"
        return res

    show_empty = (not auto) and (not auto_title_del) and is_single \
        and kid(title, "spPr") is not None and kid(title, "txPr") is not None \
        and textbody_is_empty(kid(title, "txPr")) if title is not None else False
    tx = kid(title, "tx") if title is not None else None
    empty_rich = tx is not None and kid(tx, "rich") is not None and textbody_is_empty(kid(tx, "rich"))

    if not auto and not show_empty and not empty_rich:
        res["drawn"] = "Chart Title"; res["why"] = "literal"
    elif auto:
        res["drawn"] = auto; res["why"] = "series"
    else:
        res["drawn"] = ""; res["why"] = "empty-kept"
    return res

def mso2007_of(z):
    try:
        app = ET.fromstring(z.read("docProps/app.xml"))
    except Exception:
        return False
    def get(n):
        e = app.find(q(EXTPR, n))
        return (e.text or "") if e is not None else ""
    return get("Application").lower().startswith("microsoft") and get("AppVersion").startswith("12.")

def main(root, manifest, out):
    rows = list(csv.DictReader(open(manifest), delimiter="\t"))
    w = csv.writer(open(out, "w"), delimiter="\t")
    w.writerow(["family","status","path","part","why","drawn","own","has_title",
                "auto_title_del","n_axes_sets","n_groups"])
    seen = 0
    for r in rows:
        p = os.path.join(root, r["path"])
        if not os.path.exists(p): continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        names = [n for n in z.namelist()
                 if re.search(r"charts?/chart\d*\.xml$", n) or n.endswith("/chart.xml")]
        if not names: 
            z.close(); continue
        mso = mso2007_of(z)
        for n in sorted(names):
            try:
                root_el = ET.fromstring(z.read(n))
            except Exception:
                continue
            if root_el.tag != q(C, "chartSpace"): continue
            res = analyse(root_el, mso)
            if res is None: continue
            seen += 1
            w.writerow([r["family"], r["status"], r["path"], n, res["why"], res["drawn"],
                        res["own"][:40], int(res["has_title"]), int(res["auto_title_del"]),
                        int(res["atd_stated"]), res["n_axes_sets"], res["n_groups"]])
        z.close()
    print("chart parts analysed:", seen, file=sys.stderr)

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3])
