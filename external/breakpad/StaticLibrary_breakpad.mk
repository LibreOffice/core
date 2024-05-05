# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,breakpad))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,breakpad))

$(eval $(call gb_StaticLibrary_use_unpacked,breakpad,breakpad))

$(eval $(call gb_StaticLibrary_set_include,breakpad,\
    -I$(gb_UnpackedTarball_workdir)/breakpad/src \
    -I$(gb_UnpackedTarball_workdir)/breakpad/src/client/windows \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,breakpad,\
	-DUNICODE \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,breakpad,cc))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,breakpad,\
	UnpackedTarball/breakpad/src/common/windows/guid_string \
	UnpackedTarball/breakpad/src/client/windows/handler/exception_handler \
	UnpackedTarball/breakpad/src/client/windows/crash_generation/client_info \
	UnpackedTarball/breakpad/src/client/windows/crash_generation/crash_generation_client \
	UnpackedTarball/breakpad/src/client/windows/crash_generation/crash_generation_server \
	UnpackedTarball/breakpad/src/client/windows/crash_generation/minidump_generator \
))

# vim: set noet sw=4 ts=4:
