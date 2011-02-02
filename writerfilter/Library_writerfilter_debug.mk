#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,writerfilter_debug))

$(eval $(call gb_Library_set_include,writerfilter_debug,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(SRCDIR)/writerfilter/inc \
))

$(eval $(call gb_Library_set_defs,writerfilter_debug,\
	$$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,writerfilter_debug,\
	writerfilter \
	comphelper \
	ucbhelper \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,writerfilter_debug,\
	writerfilter/unocomponent/debugservices/doctok/DocTokTestService \
	writerfilter/unocomponent/debugservices/doctok/DocTokAnalyzeService \
	writerfilter/unocomponent/debugservices/ooxml/OOXMLTestService \
	writerfilter/unocomponent/debugservices/ooxml/OOXMLAnalyzeService \
	writerfilter/unocomponent/component \
))

# disabled
#	writerfilter/unocomponent/debugservices/rtftok/ScannerTestService \
#	writerfilter/unocomponent/debugservices/rtftok/XMLScanner \

# vim: set noet sw=4 ts=4:

