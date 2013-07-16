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

$(eval $(call gb_Library_Library,svxcore))

$(eval $(call gb_Library_set_componentfile,svxcore,svx/util/svxcore))

$(eval $(call gb_Library_add_sdi_headers,svxcore,svx/sdi/svxslots))

$(eval $(call gb_Library_use_custom_headers,svxcore,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,svxcore))

$(eval $(call gb_Library_set_include,svxcore,\
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/source/inc \
    $$(INCLUDE) \
    -I$(WORKDIR)/SdiTarget/svx/sdi \
))

$(eval $(call gb_Library_set_precompiled_header,svxcore,$(SRCDIR)/svx/inc/pch/precompiled_svxcore))

#BOOST switch in customshapes
#dialog:
#.IF "$(OS)"=="WNT"
#CFLAGS+= -DUNICODE -D_UNICODE
#.ENDIF

$(eval $(call gb_Library_add_defs,svxcore,\
    -DSVX_DLLIMPLEMENTATION \
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

$(eval $(call gb_Library_use_libraries,svxcore,\
    avmedia \
    basegfx \
    sb \
    comphelper \
    cppuhelper \
    cppu \
    drawinglayer \
    editeng \
    fwe \
    i18nlangtag \
    lng \
    sal \
    salhelper \
    sax \
    sfx \
    sot \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,svxcore,\
	boost_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,svxcore,\
    svx/source/core/coreservices \
    svx/source/core/extedit \
    svx/source/core/graphichelper \
    svx/source/customshapes/EnhancedCustomShape2d \
    svx/source/customshapes/EnhancedCustomShapeGeometry \
    svx/source/customshapes/EnhancedCustomShapeTypeNames \
    svx/source/dialog/checklbx \
    svx/source/dialog/dialmgr \
    svx/source/dialog/dlgutil \
    svx/source/dialog/framelink \
    svx/source/dialog/langbox \
    svx/source/dialog/stddlg \
    svx/source/dialog/svxdlg \
    svx/source/engine3d/camera3d \
    svx/source/engine3d/cube3d \
    svx/source/engine3d/deflt3d \
    svx/source/engine3d/dragmt3d \
    svx/source/engine3d/e3dsceneupdater \
    svx/source/engine3d/e3dundo \
    svx/source/engine3d/extrud3d \
    svx/source/engine3d/helperhittest3d \
    svx/source/engine3d/helperminimaldepth3d \
    svx/source/engine3d/lathe3d \
    svx/source/engine3d/obj3d \
    svx/source/engine3d/objfac3d \
    svx/source/engine3d/polygn3d \
    svx/source/engine3d/polysc3d \
    svx/source/engine3d/scene3d \
    svx/source/engine3d/sphere3d \
    svx/source/engine3d/svx3ditems \
    svx/source/engine3d/view3d \
    svx/source/engine3d/view3d1 \
    svx/source/engine3d/viewpt3d2 \
    svx/source/gallery2/codec \
    svx/source/gallery2/galbrws \
    svx/source/gallery2/galbrws1 \
    svx/source/gallery2/galbrws2 \
    svx/source/gallery2/galctrl \
    svx/source/gallery2/galexpl \
    svx/source/gallery2/galini \
    svx/source/gallery2/gallery1 \
    svx/source/gallery2/galmisc \
    svx/source/gallery2/galobj \
    svx/source/gallery2/galtheme \
    svx/source/gallery2/GalleryControl \
    svx/source/gallery2/GallerySplitter \
    svx/source/items/chrtitem \
    svx/source/items/clipfmtitem \
    svx/source/items/customshapeitem \
    svx/source/items/drawitem \
    svx/source/items/e3ditem \
    svx/source/items/galleryitem \
    svx/source/items/grfitem \
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
    svx/source/sdr/event/eventhandler \
    svx/source/sdr/overlay/overlayline \
    svx/source/sdr/overlay/overlaycrosshair \
    svx/source/sdr/overlay/overlayrollingrectangle \
    svx/source/sdr/overlay/overlaytriangle \
    svx/source/sdr/overlay/overlayselection \
    svx/source/sdr/overlay/overlayrectangle \
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
    svx/source/svdraw/clonelist \
    svx/source/svdraw/charthelper \
    svx/source/svdraw/gradtrns \
    svx/source/svdraw/polypolygoneditor \
    svx/source/svdraw/sdrhittesthelper \
    svx/source/svdraw/sdrmasterpagedescriptor \
    svx/source/svdraw/sdrpagewindow \
    svx/source/svdraw/sdrpaintwindow \
    svx/source/svdraw/sdrundomanager \
    svx/source/svdraw/selectioncontroller \
    svx/source/svdraw/svdattr \
    svx/source/svdraw/svdcrtv \
    svx/source/svdraw/svddrag \
    svx/source/svdraw/svddrgmt \
    svx/source/svdraw/svddrgv \
    svx/source/svdraw/svdedtv \
    svx/source/svdraw/svdedtv1 \
    svx/source/svdraw/svdedtv2 \
    svx/source/svdraw/svdedxv \
    svx/source/svdraw/svdetc \
    svx/source/svdraw/svdfmtf \
    svx/source/svdraw/svdglev \
    svx/source/svdraw/svdglue \
    svx/source/svdraw/svdhdl \
    svx/source/svdraw/svdhlpln \
    svx/source/svdraw/svdibrow \
    svx/source/svdraw/svditer \
    svx/source/svdraw/svdlayer \
    svx/source/svdraw/svdmark \
    svx/source/svdraw/svdmodel \
    svx/source/svdraw/svdmrkv \
    svx/source/svdraw/svdmrkv1 \
    svx/source/svdraw/svdoashp \
    svx/source/svdraw/svdoattr \
    svx/source/svdraw/svdobj \
    svx/source/svdraw/svdocapt \
    svx/source/svdraw/svdocirc \
    svx/source/svdraw/svdoedge \
    svx/source/svdraw/svdograf \
    svx/source/svdraw/svdogrp \
    svx/source/svdraw/svdomeas \
    svx/source/svdraw/svdomedia \
    svx/source/svdraw/svdoole2 \
    svx/source/svdraw/svdopage \
    svx/source/svdraw/svdopath \
    svx/source/svdraw/svdorect \
    svx/source/svdraw/svdotext \
    svx/source/svdraw/svdotextdecomposition \
    svx/source/svdraw/svdotextpathdecomposition \
    svx/source/svdraw/svdotxat \
    svx/source/svdraw/svdotxdr \
    svx/source/svdraw/svdotxed \
    svx/source/svdraw/svdotxfl \
    svx/source/svdraw/svdotxln \
    svx/source/svdraw/svdotxtr \
    svx/source/svdraw/svdouno \
    svx/source/svdraw/svdoutl \
    svx/source/svdraw/svdoutlinercache \
    svx/source/svdraw/svdovirt \
    svx/source/svdraw/svdpage \
    svx/source/svdraw/svdpagv \
    svx/source/svdraw/svdpntv \
    svx/source/svdraw/svdpoev \
    svx/source/svdraw/svdsnpv \
    svx/source/svdraw/svdtext \
    svx/source/svdraw/svdtrans \
    svx/source/svdraw/svdundo \
    svx/source/svdraw/svdview \
    svx/source/svdraw/svdviter \
    svx/source/svdraw/svdxcgv \
    svx/source/table/cell \
    svx/source/table/cellcursor \
    svx/source/table/cellrange \
    svx/source/table/propertyset \
    svx/source/table/svdotable \
    svx/source/table/tablecolumn \
    svx/source/table/tablecolumns \
    svx/source/table/tablecontroller \
    svx/source/table/tablehandles \
    svx/source/table/tablelayouter \
    svx/source/table/tablemodel \
    svx/source/table/tablerow \
    svx/source/table/tablerows \
    svx/source/table/tableundo \
    svx/source/table/viewcontactoftableobj \
    svx/source/tbxctrls/extrusioncontrols \
    svx/source/tbxctrls/fontworkgallery \
    svx/source/tbxctrls/tbcontrl \
    svx/source/tbxctrls/tbxcolorupdate \
    svx/source/tbxctrls/SvxColorValueSet \
    svx/source/toolbars/extrusionbar \
    svx/source/toolbars/fontworkbar \
    svx/source/unodraw/gluepts \
    svx/source/unodraw/shapepropertynotifier \
    svx/source/unodraw/tableshape \
    svx/source/unodraw/unobtabl \
    svx/source/unodraw/unodtabl \
    svx/source/unodraw/UnoGraphicExporter \
    svx/source/unodraw/unogtabl \
    svx/source/unodraw/unohtabl \
    svx/source/unodraw/unomlstr \
    svx/source/unodraw/unomod \
    svx/source/unodraw/unomtabl \
    svx/source/unodraw/UnoNameItemTable \
    svx/source/unodraw/unopage \
    svx/source/unodraw/unoprov \
    svx/source/unodraw/unoshap2 \
    svx/source/unodraw/unoshap3 \
    svx/source/unodraw/unoshap4 \
    svx/source/unodraw/unoshape \
    svx/source/unodraw/unoshtxt \
    svx/source/unodraw/unottabl \
    svx/source/unodraw/XPropertyTable \
    svx/source/xml/xmleohlp \
    svx/source/xml/xmlexport \
    svx/source/xml/xmlgrhlp \
    svx/source/xml/xmlxtexp \
    svx/source/xml/xmlxtimp \
    svx/source/xoutdev/xattr \
    svx/source/xoutdev/xattr2 \
    svx/source/xoutdev/xattrbmp \
    svx/source/xoutdev/xexch \
    svx/source/xoutdev/_xoutbmp \
    svx/source/xoutdev/_xpoly \
    svx/source/xoutdev/xpool \
    svx/source/xoutdev/xtabbtmp \
    svx/source/xoutdev/xtabcolr \
    svx/source/xoutdev/xtabdash \
    svx/source/xoutdev/xtabgrdt \
    svx/source/xoutdev/xtabhtch \
    svx/source/xoutdev/xtable \
    svx/source/xoutdev/XPropertyEntry \
    svx/source/xoutdev/xtablend \
))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Library_add_exception_objects,svxcore,\
    svx/source/fmcomp/dbaexchange \
    svx/source/fmcomp/fmgridcl \
    svx/source/fmcomp/fmgridif \
    svx/source/fmcomp/gridcell \
    svx/source/fmcomp/gridcols \
    svx/source/fmcomp/gridctrl \
    svx/source/fmcomp/xmlexchg \
    svx/source/form/dataaccessdescriptor \
    svx/source/form/datalistener \
    svx/source/form/datanavi \
    svx/source/form/dbtoolsclient \
    svx/source/form/delayedevent \
    svx/source/form/fmcontrolbordermanager \
    svx/source/form/fmcontrollayout \
    svx/source/form/fmdmod \
    svx/source/form/fmdocumentclassification \
    svx/source/form/fmdpage \
    svx/source/form/fmexch \
    svx/source/form/fmexpl \
    svx/source/form/fmitems \
    svx/source/form/fmmodel \
    svx/source/form/fmobj \
    svx/source/form/fmpage \
    svx/source/form/fmpgeimp \
    svx/source/form/fmscriptingenv \
    svx/source/form/fmservs \
    svx/source/form/fmshell \
    svx/source/form/fmshimp \
    svx/source/form/fmtextcontroldialogs \
    svx/source/form/fmtextcontrolfeature \
    svx/source/form/fmtextcontrolshell \
    svx/source/form/fmtools \
    svx/source/form/fmundo \
    svx/source/form/fmview \
    svx/source/form/fmvwimp \
    svx/source/form/formcontrolfactory \
    svx/source/form/formcontroller \
    svx/source/form/formcontrolling \
    svx/source/form/formdispatchinterceptor \
    svx/source/form/formfeaturedispatcher \
    svx/source/form/formtoolbars \
    svx/source/form/legacyformcontroller \
    svx/source/form/navigatortree \
    svx/source/form/navigatortreemodel \
    svx/source/form/ParseContext \
    svx/source/form/sdbdatacolumn \
    svx/source/form/sqlparserclient \
    svx/source/form/stringlistresource \
    svx/source/form/typeconversionclient \
    svx/source/form/typemap \
    svx/source/form/xfm_addcondition \
))
endif

# the following source file can't be compiled with optimization by some compilers (crash or endless loop):
# Solaris Sparc with Sun compiler, gcc on MacOSX and Linux PPC
# the latter is currently not supported by gbuild and needs a fix here later
ifeq ($(OS),$(filter-out SOLARIS MACOSX,$(OS)))
$(eval $(call gb_Library_add_exception_objects,svxcore,\
    svx/source/customshapes/EnhancedCustomShapeFunctionParser \
))
else
$(eval $(call gb_Library_add_cxxobjects,svxcore,\
    svx/source/customshapes/EnhancedCustomShapeFunctionParser \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

$(eval $(call gb_SdiTarget_SdiTarget,svx/sdi/svxslots,svx/sdi/svx))

$(eval $(call gb_SdiTarget_set_include,svx/sdi/svxslots,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svx/inc \
    -I$(SRCDIR)/svx/sdi \
    -I$(SRCDIR)/sfx2/sdi \
))

# Runtime dependency for unit-tests
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,svxcore)) :| \
	$(call gb_AllLangResTarget_get_target,svx)

# vim: set noet sw=4 ts=4:
