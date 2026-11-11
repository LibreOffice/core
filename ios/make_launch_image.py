#!/usr/bin/env python3
# 
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Usage: make_launch_image.py ICON IMAGESET
#
# ICON: .icon directory or .appiconset
# IMAGESET: output .imageset

import itertools
import json
import os
import shutil
import subprocess
import sys
import tempfile

POINTS = 200
SCALES = (2, 3)

# ictool draws a rim around the .icon, so we need to cut it out
DOT_ICON_RIM_POINTS = 19
DOT_ICON_CENTER_POINTS = POINTS - DOT_ICON_RIM_POINTS * 2

if "ICTOOL" in os.environ:
    ictool = os.environ["ICTOOL"]
else:
    xcode = subprocess.run(
        ["xcode-select", "-p"], check=True, capture_output=True, text=True
    ).stdout.strip()
    ictool = os.path.join(
        xcode,
        "..",
        "Applications",
        "Icon Composer.app",
        "Contents",
        "Executables",
        "ictool",
    )

def copy_with_theme_background(icon, workdir):
    name = os.path.basename(icon)
    copy = os.path.join(workdir, name)

    shutil.copytree(icon, copy)

    with open(os.path.join(copy, "icon.json")) as f:
        icon_json = json.load(f)

    try:
        del icon_json["fill"]
    except KeyError:
        pass

    icon_json["fill-specializations"] = [
        {"value": {"solid": "srgb:1.00000,1.00000,1.00000,1.00000"}},
        {
            "appearance": "dark",
            "value": {"solid": "srgb:0.00000,0.00000,0.00000,1.00000"},
        },
    ]

    with open(os.path.join(copy, "icon.json"), "w") as f:
        json.dump(icon_json, f)

    return copy


def wrap_image_with_icon(image, name, workdir):
    icon = os.path.join(workdir, name + ".icon")

    os.makedirs(os.path.join(icon, "Assets"))
    shutil.copy(image, os.path.join(icon, "Assets", "icon.png"))

    icon_json = {
        "groups": [
            {
                "layers": [{"glass": False, "image-name": "icon.png", "name": "icon"}],
                "shadow": {"kind": "none", "opacity": 0},
                "translucency": {"enabled": False, "value": 0},
            }
        ],
        "supported-platforms": {"squares": "shared"},
    }
    with open(os.path.join(icon, "icon.json"), "w") as f:
        json.dump(icon_json, f)

    return icon


def appiconset_images_to_icons(appiconset, workdir):
    with open(os.path.join(appiconset, "Contents.json")) as f:
        images = [
            image
            for image in json.load(f)["images"]
            if image.get("size") == "1024x1024" and "filename" in image
        ]
    light = None
    dark = None
    for image in images:
        appearances = image.get("appearances", [])
        if not appearances:
            light = light or image["filename"]
        elif appearances == [{"appearance": "luminosity", "value": "dark"}]:
            dark = dark or image["filename"]

    light_icon = (
        wrap_image_with_icon(os.path.join(appiconset, light), "light", workdir)
        if light
        else None
    )
    dark_icon = (
        wrap_image_with_icon(os.path.join(appiconset, dark), "dark", workdir)
        if dark
        else None
    )

    light_icon = light_icon or dark_icon
    dark_icon = (
        dark_icon
        or light_icon
        or sys.exit(f"make_launch_image.py: {appiconset} has no 1024x1024 image")
    )

    return light_icon, dark_icon


icon, imageset = os.path.normpath(sys.argv[1]), os.path.normpath(sys.argv[2])
out_name = os.path.splitext(os.path.basename(imageset))[0]
is_dot_icon = icon.endswith(".icon")

with tempfile.TemporaryDirectory() as workdir:
    if is_dot_icon:
        icon = copy_with_theme_background(icon, workdir)
        light_icon = icon
        dark_icon = icon
        points = DOT_ICON_CENTER_POINTS
    else:
        light_icon, dark_icon = appiconset_images_to_icons(icon, workdir)
        points = POINTS

    shutil.rmtree(imageset, ignore_errors=True)
    os.makedirs(imageset)

    entries = []
    for suffix, source, appearance, rendition in (
        ("", light_icon, None, "Default"),
        ("-dark", dark_icon, "dark", "Dark"),
    ):
        if source is None:
            continue
        for scale in SCALES:
            filename = f"{out_name}{suffix}@{scale}x.png"
            output = os.path.join(imageset, filename)
            subprocess.run(
                itertools.chain(
                    (ictool,),
                    (source,),
                    ("--export-image",),
                    ("--output-file", output),
                    ("--platform", "iOS"),
                    ("--rendition", rendition),
                    ("--width", str(POINTS)),
                    ("--height", str(POINTS)),
                    ("--scale", str(scale)),
                ),
                check=True,
            )
            if is_dot_icon:
                pixels = str(points * scale)
                subprocess.run(
                    itertools.chain(
                        ("sips",),
                        ("--cropToHeightWidth", pixels, pixels),
                        (output,),
                    ),
                    check=True,
                )
            entry = {"filename": filename, "idiom": "universal", "scale": f"{scale}x"}
            if appearance:
                entry["appearances"] = [
                    {"appearance": "luminosity", "value": appearance}
                ]
            entries.append(entry)

    with open(os.path.join(imageset, "Contents.json"), "w") as f:
        json.dump({"images": entries, "info": {"author": "xcode", "version": 1}}, f)
