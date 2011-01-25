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

$(eval $(call gb_Library_add_package_headers,vcl,vcl_inc))

$(eval $(call gb_Library_add_precompiled_header,vcl,$(SRCDIR)/vcl/inc/pch/precompiled_vcl))

$(eval $(call gb_Library_set_include,vcl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))
ifeq ($(GUI),UNX)
$(eval $(call gb_Library_set_cxxflags,vcl,\
    $$(CXXFLAGS) \
    $$(FREETYPE_CFLAGS) \
))
endif

$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DVCL_DLLIMPLEMENTATION \
    -DCUI_DLL_NAME=\"$(gb_Library_SYSPRE)cui$(gb_Library_OOOEXT)\" \
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
$(eval $(call gb_Library_add_linked_static_libs,vcl,\
    graphite \
))
endif
ifeq ($(GUI),UNX)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    freetype \
))
endif

ifeq ($(GUI),UNX)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/unx/generic/printer/cupsmgr \
    vcl/unx/generic/printer/jobdata \
    vcl/unx/generic/printer/ppdparser \
    vcl/unx/generic/printer/printerinfomanager \
    vcl/unx/generic/fontmanager/fontcache \
    vcl/unx/generic/fontmanager/fontconfig \
    vcl/unx/generic/fontmanager/fontmanager \
    vcl/unx/generic/fontmanager/helper \
    vcl/unx/generic/fontmanager/parseAFM \
))
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
    vcl/source/glyphs/gcach_ftyp \
    vcl/source/glyphs/gcach_layout \
    vcl/source/glyphs/gcach_rbmp \
    vcl/source/glyphs/glyphcache \
    vcl/source/glyphs/graphite_adaptors \
    vcl/source/glyphs/graphite_cache \
    vcl/source/glyphs/graphite_features \
    vcl/source/glyphs/graphite_layout \
    vcl/source/glyphs/graphite_serverfont \
    vcl/source/glyphs/graphite_textsrc \
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

ifeq ($(GUI),UNX)
$(eval $(call gb_Library_add_exception_objects,vcl,\
    vcl/unx/generic/plugadapt/salplug \
))
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DSAL_DLLPREFIX=\"$(gb_Library_SYSPRE)\" \
    -DSAL_DLLPOSTFIX=\"$(gb_Library_OOOEXT)\" \
    -D_XSALSET_LIBNAME=\"$(gb_Library_SYSPRE)spa$(gb_Library_OOOEXT)\" \
))
ifneq ($(ENABLE_FONTCONFIG),)
$(eval $(call gb_Library_set_defs,vcl,\
    $$(DEFS) \
    -DENABLE_FONTCONFIG \
))
endif
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    dl \
    m \
    pthread \
))
endif

ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
$(eval $(call gb_Library_add_linked_libs,vcl,\
    mingwthrd \
    $(gb_MINGW_LIBSTDCPP) \
    mingw32 \
    $(gb_MINGW_LIBGCC) \
    uwinapi \
    moldname \
    mingwex \
    advapi32 \
    kernel32 \
    mpr \
    msvcrt \
    ole32 \
    shell32 \
    user32 \
    uuid \
))
else
$(eval $(call gb_Library_add_linked_libs,vcl,\
    advapi32 \
    kernel32 \
    mpr \
    msvcrt \
    oldnames \
    ole32 \
    shell32 \
    user32 \
    uuid \
    uwinapi \
))
endif
endif
# vim: set noet sw=4 ts=4:
