# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,mingw_gccdlls,$(MINGW_SYSROOT)/bin))

$(eval $(call gb_Package_set_outdir,mingw_gccdlls,$(INSTDIR)))

$(eval $(call gb_Package_add_files,mingw_gccdlls,program,\
    $(if $(MINGW_SHARED_GCCLIB),$(MINGW_GCCDLL)) \
    $(if $(MINGW_SHARED_GXXLIB),$(MINGW_GXXDLL)) \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
