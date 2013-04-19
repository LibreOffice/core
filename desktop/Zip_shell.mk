# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,shell,$(SRCDIR)/icon-themes/galaxy/brand/shell))

$(eval $(call gb_Zip_add_files,shell,\
    about.svg \
    backing_left.png \
    backing_right.png \
    backing_space.png \
    backing_rtl_left.png \
    backing_rtl_right.png \
))

# vim: set ts=4 sw=4 et:
