# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,xmlscript_inc,$(SRCDIR)/xmlscript))

$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xmlscriptdllapi.h,inc/xmlscript/xmlscriptdllapi.h))
$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xml_helper.hxx,inc/xmlscript/xml_helper.hxx))
$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xmldlg_imexp.hxx,inc/xmlscript/xmldlg_imexp.hxx))
$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xmllib_imexp.hxx,inc/xmlscript/xmllib_imexp.hxx))
$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xmlmod_imexp.hxx,inc/xmlscript/xmlmod_imexp.hxx))
$(eval $(call gb_Package_add_file,xmlscript_inc,inc/xmlscript/xmlns.h,inc/xmlscript/xmlns.h))

# vim: set noet sw=4 ts=4:
