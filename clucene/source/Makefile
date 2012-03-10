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

gb_PARTIALBUILD := T
include $(GBUILDDIR)/gbuild_simple.mk

ifeq ($(OS_FOR_BUILD),WNT)
FIXED_TARFILE_LOCATION=$(shell cygpath -u $(TARFILE_LOCATION))
else
FIXED_TARFILE_LOCATION=$(TARFILE_LOCATION)
endif

done :
	mkdir -p $(dir $@)
	# FIXME: do not hardcode the path here
	$(GNUTAR) -x --strip-component=1 -f $(FIXED_TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-internal-zlib.patch
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-warnings.patch
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-gcc-atomics.patch
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-debug.patch
	$(GNUPATCH) -p0 < $(SRCDIR)/clucene/patches/clucene-narrowing-conversions.patch
	#FIXME ?, our rules expect .cxx
	for i in `find . -name "*.cpp"`; do mv $$i $${i%%cpp}cxx; done
ifneq ($(OS),WNT)
	#dirent.h is a problem, move it around
	mkdir -p inc/internal/CLucene/util
	mv src/shared/CLucene/util/dirent.h inc/internal/CLucene/util
endif
	#To generate these, run cmake for each sufficiently different platform, customize and stick into configs
ifeq ($(OS),WNT)
	cp $(SRCDIR)/clucene/configs/_clucene-config-MSVC.h src/shared/CLucene/_clucene-config.h
ifeq ($(COM),MSC)
	cp $(SRCDIR)/clucene/configs/clucene-config-MSVC.h src/shared/CLucene/clucene-config.h
else
	cp $(SRCDIR)/clucene/configs/clucene-config-MINGW.h src/shared/CLucene/clucene-config.h
endif
else
	cp $(SRCDIR)/clucene/configs/clucene-config-generic.h src/shared/CLucene/clucene-config.h
ifeq ($(OS),LINUX)
	cp $(SRCDIR)/clucene/configs/_clucene-config-LINUX.h src/shared/CLucene/_clucene-config.h
else
	cp $(SRCDIR)/clucene/configs/_clucene-config-generic.h src/shared/CLucene/_clucene-config.h
endif
endif

.DEFAULT_GOAL := all
.PHONY : all
all : done

# vim: set noet sw=4 ts=4:
