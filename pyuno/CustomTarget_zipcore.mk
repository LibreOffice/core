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
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
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

$(eval $(call gb_CustomTarget_CustomTarget,pyuno/zipcore))

# system python (only mingw)
ifeq ($(SYSTEM_PYTHON),YES)
# mingw: MINGW_SYSROOT is defined in configure
ifeq ($(OS)$(COM),WNTGCC)
pyuno_PYTHON_LIB_DIR=$(MINGW_SYSROOT)/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)
endif
else
pyuno_PYTHON_LIB_DIR=$(OUTDIR)/lib/python
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
