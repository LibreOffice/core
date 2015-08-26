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

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.macosx))
else ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.windows))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.android))
else ifeq ($(OS),IOS)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.ios))
else ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.headless))
else
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.unx))
endif

$(eval $(call gb_Library_set_precompiled_header,vcl,$(SRCDIR)/vcl/inc/pch/precompiled_vcl))

$(eval $(call gb_Library_use_custom_headers,vcl,officecfg/registry vcl/generic/fontmanager))

$(eval $(call gb_Library_set_include,vcl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
	$(if $(filter WNTGCC,$(OS)$(COM)),-I$(MINGW_SYSROOT)/include/gdiplus) \
))

$(eval $(call gb_Library_add_defs,vcl,\
    -DVCL_DLLIMPLEMENTATION \
	-DCUI_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,cui))\" \
	-DDESKTOP_DETECTOR_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,desktop_detector))\" \
	-DTK_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,tk))\" \
	-DENABLE_MERGELIBS=$(if $(MERGELIBS),1,0) \
))

$(eval $(call gb_Library_use_sdk_api,vcl))

$(eval $(call gb_Library_use_custom_headers,vcl,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_externals,vcl,\
	$(if $(filter LINUX MACOSX,$(OS)), \
		curl) \
	jpeg \
	$(if $(filter-out WNT,$(OS)), \
		nss3 \
		plc4) \
	libeot \
))

$(eval $(call gb_Library_use_libraries,vcl,\
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
    cppu \
    sal \
    salhelper \
    xmlreader \
	$(gb_UWINAPI) \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_libs,vcl,\
    -framework IOKit \
    -F/System/Library/PrivateFrameworks \
    -framework CoreUI \
    -lobjc \
))
endif
ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_add_cxxflags,vcl,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/osx/OpenGLWrapper \
))

endif

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Library_use_libraries,vcl,\
    jvmaccess \
))
endif

$(eval $(call gb_Library_use_externals,vcl,\
	boost_headers \
	gio \
	glew \
	glm_headers \
	harfbuzz \
	icu_headers \
	icuuc \
	lcms2 \
	mdds_headers \
	mesa_headers \
))

$(eval $(call gb_Library_add_exception_objects,vcl,\
	vcl/opengl/DeviceInfo \
	vcl/opengl/gdiimpl \
	vcl/opengl/salbmp \
	vcl/opengl/scale \
	vcl/opengl/framebuffer \
	vcl/opengl/program \
	vcl/opengl/texture \
	vcl/opengl/FixedTextureAtlas \
    vcl/source/opengl/OpenGLContext \
    vcl/source/opengl/OpenGLHelper \
    vcl/source/window/cairo_cairo \
    vcl/source/window/openglwin \
    vcl/source/window/settings \
    vcl/source/window/paint \
    vcl/source/window/resource \
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
    vcl/source/window/btndlg \
    vcl/source/window/builder \
    vcl/source/window/cmdevt \
    vcl/source/window/cursor \
    vcl/source/window/debugevent \
    vcl/source/window/decoview \
    vcl/source/window/dialog \
    vcl/source/window/dlgctrl \
    vcl/source/window/dndevdis \
    vcl/source/window/dndlcon \
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
    vcl/source/window/mouseevent \
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
    vcl/source/control/button \
    vcl/source/control/combobox \
    vcl/source/control/ctrl \
    vcl/source/control/edit \
    vcl/source/control/field2 \
    vcl/source/control/field \
    vcl/source/control/fixed \
    vcl/source/control/fixedhyper \
    vcl/source/control/group \
    vcl/source/control/ilstbox \
    vcl/source/control/imgctrl \
    vcl/source/control/longcurr \
    vcl/source/control/lstbox \
    vcl/source/control/menubtn \
    vcl/source/control/morebtn \
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
    vcl/source/outdev/rendersettings \
    vcl/source/outdev/line \
    vcl/source/outdev/polyline \
    vcl/source/outdev/hatch \
    vcl/source/outdev/gradient \
    vcl/source/outdev/curvedshapes \
    vcl/source/outdev/wallpaper \
    vcl/source/outdev/nativecontrols \
    vcl/source/outdev/map \
    vcl/source/gdi/alpha \
    vcl/source/gdi/animate \
    vcl/source/gdi/base14 \
    vcl/source/gdi/bitmap3 \
    vcl/source/gdi/bitmap4 \
    vcl/source/gdi/bitmap \
    vcl/source/gdi/bitmapex \
    vcl/source/gdi/bmpacc2 \
    vcl/source/gdi/bmpacc3 \
    vcl/source/gdi/bmpacc \
    vcl/source/gdi/bmpfast \
    vcl/source/gdi/configsettings \
    vcl/source/gdi/cvtgrf \
    vcl/source/gdi/cvtsvm \
    vcl/source/gdi/dibtools \
    vcl/source/gdi/embeddedfontshelper \
    vcl/source/gdi/extoutdevdata \
    vcl/source/gdi/font \
    vcl/source/gdi/gdimtf \
    vcl/source/gdi/gdimetafiletools \
    vcl/source/gdi/gfxlink \
    vcl/source/gdi/gradient \
    vcl/source/gdi/graph \
    vcl/source/gdi/graphictools \
    vcl/source/gdi/hatch \
    vcl/source/gdi/image \
    vcl/source/gdi/imagerepository \
    vcl/source/gdi/impanmvw \
    vcl/source/gdi/impbmp \
    vcl/source/gdi/impfont \
    vcl/source/gdi/impgraph \
    vcl/source/gdi/impimage \
    vcl/source/gdi/impimagetree \
    vcl/source/gdi/impvect \
    vcl/source/gdi/jobset \
    vcl/source/gdi/lineinfo \
    vcl/source/gdi/mapmod \
    vcl/source/gdi/metaact \
    vcl/source/gdi/metric \
    vcl/source/gdi/octree \
    vcl/source/gdi/oldprintadaptor \
    vcl/source/gdi/pdfextoutdevdata \
    vcl/source/gdi/pdffontcache \
    vcl/source/gdi/pdfwriter \
    vcl/source/gdi/pdfwriter_impl2 \
    vcl/source/gdi/pdfwriter_impl \
    vcl/source/gdi/pngread \
    vcl/source/gdi/pngwrite \
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
    vcl/source/gdi/svgdata \
    vcl/source/gdi/textlayout \
    vcl/source/gdi/virdev \
    vcl/source/gdi/wall \
    vcl/source/bitmap/bitmapfilter \
    vcl/source/bitmap/bitmapscalesuper \
    vcl/source/bitmap/BitmapSymmetryCheck \
    vcl/source/bitmap/BitmapFilterStackBlur \
    vcl/source/bitmap/checksum \
    vcl/source/helper/canvasbitmap \
    vcl/source/helper/canvastools \
    vcl/source/helper/evntpost \
    vcl/source/helper/lazydelete \
    vcl/source/helper/strhelper \
    vcl/source/helper/threadex \
    vcl/source/helper/xconnection \
    vcl/source/app/brand \
    vcl/source/app/dbggui \
    vcl/source/app/dndhelp \
    vcl/source/app/help \
    vcl/source/app/i18nhelp \
	vcl/source/app/idle \
    vcl/source/app/idlemgr \
    vcl/source/app/salvtables \
	vcl/source/app/scheduler \
    vcl/source/app/session \
    vcl/source/app/settings \
    vcl/source/app/IconThemeInfo \
    vcl/source/app/IconThemeScanner \
    vcl/source/app/IconThemeSelector \
    vcl/source/app/sound \
    vcl/source/app/stdtext \
    vcl/source/app/svapp \
    vcl/source/app/svdata \
    vcl/source/app/svmain \
    vcl/source/app/svmainhook \
    vcl/source/app/timer \
    vcl/source/app/unohelp2 \
    vcl/source/app/unohelp \
    vcl/source/app/vclevent \
    vcl/source/components/dtranscomp \
    vcl/source/components/factory \
    vcl/source/components/fontident \
    vcl/source/filter/FilterConfigCache \
    vcl/source/filter/FilterConfigItem \
    vcl/source/filter/graphicfilter \
    vcl/source/filter/graphicfilter2 \
    vcl/source/filter/GraphicNativeTransform \
    vcl/source/filter/GraphicNativeMetadata \
    vcl/source/filter/sgfbram \
    vcl/source/filter/sgvmain \
    vcl/source/filter/sgvspln \
    vcl/source/filter/sgvtext \
    vcl/source/filter/igif/decode \
    vcl/source/filter/igif/gifread \
    vcl/source/filter/ixbm/xbmread \
    vcl/source/filter/ixpm/xpmread \
    vcl/source/filter/jpeg/Exif \
    vcl/source/filter/jpeg/jpeg \
    vcl/source/filter/jpeg/jpegc \
    vcl/source/filter/jpeg/JpegReader \
    vcl/source/filter/jpeg/JpegWriter \
    vcl/source/filter/jpeg/JpegTransform \
    vcl/source/filter/wmf/emfwr \
    vcl/source/filter/wmf/enhwmf \
    vcl/source/filter/wmf/winmtf \
    vcl/source/filter/wmf/winwmf \
    vcl/source/filter/wmf/wmf \
    vcl/source/filter/wmf/wmfwr \
    vcl/source/font/PhysicalFontCollection \
    vcl/source/font/PhysicalFontFace \
    vcl/source/font/PhysicalFontFamily \
    vcl/source/fontsubset/cff \
    vcl/source/fontsubset/fontsubset \
    vcl/source/fontsubset/gsub \
    vcl/source/fontsubset/list \
    vcl/source/fontsubset/sft \
    vcl/source/fontsubset/ttcr \
    vcl/source/fontsubset/xlat \
))

$(eval $(call gb_Library_add_cobjects,vcl,\
    vcl/source/filter/jpeg/transupp \
))

# optional parts

## handle Graphite
ifeq ($(ENABLE_GRAPHITE),TRUE)
# add graphite sources for all platforms
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/glyphs/graphite_features \
    vcl/source/glyphs/graphite_layout \
))

# handle X11 platforms, which have additional files and possibly system graphite
ifneq (,$(or $(filter unx,$(GUIBASE)),$(ENABLE_HEADLESS)))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/generic/glyphs/graphite_serverfont \
))
endif

$(eval $(call gb_Library_use_external,vcl,graphite))

endif

vcl_quartz_code= \
    vcl/quartz/salbmp \
    vcl/quartz/utils \
    vcl/quartz/salgdicommon \
    vcl/quartz/salvd \

vcl_coretext_code= \
    vcl/quartz/CTRunData \
    vcl/quartz/ctfonts \
    vcl/quartz/ctlayout \
    vcl/quartz/salgdi \

# GUIBASE specific stuff

ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_add_cxxflags,vcl,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_add_defs,vcl,\
	-DMACOSX_BUNDLE_IDENTIFIER=\"$(MACOSX_BUNDLE_IDENTIFIER)\" \
))

$(eval $(call gb_Library_add_exception_objects,vcl,\
    $(vcl_coretext_code) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,vcl,\
	ApplicationServices \
))

$(eval $(call gb_Library_add_objcxxobjects,vcl,\
    vcl/osx/a11yactionwrapper \
    vcl/osx/a11ycomponentwrapper \
    vcl/osx/a11yfactory \
    vcl/osx/a11yrolehelper \
    vcl/osx/a11yselectionwrapper \
    vcl/osx/a11ytablewrapper \
    vcl/osx/a11ytextattributeswrapper \
    vcl/osx/a11ytextwrapper \
    vcl/osx/a11yutil \
    vcl/osx/a11yvaluewrapper \
    vcl/osx/a11ywrapper \
    vcl/osx/a11ywrapperbutton \
    vcl/osx/a11ywrappercheckbox \
    vcl/osx/a11ywrappercombobox \
    vcl/osx/a11ywrappergroup \
    vcl/osx/a11ywrapperlist \
    vcl/osx/a11ywrapperradiobutton \
    vcl/osx/a11ywrapperradiogroup \
    vcl/osx/a11ywrapperrow \
    vcl/osx/a11ywrapperscrollarea \
    vcl/osx/a11ywrapperscrollbar \
    vcl/osx/a11ywrappersplitter \
    vcl/osx/a11ywrapperstatictext \
    vcl/osx/a11ywrappertabgroup \
    vcl/osx/a11ywrappertextarea \
    vcl/osx/a11ywrappertoolbar \
    vcl/osx/salnstimer \
    vcl/osx/vclnsapp \
    vcl/osx/printaccessoryview \
    vcl/osx/printview \
    vcl/osx/salframeview \
    vcl/osx/salnsmenu \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/osx/a11yfocuslistener \
    vcl/osx/a11yfocustracker \
    vcl/osx/a11ylistener \
    vcl/osx/documentfocuslistener \
    vcl/osx/saldata \
    vcl/osx/salinst \
    vcl/osx/salsys \
    vcl/osx/saltimer \
    vcl/osx/DataFlavorMapping \
    vcl/osx/DragActionConversion \
    vcl/osx/DragSource \
    vcl/osx/DragSourceContext \
    vcl/osx/DropTarget \
    vcl/osx/HtmlFmtFlt \
    vcl/osx/OSXTransferable \
    vcl/osx/PictToBmpFlt \
    vcl/osx/clipboard \
    vcl/osx/service_entry \
    vcl/quartz/cairo_quartz_cairo \
    $(vcl_quartz_code) \
    vcl/quartz/salgdiutils \
    vcl/osx/salnativewidgets \
    vcl/osx/salprn \
    vcl/osx/salframe \
    vcl/osx/salmenu \
    vcl/osx/salobj \
))
$(eval $(call gb_Library_use_system_darwin_frameworks,vcl,\
    $(if $(filter X86_64,$(CPUNAME)),,QuickTime) \
    Cocoa \
    Carbon \
    CoreFoundation \
	OpenGL \
))

ifneq ($(ENABLE_MACOSX_SANDBOX),TRUE)
$(eval $(call gb_Library_use_libraries,vcl,\
    AppleRemote \
))
endif

endif

vcl_really_generic_code= \
    vcl/generic/app/gensys \
    vcl/generic/app/geninst \

vcl_generic_code= \
	$(vcl_really_generic_code) \
    vcl/generic/app/gendisp \
    vcl/generic/print/bitmap_gfx \
    vcl/generic/print/common_gfx \
    vcl/generic/print/glyphset \
    vcl/generic/print/printerjob \
    vcl/generic/print/psputil \
    vcl/generic/print/genpspgraphics \
    vcl/generic/print/genprnpsp \
    vcl/generic/print/prtsetup \
    vcl/generic/print/text_gfx \
    vcl/generic/fontmanager/fontsubst \
    vcl/generic/glyphs/gcach_ftyp \
    vcl/generic/glyphs/gcach_layout \
    vcl/generic/glyphs/gcach_rbmp \
    vcl/generic/glyphs/glyphcache \
    vcl/generic/glyphs/scrptrun \
    vcl/generic/fontmanager/fontcache \
    vcl/generic/fontmanager/fontconfig \
    vcl/generic/fontmanager/fontmanager \
    vcl/generic/fontmanager/helper \
    vcl/generic/fontmanager/parseAFM \

vcl_headless_code= \
    vcl/headless/svpbmp \
    vcl/headless/svpdummies \
    vcl/headless/svpframe \
    vcl/headless/svpgdi \
    vcl/headless/svpinst \
    vcl/headless/svpdata \
    vcl/headless/svpvd \

vcl_headless_freetype_code=\
    vcl/headless/svpprn \
    vcl/headless/svptext \
    vcl/headless/svptextrender \

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_add_exception_objects,vcl,\
	$(vcl_generic_code) \
    vcl/unx/generic/plugadapt/salplug \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/unx/generic/gdi/cairotextrender \
    vcl/unx/generic/gdi/x11windowprovider \
    $(if $(filter TRUE,$(ENABLE_CUPS)),\
        vcl/unx/generic/printer/cupsmgr \
        vcl/unx/generic/printer/printerinfomanager \
		, \
        vcl/null/printerinfomanager \
    ) \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_libraries,vcl,\
	basebmp \
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

ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_Library_add_exception_objects,vcl,\
	$(vcl_generic_code) \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/null/printerinfomanager \
    vcl/headless/headlessinst \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_libraries,vcl,\
	basebmp \
))

$(eval $(call gb_Library_use_externals,vcl,\
	fontconfig \
	freetype \
))
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_add_libs,vcl,\
	-llog \
	-landroid \
	-llo-bootstrap \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
	$(vcl_generic_code) \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/null/printerinfomanager \
    vcl/android/androidinst \
    $(vcl_headless_code) \
    $(vcl_headless_freetype_code) \
))

$(eval $(call gb_Library_use_static_libraries,vcl,\
	basebmp \
))

$(eval $(call gb_Library_use_externals,vcl,\
	fontconfig \
	freetype \
	expat \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_Library_add_cxxflags,vcl,\
    $(gb_OBJCXXFLAGS) \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/ios/iosinst \
    vcl/ios/dummies \
    $(vcl_really_generic_code) \
    $(vcl_coretext_code) \
	$(vcl_quartz_code) \
	$(vcl_headless_code) \
))
$(eval $(call gb_Library_use_system_darwin_frameworks,vcl,\
	UIKit \
	CoreFoundation \
))
endif

# OS-specific stuff

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,vcl,\
	vcl/opengl/win/gdiimpl \
	vcl/opengl/win/WinDeviceInfo \
    vcl/win/source/app/saldata \
    vcl/win/source/app/salinfo \
    vcl/win/source/app/salinst \
    vcl/win/source/app/salshl \
    vcl/win/source/app/saltimer \
    vcl/win/source/gdi/cairo_win32_cairo \
    vcl/win/source/gdi/gdiimpl \
    vcl/win/source/gdi/salbmp \
    vcl/win/source/gdi/salgdi \
    vcl/win/source/gdi/salgdi2 \
    vcl/win/source/gdi/salgdi3 \
    vcl/win/source/gdi/salgdi_gdiplus \
    vcl/win/source/gdi/salnativewidgets-luna \
    vcl/win/source/gdi/salprn \
    vcl/win/source/gdi/salvd \
    vcl/win/source/gdi/winlayout \
    vcl/win/source/gdi/wntgdi \
    vcl/win/source/window/salframe \
    vcl/win/source/window/keynames \
    vcl/win/source/window/salmenu \
    vcl/win/source/window/salobj \
))

$(eval $(call gb_Library_use_system_win32_libs,vcl,\
	advapi32 \
	crypt32 \
	gdi32 \
	gdiplus \
    glu32 \
	imm32 \
	mpr \
	msimg32 \
    opengl32 \
	ole32 \
	shell32 \
	usp10 \
	uuid \
	version \
	winspool \
	setupapi \
	shlwapi \
))

$(eval $(call gb_Library_add_nativeres,vcl,vcl/salsrc))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vcl,\
	-lm \
	-ldl \
	-lpthread \
    -lGL \
    -lX11 \
))

$(eval $(call gb_Library_add_exception_objects,vcl,\
	vcl/opengl/x11/X11DeviceInfo \
))
endif

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,vcl,vcl))

ifeq ($(OS),WNT)
# HACK: dependency on icon themes so running unit tests don't
# prevent delivering these by having open file handles on WNT
$(eval $(call gb_Library_use_package,vcl,postprocess_images))
endif

# vim: set noet sw=4 ts=4:
