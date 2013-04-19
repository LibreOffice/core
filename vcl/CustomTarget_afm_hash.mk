#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,vcl/generic/fontmanager))

$(call gb_CustomTarget_get_target,vcl/generic/fontmanager) : \
	$(call gb_CustomTarget_get_workdir,vcl/generic/fontmanager)/afm_hash.hpp

$(call gb_CustomTarget_get_workdir,vcl/generic/fontmanager)/afm_hash.hpp : \
		$(SRCDIR)/vcl/generic/fontmanager/afm_keyword_list \
		| $(call gb_CustomTarget_get_workdir,vcl/generic/fontmanager)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),GPF,1)
	$(GPERF) -C -t -l -L C++ -m 20 -Z AfmKeywordHash -k '1,4,6,$$' $< | sed -e "s/(char\*)0/(char\*)0, NOPE/g" | grep -v "^#line" > $@

# vim: set noet sw=4:
