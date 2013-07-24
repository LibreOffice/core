# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,uui))

$(eval $(call gb_Library_set_componentfile,uui,uui/util/uui))

$(eval $(call gb_Library_use_external,uui,boost_headers))

$(eval $(call gb_Library_use_custom_headers,uui,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,uui))

$(eval $(call gb_Library_use_libraries,uui,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	i18nlangtag \
	svl \
	svt \
	tk \
	tl \
	utl \
	vcl \
	$(gb_UWINAPI) \
))

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
