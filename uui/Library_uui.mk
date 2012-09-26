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

$(eval $(call gb_Library_Library,uui))

$(eval $(call gb_Library_set_componentfile,uui,uui/util/uui))

$(eval $(call gb_Library_use_sdk_api,uui))

$(eval $(call gb_Library_use_libraries,uui,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	svt \
	tk \
	tl \
	utl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,uui))

$(eval $(call gb_Library_add_exception_objects,uui,\
	uui/source/alreadyopen \
	uui/source/filechanged \
	uui/source/fltdlg \
	uui/source/iahndl \
	uui/source/iahndl-authentication \
	uui/source/iahndl-errorhandler \
	uui/source/iahndl-filter \
	uui/source/iahndl-ioexceptions \
	uui/source/iahndl-locking \
	uui/source/iahndl-ssl \
	uui/source/interactionhandler \
	uui/source/lockfailed \
	uui/source/logindlg \
	uui/source/masterpasscrtdlg \
	uui/source/masterpassworddlg \
	uui/source/nameclashdlg \
	uui/source/newerverwarn \
	uui/source/openlocked \
	uui/source/passwordcontainer \
	uui/source/passworddlg \
	uui/source/requeststringresolver \
	uui/source/secmacrowarnings \
	uui/source/services \
	uui/source/sslwarndlg \
	uui/source/trylater \
	uui/source/unknownauthdlg \
))

# vim: set noet sw=4 ts=4:
