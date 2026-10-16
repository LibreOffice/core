#!/usr/bin/env python3
"""A control for the best-fit predicate: run it on cases whose answer the reference already gave.

`003_advanced_excel_pie`'s reference rendering says, unambiguously and from the outside, which of
its five labels fitted inside its slice: the four that are drawn on two lines near the pie fitted,
and the one drawn on one line beyond the rim did not — and the discarded inner attempt's legend
key, left behind on the page, marks it.  Those five outcomes are what this checks the port against
before any conclusion is drawn from a rendering.
"""
import math

def norm360(d):
    d = math.fmod(d, 360.0)
    return d + 360.0 if d < 0 else d


def between(ax, ay, bx, by):
    la = math.hypot(ax, ay)
    lb = math.hypot(bx, by)
    if la <= 0 or lb <= 0:
        return 0.0
    return math.degrees(math.acos(max(-1.0, min(1.0, (ax * bx + ay * by) / (la * lb)))))


def bestfit(bisector, sweep, radius, width, height, trace=False):
    half = sweep / 2.0
    ray = norm360(bisector)
    pie = radius * (1 - 0.025)
    if pie <= 0 or width <= 0 or height <= 0:
        return None

    alpha = norm360(ray + 45) - 45
    arad = math.radians(alpha)
    sector = int(math.floor((alpha + 45) / 45.0))
    nearest = sector // 2

    nl, ol, axis_y = width, height, nearest % 2 == 0
    if axis_y:
        nl, ol = height, width

    index = sector - 1
    imod2 = (index + 8) % 2
    sgn = 2.0 * (imod2 - 0.5)
    np_ = (nl / 2.0) * (1 + sgn * ((alpha - 45 * (index + imod2)) / 45.0))
    pm = nl - np_
    pf = math.hypot(pm, ol)
    if trace:
        print("      alpha=%.2f sector=%d nearest=%d nl=%.2f ol=%.2f np=%.2f pm=%.2f pf=%.2f pie=%.2f"
              % (alpha, sector, nearest, nl, ol, np_, pm, pf, pie))
    if pf > pie:
        return "FAIL(pf>r)"

    beta = math.atan2(ol, pm)
    amod90 = math.fmod(alpha + 45, 90.0) - 45
    sign = 0.0 if amod90 == 0 else (-1.0 if amod90 < 0 else 1.0)
    theta = sign * arad + (math.pi / 2) * (1 - sign * nearest) + beta
    if theta > math.pi:
        theta = 2 * math.pi - theta

    if math.fmod(theta, math.pi) == 0.0:
        cp = pie - pf
    else:
        st = math.sin(theta)
        delta = math.asin(pf * st / pie)
        cp = pie * math.sin(math.pi - (theta + delta)) / st

    px, py = math.cos(arad) * cp, math.sin(arad) * cp
    dx = -1.0 if 90 <= ray < 270 else 1.0
    dy = -1.0 if ray >= 180 else 1.0

    nx, ny = px, py
    if axis_y: ny -= dy * np_
    else:      nx -= dx * np_
    mx, my = nx, ny
    if axis_y: my += dy * nl
    else:      mx += dx * nl
    gx, gy = nx, ny
    if axis_y: gx += dx * ol
    else:      gy += dy * ol

    acm = between(px, py, mx, my)
    if trace:
        print("      theta=%.2f cp=%.2f P=(%.2f,%.2f) angleCM=%.2f half=%.2f"
              % (math.degrees(theta), cp, px, py, acm, half))
    if acm > half:
        return "FAIL(CM %.2f>%.2f)" % (acm, half)

    crosses = (ny >= 0 and my <= 0) or (ny <= 0 and my >= 0) if axis_y \
        else (nx >= 0 and mx <= 0) or (nx <= 0 and mx >= 0)
    if crosses:
        a = between(px, py, nx, ny)
        if a > half: return "FAIL(CN %.2f>%.2f)" % (a, half)
    else:
        a = between(px, py, gx, gy)
        if a > half: return "FAIL(CG %.2f>%.2f)" % (a, half)

    bx, by = nx, ny
    if axis_y:
        by += dy * nl / 2; bx += dx * ol / 2
    else:
        bx += dx * nl / 2; by += dy * ol / 2
    return (bx, by)


VALUES = [93, 100, 107, 114, 121]
TOTAL = sum(VALUES)

# The reference's own final pass on 003_advanced_excel_pie: radius 99.78, one 19-glyph label on
# one line and four wrapped onto two.  Widths are the reference's own, read off the page: the key
# is 5.98 and the key-to-text gap 8.818, and the text extents come from the drawn run origins.
CASES = [
    ("M1  19 glyphs, 1 line", 88.16, 11.21, "OUTSIDE"),
    ("M2  17+3, 2 lines",     79.80, 22.42, "inside"),
    ("M3  17+3, 2 lines",     79.80, 22.42, "inside"),
    ("M4  17+3, 2 lines",     79.80, 22.42, "inside"),
    ("M5  17+3, 2 lines",     79.80, 22.42, "inside"),
]

print("Reference 003_advanced_excel_pie, final pass, radius 99.78")
print("%-24s %-10s %-46s %s" % ("slice", "reference", "port says", "agrees"))
start = 90.0
for i, (name, w, h, expected) in enumerate(CASES):
    sweep = VALUES[i] / TOTAL * 360.0
    bis = start - sweep / 2
    start -= sweep
    got = bestfit(bis, sweep, 99.78, w, h, trace=True)
    inside = isinstance(got, tuple)
    ok = (inside and expected == "inside") or (not inside and expected == "OUTSIDE")
    shown = ("inside at (%.2f, %.2f)" % got) if inside else str(got)
    print("%-24s %-10s %-46s %s" % (name, expected, shown, "YES" if ok else "**NO**"))
