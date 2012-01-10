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

PRJNAME=cairo
TARGET=so_cairo
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF  "$(ENABLE_CAIRO)" == ""
all:
    @echo "Nothing to do (Cairo not enabled)."

.ELIF "$(SYSTEM_CAIRO)" == "YES"
all:
    @echo "Nothing to do, using system cairo."

.ENDIF

# --- Files --------------------------------------------------------

CAIROVERSION=1.8.0

TARFILE_NAME=$(PRJNAME)-$(CAIROVERSION)
TARFILE_MD5=4ea70ea87b47e92d318d4e7f5b940f47

PATCH_FILES=..$/$(TARFILE_NAME).patch

cairo_CFLAGS=$(SOLARINC)
cairo_LDFLAGS=$(SOLARLIB)

cairo_CPPFLAGS=

.IF "$(SYSTEM_ZLIB)"!="YES"
cairo_CPPFLAGS+=-I$(SOLARINCDIR)$/external$/zlib
cairo_COMPRESS=z_compress
.ELSE
cairo_COMPRESS=compress
.ENDIF
cairo_CPPFLAGS+=$(INCLUDE)

.IF "$(OS)"=="WNT"
# --------- Windows -------------------------------------------------
.IF "$(COM)"=="GCC"
cairo_LDFLAGS+=-no-undefined -L$(ILIB:s/;/ -L/)
cairo_CPPFLAGS+=-nostdinc
cairo_CC=$(CC) -mthreads

.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
cairo_CC+=-shared-libgcc
.ENDIF
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
cairo_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS=--disable-xlib --disable-ft --disable-pthread --disable-svg --enable-gtk-doc=no --enable-test-surfaces=no --enable-static=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 PKG_CONFIG=./dummy_pkg_config CC="$(cairo_CC)" LIBS="$(cairo_LIBS)" ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS) OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE   # WNT, not GCC
BUILD_ACTION=$(GNUMAKE) -f Makefile.win32 CFG=release
BUILD_DIR=
.ENDIF

OUT2INC+=src$/cairo-win32.h

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)" != ""
cairo_CFLAGS+=$(EXTRA_CFLAGS) $(EXTRA_CDEFS)
cairo_CPPFLAGS+=$(EXTRA_CFLAGS) $(EXTRA_CDEFS)
.ENDIF # "$(EXTRA_CFLAGS)" != ""
.ENDIF # "$(SYSBASE)"!=""
CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS=--enable-static=no --disable-xlib --disable-ft --disable-svg --enable-quartz --enable-quartz-font --enable-gtk-doc=no --enable-test-surfaces=no PKG_CONFIG=./dummy_pkg_config ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)
cairo_CPPFLAGS+=$(EXTRA_CDEFS)
cairo_LDFLAGS+=$(EXTRA_LINKFLAGS)
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2INC+=src$/cairo-quartz.h

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
cairo_LDFLAGS+=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec
.ELIF "$(OS)$(COM)"=="SOLARISC52"
cairo_LDFLAGS+=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF  # "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"

.IF "$(SYSBASE)"!=""
cairo_CFLAGS+=-I$(SYSBASE)$/usr$/include -I$(SOLARINCDIR)$/external $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
cairo_LDFLAGS+=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -L$(SOLARLIBDIR) -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

LDFLAGS:=$(cairo_LDFLAGS)
.EXPORT: LDFLAGS

.IF "$(COMNAME)"=="sunpro5"
cairo_CFLAGS+=-xc99=none
.ENDIF

.IF "$(CPU)"=="I"
cairo_CFLAGS+=-march=i486
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-xlib --enable-ft --disable-svg --enable-gtk-doc=no --enable-test-surfaces=no --enable-static=no ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2INC+=src$/cairo-xlib.h \
     src$/cairo-xlib-xrender.h \
     src$/cairo-ft.h

.ENDIF


# -------- All platforms --------------------------------------------

.IF "$(OS)" != "WNT" || "$(COM)" == "GCC"
# all other platforms except vanilla WNT, which does not use configure

.IF "$(BUILD_PIXMAN)" == "YES"
# pixman is in this module
# We include paths to this module also in LDFLAGS/CFLAGS to guarantee search order.
# However pixman_* vars need to be also set for configure to work properly on all platforms.
CONFIGURE_FLAGS+=pixman_CFLAGS="-I$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/inc" pixman_LIBS="-L$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/lib -lpixman-1"
.ENDIF

CONFIGURE_FLAGS+=CFLAGS="-I$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/inc $(cairo_CFLAGS)" LDFLAGS="-L$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/lib $(cairo_LDFLAGS)" CPPFLAGS="$(cairo_CPPFLAGS)"

.ENDIF

.IF "$(SYSTEM_LIBPNG)" != "YES"
CONFIGURE_FLAGS+=png_CFLAGS="-I$(SOLARINCDIR)$/external$/libpng" png_LIBS="-L$(SOLARLIBDIR) -lpng"
.ENDIF


OUT2INC+=cairo-version.h \
     src$/cairo-deprecated.h \
     src$/cairo-features.h  \
     src$/cairo-pdf.h	\
     src$/cairo-ps.h	\
     src$/cairo.h

.IF "$(OS)"=="MACOSX"
EXTRPATH=LOADER
OUT2LIB+=src$/.libs$/libcairo*.dylib
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2BIN+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
OUT2LIB+=src$/release$/*.lib
OUT2BIN+=src$/release$/*.dll
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/libcairo.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

