# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
#  (initial developer)
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,zlib/source))

ZLSO := $(call gb_CustomTarget_get_workdir,zlib/source)

$(call gb_CustomTarget_get_target,zlib/source) : $(ZLSO)/done

# FIXME: do not hardcode the path here
ifeq ($(OS_FOR_BUILD),WNT)
zl_FIXED_TARFILE_LOCATION := $(shell cygpath -u $(TARFILE_LOCATION))/c735eab2d659a96e5a594c9e8541ad63-zlib-1.2.5.tar.gz
else
zl_FIXED_TARFILE_LOCATION := $(TARFILE_LOCATION)/c735eab2d659a96e5a594c9e8541ad63-zlib-1.2.5.tar.gz
endif

$(ZLSO)/done : $(zl_FIXED_TARFILE_LOCATION)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TAR,1)
	$(call gb_Helper_abbreviate_dirs, \
	mkdir -p $(dir $@) && cd $(dir $@) && \
	$(GNUTAR) -x -z $(STRIP_COMPONENTS)=1 -f $< && \
    $(GNUPATCH) -p1 < $(SRCDIR)/zlib/patches/zlib-1.2.5-gentoo.patch && \
    $(GNUPATCH) -p1 < $(SRCDIR)/zlib/patches/minizip-null.patch && \
    $(GNUPATCH) -p1 < $(SRCDIR)/zlib/patches/zlib-1.2.5-minizip-fixuncrypt.patch && \
    $(GNUPATCH) -p1 < $(SRCDIR)/zlib/patches/zlib-1.2.5-zip64.patch )
	touch $@

# vim: set noet sw=4 ts=4:
