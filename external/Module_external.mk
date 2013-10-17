# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,external))

ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_Module_add_moduledir,external,mingw-externals))
endif

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_moduledir,external,msc-externals))
endif

$(eval $(call gb_Module_add_moduledirs,external,\
	$(call gb_Helper_optional,AFMS,afms) \
	$(call gb_Helper_optional,APACHE_COMMONS,apache-commons) \
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,BSH,beanshell) \
	$(call gb_Helper_optional,CAIRO,cairo) \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,CPPUNIT,cppunit) \
	$(call gb_Helper_optional,CT2N,ct2n) \
	$(call gb_Helper_optional,CURL,curl) \
	$(call gb_Helper_optional,EPM,epm) \
	$(call gb_Helper_optional,EXPAT,expat) \
	$(call gb_Helper_optional,FIREBIRD,firebird) \
	$(call gb_Helper_optional,FONTCONFIG,fontconfig) \
	$(call gb_Helper_optional,FREETYPE,freetype) \
	$(call gb_Helper_optional,GRAPHITE,graphite) \
	$(call gb_Helper_optional,HARFBUZZ,harfbuzz) \
	$(call gb_Helper_optional,LIBATOMIC_OPS,libatomic_ops) \
	$(call gb_Helper_optional,LIBPNG,libpng) \
))

# vim: set noet sw=4 ts=4:
