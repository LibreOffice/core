#!/usr/bin/env python3
"""How often an ODF style states style:font-name while an ancestor states fo:font-family.

Walks every .ods/.odt of the converted corpus, builds the style parent chain per family, and
reports the styles where the two spellings disagree in level.
"""
import os, re, sys, zipfile
from collections import Counter

CORPUS = "/home/user/corpus-odf"
STYLE = re.compile(
    r'<style:style\b([^>]*)>(.*?)</style:style>|<style:style\b([^>]*)/>', re.S)

def attrs(s):
    return dict(re.findall(r'([\w:-]+)="([^"]*)"', s))

def scan(path):
    try:
        z = zipfile.ZipFile(path)
        parts = [z.read(n).decode("utf-8", "replace")
                 for n in ("styles.xml", "content.xml") if n in z.namelist()]
    except Exception:
        return None
    styles = {}
    for c in parts:
        for m in STYLE.finditer(c):
            a = attrs(m.group(1) or m.group(3) or "")
            body = m.group(2) or ""
            name = a.get("style:name"); fam = a.get("style:family")
            if not name or not fam: continue
            tp = re.search(r'<style:text-properties\b([^>]*)', body)
            ta = attrs(tp.group(1)) if tp else {}
            styles[(fam, name)] = (a.get("style:parent-style-name"),
                                   ta.get("style:font-name"), ta.get("fo:font-family"))
    hits = 0
    for key in styles:
        fam = key[0]
        cur = key; depth = 0; nameAt = None; familyAt = None
        seen = set()
        while cur in styles and cur not in seen and depth < 16:
            seen.add(cur)
            parent, fn, ff = styles[cur]
            if fn is not None and nameAt is None: nameAt = depth
            if ff is not None and familyAt is None: familyAt = depth
            if parent is None: break
            cur = (fam, parent); depth += 1
        if nameAt is not None and familyAt is not None and nameAt < familyAt:
            hits += 1
    return hits

def main():
    ext = set(sys.argv[1:]) or {"ods", "odt"}
    docs = []
    for root, _d, files in os.walk(CORPUS):
        for n in files:
            if n.rsplit(".", 1)[-1].lower() in ext:
                docs.append(os.path.join(root, n))
    affected = 0; total = 0; styles = 0
    for d in sorted(docs):
        h = scan(d)
        if h is None: continue
        total += 1
        if h: affected += 1; styles += h
    print(f"{sorted(ext)}: {affected} of {total} documents, {styles} styles")

main()
