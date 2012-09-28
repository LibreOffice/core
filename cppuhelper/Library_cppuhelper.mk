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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Library_Library,cppuhelper))

$(eval $(call gb_Library_set_soversion_script,cppuhelper,3,$(SRCDIR)/cppuhelper/source/gcc3.map))

$(eval $(call gb_Library_use_packages,cppuhelper,\
	cppuhelper_inc \
))

$(eval $(call gb_Library_use_internal_comprehensive_api,cppuhelper,\
	cppuhelper \
	udkapi \
))

$(eval $(call gb_Library_add_defs,cppuhelper,\
	-DCPPUHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,cppuhelper,\
	cppu \
	reg \
	sal \
	salhelper \
	xmlreader \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_cobjects,cppuhelper,\
	cppuhelper/source/findsofficepath \
))

$(eval $(call gb_Library_add_exception_objects,cppuhelper,\
	cppuhelper/source/access_control \
	cppuhelper/source/bootstrap \
	cppuhelper/source/component_context \
	cppuhelper/source/component \
	cppuhelper/source/defaultbootstrap \
	cppuhelper/source/exc_thrower \
	cppuhelper/source/factory \
	cppuhelper/source/implbase \
	cppuhelper/source/implbase_ex \
	cppuhelper/source/implementationentry \
	cppuhelper/source/interfacecontainer \
	cppuhelper/source/macro_expander \
	cppuhelper/source/paths \
	cppuhelper/source/propertysetmixin \
	cppuhelper/source/propshlp \
	cppuhelper/source/servicefactory \
	cppuhelper/source/shlib \
	cppuhelper/source/stdidlclass \
	cppuhelper/source/supportsservice \
	cppuhelper/source/tdmgr \
	cppuhelper/source/typeprovider \
	cppuhelper/source/unourl \
	cppuhelper/source/weak \
))

# vim: set noet sw=4 ts=4:
