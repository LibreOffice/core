#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:56:35 $
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

PRJ=..$/..

PRJNAME=sysui
TARGET=launcher

TARGETTYPE=CUI
LIBTARGET=NO
NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  ..$/productversion.mk

.IF "$(ENABLE_GNOMEVFS)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gnome-vfs-2.0 ORBit-2.0 ORBit-CosNaming-2.0 gconf-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
.IF "$(SYSBASE)"!=""
.IF "$(COMNAME)"=="sunpro5"
# soften C restrictions to survive "inline" keyword
CFLAGSCC!:=$(subst,-xc99=none,-xc99=no_lib $(CFLAGSCC))
.ENDIF          # "$(COMNAME)"=="sunpro5"
.ENDIF			# "$(SYSBASE)"!=""

LINKFLAGS+=-z nodefs
.ENDIF # "$(OS)" == "SOLARIS"

# Disable lazy binding of symbols
.IF "$(OS)" == "LINUX"
LINKFLAGS+=-Wl,-z,now
.ENDIF
.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z now
.ENDIF  

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS)
.ENDIF

.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"
RPMTARFILES=$(BIN)$/rpm$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
.ENDIF

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"
DEBTARFILES=$(BIN)$/deb$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(ENABLE_GNOMEVFS)"!=""
OBJFILES += $(OBJ)$/gnome-set-default-application.obj

APP1TARGET=gnome-set-default-application
APP1OBJS=$(OBJ)$/gnome-set-default-application.obj
APP1LIBS=
APP1STDLIBS+=$(PKGCONFIG_LIBS)
.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR: $(RPMTARFILES) $(DEBTARFILES)

.IF "$(RPMTARFILES)" != ""

$(RPMTARFILES) : $(PKGDIR)
    $(MKDIRHIER) $(@:d)
    tar -C $(PKGDIR:d:d) -cf - $(PKGDIR:f)$/{$(shell @cd $(PKGDIR); ls $(@:b:b:s/-/ /:1)*)} | gzip > $@

.ENDIF # "$(TARFILE)" != ""

.IF "$(DEBTARFILES)" != ""

$(DEBTARFILES) : $(PKGDIR)
    $(MKDIRHIER) $(@:d)
    tar -C $(PKGDIR:d:d) -cf - $(PKGDIR:f)$/{$(shell @cd $(PKGDIR); ls $(@:b:b:s/-/ /:1)*.deb)} | gzip > $@
    
.ENDIF
