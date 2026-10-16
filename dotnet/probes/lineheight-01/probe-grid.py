#!/usr/bin/env python3
"""LibreOffice's ascent and line height for 195 (face, size) pairs, and the grid model's fit.

One page per (face, size) with two lines in it, so the first baseline's distance below the top
margin *is* the ascent and the gap between the two baselines *is* the line height. Both are read
out of the reference PDF's own text matrices — `pdftotext -bbox` reports the ink box, which moves
with whichever glyphs a line holds and cannot settle a one-twip question.

The model under test is `MSO1`, LibreOffice's own name for the reference device Writer formats
against:

    H  = size_twips * 6                          8640 dpi, MapTwip: one twip is six pixels
    a  = round(hheaAsc  * H / upem)              whole device pixels, separately
    d  = round(-hheaDesc* H / upem)
    g  = round(lineGap  * H / upem)
    lineHeight = round((a + d) / 6) + round(g / 6)      2 + 1, not 3 and not 1
    ascent     = round(a / 6)      + round(g / 6)       Writer charges leading to the ascent

`round` is half away from zero throughout, which is what C++ `std::round`/`llround` give.

Usage:  probe-grid.py <outdir>          measure the reference and score the model
        XRAY_OFFICE_CLI=... probe-grid.py <outdir>   also score the tree under test
"""
import os, subprocess, sys, re, struct, math

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "words-regress-01"))
from mkdocx import write

REPO = "/c/sandbox/workdir/libreoffice-core"
OPS = os.path.join(REPO, ".claude/skills/render-comparison/scripts/pdf-ops.py")
OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
os.makedirs(OUT, exist_ok=True)

L = "/usr/share/fonts/truetype/liberation/"
X = "/usr/share/fonts/truetype/crosextra/"
D = "/usr/share/fonts/truetype/dejavu/"

# (label, w:rFonts family, extra run properties, the file LibreOffice resolves that to)
SETS = {
    # The five faces the 195-pair table of `words-regress-01` was measured on.
    "core": [
        ("Liberation Serif", "Liberation Serif", "", L + "LiberationSerif-Regular.ttf"),
        ("Liberation Sans", "Liberation Sans", "", L + "LiberationSans-Regular.ttf"),
        ("Carlito", "Carlito", "", X + "Carlito-Regular.ttf"),
        ("Caladea", "Caladea", "", X + "Caladea-Regular.ttf"),
        ("DejaVu Sans", "DejaVu Sans", "", D + "DejaVuSans.ttf"),
    ],
    # Faces neither prior round touched: other cuts, other ems, a symbol face, and a CJK face
    # whose hhea and Windows metrics differ by 7.6% of the em.
    "extra": [
        ("Liberation Mono", "Liberation Mono", "", L + "LiberationMono-Regular.ttf"),
        ("Lib Serif Bold", "Liberation Serif", "<w:b/>", L + "LiberationSerif-Bold.ttf"),
        ("Lib Sans Italic", "Liberation Sans", "<w:i/>", L + "LiberationSans-Italic.ttf"),
        ("Caladea Bold", "Caladea", "<w:b/>", X + "Caladea-Bold.ttf"),
        ("DejaVu Serif", "DejaVu Serif", "", D + "DejaVuSerif.ttf"),
        ("DejaVu Sans Mono", "DejaVu Sans Mono", "", D + "DejaVuSansMono.ttf"),
        ("OpenSymbol", "OpenSymbol", "", "/usr/share/fonts/truetype/libreoffice/opens___.ttf"),
        ("IPAGothic", "IPAGothic", "", "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf"),
    ],
}
SET = os.environ.get("PROBE_SET", "core")
FONTS = [f[0] for f in SETS[SET]]
FAMILY = {f[0]: f[1] for f in SETS[SET]}
RPR = {f[0]: f[2] for f in SETS[SET]}
FILES = {f[0]: f[3] for f in SETS[SET]}
SIZES = list(range(10, 49))          # half-points, 5.0 .. 24.0 pt
TOP = 720.0                          # body top: 792 pt page less a 72 pt margin


# ---------------------------------------------------------------- font tables

def tables(path):
    d = open(path, "rb").read()
    off = 0
    if d[:4] == b"ttcf":
        off, = struct.unpack(">I", d[12:16])
    num, = struct.unpack(">H", d[off + 4:off + 6])
    t = {}
    for i in range(num):
        rec = d[off + 12 + 16 * i: off + 12 + 16 * i + 16]
        o, l = struct.unpack(">II", rec[8:16])
        t[rec[:4].decode("latin1")] = (o, l)
    return d, t


def metrics(path):
    """The three numbers `ImplCalcLineSpacing` ends up believing, and the em they are in."""
    d, t = tables(path)
    ho, _ = t["head"]
    upem, = struct.unpack(">H", d[ho + 18:ho + 20])
    hh, _ = t["hhea"]
    asc, desc, gap = struct.unpack(">hhh", d[hh + 4:hh + 10])
    a, dsc, g = asc, -desc, gap
    if "OS/2" in t:
        oo, _ = t["OS/2"]
        fs, = struct.unpack(">H", d[oo + 62:oo + 64])
        tA, tD, tG = struct.unpack(">hhh", d[oo + 68:oo + 74])
        if (fs >> 7) & 1 and tA >= 0 and tD <= 0:
            a, dsc, g = tA, -tD, tG
    return upem, a, dsc, g


# ---------------------------------------------------------------- the model

def rnd(x):
    """C++ std::round: half away from zero."""
    return math.floor(x + 0.5) if x >= 0 else -math.floor(-x + 0.5)


def model(face, halfpoints, dpi=8640):
    upem, a, d, g = metrics(FILES[face])
    tw = halfpoints * 10                       # half-points -> twips
    px_per_twip = dpi / 1440.0
    H = rnd(tw * px_per_twip)
    ap, dp, gp = (rnd(m * H / upem) for m in (a, d, g))
    asc = rnd(ap / px_per_twip) + rnd(gp / px_per_twip)
    hgt = rnd((ap + dp) / px_per_twip) + rnd(gp / px_per_twip)
    return asc, hgt


# ---------------------------------------------------------------- measurement

body = []
for fi, fam in enumerate(FONTS):
    for hp in SIZES:
        f = FAMILY[fam]
        rpr = (f'<w:rFonts w:ascii="{f}" w:hAnsi="{f}" w:eastAsia="{f}"/>'
               f'<w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/>{RPR[fam]}')
        for i in range(2):
            brk = '<w:r><w:br w:type="page"/></w:r>' if (i == 0 and body) else ''
            body.append(f'<w:p><w:pPr><w:rPr>{rpr}</w:rPr></w:pPr>{brk}'
                        f'<w:r><w:rPr>{rpr}</w:rPr><w:t>F{fi}S{hp}L{i}</w:t></w:r></w:p>')

src = write(os.path.join(OUT, "grid.docx"), "".join(body))


def render(who):
    d = os.path.join(OUT, who)
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, "grid.pdf")
    if os.path.exists(pdf):
        return pdf
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d], capture_output=True)
    else:
        subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src], capture_output=True)
    return pdf


def read(pdf):
    """Every page's (ascent, line height) in twips, from one whole-document dump.

    Keyed by **page number**, not by the label the text says: above 14 pt the subset font's
    ToUnicode map defeats `pdf-ops`' literal decoder and the record carries a glyph count and no
    string. The page order is authored here, so the page number is the reliable key — and it is
    checked against the size the record reports, which is always present.
    """
    out = subprocess.run(["python3", OPS, "dump", pdf, "--only", "text"],
                         capture_output=True, text=True).stdout
    rows = {}
    for l in out.splitlines():
        m = re.search(r'^text\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt', l)
        if m:
            rows.setdefault(int(m.group(1)), []).append((float(m.group(3)), float(m.group(4))))
    got = {}
    for fi, fam in enumerate(FONTS):
        for si, hp in enumerate(SIZES):
            page = fi * len(SIZES) + si + 1
            ys = sorted(rows.get(page, []), key=lambda r: -r[0])
            if len(ys) != 2 or abs(ys[0][1] - hp / 2.0) > 0.01:
                continue
            got[f"F{fi}S{hp}"] = (round((TOP - ys[0][0]) * 20), round((ys[0][0] - ys[1][0]) * 20))
    return got


ref = read(render("ref"))
ours = read(render("ours")) if CLI else {}

hdr = f"{'face':>16} {'pt':>5} | {'ref asc':>8} {'ref h':>6} | {'mod asc':>8} {'mod h':>6} |"
if ours:
    hdr += f" {'our asc':>8} {'our h':>6} |"
print(hdr + " flags")

nh = na = n = 0
misses = []
for fi, fam in enumerate(FONTS):
    for hp in SIZES:
        k = f"F{fi}S{hp}"
        if k not in ref:
            continue
        ra, rh = ref[k]
        ma, mh = model(fam, hp)
        n += 1
        nh += (mh == rh)
        na += (ma == ra)
        flag = ("" if ma == ra else f"ASC{ma - ra:+d} ") + ("" if mh == rh else f"H{mh - rh:+d}")
        if flag:
            misses.append((fam, hp / 2.0, ra, rh, ma, mh))
        line = f"{fam:>16} {hp / 2.0:>5.1f} | {ra:>8d} {rh:>6d} | {ma:>8d} {mh:>6d} |"
        if ours:
            oa, oh = ours.get(k, (0, 0))
            line += f" {oa:>8d} {oh:>6d} |"
            flag += ("" if oa == ra else " ourASC") + ("" if oh == rh else " ourH")
        print(line + " " + flag)

print(f"\nMODEL: line height {nh}/{n} exact, ascent {na}/{n} exact")
if ours:
    oh = sum(1 for fi, fam in enumerate(FONTS) for hp in SIZES
             if f"F{fi}S{hp}" in ref and ours.get(f"F{fi}S{hp}", (0, 0))[1] == ref[f"F{fi}S{hp}"][1])
    oa = sum(1 for fi, fam in enumerate(FONTS) for hp in SIZES
             if f"F{fi}S{hp}" in ref and ours.get(f"F{fi}S{hp}", (0, 0))[0] == ref[f"F{fi}S{hp}"][0])
    print(f"TREE:  line height {oh}/{n} exact, ascent {oa}/{n} exact")
for m in misses:
    print("  miss", m)
