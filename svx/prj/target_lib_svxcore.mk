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

$(eval $(call gb_Library_Library,svxcore))

$(eval $(call gb_Library_add_sdi_headers,svx,svx/sdi/svxslots))

$(eval $(call gb_Library_set_include,svxcore,\
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/inc/pch \
    -I$(WORKDIR)/SdiTarget/svx/sdi \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,svxcore,\
    $$(DEFS) \
    -DACCESSIBLE_LAYOUT \
    -DSVX_DLLIMPLEMENTATION \
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

$(eval $(call gb_Library_add_linked_libs,svxcore,\
    avmedia \
    basegfx \
    sb \
    comphelper \
    cppuhelper \
    cppu \
    drawinglayer \
    editeng \
    fwe \
    i18nisolang1 \
    icuuc \
    lng \
    sal \
    sfx \
    sot \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    vos3 \
    xo \
))

$(eval $(call gb_Library_add_exception_objects,svxcore,\
    svx/source/sdr/animation/scheduler \
    svx/source/sdr/animation/objectanimator \
    svx/source/sdr/animation/animationstate \
    svx/source/sdr/attribute/sdrlinefillshadowtextattribute \
    svx/source/sdr/attribute/sdrfilltextattribute \
    svx/source/sdr/attribute/sdrshadowtextattribute \
    svx/source/sdr/attribute/sdrtextattribute \
    svx/source/sdr/attribute/sdrlineshadowtextattribute \
    svx/source/sdr/attribute/sdrformtextattribute \
    svx/source/sdr/attribute/sdrformtextoutlineattribute \
    svx/source/sdr/contact/viewobjectcontactofgroup \
    svx/source/sdr/contact/viewobjectcontactofsdrpage \
    svx/source/sdr/contact/viewcontactofpageobj \
    svx/source/sdr/contact/viewcontactofe3d \
    svx/source/sdr/contact/viewcontactofe3dcube \
    svx/source/sdr/contact/viewcontactofsdrmediaobj \
    svx/source/sdr/contact/viewobjectcontactofmasterpagedescriptor \
    svx/source/sdr/contact/viewobjectcontactofpageobj \
    svx/source/sdr/contact/viewobjectcontactofe3dscene \
    svx/source/sdr/contact/viewcontactofgraphic \
    svx/source/sdr/contact/viewobjectcontactredirector \
    svx/source/sdr/contact/viewcontactofsdrcircobj \
    svx/source/sdr/contact/viewcontactofgroup \
    svx/source/sdr/contact/viewcontactofunocontrol \
    svx/source/sdr/contact/viewcontactofsdrobjcustomshape \
    svx/source/sdr/contact/viewcontactofsdrobj \
    svx/source/sdr/contact/objectcontact \
    svx/source/sdr/contact/viewcontactoftextobj \
    svx/source/sdr/contact/viewcontactofe3dlathe \
    svx/source/sdr/contact/viewcontactofsdrpage \
    svx/source/sdr/contact/viewcontactofe3dextrude \
    svx/source/sdr/contact/viewcontactofmasterpagedescriptor \
    svx/source/sdr/contact/viewcontact \
    svx/source/sdr/contact/viewcontactofsdrole2obj \
    svx/source/sdr/contact/displayinfo \
    svx/source/sdr/contact/objectcontacttools \
    svx/source/sdr/contact/viewcontactofe3dscene \
    svx/source/sdr/contact/viewcontactofsdrrectobj \
    svx/source/sdr/contact/viewobjectcontactofsdrole2obj \
    svx/source/sdr/contact/viewobjectcontactofunocontrol \
    svx/source/sdr/contact/viewobjectcontactofgraphic \
    svx/source/sdr/contact/sdrmediawindow \
    svx/source/sdr/contact/viewobjectcontactofsdrmediaobj \
    svx/source/sdr/contact/viewcontactofsdrcaptionobj \
    svx/source/sdr/contact/viewcontactofvirtobj \
    svx/source/sdr/contact/viewcontactofe3dsphere \
    svx/source/sdr/contact/viewcontactofe3dpolygon \
    svx/source/sdr/contact/viewcontactofsdredgeobj \
    svx/source/sdr/contact/viewobjectcontact \
    svx/source/sdr/contact/viewobjectcontactofsdrobj \
    svx/source/sdr/contact/objectcontactofpageview \
    svx/source/sdr/contact/viewcontactofsdrpathobj \
    svx/source/sdr/contact/viewcontactofsdrmeasureobj \
    svx/source/sdr/contact/objectcontactofobjlistpainter \
    svx/source/sdr/contact/viewobjectcontactofe3d \
    svx/source/customshapes/EnhancedCustomShapeGeometry \
    svx/source/customshapes/EnhancedCustomShapeTypeNames \
    svx/source/customshapes/EnhancedCustomShape2d \
    svx/source/customshapes/EnhancedCustomShapeFunctionParser \
    svx/source/dialog/dialmgr \
    svx/source/dialog/dlgutil \
    svx/source/dialog/framelink \
    svx/source/dialog/langbox \
    svx/source/dialog/simptabl \
    svx/source/dialog/svxdlg \
    svx/source/dialog/stddlg \
    svx/source/engine3d/deflt3d \
    svx/source/engine3d/polysc3d \
    svx/source/engine3d/sphere3d \
    svx/source/engine3d/helperminimaldepth3d \
    svx/source/engine3d/obj3d \
    svx/source/engine3d/dragmt3d \
    svx/source/engine3d/scene3d \
    svx/source/engine3d/view3d \
    svx/source/engine3d/e3dsceneupdater \
    svx/source/engine3d/viewpt3d2 \
    svx/source/engine3d/svx3ditems \
    svx/source/engine3d/camera3d \
    svx/source/engine3d/e3dundo \
    svx/source/engine3d/objfac3d \
    svx/source/engine3d/lathe3d \
    svx/source/engine3d/polygn3d \
    svx/source/engine3d/view3d1 \
    svx/source/engine3d/cube3d \
    svx/source/engine3d/helperhittest3d \
    svx/source/engine3d/extrud3d \
    svx/source/sdr/event/eventhandler \
    svx/source/fmcomp/gridctrl \
    svx/source/fmcomp/xmlexchg \
    svx/source/fmcomp/trace \
    svx/source/fmcomp/fmgridcl \
    svx/source/fmcomp/gridcell \
    svx/source/fmcomp/dbaexchange \
    svx/source/fmcomp/gridcols \
    svx/source/fmcomp/fmgridif \
    svx/source/form/formfeaturedispatcher \
    svx/source/form/typeconversionclient \
    svx/source/form/fmpgeimp \
    svx/source/form/fmcontrolbordermanager \
    svx/source/form/fmshell \
    svx/source/form/fmundo \
    svx/source/form/ParseContext \
    svx/source/form/fmexpl \
    svx/source/form/fmtools \
    svx/source/form/fmexch \
    svx/source/form/formcontroller \
    svx/source/form/dataaccessdescriptor \
    svx/source/form/fmtextcontrolshell \
    svx/source/form/datalistener \
    svx/source/form/formcontrolfactory \
    svx/source/form/sdbdatacolumn \
    svx/source/form/fmview \
    svx/source/form/delayedevent \
    svx/source/form/fmservs \
    svx/source/form/fmobj \
    svx/source/form/fmdocumentclassification \
    svx/source/form/sqlparserclient \
    svx/source/form/fmshimp \
    svx/source/form/typemap \
    svx/source/form/fmscriptingenv \
    svx/source/form/dbtoolsclient \
    svx/source/form/fmpage \
    svx/source/form/stringlistresource \
    svx/source/form/fmmodel \
    svx/source/form/formtoolbars \
    svx/source/form/fmdpage \
    svx/source/form/fmitems \
    svx/source/form/navigatortreemodel \
    svx/source/form/fmvwimp \
    svx/source/form/fmtextcontrolfeature \
    svx/source/form/fmdmod \
    svx/source/form/fmcontrollayout \
    svx/source/form/fmtextcontroldialogs \
    svx/source/form/formdispatchinterceptor \
    svx/source/form/formcontrolling \
    svx/source/form/navigatortree \
    svx/source/form/datanavi \
    svx/source/form/legacyformcontroller \
    svx/source/form/xfm_addcondition \
    svx/source/gallery2/galexpl \
    svx/source/gallery2/galtheme \
    svx/source/gallery2/galctrl \
    svx/source/gallery2/gallery1 \
    svx/source/gallery2/galmisc \
    svx/source/gallery2/galobj \
    svx/source/gallery2/galbrws2 \
    svx/source/gallery2/galbrws \
    svx/source/gallery2/galbrws1 \
    svx/source/gallery2/codec \
    svx/source/items/drawitem \
    svx/source/items/chrtitem \
    svx/source/items/e3ditem \
    svx/source/items/grfitem \
    svx/source/items/clipfmtitem \
    svx/source/items/customshapeitem \
    svx/source/sdr/overlay/overlayline \
    svx/source/sdr/overlay/overlaycrosshair \
    svx/source/sdr/overlay/overlayrollingrectangle \
    svx/source/sdr/overlay/overlaytriangle \
    svx/source/sdr/overlay/overlayselection \
    svx/source/sdr/overlay/overlayhatchrect \
    svx/source/sdr/overlay/overlaybitmapex \
    svx/source/sdr/overlay/overlaymanagerbuffered \
    svx/source/sdr/overlay/overlayhelpline \
    svx/source/sdr/overlay/overlayanimatedbitmapex \
    svx/source/sdr/overlay/overlaypolypolygon \
    svx/source/sdr/overlay/overlayobjectcell \
    svx/source/sdr/overlay/overlaytools \
    svx/source/sdr/overlay/overlayprimitive2dsequenceobject \
    svx/source/sdr/overlay/overlayobject \
    svx/source/sdr/overlay/overlaymanager \
    svx/source/sdr/overlay/overlayobjectlist \
    svx/source/sdr/primitive2d/sdrellipseprimitive2d \
    svx/source/sdr/primitive2d/sdrprimitivetools \
    svx/source/sdr/primitive2d/sdrtextprimitive2d \
    svx/source/sdr/primitive2d/primitivefactory2d \
    svx/source/sdr/primitive2d/sdrolecontentprimitive2d \
    svx/source/sdr/primitive2d/sdrcustomshapeprimitive2d \
    svx/source/sdr/primitive2d/sdrgrafprimitive2d \
    svx/source/sdr/primitive2d/sdrole2primitive2d \
    svx/source/sdr/primitive2d/sdrpathprimitive2d \
    svx/source/sdr/primitive2d/sdrrectangleprimitive2d \
    svx/source/sdr/primitive2d/sdrcaptionprimitive2d \
    svx/source/sdr/primitive2d/sdrconnectorprimitive2d \
    svx/source/sdr/primitive2d/sdrmeasureprimitive2d \
    svx/source/sdr/primitive2d/sdrattributecreator \
    svx/source/sdr/primitive2d/sdrdecompositiontools \
    svx/source/sdr/primitive3d/sdrattributecreator3d \
    svx/source/sdr/properties/pageproperties \
    svx/source/sdr/properties/e3dsceneproperties \
    svx/source/sdr/properties/itemsettools \
    svx/source/sdr/properties/captionproperties \
    svx/source/sdr/properties/e3dextrudeproperties \
    svx/source/sdr/properties/e3dproperties \
    svx/source/sdr/properties/measureproperties \
    svx/source/sdr/properties/customshapeproperties \
    svx/source/sdr/properties/rectangleproperties \
    svx/source/sdr/properties/groupproperties \
    svx/source/sdr/properties/properties \
    svx/source/sdr/properties/e3dlatheproperties \
    svx/source/sdr/properties/graphicproperties \
    svx/source/sdr/properties/circleproperties \
    svx/source/sdr/properties/emptyproperties \
    svx/source/sdr/properties/e3dsphereproperties \
    svx/source/sdr/properties/defaultproperties \
    svx/source/sdr/properties/textproperties \
    svx/source/sdr/properties/attributeproperties \
    svx/source/sdr/properties/connectorproperties \
    svx/source/sdr/properties/e3dcompoundproperties \
    svx/source/sdr/properties/oleproperties \
    svx/source/svdraw/svdglev \
    svx/source/svdraw/svdhdl \
    svx/source/svdraw/svdomedia \
    svx/source/svdraw/svdedxv \
    svx/source/svdraw/svdoedge \
    svx/source/svdraw/svdundo \
    svx/source/svdraw/svdmrkv1 \
    svx/source/svdraw/svdotxfl \
    svx/source/svdraw/sdrcomment \
    svx/source/svdraw/svdcrtv \
    svx/source/svdraw/svdouno \
    svx/source/svdraw/svditer \
    svx/source/svdraw/svdotxln \
    svx/source/svdraw/svdxcgv \
    svx/source/svdraw/svdomeas \
    svx/source/svdraw/svdpntv \
    svx/source/svdraw/svdattr \
    svx/source/svdraw/svdotxdr \
    svx/source/svdraw/svdtext \
    svx/source/svdraw/svdedtv2 \
    svx/source/svdraw/svddrag \
    svx/source/svdraw/svdopage \
    svx/source/svdraw/svddrgmt \
    svx/source/svdraw/svdogrp \
    svx/source/svdraw/svdovirt \
    svx/source/svdraw/svddrgv \
    svx/source/svdraw/sdrpaintwindow \
    svx/source/svdraw/polypolygoneditor \
    svx/source/svdraw/svdpoev \
    svx/source/svdraw/svdotextpathdecomposition \
    svx/source/svdraw/svdmrkv \
    svx/source/svdraw/svdview \
    svx/source/svdraw/svdoashp \
    svx/source/svdraw/svdoutlinercache \
    svx/source/svdraw/svdetc \
    svx/source/svdraw/svdlayer \
    svx/source/svdraw/svdfmtf \
    svx/source/svdraw/gradtrns \
    svx/source/svdraw/svdotxed \
    svx/source/svdraw/svdhlpln \
    svx/source/svdraw/svdedtv \
    svx/source/svdraw/svdmodel \
    svx/source/svdraw/svdopath \
    svx/source/svdraw/svdtrans \
    svx/source/svdraw/svdpagv \
    svx/source/svdraw/svdorect \
    svx/source/svdraw/svdedtv1 \
    svx/source/svdraw/svdograf \
    svx/source/svdraw/sdrhittesthelper \
    svx/source/svdraw/svdglue \
    svx/source/svdraw/svdobj \
    svx/source/svdraw/svdsnpv \
    svx/source/svdraw/svdotext \
    svx/source/svdraw/svdoattr \
    svx/source/svdraw/svdviter \
    svx/source/svdraw/svdpage \
    svx/source/svdraw/svdoole2 \
    svx/source/svdraw/svdibrow \
    svx/source/svdraw/sdrmasterpagedescriptor \
    svx/source/svdraw/svdocapt \
    svx/source/svdraw/svdmark \
    svx/source/svdraw/svdocirc \
    svx/source/svdraw/selectioncontroller \
    svx/source/svdraw/clonelist \
    svx/source/svdraw/svdotxtr \
    svx/source/svdraw/svdoutl \
    svx/source/svdraw/sdrpagewindow \
    svx/source/svdraw/svdotxat \
    svx/source/svdraw/svdotextdecomposition \
    svx/source/table/tablecontroller \
    svx/source/table/tablerow \
    svx/source/table/tablelayouter \
    svx/source/table/tablecolumn \
    svx/source/table/tablemodel \
    svx/source/table/svdotable \
    svx/source/table/cellcursor \
    svx/source/table/propertyset \
    svx/source/table/tablecolumns \
    svx/source/table/viewcontactoftableobj \
    svx/source/table/cellrange \
    svx/source/table/tableundo \
    svx/source/table/tablerows \
    svx/source/table/tablehandles \
    svx/source/table/cell \
    svx/source/tbxctrls/fontworkgallery \
    svx/source/tbxctrls/extrusioncontrols \
    svx/source/tbxctrls/tbcontrl \
    svx/source/tbxctrls/toolbarmenu \
    svx/source/tbxctrls/tbxcolorupdate \
    svx/source/toolbars/fontworkbar \
    svx/source/toolbars/extrusionbar \
    svx/source/unodraw/unoshap4 \
    svx/source/unodraw/unomlstr \
    svx/source/unodraw/unoshtxt \
    svx/source/unodraw/gluepts \
    svx/source/unodraw/shapepropertynotifier \
    svx/source/unodraw/tableshape \
    svx/source/unodraw/unohtabl \
    svx/source/unodraw/unoshape \
    svx/source/unodraw/UnoGraphicExporter \
    svx/source/unodraw/unobtabl \
    svx/source/unodraw/XPropertyTable \
    svx/source/unodraw/unoshap3 \
    svx/source/unodraw/unottabl \
    svx/source/unodraw/UnoNameItemTable \
    svx/source/unodraw/unogtabl \
    svx/source/unodraw/unomod \
    svx/source/unodraw/unopage \
    svx/source/unodraw/unoprov \
    svx/source/unodraw/unomtabl \
    svx/source/unodraw/unoshap2 \
    svx/source/unodraw/unodtabl \
    svx/source/xml/xmlexport \
    svx/source/xml/xmlgrhlp \
    svx/source/xml/xmlxtimp \
    svx/source/xml/xmlxtexp \
    svx/source/xml/xmleohlp \
    svx/source/xoutdev/xtablend \
    svx/source/xoutdev/xattr \
    svx/source/xoutdev/xtabgrdt \
    svx/source/xoutdev/xtabhtch \
    svx/source/xoutdev/_xoutbmp \
    svx/source/xoutdev/xtabdash \
    svx/source/xoutdev/xtabbtmp \
    svx/source/xoutdev/xexch \
    svx/source/xoutdev/xtabcolr \
    svx/source/xoutdev/xattr2 \
    svx/source/xoutdev/xtable \
    svx/source/xoutdev/xattrbmp \
    svx/source/xoutdev/_xpoly \
    svx/source/xoutdev/xpool \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,svxcore,\
    dl \
    m \
    pthread \
    stl \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,svxcore,\
    advapi32 \
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif

$(eval $(call gb_SdiTarget_SdiTarget,svx/sdi/svxslots,svx/sdi/svx))


$(eval $(call gb_SdiTarget_set_include,svx/sdi/svxslots,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/sdi \
))

# vim: set noet sw=4 ts=4:
