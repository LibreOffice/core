#!/usr/bin/env python3
"""Drive `ladder.py` two ways: sweep the label width, or sweep the tick pitch.

Sweeping the width at one pitch cannot separate `pitch - constant` from `fraction x pitch`
-- the two coincide wherever the constant was read off -- so the round's own measurements
come from the second form, which holds the label fixed and moves the pitch by the chart
frame's own width. `ladder` is the first; `frame` is the second.

    sweep.py <work> ladder <shape> <cats> <size> <lo> <hi> <steps>
    sweep.py <work> frame  <shape> <cats> <size> <text> <cx_lo> <cx_hi> <steps>

`pick` chooses `steps` strings whose advances are as evenly spread over [lo, hi] as the
catalogue allows; note that a tail containing `f` ligates in Carlito and its computed
advance is then ~0.5 pt too wide, so a measurement that has to be exact uses `n`-runs only.

Every variant is rendered by /opt/libreoffice26.2/program/soffice with its hyphenator off,
24 to an invocation, and its pptx is deleted once the PDF exists.
"""
import os
import subprocess
import sys

import ladder

SOFFICE = "/opt/libreoffice26.2/program/soffice"
# Tails carrying `f` are kept because the first ladders used them and `rows.tsv` records what
# was run; anything measured to better than half a point uses a run of one letter instead.
TAILS = ["", "i", "j", "l", "f", "t", "r", "s",
         "ii", "ij", "il", "if", "it", "ir", "is", "jj", "ff", "ft", "fr", "fs",
         "tt", "tr", "ts", "rr", "rs", "ss", "iii", "iij", "iif", "iit", "iir", "iis",
         "ift", "ifr", "ifs", "itr", "its", "irs", "fft", "ffs", "ftr", "fts", "frs",
         "ttr", "tts", "trs", "rrs", "rss", "sss", "iiii", "iiij", "iiif", "iiit",
         "iiir", "iiis", "iffs", "itts", "irss", "ffss", "ttss", "ssss"]


def catalogue(size):
    """Every `n`*k + tail string, with its advance, sorted by width."""
    out = {}
    for k in range(0, 24):
        for tail in TAILS:
            text = "n" * k + tail
            if len(text) < 2:
                continue
            out[text] = ladder.advance(text, size)
    return sorted(out.items(), key=lambda kv: kv[1])


def pick(size, lo, hi, steps):
    """`steps` strings whose advances are as evenly spread over [lo, hi] as the set allows."""
    cat = [(t, w) for t, w in catalogue(size) if lo <= w <= hi]
    if not cat:
        return []
    if len(cat) <= steps:
        return [t for t, _ in cat]
    chosen, used = [], set()
    for i in range(steps):
        target = lo + (hi - lo) * i / (steps - 1.0)
        best = min((abs(w - target), t) for t, w in cat if t not in used)
        used.add(best[1])
        chosen.append(best[1])
    return sorted(chosen, key=lambda t: ladder.advance(t, size))


def render(work, names):
    ref = os.path.join(work, "ref")
    os.makedirs(ref, exist_ok=True)
    todo = [os.path.join(work, n + ".pptx") for n in names
            if not os.path.exists(os.path.join(ref, n + ".pdf"))]
    for i in range(0, len(todo), 24):
        batch = todo[i:i + 24]
        subprocess.run(
            [SOFFICE, "--headless",
             "-env:UserInstallation=file://" + os.path.join(work, "profile"),
             "--convert-to", "pdf", "--outdir", ref] + batch,
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    missing = [n for n in names if not os.path.exists(os.path.join(ref, n + ".pdf"))]
    if missing:
        raise SystemExit("soffice produced no output for: " + ", ".join(missing[:5]))


def run(work, shape, cats, size, texts, cxs=None):
    """One rendering per (text, frame width); the pptx is deleted once its PDF exists."""
    os.makedirs(work, exist_ok=True)
    cxs = cxs or [ladder.FRAME_CX]
    cases = [(t, cx) for cx in cxs for t in texts]
    names = [ladder.write_one(work, shape, cats, size, t, cx) for t, cx in cases]
    render(work, names)
    rows = []
    for (text, cx), name in zip(cases, names):
        r = ladder.classify_one(os.path.join(work, "ref", name + ".pdf"),
                                shape, cats, size, text)
        r.update(shape=shape, cats=cats, size=size, text=text, cx=cx,
                 advance=ladder.advance(text, size),
                 label_adv=ladder.advance(ladder.label_of(shape, text), size))
        rows.append(r)
        os.remove(os.path.join(work, name + ".pptx"))
    return rows


HEAD = ("shape\tcats\tsize\tcx\ttext\tadvance\tlabel_adv\tarrangement\tlines\tink"
        "\tpitch_drawn")


def fmt(r):
    return ("%s\t%d\t%g\t%d\t%s\t%.3f\t%.3f\t%s\t%d\t%.3f\t%.3f"
            % (r["shape"], r["cats"], r["size"], r["cx"], r["text"], r["advance"],
               r["label_adv"], r["arrangement"], r["lines"], r["ink"], r["pitch_drawn"]))


def main():
    work = sys.argv[1]
    if sys.argv[2] == "ladder":
        shape, cats, size = sys.argv[3], int(sys.argv[4]), float(sys.argv[5])
        lo, hi, steps = float(sys.argv[6]), float(sys.argv[7]), int(sys.argv[8])
        rows = run(work, shape, cats, size, pick(size, lo, hi, steps))
    elif sys.argv[2] == "frame":
        # frame <shape> <cats> <size> <text> <cx_lo> <cx_hi> <steps>
        shape, cats, size, text = sys.argv[3], int(sys.argv[4]), float(sys.argv[5]), sys.argv[6]
        lo, hi, steps = int(sys.argv[7]), int(sys.argv[8]), int(sys.argv[9])
        cxs = [lo + (hi - lo) * i // (steps - 1) for i in range(steps)]
        rows = run(work, shape, cats, size, [text], cxs)
    else:
        raise SystemExit(__doc__)
    print(HEAD)
    for r in rows:
        print(fmt(r))


if __name__ == "__main__":
    main()
