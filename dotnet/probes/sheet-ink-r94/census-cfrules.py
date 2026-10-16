#!/usr/bin/env python3
"""Census every `cfRule` in the sheets corpus: its type, and — for `expression` — its shape.

Run over the whole corpus rather than the sheets track alone, because a `.docx` or a `.pptx`
could in principle carry an embedded workbook part; none does, which is itself worth recording.

Usage: census-cfrules.py <corpus-root>
"""
import collections
import glob
import os
import re
import sys
import zipfile


def shape(formula):
    """The formula with its references, strings and numbers replaced, so shapes group."""
    g = re.sub(r"\$?[A-Z]{1,3}\$?\d+", "R", formula)
    g = re.sub(r'"[^"]*"', "S", g)
    return re.sub(r"\b\d+(\.\d+)?\b", "N", g)


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files"

    kinds = collections.Counter()
    kind_docs = collections.defaultdict(set)
    shapes = collections.Counter()
    named = collections.Counter()
    named_docs = collections.defaultdict(set)
    dxf_tags = collections.Counter()
    dxf_docs = set()

    for path in sorted(glob.glob(os.path.join(root, "*/*/*/*"))):
        if not os.path.isfile(path):
            continue
        try:
            zf = zipfile.ZipFile(path)
        except Exception:
            continue

        name = os.path.basename(path)
        for part in zf.namelist():
            if "worksheets/sheet" not in part or not part.endswith(".xml"):
                continue
            text = zf.read(part).decode("utf-8", "replace")
            for rule in re.finditer(r"<cfRule([^>]*?)(?:/>|>(.*?)</cfRule>)", text, re.S):
                attrs = dict(re.findall(r'(\w+)="([^"]*)"', rule.group(1)))
                kind = attrs.get("type", "?")
                kinds[kind] += 1
                kind_docs[kind].add(name)
                if "dxfId" in attrs:
                    named[kind] += 1
                    named_docs[kind].add(name)
                if kind == "expression":
                    found = re.findall(r"<formula>(.*?)</formula>", rule.group(2) or "", re.S)
                    shapes[shape(found[0]) if found else ""] += 1

        if "xl/styles.xml" in zf.namelist():
            styles = zf.read("xl/styles.xml").decode("utf-8", "replace")
            block = re.search(r"<dxfs[^>]*>(.*?)</dxfs>", styles, re.S)
            if block:
                for dxf in re.findall(r"<dxf>(.*?)</dxf>", block.group(1), re.S):
                    dxf_docs.add(name)
                    for tag in set(re.findall(r"<(\w+)", dxf)):
                        dxf_tags[tag] += 1

    print("kind\trules\tdocuments\tnaming a dxf\tdocuments")
    for kind, count in kinds.most_common():
        print(f"{kind}\t{count}\t{len(kind_docs[kind])}"
              f"\t{named[kind]}\t{len(named_docs[kind])}")

    print()
    print("expression shape\trules")
    for form, count in shapes.most_common(30):
        print(f"{form}\t{count}")

    print()
    print(f"documents stating a dxf\t{len(dxf_docs)}")
    print("dxf child tag\tdxfs stating it")
    for tag, count in dxf_tags.most_common():
        print(f"{tag}\t{count}")


if __name__ == "__main__":
    main()
