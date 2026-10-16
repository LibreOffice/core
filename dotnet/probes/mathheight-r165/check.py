#!/usr/bin/env python3
"""Score `model.py` against 26.2.4.2's own `svg:height` for every fixture."""
import os, re, sys
from model import (R, leaf, blank, hor, frac, script, root, brace, table, matrix,
                   BASE, PT, SIZ_INDEX)

HERE = os.path.dirname(os.path.abspath(__file__))


def measured():
    out = {}
    for name in sorted(os.listdir(os.path.join(HERE, "fodt"))):
        if not name.endswith(".fodt"):
            continue
        s = open(os.path.join(HERE, "fodt", name), encoding="utf-8").read()
        m = re.search(r'<draw:frame[^>]*\ssvg:height="([0-9.]+)in"', s)
        if m:
            out[name[:-5]] = round(float(m.group(1)) * 2540)
    return out


S = SIZ_INDEX / 100.0


def L(scale=1.0):
    return leaf(scale)


def T(scale=1.0):
    return leaf(scale)[0]


def H(scale=1.0):
    return int(BASE * scale)


def F(n, d, s=1.0):
    return frac(n, d, H(s))


def SUB(b, sc, s=1.0):
    return script(b, H(s), sub=sc)


def SUP(b, sc, s=1.0):
    return script(b, H(s), sup=sc)


def SUBSUP(b, a, c, s=1.0):
    return script(b, H(s), sub=a, sup=c)


CASES = {
    "leaf-x":          lambda: T(),
    "leaf-abc":        lambda: T(),
    "leaf-digit":      lambda: T(),
    "leaf-caps":       lambda: T(),
    "leaf-rho":        lambda: T(),
    "leaf-beta":       lambda: T(),
    "leaf-plus":       lambda: T(),
    "leaf-two-runs":   lambda: hor([T(), T()]),
    "sub":             lambda: SUB(T(), T(S)),
    "sup":             lambda: SUP(T(), T(S)),
    "subsup":          lambda: SUBSUP(T(), T(S), T(S)),
    "pre":             lambda: SUBSUP(T(), T(S), T(S)),
    "sub-deep":        lambda: SUB(T(), SUB(T(S), T(S * S), S)),
    "sup-deep":        lambda: SUP(T(), SUP(T(S), T(S * S), S)),
    "sub-of-frac":     lambda: SUB(F(T(), T()), T(S)),
    "sup-tall-base":   lambda: SUP(F(T(), T()), T(S)),
    "sub-tall-script": lambda: SUB(T(), F(T(S), T(S), S)),
    "frac":            lambda: F(T(), T()),
    "frac-nest-num":   lambda: F(F(T(), T()), T()),
    "frac-nest-den":   lambda: F(T(), F(T(), T())),
    "frac-nest-both":  lambda: F(F(T(), T()), F(T(), T())),
    "frac-nest3":      lambda: F(F(F(T(), T()), T()), T()),
    "frac-scripts":    lambda: F(SUB(T(), T(S)), SUB(T(), T(S))),
    "frac-nobar":      lambda: table([T(), T()], H()),
    "frac-skw":        lambda: F(T(), T()),
    "eqarr-2":         lambda: table([T(), T()], H()),
    "eqarr-3":         lambda: table([T(), T(), T()], H()),
    "mat-1x2":         lambda: matrix([hor([T(), T()])], H()),
    "mat-2x2":         lambda: matrix([hor([T(), T()]), hor([T(), T()])], H()),
    "mat-3x2":         lambda: matrix([hor([T(), T()])] * 3, H()),
    "mat-2x2-frac":    lambda: matrix([hor([F(T(), T()), T()]), hor([T(), T()])], H()),
    "rad":             lambda: root(T(), H()),
    "rad-frac":        lambda: root(F(T(), T()), H()),
    "rad-rad":         lambda: root(root(T(), H()), H()),
    "rad-deg":         lambda: root(T(), H(), deg=T(S)),
    "delim":           lambda: brace(T(), H()),
    "delim-sq":        lambda: brace(T(), H()),
    "delim-frac":      lambda: brace(F(T(), T()), H()),
    "delim-nest":      lambda: brace(hor([T(), brace(F(T(), T()), H())]), H()),
    "box":             lambda: T(),
    "phant":           lambda: T(),
    "acc":             lambda: T(),
    "acc-tilde":       lambda: T(),
    "acc-frac":        lambda: F(T(), T()),
    "bar-top":         lambda: T(),
    "bar-bot":         lambda: T(),
    "sz8":             lambda: T(),
    "sz20":            lambda: T(),
    "sz20-sub":        lambda: SUB(T(), T(S)),
    "sz20-frac":       lambda: F(T(), T()),
    "seq-frac-then-x": lambda: hor([F(T(), T()), T()]),
    "seq-x-then-frac": lambda: hor([T(), F(T(), T())]),
    "frac-lin":        lambda: hor([T(), T(), T()]),
}

if __name__ == "__main__":
    ref = measured()
    w = max(len(k) for k in CASES)
    worst, rows = 0.0, []
    for name, build in sorted(CASES.items()):
        if name not in ref:
            print("no reference for", name)
            continue
        got, want = build().h, ref[name]
        d = (got - want) / PT
        worst = max(worst, abs(d))
        rows.append((abs(d), name, got, want, d))
    rows.sort(reverse=True)
    print(f"{'fixture':<{w}}  {'model':>6}  {'26.2':>6}  {'Δ pt':>7}")
    for _, name, got, want, d in rows:
        print(f"{name:<{w}}  {got:6d}  {want:6d}  {d:7.3f}")
    print(f"\n{len(rows)} scored, worst |Δ| = {worst:.3f} pt, "
          f"within 0.25 pt: {sum(1 for r in rows if r[0] <= 0.25)}")
