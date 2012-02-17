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

done :
	mkdir -p $(dir $@)
	# FIXME: do not hardcode the path here
	$(GNUTAR) -x --strip-component=1 -f $(TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
	#FIXME ?, our rules expect .cxx
	for i in `find . -name "*.cpp"`; do mv $$i $${i%%cpp}cxx; done
	#dirent.h is a problem, move it around
	mkdir -p inc/internal/CLucene/util
	mv src/shared/CLucene/util/dirent.h inc/internal/CLucene/util
	#To generate these, run cmake for each sufficiently different platform, customize and stick into configs
	cp $(SRCDIR)/clucene/configs/_clucene-config-generic.h src/shared/CLucene/_clucene-config.h
	cp $(SRCDIR)/clucene/configs/clucene-config-generic.h src/shared/CLucene/clucene-config.h

.DEFAULT_GOAL := all
.PHONY : all
all : done

# vim: set noet sw=4 ts=4:
