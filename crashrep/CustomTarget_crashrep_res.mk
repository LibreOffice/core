# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,crashrep/source/win32))

$(eval $(call gb_CustomTarget_register_targets,crashrep/source/win32, \
	crashrep.ulf \
	crashrep_impl.rc \
))

$(call gb_CustomTarget_get_workdir,crashrep/source/win32)/crashrep_impl.rc : \
	$(call gb_CustomTarget_get_workdir,crashrep/source/win32)/crashrep.ulf \
	$(SRCDIR)/crashrep/source/win32/rcfooter.txt \
	$(SRCDIR)/crashrep/source/win32/rcheader.txt \
	$(SRCDIR)/crashrep/source/win32/rctemplate.txt \
	$(call gb_Executable_get_runtime_dependencies,lngconvex)

#BRAND_BASE_DIR=$(call gb_Helper_make_url,$(OUTDIR)/unittest/install)
$(call gb_CustomTarget_get_workdir,crashrep/source/win32)/crashrep_impl.rc :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),LCX,1)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(SRCDIR)/crashrep/source/win32 && \
		$(call gb_Executable_get_command,lngconvex) \
			-ulf $< \
			-rc $@ \
			-rct rctemplate.txt \
			-rch rcheader.txt \
			-rcf rcfooter.txt)

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(call gb_CustomTarget_get_workdir,crashrep/source/win32)/crashrep.ulf,\
	$(SRCDIR)/crashrep/source/win32/crashrep.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/crashrep/source/win32.po)))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
