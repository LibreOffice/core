# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,readlicense_oo/license))

readlicense_oo_DIR := $(call gb_CustomTarget_get_workdir,readlicense_oo/license)

$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/license.txt

$(readlicense_oo_DIR)/license.txt : \
		$(SRCDIR)/readlicense_oo/txt/license.txt \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AWK,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(gb_AWK) 'sub("$$","\r")' $< > $@.tmp && mv $@.tmp $@ \
	)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
