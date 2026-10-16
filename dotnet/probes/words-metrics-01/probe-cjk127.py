#!/usr/bin/env python3
"""Which value does Writer's CJK 127% scale multiply — the line height, or the ascent and descent?

`lineheight-01` §7(a) records the rule as `(gridded * 127) / 100` applied to the gridded ascent and
the gridded line height, and reports it exact on 39 of 39 IPAGothic pairs. **IPAGothic's hhea
lineGap is 0**, so on that face the leading term vanishes and two different rules give the same
answer for every size. This probe measures a face where it does not vanish.

    face                  upem   hheaAsc  hheaDesc  lineGap
    IPAGothic             2048      1802      -246        0
    WenQuanYi Zen Hei     1024       986      -304       92

Three candidates, all starting from `lineheight-01`'s grid (H = twips*6; a, d, g each rounded
separately to whole device pixels; halves away from zero):

    WHOLE    h = (round((a+d)/6) + round(g/6)) * 127 / 100          scale the finished height
    PARTS    h =  round((a*127/100 + d*127/100)/6) + round(g/6)     scale the device pixels
    TWIPS    h =  round((a+d)/6) * 127 / 100 + round(g/6)           scale before the leading

`/100` is integer division throughout, which is what the C++ does.

RESULT: TWIPS is exact on 117 of 117, in both the ascent and the line height, on all three faces.
WHOLE — the rule `lineheight-01` §7(a) states — is 39/39 on IPAGothic and **0/39** on WenQuanYi Zen
Hei. TWIPS is what `SwFntObj::GetFontHeight` does:

    nRet = lcl_ApplyCjkHeightAdjustment(m_nPrtHeight, pSh, rRefDev) + GetFontLeading(pSh, rRefDev);

the scale reaching the device's ascent-plus-descent and the leading being added after it, unscaled.

Usage:  probe-cjk127.py <outdir>
"""
import os, subprocess, sys, re, struct, math

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "words-regress-01"))
from mkdocx import write                                        # noqa: E402

REPO = "/c/sandbox/workdir/libreoffice-core"
OPS = os.path.join(REPO, ".claude/skills/render-comparison/scripts/pdf-ops.py")
OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

# (label, w:rFonts family, the file LibreOffice resolves it to, whether it declares a CJK codepage)
FACES = [
    ("WenQuanYi Zen Hei", "WenQuanYi Zen Hei",
     "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc", True),
    ("IPAGothic", "IPAGothic",
     "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf", True),
    # The control: a face with a non-zero line gap that declares no CJK code page at all, so the
    # 127% must not touch it. DejaVu Sans, gap 0 in OS/2 but 236 in hhea.
    ("Liberation Serif", "Liberation Serif",
     "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf", False),
]
NAMES = [f[0] for f in FACES]
FAMILY = {f[0]: f[1] for f in FACES}
FILES = {f[0]: f[2] for f in FACES}
SIZES = list(range(10, 49))          # half-points, 5.0 .. 24.0 pt
TOP = 720.0


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


def rnd(x):
    return math.floor(x + 0.5) if x >= 0 else -math.floor(-x + 0.5)


def scale(v):
    """(v * 127) / 100 with C++ integer division."""
    return (v * 127) // 100


def model(face, halfpoints, rule):
    upem, a, d, g = metrics(FILES[face])
    cjk = dict((f[0], f[3]) for f in FACES)[face]
    H = rnd(halfpoints * 10 * 6)
    ap, dp, gp = (rnd(m * H / upem) for m in (a, d, g))

    if cjk and rule == "PARTS":
        ap, dp = scale(ap), scale(dp)

    # The two terms `SwFntObj::GetFontAscent`/`GetFontHeight` add: the device's own ascent (or
    # ascent-plus-descent) in twips, and the leading. The 127% reaches only the first of them —
    # `lcl_ApplyCjkHeightAdjustment(m_nPrtHeight, …) + GetFontLeading(…)` — which is what makes
    # TWIPS a different rule from WHOLE for any face whose line gap is not zero.
    body, lead = rnd(ap / 6), rnd(gp / 6)
    bodyh = rnd((ap + dp) / 6)

    if cjk and rule == "TWIPS":
        body, bodyh = scale(body), scale(bodyh)

    asc = body + lead
    hgt = bodyh + lead

    if cjk and rule == "WHOLE":
        asc, hgt = scale(asc), scale(hgt)

    return asc, hgt


body = []
for fi, fam in enumerate(NAMES):
    for hp in SIZES:
        f = FAMILY[fam]
        rpr = (f'<w:rFonts w:ascii="{f}" w:hAnsi="{f}" w:eastAsia="{f}"/>'
               f'<w:sz w:val="{hp}"/><w:szCs w:val="{hp}"/>')
        for i in range(2):
            brk = '<w:r><w:br w:type="page"/></w:r>' if (i == 0 and body) else ''
            body.append(f'<w:p><w:pPr><w:rPr>{rpr}</w:rPr></w:pPr>{brk}'
                        f'<w:r><w:rPr>{rpr}</w:rPr><w:t>F{fi}S{hp}L{i}</w:t></w:r></w:p>')

src = write(os.path.join(OUT, "cjk127.docx"), "".join(body))
d = os.path.join(OUT, "ref")
os.makedirs(d, exist_ok=True)
pdf = os.path.join(d, "cjk127.pdf")
if not os.path.exists(pdf):
    subprocess.run(["soffice", "-env:UserInstallation=file://" + OUT + "/prof", "--headless",
                    "--convert-to", "pdf", "--outdir", d, src], capture_output=True)

out = subprocess.run(["python3", OPS, "dump", pdf, "--only", "text"],
                     capture_output=True, text=True).stdout
rows = {}
for line in out.splitlines():
    m = re.search(r'^text\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt', line)
    if m:
        rows.setdefault(int(m.group(1)), []).append((float(m.group(3)), float(m.group(4))))

ref = {}
for fi, fam in enumerate(NAMES):
    for si, hp in enumerate(SIZES):
        page = fi * len(SIZES) + si + 1
        ys = sorted(rows.get(page, []), key=lambda r: -r[0])
        if len(ys) != 2 or abs(ys[0][1] - hp / 2.0) > 0.01:
            continue
        ref[f"F{fi}S{hp}"] = (round((TOP - ys[0][0]) * 20), round((ys[0][0] - ys[1][0]) * 20))

RULES = ("WHOLE", "PARTS", "TWIPS")
print(f"{'face':>18} {'pt':>5} | {'ref asc':>7} {'ref h':>6} | "
      + " | ".join(f"{r + ' asc':>9} {r + ' h':>7}" for r in RULES))
score = {r: [0, 0, 0] for r in RULES}
per_face = {}
for fi, fam in enumerate(NAMES):
    for hp in SIZES:
        k = f"F{fi}S{hp}"
        if k not in ref:
            continue
        ra, rh = ref[k]
        got = {r: model(fam, hp, r) for r in RULES}
        for rule, (ma, mh) in got.items():
            score[rule][0] += (ma == ra)
            score[rule][1] += (mh == rh)
            score[rule][2] += 1
            e = per_face.setdefault((fam, rule), [0, 0, 0])
            e[0] += (ma == ra)
            e[1] += (mh == rh)
            e[2] += 1
        if any(got[r] != (ra, rh) for r in RULES):
            print(f"{fam:>18} {hp / 2:>5} | {ra:>7} {rh:>6} | "
                  + " | ".join(f"{got[r][0]:>9} {got[r][1]:>7}" for r in RULES))

print()
for rule in RULES:
    a, h, n = score[rule]
    print(f"{rule:>6}: ascent {a}/{n}   height {h}/{n}")
print()
for (fam, rule), (a, h, n) in sorted(per_face.items()):
    print(f"  {fam:>18} {rule:>6}: ascent {a}/{n}  height {h}/{n}")
