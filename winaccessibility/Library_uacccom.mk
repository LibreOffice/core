# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,UAccCOM))

$(eval $(call gb_Library_use_custom_headers,UAccCOM,winaccessibility/ia2/idl))

$(eval $(call gb_Library_set_include,UAccCOM,\
	$$(INCLUDE) \
	-I$(SRCDIR)/winaccessibility/inc \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_Library_add_defs,UAccCOM,\
	-D_UNICODE=1 \
))

$(eval $(call gb_Library_use_sdk_api,UAccCOM))

$(eval $(call gb_Library_add_nativeres,UAccCOM,UAccCOM))

$(eval $(call gb_Library_add_exception_objects,UAccCOM,\
	winaccessibility/source/UAccCOM/AccAction \
	winaccessibility/source/UAccCOM/AccActionBase \
	winaccessibility/source/UAccCOM/AccComponent \
	winaccessibility/source/UAccCOM/AccComponentBase \
	winaccessibility/source/UAccCOM/AccEditableText \
	winaccessibility/source/UAccCOM/AccHyperLink \
	winaccessibility/source/UAccCOM/AccHypertext \
	winaccessibility/source/UAccCOM/AccImage \
	winaccessibility/source/UAccCOM/AccRelation \
	winaccessibility/source/UAccCOM/AccTable \
	winaccessibility/source/UAccCOM/AccText \
	winaccessibility/source/UAccCOM/AccTextBase \
	winaccessibility/source/UAccCOM/AccValue \
	winaccessibility/source/UAccCOM/EnumVariant \
	winaccessibility/source/UAccCOM/MAccessible \
	winaccessibility/source/UAccCOM/StdAfx \
	winaccessibility/source/UAccCOM/UAccCOM \
	winaccessibility/source/UAccCOM/UNOXWrapper \
))

$(eval $(call gb_Library_add_ldflags,UAccCOM,\
	/DEF:$(SRCDIR)/winaccessibility/source/UAccCOM/UAccCOM.def \
))

$(eval $(call gb_Library_use_libraries,UAccCOM,\
	cppu \
	sal \
))

$(eval $(call gb_Library_use_externals,UAccCOM,\
	boost_headers \
	icuuc \
	icui18n \
))

$(eval $(call gb_Library_use_system_win32_libs,UAccCOM,\
	oleacc \
	advapi32 \
	delayimp \
	kernel32 \
	ole32 \
	oleaut32 \
	shlwapi \
	user32 \
	uuid \
))

$(eval $(call gb_Library_add_libs,UAccCOM,\
	$(ATL_LIB)/$(if $(MSVC_USE_DEBUG_RUNTIME),atlsd.lib,atls.lib) \
))

# vim:set noet sw=4 ts=4:
