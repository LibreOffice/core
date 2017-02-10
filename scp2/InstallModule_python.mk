# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/python))

$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/python,\
	python \
	python_scriptprovider \
	python_librelogo \
))

ifeq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DDISABLE_PYUNO \
))
else ifneq ($(SYSTEM_PYTHON),)
$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DSYSTEM_PYTHON \
))

else

$(eval $(call gb_InstallModule_define_if_set,scp2/python,\
	ENABLE_MACOSX_SANDBOX \
))

$(eval $(call gb_InstallModule_add_defs,scp2/python,\
	-DPYVERSION=$(PYTHON_VERSION) \
))
endif

$(eval $(call gb_InstallModule_add_scpfiles,scp2/python,\
    scp2/source/python/file_python \
    scp2/source/python/file_python_librelogo \
    scp2/source/python/module_python \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/python,\
    scp2/source/python/module_python_librelogo \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
