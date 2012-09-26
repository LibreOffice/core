# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,avmediagst_0_10))

$(eval $(call gb_Library_set_componentfile,avmediagst_0_10,avmedia/source/gstreamer/avmediagstreamer_0_10))

$(eval $(call gb_Library_set_include,avmediagst_0_10,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
	$(GSTREAMER_0_10_CFLAGS) \
))

$(eval $(call gb_Library_use_sdk_api,avmediagst_0_10))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_defs,avmediagst_0_10,\
	-DWINNT
))
endif

$(eval $(call gb_Library_use_libraries,avmediagst_0_10,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,avmediagst_0_10,\
	$(GSTREAMER_0_10_LIBS) \
	-lgstinterfaces-0.10 \
))

$(eval $(call gb_Library_add_exception_objects,avmediagst_0_10,\
	avmedia/source/gstreamer/gst_0_10 \
))

# vim: set noet sw=4 ts=4:
