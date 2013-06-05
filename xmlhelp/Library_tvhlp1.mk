# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,tvhlp1))

$(eval $(call gb_Library_set_componentfile,tvhlp1,xmlhelp/source/treeview/tvhlp1))

$(eval $(call gb_Library_use_sdk_api,tvhlp1))

$(eval $(call gb_Library_use_externals,tvhlp1,\
	boost_headers \
	expat \
))

$(eval $(call gb_Library_use_libraries,tvhlp1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	utl \
	i18nlangtag \
))

$(eval $(call gb_Library_add_exception_objects,tvhlp1,\
    xmlhelp/source/treeview/tvfactory \
    xmlhelp/source/treeview/tvread \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
