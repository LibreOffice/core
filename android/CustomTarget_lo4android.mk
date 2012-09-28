# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,android/lo4android))

lo4android_DIR := $(call gb_CustomTarget_get_workdir,android/experimental/LibreOffice4Android)

$(call gb_CustomTarget_get_target,android/lo4android) : \
	$(lo4android_DIR)/done

# We know that CustomTarget_docloader.mk is included before this file in
# Module_android-mk, so docloader_DIR is defined.  We want that to be built
# completely first, so that we can serialize Ant access to Bootstrap and
# abs-lib, which are used by LibreOffice4Android, DocumentLoader and
# sdremote. We don't want one Ant to be cleaning out one place while another
# is building stuff that depends on it. Yeah, this sucks

$(lo4android_DIR)/done : $(docloader_DIR)/done
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,1)
	cd $(SRCDIR)/android/experimental/LibreOffice4Android && $(MAKE) clean && $(MAKE) all
	mkdir -p $(SRCDIR)/instsetoo_native/$(INPATH)/bin
	cp $(SRCDIR)/android/experimental/LibreOffice4Android/bin/*-debug.apk $(SRCDIR)/instsetoo_native/$(INPATH)/bin

# vim: set noet sw=4 ts=4:
