#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

FILES_TO_COPY_FROM_MINGW= \
    excpt.h \
     tchar.h

FILES_TO_COPY_FROM_W32API= \
    amvideo.h \
    basetyps.h \
    objidl.h \
    specstrings.h \
    uxtheme.h \
    winbase.h \
    windef.h \
    wininet.h \
    winver.h \
    wtypes.h

FILES_TO_COPY_FROM_SYS= \
    stat.h

FILES_TO_COPY_FROM_PSDK= \
    adoctint.h \
    adodef.h \
    adoguids.h \
    adoint.h \
    bcrypt.h \
    commctrl.h \
    control.h \
    dispex.h \
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
    imm.h \
    imagehlp.h \
    mapiwin.h \
    msdasc.h \
    msi.h \
    msiquery.h \
    multimon.h \
    ntquery.h \
    oaidl.h \
    ocidl.h \
    oleauto.h \
    olectl.h \
    oledb.h \
    oleidl.h \
    propidl.h \
    propkeydef.h \
    propsys.h \
    propvarutil.h \
    shlobj.h \
    shobjidl.h \
    shtypes.h \
    sspi.h \
    strmif.h \
    strsafe.h \
    structuredquery.h \
    urlmon.h \
    wincrypt.h \
    winerror.h \
    wingdi.h \
    wintrust.h \
    winuser.h \
    wspiapi.h \
    exdispid.h \
    dshow.h \
    gdiplus.h \
    msxml.h \
    ncrypt.h \
    shlguid.h \
    transact.h

.IF "$(DISABLE_ATL)"==""
MINGWHEADER_ATL_PATCH_TARGET=$(MISC)$/mingwheader_atl_patch
MINGWHEADER_ATL_COPY_TARGET=$(MISC)$/mingwheader_atl_copy
FILES_TO_COPY_FROM_ATL= \
    atlbase.h \
    atlcom.h \
    atlconv.h \
    atlctl.h \
    atlwin.h \
    statreg.h
.ENDIF

FILES_TO_COPY_FROM_DIRECTXSDK= \
    d3dtypes.h \
    d3dx9core.h \
    d3dx9math.h \
    d3dx9math.inl \
    dxtrans.h \
    d3dx9.h

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
    $(COPY) $(COMPATH)$/lib/mingw/libmsvcrt.a $(LB)$/libmsvcrt_orig.a
.ELSE
    $(COPY) $(COMPATH)$/lib/libmsvcrt.a $(LB)$/libmsvcrt_orig.a
.ENDIF
    cd $(LB) && ar -x libmsvcrt_orig.a ofmt_stub.o
    ar -d $(LB)$/libmsvcrt_orig.a ofmt_stub.o
    nm -g --defined-only $(LB)$/libmsvcrt_orig.a > $(MISC)$/msvcrt.nm
    echo EXPORTS > $(MISC)$/msvcrt.def
    sed -ne 's/.* T _//p' $(MISC)$/msvcrt.nm | sort >> $(MISC)$/msvcrt.def
    -sed -ne 's/.* I __imp__//p' $(MISC)$/msvcrt.nm | sort | diff - $(MISC)$/msvcrt.def | \
        sed -ne 's/^< \(.*\)$/\1 DATA/p' > $(MISC)$/msvcrtdata.def
    cat $(MISC)$/msvcrtdata.def >> $(MISC)$/msvcrt.def
    dlltool --dllname msvcrt.dll --input-def=$(MISC)$/msvcrt.def --kill-at --output-lib=$(LB)$/libmsvcrt.a
    cd $(LB) && ar -rs libmsvcrt.a ofmt_stub.o

clean:
    -$(RM) $(MISC)$/mingw$/include$/*.h
    -$(RM) $(MISC)$/mingwheader_copy
    -$(RM) $(MISC)$/mingwheader_patch

.ENDIF

.INCLUDE :  target.mk

