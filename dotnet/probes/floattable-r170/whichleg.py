#!/usr/bin/env python3
"""Which of the reference's two answers this tree reproduces, measured on position.

`ours.py` asks the same question of pages and alphanumerics, which cannot see a wrap at all.
This pairs spans instead: one distance against the reference as authored (the positioned table
wrapped) and one against the same document with every `w:tblpPr` removed (the table in the
flow, which is what `Paginator.PlaceFloatedTable` leaves a table with room beside it).

A document where the two distances are equal is one whose wrap moves nothing, so it carries no
evidence either way and is marked `-`.  Everywhere else the smaller distance names the leg we
are on, and the gap between them is what implementing the wrap could be worth there.
"""
import os, pathlib, re, subprocess, sys

sys.path.insert(0, str(pathlib.Path(__file__).parent))
from spanshift import spans, pair

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"


def distance(ours, other):
    pairs = pair(ours, other)
    if not pairs:
        return None, 0
    return sum(abs(x) + abs(y) for x, y in pairs) / len(pairs), len(pairs)


def main():
    root = pathlib.Path(sys.argv[2])
    print("vsFloat\tnF\tvsFlow\tnW\tnearer\tdocument")
    for line in open(sys.argv[1]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        work = root / re.sub(r"\W+", "-", path.stem)[:60]
        ourdir = work / "ours"
        ourpdf = next(ourdir.glob("*.pdf"), None)
        if ourpdf is None:
            ourdir.mkdir(parents=True, exist_ok=True)
            try:
                subprocess.run([CLI, "render", str(path), "--outdir", str(ourdir)], check=True,
                               capture_output=True, timeout=900,
                               env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
            except Exception:
                print(f"-\t-\t-\t-\tours-failed\t{path.name}")
                continue
            ourpdf = next(ourdir.glob("*.pdf"), None)
        a = next((work / "float").glob("*.pdf"), None)
        b = next((work / "flow").glob("*.pdf"), None)
        if ourpdf is None or a is None or b is None:
            print(f"-\t-\t-\t-\tmissing\t{path.name}")
            continue
        ours = spans(ourpdf)
        df, nf = distance(ours, spans(a))
        dw, nw = distance(ours, spans(b))
        if df is None or dw is None:
            print(f"-\t-\t-\t-\tunpaired\t{path.name}")
            continue
        nearer = ("-" if abs(df - dw) < 0.01 and nf == nw
                  else "float" if df < dw else "FLOW")
        print(f"{df:.2f}\t{nf}\t{dw:.2f}\t{nw}\t{nearer}\t{path.name}", flush=True)


if __name__ == "__main__":
    main()
