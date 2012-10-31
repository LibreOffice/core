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

PRJ=.

PRJNAME=glib
TARGET=so_glib

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GLIB)" == "YES"
all:
	@echo "An already available installation of glib should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

GLIBVERSION=2.28.1

TARFILE_NAME=$(PRJNAME)-$(GLIBVERSION)
TARFILE_MD5=9f6e85e1e38490c3956f4415bcd33e6e


.IF "$(OS)"!="WNT"
PATCH_FILES=glib-2.28.1.patch glib-2.28.1.noise.patch

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS= \
    glib_cv_stack_grows=no \
    glib_cv_uscore=yes \
    ac_cv_func_posix_getpwuid_r=yes \
    ac_cv_func_posix_getgrgid_r=yes \
    ac_cv_func__NSGetEnviron=no \
    --disable-shared
FRAMEWORK=-framework CoreFoundation
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF

.IF "$(OS)"=="MACOSX" && "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= \
    glib_cv_stack_grows=no \
    glib_cv_uscore=yes \
    ac_cv_func_posix_getpwuid_r=yes \
    ac_cv_func_posix_getgrgid_r=yes \
    ac_cv_func__NSGetEnviron=yes
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.ELSE
CONFIGURE_FLAGS+=--prefix=$(SRC_ROOT)$/$(PRJNAME)$/$(MISC)
.END

CONFIGURE_FLAGS+=--disable-fam
CONFIGURE_FLAGS+=CPPFLAGS="$(ARCH_FLAGS) $(EXTRA_CDEFS) -DBUILD_OS_APPLEOSX"
CONFIGURE_FLAGS+=CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS) -I$(SOLARINCDIR)$/external"
CONFIGURE_FLAGS+=LDFLAGS="-L$(SOLARLIBDIR) $(EXTRA_LINKFLAGS) $(FRAMEWORK)"

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
 
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) ./configure

.IF "$(VERBOSE)"!=""
VFLAG=V=1
.ENDIF

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) $(VFLAG) -j$(MAXPROCESS)

.IF "$(OS)"!="IOS"

.IF "$(OS)" == "MACOSX"
my_ext = .0$(DLLPOST)
.ELSE
my_ext = $(DLLPOST).0
.END

OUT2LIB+=gio/.libs/libgio-2.0$(my_ext)
OUT2LIB+=glib/.libs/libglib-2.0$(my_ext)
OUT2LIB+=gmodule/.libs/libgmodule-2.0$(my_ext)
OUT2LIB+=gobject/.libs/libgobject-2.0$(my_ext)
OUT2LIB+=gthread/.libs/libgthread-2.0$(my_ext)

OUT2BIN_NONE+=gobject/glib-mkenums
OUT2BIN_NONE+=gobject/.libs/glib-genmarshal
OUT2BIN_NONE+=gio/.libs/glib-compile-schemas

.ELSE

OUT2LIB+=gio/.libs/libgio-2.0.a
OUT2LIB+=glib/.libs/libglib-2.0.a
OUT2LIB+=gmodule/.libs/libgmodule-2.0.a
OUT2LIB+=gobject/.libs/libgobject-2.0.a
OUT2LIB+=gthread/.libs/libgthread-2.0.a

.ENDIF

.ELSE

CONVERTFILES=gobject/gmarshal.c

PATCH_FILES=glib-2.28.1-win32.patch glib-2.28.1-win32-2.patch

CONFIGURE_ACTION=

ADDITIONAL_FILES= config.h \
    gio/gvdb/makefile.msc \
    gio/win32/makefile.msc \
    glib/glibconfig.h \
    gmodule/gmoduleconf.h \
    gobject/glib-mkenums

BUILD_ACTION=unset debug; nmake -f makefile.msc

OUT2BIN+=gio$/giolo.dll
OUT2BIN+=glib$/gliblo.dll
OUT2BIN+=gmodule$/gmodulelo.dll
OUT2BIN+=gobject$/gobjectlo.dll
OUT2BIN+=gthread$/gthreadlo.dll
OUT2BIN+=gobject$/glib-mkenums
OUT2BIN+=gobject$/glib-genmarshal.exe

OUT2LIB+=build/win32/dirent/dirent.lib
OUT2LIB+=gio/gio-2.0.lib
OUT2LIB+=gio/gvdb/giogvdb.lib
OUT2LIB+=gio/win32/giowin32.lib
OUT2LIB+=glib/glib-2.0.lib
OUT2LIB+=glib/glib-2.28s.lib
OUT2LIB+=glib/gnulib/gnulib.lib
OUT2LIB+=glib/pcre/pcre.lib
OUT2LIB+=gmodule/gmodule-2.0.lib
OUT2LIB+=gobject/glib-genmarshal.lib
OUT2LIB+=gobject/gobject-2.0.lib
OUT2LIB+=gthread/gthread-2.0.lib

.IF "$(OS)"=="WNT"
OUT2INC+=build$/win32$/dirent/dirent.h
.ENDIF
OUT2INC+=build$/win32$/make.msc
OUT2INC+=build$/win32$/module.defs

.ENDIF

OUT2INC+=glib/glib.h
OUT2INC+=glib/glib-object.h
OUT2INC+=glib/glibconfig.h
OUT2INC+=gmodule/gmodule.h

OUT2INC+=gio/gdesktopappinfo.h
OUT2INC+=gio/gunixcredentialsmessage.h
OUT2INC+=gio/gunixinputstream.h
OUT2INC+=gio/gunixsocketaddress.h
OUT2INC+=gio/gfiledescriptorbased.h
OUT2INC+=gio/gunixfdlist.h
OUT2INC+=gio/gunixmounts.h
OUT2INC+=gio/gunixconnection.h
OUT2INC+=gio/gunixfdmessage.h
OUT2INC+=gio/gunixoutputstream.h


OUT2INC+=gio/gaction.h
OUT2INC+=gio/gfileicon.h
OUT2INC+=gio/gproxyaddressenumerator.h
OUT2INC+=gio/gactiongroup.h
OUT2INC+=gio/gfileinfo.h
OUT2INC+=gio/gproxyresolver.h
OUT2INC+=gio/gappinfo.h
OUT2INC+=gio/gfileinputstream.h
OUT2INC+=gio/gresolver.h
OUT2INC+=gio/gapplication.h
OUT2INC+=gio/gfileiostream.h
OUT2INC+=gio/gseekable.h
OUT2INC+=gio/gapplicationcommandline.h
OUT2INC+=gio/gfilemonitor.h
OUT2INC+=gio/gsettings.h
OUT2INC+=gio/gasyncinitable.h
OUT2INC+=gio/gfilenamecompleter.h
OUT2INC+=gio/gsettingsbackend.h
OUT2INC+=gio/gasyncresult.h
OUT2INC+=gio/gfileoutputstream.h
OUT2INC+=gio/gsimpleaction.h
OUT2INC+=gio/gbufferedinputstream.h
OUT2INC+=gio/gfilterinputstream.h
OUT2INC+=gio/gsimpleactiongroup.h
OUT2INC+=gio/gbufferedoutputstream.h
OUT2INC+=gio/gfilteroutputstream.h
OUT2INC+=gio/gsimpleasyncresult.h
OUT2INC+=gio/gcancellable.h
OUT2INC+=gio/gicon.h
OUT2INC+=gio/gsimplepermission.h
OUT2INC+=gio/gcharsetconverter.h
OUT2INC+=gio/ginetaddress.h
OUT2INC+=gio/gsocket.h
OUT2INC+=gio/gcontenttype.h
OUT2INC+=gio/ginetsocketaddress.h
OUT2INC+=gio/gsocketaddress.h
OUT2INC+=gio/gconverter.h
OUT2INC+=gio/ginitable.h
OUT2INC+=gio/gsocketaddressenumerator.h
OUT2INC+=gio/gconverterinputstream.h
OUT2INC+=gio/ginputstream.h
OUT2INC+=gio/gsocketclient.h
OUT2INC+=gio/gconverteroutputstream.h
OUT2INC+=gio/gio.h
OUT2INC+=gio/gsocketconnectable.h
OUT2INC+=gio/gcredentials.h
OUT2INC+=gio/gioenums.h
OUT2INC+=gio/gsocketconnection.h
OUT2INC+=gio/gdatainputstream.h
OUT2INC+=gio/gioenumtypes.h
OUT2INC+=gio/gsocketcontrolmessage.h
OUT2INC+=gio/gdataoutputstream.h
OUT2INC+=gio/gioerror.h
OUT2INC+=gio/gsocketlistener.h
OUT2INC+=gio/gdbusaddress.h
OUT2INC+=gio/giomodule.h
OUT2INC+=gio/gsocketservice.h
OUT2INC+=gio/gdbusauthobserver.h
OUT2INC+=gio/gioscheduler.h
OUT2INC+=gio/gsrvtarget.h
OUT2INC+=gio/gdbusconnection.h
OUT2INC+=gio/giostream.h
OUT2INC+=gio/gtcpconnection.h
OUT2INC+=gio/gdbuserror.h
OUT2INC+=gio/giotypes.h
OUT2INC+=gio/gtcpwrapperconnection.h
OUT2INC+=gio/gdbusintrospection.h
OUT2INC+=gio/gloadableicon.h
OUT2INC+=gio/gthemedicon.h
OUT2INC+=gio/gdbusmessage.h
OUT2INC+=gio/gmemoryinputstream.h
OUT2INC+=gio/gthreadedsocketservice.h
OUT2INC+=gio/gdbusmethodinvocation.h
OUT2INC+=gio/gmemoryoutputstream.h
OUT2INC+=gio/gtlsbackend.h
OUT2INC+=gio/gdbusnameowning.h
OUT2INC+=gio/gmount.h
OUT2INC+=gio/gtlscertificate.h
OUT2INC+=gio/gdbusnamewatching.h
OUT2INC+=gio/gmountoperation.h
OUT2INC+=gio/gtlsclientconnection.h
OUT2INC+=gio/gdbusproxy.h
OUT2INC+=gio/gnativevolumemonitor.h
OUT2INC+=gio/gtlsconnection.h
OUT2INC+=gio/gdbusserver.h
OUT2INC+=gio/gnetworkaddress.h
OUT2INC+=gio/gtlsserverconnection.h
OUT2INC+=gio/gdbusutils.h
OUT2INC+=gio/gnetworkservice.h
OUT2INC+=gio/gvfs.h
OUT2INC+=gio/gdrive.h
OUT2INC+=gio/goutputstream.h
OUT2INC+=gio/gvolume.h
OUT2INC+=gio/gemblem.h
OUT2INC+=gio/gpermission.h
OUT2INC+=gio/gvolumemonitor.h
OUT2INC+=gio/gemblemedicon.h
OUT2INC+=gio/gpollableinputstream.h
OUT2INC+=gio/gzlibcompressor.h
OUT2INC+=gio/gfile.h
OUT2INC+=gio/gpollableoutputstream.h
OUT2INC+=gio/gzlibdecompressor.h
OUT2INC+=gio/gfileattribute.h
OUT2INC+=gio/gproxy.h
OUT2INC+=gio/gfileenumerator.h
OUT2INC+=gio/gproxyaddress.h

OUT2INC+=glib/galloca.h
OUT2INC+=glib/gconvert.h
OUT2INC+=glib/gi18n.h
OUT2INC+=glib/goption.h
OUT2INC+=glib/gscanner.h
OUT2INC+=glib/gthreadpool.h
OUT2INC+=glib/garray.h
OUT2INC+=glib/gdataset.h
OUT2INC+=glib/giochannel.h
OUT2INC+=glib/gpattern.h
OUT2INC+=glib/gsequence.h
OUT2INC+=glib/gtimer.h
OUT2INC+=glib/gasyncqueue.h
OUT2INC+=glib/gdate.h
OUT2INC+=glib/gkeyfile.h
OUT2INC+=glib/gpoll.h
OUT2INC+=glib/gshell.h
OUT2INC+=glib/gtimezone.h
OUT2INC+=glib/gatomic.h
OUT2INC+=glib/gdatetime.h
OUT2INC+=glib/glist.h
OUT2INC+=glib/gprimes.h
OUT2INC+=glib/gslice.h
OUT2INC+=glib/gtree.h
OUT2INC+=glib/gbacktrace.h
OUT2INC+=glib/gdir.h
OUT2INC+=glib/gmacros.h
OUT2INC+=glib/gprintf.h
OUT2INC+=glib/gslist.h
OUT2INC+=glib/gtypes.h
OUT2INC+=glib/gbase64.h
OUT2INC+=glib/gerror.h
OUT2INC+=glib/gmain.h
OUT2INC+=glib/gqsort.h
OUT2INC+=glib/gspawn.h
OUT2INC+=glib/gunicode.h
OUT2INC+=glib/gbitlock.h
OUT2INC+=glib/gfileutils.h
OUT2INC+=glib/gmappedfile.h
OUT2INC+=glib/gquark.h
OUT2INC+=glib/gstdio.h
OUT2INC+=glib/gurifuncs.h
OUT2INC+=glib/gbookmarkfile.h
OUT2INC+=glib/ghash.h
OUT2INC+=glib/gmarkup.h
OUT2INC+=glib/gqueue.h
OUT2INC+=glib/gstrfuncs.h
OUT2INC+=glib/gutils.h
OUT2INC+=glib/gcache.h
OUT2INC+=glib/ghook.h
OUT2INC+=glib/gmem.h
OUT2INC+=glib/grand.h
OUT2INC+=glib/gstring.h
OUT2INC+=glib/gvariant.h
OUT2INC+=glib/gchecksum.h
OUT2INC+=glib/ghostutils.h
OUT2INC+=glib/gmessages.h
OUT2INC+=glib/gregex.h
OUT2INC+=glib/gtestutils.h
OUT2INC+=glib/gvarianttype.h
OUT2INC+=glib/gcompletion.h
OUT2INC+=glib/gi18n-lib.h
OUT2INC+=glib/gnode.h
OUT2INC+=glib/grel.h
OUT2INC+=glib/gthread.h
OUT2INC+=glib/gwin32.h


OUT2INC+=gobject/gbinding.h
OUT2INC+=gobject/gmarshal.h
OUT2INC+=gobject/gparamspecs.h
OUT2INC+=gobject/gtypemodule.h
OUT2INC+=gobject/gvaluecollector.h
OUT2INC+=gobject/gboxed.h
OUT2INC+=gobject/gobject.h
OUT2INC+=gobject/gsignal.h
OUT2INC+=gobject/gtypeplugin.h
OUT2INC+=gobject/gvaluetypes.h
OUT2INC+=gobject/gclosure.h
OUT2INC+=gobject/gobjectnotifyqueue.c  
OUT2INC+=gobject/gsourceclosure.h
OUT2INC+=gobject/gvalue.h
OUT2INC+=gobject/genums.h
OUT2INC+=gobject/gparam.h
OUT2INC+=gobject/gtype.h
OUT2INC+=gobject/gvaluearray.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

