# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,android/sdremote))

sdremote_DIR := $(call gb_CustomTarget_get_workdir,android/sdremote)

$(call gb_CustomTarget_get_target,android/sdremote) : $(sdremote_DIR)/done

$(sdremote_DIR)/done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,2)
	cd $(SRCDIR)/android/sdremote && $(MAKE) all
# If SRCDIR==BUILDDIR, copy to $(SRCDIR)/instsetoo_native/$(INPATH)/bin as that is where the tinderbox build script
# still looks for the .apk, and we want fresh daily builds to be uploaded.
	if test $(SRCDIR) = $(BUILDDIR); then \
		mkdir -p $(SRCDIR)/instsetoo_native/$(INPATH)/bin; \
		cp $(SRCDIR)/android/sdremote/bin/ImpressRemote-debug.apk $(SRCDIR)/instsetoo_native/$(INPATH)/bin; \
	fi

$(call gb_CustomTarget_get_clean_target,android/sdremote) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),MAK,2)
	cd $(SRCDIR)/android/sdremote && $(MAKE) clean

# vim: set noet sw=4 ts=4:
