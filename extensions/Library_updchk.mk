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
# [ Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> ]
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

$(eval $(call gb_Library_Library,updchk))

$(eval $(call gb_Library_set_componentfile,updchk,extensions/source/update/check/updchk.uno))

$(eval $(call gb_Library_set_include,updchk,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_Library_use_sdk_api,updchk))

$(eval $(call gb_Library_use_libraries,updchk,\
	cppuhelper \
	cppu \
	sal \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,updchk,\
	ole32 \
	shell32 \
	wininet \
))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/onlinecheck \
))
endif # OS WNT

$(eval $(call gb_Library_use_external,updchk,curl))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/download \
	extensions/source/update/check/updatecheck \
	extensions/source/update/check/updatecheckconfig \
	extensions/source/update/check/updatecheckjob \
	extensions/source/update/check/updatehdl \
	extensions/source/update/check/updateprotocol \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
