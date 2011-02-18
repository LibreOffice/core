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

$(eval $(call gb_Library_Library,sfx))

$(eval $(call gb_Library_add_package_headers,sfx,\
    sfx2_inc \
    sfx2_sdi \
))

$(eval $(call gb_Library_add_precompiled_header,sfx,$(SRCDIR)/sfx2/inc/pch/precompiled_sfx2))

$(eval $(call gb_Library_add_sdi_headers,sfx,sfx2/sdi/sfxslots))

$(eval $(call gb_Library_set_componentfile,sfx,sfx2/util/sfx))

$(eval $(call gb_Library_set_include,sfx,\
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/inc/sfx2 \
    -I$(SRCDIR)/sfx2/source/inc \
    -I$(SRCDIR)/sfx2/inc/pch \
    -I$(WORKDIR)/SdiTarget/sfx2/sdi \
    -I$(WORKDIR)/inc/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
    $(LIBXML_CFLAGS) \
))

$(eval $(call gb_Library_set_defs,sfx,\
    $$(DEFS) \
    -DSFX2_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sfx,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    i18nisolang1 \
    sal \
    sax \
    sb \
    sot \
    stl \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    vos3 \
    xml2 \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sfx,\
    sfx2/source/appl/app \
    sfx2/source/appl/appbas \
    sfx2/source/appl/appbaslib \
    sfx2/source/appl/appcfg \
    sfx2/source/appl/appchild \
    sfx2/source/appl/appdata \
    sfx2/source/appl/appdde \
    sfx2/source/appl/appinit \
    sfx2/source/appl/appmain \
    sfx2/source/appl/appmisc \
    sfx2/source/appl/appopen \
    sfx2/source/appl/appquit \
    sfx2/source/appl/appreg \
    sfx2/source/appl/appserv \
    sfx2/source/appl/appuno \
    sfx2/source/appl/childwin \
    sfx2/source/appl/fileobj \
    sfx2/source/appl/fwkhelper \
    sfx2/source/appl/helpdispatch \
    sfx2/source/appl/helpinterceptor \
    sfx2/source/appl/imagemgr \
    sfx2/source/appl/imestatuswindow \
    sfx2/source/appl/impldde \
    sfx2/source/appl/linkmgr2 \
    sfx2/source/appl/linksrc \
    sfx2/source/appl/lnkbase2 \
    sfx2/source/appl/module \
    sfx2/source/appl/newhelp \
    sfx2/source/appl/opengrf \
    sfx2/source/appl/sfxhelp \
    sfx2/source/appl/sfxpicklist \
    sfx2/source/appl/shutdownicon \
    sfx2/source/appl/workwin \
    sfx2/source/appl/xpackcreator \
    sfx2/source/bastyp/bitset \
    sfx2/source/bastyp/fltfnc \
    sfx2/source/bastyp/fltlst \
    sfx2/source/bastyp/frmhtml \
    sfx2/source/bastyp/frmhtmlw \
    sfx2/source/bastyp/helper \
    sfx2/source/bastyp/mieclip \
    sfx2/source/bastyp/minarray \
    sfx2/source/bastyp/misc \
    sfx2/source/bastyp/progress \
    sfx2/source/bastyp/sfxhtml \
    sfx2/source/bastyp/sfxresid \
    sfx2/source/config/evntconf \
    sfx2/source/control/bindings \
    sfx2/source/control/ctrlitem \
    sfx2/source/control/dispatch \
    sfx2/source/control/macro \
    sfx2/source/control/minfitem \
    sfx2/source/control/msg \
    sfx2/source/control/msgpool \
    sfx2/source/control/objface \
    sfx2/source/control/querystatus \
    sfx2/source/control/request \
    sfx2/source/control/sfxstatuslistener \
    sfx2/source/control/shell \
    sfx2/source/control/sorgitm \
    sfx2/source/control/statcach \
    sfx2/source/control/unoctitm \
    sfx2/source/dialog/alienwarn \
    sfx2/source/dialog/basedlgs \
    sfx2/source/dialog/dinfdlg \
    sfx2/source/dialog/dinfedt \
    sfx2/source/dialog/dockwin \
    sfx2/source/dialog/filedlghelper \
    sfx2/source/dialog/filtergrouping \
    sfx2/source/dialog/intro \
    sfx2/source/dialog/itemconnect \
    sfx2/source/dialog/mailmodel \
    sfx2/source/dialog/mgetempl \
    sfx2/source/dialog/navigat \
    sfx2/source/dialog/newstyle \
    sfx2/source/dialog/partwnd \
    sfx2/source/dialog/passwd \
    sfx2/source/dialog/printopt \
    sfx2/source/dialog/recfloat \
    sfx2/source/dialog/securitypage \
    sfx2/source/dialog/sfxdlg \
    sfx2/source/dialog/splitwin \
    sfx2/source/dialog/srchdlg \
    sfx2/source/dialog/styfitem \
    sfx2/source/dialog/styledlg \
    sfx2/source/dialog/tabdlg \
    sfx2/source/dialog/taskpane \
    sfx2/source/dialog/templdlg \
    sfx2/source/dialog/titledockwin \
    sfx2/source/dialog/tplcitem \
    sfx2/source/dialog/tplpitem \
    sfx2/source/dialog/versdlg \
    sfx2/source/doc/DocumentMetadataAccess \
    sfx2/source/doc/Metadatable \
    sfx2/source/doc/QuerySaveDocument \
    sfx2/source/doc/SfxDocumentMetaData \
    sfx2/source/doc/docfac \
    sfx2/source/doc/docfile \
    sfx2/source/doc/docfilt \
    sfx2/source/doc/docinf \
    sfx2/source/doc/docinsert \
    sfx2/source/doc/docmacromode \
    sfx2/source/doc/docstoragemodifylistener \
    sfx2/source/doc/doctdlg \
    sfx2/source/doc/doctempl \
    sfx2/source/doc/doctemplates \
    sfx2/source/doc/doctemplateslocal \
    sfx2/source/doc/docvor \
    sfx2/source/doc/frmdescr \
    sfx2/source/doc/graphhelp \
    sfx2/source/doc/guisaveas \
    sfx2/source/doc/iframe \
    sfx2/source/doc/new \
    sfx2/source/doc/objcont \
    sfx2/source/doc/objembed \
    sfx2/source/doc/objitem \
    sfx2/source/doc/objmisc \
    sfx2/source/doc/objserv \
    sfx2/source/doc/objstor \
    sfx2/source/doc/objuno \
    sfx2/source/doc/objxtor \
    sfx2/source/doc/oleprops \
    sfx2/source/doc/ownsubfilterservice \
    sfx2/source/doc/plugin \
    sfx2/source/doc/printhelper \
    sfx2/source/doc/querytemplate \
    sfx2/source/doc/docundomanager \
    sfx2/source/doc/sfxbasemodel \
    sfx2/source/doc/sfxmodelfactory \
    sfx2/source/doc/syspath \
    sfx2/source/explorer/nochaos \
    sfx2/source/inet/inettbc \
    sfx2/source/menu/mnuitem \
    sfx2/source/menu/mnumgr \
    sfx2/source/menu/objmnctl \
    sfx2/source/menu/thessubmenu \
    sfx2/source/menu/virtmenu \
    sfx2/source/notify/eventsupplier \
    sfx2/source/notify/hintpost \
    sfx2/source/statbar/stbitem \
    sfx2/source/toolbox/imgmgr \
    sfx2/source/toolbox/tbxitem \
    sfx2/source/view/frame \
    sfx2/source/view/frame2 \
    sfx2/source/view/frmload \
    sfx2/source/view/ipclient \
    sfx2/source/view/orgmgr \
    sfx2/source/view/printer \
    sfx2/source/view/sfxbasecontroller \
    sfx2/source/view/userinputinterception \
    sfx2/source/view/viewfac \
    sfx2/source/view/viewfrm \
    sfx2/source/view/viewfrm2 \
    sfx2/source/view/viewprn \
    sfx2/source/view/viewsh \
))

$(eval $(call gb_SdiTarget_SdiTarget,sfx2/sdi/sfxslots,sfx2/sdi/sfx))

$(eval $(call gb_SdiTarget_set_include,sfx2/sdi/sfxslots,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sfx2/inc/sfx2 \
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/sdi \
))

ifeq ($(OS),$(filter WNT MACOSX,$(OS)))
$(eval $(call gb_Library_set_defs,sfx,\
    $$(DEFS) \
    -DENABLE_QUICKSTART_APPLET \
))
endif

ifeq ($(OS),OS2)
$(eval $(call gb_Library_add_exception_objects,sfx,\
    sfx2/source/appl/shutdowniconOs2.ob \
))
endif
ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_objcxxobjects,sfx,\
    sfx2/source/appl/shutdowniconaqua \
))
$(eval $(call gb_Library_add_linked_libs,sfx,\
    objc \
    Cocoa \
))
endif

ifeq ($(OS),WNT)

# workaround: disable PCH for these objects to avoid redeclaration
# errors - needs to be fixed in module tools
$(eval $(call gb_Library_add_cxxobjects,sfx,\
    sfx2/source/appl/shutdowniconw32 \
    sfx2/source/doc/sfxacldetect \
    sfx2/source/doc/syspathw32 \
    , $(gb_LinkTarget_EXCEPTIONFLAGS) $(gb_COMPILEROPTFLAGS) -nologo -UPRECOMPILED_HEADERS \
))

$(eval $(call gb_Library_add_linked_libs,sfx,\
    gdi32 \
    advapi32 \
    ole32 \
    shell32 \
    user32 \
    uuid \
))

else

$(eval $(call gb_Library_add_cxxobjects,sfx,\
    sfx2/source/appl/shutdowniconw32 \
    sfx2/source/doc/sfxacldetect \
    , $(gb_LinkTarget_EXCEPTIONFLAGS) $(gb_COMPILEROPTFLAGS) \
))

endif
# vim: set noet sw=4 ts=4:

