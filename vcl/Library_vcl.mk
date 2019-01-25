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

$(eval $(call gb_Library_Library,vcl))

$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.common))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.macosx))
else ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.windows))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.android))
else ifeq ($(OS),iOS)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.ios))
else ifeq ($(DISABLE_GUI),TRUE)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.headless))
else
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.unx))
endif

$(eval $(call gb_Library_set_precompiled_header,vcl,$(SRCDIR)/vcl/inc/pch/precompiled_vcl))

$(eval $(call gb_Library_set_include,vcl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,vcl,\
    -DVCL_DLLIMPLEMENTATION \
    -DDLLIMPLEMENTATION_UITEST \
    -DCUI_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,cui))\" \
    -DDESKTOP_DETECTOR_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,desktop_detector))\" \
    -DTK_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,tk))\" \
    -DENABLE_MERGELIBS=$(if $(MERGELIBS),1,0) \
))

ifeq ($(SYSTEM_GLM),TRUE)
$(eval $(call gb_Library_add_defs,vcl,\
        -DGLM_ENABLE_EXPERIMENTAL \
))
endif

$(eval $(call gb_Library_use_sdk_api,vcl))

$(eval $(call gb_Library_use_custom_headers,vcl,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_externals,vcl,\
    libjpeg \
    libeot \
    libpng \
    $(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))

$(eval $(call gb_Library_use_libraries,vcl,\
    $(call gb_Helper_optional,BREAKPAD, \
        crashreport) \
    svl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    cppu \
    sal \
    salhelper \
    xmlreader \
))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Library_use_libraries,vcl,\
    jvmaccess \
))
endif

$(eval $(call gb_Library_use_externals,vcl,\
    boost_headers \
    gio \
    glm_headers \
    graphite \
    harfbuzz \
    icu_headers \
    icuuc \
    lcms2 \
    mdds_headers \
))

ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,vcl,\
     epoxy \
 ))
endif

$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/window/errinf \
    vcl/source/window/settings \
    vcl/source/window/paint \
    vcl/source/window/abstdlg \
    vcl/source/window/accel \
    vcl/source/window/accmgr \
    vcl/source/window/brdwin \
    vcl/source/window/accessibility \
    vcl/source/window/legacyaccessibility \
    vcl/source/window/clipping \
    vcl/source/window/stacking \
    vcl/source/window/debug \
    vcl/source/window/globalization \
    vcl/source/window/builder \
    vcl/source/window/commandevent \
    vcl/source/window/cursor \
    vcl/source/window/debugevent \
    vcl/source/window/decoview \
    vcl/source/window/dialog \
    vcl/source/window/dlgctrl \
    vcl/source/window/dndeventdispatcher \
    vcl/source/window/dndlistenercontainer \
    vcl/source/window/dockingarea \
    vcl/source/window/dockmgr \
    vcl/source/window/dockwin \
    vcl/source/window/event \
    vcl/source/window/floatwin \
    vcl/source/window/introwin \
    vcl/source/window/keycod \
    vcl/source/window/keyevent \
    vcl/source/window/layout \
    vcl/source/window/menu \
    vcl/source/window/menubarwindow \
    vcl/source/window/menufloatingwindow \
    vcl/source/window/menuitemlist \
    vcl/source/window/menuwindow \
    vcl/source/window/mnemonic \
    vcl/source/window/mnemonicengine \
    vcl/source/window/mouse \
    vcl/source/window/msgbox \
    vcl/source/window/popupmenuwindow \
    vcl/source/window/printdlg \
    vcl/source/window/scrwnd \
    vcl/source/window/seleng \
    vcl/source/window/split \
    vcl/source/window/splitwin \
    vcl/source/window/status \
    vcl/source/window/syschild \
    vcl/source/window/syswin \
    vcl/source/window/tabdlg \
    vcl/source/window/tabpage \
    vcl/source/window/taskpanelist \
    vcl/source/window/toolbox2 \
    vcl/source/window/toolbox \
    vcl/source/window/window2 \
    vcl/source/window/window3 \
    vcl/source/window/window \
    vcl/source/window/winproc \
    vcl/source/window/wrkwin \
    vcl/source/window/EnumContext \
    vcl/source/control/button \
    vcl/source/control/calendar \
    vcl/source/control/combobox \
    vcl/source/control/ctrl \
    vcl/source/control/edit \
    vcl/source/control/field2 \
    vcl/source/control/field \
    vcl/source/control/fixed \
    vcl/source/control/fixedhyper \
    vcl/source/control/fmtfield \
    vcl/source/control/imgctrl \
    vcl/source/control/listctrl \
    vcl/source/control/longcurr \
    vcl/source/control/imp_listbox \
    vcl/source/control/listbox \
    vcl/source/control/menubtn \
    vcl/source/control/notebookbar \
    vcl/source/control/quickselectionengine \
    vcl/source/control/prgsbar \
    vcl/source/control/scrbar \
    vcl/source/control/slider \
    vcl/source/control/spinbtn \
    vcl/source/control/spinfld \
    vcl/source/control/tabctrl \
    vcl/source/control/throbber \
    vcl/source/edit/vclmedit \
    vcl/source/edit/textdata \
    vcl/source/edit/textdoc \
    vcl/source/edit/texteng \
    vcl/source/edit/textundo \
    vcl/source/edit/textview \
    vcl/source/edit/txtattr \
    vcl/source/edit/xtextedt \
    vcl/source/toolkit/group \
    vcl/source/toolkit/morebtn \
    vcl/source/outdev/outdev \
    vcl/source/outdev/outdevstate \
    vcl/source/outdev/outdevstatestack \
    vcl/source/outdev/clipping \
    vcl/source/outdev/polygon \
    vcl/source/outdev/transparent \
    vcl/source/outdev/mask \
    vcl/source/outdev/bitmap \
    vcl/source/outdev/font \
    vcl/source/outdev/text \
    vcl/source/outdev/textline \
    vcl/source/outdev/pixel \
    vcl/source/outdev/rect \
    vcl/source/outdev/line \
    vcl/source/outdev/polyline \
    vcl/source/outdev/hatch \
    vcl/source/outdev/gradient \
    vcl/source/outdev/curvedshapes \
    vcl/source/outdev/wallpaper \
    vcl/source/outdev/vclreferencebase \
    vcl/source/outdev/nativecontrols \
    vcl/source/outdev/map \
    vcl/source/treelist/headbar \
    vcl/source/treelist/imap \
    vcl/source/treelist/imap2 \
    vcl/source/treelist/imap3 \
    vcl/source/treelist/inetimg \
    vcl/source/treelist/svtabbx \
    vcl/source/treelist/transfer \
    vcl/source/treelist/transfer2 \
    vcl/source/treelist/viewdataentry \
    vcl/source/treelist/treelist \
    vcl/source/treelist/treelistbox \
    vcl/source/treelist/treelistentry \
    vcl/source/treelist/svimpbox \
    vcl/source/treelist/svlbitm \
    vcl/source/treelist/uiobject \
    vcl/source/gdi/alpha \
    vcl/source/gdi/animate \
    vcl/source/gdi/bitmap3 \
    vcl/source/gdi/bitmapex \
    vcl/source/gdi/bmpacc2 \
    vcl/source/gdi/bmpacc3 \
    vcl/source/gdi/bmpacc \
    vcl/source/gdi/bmpfast \
    vcl/source/gdi/configsettings \
    vcl/source/gdi/cvtgrf \
    vcl/source/gdi/svmconverter \
    vcl/source/gdi/dibtools \
    vcl/source/gdi/embeddedfontshelper \
    vcl/source/gdi/FileDefinitionWidgetDraw \
    vcl/source/gdi/WidgetDefinitionReader \
    vcl/source/gdi/WidgetDefinition \
    vcl/source/gdi/extoutdevdata \
    vcl/source/gdi/gdimtf \
    vcl/source/gdi/mtfxmldump \
    vcl/source/gdi/gdimetafiletools \
    vcl/source/gdi/gfxlink \
    vcl/source/gdi/gradient \
    vcl/source/gdi/graph \
    vcl/source/gdi/graphictools \
    vcl/source/gdi/hatch \
    vcl/source/gdi/impanmvw \
    vcl/source/gdi/impglyphitem \
    vcl/source/gdi/impgraph \
    vcl/source/gdi/impvect \
    vcl/source/gdi/jobset \
    vcl/source/gdi/lineinfo \
    vcl/source/gdi/mapmod \
    vcl/source/gdi/metaact \
    vcl/source/gdi/octree \
    vcl/source/gdi/oldprintadaptor \
    vcl/source/gdi/pdfbuildin_fonts \
    vcl/source/gdi/pdfextoutdevdata \
    vcl/source/gdi/pdffontcache \
    vcl/source/gdi/pdfwriter \
    vcl/source/gdi/pdfwriter_impl2 \
    vcl/source/gdi/pdfwriter_impl \
    vcl/source/gdi/print2 \
    vcl/source/gdi/print3 \
    vcl/source/gdi/print \
    vcl/source/gdi/regband \
    vcl/source/gdi/region \
    vcl/source/gdi/regionband \
    vcl/source/gdi/salgdilayout \
    vcl/source/gdi/salgdiimpl \
    vcl/source/gdi/sallayout \
    vcl/source/gdi/salmisc \
    vcl/source/gdi/salnativewidgets-none \
    vcl/source/gdi/vectorgraphicdata \
    vcl/source/gdi/textlayout \
    vcl/source/gdi/virdev \
    vcl/source/gdi/wall \
    vcl/source/gdi/scrptrun \
    vcl/source/gdi/CommonSalLayout \
    vcl/source/graphic/GraphicLoader \
    vcl/source/graphic/GraphicObject \
    vcl/source/graphic/GraphicObject2 \
    vcl/source/graphic/grfattr \
    vcl/source/graphic/Manager \
    vcl/source/graphic/UnoGraphic \
    vcl/source/graphic/UnoGraphicDescriptor \
    vcl/source/graphic/UnoGraphicObject \
    vcl/source/graphic/UnoGraphicProvider \
    vcl/source/graphic/UnoGraphicTransformer \
    vcl/source/bitmap/bitmap \
    vcl/source/bitmap/bitmapfilter \
    vcl/source/bitmap/BitmapAlphaClampFilter \
    vcl/source/bitmap/BitmapMonochromeFilter \
    vcl/source/bitmap/BitmapSmoothenFilter \
    vcl/source/bitmap/BitmapLightenFilter \
    vcl/source/bitmap/BitmapDisabledImageFilter \
    vcl/source/bitmap/BitmapColorizeFilter \
    vcl/source/bitmap/bitmappaint \
    vcl/source/bitmap/BitmapGaussianSeparableBlurFilter \
    vcl/source/bitmap/BitmapSobelGreyFilter \
    vcl/source/bitmap/BitmapSolarizeFilter \
    vcl/source/bitmap/BitmapSepiaFilter \
    vcl/source/bitmap/BitmapMosaicFilter \
    vcl/source/bitmap/BitmapEmbossGreyFilter \
    vcl/source/bitmap/BitmapPopArtFilter \
    vcl/source/bitmap/BitmapDuoToneFilter \
    vcl/source/bitmap/BitmapConvolutionMatrixFilter \
    vcl/source/bitmap/BitmapMedianFilter \
    vcl/source/bitmap/BitmapInterpolateScaleFilter \
    vcl/source/bitmap/BitmapSeparableUnsharpenFilter \
    vcl/source/bitmap/BitmapFastScaleFilter \
    vcl/source/bitmap/BitmapScaleSuperFilter \
    vcl/source/bitmap/BitmapScaleConvolutionFilter \
    vcl/source/bitmap/BitmapSymmetryCheck \
    vcl/source/bitmap/BitmapColorQuantizationFilter \
    vcl/source/bitmap/BitmapSimpleColorQuantizationFilter \
    vcl/source/bitmap/BitmapTools \
    vcl/source/bitmap/checksum \
    vcl/source/image/Image \
    vcl/source/image/ImageTree \
    vcl/source/image/ImageRepository \
    vcl/source/image/ImplImage \
    vcl/source/image/ImplImageTree \
    vcl/source/helper/canvasbitmap \
    vcl/source/helper/canvastools \
    vcl/source/helper/commandinfoprovider \
    vcl/source/helper/displayconnectiondispatch \
    vcl/source/helper/errcode \
    vcl/source/helper/evntpost \
    vcl/source/helper/lazydelete \
    vcl/source/helper/strhelper \
    vcl/source/helper/svtaccessiblefactory \
    vcl/source/helper/threadex \
    vcl/source/app/brand \
    vcl/source/app/customweld \
    vcl/source/app/dbggui \
    vcl/source/app/dndhelp \
    vcl/source/app/help \
    vcl/source/app/i18nhelp \
    vcl/source/app/idle \
    vcl/source/app/salusereventlist \
    vcl/source/app/salvtables \
    vcl/source/app/scheduler \
    vcl/source/app/session \
    vcl/source/app/settings \
    vcl/source/app/IconThemeInfo \
    vcl/source/app/IconThemeScanner \
    vcl/source/app/IconThemeSelector \
    vcl/source/app/ITiledRenderable \
    vcl/source/app/sound \
    vcl/source/app/stdtext \
    vcl/source/app/svapp \
    vcl/source/app/svdata \
    vcl/source/app/svmain \
    vcl/source/app/timer \
    vcl/source/app/unohelp2 \
    vcl/source/app/unohelp \
    vcl/source/app/vclevent \
    vcl/source/app/winscheduler \
    vcl/source/components/dtranscomp \
    vcl/source/components/factory \
    vcl/source/components/fontident \
    vcl/source/filter/FilterConfigCache \
    vcl/source/filter/FilterConfigItem \
    vcl/source/filter/graphicfilter \
    vcl/source/filter/graphicfilter2 \
    vcl/source/filter/GraphicNativeTransform \
    vcl/source/filter/GraphicNativeMetadata \
    vcl/source/filter/igif/decode \
    vcl/source/filter/igif/gifread \
    vcl/source/filter/ipdf/pdfread \
    vcl/source/filter/ipdf/pdfdocument \
    vcl/source/filter/ixbm/xbmread \
    vcl/source/filter/ixpm/xpmread \
    vcl/source/filter/jpeg/Exif \
    vcl/source/filter/jpeg/jpeg \
    vcl/source/filter/jpeg/jpegc \
    vcl/source/filter/jpeg/JpegReader \
    vcl/source/filter/jpeg/JpegWriter \
    vcl/source/filter/jpeg/JpegTransform \
    vcl/source/filter/wmf/emfwr \
    vcl/source/filter/wmf/wmf \
    vcl/source/filter/wmf/wmfexternal \
    vcl/source/filter/wmf/wmfwr \
    vcl/source/filter/png/PngImageReader \
    vcl/source/filter/png/pngread \
    vcl/source/filter/png/pngwrite \
    vcl/source/font/Feature \
    vcl/source/font/FeatureCollector \
    vcl/source/font/FeatureParser \
    vcl/source/font/OpenTypeFeatureDefinitonList \
    vcl/source/font/PhysicalFontCollection \
    vcl/source/font/PhysicalFontFace \
    vcl/source/font/PhysicalFontFamily \
    vcl/source/font/fontattributes \
    vcl/source/font/fontselect \
    vcl/source/font/fontinstance \
    vcl/source/font/fontcache \
    vcl/source/font/fontcharmap \
    vcl/source/font/fontmetric \
    vcl/source/font/font \
    vcl/source/fontsubset/cff \
    vcl/source/fontsubset/fontsubset \
    vcl/source/fontsubset/list \
    vcl/source/fontsubset/sft \
    vcl/source/fontsubset/ttcr \
    vcl/source/fontsubset/xlat \
    vcl/source/uitest/logger \
    vcl/source/uitest/uiobject \
    vcl/source/uitest/uitest \
    vcl/source/uitest/uno/uiobject_uno \
    vcl/source/uitest/uno/uitest_uno \
    vcl/backendtest/outputdevice/bitmap \
    vcl/backendtest/outputdevice/common \
    vcl/backendtest/outputdevice/gradient \
    vcl/backendtest/outputdevice/line \
    vcl/backendtest/outputdevice/outputdevice \
    vcl/backendtest/outputdevice/pixel \
    vcl/backendtest/outputdevice/polygon \
    vcl/backendtest/outputdevice/polypolygon \
    vcl/backendtest/outputdevice/polyline \
    vcl/backendtest/outputdevice/rectangle \
))

$(eval $(call gb_Library_add_cobjects,vcl,\
    vcl/source/filter/jpeg/transupp \
))

vcl_quartz_code= \
    vcl/quartz/salbmp \
    vcl/quartz/utils \
    vcl/quartz/salgdicommon \
    vcl/quartz/salvd \

vcl_coretext_code= \
    vcl/quartz/ctfonts \
    vcl/quartz/salgdi \

vcl_headless_code= \
    vcl/headless/svpframe \
    $(if $(filter-out iOS,$(OS)), \
        vcl/headless/svpbmp \
        vcl/headless/svpgdi \
        vcl/headless/svpdata \
        vcl/headless/CustomWidgetDraw) \
    vcl/headless/svpdummies \
    vcl/headless/svpinst \
    vcl/headless/svpvd \
    vcl/unx/generic/app/gendisp \
    vcl/unx/generic/app/geninst \
    vcl/unx/generic/app/gensys \

vcl_headless_freetype_code=\
    vcl/headless/svpprn \
    vcl/headless/svptext \
    vcl/unx/generic/app/gendata \
    vcl/unx/generic/gdi/cairotextrender \
    vcl/unx/generic/glyphs/freetype_glyphcache \
    vcl/unx/generic/glyphs/glyphcache \
    vcl/unx/generic/fontmanager/fontsubst \
    vcl/unx/generic/fontmanager/fontconfig \
    vcl/unx/generic/fontmanager/fontmanager \
    vcl/unx/generic/fontmanager/helper \
    vcl/headless/svpcairotextrender \
    vcl/unx/generic/print/bitmap_gfx \
    vcl/unx/generic/print/common_gfx \
    vcl/unx/generic/print/glyphset \
    vcl/unx/generic/print/printerjob \
    vcl/unx/generic/print/psputil \
    vcl/unx/generic/print/genpspgraphics \
    vcl/unx/generic/print/genprnpsp \
    vcl/unx/generic/print/prtsetup \
    vcl/unx/generic/print/text_gfx \

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/app/salplug \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/unx/generic/gdi/nativewindowhandleprovider \
    vcl/unx/generic/window/screensaverinhibitor \
    vcl/unx/generic/printer/cpdmgr \
    $(if $(filter TRUE,$(ENABLE_CUPS)),\
        vcl/unx/generic/printer/cupsmgr \
        vcl/unx/generic/printer/printerinfomanager \
        , \
        vcl/null/printerinfomanager \
    ) \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_externals,vcl,\
    cairo \
    cups \
    dbus \
    fontconfig \
    freetype \
    valgrind \
))
endif

ifeq ($(DISABLE_GUI),TRUE)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/null/printerinfomanager \
    vcl/headless/headlessinst \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_externals,vcl,\
    cairo \
    freetype \
    fontconfig \
))
ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Library_add_libs,vcl,\
    -lpthread \
))
endif

else # ! DISABLE_GUI

$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/opengl/DeviceInfo \
    vcl/opengl/gdiimpl \
    vcl/opengl/salbmp \
    vcl/opengl/scale \
    vcl/opengl/framebuffer \
    vcl/opengl/program \
    vcl/opengl/texture \
    vcl/opengl/FixedTextureAtlas \
    vcl/opengl/PackedTextureAtlas \
    vcl/opengl/RenderList \
    vcl/opengl/LineRenderUtils \
    vcl/source/opengl/OpenGLContext \
    vcl/source/opengl/OpenGLHelper \
 ))
ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Library_add_libs,vcl,\
    -lm $(DLOPEN_LIBS) \
    -lpthread \
    -lX11 \
    -lXext \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/opengl/x11/X11DeviceInfo \
))
endif
endif # ! DISABLE_GUI


ifeq ($(OS),HAIKU)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/null/printerinfomanager \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_add_libs,vcl,\
    -lbe \
))

$(eval $(call gb_Library_add_exception_objects,vcl, \
    $(if $(or $(ENABLE_QT5),$(ENABLE_KDE5)),vcl/source/app/salplug) \
))

$(eval $(call gb_Library_use_externals,vcl,\
    cairo \
    fontconfig \
    freetype \
    expat \
))
endif


ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_add_libs,vcl,\
    -llog \
    -landroid \
    -llo-bootstrap \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/null/printerinfomanager \
    vcl/android/androidinst \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_externals,vcl,\
    cairo \
    fontconfig \
    freetype \
    expat \
))
endif


ifeq ($(OS),iOS)
$(eval $(call gb_Library_add_cxxflags,vcl,\
    $(gb_OBJCXXFLAGS) \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/ios/iosinst \
    vcl/ios/dummies \
    $(vcl_coretext_code) \
    $(vcl_quartz_code) \
    $(vcl_headless_code) \
))
$(eval $(call gb_Library_use_system_darwin_frameworks,vcl,\
    UIKit \
    CoreFoundation \
))
endif


ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,vcl,\
    Cocoa \
    CoreFoundation \
))

$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/app/salplug \
))
endif


ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/opengl/win/WinDeviceInfo \
    vcl/opengl/win/blocklist_parser \
    vcl/source/app/salplug \
))

$(eval $(call gb_Library_use_system_win32_libs,vcl,\
    ole32 \
    setupapi \
    version \
))

$(eval $(call gb_Library_add_nativeres,vcl,vcl/salsrc))

# HACK: dependency on icon themes so running unit tests don't
# prevent delivering these by having open file handles on WNT
$(eval $(call gb_Library_use_package,vcl,postprocess_images))
endif

# vim: set noet sw=4 ts=4:
