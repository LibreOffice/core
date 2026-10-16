#!/usr/bin/env python3
"""Reach over the converted ODF corpus: embedded faces, and hyperlinks in slide text.

  census.py fonts   -- how many .odp/.odt/.ods declare an svg:font-face-uri
  census.py links   -- how many .odp carry a text:a in content.xml

The corpus holds no ODF at all, so a reach census for an ODF attribute has to be taken over
`/home/user/corpus-odf`, which is 26.2.4.2's own --convert-to of it.  A census that finds
nothing in `sample-files` has found nothing about ODF.
"""
import collections, os, sys, zipfile

ROOT = os.environ.get("CORPUS_ODF", "/home/user/corpus-odf")

def walk(extensions):
    for path, _, names in os.walk(ROOT):
        for name in names:
            if name.rsplit(".", 1)[-1].lower() in extensions:
                yield os.path.join(path, name)

def parts(path, names):
    try:
        with zipfile.ZipFile(path) as zf:
            held = set(zf.namelist())
            return [zf.read(n).decode("utf-8", "replace") for n in names if n in held]
    except (zipfile.BadZipFile, OSError):
        return []

def fonts():
    totals, hits = collections.Counter(), collections.defaultdict(list)
    for path in walk({"odp", "odt", "ods"}):
        ext = path.rsplit(".", 1)[-1].lower()
        totals[ext] += 1
        n = sum(x.count("font-face-uri") for x in parts(path, ("content.xml", "styles.xml")))
        if n:
            hits[ext].append((os.path.relpath(path, ROOT), n))
    for ext in sorted(totals):
        print("%s: %d documents, %d embed a face" % (ext, totals[ext], len(hits[ext])))
    for ext in sorted(hits):
        for rel, n in sorted(hits[ext]):
            print("   %-4s %3d occurrences  %s" % (ext, n, rel))

def links():
    total = documents = occurrences = 0
    for path in walk({"odp"}):
        total += 1
        n = sum(x.count("<text:a ") for x in parts(path, ("content.xml",)))
        if n:
            documents += 1
            occurrences += n
    print("odp: %d documents, %d carry a text:a, %d occurrences"
          % (total, documents, occurrences))

if __name__ == "__main__":
    (fonts if (sys.argv[1:] or ["fonts"])[0] == "fonts" else links)()
