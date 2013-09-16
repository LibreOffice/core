# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,libtest))

$(eval $(call gb_Executable_set_include,libtest,\
	$$(INCLUDE) \
	-I$(SRCDIR)/desktop/inc \
))

$(eval $(call gb_Executable_use_libraries,libtest,\
	libreoffice \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_libs,libtest,\
	-pthread \
))

$(eval $(call gb_Executable_add_exception_objects,libtest,\
	smoketest/libtest \
))

liblibreoffice_OWN_LD_PATH_DIR := $(gb_DEVINSTALLROOT)/program/libolib
liblibreoffice_OWN_LD_SO := $(liblibreoffice_OWN_LD_PATH_DIR)/$(call gb_Library__get_workdir_linktargetname,libreoffice)

$(liblibreoffice_OWN_LD_SO) : $(call gb_Library_get_target,libreoffice)
	mkdir -p $(liblibreoffice_OWN_LD_PATH_DIR)/Library
	cp -a $(call gb_Library_get_target,libreoffice) $(liblibreoffice_OWN_LD_SO)

run_libtest: $(liblibreoffice_OWN_LD_SO)
	$(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}":$(liblibreoffice_OWN_LD_PATH_DIR)/Library" \
	$(WORKDIR)/LinkTarget/Executable/libtest \
		$(gb_DEVINSTALLROOT)/program $(OUTDIR)/bin/smoketestdoc.sxw \

# vim: set noet sw=4 ts=4:
