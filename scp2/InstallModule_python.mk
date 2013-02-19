# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DPYVERSION=$(PYTHON_VERSION) \
	-DPYMAJMIN=$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
	-DPYMAJOR=$(PYTHON_VERSION_MAJOR) \
	-DPY_FULL_DLL_NAME=$(if $(filter WNT-MSC,$(OS)-$(COM)),python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR).dll,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m.so) \
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
