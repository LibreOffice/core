#!/usr/bin/env python3
"""The two corpus witnesses of the 18.46 pt body offset, measured token by token.

Round 56 found these two by handing a composed page to a blind reviewer and then confirming with
`pdftotext -bbox`.  This re-measures them against the stored 26.2.4.2 reference bank, and it
reports the offset as a *distribution over the page's shared tokens* rather than as the first
token's y, because a uniform translation and a scale error look identical in one token and not in
five hundred.
"""
import os, re, subprocess, sys

REF = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets"
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
OUT = "/c/sandbox/workdir/scratch-r57-sheets/witness"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

CASES = [
    ("sheets/done-014/xlsx/fm-provider-service-measures.xlsx",
     "fm-provider-service-measures__xlsx.pdf", 36),
    ("sheets/done-011/xlsx/FY2023-AIP-grants.xlsx", "FY2023-AIP-grants__xlsx.pdf", 1),
]


def words(pdf, page):
    out = subprocess.run(["pdftotext", "-q", "-bbox", "-f", str(page), "-l", str(page), pdf, "-"],
                         capture_output=True, text=True).stdout
    return [(m.group(5), float(m.group(1)), float(m.group(2)))
            for m in re.finditer(
                r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>',
                out)]


def main():
    os.makedirs(OUT, exist_ok=True)
    for path, refname, page in CASES:
        src = "/c/sandbox/workdir/sample-files/" + path
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", OUT],
                       capture_output=True, env=ENV)
        ours = os.path.join(OUT, os.path.splitext(os.path.basename(src))[0] + ".pdf")
        if not os.path.exists(ours):
            raise SystemExit("no PDF at " + ours)
        r = words(os.path.join(REF, refname), page)
        o = words(ours, page)
        # Pair on (text, rounded x): a uniform vertical translation leaves x alone, so x is a
        # key rather than an observable here.
        byref = {}
        for t, x, y in r:
            byref.setdefault((t, round(x)), []).append(y)
        deltas = []
        for t, x, y in o:
            k = (t, round(x))
            if k in byref and byref[k]:
                deltas.append(round(y - byref[k].pop(0), 3))
        deltas.sort()
        if not deltas:
            print("%-34s p%-3d NO PAIRED TOKENS (ref %d, ours %d)"
                  % (os.path.basename(path), page, len(r), len(o)))
            continue
        n = len(deltas)
        print("%-34s p%-3d paired %4d/%4d  min %7.3f  median %7.3f  max %7.3f"
              % (os.path.basename(path), page, n, len(r), deltas[0], deltas[n // 2], deltas[-1]))


if __name__ == "__main__":
    main()
