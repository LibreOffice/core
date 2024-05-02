# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,android/loandroid3))

loandroid3_DIR := $(gb_CustomTarget_workdir)/android/source

$(call gb_CustomTarget_get_target,android/loandroid3) : \
	$(loandroid3_DIR)/done

$(loandroid3_DIR)/done : $(call gb_Postprocess_get_target,AllModulesButInstsetNative)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),MAK)
	cd $(SRCDIR)/android/source && $(MAKE) all
ifeq ($(ENABLE_JAVA),TRUE)
# Copy to $(BUILDDIR)/instsetoo_native as that is where the tinderbox build script
# still looks for the .apk, and we want fresh daily builds to be uploaded.
# Us "foo" instead of the old INPATH
	mkdir -p $(BUILDDIR)/instsetoo_native/foo/bin; \
	if test "$$ENABLE_ANDROID_LOK" != "TRUE" ; then cp $(SRCDIR)/android/source/build/outputs/apk/strippedUI$(if $(ENABLE_ANDROID_EDITING),Editing)/$(if $(ENABLE_RELEASE_BUILD),release/*-release-unsigned.apk,debug/*-debug.apk) $(BUILDDIR)/instsetoo_native/foo/bin ; fi
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),MAK)
endif

$(call gb_CustomTarget_get_clean_target,android/loandroid3) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),MAK,2)
	cd $(SRCDIR)/android/source && $(MAKE) clean

# vim: set noet sw=4 ts=4:
