#!/usr/bin/env python3
"""Census a:clrChange (and its siblings) over the whole corpus, resolved per document.

Estimate reach from what a shape RESOLVES to, not what a part declares: clrChange lives on
an a:blip, which is always inside a blipFill that is actually drawn -- unlike a theme's fill
style. But a blip can also sit in a layout/master that no slide instantiates, so count the
part it lives in as well.
"""
import collections, os, re, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
EXTS = {".pptx",".ppsx",".potx",".docx",".xlsx",".dotx",".xltx",".ppt",".doc",".xls",
        ".odp",".odt",".ods",".otp",".ott",".rtf",".csv",".otx"}
PATS = {
    "clrChange": re.compile(rb"<a:clrChange[ />]"),
    "duotone":   re.compile(rb"<a:duotone[ />]"),
    "alphaModFix":re.compile(rb"<a:alphaModFix[ />]"),
    "biLevel":   re.compile(rb"<a:biLevel[ />]"),
    "grayscl":   re.compile(rb"<a:grayscl[ />]"),
}

seen = {}          # casefolded relpath -> info
hits = collections.defaultdict(lambda: collections.defaultdict(int))
parts_of = collections.defaultdict(lambda: collections.Counter())

for root, dirs, files in os.walk(CORPUS):
    if ".git" in root.split(os.sep):
        continue
    for name in files:
        p = os.path.join(root, name)
        rel = os.path.relpath(p, CORPUS)
        ext = os.path.splitext(name)[1].lower()
        if ext not in EXTS:
            continue
        key = rel.casefold()           # case-insensitive mount: one inode, one document
        if key in seen:
            continue
        seen[key] = rel
        if not zipfile.is_zipfile(p):
            continue
        try:
            with zipfile.ZipFile(p) as z:
                for info in z.infolist():
                    if not info.filename.endswith(".xml"):
                        continue
                    try:
                        data = z.read(info)
                    except Exception:
                        continue
                    for tag, pat in PATS.items():
                        n = len(pat.findall(data))
                        if n:
                            hits[tag][rel] += n
                            parts_of[tag][info.filename.split("/")[1] if "/" in info.filename
                                          else info.filename] += n
        except Exception as exc:
            print("ERR", rel, exc, file=sys.stderr)

print(f"documents scanned (case-folded): {len(seen)}")
for tag in PATS:
    docs = hits[tag]
    print(f"\n=== {tag}: {len(docs)} documents, {sum(docs.values())} occurrences ===")
    print("  parts:", dict(parts_of[tag]))
    for d, n in sorted(docs.items(), key=lambda kv: -kv[1]):
        print(f"  {n:4d}  {d}")
