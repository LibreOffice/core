#!/usr/bin/env python3
"""A structural height model for OMML, in StarMath's own terms and units.

Units are 1/100 mm integers throughout, which is what StarMath computes in
(`SmO3tlLengthUnit`), so the integer divisions below are the reference's own.
Every constant is either read out of `starmath/source/format.cxx` (the `SmFormat`
defaults) or measured against 26.2.4.2 by `read-heights.py`.

Only the *vertical* half is modelled: this exists to answer how tall a line
holding a formula is, which is the question `svg:height` answers.
"""
import math

PT = 2540.0 / 72.0          # 1/100 mm per point
BASE = int(round(12 * PT))  # SmFormat's base size, 12 pt

# SmFormat defaults, format.cxx:36-60 (percentages)
SIZ_INDEX, SIZ_LIMITS = 60, 60
DIS_VERTICAL, DIS_SUPERSCRIPT, DIS_SUBSCRIPT = 5, 20, 20
DIS_FRACTION, DIS_STROKEWIDTH = 10, 5
DIS_UPPERLIMIT, DIS_LOWERLIMIT = 0, 0
DIS_BRACKETSPACE, DIS_BRACKETSIZE = 5, 5
DIS_MATRIXROW = 3
DIS_OPERATORSIZE = 50
DIS_ROOT = 0

# Measured against 26.2.4.2 (`read-heights.py`): a single row of text at the
# 12 pt base is 471, and its ascent follows from the `sup` arm.
HEIGHT_RATIO = 471.0 / BASE
ASCENT_RATIO = 376.0 / BASE


class R:
    """The vertical half of an `SmRect`."""
    __slots__ = ("top", "bot", "alignT", "alignB", "base", "hasBase")

    def __init__(self, top, bot, alignT, alignB, base, hasBase):
        self.top, self.bot = top, bot
        self.alignT, self.alignB, self.base, self.hasBase = alignT, alignB, base, hasBase

    @property
    def h(self):
        return self.bot - self.top

    def moved(self, dy):
        return R(self.top + dy, self.bot + dy, self.alignT + dy, self.alignB + dy,
                 self.base + dy, self.hasBase)

    def extend(self, o, mode="none", keep=False):
        """`SmRect::ExtendBy` — union, with the baseline decided by `mode`."""
        t, b = min(self.top, o.top), max(self.bot, o.bot)
        aT, aB = min(self.alignT, o.alignT), max(self.alignB, o.alignB)
        bl, hb = self.base, self.hasBase
        if mode == "arg":
            bl, hb = o.base, o.hasBase
        elif mode == "none":
            hb = False
        if keep:
            aT, aB, bl, hb = self.alignT, self.alignB, self.base, self.hasBase
        return R(t, b, aT, aB, bl, hb)


def leaf(scale=1.0, hr=None):
    """A row of ordinary maths text at `scale` of the base size."""
    H = int(BASE * scale)
    h = int(round(H * (hr if hr is not None else HEIGHT_RATIO)))
    a = int(round(H * ASCENT_RATIO))
    return R(0, h, a - H * 750 // 1000, a, a, True), H


def blank(h):
    """`SmRect(nWidth, nHeight)` — the rule in a fraction. No baseline."""
    return R(0, h, 0, h, 0, False)


# ---- the constructs ------------------------------------------------------

def hor(parts):
    """`SmLineNode` / `SmBinHorNode`: one row, aligned on baselines."""
    out = None
    for p in parts:
        if out is None:
            out = p
            continue
        dy = (out.base - p.base) if (out.hasBase and p.hasBase) else \
             ((out.alignT + out.alignB) // 2 - (p.alignT + p.alignB) // 2)
        out = out.extend(p.moved(dy), "this")
    return out


def frac(num, den, H):
    """`SmBinVerNode::Arrange` — node.cxx:831."""
    thick = H * DIS_STROKEWIDTH // 100
    border = H // 20
    line = blank(thick + 2 * border)
    line = line.moved(num.bot - line.top)
    d = den.moved(line.bot - den.top)
    out = num.extend(d, "none").extend(line, "none")
    return out


def script(body, H, sub=None, sup=None, presub=None, presup=None, csub=None, csup=None):
    """`SmSubSupNode::Arrange` — node.cxx:1143. `nOrigHeight` is the body's own."""
    out = body
    tmp = body
    for kind, s in (("csub", csub), ("csup", csup), ("rsub", sub), ("rsup", sup),
                    ("rsub", presub), ("rsup", presup)):
        if s is None:
            continue
        delim = out.alignB + int(0.4 * (out.alignT - out.alignB))
        if kind in ("csub", "csup"):
            dist = H * (DIS_LOWERLIMIT if kind == "csub" else DIS_UPPERLIMIT) // 100
            if kind == "csub":
                y = body.bot - s.top + dist
            else:
                y = body.top - s.h - s.top - dist
            out = out.extend(s.moved(y), "this", keep=True)
            tmp = out
            continue
        dist = H * (DIS_SUBSCRIPT if kind == "rsub" else DIS_SUPERSCRIPT) // 100
        if kind == "rsub":
            y = tmp.alignB - s.alignB + dist
            if y < delim:
                y = delim
        else:
            y = tmp.alignT - s.alignT - dist
            if y + s.h > delim:
                y = delim - s.h
        out = out.extend(s.moved(y), "this", keep=True)
    return out


def root(body, H, deg=None):
    """`SmRootNode::Arrange` — node.cxx:725, with `lcl_GetHeightVerOffset`."""
    # The sign is as tall as the body less half the body's own descender, plus the
    # tenth `SmRootSymbolNode::AdaptToY` adds (node.cxx:1765), and its foot sits that
    # same half-descender above the body's.
    off = (body.bot - 1 - body.alignB) // 2
    height = body.h - off + DIS_ROOT * H // 100
    sym = blank(height + height // 10)
    out = body.extend(sym.moved(body.bot - 1 - off - sym.bot + 1), "this")
    if deg is not None:
        # `lcl_GetExtraPos`: the degree's foot sits 52 % down the sign.
        y = sym.h * 52 // 100 - deg.h
        out = out.extend(deg.moved(out.top + y - deg.top), "this", keep=True)
    return out


def brace(body, H):
    """`SmBraceNode::Arrange` — node.cxx:1256. A scaled bracket oversizes its body."""
    h = body.h + 2 * (body.h * DIS_BRACKETSIZE // 100)
    sym = blank(h)
    centre = (body.top + body.bot) // 2
    return body.extend(sym.moved(centre - h // 2), "this")


def table(rows, H):
    """`SmTableNode::Arrange` — node.cxx:491. One column, `DIS_VERTICAL` apart."""
    dist = DIS_VERTICAL * H // 100
    out = None
    y = 0
    for i, r in enumerate(rows):
        if i:
            y += dist
        m = r.moved(y - r.top)
        y = m.bot
        out = m if out is None else out.extend(m, "none")
    return out


def matrix(rows, H):
    """`SmMatrixNode::Arrange` — node.cxx:1941. `nNormDist` is three font heights."""
    dist = (3 * H) * DIS_MATRIXROW // 100
    out = None
    y = 0
    for i, r in enumerate(rows):
        if i:
            y += dist
        m = r.moved(y - r.top)
        y = m.bot
        out = m if out is None else out.extend(m, "none")
    return out
