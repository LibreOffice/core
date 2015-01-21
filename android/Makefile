# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(module_directory)/../solenv/gbuild/partial_build.mk

.PHONY: sign

SIGNED_APK := $(BUILDDIR)/android/experimental/LOAndroid3/bin/LibreOfficeViewer.apk

sign: build
	rm -f $(SIGNED_APK)
	jarsigner --verbose -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.keystore $(BUILDDIR)/android/experimental/LOAndroid3/bin/LibreOfficeViewer-release-unsigned.apk $(key)
	$(ANDROID_SDK_HOME)/build-tools/*/zipalign -v 4 $(BUILDDIR)/android/experimental/LOAndroid3/bin/LibreOfficeViewer-release-unsigned.apk $(SIGNED_APK)
	@echo
	@echo "Resulting signed apk: $(SIGNED_APK)"

# vim: set noet sw=4 ts=4:
