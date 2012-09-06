# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boostthread))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boostthread))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boostthread,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_unpacked,boostthread,boost))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boostthread,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boostthread,\
	UnpackedTarball/boost/libs/thread/src/win32/thread \
	UnpackedTarball/boost/libs/thread/src/win32/tss_dll \
	UnpackedTarball/boost/libs/thread/src/win32/tss_pe \
))

# vim: set noet sw=4 ts=4:
