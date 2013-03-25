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

$(eval $(call gb_CustomTarget_CustomTarget,shell/source/win32/shlxthandler/res))

$(eval $(call gb_CustomTarget_register_targets,shell/source/win32/shlxthandler/res,\
	shlxthdl.ulf \
	shlxthdl_impl.rc \
))

$(call gb_CustomTarget_get_workdir,shell/source/win32/shlxthandler/res)/shlxthdl_impl.rc : \
		$(call gb_CustomTarget_get_workdir,shell/source/win32/shlxthandler/res)/shlxthdl.ulf \
		$(SRCDIR)/shell/source/win32/shlxthandler/res/rcfooter.txt \
		$(SRCDIR)/shell/source/win32/shlxthandler/res/rcheader.txt \
		$(SRCDIR)/shell/source/win32/shlxthandler/res/rctmpl.txt \
		$(call gb_Executable_get_runtime_dependencies,lngconvex)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),LCX,1)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(SRCDIR)/shell/source/win32/shlxthandler/res && \
		BRAND_BASE_DIR=$(call gb_Helper_make_url,$(OUTDIR)/unittest/install) \
		$(call gb_Executable_get_command,lngconvex) \
			-ulf $< \
			-rc $@ \
			-rct rctmpl.txt \
			-rch rcheader.txt \
			-rcf rcfooter.txt)

$(call gb_CustomTarget_get_workdir,shell/source/win32/shlxthandler/res)/shlxthdl.ulf : \
		$(SRCDIR)/shell/source/win32/shlxthandler/res/shlxthdl.ulf \
		$(call gb_Executable_get_runtime_dependencies,ulfex)
	MERGEINPUT=`$(gb_MKTEMP)` && \
	echo $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(subst $(SRCDIR)/,,$(dir $<))).po) > $${MERGEINPUT} && \
	$(call gb_Executable_get_command,ulfex) -i $< -o $@ -m $${MERGEINPUT} -l all && \
	rm -f $${MERGEINPUT}

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
