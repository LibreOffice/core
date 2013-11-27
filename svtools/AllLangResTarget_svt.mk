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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,svt))

$(eval $(call gb_AllLangResTarget_set_reslocation,svt,svtools))

$(eval $(call gb_AllLangResTarget_add_srs,svt,\
    svt/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svt/res))

$(eval $(call gb_SrsTarget_set_include,svt/res,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc \
    -I$(SRCDIR)/svtools/source/uno \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc/ \
    -I$(SRCDIR)/svtools/inc/svtools \
))

$(eval $(call gb_SrsTarget_add_files,svt/res,\
    svtools/source/brwbox/editbrowsebox.src \
    svtools/source/contnr/fileview.src \
    svtools/source/contnr/svcontnr.src \
    svtools/source/contnr/templwin.src \
    svtools/source/control/calendar.src \
    svtools/source/control/ctrlbox.src \
    svtools/source/control/ctrltool.src \
    svtools/source/control/filectrl.src \
    svtools/source/control/ruler.src \
    svtools/source/dialogs/addresstemplate.src \
    svtools/source/dialogs/filedlg2.src \
    svtools/source/dialogs/formats.src \
    svtools/source/dialogs/prnsetup.src \
    svtools/source/dialogs/so3res.src \
    svtools/source/dialogs/wizardmachine.src \
    svtools/source/filter/exportdialog.src \
    svtools/source/java/javaerror.src \
    svtools/source/misc/ehdl.src \
    svtools/source/misc/helpagent.src \
    svtools/source/misc/imagemgr.src \
    svtools/source/misc/langtab.src \
    svtools/source/misc/undo.src \
    svtools/source/plugapp/testtool.src \
    svtools/source/toolpanel/toolpanel.src \
))


# vim: set noet sw=4 ts=4:
