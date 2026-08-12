#!/usr/bin/env python3
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Generates the drag-to-Applications background for the macOS .dmg, at 1x and
# 2x, in two variants:
#
#   dmg-background.png          titled with the product name
#   dmg-background-generic.png  no product name, for rebranded builds
#
# macos/make-dmg.sh picks the variant that matches the name of the bundle it is
# packaging, and hands the 1x PNG to dmgbuild, which finds the @2x sibling by
# name and compiles the two into the multi-resolution TIFF that Finder wants.
# Rebranded builds that do want their own name in the title can regenerate the
# titled variant with --name.
#
# The geometry constants below are also written out to geometry.env, which
# make-dmg.sh sources for its create-dmg arguments, so the window size and the
# icon positions only ever get defined here.

import argparse
import math
import os

from PIL import Image, ImageDraw, ImageFont

DEFAULT_NAME = "Collabora Office"

# Window content area, in points. Passed to create-dmg as --window-size.
WIDTH = 680
HEIGHT = 400

# Icon centres, in points. Passed to create-dmg as --icon and --app-drop-link.
ICON_SIZE = 128
APP_CENTRE = (170, 190)
DROP_CENTRE = (510, 190)

# Colours
BG_TOP = (250, 250, 252)
BG_BOTTOM = (236, 236, 241)
ARROW = (150, 150, 158)
TITLE_COLOUR = (29, 29, 31)
HINT_COLOUR = (110, 110, 115)

HINT = "Drag the app icon onto the Applications folder"

# Arrow geometry, in points. The stroke widens slightly towards the head.
ARROW_BULGE = 46.0
ARROW_TAIL_WIDTH = 5.0
ARROW_HEAD_WIDTH = 9.5
ARROW_HEAD_LENGTH = 30.0
ARROW_HEAD_HALF = 17.0

FONT_DIRS = [
    "/usr/share/fonts/truetype/lato",
    "/System/Library/Fonts",
    "/usr/share/fonts/truetype/liberation2",
    "/usr/share/fonts/truetype/msttcorefonts",
]
TITLE_FONTS = ["Lato-Bold.ttf", "HelveticaNeue.ttc", "LiberationSans-Bold.ttf", "Arial_Bold.ttf"]
HINT_FONTS = ["Lato-Regular.ttf", "Helvetica.ttc", "LiberationSans-Regular.ttf", "Arial.ttf"]

# Supersampling factor for the vector parts; text is drawn at final size.
SS = 4


def load_font(candidates, size):
    for name in candidates:
        for directory in FONT_DIRS:
            path = os.path.join(directory, name)
            if os.path.exists(path):
                return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def gradient(width, height):
    image = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        draw.line(
            [(0, y), (width, y)],
            fill=tuple(round(a + (b - a) * t) for a, b in zip(BG_TOP, BG_BOTTOM)),
        )
    return image


def bezier_point(p0, p1, p2, t):
    u = 1 - t
    return (
        u * u * p0[0] + 2 * u * t * p1[0] + t * t * p2[0],
        u * u * p0[1] + 2 * u * t * p1[1] + t * t * p2[1],
    )


def bezier_normal(p0, p1, p2, t):
    """Unit normal, pointing to the left of the direction of travel."""
    u = 1 - t
    dx = 2 * u * (p1[0] - p0[0]) + 2 * t * (p2[0] - p1[0])
    dy = 2 * u * (p1[1] - p0[1]) + 2 * t * (p2[1] - p1[1])
    length = math.hypot(dx, dy) or 1.0
    return (-dy / length, dx / length), (dx / length, dy / length)


def arrow_shapes():
    """Outline of the stroke and of the head, as polygons in points.

    The stroke is built as a real offset curve rather than drawn as a thick
    polyline, because a polyline is rasterised as one rounded shape per
    segment, and the seams between those show up as a ragged edge.
    """
    x0 = APP_CENTRE[0] + ICON_SIZE / 2 + 22
    x1 = DROP_CENTRE[0] - ICON_SIZE / 2 - 14
    y = APP_CENTRE[1]
    p0 = (x0, y)
    p2 = (x1, y)
    p1 = ((x0 + x1) / 2, y - ARROW_BULGE)

    # Walk the curve to find where the head has to start, so that the head is
    # the length we asked for however the curve is shaped.
    steps = 600
    lengths = [0.0]
    previous = bezier_point(p0, p1, p2, 0.0)
    for i in range(1, steps + 1):
        current = bezier_point(p0, p1, p2, i / steps)
        lengths.append(lengths[-1] + math.dist(previous, current))
        previous = current
    target = lengths[-1] - ARROW_HEAD_LENGTH
    t_base = 1.0
    for i, travelled in enumerate(lengths):
        if travelled >= target:
            t_base = i / steps
            break

    left, right = [], []
    segments = 240
    for i in range(segments + 1):
        t = t_base * i / segments
        centre = bezier_point(p0, p1, p2, t)
        normal, _ = bezier_normal(p0, p1, p2, t)
        half = (ARROW_TAIL_WIDTH + (ARROW_HEAD_WIDTH - ARROW_TAIL_WIDTH) * i / segments) / 2
        left.append((centre[0] + normal[0] * half, centre[1] + normal[1] * half))
        right.append((centre[0] - normal[0] * half, centre[1] - normal[1] * half))

    # Round off the tail, sweeping the half circle that goes behind the start
    # rather than the one that would cut across the stroke.
    radius = ARROW_TAIL_WIDTH / 2
    angle = math.atan2(right[0][1] - p0[1], right[0][0] - p0[0])
    forward = bezier_normal(p0, p1, p2, 0.0)[1]
    if math.cos(angle + math.pi / 2) * forward[0] + math.sin(angle + math.pi / 2) * forward[1] > 0:
        step = -math.pi / 16
    else:
        step = math.pi / 16
    cap = []
    for i in range(1, 16):
        a = angle + step * i
        cap.append((p0[0] + math.cos(a) * radius, p0[1] + math.sin(a) * radius))

    stroke = left + list(reversed(right)) + cap

    base = bezier_point(p0, p1, p2, t_base)
    normal, forward = bezier_normal(p0, p1, p2, t_base)
    head = [
        (base[0] + forward[0] * ARROW_HEAD_LENGTH, base[1] + forward[1] * ARROW_HEAD_LENGTH),
        (base[0] + normal[0] * ARROW_HEAD_HALF, base[1] + normal[1] * ARROW_HEAD_HALF),
        (base[0] - normal[0] * ARROW_HEAD_HALF, base[1] - normal[1] * ARROW_HEAD_HALF),
    ]
    return stroke, head


def draw_arrow(image, scale):
    """Curved arrow from the app icon towards the Applications folder."""
    layer = Image.new("L", (image.width * SS, image.height * SS), 0)
    draw = ImageDraw.Draw(layer)
    s = scale * SS

    # Both shapes go into one mask, so that the join between them is a union
    # and not two antialiased edges meeting along a seam.
    for shape in arrow_shapes():
        draw.polygon([(x * s, y * s) for x, y in shape], fill=255)

    # A box filter is the exact average of the supersampled pixels. Lanczos
    # would ring along these edges and pit them.
    mask = layer.resize(image.size, Image.BOX)
    image.paste(Image.new("RGB", image.size, ARROW), (0, 0), mask)


def draw_text(image, scale, title):
    draw = ImageDraw.Draw(image)
    if title:
        draw.text(
            (WIDTH / 2 * scale, 50 * scale),
            title,
            font=load_font(TITLE_FONTS, round(22 * scale)),
            fill=TITLE_COLOUR,
            anchor="mm",
        )
    draw.text(
        (WIDTH / 2 * scale, 330 * scale),
        HINT,
        font=load_font(HINT_FONTS, round(13 * scale)),
        fill=HINT_COLOUR,
        anchor="mm",
    )


def render(scale, title):
    image = gradient(WIDTH * scale, HEIGHT * scale)
    draw_arrow(image, scale)
    draw_text(image, scale, title)
    return image


def write_variant(outdir, stem, title):
    for scale, suffix in ((1, ""), (2, "@2x")):
        path = os.path.join(outdir, "%s%s.png" % (stem, suffix))
        render(scale, title).save(path)
        print("wrote %s (%dx%d)" % (path, WIDTH * scale, HEIGHT * scale))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--name",
        default=DEFAULT_NAME,
        help="product name for the title of the titled variant (default: %s)" % DEFAULT_NAME,
    )
    parser.add_argument("--outdir", default=os.path.dirname(os.path.abspath(__file__)))
    args = parser.parse_args()

    write_variant(args.outdir, "dmg-background", "Install %s" % args.name)
    write_variant(args.outdir, "dmg-background-generic", None)

    path = os.path.join(args.outdir, "geometry.env")
    with open(path, "w") as env:
        env.write("# Generated by make-dmg-background.py -- do not edit.\n")
        env.write("DMG_WINDOW_WIDTH=%d\n" % WIDTH)
        env.write("DMG_WINDOW_HEIGHT=%d\n" % HEIGHT)
        env.write("DMG_ICON_SIZE=%d\n" % ICON_SIZE)
        env.write("DMG_APP_X=%d\n" % APP_CENTRE[0])
        env.write("DMG_APP_Y=%d\n" % APP_CENTRE[1])
        env.write("DMG_DROP_X=%d\n" % DROP_CENTRE[0])
        env.write("DMG_DROP_Y=%d\n" % DROP_CENTRE[1])
        # The name baked into the titled variant, so that make-dmg.sh can tell
        # whether that variant applies to the bundle it is packaging.
        env.write("DMG_TITLE_NAME='%s'\n" % args.name.replace("'", "'\\''"))
    print("wrote %s" % path)


if __name__ == "__main__":
    main()
