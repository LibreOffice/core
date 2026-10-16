#!/usr/bin/env python3
"""Measure the gen2.py family, whose calibration is per default-font size.

A cell's text sits inside its row by the *default* font's metrics, so a family that varies
the default cell font needs one no-header probe per size rather than one for the family.

    measure2.py <probedir> <outdir> [--jobs N] [--cli <xray-office>]
"""
import concurrent.futures, hashlib, os, re, subprocess, sys

SOFFICE = "/opt/libreoffice26.2/program/soffice"


def render_ours(cli, src, out):
    h = "o" + hashlib.md5(src.encode()).hexdigest()
    d = os.path.join(out, h)
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src).rsplit(".", 1)[0] + ".pdf")
    if not os.path.exists(pdf):
        subprocess.run([cli, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True, timeout=600)
    return pdf if os.path.exists(pdf) else None


def render(src, out):
    h = hashlib.md5(src.encode()).hexdigest()
    d = os.path.join(out, h)
    os.makedirs(d, exist_ok=True)
    prof = "/tmp/lo-prof-" + h
    os.makedirs(prof, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src).rsplit(".", 1)[0] + ".pdf")
    if not os.path.exists(pdf):
        subprocess.run([SOFFICE, f"-env:UserInstallation=file://{prof}", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True, timeout=600)
    return pdf if os.path.exists(pdf) else None


def read(pdf):
    import pymupdf
    doc = pymupdf.open(pdf)
    page = doc[0]
    header, first = None, None
    for block in page.get_text("dict")["blocks"]:
        for line in block.get("lines", []):
            for span in line["spans"]:
                y0, y1 = span["bbox"][1], span["bbox"][3]
                t = span["text"].strip()
                if t.startswith("H") and (header is None or y0 < header[0]):
                    header = (y0, y1)
                if t == "R1":
                    first = y0
    doc.close()
    return first, header


def main():
    probes, out = sys.argv[1], sys.argv[2]
    jobs = int(sys.argv[sys.argv.index("--jobs") + 1]) if "--jobs" in sys.argv else 2
    cli = sys.argv[sys.argv.index("--cli") + 1] if "--cli" in sys.argv else None
    os.makedirs(out, exist_ok=True)
    files = sorted(f for f in os.listdir(probes) if f.endswith(".fods"))

    def one(f):
        src = os.path.join(probes, f)
        pdf = render(src, out)
        ref = read(pdf) if pdf else (None, None)
        ours = (None, None)
        if cli:
            o = render_ours(cli, src, out)
            ours = read(o) if o else (None, None)
        return f, ref, ours

    got, mine = {}, {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for f, r, o in pool.map(one, files):
            got[f] = r
            mine[f] = o

    ocal = {}
    for f, (row, _) in mine.items():
        m = re.match(r"band-none(\d+)\.fods$", f)
        if m and row is not None:
            ocal[int(m.group(1))] = row

    cal = {}
    for f, (row, _) in got.items():
        m = re.match(r"band-none(\d+)\.fods$", f)
        if m and row is not None:
            cal[int(m.group(1))] = row
    print("# calibration (default size -> first-row top with no header):", cal)
    print("probe\trefRow\tband\thdrTop\thdrBot\thdrH\ttext\toursBand\tdelta")
    for f in sorted(got):
        if f.startswith("band-none"):
            continue
        row, hdr = got[f]
        name = f[5:-5]
        # `mixed` and the two named-face probes all state a 20 pt default; everything else
        # carries its default size in its own name.
        if name == "mixed" or name.startswith(("dserif", "ddejavu")):
            dsize = 20
        else:
            m = re.search(r"d(\d+)", name)
            dsize = int(m.group(1)) if m else 10
        base = cal.get(dsize)
        band = row - base if row is not None and base is not None else None
        orow = mine[f][0]
        oband = orow - ocal[dsize] if orow is not None and dsize in ocal else None
        cells = [name,
                 f"{row:.3f}" if row is not None else "-",
                 f"{band:.3f}" if band is not None else "-",
                 f"{hdr[0]:.3f}" if hdr else "-",
                 f"{hdr[1]:.3f}" if hdr else "-",
                 f"{hdr[1] - hdr[0]:.3f}" if hdr else "-",
                 f"{band - 28.346:.3f}" if band is not None else "-",
                 f"{oband:.3f}" if oband is not None else "-",
                 f"{oband - band:.3f}" if oband is not None and band is not None else "-"]
        print("\t".join(cells))


if __name__ == "__main__":
    main()
