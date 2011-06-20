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

$(eval $(call gb_Library_Library,xcr))

$(eval $(call gb_Library_add_package_headers,xcr,xmlscript_inc))

$(eval $(call gb_Library_set_componentfile,xcr,xmlscript/util/xcr))

$(eval $(call gb_Library_set_include,xcr,\
	-I$(SRCDIR)/xmlscript/source/inc \
	-I$(SRCDIR)/xmlscript/inc/pch \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_defs,xcr,\
	-DXCR_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,xcr,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,xcr,\
	xmlscript/source/xmldlg_imexp/xmldlg_impmodels \
	xmlscript/source/xmldlg_imexp/xmldlg_import \
	xmlscript/source/xmldlg_imexp/xmldlg_addfunc \
	xmlscript/source/xmldlg_imexp/xmldlg_export \
	xmlscript/source/xmldlg_imexp/xmldlg_expmodels \
	xmlscript/source/misc/unoservices \
	xmlscript/source/xml_helper/xml_byteseq \
	xmlscript/source/xml_helper/xml_impctx \
	xmlscript/source/xml_helper/xml_element \
	xmlscript/source/xmlflat_imexp/xmlbas_import \
	xmlscript/source/xmlflat_imexp/xmlbas_export \
	xmlscript/source/xmllib_imexp/xmllib_import \
	xmlscript/source/xmllib_imexp/xmllib_export \
	xmlscript/source/xmlmod_imexp/xmlmod_import \
	xmlscript/source/xmlmod_imexp/xmlmod_export \
))

# vim: set noet sw=4 ts=4:
