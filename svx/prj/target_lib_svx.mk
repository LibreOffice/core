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

$(eval $(call gb_Library_Library,svx))

$(eval $(call gb_Library_set_include,svx,\
    $$(SOLARINC) \
    -I$(WORKDIR)/inc/svx/ \
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,svx,\
    $$(DEFS) \
    -DACCESSIBLE_LAYOUT \
    -DSVX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,svx,\
    basegfx \
    sb \
    comphelper \
    cppuhelper \
    cppu \
    drawinglayer \
    editeng \
    i18nisolang1 \
    icuuc \
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
    vos3 \
    xo \
    xcr \
    stl \
))

#    svx/source/unodialogs/textconversiondlgs/services \
#    svx/source/unodialogs/textconversiondlgs/chinese_dictionarydialog \
#    svx/source/unodialogs/textconversiondlgs/chinese_translationdialog \
#    svx/source/unodialogs/textconversiondlgs/chinese_translation_unodialog \
#    svx/source/unodialogs/textconversiondlgs/resid \


$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/unodraw/recoveryui \
    svx/source/unodraw/unoshcol \
    svx/source/unodraw/unoctabl \
    svx/source/unodraw/unopool \
    svx/source/unodraw/UnoNamespaceMap \
    svx/source/tbxctrls/layctrl \
    svx/source/tbxctrls/tbxcolor \
    svx/source/tbxctrls/formatpaintbrushctrl \
    svx/source/tbxctrls/colrctrl \
    svx/source/tbxctrls/itemwin \
    svx/source/tbxctrls/tbxalign \
    svx/source/tbxctrls/fillctrl \
    svx/source/tbxctrls/tbunocontroller \
    svx/source/tbxctrls/subtoolboxcontrol \
    svx/source/tbxctrls/grafctrl \
    svx/source/tbxctrls/linectrl \
    svx/source/tbxctrls/tbxdrctl \
    svx/source/tbxctrls/verttexttbxctrl \
    svx/source/tbxctrls/lboxctrl \
    svx/source/table/tablertfexporter \
    svx/source/table/tablertfimporter \
    svx/source/table/accessiblecell \
    svx/source/table/accessibletableshape \
    svx/source/table/tabledesign \
    svx/source/table/celleditsource \
    svx/source/svdraw/svdoimp \
    svx/source/svdraw/svdscrol \
    svx/source/svdraw/ActionDescriptionProvider \
    svx/source/svdraw/impgrfll \
    svx/source/items/SmartTagItem \
    svx/source/items/svxempty \
    svx/source/items/viewlayoutitem \
    svx/source/items/zoomslideritem \
    svx/source/items/hlnkitem \
    svx/source/items/numfmtsh \
    svx/source/items/algitem \
    svx/source/items/zoomitem \
    svx/source/items/ofaitem \
    svx/source/items/pageitem \
    svx/source/items/numinf \
    svx/source/items/postattr \
    svx/source/items/rotmodit \
    svx/source/items/svxerr \
    svx/source/form/databaselocationinput \
    svx/source/form/fmsrcimp \
    svx/source/form/filtnav \
    svx/source/form/tabwin \
    svx/source/form/fmsrccfg \
    svx/source/form/fmobjfac \
    svx/source/form/tbxform \
    svx/source/form/fmPropBrw \
    svx/source/form/dbcharsethelper \
    svx/source/stbctrls/pszctrl \
    svx/source/stbctrls/insctrl \
    svx/source/stbctrls/selctrl \
    svx/source/stbctrls/xmlsecctrl \
    svx/source/stbctrls/modctrl \
    svx/source/stbctrls/zoomsliderctrl \
    svx/source/stbctrls/zoomctrl \
    svx/source/gengal/gengal \
    svx/source/unogallery/unogaltheme \
    svx/source/unogallery/unogalitem \
    svx/source/unogallery/unogalthemeprovider \
    svx/source/mnuctrls/SmartTagCtl \
    svx/source/mnuctrls/fntctl \
    svx/source/mnuctrls/clipboardctl \
    svx/source/mnuctrls/fntszctl \
    svx/source/fmcomp/dbaobjectex \
    svx/source/smarttags/SmartTagMgr \
    svx/source/accessibility/AccessibleOLEShape \
    svx/source/accessibility/AccessibleShapeTreeInfo \
    svx/source/accessibility/DGColorNameLookUp \
    svx/source/accessibility/charmapacc \
    svx/source/accessibility/AccessibleShape \
    svx/source/accessibility/AccessibleTextHelper \
    svx/source/accessibility/svxrectctaccessiblecontext \
    svx/source/accessibility/AccessibleGraphicShape \
    svx/source/accessibility/AccessibleEmptyEditSource \
    svx/source/accessibility/ChildrenManagerImpl \
    svx/source/accessibility/AccessibleShapeInfo \
    svx/source/accessibility/DescriptionGenerator \
    svx/source/accessibility/ShapeTypeHandler \
    svx/source/accessibility/ChildrenManager \
    svx/source/accessibility/AccessibleControlShape \
    svx/source/accessibility/AccessibleTextEventQueue \
    svx/source/accessibility/AccessibleFrameSelector \
    svx/source/accessibility/SvxShapeTypes \
    svx/source/accessibility/GraphCtlAccessibleContext \
    svx/source/customshapes/tbxcustomshapes \
    svx/source/engine3d/float3d \
    svx/source/engine3d/volume3d \
    svx/source/engine3d/viewpt3d \
    svx/source/dialog/srchdlg \
    svx/source/dialog/rubydialog \
    svx/source/dialog/srchctrl \
    svx/source/dialog/paraprev \
    svx/source/dialog/fontwork \
    svx/source/dialog/framelinkarray \
    svx/source/dialog/svxbmpnumvalueset \
    svx/source/dialog/hyperdlg \
    svx/source/dialog/wrapfield \
    svx/source/dialog/svxruler \
    svx/source/dialog/ctredlin \
    svx/source/dialog/rulritem \
    svx/source/dialog/dialcontrol \
    svx/source/dialog/databaseregistrationui \
    svx/source/dialog/txencbox \
    svx/source/dialog/charmap \
    svx/source/dialog/passwd \
    svx/source/dialog/dlgctrl \
    svx/source/dialog/hdft \
    svx/source/dialog/frmdirlbox \
    svx/source/dialog/fntctrl \
    svx/source/dialog/docrecovery \
    svx/source/dialog/checklbx \
    svx/source/dialog/swframeposstrings \
    svx/source/dialog/svxgrahicitem \
    svx/source/dialog/relfld \
    svx/source/dialog/SpellDialogChildWindow \
    svx/source/dialog/strarray \
    svx/source/dialog/prtqry \
    svx/source/dialog/connctrl \
    svx/source/dialog/contwnd \
    svx/source/dialog/pfiledlg \
    svx/source/dialog/dlgctl3d \
    svx/source/dialog/hyprlink \
    svx/source/dialog/grfflt \
    svx/source/dialog/measctrl \
    svx/source/dialog/orienthelper \
    svx/source/dialog/frmsel \
    svx/source/dialog/swframeexample \
    svx/source/dialog/_bmpmask \
    svx/source/dialog/graphctl \
    svx/source/dialog/rlrcitem \
    svx/source/dialog/fontlb \
    svx/source/dialog/optgrid \
    svx/source/dialog/imapwnd \
    svx/source/dialog/pagectrl \
    svx/source/dialog/_contdlg \
    svx/source/dialog/txenctab \
    svx/source/dialog/imapdlg \
    svx/source/customshapes/EnhancedCustomShape3d \
    svx/source/customshapes/EnhancedCustomShapeHandle \
    svx/source/customshapes/EnhancedCustomShapeEngine \
    svx/source/customshapes/EnhancedCustomShapeFontWork \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportunx \
))
$(eval $(call gb_Library_add_linked_libs,svx,\
    dl \
    m \
    pthread \
))
else
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,svx,\
    advapi32 \
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportw32 \
))
))
else
$(eval $(call gb_Library_add_exception_objects,svx,\
    svx/source/dialog/sendreportgen \
))
endif

$(eval $(call gb_Library_add_linked_libs,svx,\
    shell \
))
endif

# vim: set noet sw=4 ts=4:
