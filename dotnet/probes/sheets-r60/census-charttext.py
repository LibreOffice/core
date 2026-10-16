#!/usr/bin/env python3
"""Which corpus documents draw chart text at all — the reach of a chart text-metric change.

Two readers, because a `.xls` chart states nothing an OOXML part would: an `.xlsx`/`.xlsm` chart
lives in a `charts/chart*.xml` part (the namespace may be bound as the default, with no `c:`
prefix, which round 59's census learned the hard way), and a `.xls` chart is a BIFF substream
whose BOF `dt` field is 0x0020.

Counts *documents that would be re-laid-out*, not chart parts, and case-folds where it
accumulates — round 59's parent census reported 68 for 62 because one inode's two directory
entries were counted as two documents.

Reads all of `MANIFEST.tsv` and refuses to summarise unless every row produced an answer.
"""
import collections
import os
import re
import struct
import sys
import zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
CHART_PART = re.compile(r"charts?/chart\d*\.xml$", re.I)


def xlsx_charts(path):
    try:
        with zipfile.ZipFile(path) as z:
            return sum(1 for n in z.namelist() if CHART_PART.search(n))
    except Exception:
        return None


def ole_streams(data):
    """Walk a compound file's directory far enough to find the top-level stream names."""
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
    names = []
    seen = set()
    s = first_dir
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


def xls_charts(path):
    """A BIFF chart substream: a BOF whose document type is 0x0020."""
    try:
        data = open(path, "rb").read()
    except Exception:
        return None
    if data[:8] == b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":
        names = ole_streams(data) or []
        if not any(n in ("Workbook", "Book") for n in names):
            return 0
    n = 0
    at = 0
    end = len(data) - 4
    while at < end:
        rec, ln = struct.unpack("<HH", data[at:at + 4])
        if rec == 0x0809 and ln >= 4 and at + 4 + ln <= len(data):
            dt = struct.unpack("<H", data[at + 6:at + 8])[0]
            if dt == 0x0020:
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

    per_family = collections.Counter()
    parts = collections.Counter()
    unread = []
    hits = collections.defaultdict(list)
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
        if e in ("xlsx", "xlsm", "docx", "pptx"):
            n = xlsx_charts(full)
        else:
            n = xls_charts(full)
        if n is None:
            unread.append(path)
            continue
        if n:
            per_family[family] += 1
            parts[family] += n
            hits[family].append((status, path, n))

    if unread:
        print("REFUSING TO SUMMARISE — %d manifest rows could not be read:" % len(unread),
              file=sys.stderr)
        for u in unread[:20]:
            print("   ", u, file=sys.stderr)
        sys.exit(2)

    print("distinct manifest paths read: %d" % len(seen))
    for family in ("sheets", "slides", "words"):
        print("%-8s %3d documents hold a chart, %4d chart parts/substreams"
              % (family, per_family[family], parts[family]))
    print("\nsheets documents holding a chart, by manifest status:")
    by_status = collections.Counter(s for s, _, _ in hits["sheets"])
    for k, v in by_status.most_common():
        print("   %-8s %d" % (k, v))
    print()
    for status, path, n in sorted(hits["sheets"]):
        print("  %-8s %-96s %d" % (status, path[7:], n))


if __name__ == "__main__":
    main()
