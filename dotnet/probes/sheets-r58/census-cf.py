#!/usr/bin/env python3
"""Census of conditional formatting across the sheets corpus.

Refuses to print anything unless EVERY manifest path produced a result — a missing
input read as zero reads as a finding (round 57's audit probe, and the words round's
before it).

Three arms, because the corpus has three readers:
  * xlsx-family (zip + SpreadsheetML)  — <conditionalFormatting><cfRule>
  * .xls (OLE2 + BIFF)                 — CONDFMT 0x01B0 / CF 0x01B1 records
  * .ods                               — <calcext:conditional-formats>, <style:map>
The third arm has no corpus rows and is here so the blind spot is measured rather
than assumed.
"""
import collections, os, re, struct, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
MANIFEST = os.path.join(CORPUS, "MANIFEST.tsv")

NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"

rows = []
with open(MANIFEST, encoding="utf-8") as fh:
    header = fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        if f[0] == "sheets":
            rows.append({"path": f[2], "ext": f[3], "status": f[7], "batch": f[1]})

results = {}
errors = {}

import xml.etree.ElementTree as ET

def xlsx_arm(full):
    """Returns (per-type counts, dxf count, sqref cell count, sheets touched)."""
    types = collections.Counter()
    dxfs = 0
    cells = 0
    with zipfile.ZipFile(full) as z:
        names = z.namelist()
        for n in names:
            low = n.lower()
            if not low.endswith(".xml"):
                continue
            if "/worksheets/" not in low and not low.endswith("styles.xml"):
                continue
            data = z.read(n)
            if low.endswith("styles.xml"):
                if b"<dxfs" in data or b":dxfs" in data:
                    try:
                        root = ET.fromstring(data)
                    except ET.ParseError:
                        continue
                    d = root.find(NS + "dxfs")
                    if d is not None:
                        dxfs += len(list(d))
                continue
            if b"cfRule" not in data:
                continue
            try:
                root = ET.fromstring(data)
            except ET.ParseError:
                # count textually rather than dropping the sheet
                for m in re.finditer(rb'<cfRule[^>]*\btype="([^"]+)"', data):
                    types[m.group(1).decode()] += 1
                continue
            for cf in root.iter(NS + "conditionalFormatting"):
                sq = cf.get("sqref", "")
                cells += sqref_cells(sq)
                for rule in cf.findall(NS + "cfRule"):
                    types[rule.get("type", "?")] += 1
            # extLst x14 rules (dataBar/iconSet extensions) counted separately
    return types, dxfs, cells

CELLRE = re.compile(r"^\$?([A-Za-z]{1,3})\$?(\d{1,7})$")

def colnum(s):
    n = 0
    for ch in s.upper():
        n = n * 26 + (ord(ch) - 64)
    return n

def sqref_cells(sq):
    total = 0
    for part in sq.split():
        if ":" in part:
            a, b = part.split(":", 1)
            ma, mb = CELLRE.match(a), CELLRE.match(b)
            if not (ma and mb):
                continue
            c1, r1 = colnum(ma.group(1)), int(ma.group(2))
            c2, r2 = colnum(mb.group(1)), int(mb.group(2))
            total += (abs(c2 - c1) + 1) * (abs(r2 - r1) + 1)
        elif CELLRE.match(part):
            total += 1
    return total

def xls_arm(full):
    """BIFF CONDFMT (0x01B0) / CF (0x01B1) record counts, walked over the whole stream."""
    import olefile
    types = collections.Counter()
    if not olefile.isOleFile(full):
        return types, 0, 0
    ole = olefile.OleFileIO(full)
    stream = None
    for cand in ("Workbook", "Book"):
        if ole.exists(cand):
            stream = ole.openstream(cand).read()
            break
    ole.close()
    if stream is None:
        return types, 0, 0
    at, n = 0, len(stream)
    while at + 4 <= n:
        rid, size = struct.unpack_from("<HH", stream, at)
        at += 4
        if at + size > n:
            break
        if rid == 0x01B0:
            types["CONDFMT"] += 1
        elif rid == 0x01B1:
            # CF record: ct (1 byte) is the condition type, cp the comparison
            ct = stream[at] if size >= 1 else 0
            types["CF"] += 1
            types["CF:ct=%d" % ct] += 1
        at += size
    return types, 0, 0

def ods_arm(full):
    types = collections.Counter()
    with zipfile.ZipFile(full) as z:
        data = z.read("content.xml")
    for m in re.finditer(rb"<calcext:conditional-format\b", data):
        types["calcext:conditional-format"] += 1
    for m in re.finditer(rb"<calcext:condition\b", data):
        types["calcext:condition"] += 1
    for m in re.finditer(rb"<calcext:color-scale\b", data):
        types["calcext:color-scale"] += 1
    for m in re.finditer(rb"<style:map\b", data):
        types["style:map"] += 1
    return types, 0, 0

for r in rows:
    full = os.path.join(CORPUS, r["path"])
    try:
        if not os.path.exists(full):
            raise FileNotFoundError(full)
        ext = r["ext"].lower()
        if ext in ("xlsx", "xlsm", "xltx", "xltm"):
            r["arm"] = "xlsx"
            r["types"], r["dxfs"], r["cells"] = xlsx_arm(full)
        elif ext in ("xls", "xlt"):
            r["arm"] = "xls"
            r["types"], r["dxfs"], r["cells"] = xls_arm(full)
        elif ext in ("ods", "ots", "fods"):
            r["arm"] = "ods"
            r["types"], r["dxfs"], r["cells"] = ods_arm(full)
        else:
            r["arm"] = "other"
            r["types"], r["dxfs"], r["cells"] = collections.Counter(), 0, 0
        results[r["path"]] = r
    except Exception as exc:      # noqa: BLE001
        errors[r["path"]] = repr(exc)

if errors:
    print("REFUSING TO REPORT — %d of %d inputs produced no output:" % (len(errors), len(rows)),
          file=sys.stderr)
    for k, v in sorted(errors.items())[:20]:
        print("   %s  %s" % (k, v), file=sys.stderr)
    sys.exit(2)

assert len(results) == len(rows), (len(results), len(rows))
print("inputs: %d manifest sheets rows, %d produced output, 0 failures" % (len(rows), len(results)))

for arm in ("xlsx", "xls", "ods", "other"):
    sub = [r for r in results.values() if r["arm"] == arm]
    if not sub:
        continue
    withcf = [r for r in sub if r["types"]]
    print("\n=== arm %s: %d documents, %d carry a rule ===" % (arm, len(sub), len(withcf)))
    per = collections.Counter()
    docs = collections.Counter()
    for r in sub:
        for k, v in r["types"].items():
            per[k] += v
            docs[k] += 1
    for k, _ in per.most_common():
        print("  %-28s %5d rules  %4d documents" % (k, per[k], docs[k]))
    if arm == "xlsx":
        cells = sum(r["cells"] for r in sub)
        print("  covered cells (sqref, summed)      %d" % cells)
        # colorScale specifically
        cs = [r for r in sub if r["types"].get("colorScale")]
        print("  colorScale documents: %d  (open %d, done %d)"
              % (len(cs),
                 sum(1 for r in cs if r["status"] == "open"),
                 sum(1 for r in cs if r["status"] != "open")))
        for r in sorted(cs, key=lambda r: -r["types"]["colorScale"]):
            print("     %3d  %-6s %s" % (r["types"]["colorScale"], r["status"],
                                         os.path.basename(r["path"])))
