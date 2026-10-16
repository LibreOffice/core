#!/usr/bin/env python3
"""The two censuses that have never been run, plus the pie label census.

Round 58 shipped `colorScale` and named two blind spots in its prediction file that could
not fire because nothing was implemented in either arm.  Both are now live and neither has
ever been read:

  A. **`.xls` colour scales live in `CF12` (0x087A)**, not in `CF` (0x01B1).  Every census
     on this project has counted `CONDFMT`/`CF` and stopped.  A BIFF8 colour scale, data bar
     or icon set is a `CF12` record and is invisible to all of them.
  B. **The `x14` extension arm.**  A `cfRule` that exists only inside
     `<extLst><x14:conditionalFormattings>` is not under `<conditionalFormatting>` in the
     sheet body and no census has walked it.

  C. Pie data labels — `c:dLblPos` and `c:showLegendKey` across all three families, because
     the round's first item is `bestFit` and it is drawn by shared code.

Every manifest row must produce output or the script refuses to summarise.
"""
import collections, os, re, struct, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
MANIFEST = os.path.join(CORPUS, "MANIFEST.tsv")

rows = []
with open(MANIFEST, encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        rows.append({"family": f[0], "path": f[2], "ext": f[3].lower(), "status": f[7]})


# ─────────────────────────────────────────────────────────────── OLE2 / BIFF ──
def ole_streams(data):
    """Yield (name, bytes) for every stream in a compound file. Enough of CFB to walk it."""
    if data[:8] != b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":
        return
    ssz = 1 << struct.unpack_from("<H", data, 30)[0]
    mini = 1 << struct.unpack_from("<H", data, 32)[0]
    nfat = struct.unpack_from("<I", data, 44)[0]
    dirstart = struct.unpack_from("<I", data, 48)[0]
    ministart = struct.unpack_from("<I", data, 60)[0]
    difstart, ndif = struct.unpack_from("<II", data, 68)
    def sect(n):
        off = (n + 1) * ssz
        return data[off:off + ssz]
    fatsects = [struct.unpack_from("<I", data, 76 + 4 * i)[0] for i in range(109)]
    nxt = difstart
    for _ in range(ndif):
        if nxt >= 0xFFFFFFFA:
            break
        blk = sect(nxt)
        cnt = ssz // 4 - 1
        fatsects += [struct.unpack_from("<I", blk, 4 * i)[0] for i in range(cnt)]
        nxt = struct.unpack_from("<I", blk, 4 * cnt)[0]
    fat = []
    for s in fatsects[:nfat]:
        if s >= 0xFFFFFFFA:
            continue
        blk = sect(s)
        fat += [struct.unpack_from("<I", blk, 4 * i)[0] for i in range(len(blk) // 4)]
    def chain(start):
        out, n, seen = [], start, set()
        while n < 0xFFFFFFFA and n not in seen and n < len(fat):
            seen.add(n)
            out.append(n)
            n = fat[n]
        return out
    def read(start, size=None):
        b = b"".join(sect(n) for n in chain(start))
        return b[:size] if size is not None else b
    dirdata = read(dirstart)
    entries = []
    for i in range(len(dirdata) // 128):
        e = dirdata[128 * i:128 * i + 128]
        nlen = struct.unpack_from("<H", e, 64)[0]
        name = e[:max(0, nlen - 2)].decode("utf-16-le", "replace")
        typ = e[66]
        start, size = struct.unpack_from("<IQ", e, 116)
        entries.append((name, typ, start, size))
    minidata = b""
    if entries and entries[0][2] < 0xFFFFFFFA:
        minifat = read(ministart)
        mfat = [struct.unpack_from("<I", minifat, 4 * i)[0] for i in range(len(minifat) // 4)]
        minidata = read(entries[0][2], entries[0][3])
        def minichain(start):
            out, n, seen = [], start, set()
            while n < 0xFFFFFFFA and n not in seen and n < len(mfat):
                seen.add(n)
                out.append(n)
                n = mfat[n]
            return out
    for name, typ, start, size in entries:
        if typ != 2 or size == 0:
            continue
        if size < 4096 and minidata:
            b = b"".join(minidata[n * mini:(n + 1) * mini] for n in minichain(start))
        else:
            b = read(start, size)
        yield name, b[:size]


def biff_records(stream):
    """Yield (id, payload) with CONTINUE (0x003C) folded into the previous record."""
    i, out = 0, []
    while i + 4 <= len(stream):
        rid, rlen = struct.unpack_from("<HH", stream, i)
        body = stream[i + 4:i + 4 + rlen]
        if rid == 0x003C and out:
            out[-1] = (out[-1][0], out[-1][1] + body)
        else:
            out.append((rid, body))
        i += 4 + rlen
        if rlen == 0 and rid == 0:
            break
    return out


CF12 = 0x087A
CF = 0x01B1
CONDFMT = 0x01B0
CONDFMT12 = 0x0879


def xls_arm(full):
    with open(full, "rb") as fh:
        data = fh.read()
    got = collections.Counter()
    kinds = collections.Counter()
    for name, stream in ole_streams(data):
        if name.lower() not in ("workbook", "book"):
            continue
        for rid, body in biff_records(stream):
            if rid == CF12:
                got["CF12"] += 1
                # CF12: rt(2) grbitFrt(2) reserved(8) ... then ct(1) cp(1)
                if len(body) >= 14:
                    ct = body[12]
                    kinds["CF12:ct=%d" % ct] += 1
            elif rid == CONDFMT12:
                got["CONDFMT12"] += 1
            elif rid == CF:
                got["CF"] += 1
            elif rid == CONDFMT:
                got["CONDFMT"] += 1
    return got, kinds


# ─────────────────────────────────────────────────────────── x14 extension ──
X14CF = re.compile(rb"x14:conditionalFormattings")
X14RULE = re.compile(rb"<x14:cfRule[^>]*type=\"([A-Za-z]+)\"")
X14ANY = re.compile(rb"<x14:cfRule\b")


def x14_arm(full):
    got = collections.Counter()
    with zipfile.ZipFile(full) as z:
        for n in z.namelist():
            if "/worksheets/" not in n.lower() or not n.lower().endswith(".xml"):
                continue
            d = z.read(n)
            if not X14CF.search(d):
                continue
            got["blocks"] += 1
            for m in X14RULE.finditer(d):
                got["type:" + m.group(1).decode()] += 1
            got["rules"] += len(X14ANY.findall(d))
    return got


# ──────────────────────────────────────────────────────────── pie labels ──
DLBLPOS = re.compile(rb"<c:dLblPos +val=\"([A-Za-z]+)\"")
PIE = re.compile(rb"<c:(pieChart|pie3DChart|ofPieChart|doughnutChart)\b")
KEY = re.compile(rb"<c:showLegendKey +val=\"1\"")


def chart_arm(full):
    got = collections.Counter()
    with zipfile.ZipFile(full) as z:
        for n in z.namelist():
            low = n.lower()
            if "chart" not in low or not low.endswith(".xml") or "/charts/" not in low:
                continue
            d = z.read(n)
            kinds = set(m.group(1).decode() for m in PIE.finditer(d))
            if not kinds:
                continue
            got["pie parts"] += 1
            for k in kinds:
                got["kind:" + k] += 1
            for m in DLBLPOS.finditer(d):
                got["dLblPos:" + m.group(1).decode()] += 1
            if KEY.search(d):
                got["parts with showLegendKey=1"] += 1
    return got


ZIPPY = ("xlsx", "xlsm", "pptx", "docx")

xls_tot, xls_kind, xls_docs = collections.Counter(), collections.Counter(), collections.Counter()
x14_tot, x14_docs = collections.Counter(), []
chart_tot = collections.defaultdict(collections.Counter)
chart_docs = collections.defaultdict(list)
seen = 0
failures = []

for r in rows:
    full = os.path.join(CORPUS, r["path"])
    try:
        if r["ext"] == "xls":
            got, kinds = xls_arm(full)
            for k, v in got.items():
                xls_tot[k] += v
                xls_docs[k] += 1
            xls_kind.update(kinds)
        elif r["ext"] in ZIPPY:
            if r["family"] == "sheets":
                got = x14_arm(full)
                if got:
                    x14_docs.append((r["path"], dict(got)))
                x14_tot.update(got)
            got = chart_arm(full)
            if got:
                chart_tot[r["family"]].update(got)
                if got.get("dLblPos:bestFit"):
                    chart_docs[r["family"]].append((r["path"], got["dLblPos:bestFit"]))
        seen += 1
    except Exception as exc:                                    # noqa: BLE001
        failures.append((r["path"], repr(exc)))

if failures or seen != len(rows):
    print("REFUSING TO SUMMARISE — %d of %d rows produced no result:"
          % (len(rows) - seen, len(rows)), file=sys.stderr)
    for p, e in failures[:20]:
        print("   ", p, e, file=sys.stderr)
    sys.exit(2)

print("inputs: %d manifest rows, %d produced output, 0 failures" % (len(rows), seen))

print("\n=== A. .xls conditional formatting, CF12 (0x087A) read for the first time ===")
if not xls_tot:
    print("  nothing at all")
for k, v in sorted(xls_tot.items()):
    print("  %-12s %6d records  %3d documents" % (k, v, xls_docs[k]))
for k, v in sorted(xls_kind.items()):
    print("  %-12s %6d" % (k, v))
print("  CF12 ct codes: 1 cellIs, 2 expression, 3 colorScale, 4 dataBar, 5 (top10), 6 iconSet")

print("\n=== B. the x14 extension arm ===")
if not x14_tot:
    print("  0 sheets documents carry <x14:conditionalFormattings>")
for k, v in sorted(x14_tot.items()):
    print("  %-28s %6d" % (k, v))
for p, g in x14_docs:
    print("    %s  %s" % (p[7:], g))

print("\n=== C. pie data labels, all three families ===")
for fam in ("sheets", "slides", "words"):
    print("  -- %s --" % fam)
    for k, v in sorted(chart_tot[fam].items()):
        print("     %-34s %6d" % (k, v))
    if chart_docs[fam]:
        print("     bestFit documents (%d):" % len(chart_docs[fam]))
        for p, n in sorted(chart_docs[fam]):
            print("       %3d  %s" % (n, p))
