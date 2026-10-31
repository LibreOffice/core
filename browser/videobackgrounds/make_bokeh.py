#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Render a seamless looping bokeh background as 1280x720 RGB frames to stdout: out of focus points
# of warm light rising slowly. Every motion repeats a whole number of times over the loop, so the
# last frame runs straight into the first.
import sys
import numpy as np
from PIL import Image

FRAMES = 600               # 20 seconds at 30 fps
OUT_W, OUT_H = 1280, 720
TAU = 2 * np.pi


def vertical_gradient(top, bottom, h, w):
    y = np.linspace(0.0, 1.0, h)[:, None, None]
    return np.broadcast_to(np.array(top) * (1 - y) + np.array(bottom) * y, (h, w, 3))


# A fixed dither pattern breaks up the banding of slow gradients in 8 bit video.
DITHER = np.random.default_rng(1).uniform(-1.5, 1.5, (OUT_H, OUT_W, 1))


def finish(rgb):
    if rgb.shape[0] != OUT_H:
        channels = [Image.fromarray(rgb[:, :, c].astype(np.float32)).resize((OUT_W, OUT_H),
                                                                          Image.BICUBIC)
                    for c in range(3)]
        rgb = np.stack([np.asarray(c) for c in channels], axis=2)
    return np.clip(rgb + DITHER, 0, 255).astype(np.uint8)


def make_bokeh():
    # Out of focus points of warm light rising slowly.
    w, h = 640, 360
    rng = np.random.default_rng(13)
    base = vertical_gradient([30, 18, 22], [12, 8, 14], h, w)
    palette = np.array([[255, 190, 110], [255, 150, 90], [255, 215, 160], [240, 120, 110]],
                       dtype=float)
    lights = []
    for _ in range(46):
        depth = rng.uniform()
        r = 6 + 34 * depth ** 1.5
        lights.append(dict(r=r, x=rng.uniform(0, w), y=rng.uniform(0, h + 2 * r),
                           sway=rng.uniform(5, 25), m=rng.integers(1, 3), p=rng.uniform(0, TAU),
                           tw=rng.integers(1, 4), tp=rng.uniform(0, TAU),
                           alpha=0.45 - 0.3 * depth, edge=1.0 + 0.12 * r,
                           colour=palette[rng.integers(len(palette))]))
    for f in range(FRAMES):
        t = f / FRAMES
        light = np.zeros((h, w, 3))
        for b in lights:
            r = b["r"]
            span = h + 2 * r
            # Each light rises one full span over the loop and wraps from the top to the bottom.
            y = (b["y"] - t * span) % span - r
            x = b["x"] + b["sway"] * np.sin(TAU * b["m"] * t + b["p"])
            x0, x1 = int(max(x - r - 3, 0)), int(min(x + r + 4, w))
            y0, y1 = int(max(y - r - 3, 0)), int(min(y + r + 4, h))
            if x0 >= x1 or y0 >= y1:
                continue
            yy, xx = np.mgrid[y0:y1, x0:x1]
            dist = np.sqrt((xx - x) ** 2 + (yy - y) ** 2)
            disc = np.clip((r - dist) / b["edge"] + 0.5, 0, 1)
            rim = np.exp(-((dist - r * 0.85) / (0.12 * r + 1)) ** 2) * 0.35
            twinkle = 1 + 0.2 * np.sin(TAU * b["tw"] * t + b["tp"])
            light[y0:y1, x0:x1] += ((disc * (1 + rim)) * b["alpha"] * twinkle)[:, :, None] * b["colour"]
        rgb = 255 - (255 - base) * (1 - np.clip(light / 255, 0, 1))
        yield finish(rgb)



for frame in make_bokeh():
    sys.stdout.buffer.write(frame.tobytes())
