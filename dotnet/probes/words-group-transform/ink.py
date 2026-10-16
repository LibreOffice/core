"""First-page ink: the mean absolute difference of two renderings at 30 dpi, greyscale."""
import subprocess, sys, tempfile, os
import numpy as np
from PIL import Image

def page(pdf, out):
    subprocess.run(["pdftoppm", "-r", "30", "-f", "1", "-l", "1", "-gray", "-png", pdf, out],
                   check=True, capture_output=True)
    # pdftoppm pads the page number to the width of the document's page count, so a hundred-page
    # file writes `a-001.png` and a short one `a-1.png`. Assuming the short form silently drops
    # every long document from the measurement -- 70 of 337 on the words track.
    folder, stem = os.path.split(out)
    made = [f for f in os.listdir(folder or ".") if f.startswith(stem + "-")]
    if not made:
        raise FileNotFoundError(out)
    return os.path.join(folder, sorted(made)[0])

def ink(a, b):
    with tempfile.TemporaryDirectory() as tmp:
        try:
            ia = np.asarray(Image.open(page(a, os.path.join(tmp, "a"))).convert("L")).astype(float)
            ib = np.asarray(Image.open(page(b, os.path.join(tmp, "b"))).convert("L")).astype(float)
        except Exception:
            return None
    h = min(ia.shape[0], ib.shape[0]); w = min(ia.shape[1], ib.shape[1])
    if h == 0 or w == 0: return None
    return float(np.abs(ia[:h, :w] - ib[:h, :w]).mean())

old_dir, new_dir = sys.argv[1], sys.argv[2]
names = sys.argv[3:]
if not names:
    names = sorted(os.listdir(os.path.join(new_dir, "ours")))
rows = []
for name in names:
    o = ink(os.path.join(old_dir, "ours", name), os.path.join(old_dir, "ref", name))
    n = ink(os.path.join(new_dir, "ours", name), os.path.join(new_dir, "ref", name))
    if o is None or n is None: continue
    rows.append((n - o, o, n, name))
rows.sort()
for d, o, n, name in rows:
    if abs(d) > 0.005:
        print(f"  {o:8.3f} -> {n:8.3f}  {d:+8.3f}  {name}")
oldm = sum(r[1] for r in rows) / len(rows)
newm = sum(r[2] for r in rows) / len(rows)
print(f"{len(rows)} documents   mean ink {oldm:.3f} -> {newm:.3f}")
