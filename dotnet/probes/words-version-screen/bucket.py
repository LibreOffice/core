"""Screen the catalogued overlap/clipping bucket against both reference versions."""
import json, os, subprocess, tempfile
from pathlib import Path
import numpy as np
from PIL import Image

SP = Path("/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad")
OUT = SP / "bucket"; OUT.mkdir(exist_ok=True)
CORPUS = Path("/home/user/sample-files")
CLI = os.environ["XRAY_OFFICE_CLI"]

def page(pdf, n, dpi=30):
    with tempfile.TemporaryDirectory() as t:
        subprocess.run(["pdftoppm", "-r", str(dpi), "-f", str(n), "-l", str(n), "-gray", "-png",
                        str(pdf), os.path.join(t, "p")], capture_output=True)
        m = sorted(os.listdir(t))
        return None if not m else np.asarray(Image.open(os.path.join(t, m[0])).convert("L")).astype(float)

def ink(a, b, n):
    x, y = page(a, n), page(b, n)
    if x is None or y is None: return None
    h, w = min(x.shape[0], y.shape[0]), min(x.shape[1], y.shape[1])
    return round(float(np.abs(x[:h, :w] - y[:h, :w]).mean()), 2)

def pages(pdf):
    r = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True)
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)

def render(binary, src, folder):
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / (src.stem + ".pdf")
    if not out.exists():
        subprocess.run([binary, f"-env:UserInstallation=file://{folder / 'prof'}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(folder), str(src)],
                       capture_output=True, timeout=900)
    return out

readings = json.load(open("/home/user/libreoffice-core/dotnet/scripts/renderer-parity/pl-readings.json"))
rows = [x for x in readings if x.get("outcome") == "open"
        and x.get("outcome_why", "").startswith("overlap")]

print(f"{'#':>4} {'p':>2} {'ink24':>6} {'ink26':>6} {'pages 24/26/ours':>17}  document")
for x in sorted(rows, key=lambda r: r["rank"]):
    src = next((p for p in CORPUS.rglob(x["name"]) if p.is_file()), None)
    if src is None:
        print(f"{x['rank']:>4} {'':>2} {'-':>6} {'-':>6} {'missing':>17}  {x['name'][:44]}"); continue
    here = OUT / str(x["rank"])
    old = render("soffice", src, here / "o")
    new = render("/opt/libreoffice26.2/program/soffice", src, here / "n")
    subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(here / "p")],
                   capture_output=True, timeout=900)
    ours = here / "p" / (src.stem + ".pdf")
    if not (old.exists() and ours.exists()):
        print(f"{x['rank']:>4} {'':>2} {'-':>6} {'-':>6} {'render failed':>17}  {x['name'][:44]}"); continue
    n = min(x.get("page", 1) or 1, pages(old), pages(ours))
    v24 = ink(old, ours, n)
    v26 = ink(new, ours, n) if new.exists() else None
    pp = f"{pages(old)}/{pages(new) if new.exists() else '-'}/{pages(ours)}"
    print(f"{x['rank']:>4} {n:>2} {str(v24):>6} {str(v26):>6} {pp:>17}  {x['name'][:44]}", flush=True)
