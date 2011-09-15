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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

SRCDIR := $(realpath $(dir $(firstword $(MAKEFILE_LIST)))/../..)

ifndef ABOUT_BITMAP
ABOUT_BITMAP := $(SRCDIR)/default_images/brand/about.png
endif

ifndef INTRO_BITMAP
INTRO_BITMAP := $(SRCDIR)/default_images/brand/intro.png
endif

$(WORKDIR)/CustomTarget/desktop/branding/about.png : $(ABOUT_BITMAP)
	mkdir -p $(dir $@) && cp -p $< $@

$(WORKDIR)/CustomTarget/desktop/branding/intro.png : $(INTRO_BITMAP)
	mkdir -p $(dir $@) && cp -p $< $@

.DEFAULT_GOAL := all
.PHONY : all
all : \
	$(WORKDIR)/CustomTarget/desktop/branding/about.png \
	$(WORKDIR)/CustomTarget/desktop/branding/intro.png

# vim: set ts=4 sw=4 et:
