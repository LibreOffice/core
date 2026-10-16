"""Which documents our binary now renders differently from the gate's binary.

Compares the two sweeps' *own* renderings page by page as images, because the PDFs differ
byte for byte on their creation date alone. A document whose pages are pixel-identical did
not move; anything else did, and is a candidate for a closer look.
"""
import os, subprocess, sys, tempfile, hashlib
from pathlib import Path

A = Path(sys.argv[1])   # before, e.g. /home/user/gate-2f47/ours
B = Path(sys.argv[2])   # after
ONLY = sys.argv[3] if len(sys.argv) > 3 else ""

def digest(pdf, dpi=40):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-gray", "-png", str(pdf),
                        os.path.join(t, "p")], capture_output=True)
        h = hashlib.sha256()
        names = sorted(os.listdir(t))
        for n in names:
            h.update(open(os.path.join(t, n), "rb").read())
        return len(names), h.hexdigest()

for name in sorted(os.listdir(B)):
    if not name.endswith(".pdf"): continue
    if ONLY and ONLY not in name: continue
    a, b = A / name, B / name
    if not a.exists():
        print(f"NEW\t{name}"); continue
    na, ha = digest(a)
    nb, hb = digest(b)
    if ha != hb:
        print(f"MOVED\t{name}\t{na}->{nb} pages")
        sys.stdout.flush()
