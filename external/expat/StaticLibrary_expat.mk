# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,expat))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,expat))

$(eval $(call gb_StaticLibrary_use_unpacked,expat,expat))

# no configure step on windows, no dependency
ifneq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_use_external_project,expat,expat,full))
endif

$(eval $(call gb_StaticLibrary_set_include,expat,\
    -I$(call gb_UnpackedTarball_get_dir,expat) \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)
ifneq ($(strip $(SYSBASE)),)
$(eval $(call gb_StaticLibrary_add_defs,expat,\
    -DHAVE_MEMMOVE \
    -DHAVE_BCOPY \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,expat,\
    -DCOMPILED_FROM_DSP \
))
else
$(eval $(call gb_StaticLibrary_add_defs,expat,\
    -DHAVE_EXPAT_CONFIG_H \
))
endif

# suppress warning spam
$(eval $(call gb_StaticLibrary_add_cflags,expat,\
	-w \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,expat,\
	UnpackedTarball/expat/lib/xmlparse \
	UnpackedTarball/expat/lib/xmlrole \
	UnpackedTarball/expat/lib/xmltok \
))

# vim: set noet sw=4 ts=4:
