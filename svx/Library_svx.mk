# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,svx))

$(eval $(call gb_Library_set_componentfile,svx,svx/util/svx))

$(eval $(call gb_Library_set_include,svx,\
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,svx))

$(eval $(call gb_Library_add_defs,svx,\
    -DSVX_DLLIMPLEMENTATION \
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

$(eval $(call gb_Library_use_libraries,svx,\
    basegfx \
    sb \
    comphelper \
    cppuhelper \
    cppu \
    drawinglayer \
    editeng \
    i18nisolang1 \
    sal \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
    xmlscript \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_external,svx,icuuc))

$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/accessibility/AccessibleControlShape \
    svx/source/accessibility/AccessibleEmptyEditSource \
    svx/source/accessibility/AccessibleFrameSelector \
    svx/source/accessibility/AccessibleGraphicShape \
    svx/source/accessibility/AccessibleOLEShape \
    svx/source/accessibility/AccessibleShape \
    svx/source/accessibility/AccessibleShapeInfo \
    svx/source/accessibility/AccessibleShapeTreeInfo \
    svx/source/accessibility/AccessibleTextEventQueue \
    svx/source/accessibility/AccessibleTextHelper \
    svx/source/accessibility/ChildrenManager \
    svx/source/accessibility/ChildrenManagerImpl \
    svx/source/accessibility/DescriptionGenerator \
    svx/source/accessibility/GraphCtlAccessibleContext \
    svx/source/accessibility/ShapeTypeHandler \
    svx/source/accessibility/SvxShapeTypes \
    svx/source/accessibility/charmapacc \
    svx/source/accessibility/lookupcolorname \
    svx/source/accessibility/svxrectctaccessiblecontext \
    svx/source/customshapes/EnhancedCustomShape3d \
    svx/source/customshapes/EnhancedCustomShapeEngine \
    svx/source/customshapes/EnhancedCustomShapeFontWork \
    svx/source/customshapes/EnhancedCustomShapeHandle \
    svx/source/customshapes/tbxcustomshapes \
    svx/source/dialog/_bmpmask \
    svx/source/dialog/charmap \
    svx/source/dialog/connctrl \
    svx/source/dialog/_contdlg \
    svx/source/dialog/contwnd \
    svx/source/dialog/compressgraphicdialog \
    svx/source/dialog/ctredlin \
    svx/source/dialog/databaseregistrationui \
    svx/source/dialog/dialcontrol \
    svx/source/dialog/dlgctl3d \
    svx/source/dialog/dlgctrl \
    svx/source/dialog/docrecovery \
    svx/source/dialog/fntctrl \
    svx/source/dialog/fontlb \
    svx/source/dialog/fontwork \
    svx/source/dialog/framelinkarray \
    svx/source/dialog/frmdirlbox \
    svx/source/dialog/frmsel \
    svx/source/dialog/graphctl \
    svx/source/dialog/grfflt \
    svx/source/dialog/hdft \
    svx/source/dialog/hyperdlg \
    svx/source/dialog/imapdlg \
    svx/source/dialog/imapwnd \
    svx/source/dialog/linkwarn \
    svx/source/dialog/measctrl \
    svx/source/dialog/optgrid \
    svx/source/dialog/orienthelper \
    svx/source/dialog/pagectrl \
    svx/source/dialog/paraprev \
    svx/source/dialog/passwd \
    svx/source/dialog/pfiledlg \
    svx/source/dialog/prtqry \
    svx/source/dialog/relfld \
    svx/source/dialog/rlrcitem \
    svx/source/dialog/rubydialog \
    svx/source/dialog/rulritem \
    svx/source/dialog/SpellDialogChildWindow \
    svx/source/dialog/srchctrl \
    svx/source/dialog/srchdlg \
    svx/source/dialog/strarray \
    svx/source/dialog/svxbmpnumvalueset \
    svx/source/dialog/svxgrahicitem \
    svx/source/dialog/svxruler \
    svx/source/dialog/swframeexample \
    svx/source/dialog/swframeposstrings \
    svx/source/dialog/txencbox \
    svx/source/dialog/txenctab \
    svx/source/dialog/wrapfield \
    svx/source/engine3d/float3d \
    svx/source/items/algitem \
    svx/source/items/hlnkitem \
    svx/source/items/numfmtsh \
    svx/source/items/numinf \
    svx/source/items/ofaitem \
    svx/source/items/pageitem \
    svx/source/items/postattr \
    svx/source/items/rotmodit \
    svx/source/items/SmartTagItem \
    svx/source/items/svxerr \
    svx/source/items/viewlayoutitem \
    svx/source/items/zoomslideritem \
    svx/source/mnuctrls/clipboardctl \
    svx/source/mnuctrls/fntctl \
    svx/source/mnuctrls/fntszctl \
    svx/source/mnuctrls/SmartTagCtl \
    svx/source/stbctrls/pszctrl \
    svx/source/stbctrls/insctrl \
    svx/source/stbctrls/selctrl \
    svx/source/stbctrls/xmlsecctrl \
    svx/source/stbctrls/modctrl \
    svx/source/stbctrls/zoomsliderctrl \
    svx/source/stbctrls/zoomctrl \
    svx/source/svdraw/ActionDescriptionProvider \
    svx/source/smarttags/SmartTagMgr \
    svx/source/table/accessiblecell \
    svx/source/table/accessibletableshape \
    svx/source/table/tabledesign \
    svx/source/table/tablertfexporter \
    svx/source/table/tablertfimporter \
    svx/source/tbxctrls/colrctrl \
    svx/source/tbxctrls/fillctrl \
    svx/source/tbxctrls/formatpaintbrushctrl \
    svx/source/tbxctrls/grafctrl \
    svx/source/tbxctrls/itemwin \
    svx/source/tbxctrls/layctrl \
    svx/source/tbxctrls/lboxctrl \
    svx/source/tbxctrls/linectrl \
    svx/source/tbxctrls/subtoolboxcontrol \
    svx/source/tbxctrls/tbunocontroller \
    svx/source/tbxctrls/tbunosearchcontrollers \
    svx/source/tbxctrls/tbxalign \
    svx/source/tbxctrls/tbxcolor \
    svx/source/tbxctrls/tbxdrctl \
    svx/source/tbxctrls/verttexttbxctrl \
    svx/source/unodraw/recoveryui \
    svx/source/unodraw/unoctabl \
    svx/source/unodraw/UnoNamespaceMap \
    svx/source/unodraw/unopool \
    svx/source/unodraw/unoshcol \
    svx/source/unogallery/unogalitem \
    svx/source/unogallery/unogaltheme \
    svx/source/unogallery/unogalthemeprovider \
))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/fmcomp/dbaobjectex \
    svx/source/form/databaselocationinput \
    svx/source/form/dbcharsethelper \
    svx/source/form/filtnav \
    svx/source/form/fmobjfac \
    svx/source/form/fmPropBrw \
    svx/source/form/fmsrccfg \
    svx/source/form/fmsrcimp \
    svx/source/form/tabwin \
    svx/source/form/tbxform \
))
endif

ifneq (,$(filter LINUX DRAGONFLY OPENBSD FREEBSD NETBSD, $(OS)))
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportunx \
))
else
ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,svx,\
    advapi32 \
))
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportw32 \
))
else
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportgen \
))
endif
endif

# vim: set noet sw=4 ts=4:
