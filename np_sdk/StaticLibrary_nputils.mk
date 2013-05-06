# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,nputils))

$(eval $(call gb_StaticLibrary_use_externals,nputils,\
	npapi_headers \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_StaticLibrary_add_defs,nputils,\
	-DNO_X11 \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_exception_objects,nputils,\
	np_sdk/npsdk/npwin \
))
else
$(eval $(call gb_StaticLibrary_add_cobjects,nputils,\
	np_sdk/npsdk/npunix \
))
endif

# vim: set noet sw=4 ts=4:
