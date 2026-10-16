#!/usr/bin/env python3
"""How many legacy FORMCHECKBOX fields are in the words corpus, and in what.

    checkbox-census.py <corpus-root>

Counts `w:ffData/w:checkBox` in every part of every OOXML package -- not `document.xml` alone,
because a form's boxes live in headers and in `w:altChunk` bodies too -- and the `.doc` equivalent
is not counted here at all, which is a stated blind spot rather than an absence.
"""
import csv, os, re, sys, zipfile

MAN = "/c/sandbox/workdir/sample-files/MANIFEST.tsv"
BOX = re.compile(rb"<w:checkBox[ />]")
SIZE = re.compile(rb"<w:size\s+w:val=\"(\d+)\"")
AUTO = re.compile(rb"<w:sizeAuto")
CHECKED = re.compile(rb"<w:default\s+w:val=\"(1|true|on)\"")

root = sys.argv[1] if len(sys.argv) > 1 else "/c/sandbox/workdir/sample-files"
rows = []
total = auto = stated = checked = 0
for row in csv.DictReader(open(MAN, newline=""), delimiter="\t"):
    if row["family"] != "words":
        continue
    path = os.path.join(root, row["path"])
    if not zipfile.is_zipfile(path):
        continue
    n = a = s = c = 0
    try:
        with zipfile.ZipFile(path) as z:
            for name in z.namelist():
                if not name.endswith(".xml"):
                    continue
                blob = z.read(name)
                if b"<w:checkBox" not in blob:
                    continue
                for m in re.finditer(rb"<w:checkBox.*?</w:checkBox>|<w:checkBox[^>]*/>", blob, re.S):
                    n += 1
                    body = m.group(0)
                    if AUTO.search(body):
                        a += 1
                    if SIZE.search(body):
                        s += 1
                    if CHECKED.search(body):
                        c += 1
    except (zipfile.BadZipFile, OSError):
        continue
    if n:
        rows.append((n, a, s, c, row["ext"], row["status"], os.path.basename(row["path"])))
        total += n; auto += a; stated += s; checked += c

rows.sort(key=lambda t: -t[0])
print(f"{'boxes':>6} {'sizeAuto':>9} {'w:size':>7} {'checked':>8} {'ext':>5} {'status':>6}  document")
for n, a, s, c, ext, status, name in rows:
    print(f"{n:6d} {a:9d} {s:7d} {c:8d} {ext:>5} {status:>6}  {name}")
print(f"\n{total} boxes in {len(rows)} OOXML documents; "
      f"{auto} state w:sizeAuto, {stated} state a w:size, {checked} are checked by default")
