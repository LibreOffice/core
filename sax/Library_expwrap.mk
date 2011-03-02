#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,expwrap))

$(eval $(call gb_Library_set_componentfile,expwrap,sax/source/expatwrap/expwrap))

$(eval $(call gb_Library_set_include,expwrap,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(SRCDIR)/sax/inc \
))

$(eval $(call gb_Library_set_defs,expwrap,\
	$$(DEFS) \
))

ifeq ($(SYSTEM_ZLIB),YES)
$(eval $(call gb_Library_set_defs,expwrap,\
	$$(DEFS) \
	-DSYSTEM_ZLIB \
))
endif

$(eval $(call gb_Library_add_linked_libs,expwrap,\
	sal \
	cppu \
	cppuhelper \
	stl \
    $(gb_STDLIBS) \
))

ifeq ($(SYSTEM_EXPAT),YES)
$(eval $(call gb_Library_set_defs,expwrap,\
	$$(DEFS) \
	-DSYSTEM_EXPAT \
))
$(eval $(call gb_Library_add_linked_libs,expwrap,\
	expat \
))
else
$(eval $(call gb_Library_set_defs,expwrap,\
	$$(DEFS) \
	-DXML_UNICODE \
))
$(eval $(call gb_Library_add_linked_libs,expwrap,\
	expat_xmlparse \
	expat_xmltok \
))
endif

$(eval $(call gb_Library_add_exception_objects,expwrap,\
	sax/source/expatwrap/attrlistimpl \
	sax/source/expatwrap/sax_expat \
	sax/source/expatwrap/saxwriter \
	sax/source/expatwrap/xml2utf \
))

# vim: set noet sw=4 ts=4:
