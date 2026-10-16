#!/usr/bin/env python3
"""Score the gate's mismatches against BOTH references.

The corpus gate measures against /usr/bin/soffice, which is 24.2.7.2, while the
tree is calibrated to 26.2.4.2.  A row that fails against 24.2 and passes
against 26.2 is the version gap, not a defect; a row where the two references
disagree with *each other* is a document that needs reading rather than scoring.

Reads the stored gate parity.tsv (ours + ref24) and the rows26.tsv this round's
screen26.sh produced (ref26), and applies batch-check.sh's own verdict rule to
each pair.
"""
import pathlib
import sys

GATE = pathlib.Path("/home/user/gate-2f47/parity.tsv")
R26 = pathlib.Path("/home/user/mismatch-work/s26/rows26.tsv")


def ident(rel: str) -> str:
    base = rel.rsplit("/", 1)[-1]
    stem, _, ext = base.rpartition(".")
    return f"{stem}__{ext.lower()}"


def verdict(op, rp, og, rg):
    """batch-check.sh's rule: page count, then max(2%, 15 chars) of glyphs."""
    v = []
    if op != rp:
        v.append("pages")
    if rg > 0:
        d = abs(og - rg)
        if d > rg * 0.02 and d > 15:
            v.append("words")
    elif og > 15:
        v.append("words")
    return ",".join(v) or "match"


ref26 = {}
for line in R26.read_text().splitlines():
    f = line.split("\t")
    if len(f) < 6 or f[1] == "-":
        ref26[f[0]] = None
        continue
    ref26[f[0]] = (int(f[1]), int(f[4]))

rows = []
for line in GATE.read_text().splitlines():
    if line.startswith("#") or line.startswith("path\t"):
        continue
    f = line.split("\t")
    rel, ext, pages, _w, _fo, _un, v24, _raw, glyphs = f[:9]
    if v24 == "match":
        continue
    op, rp = (int(x) for x in pages.split("/"))
    og, rg = (int(x) for x in glyphs.split("/"))
    i = ident(rel)
    r = ref26.get(i)
    if r is None:
        v26, p26, g26 = "ref26-failed", "-", "-"
        cross = "-"
    else:
        p26, g26 = r
        v26 = verdict(op, p26, og, g26)
        cross = verdict(rp, p26, rg, g26)
    rows.append((rel, ext, op, rp, p26, og, rg, g26, v24, v26, cross))

print("# ours = X-Ray.Office @ 2f4709c08; ref24 = /usr/bin/soffice 24.2.7.2;")
print("# ref26 = /opt/libreoffice26.2 26.2.4.2 with the 8 Latin duplicate faces moved aside")
print("# fonts: Carlito+Caladea+Liberation+DejaVu installed; XRAY_OFFICE_BUNDLED_FONTS unset")
print("# verdict rule = batch-check.sh 2026-09-05 (pages, then max(2%,15) alnum chars)")
print("\t".join(["path", "ext", "pages_ours", "pages_ref24", "pages_ref26",
                 "glyphs_ours", "glyphs_ref24", "glyphs_ref26",
                 "v_vs24", "v_vs26", "ref24_vs_ref26"]))
for r in rows:
    print("\t".join(str(x) for x in r))

if "--summary" in sys.argv:
    import collections
    c = collections.Counter((r[9], r[10]) for r in rows)
    print("\n# v_vs26 x ref24_vs_ref26", file=sys.stderr)
    for k, n in c.most_common():
        print(f"{n:4d}  vs26={k[0]:<12} refs_disagree={k[1]}", file=sys.stderr)
