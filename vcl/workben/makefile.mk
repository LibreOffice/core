#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: obo $ $Date: 2008-01-07 09:53:26 $
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

PRJNAME=vcl
TARGET=svdem
LIBTARGET=NO
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/svdem.obj \
    $(OBJ)$/vcldemo.obj \
    $(OBJ)$/outdevgrind.obj

# --- Targets ------------------------------------------------------

# svdem

APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1OBJS= \
    $(OBJ)$/svdem.obj

APP1STDLIBS=	$(CPPULIB)			\
                $(UNOTOOLSLIB)	\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)

# outdevgrind

APP2TARGET= outdevgrind
APP2OBJS=	\
    $(OBJ)$/outdevgrind.obj $(OBJ)$/salmain.obj

APP2NOSAL=		TRUE
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)

# vcldemo

APP3NOSAL=TRUE
APP3TARGET=vcldemo
APP3OBJS= \
    $(OBJ)$/vcldemo.obj

APP3STDLIBS=	$(CPPULIB)			\
                $(UNOTOOLSLIB)	\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)


# --- Targets ------------------------------------------------------
.IF "$(GUIBASE)" == "unx"

APP4NOSAL=              TRUE
APP4TARGET=     svptest
APP4OBJS=               $(OBJ)$/svptest.obj

APP4STDLIBS=    $(CPPULIB)                      \
                                $(CPPUHELPERLIB)        \
                                $(COMPHELPERLIB)        \
                                $(VCLLIB)                       \
                                $(TOOLSLIB)             \
                                $(SALLIB)                       \
                                $(VOSLIB)                       \
                                $(SOTLIB)                       \
                                $(VCLLIB)

APP5NOSAL=              TRUE
APP5TARGET=     svpclient
APP5OBJS=               $(OBJ)$/svpclient.obj

APP5STDLIBS=    $(CPPULIB)                      \
                                $(CPPUHELPERLIB)        \
                                $(COMPHELPERLIB)        \
                                $(VCLLIB)                       \
                                $(TOOLSLIB)             \
                                $(SALLIB)                       \
                                $(VOSLIB)                       \
                                $(SOTLIB)                       \
                                $(VCLLIB)

.IF "$(OS)" == "SOLARIS"
APP5STDLIBS+=-lsocket
.ENDIF

.ENDIF

.INCLUDE :	target.mk

ALLTAR : $(BIN)$/applicat.rdb


$(BIN)$/applicat.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
     cd $(BIN) && \
         regcomp -register -r applicat.rdb \
             -c i18nsearch.uno$(DLLPOST) \
             -c i18npool.uno$(DLLPOST)

