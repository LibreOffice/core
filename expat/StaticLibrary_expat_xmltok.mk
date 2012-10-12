# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,expat_xmltok))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,expat_xmltok))

$(eval $(call gb_StaticLibrary_use_unpacked,expat_xmltok,expat))

# how can we do that in one liner in gnu make?
#.IF "$(OS)"=="MACOSX" && "$(SYSBASE)"!=""
ifeq ($(OS),MACOSX)
ifneq ($(strip $(SYSBASE)),)
$(eval $(call gb_StaticLibrary_add_defs,expat_xmltok,\
    -DHAVE_MEMMOVE \
    -DHAVE_BCOPY \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,expat_xmltok,\
    -DCOMPILED_FROM_DSP \
))
else # WNT
$(eval $(call gb_StaticLibrary_add_defs,expat_xmltok,\
    -DHAVE_EXPAT_CONFIG_H \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,expat_xmltok,\
	UnpackedTarball/expat/lib/xmltok \
	UnpackedTarball/expat/lib/xmlrole \
))

# vim: set noet sw=4 ts=4:
