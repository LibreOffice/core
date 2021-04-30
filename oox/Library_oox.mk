# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,oox))

$(eval $(call gb_Library_set_precompiled_header,oox,oox/inc/pch/precompiled_oox))

$(eval $(call gb_Library_use_custom_headers,oox,oox/generated))

$(eval $(call gb_Library_set_include,oox,\
    $$(INCLUDE) \
    -I$(SRCDIR)/oox/inc \
))

ifeq ($(COM)-$(OS)-$(CPUNAME),GCC-LINUX-IA64)
# at least Debian Linux ia64 fails at compile time on
# link libooxlo.so which is apparently too large
# for the toolchain to handle with default optimization
$(eval $(call gb_Library_add_cxxflags,oox,\
    -Os \
))
endif

$(eval $(call gb_Library_add_defs,oox,\
    -DOOX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,oox))

$(eval $(call gb_Library_use_api,oox,\
    oovbaapi \
))

$(eval $(call gb_Library_use_libraries,oox,\
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    expwrap \
    drawinglayer \
    msfilter \
    sal \
    i18nlangtag \
    sax \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    sot \
    tl \
    utl \
    vcl \
    xo \
    xmlscript \
))

$(eval $(call gb_Library_use_externals,oox,\
	boost_headers \
))

ifeq ($(TLS),OPENSSL)
$(eval $(call gb_Library_use_externals,oox,\
	openssl \
	openssl_headers \
))
else
ifeq ($(TLS),NSS)
$(eval $(call gb_Library_use_externals,oox,\
       plc4 \
       nss3 \
))
endif
endif

$(eval $(call gb_Library_set_componentfile,oox,oox/util/oox))

# WASM_CHART change
ifeq (,$(ENABLE_WASM_STRIP_CHART))
$(eval $(call gb_Library_add_exception_objects,oox,\
    oox/source/export/chartexport \
    oox/source/drawingml/chart/axiscontext \
    oox/source/drawingml/chart/axisconverter \
    oox/source/drawingml/chart/axismodel \
    oox/source/drawingml/chart/chartcontextbase \
    oox/source/drawingml/chart/chartconverter \
    oox/source/drawingml/chart/chartdrawingfragment \
    oox/source/drawingml/chart/chartspaceconverter \
    oox/source/drawingml/chart/chartspacefragment \
    oox/source/drawingml/chart/chartspacemodel \
    oox/source/drawingml/chart/converterbase \
    oox/source/drawingml/chart/datasourcecontext \
    oox/source/drawingml/chart/datasourceconverter \
    oox/source/drawingml/chart/datasourcemodel \
    oox/source/drawingml/chart/modelbase \
    oox/source/drawingml/chart/objectformatter \
    oox/source/drawingml/chart/plotareacontext \
    oox/source/drawingml/chart/plotareaconverter \
    oox/source/drawingml/chart/plotareamodel \
    oox/source/drawingml/chart/seriescontext \
    oox/source/drawingml/chart/seriesconverter \
    oox/source/drawingml/chart/seriesmodel \
    oox/source/drawingml/chart/titlecontext \
    oox/source/drawingml/chart/titleconverter \
    oox/source/drawingml/chart/titlemodel \
    oox/source/drawingml/chart/typegroupcontext \
    oox/source/drawingml/chart/typegroupconverter \
    oox/source/drawingml/chart/typegroupmodel \
))
endif

$(eval $(call gb_Library_add_exception_objects,oox,\
    oox/source/core/binarycodec \
    oox/source/core/contexthandler2 \
    oox/source/core/contexthandler \
    oox/source/core/fastparser \
    oox/source/core/fasttokenhandler \
    oox/source/core/filterbase \
    oox/source/core/filterdetect \
    oox/source/core/fragmenthandler2 \
    oox/source/core/fragmenthandler \
    oox/source/core/recordparser \
    oox/source/core/relations \
    oox/source/core/relationshandler \
    oox/source/core/xmlfilterbase \
    oox/source/crypto/AgileEngine \
    oox/source/crypto/CryptTools \
    oox/source/crypto/DocumentEncryption \
    oox/source/crypto/DocumentDecryption \
    oox/source/crypto/Standard2007Engine \
    oox/source/crypto/StrongEncryptionDataSpace \
    oox/source/docprop/docprophandler \
    oox/source/docprop/ooxmldocpropimport \
    oox/source/drawingml/clrschemecontext \
    oox/source/drawingml/clrscheme \
    oox/source/drawingml/colorchoicecontext \
    oox/source/drawingml/connectorshapecontext \
    oox/source/drawingml/customshapegeometry \
    oox/source/drawingml/customshapepresetdata \
    oox/source/drawingml/customshapeproperties \
    oox/source/drawingml/diagram/constraintlistcontext \
    oox/source/drawingml/diagram/datamodel \
    oox/source/drawingml/diagram/datamodelcontext \
    oox/source/drawingml/diagram/diagram \
    oox/source/drawingml/diagram/diagramdefinitioncontext \
    oox/source/drawingml/diagram/diagramfragmenthandler \
    oox/source/drawingml/diagram/diagramlayoutatoms \
    oox/source/drawingml/diagram/layoutatomvisitorbase \
    oox/source/drawingml/diagram/layoutatomvisitors \
    oox/source/drawingml/diagram/layoutnodecontext \
    oox/source/drawingml/diagram/rulelistcontext \
    oox/source/drawingml/drawingmltypes \
    oox/source/drawingml/effectproperties \
    oox/source/drawingml/effectpropertiescontext \
    oox/source/drawingml/embeddedwavaudiofile \
    oox/source/drawingml/fillproperties \
    oox/source/drawingml/misccontexts \
    oox/source/drawingml/graphicshapecontext \
    oox/source/drawingml/guidcontext \
    oox/source/drawingml/hyperlinkcontext \
    oox/source/drawingml/linepropertiescontext \
    oox/source/drawingml/lineproperties \
    oox/source/drawingml/objectdefaultcontext \
    oox/source/drawingml/presetgeometrynames \
    oox/source/drawingml/scene3dcontext \
    oox/source/drawingml/shapecontext \
    oox/source/drawingml/shape \
    oox/source/drawingml/shape3dproperties \
    oox/source/drawingml/shapegroupcontext \
    oox/source/drawingml/shapepropertiescontext \
    oox/source/drawingml/shapepropertymap \
    oox/source/drawingml/shapestylecontext \
    oox/source/drawingml/spdefcontext \
    oox/source/drawingml/table/tablebackgroundstylecontext \
    oox/source/drawingml/table/tablecellcontext \
    oox/source/drawingml/table/tablecell \
    oox/source/drawingml/table/tablecontext \
    oox/source/drawingml/table/tablepartstylecontext \
    oox/source/drawingml/table/tableproperties \
    oox/source/drawingml/table/tablerowcontext \
    oox/source/drawingml/table/tablerow \
    oox/source/drawingml/table/tablestylecellstylecontext \
    oox/source/drawingml/table/tablestylecontext \
    oox/source/drawingml/table/tablestyle \
    oox/source/drawingml/table/tablestylelist \
    oox/source/drawingml/table/tablestylelistfragmenthandler \
    oox/source/drawingml/table/tablestylepart \
    oox/source/drawingml/table/tablestyletextstylecontext \
    oox/source/drawingml/textbodycontext \
    oox/source/drawingml/textbody \
    oox/source/drawingml/textbodypropertiescontext \
    oox/source/drawingml/textbodyproperties \
    oox/source/drawingml/textcharacterpropertiescontext \
    oox/source/drawingml/textcharacterproperties \
    oox/source/drawingml/texteffectscontext \
    oox/source/drawingml/textfieldcontext \
    oox/source/drawingml/textfield \
    oox/source/drawingml/textfont \
    oox/source/drawingml/textliststylecontext \
    oox/source/drawingml/textliststyle \
    oox/source/drawingml/textparagraph \
    oox/source/drawingml/textparagraphpropertiescontext \
    oox/source/drawingml/textparagraphproperties \
    oox/source/drawingml/textrun \
    oox/source/drawingml/textspacingcontext \
    oox/source/drawingml/texttabstoplistcontext \
    oox/source/drawingml/theme \
    oox/source/drawingml/themeelementscontext \
    oox/source/drawingml/themefragmenthandler \
    oox/source/drawingml/ThemeOverrideFragmentHandler \
    oox/source/drawingml/transform2dcontext \
    oox/source/dump/dffdumper \
    oox/source/dump/dumperbase \
    oox/source/dump/oledumper \
    oox/source/dump/pptxdumper \
    oox/source/export/ColorPropertySet \
    oox/source/export/drawingml \
    oox/source/export/DMLPresetShapeExport \
    oox/source/export/shapes \
    oox/source/export/vmlexport \
    oox/source/helper/attributelist \
    oox/source/helper/binaryinputstream \
    oox/source/helper/binaryoutputstream \
    oox/source/helper/binarystreambase \
    oox/source/helper/containerhelper \
    oox/source/helper/graphichelper \
    oox/source/helper/grabbagstack \
    oox/source/helper/modelobjecthelper \
    oox/source/helper/ooxresid \
    oox/source/helper/progressbar \
    oox/source/helper/propertymap \
    oox/source/helper/propertyset \
    oox/source/helper/storagebase \
    oox/source/helper/textinputstream \
    oox/source/helper/zipstorage \
    oox/source/mathml/export \
    oox/source/mathml/import \
    oox/source/mathml/importutils \
    oox/source/ole/axbinaryreader \
    oox/source/ole/axbinarywriter \
    oox/source/ole/axfontdata \
    oox/source/ole/axcontrol \
    oox/source/ole/axcontrolfragment \
    oox/source/ole/olehelper \
    oox/source/ole/oleobjecthelper \
    oox/source/ole/olestorage \
    oox/source/ole/vbacontrol \
    oox/source/ole/vbaexport \
    oox/source/ole/vbahelper \
    oox/source/ole/vbainputstream \
    oox/source/ole/vbamodule \
    oox/source/ole/vbaproject \
    oox/source/ppt/animationspersist \
    oox/source/ppt/animationtypes \
    oox/source/ppt/animvariantcontext \
    oox/source/ppt/backgroundproperties \
    oox/source/ppt/buildlistcontext \
    oox/source/ppt/comments \
    oox/source/ppt/commonbehaviorcontext \
    oox/source/ppt/commontimenodecontext \
    oox/source/ppt/conditioncontext \
    oox/source/ppt/customshowlistcontext \
    oox/source/ppt/headerfootercontext \
    oox/source/ppt/layoutfragmenthandler \
    oox/source/ppt/pptfilterhelpers \
    oox/source/ppt/pptgraphicshapecontext \
    oox/source/ppt/pptimport \
    oox/source/ppt/pptshapecontext \
    oox/source/ppt/pptshape \
    oox/source/ppt/pptshapegroupcontext \
    oox/source/ppt/pptshapepropertiescontext \
    oox/source/ppt/presentationfragmenthandler \
    oox/source/ppt/presPropsfragmenthandler \
    oox/source/ppt/slidefragmenthandler \
    oox/source/ppt/slidemastertextstylescontext \
    oox/source/ppt/slidepersist \
    oox/source/ppt/slidetimingcontext \
    oox/source/ppt/slidetransitioncontext \
    oox/source/ppt/slidetransition \
    oox/source/ppt/soundactioncontext \
    oox/source/ppt/timeanimvaluecontext \
    oox/source/ppt/timenode \
    oox/source/ppt/timenodelistcontext \
    oox/source/ppt/timetargetelementcontext \
    oox/source/ppt/extdrawingfragmenthandler \
    oox/source/shape/LockedCanvasContext \
    oox/source/shape/ShapeContextHandler \
    oox/source/shape/ShapeDrawingFragmentHandler \
    oox/source/shape/ShapeFilterBase \
    oox/source/shape/WpgContext \
    oox/source/shape/WpsContext \
    oox/source/token/namespacemap \
    oox/source/token/propertynames \
    oox/source/token/tokenmap \
    oox/source/token/relationship \
    oox/source/vml/vmldrawing \
    oox/source/vml/vmldrawingfragment \
    oox/source/vml/vmlformatting \
    oox/source/vml/vmlinputstream \
    oox/source/vml/vmlshapecontainer \
    oox/source/vml/vmlshapecontext \
    oox/source/vml/vmlshape \
    oox/source/vml/vmltextboxcontext \
    oox/source/vml/vmltextbox \
))

ifeq ($(OS),iOS)
# Either a compiler bug in Xcode 5.1.1 or some hard-to-spot undefined
# behaviour in the source code... Compiling this source file with
# optimization causes some Smart Art images to end up with completely
# wrong colour, some even totally black.
$(eval $(call gb_Library_add_exception_objects,oox,\
    oox/source/drawingml/color \
    , $(gb_COMPILERNOOPTFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,oox,\
    oox/source/drawingml/color \
))
endif

ifeq ($(CPUNAME),M68K)
$(eval $(call gb_Library_add_cxxflags,oox,\
    -mlong-jump-table-offsets \
))
endif

# vim: set noet sw=4 ts=4:
