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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,sfx))

$(eval $(call gb_AllLangResTarget_set_reslocation,sfx,sfx2))

$(eval $(call gb_AllLangResTarget_add_srs,sfx,\
    sfx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sfx/res))

$(eval $(call gb_SrsTarget_set_include,sfx/res,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
    -I$(WORKDIR)/inc \
    -I$(SRCDIR)/sfx2/source/dialog \
    -I$(SRCDIR)/sfx2/source/inc \
    -I$(SRCDIR)/sfx2/inc/ \
    -I$(SRCDIR)/sfx2/inc/sfx \
))

$(eval $(call gb_SrsTarget_add_files,sfx/res,\
    sfx2/source/appl/app.src \
    sfx2/source/appl/dde.src \
    sfx2/source/appl/newhelp.src \
    sfx2/source/appl/sfx.src \
    sfx2/source/bastyp/bastyp.src \
    sfx2/source/bastyp/fltfnc.src \
    sfx2/source/dialog/alienwarn.src \
    sfx2/source/dialog/dialog.src \
    sfx2/source/dialog/dinfdlg.src \
    sfx2/source/dialog/dinfedt.src \
    sfx2/source/dialog/filedlghelper.src \
    sfx2/source/dialog/mailwindow.src \
    sfx2/source/dialog/mgetempl.src \
    sfx2/source/dialog/newstyle.src \
    sfx2/source/dialog/passwd.src \
    sfx2/source/dialog/printopt.src \
    sfx2/source/dialog/recfloat.src \
    sfx2/source/dialog/securitypage.src \
    sfx2/source/dialog/srchdlg.src \
    sfx2/source/dialog/taskpane.src \
    sfx2/source/dialog/templdlg.src \
    sfx2/source/dialog/titledockwin.src \
    sfx2/source/dialog/versdlg.src \
    sfx2/source/doc/doc.src \
    sfx2/source/doc/doctdlg.src \
    sfx2/source/doc/doctempl.src \
    sfx2/source/doc/docvor.src \
    sfx2/source/doc/graphhelp.src \
    sfx2/source/doc/new.src \
    sfx2/source/menu/menu.src \
    sfx2/source/sidebar/Sidebar.src \
    sfx2/source/view/view.src \
))


