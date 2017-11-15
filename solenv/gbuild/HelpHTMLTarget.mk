# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This gbuild module handles building of localized help HTML packs. The main
# entry point is class HelpHTMLTarget; the other classes should be
# considered private implementation details and not used directly from
# outside of this file.
#


gb_HelpHTMLTarget_get_target_version=$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR)
gb_HelpHTMLTarget_get_target_dir=$(WORKDIR)/HelpHTMLTarget/$(gb_HelpHTMLTarget_get_target_version)

gb_HelpHTMLTarget_HELPFILES_SRC=$(realpath source)
gb_HelpHTMLTarget_XSLTPROC=$(shell which xsltproc)

gb_HelpHTMLTarget_HID2FILE_file=$(gb_HelpHTMLTarget_get_target_dir)/hid2file.js
gb_HelpHTMLTarget_HID2FILE_filter=$(realpath help3xsl/get_url.xsl)


gb_HelpHTMLTarget_get_target_DEPS := $(dir $(call gb_HelpHTMLTarget_get_target,%)).dir \
				$(gb_HelpHTMLTarget_HID2FILE_file)

html: $(gb_HelpHTMLTarget_get_target)
$(warning Done)

gb_HelpHTMLTarget_HID2FILE :
	$(warning hidtofile)
	$(shell rm -f $(gb_HelpHTMLTarget_HID2FILE_file))

$(gb_HelpHTMLTarget_HID2FILE_file) : $(gb_HelpHTMLTarget_HID2FILE)
	$(file >$@, var map={)
	$(file >>$@,$(shell find $(gb_HelpHTMLTarget_HELPFILES_SRC) \
		-type f -name "*.xhp" \
		-exec $(gb_HelpHTMLTarget_XSLTPROC) $(gb_HelpHTMLTarget_HID2FILE_filter) {} + | sort))
	$(file >>$@,};)

$(dir $(call gb_HelpHTMLTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpHTMLTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpHTMLTarget_get_target,%) : $(gb_HelpHTMLTarget_get_target_DEPS)

.PHONY : $(call gb_HelpHTMLTarget_get_clean_target,%)
$(call gb_HelpHTMLTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HTM,3)
	rm -f $(call gb_HelpHTMLTarget_get_target,$*)

# vim: set noet sw=4 ts=4:
