#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.39 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 15:01:47 $
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

PRJNAME=transex
TARGET=transex3
TARGETTYPE=CUI
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
CDEFS+= -DYY_NEVER_INTERACTIVE=1

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=   			\
    $(OBJ)$/export.obj	\
    $(OBJ)$/export2.obj	\
    $(OBJ)$/merge.obj   \
    $(OBJ)$/wrdtrans.obj	\
    $(OBJ)$/wtratree.obj	\
    $(OBJ)$/wtranode.obj    \
    $(OBJ)$/srciter.obj		\
    $(OBJ)$/utf8conv.obj	\
    $(OBJ)$/xmlparse.obj    \
    $(OBJ)$/helpmerge.obj   \
    $(OBJ)$/helpex.obj      \
    $(OBJ)$/hw2fw.obj

LIB1TARGET= $(LB)$/transex.lib
LIB1ARCHIV= $(LB)$/libtransex.a
#LIB1FILES=  $(LB)$/transex3.lib
LIB1OBJFILES=        $(OBJ)$/export.obj      \
        $(OBJ)$/export2.obj     \
        $(OBJ)$/merge.obj   \
        $(OBJ)$/wrdtrans.obj    \
        $(OBJ)$/wtratree.obj    \
        $(OBJ)$/wtranode.obj    \
        $(OBJ)$/srciter.obj             \
        $(OBJ)$/utf8conv.obj    \
        $(OBJ)$/hw2fw.obj

APP1VERSIONMAP=exports.map

# extractor and merger for *.src and *.hrc
APP1TARGET=     $(TARGET)
APP1OBJS=   $(OBJ)$/src_yy.obj

.IF "$(GUI)"=="WNT"
BOOTSTRP2 = bootstrp2.lib
.ELSE
BOOTSTRP2 = -lbootstrp2
.ENDIF
.IF "$(OS)"!="MACOSX"
APP1STDLIBS+= $(BTSTRPLIB) $(BOOTSTRP2)
.ENDIF

APP1STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP1STDLIBS+= $(BTSTRPLIB) $(BOOTSTRP2)
.ENDIF

APP1LIBS+=	$(LB)$/$(PRJNAME).lib
APP1DEPN=   $(OBJ)$/src_yy.obj $(LB)$/$(PRJNAME).lib

#APP2TARGET= termilo
#APP2STACK=  16000
#APP2OBJS=   $(OBJ)$/termino.obj
#APP2STDLIBS=$(TOOLSLIBST) $(L)$/bootstrp.lib

APP2TARGET= helpex
APP2OBJS= $(OBJ)$/helpmerge.obj  $(OBJ)$/xmlparse.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj $(OBJ)$/merge.obj $(OBJ)$/helpex.obj $(OBJ)$/hw2fw.obj

.IF "$(OS)"!="MACOSX"
APP2STDLIBS+= $(BTSTRPLIB)
.ENDIF

APP2STDLIBS+=$(SALLIB) $(EXPATASCII3RDLIB) $(TOOLSLIB) $(VOSLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP2STDLIBS+= $(BTSTRPLIB)
.ENDIF

# extractor and merger for *.lng and *.lng
APP3TARGET= ulfex
APP3OBJS=   $(OBJ)$/lngmerge.obj $(OBJ)$/hw2fw.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/lngex.obj $(OBJ)$/utf8conv.obj

.IF "$(OS)"!="MACOSX"
APP3STDLIBS+= $(BTSTRPLIB)
.ENDIF
APP3STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)
.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP3STDLIBS+= $(BTSTRPLIB)
.ENDIF

# encoding converter for *.gsi
APP4TARGET= gsiconv
APP4OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/gsiconv.obj
APP4STDLIBS+= \
            $(BTSTRPLIB) \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

# tag checker for *.gsi
APP5TARGET= gsicheck
APP5OBJS=   $(OBJ)$/gsicheck.obj $(OBJ)$/tagtest.obj
APP5STDLIBS+= \
            $(BTSTRPLIB) \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

# extractor and merger for *.cfg
APP6TARGET= cfgex
APP6OBJS=   $(OBJ)$/cfgmerge.obj $(OBJ)$/cfg_yy.obj $(OBJ)$/hw2fw.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj

.IF "$(OS)"!="MACOSX"
APP6STDLIBS+= $(BTSTRPLIB)
.ENDIF

APP6STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP6STDLIBS+= $(BTSTRPLIB)
.ENDIF

# extractor and merger for *.xrm
APP7TARGET= xrmex
APP7OBJS=   $(OBJ)$/xrmmerge.obj $(OBJ)$/xrm_yy.obj $(OBJ)$/hw2fw.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj

.IF "$(OS)"!="MACOSX"
APP7STDLIBS+= $(BTSTRPLIB)
.ENDIF

APP7STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP7STDLIBS+= $(BTSTRPLIB)
.ENDIF

#APP8TARGET=  sdfupdate
#APP8OBJS= $(OBJ)$/sdfupdate.obj
#APP8STDLIBS+= \
#            $(TOOLSLIB) \
#            $(VOSLIB) \
#            $(SALLIB)

#APP8TARGET= xgfconv
#APP8STACK=  16000
#APP8OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/xgfconv.obj $(OBJ)$/export2.obj
#APP8STDLIBS=$(BTSTRPLIB) $(TOOLSLIBST)

# encoding converter for text files
APP8TARGET= txtconv
APP8STACK=  16000
APP8OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/txtconv.obj $(OBJ)$/hw2fw.obj
APP8STDLIBS=$(TOOLSLIB) $(SALLIB)

# localizer for l10n framework
APP9TARGET= localize_sl
EXCEPTIONSFILES=                            \
                    $(OBJ)$/localize.obj
APP9OBJS=   $(OBJ)$/localize.obj $(OBJ)$/utf8conv.obj $(OBJ)$/srciter.obj $(OBJ)$/export2.obj

.IF "$(OS)"!="MACOSX"
.IF "$(GUI)"=="UNX"
APP9STDLIBS+= $(BTSTRPLIB) -lbootstrp2
.ELSE
APP9STDLIBS+= $(BTSTRPLIB) $(LIBPRE) bootstrp2.lib
.ENDIF
.ENDIF

APP9STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
APP9STDLIBS+= $(BTSTRPLIB) $(BOOTSTRP2)
.ENDIF

DEPOBJFILES=$(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS) $(APP6OBJS) $(APP7OBJS) $(APP8OBJS) $(APP9OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%_yy.c : %lex.l
    +flex -l -8 -o$@ $<

