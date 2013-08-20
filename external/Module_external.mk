# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,external))

$(eval $(call gb_Module_add_targets,external,\
))

ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_Module_add_targets,external,\
	CustomTarget_jawt \
	Package_jawt \
	Package_mingw_dlls \
	Package_mingw_gccdlls \
))
endif

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,external,\
	Package_dbghelp \
	$(if $(MSM_PATH),Package_msms) \
	Package_msvc_dlls \
	$(if $(filter YES,$(WITH_MOZAB4WIN)),Package_msvc80_dlls) \
))
endif

ifeq ($(HAVE_READDIR_R),NO)
ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,external,\
	UnpackedTarball_glibc \
	StaticLibrary_gnu_readdir_r \
))
endif
endif

# vim: set noet sw=4 ts=4:
