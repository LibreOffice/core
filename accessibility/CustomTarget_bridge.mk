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
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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
