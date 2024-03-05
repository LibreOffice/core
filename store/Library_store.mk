# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,store))

$(eval $(call gb_Library_add_defs,store,\
	-DSTORE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,store,\
	sal \
	salhelper \
))

$(eval $(call gb_Library_add_exception_objects,store,\
	store/source/object \
	store/source/lockbyte \
	store/source/storbase \
	store/source/storbios \
	store/source/storcach \
	store/source/stordata \
	store/source/stordir \
	store/source/storlckb \
	store/source/stortree \
	store/source/storpage \
	store/source/store \
))

# vim: set noet sw=4 ts=4:
