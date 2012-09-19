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
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

ifeq ($(GUI)$(COM),WNTMSC)
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
	berkeleydb \
	clucene \
	expat_utf8 \
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
