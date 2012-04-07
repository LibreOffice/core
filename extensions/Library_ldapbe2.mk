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
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
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
#

$(eval $(call gb_Library_Library,ldapbe2))

$(eval $(call gb_Library_set_componentfile,ldapbe2,extensions/source/config/ldap/ldapbe2))

$(eval $(call gb_Library_use_api,ldapbe2,\
	offapi \
	udkapi \
))

ifeq ($(LDAPSDKINCLUDES),)
ifneq ($(WITH_OPENLDAP),YES)
ifeq ($(SYSTEM_MOZILLA),YES)
$(eval $(call gb_Library_set_include,ldapbe2,\
	$$(INCLUDE) \
	$(MOZ_LDAP_CFLAGS) \
))
else # SYSTEM_MOZILLA=NO
$(eval $(call gb_Library_set_include,ldapbe2,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/mozilla \
))
endif # SYSTEM_MOZILLA=YES
else # WITH_OPENLDAP=YES
$(eval $(call gb_Library_add_defs,ldapbe2,\
	-DWITH_OPENLDAP \
	-DLDAP_DEPRECATED \
))
endif # WITH_OPENLDAP=NO
endif # LDAPSDKINCLUDES=

$(eval $(call gb_Library_add_exception_objects,ldapbe2,\
	extensions/source/config/ldap/componentdef \
	extensions/source/config/ldap/ldapaccess \
	extensions/source/config/ldap/ldapuserprofilebe \
))

$(eval $(call gb_Library_use_libraries,ldapbe2,\
	cppuhelper \
	cppu \
	salhelper \
	sal \
))

ifeq ($(OS),FREEBSD)
$(eval $(call gb_Library_add_libs,ldapbe2,\
	-lcompat \
))
ifneq ($(WITH_OPENLDAP),YES)
$(eval $(call gb_Library_add_libs,ldapbe2,\
	-Wl,-Bstatic
	-llber50 \
))
endif # WITH_OPENLDAP=NO
endif # OS=FREEBSD

# vim:set shiftwidth=4 softtabstop=4 expandtab:
