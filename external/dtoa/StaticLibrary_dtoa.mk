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

# A place that duplicates these settings is jurt/source/pipe/staticsalhack.cxx:
$(eval $(call gb_StaticLibrary_add_cflags,dtoa,\
    $(if $(filter little,$(ENDIANNESS)),-DIEEE_8087,-DIEEE_MC68k) \
    -DMULTIPLE_THREADS \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,dtoa))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,dtoa,\
    UnpackedTarball/dtoa/src/dtoa \
))

# This static library uses osl::Mutex from sal (see external/dtoa/source/dtoa_locking.cxx); sal
# links this library, so dependency of dtoa on sal is not defined here to avoid circular dependency

$(eval $(call gb_StaticLibrary_add_exception_objects,dtoa,\
    external/dtoa/source/dtoa_locking \
))

# vim: set noet sw=4 ts=4:
