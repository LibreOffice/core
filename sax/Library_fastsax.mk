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

$(eval $(call gb_Library_Library,fastsax))

$(eval $(call gb_Library_set_componentfile,fastsax,sax/source/fastparser/fastsax))

$(eval $(call gb_Library_set_include,fastsax,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(SRCDIR)/sax/inc \
))

$(eval $(call gb_Library_set_defs,fastsax,\
	$$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,fastsax,\
	sal \
	cppu \
	cppuhelper \
	sax \
	stl \
    $(gb_STDLIBS) \
))

ifeq ($(SYSTEM_ZLIB),YES)
$(eval $(call gb_Library_set_defs,fastsax,\
	$$(DEFS) \
	-DSYSTEM_ZLIB \
))
endif

ifeq ($(SYSTEM_EXPAT),YES)
$(eval $(call gb_Library_set_defs,fastsax,\
	$$(DEFS) \
	-DSYSTEM_EXPAT \
))
$(eval $(call gb_Library_add_linked_libs,fastsax,\
	expat \
))
else
$(eval $(call gb_Library_add_linked_static_libs,fastsax,\
	expat_xmlparse \
	expat_xmltok \
))
endif

# re-uses xml2utf object from sax.uno lib (see below)
$(eval $(call gb_Library_add_exception_objects,fastsax,\
	sax/source/fastparser/facreg \
	sax/source/fastparser/fastparser \
	sax/source/expatwrap/xml2utf \
))

# mba: currently it's unclear whether xml2utf needs to be compiled with the
# same compiler settings as in the sax.uno lib; in case not, we have to use 
# a custom target
#$(eval $(call gb_Library_add_generated_exception_objects,fastsax,\
#	sax/source/expatwrap/xml2utf \
#))

# vim: set noet sw=4 ts=4:
