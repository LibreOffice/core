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

include $(GBUILDDIR)/gbuild_simple.mk

DESTDIR := $(WORKDIR)/CustomTarget/desktop/win32/source/setup
ifneq ($(WITH_LANG),)
# TODO: how the localized version is made?
ULF := setup.ulf
else
ULF := setup.ulf
endif

$(DESTDIR)/loader2.rc : $(ULF) rcfooter.txt rcheader.txt rctmpl.txt
	lngconvex.exe -ulf $(ULF) -rc $@ -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

.DEFAULT_TARGET := all
.PHONY : all
all : $(DESTDIR)/loader2.rc

# vim: set ts=4 sw=4 noet:
