# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,touch/touch_javamaker))

touch_javamaker_DIR := $(call gb_CustomTarget_get_workdir,touch)

$(call gb_CustomTarget_get_target,touch/touch_javamaker) : $(touch_javamaker_DIR)/done

$(touch_javamaker_DIR)/done : $(call gb_UnoApiTarget_get_target,touch) $(OUTDIR)/bin/types.rdb \
		$(call gb_Executable_get_target_for_build,javamaker) | $(touch_javamaker_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_Helper_execute,javamaker -BUCR -nD -O$(touch_javamaker_DIR)/class -X$(OUTDIR)/bin/types.rdb $<) && touch $@)

# vim: set noet sw=4 ts=4:
