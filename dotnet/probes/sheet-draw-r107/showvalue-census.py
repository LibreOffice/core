#!/usr/bin/env python3
"""Which corpus workbooks state showValue="0" on an iconSet or a dataBar, and where.

Walks every worksheet part of every zip-shaped spreadsheet given on the command line and counts
the rules whose `showValue` is off, split by namespace (the main `cfRule` and the `x14` extension)
and by family. Prints one row per document that states one and a total.
"""
import re
import sys
import zipfile

MAIN = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
X14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"

OFF = ("0", "false", "False", "FALSE")


def rules(xml):
    """(namespace, family) for every iconSet/dataBar element whose showValue is off."""
    out = []
    for m in re.finditer(rb"<(?:(\w+):)?(iconSet|dataBar)\b([^>]*)>", xml):
        prefix, family, attrs = m.group(1), m.group(2).decode(), m.group(3)
        sv = re.search(rb'showValue="([^"]*)"', attrs)
        if sv is None or sv.group(1).decode() not in OFF:
            continue
        out.append(("x14" if prefix else "main", family))
    return out


def main(paths):
    total = {}
    documents = 0
    for path in paths:
        if not zipfile.is_zipfile(path):
            continue
        found = {}
        try:
            with zipfile.ZipFile(path) as z:
                for name in z.namelist():
                    if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                        continue
                    for key in rules(z.read(name)):
                        found[key] = found.get(key, 0) + 1
        except Exception as exc:                                     # noqa: BLE001
            print(f"# {path}: {exc}", file=sys.stderr)
            continue
        if not found:
            continue
        documents += 1
        for k, v in found.items():
            total[k] = total.get(k, 0) + v
        print(f"{path.split('/')[-1]}\t" + " ".join(f"{a}:{b}={c}" for (a, b), c in sorted(found.items())))
    print(f"# {documents} documents state showValue off; rules " +
          " ".join(f"{a}:{b}={c}" for (a, b), c in sorted(total.items())), file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1:])
