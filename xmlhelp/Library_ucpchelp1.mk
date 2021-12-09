# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpchelp1))

$(eval $(call gb_Library_set_componentfile,ucpchelp1,xmlhelp/util/ucpchelp1,services))

$(eval $(call gb_Library_set_include,ucpchelp1,\
	-I$(SRCDIR)/xmlhelp/source/cxxhelp/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,ucpchelp1, \
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,ucpchelp1))

$(eval $(call gb_Library_use_externals,ucpchelp1,\
	boost_headers \
	clucene \
	expat \
	libxml2 \
	libxslt \
))

$(eval $(call gb_Library_use_libraries,ucpchelp1,\
	comphelper \
	cppu \
	cppuhelper \
	helplinker \
	sal \
	tl \
	ucbhelper \
	utl \
	i18nlangtag \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,ucpchelp1,\
    xmlhelp/source/cxxhelp/provider/content \
    xmlhelp/source/cxxhelp/provider/contentcaps \
    xmlhelp/source/cxxhelp/provider/databases \
    xmlhelp/source/cxxhelp/provider/db \
    xmlhelp/source/cxxhelp/provider/inputstream \
    xmlhelp/source/cxxhelp/provider/provider \
    xmlhelp/source/cxxhelp/provider/resultset \
    xmlhelp/source/cxxhelp/provider/resultsetbase \
    xmlhelp/source/cxxhelp/provider/resultsetforquery \
    xmlhelp/source/cxxhelp/provider/resultsetforroot \
    xmlhelp/source/cxxhelp/provider/urlparameter \
    xmlhelp/source/treeview/tvfactory \
    xmlhelp/source/treeview/tvread \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
