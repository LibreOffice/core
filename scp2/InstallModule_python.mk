# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/python))

ifeq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DDISABLE_PYUNO \
))
else ifeq ($(SYSTEM_PYTHON),YES)
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DSYSTEM_PYTHON \
))

# mingw: mix mode copy file from system python to installation set
ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DPYVERSION=$(PYTHON_VERSION) \
	-DMINGW_SYSTEM_PYTHON \
))
endif

else

$(eval $(call gb_InstallModule_define_if_set,scp2/python,\
	ENABLE_MACOSX_MACLIKE_APP_STRUCTURE \
	ENABLE_MACOSX_SANDBOX \
))

$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DPYVERSION=$(PYTHON_VERSION) \
	-DPYMAJMIN=$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
	-DPYMAJOR=$(PYTHON_VERSION_MAJOR) \
	-DPY_FULL_DLL_NAME=$(if $(filter WNT-MSC,$(OS)-$(COM)),python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)$(if $(MSVC_USE_DEBUG_RUNTIME),_d).dll,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m.so) \
	$(if $(filter MSC,$(COM)),$(if $(MSVC_USE_DEBUG_RUNTIME),-DMSVC_PKG_DEBUG_RUNTIME)) \
))
endif

$(eval $(call gb_InstallModule_add_scpfiles,scp2/python,\
    scp2/source/python/file_python \
    scp2/source/python/file_python_librelogo \
    scp2/source/python/profileitem_python \
    scp2/source/python/module_python \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/python,\
    scp2/source/python/module_python_librelogo \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
