# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,expat_utf16))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,expat_utf16))

$(eval $(call gb_StaticLibrary_use_unpacked,expat_utf16,expat))

# no configure step on windows, no dependency
ifneq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_StaticLibrary_use_external_project,expat_utf16,expat))
endif

$(eval $(call gb_StaticLibrary_add_defs,expat_utf16,\
    -DXML_UNICODE \
))

$(eval $(call gb_StaticLibrary_set_include,expat_utf16,\
    -I$(call gb_UnpackedTarball_get_dir,expat) \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)
ifneq ($(strip $(SYSBASE)),)
$(eval $(call gb_StaticLibrary_add_defs,expat_utf16,\
    -DHAVE_MEMMOVE \
    -DHAVE_BCOPY \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,expat_utf16,\
    -DCOMPILED_FROM_DSP \
))
else
$(eval $(call gb_StaticLibrary_add_defs,expat_utf16,\
    -DHAVE_EXPAT_CONFIG_H \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,expat_utf16,\
	UnpackedTarball/expat/lib/unicode_xmlparse \
	UnpackedTarball/expat/lib/unicode_xmlrole \
	UnpackedTarball/expat/lib/unicode_xmltok \
))

# vim: set noet sw=4 ts=4:
