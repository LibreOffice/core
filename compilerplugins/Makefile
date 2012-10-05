# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(SOLARENV),)
ifeq ($(gb_Side),)
gb_Side:=host
endif
include $(dir $(realpath $(lastword $(MAKEFILE_LIST))))../config_$(gb_Side).mk
endif

include $(SRCDIR)/compilerplugins/Makefile.mk

all: build
build: compilerplugins
clean: compilerplugins-clean

# vim: set noet sw=4 ts=4:
