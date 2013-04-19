# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,accessibility/bridge))

$(call gb_CustomTarget_get_target,accessibility/bridge) : \
    $(call gb_CustomTarget_get_workdir,accessibility/bridge)/org/openoffice/java/accessibility/Build.java

ifeq ($(ENABLE_DBGUTIL),TRUE)
accessibility_DEBUG := true
accessibility_PRODUCT := false
else
accessibility_DEBUG := false
accessibility_PRODUCT := true
endif

$(call gb_CustomTarget_get_workdir,accessibility/bridge)/org/openoffice/java/accessibility/Build.java :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	mkdir -p $(dir $@) && (\
        echo package org.openoffice.java.accessibility\; && \
        echo public class Build \{ && \
        echo public static final boolean DEBUG = $(accessibility_DEBUG)\; && \
        echo public static final boolean PRODUCT = $(accessibility_PRODUCT)\; && \
        echo \} \
        ) > $@

# vim: set ts=4 sw=4 et:
