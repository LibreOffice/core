# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,sdext/pdfimport))

$(call gb_CustomTarget_get_target,sdext/pdfimport) : \
	$(call gb_CustomTarget_get_workdir,sdext/pdfimport)/hash.cxx

$(call gb_CustomTarget_get_workdir,sdext/pdfimport)/hash.cxx : \
		$(SRCDIR)/sdext/source/pdfimport/wrapper/keyword_list \
		| $(call gb_CustomTarget_get_workdir,sdext/pdfimport)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GPF,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),GPF)
	$(call gb_Helper_wsl_path,$(GPERF) -C -t -l -L C++ -m 20 --switch=2 --readonly-tables -Z PdfKeywordHash -k'4-5$(COMMA)$$' $<) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),GPF)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
