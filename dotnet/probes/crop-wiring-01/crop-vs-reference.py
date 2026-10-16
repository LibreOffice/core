#!/usr/bin/env python3
"""Compare every cropped picture against the reference, paired by its frame.

    crop-vs-reference.py <our-sweep> <refdir> <name> [<name> ...]

**Why not a page-by-page pixel diff.** Six of this round's seven changed renderings
already fail the gate's page-count check, so page N of ours is not page N of the
reference: our `150_5300_13_chg10` page 50 and the reference's page 47 both hold Figure
4-19. A page-level "closer or further" tally there compares one figure with another and
produces a number that looks like a fidelity result and is not one.

A crop shows up in the PDF as an exact pair of operators — a clip rectangle and an image
placed larger than it — so it can be read directly and compared without either side's
pagination mattering at all:

    q  x y w h re W n   q  W 0 0 H  x y cm  /Im1 Do  Q  Q

The frame is `w x h`, the growth is `W/w` and `H/h`, and a picture is paired with the
reference's by that frame — which both legs agree about, because a crop moves no line.
"""
import re
import sys
import zlib
from pathlib import Path

PLACEMENT = re.compile(
    rb'(?:([\d.]+) ([\d.]+) ([\d.]+) ([\d.]+) re\s+W\*? n\s+)?'
    rb'q\s+([-\d.]+) 0 0 ([-\d.]+) ([-\d.]+) ([-\d.]+) cm\s*/(\w+)\s+Do')

CROPPED = 1.005      # growth below this is rounding, not a crop
AGREES = 0.02        # growth within this of the reference's counts as agreement
SAME_FRAME = 2.5     # points; the frame either side resolves the same anchor to


def images(path):
    """Every image placement in a PDF, with the clip that was in force."""
    data = Path(path).read_bytes()
    out = []
    for m in re.finditer(rb'stream\r?\n', data):
        start = m.end()
        end = data.find(b'endstream', start)
        try:
            content = zlib.decompress(data[start:end])
        except zlib.error:
            continue
        for k in PLACEMENT.finditer(content):
            width, height = float(k.group(5)), float(k.group(6))
            frame_w = float(k.group(3)) if k.group(3) else width
            frame_h = float(k.group(4)) if k.group(4) else height
            out.append({"w": width, "h": height, "fw": frame_w, "fh": frame_h})
    return out


def growth(image):
    return image["w"] / image["fw"], image["h"] / image["fh"]


def main():
    ours_dir, ref_dir = Path(sys.argv[1]), Path(sys.argv[2])
    tally = {"MATCH": 0, "OVER": 0, "UNDER": 0, "NO PAIR": 0}

    for name in sys.argv[3:]:
        ours = images(ours_dir / name)
        ref = images(ref_dir / name)
        seen = set()

        for side, source, other in (("ours", ours, ref), ("ref", ref, ours)):
            for image in source:
                gx, gy = growth(image)
                if gx < CROPPED and gy < CROPPED:
                    continue

                key = (round(image["fw"]), round(image["fh"]))
                if key in seen:
                    continue
                seen.add(key)

                paired = [o for o in other
                          if abs(o["fw"] - image["fw"]) < SAME_FRAME
                          and abs(o["fh"] - image["fh"]) < SAME_FRAME]
                if not paired:
                    tally["NO PAIR"] += 1
                    print(f'NO PAIR  {name[:36]:36} frame {image["fw"]:7.1f}x{image["fh"]:6.1f}  '
                          f'{side} grows {gx:.3f}/{gy:.3f}, the other side has no such frame')
                    continue

                best = max(paired, key=lambda o: max(growth(o)))
                a = (gx, gy) if side == "ours" else growth(best)
                r = growth(best) if side == "ours" else (gx, gy)
                apart = max(abs(a[0] - r[0]), abs(a[1] - r[1]))
                verdict = ("MATCH" if apart < AGREES
                           else "OVER" if max(a) > max(r) else "UNDER")
                tally[verdict] += 1
                print(f'{verdict:8} {name[:36]:36} frame {image["fw"]:7.1f}x{image["fh"]:6.1f}  '
                      f'ours {a[0]:.3f}/{a[1]:.3f}  ref {r[0]:.3f}/{r[1]:.3f}')

    print('\n' + '  '.join(f'{k} {v}' for k, v in tally.items()))


if __name__ == "__main__":
    main()
