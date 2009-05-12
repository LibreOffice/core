#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.8.10.1 $
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

PRJ=..
PRJNAME=external
TARGET=mingwheaders

.INCLUDE :  settings.mk

.IF "$(OS)$(COM)" == "WNTGCC"

# Cygwin and MinGW use different directories for the W32API headers
.IF "$(USE_MINGW)" == "cygwin"
MINGW_INCLUDE_DIR=$/usr$/include/mingw/
MINGW_W32API_INCLUDE_DIR=$/usr$/include/w32api/
MINGW_W32API_LIB_DIR=$/usr$/include/w32api/
.ELSE
MINGW_INCLUDE_DIR=$(COMPATH)$/include/
MINGW_W32API_INCLUDE_DIR=$(COMPATH)$/include/
MINGW_W32API_LIB_DIR=$(COMPATH)$/lib/
.ENDIF

SYS_INCLUDE_DIR=$(MINGW_INCLUDE_DIR)$/sys/

PSDK_INCLUDE_DIR=$(PSDK_HOME)$/include$/

ATL_INCLUDE_DIR=$(PSDK_HOME)$/include$/atl$/

DIRECTXSDK_INCLUDE_DIR=$(DIRECTXSDK_HOME)$/include$/

FILES_TO_COPY_FROM_MINGW=excpt.h \
             tchar.h

FILES_TO_COPY_FROM_W32API=amvideo.h \
              bdatypes.h \
              imm.h \
              oaidl.h \
              objfwd.h \
              objidl.h \
              uxtheme.h \
              winbase.h \
              wininet.h \
              winnt.h \
              winver.h \
              wtypes.h

FILES_TO_COPY_FROM_SYS=stat.h

FILES_TO_COPY_FROM_PSDK=adoctint.h \
            adodef.h \
            adoguids.h \
            adoint.h \
            bcrypt.h \
            commctrl.h \
            control.h \
            filter.h \
            gdiplusbitmap.h \
            gdiplusbrush.h \
            gdiplusenums.h \
            gdiplusfont.h \
            gdiplusheaders.h \
            gdiplusimageattributes.h \
            gdiplusimaging.h \
            gdiplusmatrix.h \
            gdipluspath.h \
            gdipluspen.h \
            gdiplusregion.h \
            gdiplusstringformat.h \
            imagehlp.h \
            inaddr.h \
            intsafe.h \
            mapinls.h \
            mapiwin.h \
                msdasc.h \
                msi.h \
                msiquery.h \
            multimon.h \
            ncrypt.h \
                ntquery.h \
            ocidl.h \
            oledb.h \
            oleidl.h \
            propidl.h \
            propkeydef.h \
            propsys.h \
            qedit.h \
            shlobj.h \
            shobjidl.h \
            shtypes.h \
            specstrings.h \
            specstrings_adt.h \
            specstrings_strict.h \
            sspi.h \
            strmif.h \
            strsafe.h \
            structuredquery.h \
            urlmon.h \
            wincrypt.h \
            wingdi.h \
            winsock2.h \
            winuser.h \
            ws2def.h \
            ws2ipdef.h \
            ws2tcpip.h \
            wspiapi.h \
            adoid.h \
            amstream.h \
            austream.h \
            ddstream.h \
            dispex.h \
            dshow.h \
            exdispid.h \
            gdiplus.h \
            mmstream.h \
            msxml.h \
            oleauto.h \
            olectl.h \
            qos.h \
            shlguid.h \
            transact.h \
            winerror.h

.IF "$(DISABLE_ATL)"==""
MINGWHEADER_ATL_PATCH_TARGET=$(MISC)$/mingwheader_atl_patch
MINGWHEADER_ATL_COPY_TARGET=$(MISC)$/mingwheader_atl_copy
FILES_TO_COPY_FROM_ATL=atlbase.h \
               atlcom.h \
               atlconv.h \
               atlctl.h \
               atlwin.h \
               statreg.h
.ENDIF

FILES_TO_COPY_FROM_DIRECTXSDK=dxtrans.h \
                  d3dx9.h \
                  d3d.h \
                  d3dtypes.h \
                  d3dx9math.h \
                  d3dx9math.inl \
                  d3dx9core.h

$(MISC)$/mingwheader_patch_all : $(MISC)$/mingwheader_patch $(MINGWHEADER_ATL_PATCH_TARGET) $(LB)$/libmsvcrt.a

$(MISC)$/mingwheader_patch : $(MISC)$/mingwheader_copy
    patch -bd $(MISC)$/mingw -p0 -N -f -Z <mingw_headers.patch
    $(TOUCH) $(MISC)$/mingwheader_patch

.IF "$(DISABLE_ATL)"==""
$(MINGWHEADER_ATL_PATCH_TARGET) : $(MINGWHEADER_ATL_COPY_TARGET)
    patch -bd $(MISC)$/mingw -p0 -N -f -Z <mingw_atl_headers.patch
    $(TOUCH) $(MINGWHEADER_ATL_PATCH_TARGET)
.ENDIF

$(MISC)$/mingwheader_copy:
    $(MKDIRHIER) $(MISC)$/mingw$/include
    $(MKDIRHIER) $(MISC)$/mingw$/include/sys
    $(COPY) $(FILES_TO_COPY_FROM_MINGW:^$(MINGW_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_W32API:^$(MINGW_W32API_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_SYS:^$(SYS_INCLUDE_DIR)) $(MISC)$/mingw$/include$/sys
    $(COPY) $(FILES_TO_COPY_FROM_PSDK:^$(PSDK_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_DIRECTXSDK:^$(DIRECTXSDK_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(TOUCH) $(MISC)$/mingwheader_copy

.IF "$(DISABLE_ATL)"==""
$(MINGWHEADER_ATL_COPY_TARGET):
    $(MKDIRHIER) $(MISC)$/mingw$/include/atl
    $(COPY) $(FILES_TO_COPY_FROM_ATL:^$(ATL_INCLUDE_DIR)) $(MISC)$/mingw$/include$/atl
     $(TOUCH) $(MINGWHEADER_ATL_COPY_TARGET)
.ENDIF

$(LB)$/libmsvcrt.a:
.IF "$(USE_MINGW)" == "cygwin"
    $(WRAPCMD) nm -g --defined-only $(COMPATH)$/lib/mingw/libmsvcrt.a > $(MISC)$/msvcrt.nm
.ELSE
    $(WRAPCMD) nm -g --defined-only $(COMPATH)$/lib/libmsvcrt.a > $(MISC)$/msvcrt.nm
.ENDIF
    echo EXPORTS > $(MISC)$/msvcrt.def
    sed -ne 's/.* T _//p' $(MISC)$/msvcrt.nm | sort >> $(MISC)$/msvcrt.def
    -sed -ne 's/.* I __imp__//p' $(MISC)$/msvcrt.nm | sort | diff - $(MISC)$/msvcrt.def | \
        sed -ne 's/^< \(.*\)$/\1 DATA/p' > $(MISC)$/msvcrtdata.def
    cat $(MISC)$/msvcrtdata.def >> $(MISC)$/msvcrt.def
    dlltool --dllname msvcrt.dll --input-def=$(MISC)$/msvcrt.def --kill-at --output-lib=$(LB)$/libmsvcrt.a

clean:
    -$(RM) $(MISC)$/mingw$/include$/*.h
    -$(RM) $(MISC)$/mingwheader_copy
    -$(RM) $(MISC)$/mingwheader_patch

.ENDIF

.INCLUDE :  target.mk

