#!/usr/bin/env python3
"""Mean luma of a whole page, ours against the reference.

`pdf-image-diff.py` computes a page's ink figure as
    abs( sum over regions of luma_gap * area ) / (pixels * 255)
— the sum is SIGNED and the absolute value is taken over the total, so a region where
we draw too little cancels one where we draw too much. This reports the page's overall
sign, which is what says whether removing a deficit should raise or lower that figure.

Positive gap = ours is lighter = the reference has ink we do not.
"""
import subprocess, sys, tempfile, os

def pgm(pdf, page, dpi=150):
    with tempfile.TemporaryDirectory() as d:
        subprocess.run(["pdftoppm", "-gray", "-r", str(dpi), "-f", str(page), "-l", str(page),
                        pdf, os.path.join(d, "p")], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        data = open(os.path.join(d, sorted(os.listdir(d))[0]), "rb").read()
    parts, i = [], 0
    while len(parts) < 4:
        while data[i:i+1].isspace(): i += 1
        if data[i:i+1] == b"#":
            while data[i:i+1] != b"\n": i += 1
            continue
        j = i
        while not data[j:j+1].isspace(): j += 1
        parts.append(data[i:j]); i = j
    i += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[i:i + w*h]

ours, ref = sys.argv[1], sys.argv[2]
for page in (int(p) for p in sys.argv[3:]):
    ow, oh, op = pgm(ours, page)
    rw, rh, rp = pgm(ref, page)
    if (ow, oh) != (rw, rh):
        print("page %d: size differs %dx%d vs %dx%d" % (page, ow, oh, rw, rh))
        continue
    om = sum(op) / len(op)
    rm = sum(rp) / len(rp)
    print("page %-4d ours mean luma %7.3f   ref %7.3f   gap %+7.3f   (%s)"
          % (page, om, rm, om - rm,
             "ours lighter: reference has ink we lack" if om > rm
             else "ours darker: we draw ink the reference does not"))
