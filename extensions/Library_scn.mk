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

$(eval $(call gb_Library_Library,scn))

$(eval $(call gb_Library_use_externals,scn,\
	boost_headers \
	$(if $(filter-out WNT,$(OS)),sane_headers) \
))

$(eval $(call gb_Library_set_include,scn,\
    -I$(SRCDIR)/extensions/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,scn,extensions/source/scanner/scn,services))

$(eval $(call gb_Library_use_sdk_api,scn))

$(eval $(call gb_Library_use_libraries,scn,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	$(if $(filter WNT,$(OS)),salhelper) \
	svt \
	$(if $(filter WNT,$(OS)),tk) \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/scanner \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/scanwin \
))
else
ifneq ($(OS),WNT)

$(eval $(call gb_Library_add_exception_objects,scn,\
	extensions/source/scanner/grid \
	extensions/source/scanner/sane \
	extensions/source/scanner/sanedlg \
	extensions/source/scanner/scanunx \
))
ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,scn,\
	-ldl \
))
endif
endif
endif

# vim:set noet sw=4 ts=4:
