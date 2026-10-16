#!/usr/bin/env python3
"""How many corpus documents gain the automatic `D9D9D9` chart-area border.

`LineFormatter`'s constructor (`oox/source/drawingml/chart/objectformatter.cxx:826-852`) gives
every `OBJECTTYPE_CHARTSPACE` an automatic solid `D9D9D9` line 9525 EMU (0.75 pt) wide — and
skips it when the filter name starts with `Impress` (tdf#150176).  A stated
`c:chartSpace/c:spPr/a:ln` then overrides it through `assignUsed`.

So a document gains a border for every OOXML chart part that is **not** in a presentation and
whose chart space either states no `a:ln` at all or states one that leaves the fill unset.
Counted here per part and per document, by family, from `MANIFEST.tsv`.

What this cannot see, and is written down before it runs:

  * a BIFF (`.xls`) or ODF (`.ods`/`.odp`/`.odt`) chart never goes through `oox`, so an
    extension count under-reaches by exactly those documents and this says nothing about them;
  * whether *we* draw the chart at all — `DrawingChartPlot.Read` returns null for several
    chart kinds, and a part that draws nothing gains no border however it is formatted;
  * whether the chart is on a page the document actually prints;
  * `a:ln` with `a:noFill`, which is a stated line that turns the border *off* and is counted
    separately below rather than folded in.
"""
import os, re, sys, zipfile, collections

CORPUS = "/c/sandbox/workdir/sample-files"
CHART = re.compile(r"^(xl|word|ppt)/charts/chart\d+\.xml$", re.I)
LN = re.compile(r"<c:chartSpace\b.*?", re.S)


def chart_parts(path):
    try:
        with zipfile.ZipFile(path) as z:
            for n in z.namelist():
                if CHART.match(n):
                    yield n, z.read(n).decode("utf-8", "replace")
    except Exception:
        return


def space_line(xml):
    """('none'|'noFill'|'stated') for the chart space's own a:ln."""
    # The chart space's own spPr is the first one, before <c:chart>.
    head = xml.split("<c:chart>", 1)[0]
    m = re.search(r"<c:spPr\b.*?</c:spPr>", head, re.S)
    if not m:
        return "none"
    ln = re.search(r"<a:ln\b.*?(?:</a:ln>|/>)", m.group(0), re.S)
    if not ln:
        return "none"
    body = ln.group(0)
    if "<a:noFill" in body:
        return "noFill"
    if re.search(r"<a:(solidFill|gradFill|pattFill|blipFill)\b", body):
        return "stated"
    return "none"


rows = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        rows.append(f)

per_family = collections.Counter()
per_family_docs = collections.defaultdict(set)
states = collections.Counter()
gaining = collections.defaultdict(list)

for f in rows:
    family, path, status = f[0], f[2], f[7]
    full = os.path.join(CORPUS, path)
    if not os.path.exists(full):
        print("MISSING", path, file=sys.stderr)
        continue
    for name, xml in chart_parts(full):
        host = name.split("/")[0].lower()
        state = space_line(xml)
        states[(host, state)] += 1
        # ppt/ is the Impress filter and is excluded by the C++ itself.
        if host != "ppt" and state == "none":
            per_family[family] += 1
            per_family_docs[family].add(path)
            gaining[path].append((name, status))

print("chart-space a:ln by host and state:")
for (host, state), n in sorted(states.items()):
    print("  %-5s %-8s %4d parts" % (host, state, n))
print()
print("documents that gain the automatic border (non-Impress, no stated chart-space line):")
for family in sorted(per_family_docs):
    print("  %-7s %3d documents, %3d parts" % (family, len(per_family_docs[family]),
                                               per_family[family]))
print()
for path in sorted(gaining):
    print("  %-4s %s" % (gaining[path][0][1], path), len(gaining[path]), "parts")
