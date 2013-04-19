# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpchelp1))

$(eval $(call gb_Library_set_componentfile,ucpchelp1,xmlhelp/util/ucpchelp1))

# TODO: is this still relevant?
# GCC versions 4.2.x introduced a warning "allocating zero-element array"
# Allocating zero-element arrays is an allowed if not somewhat dubious 
# technique though, so this warning is plain wrong and has been fixed 
# in gcc 4.3. Unfortunately there is no way at all to suppress this warning.
# Some files in this directory use zero allocated arrays, we need to
# diable the WaE mechanism for the GCC 4.2.x series.
# .IF "$(COM)"=="GCC"
# .IF "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
# CFLAGSWERRCXX:=
# .ENDIF # "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
# .ENDIF # "$(COM)"=="GCC"

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Library_add_cxxflags,ucpchelp1,\
	-GR \
))
endif

$(eval $(call gb_Library_set_include,ucpchelp1,\
	-I$(SRCDIR)/xmlhelp/source/cxxhelp/inc \
	$$(INCLUDE) \
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
	ucbhelper \
	utl \
	i18nlangtag \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,ucpchelp1,\
    xmlhelp/source/cxxhelp/provider/bufferedinputstream \
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
    xmlhelp/source/cxxhelp/provider/services \
    xmlhelp/source/cxxhelp/provider/urlparameter \
    xmlhelp/source/cxxhelp/qe/DocGenerator \
    xmlhelp/source/cxxhelp/util/Decompressor \
))

ifeq ($(HAVE_CXX0X),TRUE)
$(eval $(call gb_Library_add_defs,ucpchelp1,\
    -DHAVE_CXX0X \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
