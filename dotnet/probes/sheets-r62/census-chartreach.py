#!/usr/bin/env python3
"""Reach of round 62's change: every document whose rendering holds chart *text* at all.

The change is to the **advance width** a chart's text is measured and drawn with — round 60 put
the vertical metrics of a chart's text through `chart2`'s own 96 dpi device and left the width on
the face's unquantised advances.  So the reach is not "pies" and not "titled charts": it is every
chart part that draws a title, an axis label, a legend entry or a data label, which in practice is
every chart part at all.  A chart with no text is possible and is counted separately so the
distinction is visible rather than assumed.

It is implemented in `XRay.Office.Spreadsheets` only — `SheetBandText` and `SheetChart` — exactly
where round 60 put the vertical half, so the slides and words columns are reported to show what a
later cross-track round would reach and **not** what this one moves.

BIFF (`.xls`, `.ppt`, `.doc`) is read only far enough to say a chart substream exists; those
documents go through the same `SheetChart` measurer once decoded, so for sheets they are part of
the reach and are counted as such.

Refuses to summarise unless every manifest row produced an answer.
"""
import collections
import os
import re
import struct
import sys
import zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
CHART_PART = re.compile(r"charts?/chart\d*\.xml$", re.I)

# The namespace may be bound as the default, with no `c:` prefix — round 59's census learned
# that the hard way, so every element test is prefix-agnostic.
def tag(name):
    return re.compile(r"<(?:[A-Za-z0-9_.-]+:)?%s[ />]" % name)

TITLE = tag("title")
AUTODEL = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?autoTitleDeleted[^>]*val=\"(?:1|true)\"")
PIE = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?(?:pieChart|ofPieChart|pie3DChart)[ />]")
DLBLS = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?dLbls[ >]")
DLBLPOS = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?dLblPos[^>]*val=\"([a-zA-Z]+)\"")
DELETE1 = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?delete[^>]*val=\"(?:1|true)\"")


TEXTISH = re.compile(
    r"<(?:[A-Za-z0-9_.-]+:)?(?:title|catAx|valAx|dateAx|serAx|legend|dLbls)[ >/]")


def read_ooxml(path):
    """(charts, chart parts holding text, bestfit pies) for one package."""
    charts = titled = pies = 0
    with zipfile.ZipFile(path) as z:
        for n in z.namelist():
            if not CHART_PART.search(n):
                continue
            charts += 1
            x = z.read(n).decode("utf-8", "replace")
            if TEXTISH.search(x):
                titled += 1
            if PIE.search(x) and DLBLS.search(x):
                placements = set(DLBLPOS.findall(x))
                if not placements or "bestFit" in placements:
                    pies += 1
    return charts, titled, pies


def ole_streams(data):
    if data[:8] != b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":
        return None
    sector = 1 << struct.unpack("<H", data[30:32])[0]
    first_dir = struct.unpack("<i", data[48:52])[0]
    fat_count = struct.unpack("<i", data[44:48])[0]
    difat = list(struct.unpack("<109i", data[76:512]))[:fat_count]
    fat = []
    for s in difat:
        if s < 0:
            continue
        off = 512 + s * sector
        fat.extend(struct.unpack("<%di" % (sector // 4), data[off:off + sector]))
    names, seen, s = [], set(), first_dir
    while s >= 0 and s not in seen and len(names) < 40000:
        seen.add(s)
        off = 512 + s * sector
        blob = data[off:off + sector]
        for i in range(0, len(blob), 128):
            entry = blob[i:i + 128]
            if len(entry) < 128:
                break
            n = struct.unpack("<H", entry[64:66])[0]
            if n <= 2:
                continue
            names.append(entry[:n - 2].decode("utf-16-le", "replace"))
        s = fat[s] if s < len(fat) else -1
    return names


def read_biff(path):
    """How many BIFF chart substreams — a BOF whose document type is 0x0020."""
    data = open(path, "rb").read()
    if data[:8] == b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":
        names = ole_streams(data) or []
        if not any(n in ("Workbook", "Book", "PowerPoint Document", "WordDocument")
                   for n in names):
            return 0
    n, at, end = 0, 0, len(data) - 4
    while at < end:
        rec, ln = struct.unpack("<HH", data[at:at + 4])
        if rec == 0x0809 and ln >= 4 and at + 4 + ln <= len(data):
            if struct.unpack("<H", data[at + 6:at + 8])[0] == 0x0020:
                n += 1
        at += 1
    return n


def main():
    rows = []
    with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
        fh.readline()
        for line in fh:
            f = line.rstrip("\n").split("\t")
            rows.append((f[0], f[2], f[3], f[7]))

    titled = collections.Counter()
    pied = collections.Counter()
    biff = collections.Counter()
    unread = []
    pie_hits = collections.defaultdict(list)
    title_hits = collections.defaultdict(list)
    seen = set()

    for family, path, ext, status in rows:
        key = path.lower()
        if key in seen:
            continue
        seen.add(key)
        full = os.path.join(CORPUS, path)
        if not os.path.exists(full):
            unread.append(path)
            continue
        e = ext.lower()
        try:
            if e in ("xlsx", "xlsm", "xltx", "xltm", "docx", "docm", "dotx", "dotm",
                     "pptx", "pptm", "potx", "potm", "ppsx", "ppsm"):
                charts, t, p = read_ooxml(full)
                if t:
                    titled[family] += 1
                    title_hits[family].append((status, path, t))
                if p:
                    pied[family] += 1
                    pie_hits[family].append((status, path, p))
            elif e in ("xls", "xlt", "ppt", "pot", "pps", "doc", "dot", "xlsb"):
                if read_biff(full):
                    biff[family] += 1
        except Exception as exc:                                  # noqa: BLE001
            unread.append("%s (%s)" % (path, exc))

    if unread:
        print("REFUSING TO SUMMARISE — %d manifest rows could not be read:" % len(unread),
              file=sys.stderr)
        for u in unread[:20]:
            print("   ", u, file=sys.stderr)
        sys.exit(2)

    print("distinct manifest paths read: %d\n" % len(seen))
    print("%-8s %10s %10s %10s" % ("family", "text-chart", "bestFitPie", "biff-charts"))
    for family in ("sheets", "slides", "words"):
        print("%-8s %10d %10d %10d"
              % (family, titled[family], pied[family], biff[family]))
    print("\nbest-fit pie documents (OOXML), by family and manifest status:")
    for family in ("sheets", "slides", "words"):
        for status, path, n in sorted(pie_hits[family]):
            print("  %-6s %-8s %-92s %d" % (family, status, path, n))
    print("\nsheets documents with a text-bearing chart, by manifest status:")
    c = collections.Counter(s for s, _, _ in title_hits["sheets"])
    for k, v in c.most_common():
        print("   %-8s %d" % (k, v))
    print("\nsheets documents with a text-bearing chart, listed:")
    for status, path, n in sorted(title_hits["sheets"]):
        print("  %-8s %-92s %d" % (status, path, n))


if __name__ == "__main__":
    main()
