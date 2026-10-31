#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Render a seamless looping cloud time-lapse as 1280x720 RGB frames to stdout:
#   make_clouds.py day|dusk
# The noise field comes from a 3D spectral synthesis whose time axis is periodic, so the last frame
# runs straight into the first. The clouds also drift exactly one field width over the loop.
import sys
import numpy as np
from PIL import Image

variant = sys.argv[1]
W, H = 320, 180            # field size, upscaled 4x to 1280x720
FRAMES = 600               # 20 seconds at 30 fps, which shows each frame for two refreshes at 60 Hz
OUT_W, OUT_H = 1280, 720

rng = np.random.default_rng(7 if variant == "day" else 11)
kx = np.fft.fftfreq(W)[None, None, :]
ky = np.fft.fftfreq(H)[None, :, None]
kt = np.fft.fftfreq(FRAMES)[:, None, None]
spatial = np.sqrt(kx ** 2 + ky ** 2)
spatial[0, 0, 0] = 1.0
# Fractal spatial spectrum, with fine detail changing faster than the large shapes.
amplitude = spatial ** -2.1 * np.exp(-(kt * FRAMES / (1.0 + 60.0 * spatial)) ** 2)
amplitude[0, 0, 0] = 0.0
phase = rng.uniform(0, 2 * np.pi, (FRAMES, H, W))
spectrum = (amplitude * np.exp(1j * phase)).astype(np.complex64)
# Drift: shifting by one field width over the loop multiplies each x frequency by a phase ramp.
time_index = np.arange(FRAMES)
# The real part of the inverse transform is a field that is periodic in x, y and time.
field = np.fft.fft2(np.real(np.fft.ifftn(spectrum)), axes=(1, 2))
shift = np.exp(-2j * np.pi * np.fft.fftfreq(W)[None, None, :] * W
               * (time_index[:, None, None] / FRAMES))
field = np.real(np.fft.ifft2(field * shift, axes=(1, 2))).astype(np.float32)
field = (field - field.mean()) / field.std()

yy = np.linspace(0.0, 1.0, OUT_H)[:, None, None]
if variant == "day":
    sky_top, sky_bottom = np.array([40, 92, 170]), np.array([150, 190, 230])
    cloud_lit, cloud_shade = np.array([255, 255, 255]), np.array([150, 160, 180])
    cover = 0.25
else:
    sky_top, sky_bottom = np.array([30, 20, 70]), np.array([220, 120, 110])
    cloud_lit, cloud_shade = np.array([255, 200, 170]), np.array([90, 60, 110])
    cover = 0.45
sky = sky_top * (1 - yy) + sky_bottom * yy

for t in range(FRAMES):
    f = Image.fromarray(field[t]).resize((OUT_W, OUT_H), Image.BICUBIC)
    n = np.asarray(f)[:, :, None]
    density = np.clip((n - cover) / 1.8, 0.0, 1.0)
    density = density * density * (3 - 2 * density)
for t in range(FRAMES):
    f = Image.fromarray(field[t]).resize((OUT_W, OUT_H), Image.BICUBIC)
    n = np.asarray(f)[:, :, None]
    density = np.clip((n - cover) / 1.8, 0.0, 1.0)
    density = density * density * (3 - 2 * density)
    # The sun is above, so a cloud is brighter where its density falls away towards the top.
    below = np.roll(n, -12, axis=0)
    light = np.clip(0.5 + (n - below) * 2.0, 0.0, 1.0)
    colour = cloud_shade + (cloud_lit - cloud_shade) * (0.35 + 0.65 * light)
    rgb = sky * (1 - density) + colour * density
    sys.stdout.buffer.write(np.clip(rgb, 0, 255).astype(np.uint8).tobytes())
