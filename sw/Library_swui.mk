# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,swui))

$(eval $(call gb_Library_set_include,swui,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(WORKDIR)/SdiTarget/sw/sdi \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,swui,$(SRCDIR)/sw/inc/pch/precompiled_swui))

$(eval $(call gb_Library_use_externals,swui,\
	boost_headers \
	icu_headers \
))

$(eval $(call gb_Library_use_custom_headers,swui,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,swui))

ifneq ($(SYSTEM_LIBXML),)
$(eval $(call gb_Library_add_cxxflags,swui,\
	$(LIBXML_CFLAGS) \
))
endif

$(eval $(call gb_Library_use_libraries,swui,\
    comphelper \
    cppu \
    cppuhelper \
    $(call gb_Helper_optional,DBCONNECTIVITY, \
        dbtools) \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    sal \
    salhelper \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svx \
    svxcore \
    sw \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,swui,\
    sw/source/ui/chrdlg/break \
    sw/source/ui/chrdlg/chardlg \
    sw/source/ui/chrdlg/drpcps \
    sw/source/ui/chrdlg/numpara \
    sw/source/ui/chrdlg/pardlg \
    sw/source/ui/chrdlg/swuiccoll \
    sw/source/ui/chrdlg/tblnumfm \
    sw/source/ui/config/mailconfigpage \
    sw/source/ui/config/optcomp \
    sw/source/ui/config/optload \
    sw/source/ui/config/optpage \
    sw/source/ui/dialog/abstract \
    sw/source/ui/dialog/addrdlg \
    sw/source/ui/dialog/ascfldlg \
    sw/source/ui/dialog/docstdlg \
    sw/source/ui/dialog/macassgn \
    sw/source/ui/dialog/swdialmgr \
    sw/source/ui/dialog/swdlgfact \
    sw/source/ui/dialog/swmessdialog \
    sw/source/ui/dialog/swuiexp \
    sw/source/ui/dialog/uiregionsw \
    sw/source/ui/dialog/wordcountdialog \
    sw/source/ui/dochdl/selglos \
    sw/source/ui/envelp/envfmt \
    sw/source/ui/envelp/envlop1 \
    sw/source/ui/envelp/envprt \
    sw/source/ui/envelp/label1 \
    sw/source/ui/envelp/labelexp \
    sw/source/ui/envelp/labfmt \
    sw/source/ui/envelp/labprt \
    sw/source/ui/envelp/mailmrge \
    sw/source/ui/fldui/DropDownFieldDialog \
    sw/source/ui/fldui/FldRefTreeListBox \
    sw/source/ui/fldui/changedb \
    sw/source/ui/fldui/flddb \
    sw/source/ui/fldui/flddinf \
    sw/source/ui/fldui/flddok \
    sw/source/ui/fldui/fldedt \
    sw/source/ui/fldui/fldfunc \
    sw/source/ui/fldui/fldpage \
    sw/source/ui/fldui/fldref \
    sw/source/ui/fldui/fldtdlg \
    sw/source/ui/fldui/fldvar \
    sw/source/ui/fldui/inpdlg \
    sw/source/ui/fldui/javaedit \
    sw/source/ui/fmtui/tmpdlg \
    sw/source/ui/frmdlg/column \
    sw/source/ui/frmdlg/cption \
    sw/source/ui/frmdlg/frmdlg \
    sw/source/ui/frmdlg/frmpage \
    sw/source/ui/frmdlg/pattern \
    sw/source/ui/frmdlg/uiborder \
    sw/source/ui/frmdlg/wrap \
    sw/source/ui/index/cntex \
    sw/source/ui/index/cnttab \
    sw/source/ui/index/multmrk \
    sw/source/ui/index/swuiidxmrk \
    sw/source/ui/misc/bookmark \
    sw/source/ui/misc/docfnote \
    sw/source/ui/misc/glosbib \
    sw/source/ui/misc/glossary \
    sw/source/ui/misc/insfnote \
    sw/source/ui/misc/linenum \
    sw/source/ui/misc/num \
    sw/source/ui/misc/outline \
    sw/source/ui/misc/pgfnote \
    sw/source/ui/misc/pggrid \
    sw/source/ui/misc/srtdlg \
    sw/source/ui/misc/swmodalredlineacceptdlg \
    sw/source/ui/misc/titlepage \
    sw/source/ui/table/colwd \
    sw/source/ui/table/convert \
    sw/source/ui/table/instable \
    sw/source/ui/table/mergetbl \
    sw/source/ui/table/rowht \
    sw/source/ui/table/splittbl \
    sw/source/ui/table/tabledlg \
    sw/source/ui/table/tautofmt \
    sw/source/ui/utlui/swrenamexnameddlg \
))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
$(eval $(call gb_Library_add_exception_objects,swui,\
    sw/source/ui/dbui/addresslistdialog \
    sw/source/ui/dbui/createaddresslistdialog \
    sw/source/ui/dbui/customizeaddresslistdialog \
    sw/source/ui/dbui/dbinsdlg \
    sw/source/ui/dbui/dbtablepreviewdialog \
    sw/source/ui/dbui/mailmergewizard \
    sw/source/ui/dbui/mmaddressblockpage \
    sw/source/ui/dbui/mmdocselectpage \
    sw/source/ui/dbui/mmgreetingspage \
    sw/source/ui/dbui/mmlayoutpage \
    sw/source/ui/dbui/mmoutputtypepage \
    sw/source/ui/dbui/mmresultdialogs \
    sw/source/ui/dbui/selectdbtabledialog \
))
endif

# vim: set noet sw=4 ts=4:
