# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# dmgbuild settings for the installer disk image. Not run directly: macos/
# make-dmg.sh passes every value in through -D, so the geometry stays defined
# in make-dmg-background.py alone. See
# https://dmgbuild.readthedocs.io/en/latest/settings.html

import os

application = defines["app"]
appname = os.path.basename(application)


def geometry(key):
    return int(defines[key])


format = "UDZO"
filesystem = "HFS+"

files = [application]
symlinks = {"Applications": "/Applications"}
hide_extensions = [appname]

# dmgbuild picks up the @2x sibling of this file by itself and compiles the two
# into a multi-resolution TIFF, so that Finder does not upscale the background
# on Retina displays.
background = defines["background"]

# Without a volume icon the disk image gets the generic white one.
icon = defines.get("volicon") or None

default_view = "icon-view"
show_status_bar = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
arrange_by = None
show_item_info = False
show_icon_preview = False
label_pos = "bottom"

icon_size = geometry("icon_size")
text_size = 12

# The y of 100000 is the idiom from the dmgbuild documentation for "put the
# window near the top of the screen", whatever the screen turns out to be.
window_rect = ((100, 100000), (geometry("win_w"), geometry("win_h")))

# Icon centres, relative to the top left of the window content area.
icon_locations = {
    appname: (geometry("app_x"), geometry("app_y")),
    "Applications": (geometry("drop_x"), geometry("drop_y")),
}
