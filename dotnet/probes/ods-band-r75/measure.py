#!/usr/bin/env python3
"""Render every probe through 26.2.4.2 and read the band off the first cell row.

    measure.py <probedir> <outdir> [--cli <xray-office>] [--jobs N]

Prints one row per probe: the top of the first cell row's text, the top and bottom of the
header's own ink, and the band, which is the first row's shift against the `band-none` probe.

One directory per *document*, never per worker slot: a thread pool does not work consecutive
indices, so two renders otherwise land in one directory and one deletes the other's output.
"""
import concurrent.futures, hashlib, os, subprocess, sys

SOFFICE = "/opt/libreoffice26.2/program/soffice"


def render_ref(src, out):
    d = os.path.join(out, hashlib.md5(src.encode()).hexdigest())
    os.makedirs(d, exist_ok=True)
    prof = "/tmp/lo-prof-" + hashlib.md5(src.encode()).hexdigest()
    os.makedirs(prof, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src).rsplit(".", 1)[0] + ".pdf")
    if not os.path.exists(pdf):
        subprocess.run([SOFFICE, f"-env:UserInstallation=file://{prof}", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True, timeout=600)
    return pdf if os.path.exists(pdf) else None


def render_ours(cli, src, out):
    d = os.path.join(out, "o" + hashlib.md5(src.encode()).hexdigest())
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src).rsplit(".", 1)[0] + ".pdf")
    if not os.path.exists(pdf):
        subprocess.run([cli, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True, timeout=600)
    return pdf if os.path.exists(pdf) else None


def read(pdf):
    import pymupdf
    doc = pymupdf.open(pdf)
    page = doc[0]
    header = None
    first = None
    for block in page.get_text("dict")["blocks"]:
        for line in block.get("lines", []):
            for span in line["spans"]:
                x0, y0, x1, y1 = span["bbox"]
                t = span["text"].strip()
                if t.startswith("H"):
                    if header is None or y0 < header[0]:
                        header = (y0, y1, span["size"])
                if t == "R1":
                    first = y0
    doc.close()
    return first, header


def main():
    probes, out = sys.argv[1], sys.argv[2]
    cli = sys.argv[sys.argv.index("--cli") + 1] if "--cli" in sys.argv else None
    jobs = int(sys.argv[sys.argv.index("--jobs") + 1]) if "--jobs" in sys.argv else 2
    os.makedirs(out, exist_ok=True)
    files = sorted(f for f in os.listdir(probes) if f.endswith(".fods"))

    def one(f):
        src = os.path.join(probes, f)
        r = render_ref(src, out)
        got = read(r) if r else (None, None)
        ours = None
        if cli:
            o = render_ours(cli, src, out)
            ours = read(o) if o else (None, None)
        return f, got, ours

    rows = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for f, got, ours in pool.map(one, files):
            rows.append((f, got, ours))

    base = next((g[0] for f, g, _ in rows if f == "band-none.fods"), None)
    obase = next((o[0] for f, _, o in rows if f == "band-none.fods" and o), None)
    print(f"# calibration: ref first-row top with no header = {base}")
    print("probe\trefRow\trefBand\thdrTop\thdrBot\thdrH\toursRow\toursBand")
    for f, (row, hdr), ours in sorted(rows):
        band = f"{row - base:.3f}" if row is not None and base is not None else "-"
        ht = f"{hdr[0]:.3f}" if hdr else "-"
        hb = f"{hdr[1]:.3f}" if hdr else "-"
        hh = f"{hdr[1] - hdr[0]:.3f}" if hdr else "-"
        orow = f"{ours[0]:.3f}" if ours and ours[0] is not None else "-"
        oband = (f"{ours[0] - obase:.3f}"
                 if ours and ours[0] is not None and obase is not None else "-")
        rr = f"{row:.3f}" if row is not None else "-"
        print(f"{f[5:-5]}\t{rr}\t{band}\t{ht}\t{hb}\t{hh}\t{orow}\t{oband}")


if __name__ == "__main__":
    main()
