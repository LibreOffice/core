# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# [ Copyright (C) 2011 Red Hat, Inc., Michael Stahl <mstahl@redhat.com> (initial developer) ]
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
#

$(eval $(call gb_Library_Library,scn))

$(eval $(call gb_Library_set_componentfile,scn,extensions/source/scanner/scn))

$(eval $(call gb_Library_use_sdk_api,scn))

$(eval $(call gb_Library_use_libraries,scn,\
	svt \
	vcl \
	tl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/scnserv \
	extensions/source/scanner/scanner \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_use_packages,scn,twain_inc))

$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/scanwin \
))
else
ifeq ($(GUI),UNX)
ifneq (,$(filter SANE,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_packages,scn,sane_inc))
endif

$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/grid \
	extensions/source/scanner/sane \
	extensions/source/scanner/sanedlg \
	extensions/source/scanner/scanunx \
))
ifeq ($(OS),LINUX)
$(eval $(call gb_Library_use_libraries,scn,\
	dl \
))
endif
endif
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
