# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/fontconfig))

$(call gb_CustomTarget_get_workdir,extras/fontconfig)/fc_local.conf: \
    $(SRCDIR)/extras/source/truetype/symbol/fc_local.snippet \
    $(SRCDIR)/extras/CustomTarget_fontconfig.mk \
    | $(call gb_CustomTarget_get_workdir,extras/fontconfig)/.dir

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(call gb_CustomTarget_get_workdir,extras/fontconfig)/fc_local.conf: \
    $(SRCDIR)/external/more_fonts/fc_local.snippet
endif

$(call gb_CustomTarget_get_workdir,extras/fontconfig)/fc_local.conf:
	printf '<?xml version="1.0"?>\n<!DOCTYPE fontconfig SYSTEM "/etc/fonts/conf.d/fonts.dtd">\n<fontconfig>\n' >$@
	cat $(SRCDIR)/extras/source/truetype/symbol/fc_local.snippet >>$@
ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
	cat $(SRCDIR)/external/more_fonts/fc_local.snippet >>$@
endif
	printf '</fontconfig>\n' >>$@

# vim: set noet sw=4 ts=4:
