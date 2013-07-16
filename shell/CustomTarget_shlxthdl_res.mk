# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(call gb_CustomTarget_get_workdir,shell/source/win32/shlxthandler/res)/shlxthdl.ulf,\
	$(SRCDIR)/shell/source/win32/shlxthandler/res/shlxthdl.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/shell/source/win32/shlxthandler/res.po)))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
