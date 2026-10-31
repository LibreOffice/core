#!/bin/sh
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Render and encode the sample video backgrounds as VP9 in WebM. Needs python3 with numpy and
# pillow, and gst-launch-1.0 with the vpx plugin. Constant quality mode keeps the smooth clips
# small.
set -e
cd "$(dirname "$0")"
encode() {
  gst-launch-1.0 -q -e fdsrc fd=0 ! rawvideoparse width=1280 height=720 format=rgb framerate=30/1 \
    ! videoconvert ! video/x-raw,format=I420 \
    ! vp9enc end-usage=cq cq-level=32 target-bitrate=4000000 keyframe-max-dist=600 \
      deadline=1000000 cpu-used=2 row-mt=true threads=8 \
    ! webmmux ! filesink location="$1"
}
python3 -I make_clouds.py day | encode clouds-day.webm
python3 -I make_clouds.py dusk | encode clouds-dusk.webm
python3 -I make_bokeh.py | encode bokeh.webm
