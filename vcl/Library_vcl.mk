#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

$(eval $(call gb_Library_Library,vcl))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.macosx))
else ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.windows))
else
$(eval $(call gb_Library_set_componentfile,vcl,vcl/vcl.unx))
endif

$(eval $(call gb_Library_add_package_headers,vcl,vcl_inc))

$(eval $(call gb_Library_set_include,vcl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))
ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_set_cxxflags,vcl,\
    $$(CXXFLAGS) \
    $$(FREETYPE_CFLAGS) \
))
endif

$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DVCL_DLLIMPLEMENTATION \
    -DCUI_DLL_NAME=\"$(or $(call gb_Library_get_dllname,cui),$(call gb_Library_get_filename,cui))\" \
    -DDLLPOSTFIX=$(subst $(or $(gb_Library_DLLEXT),$(gb_Library_PLAINEXT)),,$(gb_Library_OOOEXT)) \
))

$(eval $(call gb_Library_add_linked_libs,vcl,\
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    icuuc \
    icule \
    i18nisolang1 \
    i18npaper \
    i18nutil \
    jvmaccess \
    stl \
    cppu \
    sal \
    vos3 \
))

ifneq ($(ENABLE_GRAPHITE),)
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    graphite_dll \
))
endif
endif
ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    freetype \
))
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Library_set_cxxflags,vcl,\
    $$(CXXFLAGS) \
    $$(OBJCXXFLAGS) \
))
ifeq ($(ENABLE_CAIRO),TRUE)
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DCAIRO \
))
endif
$(eval $(call gb_Library_add_objcxxobjects,vcl,\
    vcl/aqua/source/a11y/aqua11yactionwrapper \
    vcl/aqua/source/a11y/aqua11ycomponentwrapper \
    vcl/aqua/source/a11y/aqua11yfactory \
    vcl/aqua/source/a11y/aqua11yrolehelper \
    vcl/aqua/source/a11y/aqua11yselectionwrapper \
    vcl/aqua/source/a11y/aqua11ytablewrapper \
    vcl/aqua/source/a11y/aqua11ytextattributeswrapper \
    vcl/aqua/source/a11y/aqua11ytextwrapper \
    vcl/aqua/source/a11y/aqua11yutil \
    vcl/aqua/source/a11y/aqua11yvaluewrapper \
    vcl/aqua/source/a11y/aqua11ywrapper \
    vcl/aqua/source/a11y/aqua11ywrapperbutton \
    vcl/aqua/source/a11y/aqua11ywrappercheckbox \
    vcl/aqua/source/a11y/aqua11ywrappercombobox \
    vcl/aqua/source/a11y/aqua11ywrappergroup \
    vcl/aqua/source/a11y/aqua11ywrapperlist \
    vcl/aqua/source/a11y/aqua11ywrapperradiobutton \
    vcl/aqua/source/a11y/aqua11ywrapperradiogroup \
    vcl/aqua/source/a11y/aqua11ywrapperrow \
    vcl/aqua/source/a11y/aqua11ywrapperscrollarea \
    vcl/aqua/source/a11y/aqua11ywrapperscrollbar \
    vcl/aqua/source/a11y/aqua11ywrappersplitter \
    vcl/aqua/source/a11y/aqua11ywrapperstatictext \
    vcl/aqua/source/a11y/aqua11ywrappertabgroup \
    vcl/aqua/source/a11y/aqua11ywrappertextarea \
    vcl/aqua/source/a11y/aqua11ywrappertoolbar \
    vcl/aqua/source/app/salnstimer \
    vcl/aqua/source/app/vclnsapp \
    vcl/aqua/source/gdi/aquaprintaccessoryview \
    vcl/aqua/source/gdi/aquaprintview \
    vcl/aqua/source/window/salframeview \
    vcl/aqua/source/window/salnsmenu \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/aqua/source/a11y/aqua11yfocuslistener \
    vcl/aqua/source/a11y/aqua11yfocustracker \
    vcl/aqua/source/a11y/aqua11ylistener \
    vcl/aqua/source/a11y/documentfocuslistener \
    vcl/aqua/source/app/saldata \
    vcl/aqua/source/app/salinst \
    vcl/aqua/source/app/salsys \
    vcl/aqua/source/app/saltimer \
    vcl/aqua/source/dtrans/DataFlavorMapping \
    vcl/aqua/source/dtrans/DragActionConversion \
    vcl/aqua/source/dtrans/DragSource \
    vcl/aqua/source/dtrans/DragSourceContext \
    vcl/aqua/source/dtrans/DropTarget \
    vcl/aqua/source/dtrans/HtmlFmtFlt \
    vcl/aqua/source/dtrans/OSXTransferable \
    vcl/aqua/source/dtrans/PictToBmpFlt \
    vcl/aqua/source/dtrans/aqua_clipboard \
    vcl/aqua/source/dtrans/service_entry \
    vcl/aqua/source/gdi/salatslayout \
    vcl/aqua/source/gdi/salatsuifontutils \
    vcl/aqua/source/gdi/salbmp \
    vcl/aqua/source/gdi/salcolorutils \
    vcl/aqua/source/gdi/salgdi \
    vcl/aqua/source/gdi/salgdiutils \
    vcl/aqua/source/gdi/salmathutils \
    vcl/aqua/source/gdi/salnativewidgets \
    vcl/aqua/source/gdi/salprn \
    vcl/aqua/source/gdi/salvd \
    vcl/aqua/source/window/salframe \
    vcl/aqua/source/window/salmenu \
    vcl/aqua/source/window/salobj \
))
endif

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DSAL_DLLPREFIX=\"$(gb_Library_SYSPRE)\" \
    -DSAL_DLLPOSTFIX=\"$(gb_Library_OOOEXT)\" \
    -D_XSALSET_LIBNAME=\"$(call gb_Library_get_filename,spa)\" \
))
## handle fontconfig
ifneq ($(ENABLE_FONTCONFIG),)
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DENABLE_FONTCONFIG \
))
## handle CUPS
ifneq ($(ENABLE_CUPS),)
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DENABLE_CUPS \
))
endif
endif
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/glyphs/gcach_ftyp \
    vcl/source/glyphs/gcach_layout \
    vcl/source/glyphs/gcach_rbmp \
    vcl/source/glyphs/glyphcache \
    vcl/unx/generic/fontmanager/fontcache \
    vcl/unx/generic/fontmanager/fontconfig \
    vcl/unx/generic/fontmanager/fontmanager \
    vcl/unx/generic/fontmanager/helper \
    vcl/unx/generic/fontmanager/parseAFM \
    vcl/unx/generic/plugadapt/salplug \
    vcl/unx/generic/printer/cupsmgr \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/unx/generic/printer/printerinfomanager \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/win/source/app/saldata \
    vcl/win/source/app/salinfo \
    vcl/win/source/app/salinst \
    vcl/win/source/app/salshl \
    vcl/win/source/app/saltimer \
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
    vcl/win/source/window/salmenu \
    vcl/win/source/window/salobj \
))

$(eval $(call gb_Library_add_nativeres,vcl,src))

endif

$(eval $(call gb_Library_add_cobjects,vcl,\
    vcl/source/fontsubset/list \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/app/dbggui \
    vcl/source/app/dndhelp \
    vcl/source/app/help \
    vcl/source/app/i18nhelp \
    vcl/source/app/idlemgr \
    vcl/source/app/salvtables \
    vcl/source/app/session \
    vcl/source/app/settings \
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
    vcl/source/components/display \
    vcl/source/components/dtranscomp \
    vcl/source/components/factory \
    vcl/source/components/fontident \
    vcl/source/components/stringmirror \
    vcl/source/control/button \
    vcl/source/control/combobox \
    vcl/source/control/ctrl \
    vcl/source/control/edit \
    vcl/source/control/field2 \
    vcl/source/control/field \
    vcl/source/control/fixbrd \
    vcl/source/control/fixed \
    vcl/source/control/group \
    vcl/source/control/ilstbox \
    vcl/source/control/imgctrl \
    vcl/source/control/longcurr \
    vcl/source/control/lstbox \
    vcl/source/control/menubtn \
    vcl/source/control/morebtn \
    vcl/source/control/quickselectionengine \
    vcl/source/control/scrbar \
    vcl/source/control/slider \
    vcl/source/control/spinbtn \
    vcl/source/control/spinfld \
    vcl/source/control/tabctrl \
    vcl/source/control/throbber \
    vcl/source/fontsubset/cff \
    vcl/source/fontsubset/fontsubset \
    vcl/source/fontsubset/gsub \
    vcl/source/fontsubset/sft \
    vcl/source/fontsubset/ttcr \
    vcl/source/fontsubset/xlat \
    vcl/source/gdi/alpha \
    vcl/source/gdi/animate \
    vcl/source/gdi/base14 \
    vcl/source/gdi/bitmap2 \
    vcl/source/gdi/bitmap3 \
    vcl/source/gdi/bitmap4 \
    vcl/source/gdi/bitmap \
    vcl/source/gdi/bitmapex \
    vcl/source/gdi/bmpacc2 \
    vcl/source/gdi/bmpacc3 \
    vcl/source/gdi/bmpacc \
    vcl/source/gdi/bmpconv \
    vcl/source/gdi/bmpfast \
    vcl/source/gdi/configsettings \
    vcl/source/gdi/cvtgrf \
    vcl/source/gdi/cvtsvm \
    vcl/source/gdi/extoutdevdata \
    vcl/source/gdi/font \
    vcl/source/gdi/gdimtf \
    vcl/source/gdi/gfxlink \
    vcl/source/gdi/gradient \
    vcl/source/gdi/graph \
    vcl/source/gdi/graphictools \
    vcl/source/gdi/hatch \
    vcl/source/gdi/image \
    vcl/source/gdi/imagerepository \
    vcl/source/gdi/impanmvw \
    vcl/source/gdi/impbmp \
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
    vcl/source/gdi/outdev2 \
    vcl/source/gdi/outdev3 \
    vcl/source/gdi/outdev4 \
    vcl/source/gdi/outdev5 \
    vcl/source/gdi/outdev6 \
    vcl/source/gdi/outdev \
    vcl/source/gdi/outdevnative \
    vcl/source/gdi/outmap \
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
    vcl/source/gdi/salgdilayout \
    vcl/source/gdi/sallayout \
    vcl/source/gdi/salmisc \
    vcl/source/gdi/salnativewidgets-none \
    vcl/source/gdi/textlayout \
    vcl/source/gdi/virdev \
    vcl/source/gdi/wall \
    vcl/source/helper/canvasbitmap \
    vcl/source/helper/canvastools \
    vcl/source/helper/evntpost \
    vcl/source/helper/lazydelete \
    vcl/source/helper/strhelper \
    vcl/source/helper/threadex \
    vcl/source/helper/xconnection \
    vcl/source/salmain/salmain \
    vcl/source/window/abstdlg \
    vcl/source/window/accel \
    vcl/source/window/accmgr \
    vcl/source/window/arrange \
    vcl/source/window/brdwin \
    vcl/source/window/btndlg \
    vcl/source/window/cmdevt \
    vcl/source/window/cursor \
    vcl/source/window/decoview \
    vcl/source/window/dialog \
    vcl/source/window/dlgctrl \
    vcl/source/window/dndevdis \
    vcl/source/window/dndlcon \
    vcl/source/window/dockingarea \
    vcl/source/window/dockmgr \
    vcl/source/window/dockwin \
    vcl/source/window/floatwin \
    vcl/source/window/introwin \
    vcl/source/window/javachild \
    vcl/source/window/keycod \
    vcl/source/window/keyevent \
    vcl/source/window/menu \
    vcl/source/window/mnemonic \
    vcl/source/window/mnemonicengine \
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
    vcl/source/window/window4 \
    vcl/source/window/window \
    vcl/source/window/winproc \
    vcl/source/window/wpropset \
    vcl/source/window/wrkwin \
))

## handle Graphite
ifneq ($(ENABLE_GRAPHITE),)
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DENABLE_GRAPHITE \
))
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/glyphs/graphite_cache \
    vcl/source/glyphs/graphite_features \
    vcl/source/glyphs/graphite_layout \
    vcl/source/glyphs/graphite_textsrc \
))
ifeq ($(GUIBASE),unx)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/source/glyphs/graphite_adaptors \
    vcl/source/glyphs/graphite_serverfont \
))
ifeq ($(SYSTEM_GRAPHITE),YES)
$(eval $(call gb_Library_set_ldflags,vcl,\
    $$(LDFLAGS) \
    $(GRAPHITE_LIBS)
))
else
$(eval $(call gb_Library_add_linked_static_libs,vcl,\
    graphite \
))
endif
endif
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    dl \
    m \
    pthread \
))
endif

ifeq ($(OS),SOLARIS)
ifeq ($(CPUNAME)$(CPU),SPARCU)
$(eval $(call gb_Library_set_ldflags,vcl,\
    $$(LDFLAGS) \
    -R/usr/sfw/lib/64 \
))
else
$(eval $(call gb_Library_set_ldflags,vcl,\
    $$(LDFLAGS) \
    -R/usr/sfw/lib \
))
endif
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    AppleRemote \
))
$(eval $(call gb_Library_set_ldflags,vcl,\
    $$(LDFLAGS) \
    -framework QuickTime \
    -framework Cocoa \
    -framework Carbon \
    -framework CoreFoundation \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_ldflags,vcl,\
    $$(LDFLAGS) \
    /ENTRY:LibMain@12 \
))
$(eval $(call gb_Library_add_linked_libs,vcl,\
    advapi32 \
    gdi32 \
    gdiplus \
    imm32 \
    kernel32 \
    mpr \
    msimg32 \
    msvcrt \
    oldnames \
    ole32 \
    shell32 \
    user32 \
    uuid \
    uwinapi \
    winspool \
))
endif
# vim: set noet sw=4 ts=4:
