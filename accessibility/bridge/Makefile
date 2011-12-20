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

gb_PARTIALBUILD := T
include $(GBUILDDIR)/gbuild.mk

TARGETDIR := $(WORKDIR)/CustomTarget/accessibility/bridge
PACKAGE := org/openoffice/java/accessibility

ifeq ($(PRODUCT),)
DEBUGSWITCH := true
PRODUCTSWITCH := false
else
DEBUGSWITCH := false
PRODUCTSWITCH := true
endif

$(TARGETDIR)/done : $(TARGETDIR)/classes/$(PACKAGE)/Build.class
	mkdir -p $(dir $@) && touch $@

$(TARGETDIR)/classes/$(PACKAGE)/Build.class : $(TARGETDIR)/src/$(PACKAGE)/Build.java | $(gb_JavaClassSet_JAVACCOMMAND)
	mkdir -p $(dir $@) && \
        $(gb_JavaClassSet_JAVACCOMMAND) $(gb_JavaClassSet_JAVACDEBUG) -d $(TARGETDIR)/classes $^

$(TARGETDIR)/src/$(PACKAGE)/Build.java :
	mkdir -p $(dir $@) && (\
        echo package org.openoffice.java.accessibility\; && \
        echo public class Build \{ && \
        echo public static final boolean DEBUG = $(DEBUGSWITCH)\; && \
        echo public static final boolean PRODUCT = $(PRODUCTSWITCH)\; && \
        echo \} \
        ) > $@

.DEFAULT_GOAL := all
.PHONY : all
all : \
	$(TARGETDIR)/done

# vim: set ts=4 sw=4 et:
