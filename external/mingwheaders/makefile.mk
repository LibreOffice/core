#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:19:24 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..
PRJNAME=external
TARGET=mingwheaders

.INCLUDE :  settings.mk

.IF "$(OS)$(COM)" == "WNTGCC"

# Cygwin and MinGW use different directories for the W32API headers
.IF "$(USE_MINGW)" == "cygwin"
MINGW_INCLUDE_DIR=$(COMPATH)$/include/mingw/include/
MINGW_W32API_INCLUDE_DIR=$(COMPATH)$/include/w32api/
MINGW_W32API_LIB_DIR=$(COMPATH)$/include/w32api/
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
              basetyps.h \
              bdatypes.h \
              oaidl.h \
              objfwd.h \
              objidl.h \
              uxtheme.h \
              winbase.h \
              wininet.h \
              winnt.h \
              winuser.h \
              winver.h \
              wtypes.h

FILES_TO_COPY_FROM_SYS=stat.h

FILES_TO_COPY_FROM_PSDK=adoctint.h \
            adodef.h \
            adoguids.h \
            adoint.h \
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
            imagehlp.h \
            mapinls.h \
            mapiwin.h \
                msdasc.h \
            multimon.h \
            ocidl.h \
            oledb.h \
            oleidl.h \
            propidl.h \
            qedit.h \
            shlobj.h \
            shobjidl.h \
            shtypes.h \
            specstrings.h \
            sspi.h \
            strmif.h \
            strsafe.h \
            urlmon.h \
            wincrypt.h \
            wingdi.h \
            winsock2.h \
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

FILES_TO_COPY_FROM_ATL=atlbase.h \
               atlcom.h \
               atlconv.h \
               atlctl.h \
               atlwin.h \
               statreg.h

FILES_TO_COPY_FROM_DIRECTXSDK=dxtrans.h \
                  d3dx9.h

$(MISC)$/mingwheader_patch_all : $(MISC)$/mingwheader_patch $(LB)$/libmsvcrt.a

$(MISC)$/mingwheader_patch : $(MISC)$/mingwheader_copy
    patch -bd $(MISC)$/mingw -p0 -N -f -Z <mingw_headers.patch
    $(TOUCH) $(MISC)$/mingwheader_add
    $(TOUCH) $(MISC)$/mingwheader_patch

$(MISC)$/mingwheader_copy:
    $(MKDIRHIER) $(MISC)$/mingw$/include
    $(MKDIRHIER) $(MISC)$/mingw$/include/atl
    $(MKDIRHIER) $(MISC)$/mingw$/include/sys
    $(COPY) $(FILES_TO_COPY_FROM_MINGW:^$(MINGW_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_W32API:^$(MINGW_W32API_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_SYS:^$(SYS_INCLUDE_DIR)) $(MISC)$/mingw$/include$/sys
    $(COPY) $(FILES_TO_COPY_FROM_PSDK:^$(PSDK_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_ATL:^$(ATL_INCLUDE_DIR)) $(MISC)$/mingw$/include$/atl
    $(COPY) $(FILES_TO_COPY_FROM_DIRECTXSDK:^$(DIRECTXSDK_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(TOUCH) $(MISC)$/mingwheader_copy

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

