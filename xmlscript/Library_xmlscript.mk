# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xmlscript))

$(eval $(call gb_Library_use_packages,xmlscript,\
	tools_inc \
	xmlscript_inc \
))

$(eval $(call gb_Library_set_componentfile,xmlscript,xmlscript/util/xmlscript))

$(eval $(call gb_Library_use_sdk_api,xmlscript))

$(eval $(call gb_Library_use_libraries,xmlscript,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_include,xmlscript,\
    -I$(SRCDIR)/xmlscript/source/inc \
    $$(INCLUDE) \
))


$(eval $(call gb_Library_add_defs,xmlscript,\
    -DXMLSCRIPT_DLLIMPLEMENTATION \
))


$(eval $(call gb_Library_add_exception_objects,xmlscript,\
    xmlscript/source/misc/unoservices \
    xmlscript/source/xml_helper/xml_byteseq \
    xmlscript/source/xml_helper/xml_element \
    xmlscript/source/xml_helper/xml_impctx \
    xmlscript/source/xmldlg_imexp/xmldlg_addfunc \
    xmlscript/source/xmldlg_imexp/xmldlg_expmodels \
    xmlscript/source/xmldlg_imexp/xmldlg_export \
    xmlscript/source/xmldlg_imexp/xmldlg_impmodels \
    xmlscript/source/xmldlg_imexp/xmldlg_import \
    xmlscript/source/xmlflat_imexp/xmlbas_export \
    xmlscript/source/xmlflat_imexp/xmlbas_import \
    xmlscript/source/xmllib_imexp/xmllib_export \
    xmlscript/source/xmllib_imexp/xmllib_import \
    xmlscript/source/xmlmod_imexp/xmlmod_export \
    xmlscript/source/xmlmod_imexp/xmlmod_import \
))

# vim: set noet sw=4 ts=4:
