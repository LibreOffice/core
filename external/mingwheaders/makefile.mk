#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 13:40:04 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

PSDK_INCLUDE_DIR=$(PSDK_HOME)$/include$/

ATL_INCLUDE_DIR=$(PSDK_HOME)$/include$/atl$/

FILES_TO_COPY_FROM_MINGW=excpt.h \
             tchar.h

FILES_TO_COPY_FROM_W32API=basetyps.h \
              objfwd.h \
              rpcndr.h \
              winbase.h \
              wininet.h \
              winnt.h \
              winver.h \
              wtypes.h

FILES_TO_COPY_FROM_PSDK=adoint.h \
            commctrl.h \
            exdispid.h \
            gdiplusenums.h \
            gdiplusheaders.h \
            gdiplusimaging.h \
            imagehlp.h \
            mapiwin.h \
            oaidl.h \
            objidl.h \
            ocidl.h \
            oleauto.h \
            olectl.h \
            oledb.h \
            propidl.h \
            qedit.h \
            shlobj.h \
            shtypes.h \
            specstrings.h \
            strmif.h \
            wincrypt.h \
            winerror.h \
            wingdi.h \
            winsock2.h \
            ws2tcpip.h \
            wspiapi.h \
            amstream.h \
            austream.h \
            ddstream.h \
            dispex.h \
            filter.h \
            gdiplus.h \
            mmstream.h \
                msdasc.h \
            msxml.h \
            oleidl.h \
            qos.h \
            shobjidl.h \
            shlguid.h \
            transact.h \
            urlmon.h

FILES_TO_COPY_FROM_ATL=atlbase.h \
               atlcom.h \
               atlconv.h \
               atlctl.h \
               atlwin.h \
               statreg.h

$(MISC)$/mingwheader_patch_all : $(MISC)$/mingwheader_patch $(LB)$/libmsvcrt.a

$(MISC)$/mingwheader_patch : $(MISC)$/mingwheader_copy
    -patch -bd $(MISC)$/mingw -p0 -N -f -Z <mingw_headers.patch
    $(COPY) $(MISC)$/mingw$/include/adoctint.h ../ado
    $(TOUCH) $(MISC)$/mingwheader_add
    $(TOUCH) $(MISC)$/mingwheader_patch

$(MISC)$/mingwheader_copy:
    $(MKDIR) $(MISC)$/mingw$/include
    $(MKDIR) $(MISC)$/mingw$/include/atl
    $(COPY) $(FILES_TO_COPY_FROM_MINGW:^$(MINGW_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_W32API:^$(MINGW_W32API_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_PSDK:^$(PSDK_INCLUDE_DIR)) $(MISC)$/mingw$/include
    $(COPY) $(FILES_TO_COPY_FROM_ATL:^$(ATL_INCLUDE_DIR)) $(MISC)$/mingw$/include$/atl
    $(COPY) ../ado/adoctint.h $(MISC)$/mingw$/include
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

