# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

gb_PARTIALBUILD:=T
include $(GBUILDDIR)/gbuild.mk

all: qa/cppumaker.flag qa/cppumaker/cppumaker.flag

%/cppumaker.flag: %/cppumaker.rdb $(gb_UnoApiTarget_CPPUMAKERTARGET)
	$(call gb_Helper_abbreviate_dirs_native, \
		$(call gb_Helper_execute,cppumaker) -O. -BUCR -C $< $(OUTDIR_FOR_BUILD)/bin/udkapi.rdb)
	touch $@

%/cppumaker.rdb: %/types.urd
	$(call gb_Helper_abbreviate_dirs_native, \
		$(call gb_Helper_execute,regmerge) $@ /UCR $<)

%/types.urd: $(SRCDIR)/cppu/%/types.idl
	$(call gb_Helper_abbreviate_dirs_native, \
		$(call gb_Helper_execute,idlc) -O$* -I$(OUTDIR_FOR_BUILD)/idl -cid -we $<)

.DEFAULT_GOAL:=all
.PHONY: all
# vim: set noet sw=4 ts=4:
