#!/usr/bin/env python3
"""O43 r111 -- read 26.2.4.2's own wrap paper straight off a rendering.

Round 110 left the two-word restart at "about 0.875 of the pitch, and it moves with the
first word", against a wrap limit its own one-word ladder pins at 0.95. Both of those are
inferences from *when the axis turns*. This instrument measures the thing itself: a label
made of many short words can never break inside a word, so the axis never restarts and the
label is simply drawn wrapped -- and **which words land on which line is the paper width**,
read out of the PDF with no threshold model in between.

    paper.py <work> multi <cats> <size> <marker> <word> <nwords> <cx_lo> <cx_hi> <steps>
    paper.py <work> pair  <cats> <size> <first> <second> <cx_lo> <cx_hi> <steps>

Every variant is `038_Competitive_Advantage_Card`'s bar chart with its categories rewritten,
its runs retagged `de-DE` (r105's hyphenator switch) and its `p:graphicFrame` widened or
narrowed and re-centred, exactly as `probes/chart-collide-r110/ladder.py` does it -- this
imports that module rather than restating it.
"""
import os
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "..", "chart-collide-r110"))

import pymupdf  # noqa: E402
import ladder  # noqa: E402

SOFFICE = "/opt/libreoffice26.2/program/soffice"


def label_multi(marker, word, nwords):
    return " ".join([marker] + [word] * nwords)


def write(work, cats, size, label, cx, tag):
    """One deck, every category carrying `label`."""
    name = "%s-c%d-s%s-x%d" % (tag, cats, ("%g" % size).replace(".", "_"), cx)
    dst = os.path.join(work, name + ".pptx")
    import zipfile
    with zipfile.ZipFile(ladder.SRC) as zin, \
            zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as out:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == ladder.CHART:
                data = ladder.rewrite_chart(data.decode("utf-8"), label, cats, size,
                                            "de-DE").encode("utf-8")
            elif item.filename.startswith("ppt/slides/slide") and cx != ladder.FRAME_CX:
                text = data.decode("utf-8")
                text = text.replace('<a:ext cx="%d"' % ladder.FRAME_CX,
                                    '<a:ext cx="%d"' % cx)
                text = text.replace('<a:off x="%d"' % ladder.FRAME_X,
                                    '<a:off x="%d"' % max(0, (9144000 - cx) // 2))
                data = text.encode("utf-8")
            out.writestr(item, data)
    return name


def render(work, names):
    ref = os.path.join(work, "ref")
    os.makedirs(ref, exist_ok=True)
    todo = [os.path.join(work, n + ".pptx") for n in names
            if not os.path.exists(os.path.join(ref, n + ".pdf"))]
    for i in range(0, len(todo), 24):
        subprocess.run(
            [SOFFICE, "--headless",
             "-env:UserInstallation=file://" + os.path.join(work, "profile"),
             "--convert-to", "pdf", "--outdir", ref] + todo[i:i + 24],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    missing = [n for n in names if not os.path.exists(os.path.join(ref, n + ".pdf"))]
    if missing:
        raise SystemExit("soffice produced no output for: " + ", ".join(missing[:5]))


def read(path, marker, word, cats):
    """Lines of one label, and the tick pitch measured on the marker word."""
    with pymupdf.open(path) as doc:
        page = doc[0]
        turned = [line
                  for b in page.get_text("dict")["blocks"] if b["type"] == 0
                  for line in b["lines"] if abs(line["dir"][0] - 0.70711) < 0.004]
        words = page.get_text("words")
        # A 45-degree label is sometimes drawn as outlined paths rather than text, in which
        # case `dir` finds nothing and the label's own tokens are absent: r110's rule is
        # >= 6 glyph-sized filled paths per category.
        fills = [d for d in page.get_drawings()
                 if d["fill"] is not None
                 and 0.3 < d["rect"].width < 40 and 0.3 < d["rect"].height < 40]
    marks = sorted((w[0] + w[2]) / 2 for w in words if w[4] == marker)
    pitch = ((marks[-1] - marks[0]) / (len(marks) - 1)) if len(marks) > 1 else float("nan")
    # group every occurrence of the label's own tokens by baseline
    rows = {}
    for w in words:
        if w[4] not in (marker, word):
            continue
        rows.setdefault(round(w[3], 1), []).append(w)
    # Every category carries the same label, so a baseline holds `cats` copies of one line
    # of it: the tokens per line is the baseline's token count divided by the label count,
    # which is immune to two adjacent labels being merged into one PyMuPDF span.
    lines = []
    for y in sorted(rows):
        band = sorted(rows[y], key=lambda w: w[0])
        lines.append(band)
    per = [len(b) / float(cats) for b in lines]
    return {
        "turned": bool(turned) or len(fills) >= 6 * cats,
        "pitch": pitch,
        "nlines": len(lines),
        "per": per,
        "exact": all(abs(p - round(p)) < 1e-9 for p in per),
        "lines": "|".join("%g" % p for p in per),
    }


PITCH_PER_CX = 1.511780e-5  # r110 §1.1, `038` at 5 categories

HEAD = ("tag\tcats\tsize\tcx\tpitch_calib\tpitch_drawn\tturned\tnlines\tline1\texact"
        "\tw_word\tw_space\tw_marker\tperline")


def main():
    work, mode = sys.argv[1], sys.argv[2]
    os.makedirs(work, exist_ok=True)
    if mode == "multi":
        cats, size = int(sys.argv[3]), float(sys.argv[4])
        marker, word, nwords = sys.argv[5], sys.argv[6], int(sys.argv[7])
        lo, hi, steps = int(sys.argv[8]), int(sys.argv[9]), int(sys.argv[10])
        label = label_multi(marker, word, nwords)
        tag = "m-%s-%s%d" % (marker, word, nwords)
    elif mode == "pair":
        # An explicit two-word label: the first word, one blank, the second. The frame width
        # sweeps the pitch continuously, so the boundary is read in `cx` and converted by
        # r110 s own calibration line rather than off a turned rendering's squeezed plot.
        cats, size = int(sys.argv[3]), float(sys.argv[4])
        marker, word = sys.argv[5], sys.argv[6]
        lo, hi, steps = int(sys.argv[7]), int(sys.argv[8]), int(sys.argv[9])
        label = marker + " " + word
        tag = "p-%s-%s" % (marker, word)
    elif mode == "label":
        # An arbitrary label, written with `_` for the blank, and two of its tokens named so
        # the reader can find the pitch and count the lines.
        cats, size = int(sys.argv[3]), float(sys.argv[4])
        spec, marker, word = sys.argv[5], sys.argv[6], sys.argv[7]
        lo, hi, steps = int(sys.argv[8]), int(sys.argv[9]), int(sys.argv[10])
        label = spec.replace("_", " ")
        tag = "l-" + spec
    elif mode == "shape":
        # ladder.py's own label shapes, swept by the frame width, read for BOTH the
        # arrangement and the composition of each drawn line.
        shape, cats, size, text = sys.argv[3], int(sys.argv[4]), float(sys.argv[5]), sys.argv[6]
        lo, hi, steps = int(sys.argv[7]), int(sys.argv[8]), int(sys.argv[9])
        label = ladder.label_of(shape, text)
        parts = label.split()
        marker = parts[0] if parts[0] != text else text
        word = text
        tag = "s-%s-%s" % (shape, text)
    else:
        raise SystemExit(__doc__)
    cxs = [lo + (hi - lo) * i // (steps - 1) for i in range(steps)] if steps > 1 else [lo]
    names = [write(work, cats, size, label, cx, tag) for cx in cxs]
    render(work, names)
    print(HEAD)
    for cx, name in zip(cxs, names):
        r = read(os.path.join(work, "ref", name + ".pdf"), marker, word, cats)
        print("%s\t%d\t%g\t%d\t%.3f\t%.3f\t%s\t%d\t%g\t%s\t%.3f\t%.3f\t%.3f\t%s"
              % (tag, cats, size, cx, PITCH_PER_CX * cx, r["pitch"], r["turned"], r["nlines"],
                 (r["per"][0] if r["per"] else 0), r["exact"],
                 ladder.advance(word, size), ladder.advance(" ", size),
                 ladder.advance(marker, size), r["lines"]))
        os.remove(os.path.join(work, name + ".pptx"))


if __name__ == "__main__":
    main()
