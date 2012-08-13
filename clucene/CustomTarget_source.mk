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

$(eval $(call gb_CustomTarget_CustomTarget,clucene/source))

clucene_DIR := $(call gb_CustomTarget_get_workdir,clucene/source)

$(call gb_CustomTarget_get_target,clucene/source) : $(clucene_DIR)/done

# FIXME: do not hardcode the path here
ifeq ($(OS_FOR_BUILD),WNT)
clucene_FIXED_TARFILE_LOCATION := $(shell cygpath -u $(TARFILE_LOCATION))/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
else
clucene_FIXED_TARFILE_LOCATION := $(TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
endif

ifeq ($(OS),WNT)
_CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/_clucene-config-MSVC.h
ifeq ($(COM),MSC)
CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/clucene-config-MSVC.h
else
CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/clucene-config-MINGW.h
endif
else
CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/clucene-config-generic.h
ifeq ($(OS),LINUX)
_CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/_clucene-config-LINUX.h
else
_CLUCENE_CONFIG_H := $(SRCDIR)/clucene/configs/_clucene-config-generic.h
endif
endif

# clucene-multimap-put.patch was proposed upstream, see
# <http://sourceforge.net/mailarchive/message.php?msg_id=29143260>:
$(clucene_DIR)/done : $(clucene_FIXED_TARFILE_LOCATION) \
		$(_CLUCENE_CONFIG_H) $(CLUCENE_CONFIG_H)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TAR,1)
	$(call gb_Helper_abbreviate_dirs, \
	mkdir -p $(dir $@) && cd $(dir $@) && \
	$(GNUTAR) -x -z $(STRIP_COMPONENTS)=1 -f $< && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-internal-zlib.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-warnings.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-gcc-atomics.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-debug.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-narrowing-conversions.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-multimap-put.patch && \
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-nullptr.patch && \
	for i in `find $(clucene_DIR)/ -name "*.cpp"`; do mv $$i $${i%%cpp}cxx; done)
	#FIXME ?, our rules expect .cxx
ifneq ($(OS),WNT)
	#dirent.h is a problem, move it around
	mkdir -p $(clucene_DIR)/inc/internal/CLucene/util
	mv $(clucene_DIR)/src/shared/CLucene/util/dirent.h $(clucene_DIR)/inc/internal/CLucene/util
endif
	#To generate these, run cmake for each sufficiently different platform, customize and stick into configs
	cp $(CLUCENE_CONFIG_H) $(clucene_DIR)/src/shared/CLucene/clucene-config.h
	cp $(_CLUCENE_CONFIG_H) $(clucene_DIR)/src/shared/CLucene/_clucene-config.h
	touch $@

# vim: set noet sw=4 ts=4:
