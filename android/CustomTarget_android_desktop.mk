# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,android/desktop))

android_desktop_DIR := $(call gb_CustomTarget_get_workdir,android/experimental/Desktop)

$(call gb_CustomTarget_get_target,android/desktop) : \
	$(android_desktop_DIR)/done

# We want that to be built completely first,
# so that we can serialize Ant access to Bootstrap, which is used
# by several LO-android apps. We don't want one Ant to be cleaning
# out Bootstrap while another is building stuff that depends on it.
# Yeah, this sucks

$(android_desktop_DIR)/done : $(call gb_Postprocess_get_target,AllModulesButInstsetNative)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,2)
	cd $(SRCDIR)/android/experimental/desktop && $(MAKE) all
# Copy to $(BUILDDIR)/instsetoo_native as that is where the tinderbox build script
# still looks for the .apk, and we want fresh daily builds to be uploaded. Even if
# the apps as such are mostly useless.
# Us "foo" instead of the old INPATH
	mkdir -p $(BUILDDIR)/instsetoo_native/foo/bin; \
	cp $(SRCDIR)/android/experimental/desktop/bin/*.apk $(BUILDDIR)/instsetoo_native/foo/bin

$(call gb_CustomTarget_get_clean_target,android/desktop) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),MAK,2)
	cd $(SRCDIR)/android/experimental/desktop && $(MAKE) clean

# vim: set noet sw=4 ts=4:
