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

SIGNED_APK := $(BUILDDIR)/android/source/bin/LibreOfficeViewer.apk
RELEASE_APK_USAGE := echo; echo "Usage: make versionCode=<version_num+1> exampleDocument=</absolute/path/example.odt> key=<key_name> release-apk"

release-apk: build
	# versionCode and key are mandatory, examplDocument is not
	@if test -z "$(versionCode)" ; then $(RELEASE_APK_USAGE) ; exit 1 ; fi
	@if test -z "$(key)" ; then $(RELEASE_APK_USAGE) ; exit 1 ; fi

	rm -f $(SIGNED_APK)

	# the actual signing
	jarsigner --verbose -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.keystore $(BUILDDIR)/android/source/build/outputs/apk/LibreOfficeViewer-strippedUI-release-unsigned.apk $(key)
	$(ANDROID_SDK_HOME)/build-tools/*/zipalign -v 4 $(BUILDDIR)/android/source/build/outputs/apk/LibreOfficeViewer-strippedUI-release-unsigned.apk $(SIGNED_APK)

	@echo
	@echo "Resulting signed apk: $(SIGNED_APK)"

# vim: set noet sw=4 ts=4:
