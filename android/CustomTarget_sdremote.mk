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

$(sdremote_DIR)/done : $(gb_Helper_PHONY)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,1)
# Lock needed to serialize the Ant cleaning/building which cleans/builds also
# abs-lib both for DocumentLoader and sdremote. We don't want one Ant to be
# cleaning out abs-lib while another is building stuff that depends on
# it. yeah, this sucks
	cd $(SRCDIR)/android/sdremote && flock $(SRCDIR)/android/lock sh -c "$(MAKE) clean && $(MAKE) all"
	mkdir -p $(SRCDIR)/instsetoo_native/$(INPATH)/bin
	cp $(SRCDIR)/android/sdremote/bin/ImpressRemote-debug.apk $(SRCDIR)/instsetoo_native/$(INPATH)/bin

# vim: set noet sw=4 ts=4:
