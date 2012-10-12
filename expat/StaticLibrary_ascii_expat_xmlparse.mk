# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,ascii_expat_xmlparse))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,ascii_expat_xmlparse))

$(eval $(call gb_StaticLibrary_use_unpacked,ascii_expat_xmlparse,expat))

# no configure step on windows, no dependency
ifneq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_StaticLibrary_use_external_project,ascii_expat_xmlparse,expat))
endif

$(eval $(call gb_StaticLibrary_set_include,ascii_expat_xmlparse,\
    -I$(call gb_UnpackedTarball_get_dir,expat) \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)
ifneq ($(strip $(SYSBASE)),)
$(eval $(call gb_StaticLibrary_add_defs,ascii_expat_xmlparse,\
    -DHAVE_MEMMOVE \
    -DHAVE_BCOPY \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,ascii_expat_xmlparse,\
    -DCOMPILED_FROM_DSP \
))
else
$(eval $(call gb_StaticLibrary_add_defs,ascii_expat_xmlparse,\
    -DHAVE_EXPAT_CONFIG_H \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,ascii_expat_xmlparse,\
	UnpackedTarball/expat/lib/xmlparse \
))

# vim: set noet sw=4 ts=4:
