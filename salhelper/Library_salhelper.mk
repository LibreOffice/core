# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,salhelper))

$(eval $(call gb_Library_add_defs,salhelper,\
	-DSALHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_is_ure_library_or_dependency,salhelper))

$(eval $(call gb_Library_use_libraries,salhelper,\
	sal \
))

$(eval $(call gb_Library_add_exception_objects,salhelper,\
    salhelper/source/condition \
    salhelper/source/dynload \
    salhelper/source/simplereferenceobject \
    salhelper/source/thread \
    salhelper/source/timer \
))

$(eval $(call gb_Library_set_soversion,salhelper))

# vim: set noet sw=4 ts=4:
