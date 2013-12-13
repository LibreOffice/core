# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

ifeq ($(WORKDIR),)
include ../../config_host.mk
endif

all: ant.properties
	$(ANT) debug

clean: ant.properties
	$(ANT) -quiet -keep-going clean

# library references must be relative, let python do the magic.
ant.properties: $(WORKDIR)/appcompat/build.xml
	python -c "import os.path; print 'android.library.reference.1='+ os.path.relpath('$(<D)', '$(CURDIR)')" > $@ || rm $@

# sdk-manager installs it without an build.xml, to not mess with files outside of the
# build-tree, copy to workdir
$(WORKDIR)/appcompat/build.xml:
	-rm -r $(WORKDIR)/appcompat
	cp -a $(ANDROID_SDK_HOME)/extras/android/support/v7/appcompat $(WORKDIR)/appcompat
	$(ANDROID_SDK_HOME)/tools/android update project -p $(WORKDIR)/appcompat

install: all
	$(ANDROID_SDK_HOME)/platform-tools/adb install -r ./bin/ImpressRemote-debug.apk

# vim: set noet sw=4 ts=4:
