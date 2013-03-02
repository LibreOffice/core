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
# [ Copyright (C) 2012 Red Hat, Inc., Michael Stahl <mstahl@redhat.com> (initial developer) ]
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

ifeq ($(OS),WNT)
$(error only for UNX)
endif

$(eval $(call gb_StaticLibrary_StaticLibrary,plugcon))

$(eval $(call gb_StaticLibrary_use_packages,plugcon,\
    basegfx_inc \
    comphelper_inc \
    cppu_odk_headers \
    i18npool_inc \
    np_sdk_inc \
	o3tl_inc \
	rsc_inc \
    tools_inc \
    unotools_inc \
    vcl_inc \
))

$(eval $(call gb_StaticLibrary_use_externals,plugcon,\
    boost_headers \
    npapi_headers \
))

$(eval $(call gb_StaticLibrary_set_include,plugcon,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/plugin/inc \
))

$(eval $(call gb_StaticLibrary_use_api,plugcon,\
	offapi \
	udkapi \
))

ifeq ($(SOLAR_JAVA),TRUE)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DOJI \
))

endif # SOLAR_JAVA=YES

ifeq ($(SYSTEM_MOZILLA),YES)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DSYSTEM_MOZILLA \
))
endif # SYSTEM_MOZILLA=YES

ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_StaticLibrary_add_defs,plugcon,\
	-DENABLE_GTK \
))

$(eval $(call gb_StaticLibrary_use_external,plugcon,gtk))
endif # ENABLE_GTK=TRUE

$(eval $(call gb_StaticLibrary_add_exception_objects,plugcon,\
	extensions/source/plugin/unx/mediator \
	extensions/source/plugin/unx/plugcon \
))

# vim:set noet sw=4 ts=4:

