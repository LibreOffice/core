#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,swui))

$(eval $(call gb_Library_set_include,swui,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/inc/pch \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(WORKDIR)/SdiTarget/sw/sdi \
    -I$(WORKDIR)/Misc/sw \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,swui,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,swui,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    msfilter \
    sal \
    sfx \
    sot \
    stl \
    svl \
    svt \
    svx \
    svx \
    svxcore \
    sw \
    swd \
    ootk \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,swui,\
    sw/source/core/undo/SwRewriter \
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
    sw/source/ui/dbui/mmmergepage \
    sw/source/ui/dbui/mmoutputpage \
    sw/source/ui/dbui/mmoutputtypepage \
    sw/source/ui/dbui/mmpreparemergepage \
    sw/source/ui/dbui/selectdbtabledialog \
    sw/source/ui/dialog/abstract \
    sw/source/ui/dialog/addrdlg \
    sw/source/ui/dialog/ascfldlg \
    sw/source/ui/dialog/docstdlg \
    sw/source/ui/dialog/macassgn \
    sw/source/ui/dialog/swdialmgr \
    sw/source/ui/dialog/swdlgfact \
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
    sw/source/ui/misc/insrule \
    sw/source/ui/misc/linenum \
    sw/source/ui/misc/num \
    sw/source/ui/misc/outline \
    sw/source/ui/misc/pgfnote \
    sw/source/ui/misc/pggrid \
    sw/source/ui/misc/srtdlg \
    sw/source/ui/misc/swmodalredlineacceptdlg \
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

# vim: set noet sw=4 ts=4:
