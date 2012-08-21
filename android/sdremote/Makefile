# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include ../../config_host.mk

all: properties
	mkdir -p ../abs-lib/libs
	cp $(ANDROID_SDK_HOME)/extras/android/support/v4/android-support-v4.jar ../abs-lib/libs
	$(ANT) debug

properties:
	echo sdk.dir=$(ANDROID_SDK_HOME) >local.properties
	echo sdk.dir=$(ANDROID_SDK_HOME) >../abs-lib/local.properties

clean: properties
	$(ANT) clean

# vim: set noet sw=4 ts=4:
