# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,pyuno/zipcore))

# system python (only mingw)
ifeq ($(SYSTEM_PYTHON),YES)
# mingw: MINGW_SYSROOT is defined in configure
ifeq ($(OS)$(COM),WNTGCC)
pyuno_PYTHON_LIB_DIR=$(MINGW_SYSROOT)/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)
endif
else
$(error SHOULD NOT GET HERE)
endif

pyuno_PYTHON_ARCHIVE_NAME:=python-core-$(PYTHON_VERSION).zip

$(call gb_CustomTarget_get_target,pyuno/zipcore) : \
    $(call gb_CustomTarget_get_workdir,pyuno/zipcore)/$(pyuno_PYTHON_ARCHIVE_NAME)

# create zip archive
$(call gb_CustomTarget_get_workdir,pyuno/zipcore)/$(pyuno_PYTHON_ARCHIVE_NAME) \
		: $(call gb_ExternalPackage_get_target,python3) \
		| $(call gb_CustomTarget_get_workdir,pyuno/zipcore)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ZIP,1)
	cd $(pyuno_PYTHON_LIB_DIR) && zip -q $@ $(shell cd $(pyuno_PYTHON_LIB_DIR) && find . -type f | grep -v "\.pyc" | grep -v "\.py~" | grep -v .orig | grep -v _failed)

# vim: set noet sw=4 ts=4:
