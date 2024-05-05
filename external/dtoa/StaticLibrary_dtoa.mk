# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,dtoa))

$(eval $(call gb_StaticLibrary_use_unpacked,dtoa,dtoa))

# A place that duplicates these settings is jurt/Library_jpipe.mk
$(eval $(call gb_StaticLibrary_add_defs,dtoa,\
    $(if $(filter little,$(ENDIANNESS)),-DIEEE_8087,-DIEEE_MC68k)\
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,dtoa))

$(eval $(call gb_StaticLibrary_set_include,dtoa,\
    -I$(gb_UnpackedTarball_workdir)/dtoa/src/\
    $$(INCLUDE)\
))

$(eval $(call gb_StaticLibrary_add_exception_objects,dtoa,\
    external/dtoa/source/dtoa \
))

# vim: set noet sw=4 ts=4:
