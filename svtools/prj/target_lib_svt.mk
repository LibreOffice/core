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

$(eval $(call gb_Library_Library,svt))

$(call gb_Library_get_headers_target,svt) : $(call gb_Package_get_target,svtools_inc)

$(eval $(call gb_Library_set_include,svt,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc/svtools \
    -I$(WORKDIR)/inc/ \
    -I$(OUTDIR)/inc/ \
    -I$(SRCDIR)/svtools/inc \
    -I$(SRCDIR)/svtools/inc/svtools \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,svt,\
    $$(DEFS) \
    -DSVT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,svt,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    i18nutil \
    icuuc \
    jvmfwk \
    sal \
    sot \
    stl \
    svl \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    vos3 \
))

ifeq ($(SYSTEM_JPEG),YES)
$(eval $(call gb_Library_add_linked_libs,svt,\
    jpeg \
))
else
$(eval $(call gb_Library_add_linked_static_libs,svt,\
    jpeglib \
))
endif

$(eval $(call gb_Library_add_exception_objects,svt,\
    svtools/source/edit/svmedit \
    svtools/source/edit/svmedit2 \
    svtools/source/edit/textundo \
    svtools/source/edit/syntaxhighlight \
    svtools/source/edit/textwindowpeer \
    svtools/source/edit/sychconv \
    svtools/source/edit/xtextedt \
    svtools/source/edit/txtattr \
    svtools/source/edit/textdoc \
    svtools/source/edit/texteng \
    svtools/source/edit/textdata \
    svtools/source/edit/editsyntaxhighlighter \
    svtools/source/edit/textview \
    svtools/source/urlobj/inetimg \
    svtools/source/java/javainteractionhandler \
    svtools/source/java/javacontext \
    svtools/source/svhtml/htmlout \
    svtools/source/svhtml/htmlsupp \
    svtools/source/svhtml/parhtml \
    svtools/source/svhtml/htmlkywd \
    svtools/source/config/itemholder2 \
    svtools/source/config/optionsdrawinglayer \
    svtools/source/config/menuoptions \
    svtools/source/config/helpopt \
    svtools/source/config/accessibilityoptions \
    svtools/source/config/printoptions \
    svtools/source/config/miscopt \
    svtools/source/config/extcolorcfg \
    svtools/source/config/colorcfg \
    svtools/source/config/fontsubstconfig \
    svtools/source/config/apearcfg \
    svtools/source/misc/svtaccessiblefactory \
    svtools/source/misc/templatefoldercache \
    svtools/source/misc/imagemgr \
    svtools/source/misc/imap3 \
    svtools/source/misc/acceleratorexecute \
    svtools/source/misc/helpagentwindow \
    svtools/source/misc/imap \
    svtools/source/misc/wallitem \
    svtools/source/misc/langtab \
    svtools/source/misc/dialogclosedlistener \
    svtools/source/misc/itemdel \
    svtools/source/misc/dialogcontrolling \
    svtools/source/misc/svtdata \
    svtools/source/misc/transfer2 \
    svtools/source/misc/chartprettypainter \
    svtools/source/misc/imageresourceaccess \
    svtools/source/misc/imap2 \
    svtools/source/misc/ehdl \
    svtools/source/misc/embedtransfer \
    svtools/source/misc/embedhlp \
    svtools/source/misc/cliplistener \
    svtools/source/misc/transfer \
    svtools/source/misc/stringtransfer \
    svtools/source/plugapp/ttprops \
    svtools/source/control/ctrlbox \
    svtools/source/control/valueacc \
    svtools/source/control/valueset \
    svtools/source/control/taskbar \
    svtools/source/control/urlcontrol \
    svtools/source/control/scriptedtext \
    svtools/source/control/headbar \
    svtools/source/control/fileurlbox \
    svtools/source/control/fmtfield \
    svtools/source/control/collatorres \
    svtools/source/control/indexentryres \
    svtools/source/control/filectrl \
    svtools/source/control/scrwin \
    svtools/source/control/taskbox \
    svtools/source/control/hyperlabel \
    svtools/source/control/filectrl2 \
    svtools/source/control/fixedhyper \
    svtools/source/control/taskstat \
    svtools/source/control/inettbc \
    svtools/source/control/calendar \
    svtools/source/control/roadmap \
    svtools/source/control/ctrltool \
    svtools/source/control/asynclink \
    svtools/source/control/stdctrl \
    svtools/source/control/ctrldll \
    svtools/source/control/tabbar \
    svtools/source/control/ruler \
    svtools/source/control/stdmenu \
    svtools/source/control/prgsbar \
    svtools/source/control/taskmisc \
    svtools/source/dialogs/insdlg \
    svtools/source/dialogs/roadmapwizard \
    svtools/source/dialogs/mcvmath \
    svtools/source/dialogs/logindlg \
    svtools/source/dialogs/wizdlg \
    svtools/source/dialogs/addresstemplate \
    svtools/source/dialogs/printdlg \
    svtools/source/dialogs/filedlg2 \
    svtools/source/dialogs/colctrl \
    svtools/source/dialogs/colrdlg \
    svtools/source/dialogs/wizardmachine \
    svtools/source/dialogs/prnsetup \
    svtools/source/dialogs/filedlg \
    svtools/source/dialogs/property \
    svtools/source/table/tablecontrol \
    svtools/source/table/tablegeometry \
    svtools/source/table/gridtablerenderer \
    svtools/source/table/tabledatawindow \
    svtools/source/table/defaultinputhandler \
    svtools/source/table/tablecontrol_impl \
    svtools/source/contnr/svlbox \
    svtools/source/contnr/svtabbx \
    svtools/source/contnr/ivctrl \
    svtools/source/contnr/templwin \
    svtools/source/contnr/svicnvw \
    svtools/source/contnr/contentenumeration \
    svtools/source/contnr/svimpbox \
    svtools/source/contnr/imivctl1 \
    svtools/source/contnr/svtreebx \
    svtools/source/contnr/fileview \
    svtools/source/contnr/svimpicn \
    svtools/source/contnr/svlbitm \
    svtools/source/contnr/ctrdll \
    svtools/source/contnr/tooltiplbox \
    svtools/source/contnr/treelist \
    svtools/source/contnr/imivctl2 \
    svtools/source/uno/unoiface \
    svtools/source/uno/genericunodialog \
    svtools/source/uno/unocontroltablemodel \
    svtools/source/uno/contextmenuhelper \
    svtools/source/uno/toolboxcontroller \
    svtools/source/uno/generictoolboxcontroller \
    svtools/source/uno/addrtempuno \
    svtools/source/uno/miscservices \
    svtools/source/uno/statusbarcontroller \
    svtools/source/uno/svtxgridcontrol \
    svtools/source/uno/unoimap \
    svtools/source/uno/unoevent \
    svtools/source/uno/treecontrolpeer \
    svtools/source/uno/framestatuslistener \
    svtools/source/brwbox/editbrowsebox \
    svtools/source/brwbox/brwhead \
    svtools/source/brwbox/brwbox3 \
    svtools/source/brwbox/brwbox1 \
    svtools/source/brwbox/ebbcontrols \
    svtools/source/brwbox/brwbox2 \
    svtools/source/brwbox/datwin \
    svtools/source/brwbox/editbrowsebox2 \
    svtools/source/svrtf/rtfkeywd \
    svtools/source/svrtf/svparser \
    svtools/source/svrtf/parrtf \
    svtools/source/svrtf/rtfout \
    svtools/source/filter.vcl/filter/dlgepng \
    svtools/source/filter.vcl/filter/filter2 \
    svtools/source/filter.vcl/filter/FilterConfigItem \
    svtools/source/filter.vcl/filter/sgvspln \
    svtools/source/filter.vcl/filter/filter \
    svtools/source/filter.vcl/filter/sgvtext \
    svtools/source/filter.vcl/filter/sgfbram \
    svtools/source/filter.vcl/filter/SvFilterOptionsDialog \
    svtools/source/filter.vcl/filter/dlgexpor \
    svtools/source/filter.vcl/filter/fldll \
    svtools/source/filter.vcl/filter/sgvmain \
    svtools/source/filter.vcl/filter/FilterConfigCache \
    svtools/source/filter.vcl/filter/dlgejpg \
    svtools/source/filter.vcl/wmf/winwmf \
    svtools/source/filter.vcl/wmf/winmtf \
    svtools/source/filter.vcl/wmf/emfwr \
    svtools/source/filter.vcl/wmf/wmf \
    svtools/source/filter.vcl/wmf/enhwmf \
    svtools/source/filter.vcl/wmf/wmfwr \
    svtools/source/filter.vcl/igif/decode \
    svtools/source/filter.vcl/igif/gifread \
    svtools/source/filter.vcl/ixbm/xbmread \
    svtools/source/filter.vcl/jpeg/jpeg \
    svtools/source/filter.vcl/ixpm/xpmread \
))

$(eval $(call gb_Library_add_cobjects,svt,\
    svtools/source/filter.vcl/jpeg/jpegc \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,svt,\
    dl \
    m \
    pthread \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,svt,\
    advapi32 \
    gdi32 \
    kernel32 \
    msvcrt \
    oldnames \
    ole32 \
    oleaut32 \
    user32 \
    uuid \
    uwinapi \
))
endif
# vim: set noet sw=4 ts=4:
