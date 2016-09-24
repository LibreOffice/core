# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,merged))

# gb_MERGEDLIBS is defined in solenv/gbuild/extensions/pre_MergedLibsList.mk
$(eval $(call gb_Library_use_library_objects,merged,\
	$(gb_MERGEDLIBS) \
))

$(eval $(call gb_Library_use_libraries,merged,\
	cppu \
	cppuhelper \
	$(if $(ENABLE_JAVA), \
		jvmaccess \
		jvmfwk) \
	sal \
	salhelper \
	xmlreader \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
# prevent warning spamming
$(eval $(call gb_Library_add_ldflags,merged,\
	/ignore:4049 \
	/ignore:4217 \
))
# cursors
$(eval $(call gb_Library_add_nativeres,merged,vcl/salsrc))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_libraries,merged,\
	AppleRemote \
))
endif

# vim: set noet sw=4 ts=4:
