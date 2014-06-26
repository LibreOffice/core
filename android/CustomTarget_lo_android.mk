# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,android/loandroid3))

loandroid3_DIR := $(call gb_CustomTarget_get_workdir,android/experimental/LOAndroid3)

$(call gb_CustomTarget_get_target,android/loandroid3) : \
	$(loandroid3_DIR)/done

# We want that to be built completely first,
# so that we can serialize Ant access to Bootstrap, which is used both
# by DocumentLoader. We don't want one Ant to be cleaning
# out Bootstrap while another is building stuff that depends on it.
# Yeah, this sucks

$(loandroid3_DIR)/done : $(call gb_Postprocess_get_target,AllModulesButInstsetNative)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,2)
	cd $(SRCDIR)/android/experimental/LOAndroid3 && $(MAKE) all
# Copy to $(BUILDDIR)/instsetoo_native as that is where the tinderbox build script
# still looks for the .apk, and we want fresh daily builds to be uploaded. Even if
# the apps as such are mostly useless.
# Us "foo" instead of the old INPATH
	mkdir -p $(BUILDDIR)/instsetoo_native/foo/bin; \
	cp $(SRCDIR)/android/experimental/LOAndroid3/bin/*-debug.apk $(BUILDDIR)/instsetoo_native/foo/bin

$(call gb_CustomTarget_get_clean_target,android/loandroid3) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),MAK,2)
	cd $(SRCDIR)/android/experimental/LOAndroid3 && $(MAKE) clean

# vim: set noet sw=4 ts=4:
