# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,ldapbe2))

$(eval $(call gb_Library_set_componentfile,ldapbe2,extensions/source/config/ldap/ldapbe2,services))

$(eval $(call gb_Library_use_sdk_api,ldapbe2))

ifneq ($(OS),WNT)
$(eval $(call gb_Library_add_defs,ldapbe2,\
	-DLDAP_DEPRECATED \
))
endif

$(eval $(call gb_Library_add_exception_objects,ldapbe2,\
	extensions/source/config/ldap/ldapaccess \
	extensions/source/config/ldap/ldapuserprofilebe \
))

$(eval $(call gb_Library_use_libraries,ldapbe2,\
	comphelper \
	cppuhelper \
	cppu \
	salhelper \
	sal \
	tl \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_externals,ldapbe2,\
	boost_headers \
))
$(eval $(call gb_Library_use_system_win32_libs,ldapbe2,\
	wldap32 \
))
else # 0S!=WNT
$(eval $(call gb_Library_use_externals,ldapbe2,\
	boost_headers \
	openldap \
	nss3 \
	plc4 \
	ssl3 \
))
endif



ifeq ($(OS),FREEBSD)
$(eval $(call gb_Library_add_libs,ldapbe2,\
	-lcompat \
))
endif # OS=FREEBSD

# vim:set noet sw=4 ts=4:
