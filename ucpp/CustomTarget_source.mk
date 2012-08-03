# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,ucpp/source))

$(call gb_CustomTarget_get_target,ucpp/source) : $(call gb_CustomTarget_get_workdir,ucpp/source)/done

# FIXME: do not hardcode the path here
ifeq ($(OS_FOR_BUILD),WNT)
ucpp_FIXED_TARFILE_LOCATION := $(shell cygpath -u $(TARFILE_LOCATION))/0168229624cfac409e766913506961a8-ucpp-1.3.2.tar.gz
else
ucpp_FIXED_TARFILE_LOCATION := $(TARFILE_LOCATION)/0168229624cfac409e766913506961a8-ucpp-1.3.2.tar.gz
endif

$(call gb_CustomTarget_get_workdir,ucpp/source)/done : \
	$(ucpp_FIXED_TARFILE_LOCATION) \
	$(SRCDIR)/ucpp/ucpp.patch \
	| $(call gb_CustomTarget_get_workdir,ucpp/source)/.dir

$(call gb_CustomTarget_get_workdir,ucpp/source)/done :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TAR,1)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $@) && \
		$(GNUTAR) -x -z $(STRIP_COMPONENTS)=1 -f $< && \
		$(GNUPATCH) -p0 < $(SRCDIR)/ucpp/ucpp.patch && \
		touch $@ \
	)

# vim: set noet sw=4 ts=4:
