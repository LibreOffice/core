#!/usr/bin/env python3
"""Two more 24.2.7.2 re-checks: the ### threshold and the 720 dpi device round trip."""
import os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from audit_mkwb import workbook
from audit_colwidth import render_ref, render_ours, BASE

def words(pdf):
    if not os.path.exists(pdf): return []
    out = subprocess.run(["pdftotext", "-bbox", pdf, "-"], capture_output=True).stdout.decode("utf8", "replace")
    return [(float(m.group(1)), float(m.group(3)), m.group(4)) for m in re.finditer(
        r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="[\d.]+">([^<]*)</word>', out)]

print("=== SheetGeneralWidth: the width at which a number becomes ### ===")
print(f"{'font':>18} {'sz':>3} {'width':>6} {'ref':>14} {'ours':>14} {'agree':>6}")
bad = 0; n = 0
for font, size in [("Calibri", 11), ("Liberation Sans", 11), ("Calibri", 14)]:
    for width in [3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 7.0, 8.0]:
        name = f"gw_{font.replace(' ','')}_{size}_{width}".replace('.', 'p') + ".xlsx"
        p = workbook(os.path.join(BASE, name), font=font, size=size,
                     cols=[(1, 1, width)], rows=[(1, [("A", "n", "123456.789")])])
        r = [w[2] for w in words(render_ref(p))]
        o = [w[2] for w in words(render_ours(p))]
        rt = " ".join(r)[:14]; ot = " ".join(o)[:14]
        n += 1
        agree = rt == ot
        if not agree: bad += 1
        print(f"{font:>18} {size:>3} {width:>6} {rt:>14} {ot:>14} {str(agree):>6}")
print(f"cases {n}, disagreeing {bad}")

print()
print("=== SheetDeviceUnits: drawn text width across sizes (the 720 dpi round trip) ===")
print(f"{'font':>18} {'sz':>5} {'ref w':>9} {'ours w':>9} {'delta':>8}")
bad2 = 0; n2 = 0
for font in ["Calibri", "Liberation Sans", "Times New Roman"]:
    for size in [6, 7.5, 8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 36, 48, 72]:
        name = f"du_{font.replace(' ','')}_{size}".replace('.', 'p') + ".xlsx"
        p = workbook(os.path.join(BASE, name), font=font, size=size,
                     cols=[(1, 1, 90.0)], rows=[(1, [("A", "s", "HAMBURGEFONSTIV")])])
        rw = [w for w in words(render_ref(p)) if w[2] == "HAMBURGEFONSTIV"]
        ow = [w for w in words(render_ours(p)) if w[2] == "HAMBURGEFONSTIV"]
        if not rw or not ow:
            print(f"{font:>18} {size:>5} {'-':>9} {'-':>9} {'MISSING':>8}"); bad2 += 1; n2 += 1; continue
        r = rw[0][1] - rw[0][0]; o = ow[0][1] - ow[0][0]
        n2 += 1
        if abs(o - r) > 0.5: bad2 += 1
        print(f"{font:>18} {size:>5} {round(r,2):>9} {round(o,2):>9} {round(o-r,3):>8}")
print(f"cases {n2}, outside 0.5 pt {bad2}")
