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

include $(OUTDIR)/inc/pyversion.Makefile
pyuno_PYTHON_ARCHIVE_NAME:=python-core-$(PYVERSION).zip
FIND=find
GREP=grep

$(call gb_CustomTarget_get_target,pyuno/zipcore) : \
    $(call gb_CustomTarget_get_workdir,pyuno/zipcore)/$(pyuno_PYTHON_ARCHIVE_NAME)

# capture the files to have them in prerequisite list
pyuno_zipcore_FINDLIBFILES:=\
    $(shell $(FIND) $(OUTDIR)/lib/python -type f| $(GREP) -v "\.pyc" | $(GREP) -v "\.py~" | $(GREP) -v .orig | $(GREP) -v _failed)

# create zip archive
$(call gb_CustomTarget_get_workdir,pyuno/zipcore)/$(pyuno_PYTHON_ARCHIVE_NAME) : $(pyuno_zipcore_FINDLIBFILES) | \
    $(call gb_CustomTarget_get_workdir,pyuno/zipcore)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ZIP,1)
	cd $(OUTDIR)/lib/python && zip $@ $(shell cd $(OUTDIR)/lib/python && $(FIND) . -type f | $(GREP) -v "\.pyc" | $(GREP) -v "\.py~" | $(GREP) -v .orig | $(GREP) -v _failed)

# vim: set noet sw=4 ts=4:
