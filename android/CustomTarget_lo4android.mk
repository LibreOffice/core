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

# We know that CustomTarget_sdremote.mk is included first, so
# sdremote_DIR is defined. We want that to be built completely first,
# so that we can serialize Ant access to Bootstrap, which is used both
# by DocumentLoader and sdremote. We don't want one Ant to be cleaning
# out Bootstrap while another is building stuff that depends on it.
# Yeah, this sucks

$(lo4android_DIR)/done : $(sdremote_DIR)/done $(call gb_Postprocess_get_target,AllModulesButInstsetNative)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MAK,2)
	cd $(SRCDIR)/android/experimental/LibreOffice4Android && $(MAKE) all

$(call gb_CustomTarget_get_clean_target,android/lo4android) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),MAK,2)
	cd $(SRCDIR)/android/experimental/LibreOffice4Android && $(MAKE) clean

# vim: set noet sw=4 ts=4:
